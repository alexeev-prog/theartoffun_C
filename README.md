# The Art of Fun C - Algorithm Exploration Toolkit

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

Explore fascinating algorithms implemented in C with this educational toolkit. The project demonstrates various mathematical tricks, optimization techniques, and clever programming approaches through a practical command-line interface.

## Table of Contents
- [Key Features](#key-features)
- [Setup Instructions](#setup-instructions)
- [Usage Examples](#usage-examples)
- [Algorithm Details](#algorithm-details)
- [Performance Benchmarks](#performance-benchmarks)
- [Project Structure](#project-structure)
- [Development Environment](#development-environment)
- [Contribution Guidelines](#contribution-guidelines)

## Key Features

### 🧮 Mathematical Algorithms
- **Fibonacci-based conversions**:
  - Basic adjacent number conversion
  - Linear interpolation between Fibonacci numbers
  - Pre-calculated sequence caching
  - Golden ratio approximation
- **Binary Exponentiation** - Efficient O(log n) exponentiation
- **Xorshift PRNG** - Fast 64-bit pseudorandom generator
- **Quake III Inverse Square Root** - Famous bit-manipulation trick
- **Lehmer64 & Xoshiro256++** - Advanced PRNG implementations

### ⚙️ Command-Line Interface
- Intuitive command parser with help system
- Input validation and error handling
- Benchmarking mode for performance testing
- Cross-platform support (Windows/Linux/macOS)

### 📊 Performance Tracking
- Built-in benchmark suite
- Timing measurements for all algorithms
- Accuracy comparisons
- Memory usage analysis

## Setup Instructions

### Prerequisites
- GCC compiler (min version 9.0)
- GNU Make
- Math library (`-lm`)
- (Optional) Nix for reproducible builds

### Building from Source
```bash
# Clone repository
git clone https://github.com/your-username/theartoffun_c.git
cd theartoffun_c

# Build project
make

# Run executable
./bin/theartoffun --help
```

### Using Nix Development Environment
```bash
nix-shell shell.nix  # Enters development environment
make                # Build inside environment
```

## Usage Examples

### Conversion Methods
```bash
# Basic Fibonacci conversion (5 miles ≈ 8 km)
./bin/theartoffun --fib 5

# Golden ratio conversion
./bin/theartoffun --fib-golden 20

# Cached Fibonacci conversion
./bin/theartoffun --fib-cache 100
```

### Mathematical Operations
```bash
# Binary exponentiation (calculate 10^5)
./bin/theartoffun --binary-power 10 --exponent 5

# Generate random numbers (Xorshift)
./bin/theartoffun --xorshift-random

# Calculate inverse square root (Quake III)
./bin/theartoffun --q-rsqrt-quake 25
```

### Benchmarking
```bash
# Run full benchmark suite
./bin/theartoffun -B

# Sample benchmark output:
# PRNG Performance (10,000,000 iterations):
# -----------------------------------------
# xorshift64:      14.18 ms  (705.37M numbers/s)
# lehmer64:        20.71 ms  (482.89M numbers/s)
# xoshiro256pp:    15.95 ms  (626.77M numbers/s)
```

## Algorithm Details

### Fibonacci Conversion
- **Mathematical Basis**: Uses relationship between Fibonacci sequence (Fₙ₊₁/Fₙ → φ ≈ 1.618) and miles/km ratio (1.609)
- **Error Analysis**: Consistent ~0.54% error due to golden ratio approximation
- **Four Implementations**:
  1. Basic adjacent number lookup
  2. Linear interpolation between sequence points
  3. Pre-cached sequence for O(1) lookup
  4. Golden ratio with binary exponentiation

### Binary Exponentiation
```c
double binary_pow(double b, unsigned long long e) {
    double v = 1.0;
    while(e != 0) {
        if((e & 1) != 0) v *= b;
        b *= b;
        e >>= 1;
    }
    return v;
}
```
- **Complexity**: O(log n) vs O(n) for naive multiplication
- **Applications**: Efficient computation in Fibonacci formulas

### Xorshift PRNG
```c
uint64_t xorshift64(uint64_t *state) {
    uint64_t x = *state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    *state = x;
    return x;
}
```
- **Performance**: 3 operations per number generation
- **Period**: 2⁶⁴ - 1
- **Quality**: Passes basic statistical tests

### Quake III Inverse Square Root
```c
float Q_rsqrt(float number) {
    int32_t i;
    const float threehalfs = 1.5F;
    float x2 = number * 0.5F;
    float y = number;

    i = *(int32_t*)&y;
    i = 0x5f3759df - (i >> 1);
    y = *(float*)&i;
    y = y * (threehalfs - (x2 * y * y));

    return y;
}
```
- **Magic Constant**: 0x5f3759df
- **Accuracy**: Within 0.2% after one Newton iteration
- **Innovation**: Combines bit manipulation with numerical approximation

## Performance Benchmarks

### PRNG Throughput (10M iterations)
| Algorithm     | Time (ms) | Speed (M nums/s) |
|---------------|-----------|------------------|
| xorshift64    | 14.18     | 705.37           |
| lehmer64      | 20.71     | 482.89           |
| xoshiro256pp  | 15.95     | 626.77           |

### Conversion Methods (20k calls)
| Method                     | Time (ms) | Per Call (μs) |
|----------------------------|-----------|---------------|
| Basic Multiplication       | 0.28      | 0.001         |
| Fibonacci Interpolation    | 1.56      | 0.008         |
| Cached Fibonacci           | 1.41      | 0.007         |
| Golden Ratio               | 16.33     | 0.082         |
| Golden Ratio (Binary Pow)  | 3.56      | 0.018         |

## Project Structure

### Core Components
```
├── bin/                   # Compiled binaries
├── src/                   # Source code
│   ├── algos.c            # PRNG and Q_rsqrt implementations
│   ├── algos.h            # Algorithm headers
│   ├── cmdparser.h        # Command-line parser (600+ lines)
│   ├── fib_algos.c        # Fibonacci implementations
│   ├── fib_algos.h        # Fibonacci headers
│   ├── main.c             # Entry point (500+ lines)
│   ├── pow_algos.c        # Binary exponentiation
│   └── pow_algos.h        # Power algorithm headers
├── docs/
│   └── article.md         # Detailed technical article (Russian)
```

### Support Files
```
├── Makefile               # Build configuration
├── shell.nix              # Nix development environment
├── format-code.py         # Code formatter script
├── LICENSE                # MIT License
└── CHANGELOG.md           # Version history
```

## Development Environment

### Nix Configuration
The `shell.nix` file provides a reproducible development environment with:
- GCC compiler
- GNU Make
- Binutils
- pkg-config
- Valgrind (memory debugging)
- GDB (debugger)

### Building and Testing
```bash
# Build project
make

# Run tests
./bin/theartoffun -B  # Benchmarks
./bin/theartoffun --fib 5  # Sample test

# Format code
python format-code.py src/ --style=LLVM
```

### Debugging
```bash
# Start debug session
gdb ./bin/theartoffun

# Memory checking
valgrind --leak-check=full ./bin/theartoffun --fib 10
```

## Contribution Guidelines

### Workflow
1. Fork the repository
2. Create feature branch (`git checkout -b feature/algorithm-name`)
3. Commit changes with descriptive messages
4. Submit pull request

### Coding Standards
- Follow C99 standard
- Use descriptive variable names
- Include Doxygen-style comments for functions
- Maintain consistent 4-space indentation
- Validate all inputs with error checking

### Testing Requirements
- Add benchmarks for new algorithms
- Include edge case tests
- Maintain >95% code coverage
- Update documentation for new features

## License
This project is licensed under the MIT License - see [LICENSE](LICENSE) for details.

