# The Art of Fun C - Advanced Algorithm Exploration Toolkit

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

A comprehensive collection of low-level programming techniques, mathematical optimizations, and algorithmic implementations in C. This project explores the intersection of mathematics, bit manipulation, and performance optimization through practical implementations and detailed analysis.

## Table of Contents
- [Overview](#overview)
- [Project Structure](#project-structure)
- [Core Algorithm Categories](#core-algorithm-categories)
- [Implementation Details](#implementation-details)
- [Building and Installation](#building-and-installation)
- [Usage Examples](#usage-examples)
- [Performance Analysis](#performance-analysis)
- [Technical Documentation](#technical-documentation)
- [Development](#development)

## Overview

This project implements and benchmarks numerous algorithms spanning multiple domains:
- **Mathematical optimizations** (fast division, power calculations, square roots)
- **Pseudorandom number generators** (xorshift, PCG, xoshiro, wyrand, MSWS, RomuDuo)
- **Bit manipulation techniques** (Gray codes, trailing zero counting, power-of-two checks)
- **Numerical approximations** (Quake III inverse square root, Fibonacci-based conversions)
- **Sorting and hashing algorithms** (counting sort, Jenkins hash)

All implementations are rigorously tested and benchmarked, with detailed performance metrics provided.

## Project Structure

```
.
├── src/
│   ├── algos.h              # Main algorithm declarations
│   ├── algos.c              # Core algorithm implementations
│   ├── fib_algos.h          # Fibonacci-based conversion declarations
│   ├── fib_algos.c          # Fibonacci algorithm implementations
│   ├── pow_algos.h          # Power calculation declarations
│   ├── pow_algos.c          # Exponentiation algorithms
│   ├── cmdparser.h          # Advanced command-line parser (600+ lines)
│   └── main.c               # Application entry point and benchmarking (500+ lines)
├── docs/
│   ├── article.md           # Part 1: Mathematics, bits, and C programming
│   ├── article_part2.md     # Part 2: Hacks, tricks, and abnormal programming
│   └── article_part3.md     # Part 3: Dark side of C - tricks, hacks, magic
├── bin/                     # Compiled binaries and object files
├── Makefile                 # Build configuration
├── shell.nix               # Nix development environment
├── format-code.py          # Code formatting utility
├── LICENSE                 # MIT License
└── CHANGELOG.md            # Version history
```

## Core Algorithm Categories

### Mathematical Algorithms

**Fibonacci-based Conversions:**
- `fibonacci()` - Classical iterative Fibonacci computation
- `fib_interpolate()` - Linear interpolation between Fibonacci numbers
- `fib_cache_convert()` - Pre-cached sequence for O(1) lookup
- `fib_golden_ratio()` - Formula-based using Binet's formula
- `fib_golden_ratio_binary()` - Optimized with binary exponentiation

**Numerical Optimizations:**
- `binary_pow()` - O(log n) exponentiation via bit manipulation
- `fast_pow()` - IEEE 754-based approximation for floating-point powers
- `fastest_pow()` - Optimized float version of power approximation
- `div3()` - Fast division by 3 using multiplication and shift
- `isqrt()` - Integer square root via binary search
- `Q_rsqrt()` - Famous Quake III inverse square root approximation

### Pseudorandom Number Generators

**High-Performance PRNGs:**
- `xorshift64()` - Ultra-fast 64-bit generator (3 operations per number)
- `lehmer64()` - Linear congruential generator with 128-bit state
- `xoshiro256pp()` - 256-bit state generator with excellent statistical properties
- `pcg32_random_r()` - Permuted congruential generator with output transformation
- `wyrand()` - 128-bit arithmetic based generator
- `msws32()` - Middle-square Weyl sequence generator
- `romu_duo()` - Fast generator ideal for simulations

### Bit Manipulation and Hashing

**Bit-Level Operations:**
- `xor_swap()` - Variable swap without temporary storage
- `is_power_of_two()` - Single-expression power-of-two check
- `fast_mod()` - Efficient modulus for powers of two
- `to_gray()/from_gray()` - Gray code conversion algorithms
- `count_trailing_zeros()` - Multiple implementations for zero bit counting

**Hashing Functions:**
- `jenkins_mix()/jenkins_final()` - Lookup3 hash mixing primitives
- `jenkins_hash()` - Complete Jenkins hash implementation

### Sorting and Data Processing
- `counting_sort_256()` - Linear-time sort for small value ranges

## Implementation Details

### Key Header Files

**`algos.h`** - Main algorithm declarations:
```c
// PRNG state structures
typedef struct { uint64_t s[4]; } xoshiro256pp_state;
typedef struct { uint64_t state; uint64_t inc; } pcg32_random_t;

// Core algorithm declarations
uint64_t xorshift64(uint64_t* state);
uint32_t pcg32_random_r(pcg32_random_t* rng);
float Q_rsqrt(float number);
uint32_t isqrt(uint32_t x);
uint32_t to_gray(uint32_t n);
void counting_sort_256(uint8_t* arr, size_t n);
```

**`fib_algos.h`** - Fibonacci-based conversions:
```c
uint64_t fibonacci(int num);
float fib_interpolate(float miles);
float fib_golden_ratio(float miles);
float fib_golden_ratio_binary(float miles);
```

**`pow_algos.h`** - Exponentiation algorithms:
```c
double binary_pow(double b, unsigned long long e);
double fast_pow(double a_coeff, double base);
float fastest_pow(float a, float b);
```

### Mathematical Foundations

**Fibonacci Conversion Principle:**
The relationship between Fibonacci numbers (Fₙ₊₁/Fₙ → φ ≈ 1.618) approximates the miles-to-kilometers conversion factor (1.609344) with ~0.54% error. This enables multiple conversion strategies with varying accuracy/performance tradeoffs.

**Fast Inverse Square Root:**
The Quake III algorithm combines bit manipulation with Newton-Raphson iteration:
```c
float Q_rsqrt(float number) {
    int32_t i;
    float x2 = number * 0.5F;
    float y = number;

    i = *(int32_t*)&y;           // Bit-level hacking
    i = 0x5f3759df - (i >> 1);   // Magic constant and shift
    y = *(float*)&i;
    y = y * (1.5F - (x2 * y * y)); // Newton iteration

    return y;
}
```

**Binary Exponentiation:**
Efficient O(log n) power computation:
```c
double binary_pow(double b, unsigned long long e) {
    double v = 1.0;
    while (e != 0) {
        if ((e & 1) != 0) v *= b;
        b *= b;
        e >>= 1;
    }
    return v;
}
```

## Building and Installation

### Prerequisites
- GCC compiler (version 9.0 or later)
- GNU Make
- Math library (`-lm`)
- (Optional) Nix for reproducible builds

### Build Instructions
```bash
# Clone and build
git clone https://github.com/alexeev-prog/theartoffun_c.git
cd theartoffun_c
make

# Run the application
./bin/theartoffun --help
```

### Nix Development Environment
```bash
nix-shell shell.nix  # Enter development environment
make clean && make   # Build inside environment
```

## Usage Examples

### Mathematical Operations
```bash
# Fibonacci-based conversion (5 miles ≈ 8 km)
./bin/theartoffun --fib 5

# Golden ratio conversion with binary exponentiation
./bin/theartoffun --fib-golden-binry 20

# Binary exponentiation (compute 2^10)
./bin/theartoffun --binary-power 2 --exponent 10

# Fast inverse square root (Quake III algorithm)
./bin/theartoffun --q-rsqrt-quake 25
```

### Pseudorandom Generation
```bash
# Generate random numbers with various algorithms
./bin/theartoffun --xorshift-random
./bin/theartoffun --lehmer-random
./bin/theartoffun --xorshiro256pp-random
./bin/theartoffun --pcg32-random
```

### Bit Manipulation and Hashing
```bash
# XOR swap demonstration
./bin/theartoffun --xor-swap 42 1337

# Fast division by 3
./bin/theartoffun --div3 123

# Jenkins hash calculation
./bin/theartoffun --jenkins-hash "test_data"

# Power-of-two check
./bin/theartoffun --power-of-two 1024
```

### Comprehensive Benchmarking
```bash
# Run full benchmark suite
./bin/theartoffun --benchmark
```

## Performance Analysis

### PRNG Performance (10,000,000 iterations)
```
-----------------------------------------
xorshift64:      15.31 ms  (652.97M numbers/s)
lehmer64:        21.34 ms  (468.57M numbers/s)
xoshiro256pp:    16.12 ms  (620.18M numbers/s)
pcg32:           14.07 ms  (710.60M numbers/s)
wyrand:          18.06 ms  (553.85M numbers/s)
msws32:          17.99 ms  (555.97M numbers/s)
romu_duo:        14.21 ms  (703.78M numbers/s)
-----------------------------------------
```

### Conversion Methods Performance
```
----------------------------------------------------------------------
Basic                    :     0.23 ms  ( 0.001 us/call)
Fibonacci Interpolation  :     1.58 ms  ( 0.008 us/call)
Fibonacci Cache          :     1.40 ms  ( 0.007 us/call)
Golden Ratio             :    17.02 ms  ( 0.085 us/call)
Golden Ratio (Binary)    :     3.54 ms  ( 0.018 us/call)
----------------------------------------------------------------------
```

### Mathematical Algorithms Performance
```
--------------------------------------------
fast_pow:                1.41 ms  ( 0.001 us/call)
fastest_pow:             1.32 ms  ( 0.001 us/call)
fast_mod:                1.70 ms  ( 0.002 us/call)
is_power_of_two:         1.46 ms  ( 0.001 us/call)
jenkins_hash:           30.17 ms  ( 0.030 us/call)
jenkins_mix+final:      29.34 ms  ( 0.029 us/call)
xor_swap:                2.21 ms  ( 0.002 us/call)
div3:                    1.13 ms  ( 0.001 us/call)
isqrt:                   6.12 ms  ( 0.006 us/call)
to_gray:                 1.16 ms  ( 0.001 us/call)
from_gray:               1.37 ms  ( 0.001 us/call)
counting_sort_256:       7.58 ms  ( 0.758 us/call)
--------------------------------------------
```

## Technical Documentation

### Algorithm Specifications

**Xorshift64 PRNG:**
```c
uint64_t xorshift64(uint64_t* state) {
    uint64_t x = *state;
    x ^= x << 13;  // Spread bits to higher positions
    x ^= x >> 7;   // Compensate with reverse shift
    x ^= x << 17;  // Final mixing
    *state = x;
    return x;
}
```

**PCG32 Random Number Generator:**
```c
uint32_t pcg32_random_r(pcg32_random_t* rng) {
    uint64_t oldstate = rng->state;
    rng->state = oldstate * 6364136223846793005ULL + (rng->inc | 1);
    uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    uint32_t rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}
```

**Integer Square Root (ISQRT):**
```c
uint32_t isqrt(uint32_t x) {
    uint32_t res = 0;
    uint32_t bit = 1 << 30;

    while (bit > x) bit >>= 2;  // Find highest possible bit

    while (bit) {
        if (x >= res + bit) {
            x -= res + bit;
            res = (res >> 1) + bit;
        } else {
            res >>= 1;
        }
        bit >>= 2;
    }
    return res;
}
```

### Error Analysis

**Fibonacci Conversion Accuracy:**
All Fibonacci-based methods maintain approximately 0.54% error due to the mathematical relationship between the golden ratio (φ ≈ 1.618034) and the actual conversion factor (1.609344).

**Fast Power Approximation:**
- `fast_pow()`: ~1-10% error depending on inputs
- `fastest_pow()`: Faster but less accurate float version
- Both methods significantly outperform standard `pow()` function

## Development

### Code Quality
- C99 standard compliance
- Comprehensive error handling
- Cross-platform compatibility (Windows/Linux/macOS)
- Extensive benchmarking infrastructure
- Memory safety validation

### Testing and Validation
```bash
# Run benchmarks
./bin/theartoffun -B

# Memory checking with Valgrind
valgrind --leak-check=full ./bin/theartoffun --fib 10

# Debug build
make DEBUG=1
gdb ./bin/theartoffun
```

### Code Formatting
```bash
python format-code.py src/ --style=LLVM
```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## References

1. [PCG: A Family of Simple Fast Space-Efficient Statistically Good Algorithms for Random Number Generation](https://www.pcg-random.org/)
2. [Xorshift RNGs by George Marsaglia](https://www.jstatsoft.org/article/view/v008i14)
3. [Xoshiro / xoroshiro generators and the PRNG shootout](https://prng.di.unimi.it/)
4. [Fast Inverse Square Root - Quake III Arena](https://github.com/id-Software/Quake-III-Arena)
5. [Bob Jenkins' Hash Functions](https://burtleburtle.net/bob/hash/doobs.html)

---

*This project serves as both an educational resource and a practical toolkit for understanding low-level optimization techniques and algorithmic efficiency in C programming.*
