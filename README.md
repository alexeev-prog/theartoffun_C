# The Art of Fun C
Article in russian [here](./docs/article.md).

## Overview
This project implements several interesting algorithms in C, including Fibonacci-based miles to kilometers conversion, binary exponentiation, Xorshift pseudo-random number generation, and Quake III's fast inverse square root algorithm. The program provides a command-line interface to demonstrate these algorithms in action.

## Features

### Fibonacci-Based Conversion
- **Basic Fibonacci**: Uses adjacent Fibonacci numbers for conversion
- **Fibonacci Interpolation**: Linear interpolation between Fibonacci numbers
- **Cached Fibonacci**: Pre-calculates Fibonacci sequence for faster lookup
- **Golden Ratio**: Uses the golden ratio for conversion

### Mathematical Algorithms
- **Binary Exponentiation**: Efficient O(log n) exponentiation algorithm
- **Xorshift PRNG**: Fast pseudo-random number generator
- **Quake III Inverse Square Root**: Famous optimization trick from game development

### Command-Line Interface
- Intuitive command parser with help system
- Support for multiple conversion methods
- Input validation and error handling

## Building the Project

### Requirements
- GCC compiler
- GNU Make
- Math library (`-lm`)

### Build Instructions
```bash
# Clone repository
git clone https://github.com/your-username/theartoffun_c.git
cd theartoffun_c

# Build project
make

# Run executable
./bin/theartoffun
```

## Usage Examples

```
TheArtOfFun-C
Usage: ./bin/theartoffun [OPTIONS] [commands]

Options:
  -h, --help                     Show help information
  -f, --fib=ARG                  Convert miles to km miles to km using basic Fibonacci
  -m, --miles-to-km=ARG          Convert miles to km miles to km using standard formula
  -i, --fib-interp=ARG           Convert miles to km using Fibonacci interpolation
  -c, --fib-cache=ARG            Convert miles to km using cached Fibonacci
  -g, --fib-golden=ARG           Convert miles to km using golden ratio
  -e, --exponent=ARG             Set exponent for pow-algos
  -p, --binary-power=ARG         Power the number by binary power algorithm
  -x, --xorshift-random          Generate pseudo random numbers by xorshift64
  -d, --xorshift-double-random   Generate pseudo random float numbers by xorshift64
  -q, --q-rsqrt-quake=ARG        Q_rsqrt from Quake III Arena
  -b, --fib-golden-binry=ARG     Convert miles to km using golden ratio with binary pow
  -l, --lehmer-random            Generate pseudo random numbers by lehmer64
  -o, --xorshiro256pp-random     Generate pseudo random numbers by xoshiro256pp
```

### Fibonacci Conversion
```bash
# Basic Fibonacci conversion (5 miles ≈ 8 km)
./bin/theartoffun --fib 5

# Fibonacci interpolation conversion
./bin/theartoffun --fib-interp 10

# Golden ratio conversion
./bin/theartoffun --fib-golden 20
```

### Mathematical Operations
```bash
# Binary exponentiation (calculate 10^5)
./bin/theartoffun --binary-power 10 --exponent 5

# Generate random numbers
./bin/theartoffun --xorshift-random

# Calculate inverse square root (Quake III algorithm)
./bin/theartoffun --q-rsqrt-quake 25
```

### Getting Help
```bash
# Show help message
bin/theartoffun --help
```

### Launch Benchmark
```bash
# Run benchmark
bin/theartoffun -B
```

## Algorithm Details

### Fibonacci Conversion
- Uses properties of Fibonacci sequence and golden ratio
- Approximation error ≈ 0.54%
- Multiple implementations for comparison

### Binary Exponentiation
- Efficient O(log n) algorithm
- Avoids expensive repeated multiplication
- Suitable for large exponents

### Xorshift PRNG
- Extremely fast O(1) generation
- Passes statistical randomness tests
- Period of 2⁶⁴ - 1

### Fast Inverse Square Root
- From Quake III Arena source code
- Uses bit manipulation and Newton-Raphson iteration
- Accuracy within 0.2%

## Technical Notes
- 64-bit integers used for Fibonacci calculations to prevent overflow
- IEEE 754 floating-point representation compliance
- Input validation for all parameters
- Platform-independent code with Windows compatibility

## Repository Structure
```
├── bin/              # Build output directory
├── src/              # Source code
│   ├── algos.c       # Xorshift and Q_rsqrt implementations
│   ├── algos.h       # Header for algos.c
│   ├── cmdparser.h   # Command-line parser
│   ├── fib_algos.c   # Fibonacci conversion implementations
│   ├── fib_algos.h   # Header for fib_algos.c
│   ├── main.c        # Main program
│   ├── pow_algos.c   # Binary exponentiation
│   └── pow_algos.h   # Header for pow_algos.c
├── Makefile          # Build configuration
└── shell.nix         # Nix development environment
```

## Contributing
Contributions are welcome! Please follow these guidelines:
1. Fork the repository
2. Create a feature branch
3. Submit a pull request with clear description
4. Maintain consistent coding style
5. Include tests for new functionality

## License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
