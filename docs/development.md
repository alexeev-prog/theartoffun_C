# Development guide

## Layout and scope

`include/` contains shared API headers and the header-only command-line parser.
`src/` contains reusable algorithm implementations, `main.c` for CLI dispatch,
and `benchmarks.c` for timing experiments. `tests/` is independent of the CLI
entry point and links the same algorithm objects as the application.

Standalone programs live in `examples/`. They are historical educational
experiments, not part of the default build or the tested API. Some intentionally
use obfuscated syntax, platform-specific features, unsafe techniques, or endless
animation loops. Inspect them before running. Their existing IOCCC makefiles
are retained; run them from that directory, for example:

```sh
make -C examples -f bonsai.Makefile
cc -std=c11 examples/kmp.c -o bin/kmp
```

Articles `docs/article*.md` are retained as background material, not as a
specification or a guarantee of correctness.

## Build configuration

GNU Make, GCC or Clang with C11 and `__uint128_t` support, a POSIX environment,
and libm are required. Linux is the CI target; Windows is not currently tested.
Python 3 is needed for CLI tests, and Doxygen for HTML documentation.
No third-party test framework is required.

The root Makefile delegates configuration, build, tests, and documentation to
`mk/config.mk`, `mk/build.mk`, `mk/test.mk`, and `mk/docs.mk` respectively.
Libraries belong in `LDLIBS`, linker options in `LDFLAGS`, preprocessor options
in `CPPFLAGS`. Header dependencies are generated automatically (`-MMD -MP`).

```sh
make -j2
make run ARGS="--help"
make DEBUG=1 test                 # bin/debug and build/debug
make sanitize                    # bin/sanitize and build/sanitize
make CC=clang BUILD_DIR=build/clang BIN_DIR=bin/clang test
make docs                        # build/docs/html/index.html
```

Release, debug, and sanitizer builds are isolated. Use a separate `BUILD_DIR`
and `BIN_DIR` when changing compiler or custom flags, or run `make clean` first:
Make does not detect arbitrary changes to compiler flags. `make clean` removes
generated `build/` and `bin/` trees, including local documentation.

## Tests

- `test_algorithms.c`: deterministic boundary tests, known hash/Fibonacci
  vectors, and 10,000 property checks for Morton/Gray round trips, square root,
  division, random ranges, and bit operations.
- `test_strings.c`: edit-distance examples, compression round trips, long RLE
  series, malformed and overflowing RLE counts.
- `test_cli.py`: subprocess integration checks for help, parser errors,
  exponent validation, and successful calculations.
- `make sanitize`: the same tests with AddressSanitizer and UndefinedBehaviorSanitizer.

Checks remain active with `NDEBUG`. Tests do not measure performance and do not
run the full benchmark suite. Add `tests/test_*.c` files for new algorithm tests;
Make discovers them automatically. CI runs normal and sanitizer tests with both
GCC and Clang. Standalone examples are deliberately excluded.

## API limitations

- Approximate math routines are experiments, not replacements for libm.
  Bit-based power approximations assume IEEE-754; `fast_pow` additionally assumes
  little-endian doubles. Use positive finite inputs in the demonstrated range.
- `binary_pow` takes an unsigned integer exponent; fractional exponents are not
  supported. `fibonacci` fits in `uint64_t` only up to index 93.
- Fibonacci conversion APIs document the range 0–100000 miles. They approximate
  distances rather than performing exact unit conversion.
- `fast_mod` requires a nonzero power-of-two modulus. `next_power_of_two` returns
  zero for zero or overflow above 2^31.
- PRNGs are not cryptographically secure. Xorshift needs a nonzero seed; APIs
  using global state are not thread-safe. `sha1_prng` needs 16 state words.
- `is_palindrome_bit` checks whether lowercase ASCII letters can be rearranged
  into a palindrome, not whether the string already is a palindrome. Other
  characters return false.
- RLE's legacy format cannot represent literal digits. `rle_encode` requires
  digit-free input and a buffer of at least `strlen(input) * 3 + 1` bytes.
  `rle_decode` allocates the exact expanded size; free its result. Invalid counts,
  arithmetic overflow, or allocation failure return NULL. Do not decode untrusted
  inputs without an application-level size limit: large counts can exhaust memory.
- `fletcher32_string` uses little-endian 16-bit words, zero-padding an odd final
  byte. `fletcher32` takes a count of words, not bytes.
- `levenshtein` returns -1 on allocation failure or unsupported input length.
  Unless otherwise documented, pointers must be valid and non-NULL.

Benchmark timings depend on the compiler and machine. The suite is educational,
not a statistically controlled performance comparison; even an observable result
does not prevent all constant folding and optimization.

## Documentation and GitHub Pages

`Doxyfile` generates HTML from the public headers, source, README, and this guide.
The documentation workflow builds on pull requests and publishes only on pushes
to `main` or manual runs on `main`. To enable deployment in GitHub:

1. Open **Settings → Pages → Build and deployment**.
2. Choose **GitHub Actions** as the source.
3. Push the changes to `main` or run the **Documentation** workflow manually.

The `github-pages` environment may require approval if repository protection
rules demand it. The deployment job reports the actual published URL.
