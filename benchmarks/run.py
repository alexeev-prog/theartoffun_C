#!/usr/bin/env python3
"""Collect raw samples; deliberately knows nothing about plotting."""

import argparse
import csv
import datetime
import io
import json
import os
from pathlib import Path
import platform
import random
import shlex
import subprocess
import sys
import tempfile
import time
import uuid
from data import write_csv


def environment():
    """Best effort: unsupported OS fields remain explicitly unknown."""

    def proc(path):
        try:
            return Path(path).read_text()
        except OSError:
            return ""

    cpu = next(
        (
            line.split(":", 1)[1].strip()
            for line in proc("/proc/cpuinfo").splitlines()
            if line.startswith("model name")
        ),
        platform.processor() or "unknown",
    )
    memory = next(
        (
            line.split(":", 1)[1].strip()
            for line in proc("/proc/meminfo").splitlines()
            if line.startswith("MemTotal:")
        ),
        "unknown",
    )
    compiler = os.environ.get("BENCH_CC")
    version = "unknown (build command not supplied)"
    if compiler:
        try:
            version = subprocess.check_output(
                shlex.split(compiler) + ["--version"],
                text=True,
                stderr=subprocess.STDOUT,
                timeout=10,
            ).strip()
        except (OSError, subprocess.SubprocessError):
            version = "unavailable"
    return {
        "cpu_model": cpu,
        "logical_cpus": os.cpu_count(),
        "memory_total": memory,
        "cpu_affinity": sorted(os.sched_getaffinity(0))
        if hasattr(os, "sched_getaffinity")
        else None,
        "compiler_version": version,
    }


def positive(text):
    value = int(text)
    if value < 1:
        raise argparse.ArgumentTypeError("must be positive")
    return value


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--runner", type=Path, default=Path("bin/benchmark"))
    parser.add_argument("--output", type=Path, default=Path("results/benchmarks"))
    parser.add_argument(
        "--algorithms", nargs="+", help="exact algorithm names; default: all"
    )
    parser.add_argument(
        "--sizes", nargs="+", type=positive, default=[64, 256, 1024, 4096]
    )
    parser.add_argument("--iterations", type=positive, default=10000)
    parser.add_argument("--samples", type=positive, default=7)
    parser.add_argument("--warmups", type=positive, default=2)
    parser.add_argument("--seed", type=positive, default=42)
    parser.add_argument(
        "--timeout", type=positive, default=120, help="seconds per worker"
    )
    parser.add_argument(
        "--scenarios",
        nargs="+",
        help="filter catalog scenarios; default: all supported",
    )
    parser.add_argument(
        "--processes",
        type=positive,
        default=3,
        help="independent sequential worker launches",
    )
    parser.add_argument(
        "--min-time-ms", type=positive, default=20, help="calibration target per batch"
    )
    parser.add_argument("--list", action="store_true")
    args = parser.parse_args()
    runner = args.runner.resolve()
    catalog = subprocess.check_output([str(runner), "--catalog"], text=True, timeout=10)
    if args.list:
        print(catalog, end="")
        return
    available = {
        name: (group, axis, scenarios.split(","), accuracy == "1", int(limit))
        for group, name, axis, scenarios, accuracy, limit in (
            line.split("\t") for line in catalog.splitlines()
        )
    }
    algorithms = args.algorithms or list(available)
    if len(set(algorithms)) != len(algorithms) or set(algorithms) - available.keys():
        parser.error("duplicate or unknown algorithms; use --list")
    if (
        max(args.sizes) > 65536
        or len(set(args.sizes)) != len(args.sizes)
        or args.samples > 1000
        or args.warmups > 1000
        or args.iterations > 1000000000
        or args.seed > 4294967295
        or args.processes > 100
        or args.min_time_ms > 10000
    ):
        parser.error(
            "limits: unique sizes 1..65536, samples/warmups <=1000, iterations <=1e9, seed <=2^32-1"
        )
    known_scenarios = {
        s for _, _, scenarios, _, _ in available.values() for s in scenarios
    }
    if args.scenarios and (
        set(args.scenarios) - known_scenarios
        or len(set(args.scenarios)) != len(args.scenarios)
    ):
        parser.error("unknown or duplicate scenarios")
    jobs = []
    for name in algorithms:
        _, axis, scenarios, _, limit = available[name]
        selected = [s for s in scenarios if not args.scenarios or s in args.scenarios]
        if not selected:
            parser.error(f"no compatible scenarios for {name}")
        sizes = (
            [1]
            if axis == "batch_parameter"
            else sorted({min(size, limit) for size in args.sizes})
        )
        jobs.extend(
            (name, size, scenario, process)
            for size in sizes
            for scenario in selected
            for process in range(1, args.processes + 1)
        )
    random.Random(args.seed).shuffle(jobs)
    args.output.mkdir(parents=True, exist_ok=True)
    run_id = (
        datetime.datetime.now(datetime.timezone.utc).strftime("%Y%m%dT%H%M%SZ")
        + "-"
        + uuid.uuid4().hex[:8]
    )
    # Publish only complete runs. Failure retains no misleading partial CSV tree.
    with tempfile.TemporaryDirectory(prefix=".pending-", dir=args.output) as temporary:
        root = Path(temporary)
        started = time.time()
        collected, errors = {}, {}
        for order, (name, size, scenario, process) in enumerate(jobs, 1):
            group, _, _, accuracy, _ = available[name]
            command = [
                str(runner),
                name,
                str(size),
                str(args.iterations),
                str(args.samples),
                str(args.seed),
                str(args.warmups),
                "2",
                scenario,
                str(args.min_time_ms),
            ]

            def invoke(mode):
                result = subprocess.run(
                    command + [mode],
                    check=True,
                    capture_output=True,
                    text=True,
                    timeout=args.timeout,
                )
                rows = list(csv.DictReader(io.StringIO(result.stdout)))
                if not rows:
                    raise ValueError("worker returned no measurements")
                return rows

            rows = invoke("timing")
            for row in rows:
                row.update(process=str(process), order=str(order))
            collected.setdefault(name, []).extend(rows)
            if process == 1 and accuracy:
                errors.setdefault(name, []).extend(invoke("accuracy"))
            print(
                f"[{order}/{len(jobs)}] {group}/{name} {scenario} size={size} process={process}",
                file=sys.stderr,
            )
        for name, rows in collected.items():
            directory = root / available[name][0] / name
            write_csv(directory / "samples.csv", list(rows[0]), rows)
            if name in errors:
                write_csv(
                    directory / "accuracy.csv", list(errors[name][0]), errors[name]
                )
        try:
            revision = subprocess.check_output(
                ["git", "rev-parse", "HEAD"],
                text=True,
                stderr=subprocess.DEVNULL,
                timeout=10,
            ).strip()
            dirty = bool(
                subprocess.check_output(
                    ["git", "status", "--porcelain"], text=True, timeout=10
                )
            )
        except (OSError, subprocess.SubprocessError):
            revision, dirty = None, None
        import hashlib

        metadata = {
            "schema_version": 2,
            "run_id": run_id,
            "created_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
            "duration_seconds": time.time() - started,
            "platform": platform.platform(),
            "machine": platform.machine(),
            "processor": platform.processor(),
            "python": platform.python_version(),
            "environment": environment(),
            "git_revision": revision,
            "git_dirty": dirty,
            "runner_sha256": hashlib.sha256(runner.read_bytes()).hexdigest(),
            "build": {
                key: os.environ.get(key)
                for key in ("BENCH_CC", "BENCH_CFLAGS", "BENCH_CPPFLAGS")
            },
            "parameters": {
                key: value
                for key, value in vars(args).items()
                if key not in ("runner", "output", "list")
            },
            "algorithms": algorithms,
            "catalog": {
                name: dict(
                    zip(
                        ("group", "axis", "scenarios", "accuracy", "max_size"),
                        available[name],
                    )
                )
                for name in algorithms
            },
            "method": "CLOCK_MONOTONIC; adaptive batch duration; excluded calibration/warmups; independent sequential processes; seeded shuffled order; no baseline subtraction; accuracy measured separately",
            "coverage": {
                name: json.loads(Path(__file__).with_name(name + ".json").read_text())
                for name in ("coverage", "examples", "cli")
            },
        }
        (root / "metadata.json").write_text(
            json.dumps(metadata, indent=2) + "\n", encoding="utf-8"
        )
        destination = args.output / run_id
        root.rename(destination)
        # TemporaryDirectory tolerates the directory having been renamed.
    print(destination)


if __name__ == "__main__":
    try:
        main()
    except (OSError, ValueError, subprocess.SubprocessError) as error:
        sys.exit(f"benchmark collection failed: {error}")
