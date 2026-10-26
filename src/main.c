#ifdef _WIN32
#    include <windows.h>
#endif
#include <inttypes.h>
#include <errno.h>
#include <math.h>
#include <stdint.h>
#ifdef _WIN32
#    include <windows.h>
#else
#    include <time.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "algos.h"
#include "benchmarks.h"
#include "cmdparser.h"
#include "compressing.h"
#include "fib_algos.h"
#include "pow_algos.h"

uint64_t get_seed() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * (uint64_t)1000000 + tv.tv_usec;
}

int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    int help_flag = 0;
    char* fib_value = NULL;
    char* basic_value = NULL;
    char* fib_interp_value = NULL;
    char* fib_cache_value = NULL;
    char* fib_golden_value = NULL;
    char* binary_power = NULL;
    int xorshift_flag = 0;
    int xorshift_double_flag = 0;
    char* q_rsqrt_quake = NULL;
    char* fib_golden_binary_value = NULL;
    int lehmer64_value_flag = 0;
    int xoshiro256pp_flag = 0;
    int benchmark_flag = 0;
    char* fast_pow_base = NULL;
    char* fastest_pow_base = NULL;
    char* fast_mod_value = NULL;
    char* power_of_two_value = NULL;
    char* jenkins_hash_data = NULL;
    int jenkins_mix_flag = 0;
    int pcg32_flag = 0;
    char* xor_swap_a = NULL;
    char* div3_value = NULL;
    char* rle_encode_data = NULL;
    char* rle_decode_data = NULL;
    char* is_palindrome_str = NULL;
    char* leap_year_value = NULL;
    char* zellers_day = NULL;
    char* count_trailing_zeros_value = NULL;
    char* next_power_value = NULL;
    char* fisher_yates_size = NULL;
    char* sfc32_flag = NULL;
    char* sha1_prng_flag = NULL;

    char* exponent = NULL;

    struct CommandOption options[] = {
        { .help = "Show help information",
         .long_name = "help",
         .short_name = 'h',
         .has_arg = 0,
         .default_value = NULL,
         .handler = &help_flag                  },
        { .help = "Convert miles to km miles to km using basic Fibonacci",
         .long_name = "fib",
         .short_name = 'f',
         .has_arg = 1,
         .default_value = NULL,
         .handler = &fib_value                  },
        { .help = "Convert miles to km miles to km using standard formula",
         .long_name = "miles-to-km",
         .short_name = 'm',
         .has_arg = 1,
         .default_value = NULL,
         .handler = &basic_value                },
        { .help = "Convert miles to km using Fibonacci interpolation",
         .long_name = "fib-interp",
         .short_name = 'i',
         .has_arg = 1,
         .default_value = NULL,
         .handler = &fib_interp_value           },
        { .help = "Convert miles to km using cached Fibonacci",
         .long_name = "fib-cache",
         .short_name = 'c',
         .has_arg = 1,
         .default_value = NULL,
         .handler = &fib_cache_value            },
        { .help = "Convert miles to km using golden ratio",
         .long_name = "fib-golden",
         .short_name = 'g',
         .has_arg = 1,
         .default_value = NULL,
         .handler = &fib_golden_value           },
        { .help = "Set exponent for pow-algos",
         .long_name = "exponent",
         .short_name = 'e',
         .has_arg = 1,
         .default_value = NULL,
         .handler = &exponent                   },
        { .help = "Power the number by binary power algorithm",
         .long_name = "binary-power",
         .short_name = 'p',
         .has_arg = 1,
         .default_value = NULL,
         .handler = &binary_power               },
        { .help = "Generate pseudo random numbers by xorshift64",
         .long_name = "xorshift-random",
         .short_name = 'x',
         .has_arg = 0,
         .default_value = NULL,
         .handler = &xorshift_flag              },
        { .help = "Generate pseudo random float numbers by xorshift64",
         .long_name = "xorshift-double-random",
         .short_name = 'd',
         .has_arg = 0,
         .default_value = NULL,
         .handler = &xorshift_double_flag       },
        { .help = "Q_rsqrt from Quake III Arena",
         .long_name = "q-rsqrt-quake",
         .short_name = 'q',
         .has_arg = 1,
         .default_value = NULL,
         .handler = &q_rsqrt_quake              },
        { .help = "Convert miles to km using golden ratio with binary pow",
         .long_name = "fib-golden-binry",
         .short_name = 'b',
         .has_arg = 1,
         .default_value = NULL,
         .handler = &fib_golden_binary_value    },
        { .help = "Generate pseudo random numbers by lehmer64",
         .long_name = "lehmer-random",
         .short_name = 'l',
         .has_arg = 0,
         .default_value = NULL,
         .handler = &lehmer64_value_flag        },
        { .help = "Generate pseudo random numbers by xoshiro256pp",
         .long_name = "xorshiro256pp-random",
         .short_name = 'o',
         .has_arg = 0,
         .default_value = NULL,
         .handler = &xoshiro256pp_flag          },
        { .help = "Run benchmarks for PRNGs and conversion methods",
         .long_name = "benchmark",
         .short_name = 'B',
         .has_arg = 0,
         .default_value = NULL,
         .handler = &benchmark_flag             },
        { .help = "Fast power calculation (base)",
         .long_name = "fast-pow",
         .short_name = 'P',
         .has_arg = 1,
         .default_value = NULL,
         .handler = &fast_pow_base              },
        { .help = "Fastest power calculation (base)",
         .long_name = "fastest-pow",
         .short_name = 'F',
         .has_arg = 1,
         .default_value = NULL,
         .handler = &fastest_pow_base           },
        { .help = "Fast modulo calculation (value)",
         .long_name = "fast-mod",
         .short_name = 'M',
         .has_arg = 1,
         .default_value = NULL,
         .handler = &fast_mod_value             },
        { .help = "Check if number is power of two",
         .long_name = "power-of-two",
         .short_name = 'T',
         .has_arg = 1,
         .default_value = NULL,
         .handler = &power_of_two_value         },
        { .help = "Jenkins hash calculation (data)",
         .long_name = "jenkins-hash",
         .short_name = 'J',
         .has_arg = 1,
         .default_value = NULL,
         .handler = &jenkins_hash_data          },
        { .help = "Test Jenkins mix and final functions",
         .long_name = "jenkins-mix",
         .short_name = 'j',
         .has_arg = 0,
         .default_value = NULL,
         .handler = &jenkins_mix_flag           },
        { .help = "Generate pseudo random numbers by PCG32",
         .long_name = "pcg32-random",
         .short_name = 'R',
         .has_arg = 0,
         .default_value = NULL,
         .handler = &pcg32_flag                 },
        { .help = "XOR swap two numbers (first value)",
         .long_name = "xor-swap",
         .short_name = 'X',
         .has_arg = 1,
         .default_value = NULL,
         .handler = &xor_swap_a                 },
        { .help = "Fast division by 3",
         .long_name = "div3",
         .short_name = 'D',
         .has_arg = 1,
         .default_value = NULL,
         .handler = &div3_value                 },
        { .help = "RLE encode string",
         .long_name = "rle-encode",
         .short_name = 'E',
         .has_arg = 1,
         .default_value = NULL,
         .handler = &rle_encode_data            },
        { .help = "RLE decode string",
         .long_name = "rle-decode",
         .short_name = 'C',
         .has_arg = 1,
         .default_value = NULL,
         .handler = &rle_decode_data            },
        { .help = "Check if string is palindrome (bit method)",
         .long_name = "is-palindrome",
         .short_name = 'A',
         .has_arg = 1,
         .default_value = NULL,
         .handler = &is_palindrome_str          },
        { .help = "Check if year is leap year",
         .long_name = "leap-year",
         .short_name = 'Y',
         .has_arg = 1,
         .default_value = NULL,
         .handler = &leap_year_value            },
        { .help = "Zellers congruence (day)",
         .long_name = "zellers-day",
         .short_name = 'Z',
         .has_arg = 1,
         .default_value = NULL,
         .handler = &zellers_day                },
        { .help = "Count trailing zeros in number",
         .long_name = "count-trailing-zeros",
         .short_name = 'z',
         .has_arg = 1,
         .default_value = NULL,
         .handler = &count_trailing_zeros_value },
        { .help = "Find next power of two",
         .long_name = "next-power",
         .short_name = 'N',
         .has_arg = 1,
         .default_value = NULL,
         .handler = &next_power_value           },
        { .help = "Fisher-Yates shuffle array size",
         .long_name = "fisher-yates",
         .short_name = 'S',
         .has_arg = 1,
         .default_value = NULL,
         .handler = &fisher_yates_size          },
        { .help = "Generate pseudo random numbers by sfc32",
         .long_name = "sfc32-random",
         .short_name = 'U',
         .has_arg = 0,
         .default_value = NULL,
         .handler = &sfc32_flag                 },
        { .help = "Generate pseudo random numbers by sha1_prng",
         .long_name = "sha1-prng",
         .short_name = 'H',
         .has_arg = 0,
         .default_value = NULL,
         .handler = &sha1_prng_flag             },
    };

    struct CLIMetadata meta = { .prog_name = argv[0],
                                .description = "TheArtOfFun-C",
                                .usage_args = "[commands]",
                                .options = options,
                                .options_count = sizeof(options) / sizeof(options[0]) };

    int pos_index = parse_options(argc, argv, meta.options, meta.options_count);
    if (pos_index < 0) {
        return EXIT_FAILURE;
    }

    if (help_flag) {
        print_help(&meta);
        return EXIT_SUCCESS;
    }

    int fib_methods_count = 0;
    if (fib_value) {
        fib_methods_count++;
    }
    if (fib_interp_value) {
        fib_methods_count++;
    }
    if (fib_cache_value) {
        fib_methods_count++;
    }
    if (fib_golden_value) {
        fib_methods_count++;
    }

    if (benchmark_flag) {
        run_benchmarks();
        return EXIT_SUCCESS;
    }

    if (fib_methods_count > 1) {
        fprintf(stderr, "Error: Use only one Fibonacci conversion method\n");
        return EXIT_FAILURE;
    }
    if (basic_value && fib_methods_count > 0) {
        fprintf(stderr, "Error: Cannot combine basic and Fibonacci conversions\n");
        return EXIT_FAILURE;
    }

    if (rle_encode_data) {
        if (strpbrk(rle_encode_data, "0123456789")) {
            fprintf(stderr, "Error: RLE input must not contain digits\n");
            return EXIT_FAILURE;
        }
        char* encoded = malloc(strlen(rle_encode_data) * 3 + 1);
        if (!encoded) {
            return EXIT_FAILURE;
        }
        rle_encode(rle_encode_data, encoded);
        printf("RLE Encoded: %s\n", encoded);
        free(encoded);
        return EXIT_SUCCESS;
    }

    if (rle_decode_data) {
        char* decoded = rle_decode(rle_decode_data);
        if (!decoded) {
            fprintf(stderr, "Error: Invalid RLE data or allocation failure\n");
            return EXIT_FAILURE;
        }
        printf("RLE Decoded: %s\n", decoded);
        free(decoded);
        return EXIT_SUCCESS;
    }

    if (is_palindrome_str) {
        int result = is_palindrome_bit(is_palindrome_str);
        printf("is_palindrome_bit(\"%s\") = %s\n", is_palindrome_str, result ? "true" : "false");
        return EXIT_SUCCESS;
    }

    if (leap_year_value) {
        char* endptr;
        int year = strtol(leap_year_value, &endptr, 10);
        if (*endptr != '\0') {
            fprintf(stderr, "Error: Invalid year value\n");
            return EXIT_FAILURE;
        }
        int result = is_leap_year(year);
        printf("is_leap_year(%d) = %s\n", year, result ? "true" : "false");
        return EXIT_SUCCESS;
    }

    if (zellers_day) {
        char* endptr;
        int day = strtol(zellers_day, &endptr, 10);
        if (*endptr != '\0' || day < 1 || day > 31) {
            fprintf(stderr, "Error: Invalid day value\n");
            return EXIT_FAILURE;
        }

        int month;
        if (pos_index < argc) {
            month = strtol(argv[pos_index], &endptr, 10);
            if (*endptr != '\0' || month < 1 || month > 12) {
                fprintf(stderr, "Error: Invalid month value\n");
                return EXIT_FAILURE;
            }
            pos_index++;
        } else {
            fprintf(stderr, "Error: Missing month for zellers\n");
            return EXIT_FAILURE;
        }

        int year;
        if (pos_index < argc) {
            year = strtol(argv[pos_index], &endptr, 10);
            if (*endptr != '\0' || year < 1) {
                fprintf(stderr, "Error: Invalid year value\n");
                return EXIT_FAILURE;
            }
            pos_index++;
        } else {
            fprintf(stderr, "Error: Missing year for zellers\n");
            return EXIT_FAILURE;
        }

        int result = zellers_congruence(day, month, year);
        const char* days[] = { "Saturday", "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday" };
        printf("zellers_congruence(%d, %d, %d) = %d (%s)\n", day, month, year, result, days[result]);
        return EXIT_SUCCESS;
    }

    if (count_trailing_zeros_value) {
        char* endptr;
        unsigned int value = strtoul(count_trailing_zeros_value, &endptr, 10);
        if (*endptr != '\0') {
            fprintf(stderr, "Error: Invalid number\n");
            return EXIT_FAILURE;
        }

        int result = count_trailing_zeros(value);
        int result_kernighan = count_trailing_zeros_kernighan(value);
        printf("count_trailing_zeros(%u) = %d\n", value, result);
        printf("count_trailing_zeros_kernighan(%u) = %d\n", value, result_kernighan);
        return EXIT_SUCCESS;
    }

    if (next_power_value) {
        char* endptr;
        uint32_t value = strtoul(next_power_value, &endptr, 10);
        if (*endptr != '\0') {
            fprintf(stderr, "Error: Invalid number\n");
            return EXIT_FAILURE;
        }

        uint32_t result = next_power_of_two(value);
        printf("next_power_of_two(%u) = %u\n", value, result);
        return EXIT_SUCCESS;
    }

    if (fisher_yates_size) {
        char* endptr;
        size_t size = strtoul(fisher_yates_size, &endptr, 10);
        if (*endptr != '\0' || size == 0 || size > 1000000) {
            fprintf(stderr, "Error: Invalid array size\n");
            return EXIT_FAILURE;
        }

        uint32_t* arr = malloc(size * sizeof(uint32_t));
        for (size_t i = 0; i < size; i++) {
            arr[i] = i;
        }

        uint64_t seed = get_seed();
        fisher_yates_shuffle(arr, size, &seed);

        printf("Fisher-Yates shuffle first 10 elements: ");
        for (size_t i = 0; i < (size < 10 ? size : 10); i++) {
            printf("%u ", arr[i]);
        }
        printf("\n");

        free(arr);
        return EXIT_SUCCESS;
    }

    if (sfc32_flag) {
        uint32_t num = sfc32();
        printf("sfc32() = %u (0x%08X)\n", num, num);
        return EXIT_SUCCESS;
    }

    if (sha1_prng_flag) {
        uint32_t state[16] = { 0 };
        for (int i = 0; i < 16; i++) {
            state[i] = get_seed() + i;
        }
        uint32_t num = sha1_prng(state);
        printf("sha1_prng() = %u (0x%08X)\n", num, num);
        return EXIT_SUCCESS;
    }

    if (binary_power) {
        if (!exponent) {
            fprintf(stderr, "Error: Missing --exponent\n");
            return EXIT_FAILURE;
        }
        char* endptr;
        errno = 0;
        double number = strtod(binary_power, &endptr);
        if (errno || endptr == binary_power || *endptr != '\0' || !isfinite(number)) {
            fprintf(stderr, "Error: Invalid base value '%s'\n", binary_power);
            return EXIT_FAILURE;
        }

        errno = 0;
        unsigned long long e = strtoull(exponent, &endptr, 10);
        if (errno || exponent[0] == '-' || endptr == exponent || *endptr != '\0') {
            fprintf(stderr, "Error: Invalid exponent value '%s'\n", exponent);
            return EXIT_FAILURE;
        }

        double powered_number = binary_pow(number, e);
        printf("%.2f ** %llu = %.2f\n", number, e, powered_number);
        return EXIT_SUCCESS;
    }

    if (fast_pow_base) {
        char* endptr;
        double base = strtod(fast_pow_base, &endptr);
        if (*endptr != '\0') {
            fprintf(stderr, "Error: Invalid base value\n");
            return EXIT_FAILURE;
        }

        double exp;
        if (pos_index < argc) {
            exp = strtod(argv[pos_index], &endptr);
            if (*endptr != '\0') {
                fprintf(stderr, "Error: Invalid exponent value\n");
                return EXIT_FAILURE;
            }
            pos_index++;
        } else {
            fprintf(stderr, "Error: Missing exponent for fast-pow\n");
            return EXIT_FAILURE;
        }

        double result = fast_pow(base, exp);
        printf("fast_pow(%.2f, %.2f) = %.6f\n", base, exp, result);
        return EXIT_SUCCESS;
    }

    if (fastest_pow_base) {
        char* endptr;
        float base = strtof(fastest_pow_base, &endptr);
        if (*endptr != '\0') {
            fprintf(stderr, "Error: Invalid base value\n");
            return EXIT_FAILURE;
        }

        float exp;
        if (pos_index < argc) {
            exp = strtof(argv[pos_index], &endptr);
            if (*endptr != '\0') {
                fprintf(stderr, "Error: Invalid exponent value\n");
                return EXIT_FAILURE;
            }
            pos_index++;
        } else {
            fprintf(stderr, "Error: Missing exponent for fastest-pow\n");
            return EXIT_FAILURE;
        }

        float result = fastest_pow(base, exp);
        printf("fastest_pow(%.2f, %.2f) = %.6f\n", base, exp, result);
        return EXIT_SUCCESS;
    }

    if (fast_mod_value) {
        char* endptr;
        uint32_t value = strtoul(fast_mod_value, &endptr, 10);
        if (*endptr != '\0') {
            fprintf(stderr, "Error: Invalid value\n");
            return EXIT_FAILURE;
        }

        uint32_t mod;
        if (pos_index < argc) {
            mod = strtoul(argv[pos_index], &endptr, 10);
            if (*endptr != '\0' || mod == 0) {
                fprintf(stderr, "Error: Invalid modulus value\n");
                return EXIT_FAILURE;
            }
            pos_index++;
        } else {
            fprintf(stderr, "Error: Missing modulus for fast-mod\n");
            return EXIT_FAILURE;
        }

        uint32_t result = fast_mod(value, mod);
        printf("fast_mod(%u, %u) = %u\n", value, mod, result);
        return EXIT_SUCCESS;
    }

    if (power_of_two_value) {
        char* endptr;
        uint32_t value = strtoul(power_of_two_value, &endptr, 10);
        if (*endptr != '\0') {
            fprintf(stderr, "Error: Invalid number\n");
            return EXIT_FAILURE;
        }

        int8_t result = is_power_of_two(value);
        printf("is_power_of_two(%u) = %s\n", value, result ? "true" : "false");
        return EXIT_SUCCESS;
    }

    if (jenkins_hash_data) {
        char* endptr;
        uint32_t seed = 0;
        if (pos_index < argc) {
            seed = strtoul(argv[pos_index], &endptr, 10);
            if (*endptr != '\0') {
                seed = 0;
            } else {
                pos_index++;
            }
        }

        uint32_t hash = jenkins_hash(jenkins_hash_data, strlen(jenkins_hash_data), seed);
        printf("jenkins_hash(\"%s\", %u) = 0x%08X\n", jenkins_hash_data, seed, hash);
        return EXIT_SUCCESS;
    }

    if (jenkins_mix_flag) {
        uint32_t a = 0xdeadbeef, b = 0x12345678, c = 0x87654321;
        printf("Before jenkins_mix: a=0x%08X, b=0x%08X, c=0x%08X\n", a, b, c);
        jenkins_mix(&a, &b, &c);
        jenkins_final(&a, &b, &c);
        printf("After jenkins_mix+final: a=0x%08X, b=0x%08X, c=0x%08X\n", a, b, c);
        return EXIT_SUCCESS;
    }

    if (pcg32_flag) {
        pcg32_random_t rng = { get_seed(), 0 };
        uint32_t num = pcg32_random_r(&rng);
        printf("pcg32_random_r() = %u (0x%08X)\n", num, num);
        return EXIT_SUCCESS;
    }

    if (xor_swap_a) {
        char* endptr;
        int a = strtol(xor_swap_a, &endptr, 10);
        if (*endptr != '\0') {
            fprintf(stderr, "Error: Invalid first value\n");
            return EXIT_FAILURE;
        }

        int b;
        if (pos_index < argc) {
            b = strtol(argv[pos_index], &endptr, 10);
            if (*endptr != '\0') {
                fprintf(stderr, "Error: Invalid second value\n");
                return EXIT_FAILURE;
            }
            pos_index++;
        } else {
            fprintf(stderr, "Error: Missing second value for xor-swap\n");
            return EXIT_FAILURE;
        }

        printf("Before xor_swap: a=%d, b=%d\n", a, b);
        xor_swap(&a, &b);
        printf("After xor_swap: a=%d, b=%d\n", a, b);
        return EXIT_SUCCESS;
    }

    if (div3_value) {
        char* endptr;
        uint32_t value = strtoul(div3_value, &endptr, 10);
        if (*endptr != '\0') {
            fprintf(stderr, "Error: Invalid number\n");
            return EXIT_FAILURE;
        }

        uint32_t result = div3(value);
        printf("div3(%u) = %u\n", value, result);
        return EXIT_SUCCESS;
    }

    if (xorshift_flag) {
        uint64_t seed = get_seed();
        uint64_t num = xorshift64(&seed);
        uint64_t min_num = 10;
        uint64_t max_num = 100;
        uint64_t ranged_num = rand_range(&seed, min_num, max_num);
        printf("xorshift64 random number: %lu\n", num);
        printf("xorshift64 random num from 10 to 100: %lu\n", ranged_num);
        return EXIT_SUCCESS;
    }

    if (xorshift_double_flag) {
        uint64_t seed = get_seed();
        double num = rand_double(&seed);
        printf("xorshift64 double random number: %f\n", num);
        return EXIT_SUCCESS;
    }

    if (lehmer64_value_flag) {
        uint64_t number = lehmer64();

        printf("lehmer64 random number: %lu", number);
        return EXIT_SUCCESS;
    }

    if (xoshiro256pp_flag) {
        uint64_t seed = get_seed();
        xoshiro256pp_state state;
        xoshiro256pp_init(&state, seed);

        printf("xorshiro256pp random number: %lu\n", xoshiro256pp_next(&state));
        return EXIT_SUCCESS;
    }

    if (q_rsqrt_quake) {
        char* endptr;
        float number = strtof(q_rsqrt_quake, &endptr);

        if (endptr == q_rsqrt_quake || *endptr != '\0') {
            fprintf(stderr, "Error: Invalid number format for Q_rsqrt\n");
            return EXIT_FAILURE;
        }

        if (number <= 0.0f) {
            fprintf(stderr, "Error: Q_rsqrt requires positive number\n");
            return EXIT_FAILURE;
        }

        float result = Q_rsqrt(number);
        printf("Q_rsqrt(%.2f) = %f\n", number, result);
        return EXIT_SUCCESS;
    }

    if (fib_value) {
        char* endptr;
        double miles = strtod(fib_value, &endptr);
        if (*endptr != '\0' || miles < 0) {
            fprintf(stderr, "Error: Invalid distance value '%s'\n", fib_value);
            return EXIT_FAILURE;
        }

        uint64_t km = fibonacci(miles + 1.0);
        printf("%.2f miles ≈ %" PRIu64 " km (Fibonacci)\n", miles, km);
        return EXIT_SUCCESS;
    }

    if (fib_interp_value) {
        char* endptr;
        float miles = strtof(fib_interp_value, &endptr);
        if (*endptr != '\0' || miles < 0) {
            fprintf(stderr, "Error: Invalid distance value '%s'\n", fib_interp_value);
            return EXIT_FAILURE;
        }

        float km = fib_interpolate(miles);
        printf("%.2f miles ≈ %.2f km (Fibonacci interpolation)\n", miles, km);
        return EXIT_SUCCESS;
    }

    if (fib_cache_value) {
        char* endptr;
        float miles = strtof(fib_cache_value, &endptr);
        if (*endptr != '\0' || miles < 0) {
            fprintf(stderr, "Error: Invalid distance value '%s'\n", fib_cache_value);
            return EXIT_FAILURE;
        }

        float km = fib_cache_convert(miles);
        printf("%.2f miles ≈ %.2f km (Cached Fibonacci)\n", miles, km);
        return EXIT_SUCCESS;
    }

    if (fib_golden_binary_value) {
        char* endptr;
        float miles = strtof(fib_golden_binary_value, &endptr);
        if (*endptr != '\0' || miles < 0) {
            fprintf(stderr, "Error: Invalid distance value '%s'\n", fib_golden_binary_value);
            return EXIT_FAILURE;
        }

        float km = fib_golden_ratio_binary(miles);
        printf("%.2f miles ≈ %.2f km (Golden Ratio with Binary Pow)\n", miles, km);
        return EXIT_SUCCESS;
    }

    if (fib_golden_value) {
        char* endptr;
        float miles = strtof(fib_golden_value, &endptr);
        if (*endptr != '\0' || miles < 0) {
            fprintf(stderr, "Error: Invalid distance value '%s'\n", fib_golden_value);
            return EXIT_FAILURE;
        }

        float km = fib_golden_ratio(miles);
        printf("%.2f miles ≈ %.2f km (Golden Ratio)\n", miles, km);
        return EXIT_SUCCESS;
    }

    if (basic_value) {
        char* endptr;
        float miles = strtod(basic_value, &endptr);
        if (*endptr != '\0' || miles < 0) {
            fprintf(stderr, "Error: Invalid distance value '%s'\n", basic_value);
            return EXIT_FAILURE;
        }

        float km = basic_miles2km(miles);
        printf("%f miles = %f km\n", miles, km);
        return EXIT_SUCCESS;
    }

    if (pos_index < argc) {
        for (int i = pos_index; i < argc; i++) {
            char* endptr;
            double miles = strtod(argv[i], &endptr);

            if (*endptr != '\0' || miles < 0) {
                fprintf(stderr, "Error: Invalid distance value '%s'. Skipping.\n", argv[i]);
                continue;
            }

            float km = basic_miles2km(miles);
            printf("%.2f miles = %.2f km\n", miles, km);
        }
        return EXIT_SUCCESS;
    }

    print_help(&meta);
    return EXIT_SUCCESS;
}
