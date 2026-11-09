"""Offline report assembly; charts are injected to keep rendering reusable."""

from collections import defaultdict
from html import escape
import json
import math
from pathlib import Path
import shutil
import statistics
import zipfile
import posixpath
import re

from data import ACCURACY, finite, read_csv, write_csv


def page(title, body):
    return (
        '<!doctype html><html lang="en"><head><meta charset="utf-8">'
        '<meta name="viewport" content="width=device-width,initial-scale=1">'
        f"<title>{escape(title)}</title><style>"
        "body{font-family:serif;line-height:1.5;max-width:80em;margin:2em auto;padding:0 1em}"
        "table{border-collapse:collapse}td,th{text-align:left;vertical-align:top;padding:.4em .8em;border-bottom:1px solid #ccc}"
        ".scroll{overflow-x:auto}pre{white-space:pre-wrap;overflow-wrap:anywhere}"
        "</style></head><body>" + body + "</body></html>\n"
    )


def accuracy_data(source):
    result = defaultdict(list)
    for path in sorted(source.glob("*/*/accuracy.csv")):
        fields, rows = read_csv(path)
        if fields != ACCURACY:
            raise ValueError(f"{path}: invalid accuracy header")
        seen = set()
        for row in rows:
            group, name, scenario = (row[k] for k in ("group", "algorithm", "scenario"))
            if (group, name) != (path.parent.parent.name, path.parent.name):
                raise ValueError(f"{path}: mismatched accuracy identity")
            if not scenario.isidentifier() or row["schema_version"] != "2":
                raise ValueError(f"{path}: invalid scenario/schema")
            size, point = int(row["size"]), int(row["point"])
            if size < 1 or point < 1 or (scenario, size, point) in seen:
                raise ValueError(f"{path}: invalid/duplicate accuracy point")
            seen.add((scenario, size, point))
            actual, reference = finite(row["actual"]), finite(row["reference"])
            absolute = finite(row["absolute_error"], 0)
            relative = (
                finite(row["relative_error"], 0) if row["relative_error"] else None
            )
            if not math.isclose(
                absolute, abs(actual - reference), rel_tol=1e-8, abs_tol=1e-14
            ):
                raise ValueError(f"{path}: inconsistent absolute error")
            if (reference == 0 and relative is not None) or (
                reference != 0
                and (
                    relative is None
                    or not math.isclose(
                        relative, absolute / abs(reference), rel_tol=1e-8, abs_tol=1e-14
                    )
                )
            ):
                raise ValueError(f"{path}: inconsistent relative error")
            result[(group, name, scenario, size)].append(
                {
                    "input": finite(row["input"]),
                    "absolute": absolute,
                    "relative": relative,
                }
            )
    return result


def table(headers, rows):
    def cell(value):
        return escape(f"{value:.6g}" if isinstance(value, float) else str(value))

    return (
        '<div class="scroll"><table><thead><tr>'
        + "".join(f"<th>{cell(h)}</th>" for h in headers)
        + "</tr></thead><tbody>"
        + "".join(
            "<tr>" + "".join(f"<td>{cell(v)}</td>" for v in row) + "</tr>"
            for row in rows
        )
        + "</tbody></table></div>"
    )


def build(source, output, series, chart):
    errors = accuracy_data(source)
    for group, name, scenario, size in errors:
        if not any(
            g == group and n == name and s == scenario and size in values
            for (g, n, _, s), values in series.items()
        ):
            raise ValueError("accuracy point has no matching timing workload")
    metadata_path = source / "metadata.json"
    metadata = (
        json.loads(metadata_path.read_text())
        if metadata_path.exists()
        else {"warning": "Legacy run: metadata unavailable"}
    )
    if output.exists() and any(output.iterdir()):
        raise ValueError(f"output directory is not empty: {output}")
    if (
        output.resolve() == source.resolve()
        or output.resolve() in source.resolve().parents
    ):
        raise ValueError("output must not equal or contain input")
    if source.resolve() in output.resolve().parents:
        raise ValueError("output must not be inside input")
    output.mkdir(parents=True, exist_ok=True)
    # Explicit whitelist: never copy executables or unrelated files into a published site.
    downloads = []
    for path in sorted(source.glob("*/*/*.csv")) + (
        [metadata_path] if metadata_path.exists() else []
    ):
        if path.name not in ("samples.csv", "accuracy.csv", "metadata.json"):
            continue
        relative = Path("data") / path.relative_to(source)
        (output / relative).parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(path, output / relative)
        downloads.append(str(relative))
    catalog, timing_rows, accuracy_rows, warnings = [], [], [], []
    raw_samples = {}
    groups = defaultdict(dict)
    tradeoffs = defaultdict(dict)
    maxima = defaultdict(lambda: defaultdict(dict))

    def image(path, title, axis, lines, **kwargs):
        (output / path).parent.mkdir(parents=True, exist_ok=True)
        (output / path).write_text(
            chart(title, axis, lines, **kwargs), encoding="utf-8"
        )
        return f'<p><a href="{path}">{escape(title)}</a></p>'

    for (group, name, axis, scenario), values in sorted(series.items()):
        base = Path(group) / name / scenario
        body = image(base / "timing.svg", f"{name} / {scenario}", axis, {name: values})
        groups[(group, axis, scenario)][name] = values
        stats = []
        for size, samples in sorted(values.items()):
            median = statistics.median(samples)
            mean = statistics.mean(samples)
            deviation = statistics.stdev(samples) if len(samples) > 1 else 0
            cv = deviation / mean
            row = dict(
                group=group,
                algorithm=name,
                scenario=scenario,
                axis=axis,
                size=size,
                samples=len(samples),
                median_ns=median,
                min_ns=min(samples),
                max_ns=max(samples),
                mean_ns=mean,
                stdev_ns=deviation,
                cv=cv,
            )
            timing_rows.append(row)
            stats.append([size, len(samples), median, min(samples), max(samples), cv])
            if cv > 0.1:
                warnings.append(
                    f"{group}/{name}/{scenario}, size={size}: CV={cv:.1%}; unstable timing"
                )
            points = errors.get((group, name, scenario, size), [])
            if not points:
                continue
            metrics = {}
            for metric in ("absolute", "relative"):
                observed = [p[metric] for p in points if p[metric] is not None]
                if not observed:
                    continue
                ordered = sorted(observed)
                metrics[metric] = dict(
                    max=max(observed),
                    mean=statistics.mean(observed),
                    median=statistics.median(observed),
                    p95=ordered[math.ceil(0.95 * len(ordered)) - 1],
                )
                maxima[(group, scenario, metric)][name][size] = [max(observed)]
                by_input = defaultdict(list)
                for p in points:
                    if p[metric] is not None:
                        by_input[p["input"]].append(p[metric])
                body += image(
                    base / f"{size}-{metric}.svg",
                    f"{name}: {metric} error, size={size}",
                    "raw input (power base = 1 + input / size)",
                    {name: by_input},
                    ylabel=f"{metric} error",
                    subtitle="median and min–max at each input",
                )
                # Empirical CDF: preserves zeros and avoids arbitrary histogram bins.
                cdf = {
                    value: [100 * (index + 1) / len(ordered)]
                    for index, value in enumerate(ordered)
                }
                body += image(
                    base / f"{size}-{metric}-cdf.svg",
                    f"{name}: {metric} error distribution, size={size}",
                    f"{metric} error",
                    {name: cdf},
                    ylabel="cumulative observations (%)",
                    subtitle="empirical cumulative distribution (ECDF)",
                )
            accuracy_rows.append(
                dict(
                    group=group,
                    algorithm=name,
                    scenario=scenario,
                    size=size,
                    points=len(points),
                    **{
                        f"{metric}_{stat}": value
                        for metric, stats_ in metrics.items()
                        for stat, value in stats_.items()
                    },
                )
            )
            if "relative" in metrics:
                tradeoffs[(group, scenario, size)][name] = {
                    median: [metrics["relative"]["max"]]
                }
        body += table(["Size", "Samples", "Median ns/op", "Min", "Max", "CV"], stats)
        detail_path = base / "index.html"
        raw_path = Path("data") / group / name / "samples.csv"
        if (group, name) not in raw_samples:
            raw_samples[(group, name)] = read_csv(
                source / group / name / "samples.csv"
            )[1]
        raw = raw_samples[(group, name)]
        raw = [r for r in raw if r.get("scenario", "legacy") == scenario]

        def extent(key):
            values_ = sorted({int(r[key]) for r in raw})
            return (
                str(values_[0]) if len(values_) == 1 else f"{values_[0]}–{values_[-1]}"
            )

        conditions = f"{axis}; sizes: {', '.join(map(str, sorted(values)))}; iterations/batch: {extent('iterations')}; samples: {len(raw)}; processes: {len({r.get('process', '1') for r in raw})}; seed: {extent('seed')}"
        data_links = f'<a href="{raw_path}" download>timing CSV</a>'
        if (source / group / name / "accuracy.csv").exists():
            data_links += f' · <a href="data/{group}/{name}/accuracy.csv" download>accuracy CSV</a>'
        # All generated links use site-root-relative paths until this final rewrite.
        body = re.sub(
            r'href="([^"]+)"',
            lambda m: 'href="' + posixpath.relpath(m[1], str(base)) + '"',
            body,
        )
        (output / detail_path).write_text(
            page(
                f"{name} / {scenario}",
                f'<p><a href="{posixpath.relpath("index.html", str(base))}">All algorithms</a></p>'
                f"<h1>{escape(name)} / {escape(scenario)}</h1><p>{escape(conditions)}</p>"
                + body,
            ),
            encoding="utf-8",
        )
        catalog.append(
            (
                group,
                name,
                scenario,
                conditions,
                str(detail_path),
                str(base / "timing.svg"),
                data_links,
            )
        )

    overview = []
    for (group, axis, scenario), lines in sorted(groups.items()):
        overview.append(
            image(
                Path(group) / scenario / "comparison.svg",
                f"{group} / {scenario}: timing",
                axis,
                lines,
            )
        )
    for (group, scenario, size), lines in sorted(tradeoffs.items()):
        overview.append(
            image(
                Path(group) / scenario / f"{size}-tradeoff.svg",
                f"{group} / {scenario}: time vs accuracy, size={size}",
                "median ns / operation (lower is better)",
                lines,
                ylabel="maximum relative error (lower is better)",
                subtitle="one point per algorithm; no equivalence of accuracy assumed",
                scatter=True,
            )
        )
    for (group, scenario, metric), lines in sorted(maxima.items()):
        overview.append(
            image(
                Path(group) / scenario / f"max-{metric}.svg",
                f"{group} / {scenario}: maximum {metric} error",
                "input range size",
                lines,
                ylabel=f"maximum {metric} error",
                subtitle="observed maximum over each workload's sampled inputs",
            )
        )
    write_csv(output / "summary.csv", list(timing_rows[0]), timing_rows)
    downloads.append("summary.csv")
    if accuracy_rows:
        fields = list(dict.fromkeys(k for row in accuracy_rows for k in row))
        write_csv(output / "accuracy-summary.csv", fields, accuracy_rows)
        downloads.append("accuracy-summary.csv")
    # Quality diagnostics from the raw batch durations, not normalized time.
    for path in source.glob("*/*/samples.csv"):
        _, rows = read_csv(path)
        if min(int(r["elapsed_ns"]) for r in rows) < 1000000:
            warnings.append(
                f"{path.parent.name}: batches under 1 ms; increase --min-time-ms"
            )
    warnings = list(dict.fromkeys(warnings))
    report = {
        "metadata": metadata,
        "timing_workloads": len(timing_rows),
        "accuracy_workloads": len(accuracy_rows),
        "warnings": warnings,
        "accuracy": accuracy_rows,
        "timing": timing_rows,
    }
    (output / "report.json").write_text(
        json.dumps(report, indent=2, ensure_ascii=False) + "\n", encoding="utf-8"
    )
    downloads.append("report.json")
    accuracy_fields = list(dict.fromkeys(k for row in accuracy_rows for k in row))
    accuracy_table = (
        table(
            accuracy_fields,
            [[row.get(k, "n/a") for k in accuracy_fields] for row in accuracy_rows],
        )
        if accuracy_rows
        else "<p>No accuracy references registered for these algorithms.</p>"
    )
    html = "<h1>Benchmark experiment report</h1>"
    html += f"<p>{len({name for _, name, _, _ in series})} algorithms / functions · {len({group for group, _, _, _ in series})} groups · {len(series)} algorithm/scenario series · {len(timing_rows)} timing workloads · {len(accuracy_rows)} accuracy workloads</p>"
    parameters = metadata.get("parameters", {})
    html += (
        "<p>"
        + escape(
            " · ".join(
                f"{key}: {parameters[key]}"
                for key in ("samples", "processes", "warmups", "min_time_ms", "seed")
                if key in parameters
            )
        )
        + "</p>"
    )
    html += '<h2>Algorithms</h2><div class="scroll"><table><thead><tr><th>Algorithm / group</th><th>Scenario</th><th>Run conditions</th><th>Graphs &amp; data</th></tr></thead><tbody>'
    for group, name, scenario, conditions, detail, timing, data_links in catalog:
        html += (
            f"<tr><td>{escape(name)}<br>{escape(group)}</td><td>{escape(scenario)}</td>"
            f'<td>{escape(conditions)}</td><td><a href="{detail}">All graphs &amp; statistics</a> · '
            f'<a href="{timing}">Timing SVG</a> · {data_links}</td></tr>'
        )
    html += '</tbody></table></div><p><a href="experiment.html">Experiment, environment &amp; method</a> · <a href="comparisons.html">Group comparison graphs</a> · <a href="accuracy.html">Accuracy summary</a> · <a href="site.zip" download>Complete site ZIP</a></p>'
    coverage = metadata.get("coverage", {})
    if coverage:
        coverage_body = '<p><a href="index.html">Measured algorithms</a></p><h1>Source &amp; CLI coverage</h1><p>This page maps repository algorithms to benchmark names. A mapping does not imply the algorithm was selected in this particular run. Historical unsafe/interactive programs are not executed.</p>'
        coverage_body += "<h2>CLI operations</h2>" + table(
            ["Option", "Benchmark functions"],
            [
                [
                    "--" + option,
                    ", ".join(functions)
                    or "Control/help option, no algorithm workload",
                ]
                for option, functions in coverage.get("cli", {}).items()
            ],
        )
        coverage_body += "<h2>Example sources</h2>" + table(
            ["Source", "Benchmark functions / exclusion"],
            [
                [name, entry.get("excluded", ", ".join(entry.get("benchmarks", [])))]
                for name, entry in coverage.get("examples", {}).items()
            ],
        )
        (output / "coverage.html").write_text(
            page("Algorithm coverage", coverage_body), encoding="utf-8"
        )
        html += '<p><a href="coverage.html">All CLI operations and example sources: coverage / exclusions</a></p>'
    experiment = (
        '<p><a href="index.html">All algorithms</a></p><h1>Experiment report</h1>'
    )
    environment = metadata.get("environment", {})
    experiment += (
        '<section id="environment"><h2>Experiment &amp; environment</h2>'
        + table(
            ["Property", "Value"],
            [
                ["Run", metadata.get("run_id", "unknown")],
                ["Created (UTC)", metadata.get("created_utc", "unknown")],
                ["CPU", environment.get("cpu_model", "unknown")],
                ["Logical CPUs", environment.get("logical_cpus", "unknown")],
                ["Physical memory", environment.get("memory_total", "unknown")],
                ["Compiler", environment.get("compiler_version", "unknown")],
                ["Flags", metadata.get("build", {}).get("BENCH_CFLAGS", "unknown")],
                ["OS", metadata.get("platform", "unknown")],
            ],
        )
    )
    experiment += (
        "<details><summary>Full experiment metadata &amp; parameters</summary><pre>"
        + escape(json.dumps(metadata, indent=2, ensure_ascii=False))
        + "</pre></details></section>"
    )
    experiment += """<section id="method"><h2>Method &amp; interpretation</h2><p>Wall time: CLOCK_MONOTONIC. Calibration and warmup excluded. Independent worker processes are run sequentially in seeded shuffled order. Algorithm initialization, input indexing, checksums, sorting copies and decode allocation may contribute to timing. No empty-loop subtraction. Samples from different processes are pooled; whiskers are observed min–max, not confidence intervals.</p>
<p>Accuracy is measured separately, on the same deterministic input set, against registered double-precision references (not arbitrary-precision mathematical truth). Absolute error = |actual − reference|; relative error = absolute / |reference|, undefined for a zero reference. Maximum means the maximum over sampled inputs, not a global bound. ECDF includes every observed point, including duplicates and zeros. CV = sample standard deviation / mean. Timing/accuracy plots match group, scenario and size; lower-left is better. Not all algorithms within a group are interchangeable.</p>
<p>Scenarios: random = seeded uniform data; sorted/reverse = ascending/descending input; repeated = one value/long character run; boundary = alternating range endpoints/characters; default = stateful PRNG, size fixed to 1. For power, raw input maps to base 1 + input/size, exponent 7. Date inputs are bounded valid dates. Byte scenarios use lowercase text valid for RLE. Sorting restores input each operation. Accuracy references exist only where explicitly registered; hashes and PRNGs have no numerical-error metric.</p></section>"""
    experiment += (
        "<section><h2>Quality warnings</h2>"
        + (
            "<ul>" + "".join(f"<li>{escape(w)}</li>" for w in warnings) + "</ul>"
            if warnings
            else "<p>No automatic warnings. This is not a guarantee of measurement quality.</p>"
        )
        + "</section>"
    )
    (output / "experiment.html").write_text(
        page("Experiment report", experiment), encoding="utf-8"
    )
    (output / "comparisons.html").write_text(
        page(
            "Group comparisons",
            '<p><a href="index.html">All algorithms</a></p><h1>Group comparison graphs</h1>'
            + "".join(overview),
        ),
        encoding="utf-8",
    )
    (output / "accuracy.html").write_text(
        page(
            "Accuracy summary",
            '<p><a href="index.html">All algorithms</a></p><h1>Accuracy summary</h1>'
            + accuracy_table,
        ),
        encoding="utf-8",
    )
    html += (
        "<h2>Downloads</h2><ul>"
        + "".join(
            f'<li><a download href="{escape(p)}">{escape(p)}</a></li>'
            for p in downloads
            if not p.startswith("data/")
        )
        + "</ul><p>Extract site.zip and open index.html locally. No server or network required. The archive excludes itself.</p>"
    )
    (output / "index.html").write_text(
        page("Benchmark experiments", html), encoding="utf-8"
    )
    with zipfile.ZipFile(
        output / "site.zip", "w", compression=zipfile.ZIP_DEFLATED
    ) as archive:
        for path in sorted(output.rglob("*")):
            if path.is_file() and path.name != "site.zip":
                archive.write(path, path.relative_to(output))
    return len(series)
