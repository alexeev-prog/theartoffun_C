"""Run only the explicitly supported finite examples, with a per-process timeout."""
import pathlib
import subprocess
import sys

directory = pathlib.Path(sys.argv[1]).resolve()
for name in sys.argv[2:]:
    result = subprocess.run([str(directory / name)], capture_output=True, timeout=15)
    if result.returncode:
        sys.stderr.buffer.write(result.stderr)
        raise SystemExit(f"{name}: failed with status {result.returncode}")
    if not result.stdout:
        raise SystemExit(f"{name}: produced no output")
    print(f"PASS {name}")

if "mortoncode" in sys.argv[2:]:
    result = subprocess.run(
        [str(directory / "mortoncode"), "65535", "65535"],
        capture_output=True, text=True, timeout=15,
    )
    if result.returncode or "4294967295" not in result.stdout or "x=65535, y=65535" not in result.stdout:
        raise SystemExit("Morton full-width round trip failed")
    for args in [["65536", "1"], ["1"], ["-1", "2"]]:
        result = subprocess.run([str(directory / "mortoncode"), *args], capture_output=True, timeout=15)
        if not result.returncode:
            raise SystemExit(f"Morton accepted invalid input: {args}")

if "pi_leibniz" in sys.argv[2:]:
    for value in ["0", "-1", "10000001", "bad"]:
        result = subprocess.run([str(directory / "pi_leibniz"), value], capture_output=True, timeout=15)
        if not result.returncode:
            raise SystemExit(f"Pi example accepted invalid iteration count: {value}")
