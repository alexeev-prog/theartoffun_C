#!/usr/bin/env python3
"""Compare two saved benchmark reports without requiring a benchmark runner."""

import argparse
import csv
import html
import json
from pathlib import Path


KEYS = ("group", "algorithm", "scenario", "size")


def load(path):
    report = json.loads((Path(path) / "report.json").read_text(encoding="utf-8"))
    return {tuple(str(row[key]) for key in KEYS): row for row in report["timing"]}


def compare(current, baseline, threshold):
    rows = []
    for key in sorted(set(current) & set(baseline)):
        before = float(baseline[key]["median_ns"])
        after = float(current[key]["median_ns"])
        change = (after / before - 1) * 100 if before else 0
        rows.append((*key, before, after, change, abs(change) >= threshold))
    return rows


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--current", type=Path, required=True)
    parser.add_argument("--baseline", type=Path, required=True)
    parser.add_argument("--output", type=Path)
    parser.add_argument("--threshold", type=float, default=5.0)
    args = parser.parse_args()
    if args.threshold < 0:
        parser.error("--threshold must not be negative")
    rows = compare(load(args.current), load(args.baseline), args.threshold)
    fields = [*KEYS, "baseline_median_ns", "current_median_ns", "change_percent", "significant"]
    output = args.output or args.current / "comparison.csv"
    output.parent.mkdir(parents=True, exist_ok=True)
    with output.open("w", newline="", encoding="utf-8") as stream:
        writer = csv.writer(stream)
        writer.writerow(fields)
        writer.writerows(rows)
    regressions = [row for row in rows if row[-2] >= args.threshold]
    improved = [row for row in rows if row[-2] <= -args.threshold]
    html_path = output.with_suffix(".html")
    body = ["<!doctype html><meta charset='utf-8'><title>Benchmark comparison</title>",
            "<style>body{font-family:serif;max-width:90em;margin:2em auto}table{border-collapse:collapse}td,th{padding:.4em .7em;border-bottom:1px solid #ccc}.bad{color:#b91c1c}.good{color:#047857}</style>",
            f"<h1>Benchmark comparison</h1><p>Threshold: {args.threshold:g}% · regressions: {len(regressions)} · improvements: {len(improved)}</p>",
            "<table><tr>" + "".join(f"<th>{html.escape(field)}</th>" for field in fields) + "</tr>"]
    for row in rows:
        cls = "bad" if row[-2] >= args.threshold else "good" if row[-2] <= -args.threshold else ""
        body.append("<tr class='%s'>%s</tr>" % (cls, "".join(f"<td>{html.escape(str(value))}</td>" for value in row)))
    body.append("</table>")
    html_path.write_text("\n".join(body), encoding="utf-8")
    print(output)
    print(html_path)
    raise SystemExit(1 if regressions else 0)


if __name__ == "__main__":
    main()
