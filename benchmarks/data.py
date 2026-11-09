"""Shared, dependency-free experiment schema and validation."""

import csv
import math
from pathlib import Path

TIMING = "schema_version group algorithm axis size sample iterations seed elapsed_ns ns_per_op checksum".split()
ACCURACY = "schema_version group algorithm scenario size point input actual reference absolute_error relative_error".split()


def read_csv(path):
    with Path(path).open(encoding="utf-8", newline="") as stream:
        reader = csv.DictReader(stream)
        rows = list(reader)
        if not rows or any(None in row or None in row.values() for row in rows):
            raise ValueError(f"{path}: empty or malformed CSV")
        return reader.fieldnames, rows


def write_csv(path, fields, rows):
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=fields)
        writer.writeheader()
        writer.writerows(rows)


def finite(text, minimum=None):
    value = float(text)
    if not math.isfinite(value) or (minimum is not None and value < minimum):
        raise ValueError("non-finite or out-of-range value")
    return value
