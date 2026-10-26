"""A short, deterministic tour of the application. No third-party dependencies."""
import pathlib
import subprocess
import sys

binary = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else "bin/theartoffun").resolve()
cases = [
    (["--binary-power", "2", "--exponent", "10"], "1024.00"),
    (["--isqrt", "4294967295"], "65535"),
    (["--morton-encode", "10", "13"], "230"),
    (["--morton-decode", "230"], "10 13"),
    (["--levenshtein", "kitten", "sitting"], "3"),
    (["--rle-encode", "aaabbc"], "3a2bc"),
    (["--rle-decode", "3a2bc"], "aaabbc"),
]
for args, expected in cases:
    print(f"\n$ theartoffun {' '.join(args)}", flush=True)
    result = subprocess.run([str(binary), *args], capture_output=True, text=True, timeout=10)
    if result.returncode or expected not in result.stdout:
        raise SystemExit(result.stderr or f"Unexpected result: {result.stdout}")
    print(result.stdout, end="")
