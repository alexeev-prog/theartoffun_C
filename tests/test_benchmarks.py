#!/usr/bin/env python3
"""Fast protocol and offline-pipeline tests; never assert performance thresholds."""

import csv
import importlib.util
import json
import re
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
import zipfile
from html.parser import HTMLParser
from urllib.parse import unquote
import xml.etree.ElementTree as ET

RUNNER = Path(sys.argv[1]).resolve()
sys.argv = sys.argv[:1]
ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "benchmarks"))
spec = importlib.util.spec_from_file_location(
    "benchmark_plot", ROOT / "benchmarks/plot.py"
)
plot = importlib.util.module_from_spec(spec)
spec.loader.exec_module(plot)


class Benchmarks(unittest.TestCase):
    def command(self, *args, ok=True):
        result = subprocess.run(
            [str(arg) for arg in args], capture_output=True, text=True, timeout=30
        )
        self.assertEqual(result.returncode == 0, ok, result.stderr)
        return result

    def test_all_workers_and_reproducibility(self):
        catalog = self.command(RUNNER, "--list").stdout.splitlines()
        self.assertGreaterEqual(len(catalog), 20)
        for entry in catalog:
            group, name, axis = entry.split("\t")
            with self.subTest(name=name):
                command = [RUNNER, name, 64, 20, 2, 42, 1, 1]
                first = list(csv.DictReader(self.command(*command).stdout.splitlines()))
                second = list(
                    csv.DictReader(self.command(*command).stdout.splitlines())
                )
                self.assertEqual(len(first), 2)
                self.assertEqual(first[0]["group"], group)
                self.assertEqual(first[0]["axis"], axis)
                self.assertEqual(
                    [r["checksum"] for r in first], [r["checksum"] for r in second]
                )
                for row in first:
                    self.assertGreater(int(row["elapsed_ns"]), 0)
                    self.assertAlmostEqual(
                        float(row["ns_per_op"]), int(row["elapsed_ns"]) / 20
                    )

    def test_public_algorithm_coverage(self):
        manifest = json.loads((ROOT / "benchmarks/coverage.json").read_text())
        names = {
            line.split("\t")[1]
            for line in self.command(RUNNER, "--list").stdout.splitlines()
        }
        public = set()
        for header in (
            "algos.h",
            "pow_algos.h",
            "fib_algos.h",
            "compressing.h",
            "mortoncode.h",
            "levenshtein.h",
        ):
            text = (ROOT / "include" / header).read_text()
            public.update(
                re.findall(r"^\w+[\w\s*]*?\b(\w+)\([^;{}]*\);", text, re.MULTILINE)
            )
        for function in sorted(public):
            if function in manifest["excluded"]:
                self.assertTrue(manifest["excluded"][function])
            else:
                self.assertIn(
                    manifest["aliases"].get(function, function),
                    names,
                    f"Public algorithm {function} needs a benchmark or explicit exclusion",
                )
        self.assertFalse(set(manifest["excluded"]) - public)
        self.assertFalse(set(manifest["aliases"]) - public)

    def test_examples_and_cli_coverage(self):
        names = {
            line.split("\t")[1]
            for line in self.command(RUNNER, "--list").stdout.splitlines()
        }
        manifest = json.loads((ROOT / "benchmarks/examples.json").read_text())
        self.assertEqual(
            set(manifest), {p.name for p in (ROOT / "examples").glob("*.c")}
        )
        for file, entry in manifest.items():
            if "excluded" in entry:
                self.assertTrue(entry["excluded"])
                continue
            self.assertTrue(entry["benchmarks"])
            self.assertFalse(set(entry["benchmarks"]) - names, file)
            source = (ROOT / "examples" / file).read_text()
            functions = set(
                re.findall(
                    r"^(?:static\s+)?(?:inline\s+)?\w+[\w\s*]*?\b(\w+)\([^;{}]*\)\s*\{",
                    source,
                    re.MULTILINE,
                )
            )
            self.assertFalse(
                functions
                - {"main"}
                - set(entry["benchmarks"])
                - set(entry.get("excluded_functions", {})),
                file,
            )
        cli = json.loads((ROOT / "benchmarks/cli.json").read_text())
        source = (
            (ROOT / "src/main.c")
            .read_text()
            .split("struct CommandOption options[] = {", 1)[1]
            .split("};", 1)[0]
        )
        options = set(re.findall(r'\.long_name\s*=\s*"([^"]+)"', source))
        options.update(re.findall(r'\{\s*"[^"]*",\s*"([^"]+)"', source))
        self.assertEqual(set(cli), options)
        for option, functions in cli.items():
            self.assertFalse(set(functions) - names, option)
            self.assertTrue(functions or option in {"help", "exponent", "benchmark"})

    def test_invalid_worker_arguments(self):
        for name, size, count in [
            ("missing", 64, 1),
            ("isqrt", 0, 1),
            ("isqrt", 65537, 1),
            ("isqrt", 64, "-1"),
        ]:
            self.command(RUNNER, name, size, count, 1, 42, 1, 1, ok=False)

    def test_pipeline_and_validation(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            result = self.command(
                sys.executable,
                ROOT / "benchmarks/run.py",
                "--runner",
                RUNNER,
                "--output",
                root / "csv",
                "--algorithms",
                "isqrt",
                "div3",
                "--sizes",
                16,
                64,
                "--iterations",
                20,
                "--samples",
                3,
                "--warmups",
                1,
                "--processes",
                1,
                "--scenarios",
                "random",
                "--min-time-ms",
                1,
            )
            run = Path(result.stdout.strip())
            metadata = json.loads((run / "metadata.json").read_text())
            self.assertEqual(metadata["parameters"]["seed"], 42)
            self.assertEqual(len(metadata["runner_sha256"]), 64)
            # Offline plot generation has no access to, or dependency on, the worker.
            output = root / "plots"
            self.command(
                sys.executable,
                ROOT / "benchmarks/plot.py",
                "--input",
                run,
                "--output",
                output,
            )
            for image in output.rglob("*.svg"):
                ET.parse(image)
            self.assertEqual(len(list(output.rglob("*.svg"))), 3)
            with zipfile.ZipFile(output / "site.zip") as archive:
                self.assertIn("index.html", archive.namelist())
                self.assertIn("data/metadata.json", archive.namelist())
                self.assertNotIn("site.zip", archive.namelist())

            class Links(HTMLParser):
                def __init__(self):
                    super().__init__()
                    self.links = []
                    self.tags = []

                def handle_starttag(self, tag, attrs):
                    self.tags.append(tag)
                    self.links.extend(value for key, value in attrs if key == "href")

            for document in output.rglob("*.html"):
                parser = Links()
                parser.feed(document.read_text())
                self.assertNotIn("img", parser.tags)
                self.assertNotIn("script", parser.tags)
                for link in parser.links:
                    self.assertTrue(
                        (document.parent / unquote(link.split("#")[0])).exists(),
                        f"Broken link in {document}: {link}",
                    )
            index = (output / "index.html").read_text()
            self.assertIn("font-family:serif", index)
            self.assertNotIn("border-radius", index)
            self.assertIn("iterations/batch", index)
            with (output / "summary.csv").open() as stream:
                summary = list(csv.DictReader(stream))
            self.assertEqual(len(summary), 4)
            self.assertTrue(all(row["samples"] == "3" for row in summary))
            with self.assertRaises(ValueError):
                plot.generate(run, output)
            source = run / "integer/isqrt/samples.csv"
            original = source.read_text()
            # Duplicate sample, unsupported schema, NaN, path spoofing, malformed row.
            lines = original.splitlines()
            bad_rows = [
                lines + [lines[1]],
                [lines[0], "9" + lines[1][1:]],
                [lines[0], lines[1].replace(",isqrt,", ",../isqrt,")],
                [lines[0], lines[1].replace(",random,", ",../random,")],
                [lines[0], lines[1] + ",extra"],
                [lines[0]],
            ]
            for bad in bad_rows:
                source.write_text("\n".join(bad) + "\n")
                with self.assertRaises(ValueError):
                    plot.load(run)
            source.write_text(original)
            with self.assertRaises(ValueError):
                plot.load(root / "empty")

    def test_failed_run_not_published(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            self.command(
                sys.executable,
                ROOT / "benchmarks/run.py",
                "--runner",
                RUNNER,
                "--output",
                root,
                "--algorithms",
                "not_an_algorithm",
                ok=False,
            )
            self.assertEqual(list(root.iterdir()), [])

    def test_legacy_run_and_zero_reference(self):
        from report import accuracy_data
        from data import write_csv, ACCURACY

        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            source = root / "old"
            path = source / "power/libm_pow/samples.csv"
            path.parent.mkdir(parents=True)
            path.write_text(self.command(RUNNER, "libm_pow", 1, 10, 2, 42, 1, 1).stdout)
            write_csv(
                path.with_name("accuracy.csv"),
                ACCURACY,
                [
                    dict(
                        zip(
                            ACCURACY,
                            [
                                "2",
                                "power",
                                "libm_pow",
                                "legacy",
                                "1",
                                "1",
                                "0",
                                "0",
                                "0",
                                "0",
                                "",
                            ],
                        )
                    )
                ],
            )
            self.assertIsNone(next(iter(accuracy_data(source).values()))[0]["relative"])
            plot.generate(source, root / "site")
            self.assertTrue((root / "site/index.html").exists())

    def test_accuracy_scenarios_and_report(self):
        from report import accuracy_data

        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            result = self.command(
                sys.executable,
                ROOT / "benchmarks/run.py",
                "--runner",
                RUNNER,
                "--output",
                root / "raw",
                "--algorithms",
                "libm_pow",
                "fast_pow",
                "--sizes",
                16,
                "--samples",
                2,
                "--processes",
                2,
                "--min-time-ms",
                1,
                "--warmups",
                1,
                "--iterations",
                10,
            )
            run = Path(result.stdout.strip())
            errors = accuracy_data(run)
            self.assertEqual(len(errors), 10)
            self.assertTrue(
                all(
                    p["absolute"] == 0
                    for (g, n, s, z), points in errors.items()
                    if n == "libm_pow"
                    for p in points
                )
            )
            plot.generate(run, root / "site")
            report = json.loads((root / "site/report.json").read_text())
            self.assertEqual(len(report["accuracy"]), 10)
            self.assertEqual(len(report["timing"]), 10)
            self.assertTrue(all(r["samples"] == 4 for r in report["timing"]))
            self.assertIn("cpu_model", report["metadata"]["environment"])
            self.assertEqual(len(list((root / "site").rglob("*tradeoff.svg"))), 5)
            for image in (root / "site").rglob("*.svg"):
                ET.parse(image)
            path = run / "power/fast_pow/accuracy.csv"
            text = path.read_text()
            lines = text.splitlines()
            row = lines[1].split(",")
            row[-2] = "999"
            path.write_text(lines[0] + "\n" + ",".join(row) + "\n")
            with self.assertRaises(ValueError):
                plot.generate(run, root / "bad")
            self.assertFalse((root / "bad").exists())


if __name__ == "__main__":
    unittest.main()
