# Development guide

## Layout and scope

`include/` contains shared API headers and the header-only command-line parser.
`src/` contains reusable algorithm implementations, `main.c` for CLI dispatch,
and `benchmarks.c` for timing experiments. `tests/` is independent of the CLI
entry point and links the same algorithm objects as the application.

Standalone programs live in `examples/`. A curated list of 19 finite programs
is available through `mk/examples.mk`. Historical unsafe and animated experiments
remain excluded. The example catalog records the distinction. Run from the root:

```sh
make run-example EXAMPLE=kmp
make test-examples
make demo
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
`mk/examples.mk` owns the explicit example allowlist and demo targets. All
supported examples link the shared algorithm objects, just like C tests; no
per-example dependency list is needed.
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
- `test_parser.c`: parser grammar, repeated options, missing arguments, option
  table validation, and strict numeric conversion boundaries.
- `make test-examples`: each supported example runs in a subprocess with a timeout.
- `make demo`: deterministic, output-checked scenarios from `demos/tour.py`.
- `make sanitize`: the same tests with AddressSanitizer and UndefinedBehaviorSanitizer.

Checks remain active with `NDEBUG`. Tests do not measure performance and do not
run the full benchmark suite. Add `tests/test_*.c` files for new algorithm tests;
Make discovers them automatically. CI runs normal and sanitizer tests with both
GCC and Clang. CI also builds and smoke-tests the curated examples and runs the
tour; unsafe historical experiments are deliberately excluded.

## Adding an algorithm or trick

Keep the algorithm independent of input/output. Reuse an existing module when
the topic fits; otherwise add `include/<name>.h` and `src/<name>.c`. Make discovers
`src/*.c` automatically. Do not add another registry or copy the implementation
into tests, examples, or benchmarks. A CLI option is optional: an example is the
simplest way to run an experiment without extending `src/main.c`.

### 1. Define the contract and implement

Document input ranges, zero/overflow behavior, ownership, approximation error,
and platform assumptions in the public header. Prefer fixed-width unsigned
integers for bit tricks. Avoid signed overflow, out-of-range shifts, and pointer
type-punning; use `memcpy` for bit representations. State preconditions explicitly
rather than silently relying on undefined behavior.

For example, a new `include/bit_count.h` could contain:

```c
#ifndef BIT_COUNT_H
#define BIT_COUNT_H
#include <stdint.h>

/** Count set bits in any uint32_t value, including zero. */
unsigned bit_count(uint32_t value);
#endif
```

With `src/bit_count.c`:

```c
#include "bit_count.h"

unsigned bit_count(uint32_t value) {
    unsigned count = 0;
    while (value) {
        value &= value - 1;
        ++count;
    }
    return count;
}
```

These snippets are a recipe, not an additional installed API. Use a descriptive
name for your actual algorithm, preserve attribution, and explain why the trick
works. Keep a straightforward reference implementation in tests, not production.

### 2. Add a deterministic test

Create `tests/test_bit_count.c`; all `tests/test_*.c` files are discovered and
linked with the shared implementation automatically. Use `CHECK` from `check.h`,
not `assert` (which disappears with `NDEBUG`). Each test file has its own `main`:

```c
#include "bit_count.h"
#include "check.h"

static unsigned reference(uint32_t value) {
    unsigned count = 0;
    for (unsigned bit = 0; bit < 32; ++bit) {
        count += (value >> bit) & 1u;
    }
    return count;
}

int main(void) {
    CHECK(bit_count(0) == 0);
    CHECK(bit_count(UINT32_MAX) == 32);
    for (unsigned bit = 0; bit < 32; ++bit) {
        CHECK(bit_count(UINT32_C(1) << bit) == 1);
    }
    uint32_t state = 1; /* Fixed seed: failures are reproducible. */
    for (unsigned i = 0; i < 10000; ++i) {
        state = state * UINT32_C(1664525) + UINT32_C(1013904223);
        CHECK(bit_count(state) == reference(state));
        CHECK(bit_count(state) + bit_count(~state) == 32);
    }
    return EXIT_SUCCESS;
}
```

Cover known answers and boundaries first, then compare against an independent
reference and check properties (round trips, invariants, monotonicity where
applicable). Test invalid inputs only when the API defines their behavior. For
floating point, specify absolute/relative tolerances and test the documented
domain, including non-finite values if supported. Do not use timings as assertions.

```sh
make list-tests
make run-test TEST=test_bit_count
make DEBUG=1 run-test TEST=test_bit_count
make SANITIZE=1 run-test TEST=test_bit_count
make test-unit                   # All C tests, without building the CLI
```

### 3. Make it runnable (optional)

Add `examples/bit_count.c` with a finite, non-interactive `main` that calls the
shared API and prints the result. Add `bit_count` to `EXAMPLE_NAMES` in
`mk/examples.mk` and describe it in `examples/README.md`. This explicit allowlist
keeps unsafe historical programs out of normal builds; do not glob all examples.
No other build rule is needed, even when the example uses shared algorithms.

```sh
make list-examples
make run-example EXAMPLE=bit_count
make test-examples
```

`tests/test_examples.py` already checks exit status, nonempty output, and a
15-second timeout for every listed example. Add algorithm-specific output and
invalid-argument checks there when needed; smoke tests alone do not prove
correctness. If the example accepts arguments, validate them before narrowing
types; reuse `cli_values.h` rather than `atoi` or unchecked conversions.

If a main-application CLI option is useful, add its option/dispatch in
`src/main.c`, validate argument count and numeric range, and extend
`tests/test_cli.py` with success, boundary, missing, and invalid-input cases.
Update `docs/usage.md`; add a checked scenario to `demos/tour.py` only if useful.
Keep parsing and printing outside the algorithm.

### 4. Verify before submitting

```sh
make -j2 check                   # C + CLI tests, supported examples, CLI tour
make sanitize                   # C + CLI tests under ASan/UBSan
make CC=clang BUILD_DIR=build/clang BIN_DIR=bin/clang check
make docs                       # If Doxygen is installed
```

`make SANITIZE=1 check` also checks examples and the tour with instrumentation.
Review compiler warnings, public contracts, reference/property tests, and docs.
If claiming a speedup, benchmark separately with the same inputs/compiler/flags,
consume results to discourage dead-code elimination, and report tradeoffs and
error bounds. A shorter expression is not necessarily faster or more portable.

## API limitations

- Approximate math routines are experiments, not replacements for libm.
  Bit-based power approximations assume IEEE-754; `fast_pow` additionally assumes
  little-endian doubles. Use positive finite inputs in the demonstrated range.
  Approximate powers return NaN for invalid inputs or unsupported bit ranges.
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
  arithmetic overflow, or allocation failure return NULL. Use `rle_decode_limit`
  for untrusted input: it rejects output beyond a chosen limit before allocation.
  The CLI uses a 1 MiB limit; the legacy unbounded API remains available.
- `fletcher32_string` uses little-endian 16-bit words, zero-padding an odd final
  byte. `fletcher32` takes a count of words, not bytes.
- `levenshtein` returns -1 on allocation failure or unsupported input length.
  Unless otherwise documented, pointers must be valid and non-NULL.

Benchmark timings depend on the compiler and machine. The suite is educational,
not a statistically controlled performance comparison; even an observable result
does not prevent all constant folding and optimization.

## Documentation and GitHub Pages

`Doxyfile` generates HTML from the public headers, source, README, and this guide.
It also includes the CLI cookbook, example catalog/sources, and demo guide.
`docs/theme.css` layers a local responsive style over Doxygen, including dark
colors, readable code blocks and keyboard focus outlines. Navigation and search
use Doxygen's built-in assets; no remote theme is downloaded during CI.
The documentation workflow builds on pull requests and publishes only on pushes
to `main` or manual runs on `main`. To enable deployment in GitHub:

1. Open **Settings → Pages → Build and deployment**.
2. Choose **GitHub Actions** as the source.
3. Push the changes to `main` or run the **Documentation** workflow manually.

The `github-pages` environment may require approval if repository protection
rules demand it. The deployment job reports the actual published URL.
