#!/usr/bin/env python3
"""Fast protocol and offline-pipeline tests; never assert performance thresholds."""
import csv
import importlib.util
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
import xml.etree.ElementTree as ET

RUNNER = Path(sys.argv[1]).resolve()
sys.argv = sys.argv[:1]
ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("benchmark_plot", ROOT / "benchmarks/plot.py")
plot = importlib.util.module_from_spec(spec)
spec.loader.exec_module(plot)


class Benchmarks(unittest.TestCase):
    def command(self, *args, ok=True):
        result = subprocess.run([str(arg) for arg in args], capture_output=True, text=True, timeout=30)
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
                second = list(csv.DictReader(self.command(*command).stdout.splitlines()))
                self.assertEqual(len(first), 2)
                self.assertEqual(first[0]["group"], group)
                self.assertEqual(first[0]["axis"], axis)
                self.assertEqual([r["checksum"] for r in first], [r["checksum"] for r in second])
                for row in first:
                    self.assertGreater(int(row["elapsed_ns"]), 0)
                    self.assertAlmostEqual(float(row["ns_per_op"]), int(row["elapsed_ns"]) / 20)

    def test_invalid_worker_arguments(self):
        for name, size, count in [("missing", 64, 1), ("isqrt", 0, 1),
                                  ("isqrt", 65537, 1), ("isqrt", 64, "-1")]:
            self.command(RUNNER, name, size, count, 1, 42, 1, 1, ok=False)

    def test_pipeline_and_validation(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            result = self.command(sys.executable, ROOT / "benchmarks/run.py", "--runner", RUNNER,
                                  "--output", root / "csv", "--algorithms", "isqrt", "div3",
                                  "--sizes", 16, 64, "--iterations", 20, "--samples", 3, "--warmups", 1)
            run = Path(result.stdout.strip())
            metadata = json.loads((run / "metadata.json").read_text())
            self.assertEqual(metadata["parameters"]["seed"], 42)
            self.assertEqual(len(metadata["runner_sha256"]), 64)
            # Offline plot generation has no access to, or dependency on, the worker.
            output = root / "plots"
            self.command(sys.executable, ROOT / "benchmarks/plot.py", "--input", run, "--output", output)
            for image in output.rglob("*.svg"):
                ET.parse(image)
            self.assertEqual(len(list(output.rglob("*.svg"))), 3)
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
            bad_rows = [lines + [lines[1]],
                        [lines[0], "2" + lines[1][1:]],
                        [lines[0], lines[1].replace(",isqrt,", ",../isqrt,")],
                        [lines[0], lines[1].rsplit(",", 1)[0] + ",nan"],
                        [lines[0], lines[1] + ",extra"], [lines[0]]]
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
            self.command(sys.executable, ROOT / "benchmarks/run.py", "--runner", RUNNER,
                         "--output", root, "--algorithms", "not_an_algorithm", ok=False)
            self.assertEqual(list(root.iterdir()), [])


if __name__ == "__main__":
    unittest.main()
