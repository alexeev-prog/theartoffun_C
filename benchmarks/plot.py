#!/usr/bin/env python3
"""Validate saved benchmark CSV and render standalone SVG (no C runner needed)."""
import argparse
import csv
from collections import defaultdict
from html import escape
import math
from pathlib import Path
import re
import statistics
import sys

FIELDS = "schema_version group algorithm axis size sample iterations seed elapsed_ns ns_per_op checksum".split()
IDENTIFIER = re.compile(r"[A-Za-z][A-Za-z0-9_]*\Z")


def load(root):
    series = defaultdict(lambda: defaultdict(list))
    identities = set()
    files = sorted(root.glob("*/*/samples.csv"))
    if not files:
        raise ValueError(f"no group/algorithm/samples.csv files in {root}")
    for path in files:
        with path.open(newline="", encoding="utf-8") as stream:
            reader = csv.DictReader(stream)
            if reader.fieldnames != FIELDS:
                raise ValueError(f"{path}: unsupported CSV header")
            rows = 0
            configuration = None
            for line, row in enumerate(reader, 2):
                try:
                    group, name, axis = (row[k] for k in ("group", "algorithm", "axis"))
                    if not all(IDENTIFIER.fullmatch(v) for v in (group, name, axis)):
                        raise ValueError("invalid identifier")
                    if (group, name) != (path.parent.parent.name, path.parent.name):
                        raise ValueError("CSV identity differs from directory")
                    if row["schema_version"] != "1" or None in row or None in row.values():
                        raise ValueError("unsupported schema or malformed row")
                    size, sample, count, seed, elapsed = (int(row[k]) for k in
                        ("size", "sample", "iterations", "seed", "elapsed_ns"))
                    ns, checksum = float(row["ns_per_op"]), float(row["checksum"])
                    if min(size, sample, count, seed, elapsed) < 1 or not all(map(math.isfinite, (ns, checksum))) or ns <= 0:
                        raise ValueError("non-positive or non-finite measurement")
                    if not math.isclose(ns, elapsed / count, rel_tol=1e-7, abs_tol=1e-8):
                        raise ValueError("ns_per_op inconsistent with elapsed_ns/iterations")
                    identity = (group, name, size, sample)
                    if identity in identities:
                        raise ValueError("duplicate sample")
                    current = (axis, count, seed)
                    if configuration is not None and current != configuration:
                        raise ValueError("mixed configurations in one algorithm")
                    configuration = current
                    identities.add(identity)
                    series[(group, name, axis)][size].append(ns)
                    rows += 1
                except (ValueError, TypeError) as error:
                    raise ValueError(f"{path}:{line}: {error}") from error
            if not rows:
                raise ValueError(f"{path}: no measurements")
    return series


COLORS = ["#2563eb", "#dc2626", "#059669", "#9333ea", "#d97706", "#0891b2", "#475569"]


def svg(title, axis, lines):
    sizes = sorted({size for values in lines.values() for size in values})
    maximum = max(max(samples) for values in lines.values() for samples in values.values()) * 1.1
    width, height = 1000, 540 + len(lines) * 24
    left, top, plot_width, plot_height = 100, 70, 850, 370
    def x(size):
        return left + (sizes.index(size) / max(1, len(sizes) - 1)) * plot_width
    def y(value):
        return top + plot_height * (1 - value / maximum)
    parts = [f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" viewBox="0 0 {width} {height}" role="img">',
             f'<title>{escape(title)}</title>', '<rect width="100%" height="100%" fill="white"/>',
             '<g font-family="sans-serif" font-size="14" fill="#172033">',
             f'<text x="100" y="30" font-size="22">{escape(title)}</text>',
             '<text x="100" y="53">ns / operation — median; whiskers: min–max (not confidence intervals)</text>']
    for tick in range(6):
        value = maximum * tick / 5
        py = y(value)
        parts += [f'<path d="M100 {py:.2f} H950" stroke="#e2e8f0"/>',
                  f'<text x="90" y="{py + 5:.2f}" text-anchor="end">{value:.3g}</text>']
    for size in sizes:
        parts.append(f'<text x="{x(size):.2f}" y="466" text-anchor="middle">{size}</text>')
    parts.append(f'<text x="525" y="495" text-anchor="middle">{escape(axis)} (equally spaced tested sizes)</text>')
    for index, (name, values) in enumerate(sorted(lines.items())):
        color = COLORS[index % len(COLORS)]
        points = " ".join(f"{x(size):.2f},{y(statistics.median(samples)):.2f}" for size, samples in sorted(values.items()))
        parts.append(f'<polyline points="{points}" fill="none" stroke="{color}" stroke-width="2"/>')
        for size, samples in sorted(values.items()):
            px, lo, hi, mid = x(size), y(min(samples)), y(max(samples)), y(statistics.median(samples))
            parts += [f'<path d="M{px:.2f} {lo:.2f} V{hi:.2f} M{px-5:.2f} {lo:.2f} h10 M{px-5:.2f} {hi:.2f} h10" stroke="{color}"/>',
                      f'<circle cx="{px:.2f}" cy="{mid:.2f}" r="4" fill="{color}"><title>{escape(name)}: size={size}, n={len(samples)}, median={statistics.median(samples):.4g} ns/op</title></circle>']
        parts.append(f'<text x="100" y="{535 + index * 24}" fill="{color}">{escape(name)}</text>')
    return "\n".join(parts + ["</g></svg>\n"])


def generate(source, output):
    series = load(source)  # Validate everything before writing any output.
    if output.exists() and any(output.iterdir()):
        raise ValueError(f"output directory is not empty: {output}; choose a new directory")
    output.mkdir(parents=True, exist_ok=True)
    summaries = []
    groups = defaultdict(dict)
    links = []
    for (group, name, axis), values in sorted(series.items()):
        path = Path(group) / name / "timing.svg"
        (output / path).parent.mkdir(parents=True, exist_ok=True)
        (output / path).write_text(svg(f"{group} / {name}", axis, {name: values}), encoding="utf-8")
        links.append(str(path))
        groups[(group, axis)][name] = values
        for size, samples in sorted(values.items()):
            summaries.append([group, name, axis, size, len(samples), statistics.median(samples),
                              min(samples), max(samples), statistics.mean(samples),
                              statistics.stdev(samples) if len(samples) > 1 else 0])
    for (group, axis), lines in sorted(groups.items()):
        path = Path(group) / "comparison.svg"
        (output / path).write_text(svg(f"{group} — comparison", axis, lines), encoding="utf-8")
        links.append(str(path))
    with (output / "summary.csv").open("w", newline="", encoding="utf-8") as stream:
        writer = csv.writer(stream)
        writer.writerow("group algorithm axis size samples median_ns min_ns max_ns mean_ns stdev_ns".split())
        writer.writerows(summaries)
    (output / "index.html").write_text('<!doctype html><html lang="en"><meta charset="utf-8">'
        '<title>Benchmark plots</title><h1>Benchmark plots</h1>'
        '<p>Timing only; approximate algorithms are not accuracy-equivalent. '
        'Compare only matching workloads and build configurations.</p><ul>' +
        "".join(f'<li><a href="{escape(link, quote=True)}">{escape(link)}</a></li>' for link in links)
        + '</ul></html>\n', encoding="utf-8")
    return len(series)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", type=Path, required=True, help="one saved run, not the parent of multiple runs")
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    if args.input.resolve() == args.output.resolve() or args.input.resolve().is_relative_to(args.output.resolve()):
        parser.error("output must not equal or contain the input directory")
    count = generate(args.input, args.output)
    print(f"Rendered {count} algorithms: {args.output / 'index.html'}")


if __name__ == "__main__":
    try:
        main()
    except (OSError, ValueError, csv.Error) as error:
        sys.exit(f"plot generation failed: {error}")
