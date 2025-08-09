# The Art of Fun C

A collection of interesting algorithms and programming techniques implemented in C, demonstrating mathematical concepts and efficient computation methods.

## Project Overview

This repository contains implementations of:
- Fibonacci-based distance conversion algorithms
- Efficient binary exponentiation
- Xorshift pseudo-random number generators
- Command-line parsing utilities
- Mathematical curiosities

## Building the Project

### Prerequisites
- GCC compiler
- GNU Make
- Standard C library
- Math library (`-lm`)

### Build Instructions
```bash
make          # Build main executable (bin/theartoffun)
make secret   # Build easter egg executable (bin/secret)
make clean    # Remove build artifacts
```

### Build Outputs
- `bin/theartoffun`: Main CLI program

## Algorithms

### Fibonacci-based Conversions
- **Basic Implementation**: `fibonacci()`
- **Miles to Kilometers**:
  - Standard conversion: `basic_miles2km()`
  - Fibonacci interpolation: `fib_interpolate()`
  - Cached Fibonacci: `fib_cache_convert()`
  - Golden ratio method: `fib_golden_ratio()`

### Efficient Exponentiation
- **Binary Power Algorithm**: `binary_pow()`
  - Computes b^e in O(log n) time
  - Supports positive integer exponents

### Random Number Generation
- **Xorshift64 PRNG**:
  - Core generator: `xorshift64()`
  - Range-based integers: `rand_range()`
  - Double-precision floats: `rand_double()`
  - Seed from system time: `get_seed()`

## Command Line Interface

### Usage
```bash
TheArtOfFun-C
Usage: ./bin/theartoffun [OPTIONS] [commands]

Options:
  -h, --help                     Show help information
  -f, --fib=ARG                  Convert miles to km using basic Fibonacci
  -b, --basic=ARG                Convert miles to km using standard formula
  -i, --fib-interp=ARG           Convert using Fibonacci interpolation
  -c, --fib-cache=ARG            Convert using cached Fibonacci
  -g, --fib-golden=ARG           Convert using golden ratio
  -e, --exponent=ARG             Set exponent for pow-algos
  -p, --binary-power=ARG         Power the number by binary power algorithm
  -x, --xorshift-random          Generate pseudo random numbers
  -d, --xorshift-double-random   Generate pseudo random float numbers
```

### Conversion Options
```
--basic VALUE       Convert miles to km using standard formula
--fib VALUE         Convert using basic Fibonacci method
--fib-interp VALUE  Convert using Fibonacci interpolation
--fib-cache VALUE   Convert using cached Fibonacci values
--fib-golden VALUE  Convert using golden ratio method
```

### Computational Options
```
--binary-power BASE --exponent EXP  Compute power using binary exponentiation
--xorshift-random                   Generate random integers
--xorshift-double-random            Generate random doubles
```

### Examples
```bash
# Convert 5 miles to km using golden ratio method
./bin/theartoffun --fib-golden 5

# Compute 10.5^4 using binary exponentiation
./bin/theartoffun --binary-power 10.5 --exponent 4

# Generate random numbers
./bin/theartoffun --xorshift-random
```

### Easter Egg
```bash
clang secret.c -o secret
./secret  # Outputs a surprise message
```

## File Structure
```
src/
├── algos.c           # Random number implementations
├── algos.h
├── cmdparser.h       # Command-line parsing
├── fib_algos.c       # Fibonacci algorithms
├── fib_algos.h
├── main.c            # Entry point
├── pow_algos.c       # Exponentiation
├── pow_algos.h
└── secret.c          # Easter egg
bin/                  # Build output directory
Makefile              # Build configuration
```

## Technical Notes
- All floating-point operations use IEEE 754 double precision
- Fibonacci implementations handle values up to 2^64-1
- Random number generator passes basic statistical tests
- Safe input validation for all user-provided values

## Dependencies
- Standard C library (C11)
- Math library (-lm)

## Contributing
Issues and pull requests are welcome. Please ensure:
- Code follows project style conventions
- Documentation is updated

