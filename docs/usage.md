# Application cookbook

## Build, explore, reproduce

```sh
make -j2
make run ARGS="--help"
make demo
```

The application performs **one operation per invocation**. Modifiers such as
`--exponent` are not operations. Unexpected trailing arguments, repeated options,
and conflicting operations fail with a nonzero status instead of silently
ignoring part of the command.

## Exact operations

| Command | Result |
| :--- | :--- |
| `--binary-power 2 --exponent 10` | 1024 |
| `--isqrt 4294967295` | 65535, floor of the square root |
| `--morton-encode 10 13` | 230 |
| `--morton-decode 230` | Coordinates 10, 13 |
| `--levenshtein kitten sitting` | Edit distance 3 |
| `--div3 123` | 41 |
| `--fast-mod 123 16` | 11; modulus must be a power of two |

Unsigned values are decimal integers with no sign or whitespace. Morton
coordinates fit in 16 bits; codes and integer-square-root arguments fit in
32 bits. Binary exponents fit in 64 bits and must be integers. Levenshtein
distance works on bytes, not Unicode code points; shell-quote strings with spaces.

## Compression and safety

```sh
./bin/theartoffun --rle-encode aaabbc       # 3a2bc
./bin/theartoffun --rle-decode 3a2bc        # aaabbc
```

The legacy RLE format cannot encode literal digits. Malformed, zero, and
overflowing counts fail. The CLI caps decoded output at **1 MiB** before
allocating memory. Library users can select their own cap with
`rle_decode_limit(input, max_output)` and must free successful results.

## Approximate operations

```sh
./bin/theartoffun --miles-to-km 5
./bin/theartoffun --fib-interp 5
./bin/theartoffun --q-rsqrt-quake 25
./bin/theartoffun --fast-pow 2 3
```

Distance conversion methods accept 0..100000 miles. The historical `--fib N`
command is different: it evaluates F(N+1), accepts integer N in 0..92, and is
not a general distance-conversion formula. Use `--fib-interp` for an actual
distance approximation. The historical spelling `--fib-golden-binry` is retained.

Inverse-square-root input must be a positive normal finite float. Approximate
powers accept positive finite bases and exponents in -1000..1000; unsupported
bit-approximation results fail rather than invoking an unsafe integer cast.
Use libm, not these educational approximations, for numerical applications.

## Parser syntax

```sh
./bin/theartoffun --binary-power=2 --exponent=10
./bin/theartoffun -p2 -e10
./bin/theartoffun --levenshtein=-text -- -test
./bin/theartoffun 1 5 10
```

- Long options accept `--name=value` and `--name value`.
- Short flags can be grouped; a value-taking short option consumes the remainder
  of its token, as in `-p2`. Short `-p=2` is intentionally rejected.
- Negative numeric values can follow a value-taking option. Other dash-prefixed
  text should use `--name=-text` to avoid ambiguity with a missing argument.
- `--` ends option parsing; parsing also stops at the first positional value.
  Put options before positional values.
- An explicit empty string (`--rle-encode=`) is distinct from a missing argument.
- Repeating an option, even through its short alias, is an error.
- With no operation, positional arguments are distances in miles. Invalid entries
  are reported and the overall invocation exits unsuccessfully.

## Explore further

`make demo` runs checked command scenarios. `make list-examples` lists individual
C programs, and `make run-example EXAMPLE=mortoncode ARGS="10 13"` builds and
runs one. See the example catalog and development guide for limitations.
