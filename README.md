# The Art of Fun C

[![Build and test](https://github.com/alexeev-prog/theartoffun_c/actions/workflows/ci.yml/badge.svg)](https://github.com/alexeev-prog/theartoffun_c/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

**[CLI cookbook](docs/usage.md) · [Example catalog](examples/README.md) ·
[Development](docs/development.md)**

An educational collection of C algorithms, bit tricks, approximate mathematics,
pseudorandom generators, and timing experiments. The main application exposes
selected algorithms through a command-line interface. This is an exploration
toolkit, not a production cryptography or numerical library.

## Quick start

Requirements: GNU Make, GCC or Clang with C11 and 128-bit integer support,
a POSIX environment, and libm. Tests also require Python 3.

```sh
make -j2
./bin/theartoffun --help
make run ARGS="--binary-power 2 --exponent 10"
make test
make demo
```

Optional Nix environment: run `nix-shell`, then use the same commands.

## Commands

```sh
make                          # Build bin/theartoffun
make run ARGS="--help"        # Build and run
make test                     # Unit, property, and CLI integration tests
make sanitize                 # Tests with ASan and UBSan
make DEBUG=1 test             # Debug build and tests
make examples                 # Build 19 finite C programs
make run-example EXAMPLE=mortoncode ARGS="10 13"
make test-examples            # Timeout-protected smoke tests
make demo                     # Checked, deterministic CLI tour
make docs                     # Requires Doxygen; build/docs/html/index.html
make clean                    # Remove build/ and bin/
make help                     # Show build targets
```

Compiler selection: `make CC=clang BUILD_DIR=build/clang BIN_DIR=bin/clang test`.
Separate build paths avoid mixing compiler outputs.

## Examples

```sh
./bin/theartoffun --fib 5
./bin/theartoffun --binary-power 2 --exponent 10
./bin/theartoffun --isqrt 4294967295
./bin/theartoffun --morton-encode 10 13
./bin/theartoffun --levenshtein kitten sitting
./bin/theartoffun --q-rsqrt-quake 25
./bin/theartoffun --div3 123
./bin/theartoffun --jenkins-hash "test_data"
./bin/theartoffun --rle-encode aaabbc
./bin/theartoffun --rle-decode 3a2bc
./bin/theartoffun --benchmark
```

Use `--help` for the complete option list. Binary exponentiation requires a
nonnegative integer exponent. RLE input text must not contain literal digits.
Choose one operation per invocation. Duplicate options, invalid numeric ranges,
and extra arguments are rejected. RLE decoding is capped at 1 MiB in the CLI.
Benchmark results are machine-dependent, not correctness tests.

## Repository structure

```text
include/          Shared API headers and header-only CLI parser
src/              Algorithm implementations, main.c, benchmarks.c
tests/            C unit/property tests and Python CLI tests
examples/         Runnable C examples, catalog, and preserved historical sources
demos/            Checked multi-command application tour
mk/               Configuration, build, test, and documentation Make fragments
docs/             Development guide and original educational articles
.github/workflows/  Compiler/sanitizer CI and GitHub Pages documentation
Doxyfile          HTML API documentation configuration
```

Implemented topics include Fibonacci distance approximations, powers, integer
square roots, Gray and Morton codes, PRNGs, hashes/checksums, counting sort,
Levenshtein distance, and run-length encoding. Tests cover selected algorithms
and regression cases; they do not validate every historical experiment.

`make examples` builds a curated set of finite, non-interactive programs;
`make test-examples` runs them with timeouts. Historical unsafe experiments,
terminal animations, and IOCCC submissions are explicitly excluded. See the
[example catalog](examples/README.md) for the supported list and reasons.

## Documentation

- [Development guide](docs/development.md): build configuration, tests, API
  contracts and limitations, and GitHub Pages setup.
- [Original articles](https://github.com/alexeev-prog/theartoffun_c/blob/main/docs/article.md): background on mathematics and C tricks;
  further parts are in `docs/article_part*.md`.
- Generate API documentation with `make docs`. The documentation workflow
  publishes it to GitHub Pages after enabling **Settings → Pages → GitHub Actions**.
- HTML includes a local responsive stylesheet, system-aware dark colors, a
  navigation tree, source browsing, and search. No CDN or external theme download
  is required. Open `build/docs/html/index.html` after generation.

## References

- [PCG random generators](https://www.pcg-random.org/)
- [PRNG shootout](https://prng.di.unimi.it/)
- [Quake III source](https://github.com/id-Software/Quake-III-Arena)
- [Bob Jenkins' hashes](https://burtleburtle.net/bob/hash/doobs.html)

## License

[MIT](LICENSE). Original standalone submissions retain their source attribution.
