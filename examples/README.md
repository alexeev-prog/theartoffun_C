# Runnable C examples

Build the supported, finite examples from the repository root:

```sh
make examples
make list-examples
make run-example EXAMPLE=mortoncode ARGS="65535 65535"
make run-example EXAMPLE=pi_leibniz ARGS="1000000"
make test-examples
```

Executables are written to `bin/examples/` (or `$(BIN_DIR)/examples/` for a
custom/debug build). Every supported example runs without input and terminates.
Smoke tests impose a 15-second timeout and require successful exit and output;
they are not proofs of algorithmic correctness. Morton and XXTEA additionally
check their round trips. Morton, Fletcher32, and pi link the same implementations
as the application instead of maintaining duplicate copies.

## Choose a topic

| Topic | Example names | What to expect |
| :--- | :--- | :--- |
| Search | `kmp`, `boyermoore` | Pattern matching on built-in strings |
| Geometry | `bresenham_line`, `bresenham_circle` | ASCII rasterization |
| Coordinates | `mortoncode` | Optional X Y, each 0..65535; round-trip check |
| Numbers | `pi_leibniz`, `karatsuba`, `q_rsqrt`, `fuzzy_float_compare` | Approximation, multiplication, and comparison |
| Bits | `bitboard_example`, `float_introspection`, `xor_fold` | Bit layouts and operations |
| Data | `floyd_warshall`, `FTaH`, `fletcher32_checksum` | Paths, cycle detection, checksums |
| Generators | `gjf_example`, `tyche_example` | A few non-cryptographic random samples |
| Other | `secret`, `xxtea_example` | String trick and educational cipher round trip |

`pi_leibniz` defaults to 100000 iterations and accepts 1..10000000. It no longer
requires OpenMP or runs a billion iterations by default. `q_rsqrt` uses a fixed
seed and positive inputs; its timing results still vary across machines.
Float introspection assumes the target's IEEE-754 bit layout. XXTEA is an
educational example, not a recommended encryption interface.

## Historical experiments (manual only)

These sources are preserved but deliberately excluded from `make examples` and
CI execution:

| Source | Reason |
| :--- | :--- |
| `stackoverflow.c` | Intentional stack exhaustion; do not run in normal workflows |
| `dmabypass.c` | Unsafe memory-access experiment |
| `fun.c`, `superfun.c` | Historical/obfuscated token syntax, not portable C11 |
| `donut.c` | Continuous terminal animation; stop with Ctrl+C |
| `bonsai.c`, `winner2024ioccc.c` | IOCCC submissions with platform-specific build assumptions |

For the original IOCCC build instructions, inspect their sources and makefiles:
`make -C examples -f bonsai.Makefile` or
`make -C examples -f winner2024ioccc.Makefile`. They are not automatically
validated. The curated target intentionally cannot launch these by name.
