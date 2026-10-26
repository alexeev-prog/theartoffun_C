# Guided demonstrations

Run `make demo` from the repository root for a deterministic tour of exact
integer math, Morton coordinates, edit distance, and compression. Every step
checks its output and exits on failure; it never runs benchmarks or animations.

You can also run `python3 demos/tour.py bin/theartoffun` or pass a debug/sanitizer
binary explicitly. This directory contains multi-command application scenarios;
`examples/` contains individual C programs.
