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
import subprocess
import sys
import tempfile
import time
import uuid


def positive(text):
    value = int(text)
    if value < 1:
        raise argparse.ArgumentTypeError("must be positive")
    return value


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--runner", type=Path, default=Path("bin/benchmark"))
    parser.add_argument("--output", type=Path, default=Path("results/benchmarks"))
    parser.add_argument("--algorithms", nargs="+", help="exact algorithm names; default: all")
    parser.add_argument("--sizes", nargs="+", type=positive, default=[64, 256, 1024, 4096])
    parser.add_argument("--iterations", type=positive, default=10000)
    parser.add_argument("--samples", type=positive, default=7)
    parser.add_argument("--warmups", type=positive, default=2)
    parser.add_argument("--seed", type=positive, default=42)
    parser.add_argument("--timeout", type=positive, default=120, help="seconds per worker")
    parser.add_argument("--list", action="store_true")
    args = parser.parse_args()
    runner = args.runner.resolve()
    catalog = subprocess.check_output([str(runner), "--list"], text=True, timeout=10)
    if args.list:
        print(catalog, end="")
        return
    available = {name: (group, axis) for group, name, axis in
                 (line.split("\t") for line in catalog.splitlines())}
    algorithms = args.algorithms or list(available)
    if len(set(algorithms)) != len(algorithms) or set(algorithms) - available.keys():
        parser.error("duplicate or unknown algorithms; use --list")
    if (max(args.sizes) > 65536 or len(set(args.sizes)) != len(args.sizes)
            or args.samples > 1000 or args.warmups > 1000
            or args.iterations > 1000000000 or args.seed > 4294967295):
        parser.error("limits: unique sizes 1..65536, samples/warmups <=1000, iterations <=1e9, seed <=2^32-1")
    args.output.mkdir(parents=True, exist_ok=True)
    run_id = datetime.datetime.now(datetime.timezone.utc).strftime("%Y%m%dT%H%M%SZ") + "-" + uuid.uuid4().hex[:8]
    # Publish only complete runs. Failure retains no misleading partial CSV tree.
    with tempfile.TemporaryDirectory(prefix=".pending-", dir=args.output) as temporary:
        root = Path(temporary)
        started = time.time()
        for name in algorithms:
            group, _ = available[name]
            destination = root / group / name / "samples.csv"
            destination.parent.mkdir(parents=True)
            with destination.open("w", newline="", encoding="utf-8") as output:
                writer = None
                for size in sorted(args.sizes):
                    command = [str(runner), name, str(size), str(args.iterations),
                               str(args.samples), str(args.seed), str(args.warmups), "1"]
                    result = subprocess.run(command, check=True, capture_output=True,
                                            text=True, timeout=args.timeout)
                    reader = csv.DictReader(io.StringIO(result.stdout))
                    if writer is None:
                        writer = csv.DictWriter(output, fieldnames=reader.fieldnames)
                        writer.writeheader()
                    writer.writerows(reader)
            print(f"Measured {group}/{name}", file=sys.stderr)
        try:
            revision = subprocess.check_output(["git", "rev-parse", "HEAD"], text=True,
                                               stderr=subprocess.DEVNULL, timeout=10).strip()
            dirty = bool(subprocess.check_output(["git", "status", "--porcelain"], text=True, timeout=10))
        except (OSError, subprocess.SubprocessError):
            revision, dirty = None, None
        import hashlib
        metadata = {
            "schema_version": 1, "run_id": run_id,
            "created_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
            "duration_seconds": time.time() - started,
            "platform": platform.platform(), "machine": platform.machine(),
            "processor": platform.processor(), "python": platform.python_version(),
            "git_revision": revision, "git_dirty": dirty,
            "runner_sha256": hashlib.sha256(runner.read_bytes()).hexdigest(),
            "build": {key: os.environ.get(key) for key in ("BENCH_CC", "BENCH_CFLAGS", "BENCH_CPPFLAGS")},
            "parameters": {key: value for key, value in vars(args).items()
                           if key not in ("runner", "output", "list")},
            "algorithms": algorithms,
            "method": "CLOCK_MONOTONIC; warmups excluded; sequential workers; no baseline subtraction",
        }
        (root / "metadata.json").write_text(json.dumps(metadata, indent=2) + "\n", encoding="utf-8")
        destination = args.output / run_id
        root.rename(destination)
        # TemporaryDirectory tolerates the directory having been renamed.
    print(destination)


if __name__ == "__main__":
    try:
        main()
    except (OSError, ValueError, subprocess.SubprocessError) as error:
        sys.exit(f"benchmark collection failed: {error}")
