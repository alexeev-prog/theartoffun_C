#include "algos.h"
#include "fib_algos.h"
#include "mortoncode.h"
#include "pow_algos.h"
#include "check.h"

#include <limits.h>
#include <math.h>
#include <string.h>

int main(void) {
    CHECK(binary_pow(2, 10) == 1024);
    CHECK(binary_pow(-2, 3) == -8);
    CHECK(binary_pow(0, 0) == 1);
    CHECK(fibonacci(0) == 0);
    CHECK(fibonacci(10) == 55);
    CHECK(fibonacci(93) == UINT64_C(12200160415121876738));
    CHECK(fabsf(fib_golden_ratio_binary(0.1f) - basic_miles2km(0.1f)) < 1e-6f);
    CHECK(fabsf(Q_rsqrt(25) - 0.2f) < 1e-5f);
    CHECK(count_trailing_zeros(0) == (int)(sizeof(unsigned) * CHAR_BIT));
    CHECK(is_power_of_two(0) == 0);
    CHECK(is_power_of_two(UINT32_C(1) << 31));
    CHECK(next_power_of_two(UINT32_MAX) == 0);

    uint64_t state = 42;
    for (unsigned i = 0; i < 10000; i++) {
        uint32_t value = (uint32_t)xorshift64(&state);
        CHECK(div3(value) == value / 3);
        CHECK(from_gray(to_gray(value)) == value);
        CHECK(reverse_bits(reverse_bits(value)) == value);
        CHECK(count_trailing_zeros(value) == count_trailing_zeros_kernighan(value));
        uint32_t root = isqrt(value);
        CHECK((uint64_t)root * root <= value);
        CHECK((uint64_t)(root + 1) * (root + 1) > value);
        uint16_t x = (uint16_t)value, y = (uint16_t)(value >> 16), dx, dy;
        morton_decode(morton_encode(x, y), &dx, &dy);
        CHECK(dx == x && dy == y);
        double random = rand_double(&state);
        CHECK(random >= 0 && random < 1);
        uint64_t ranged = rand_range(&state, 10, 20);
        CHECK(ranged >= 10 && ranged <= 20);
    }
    CHECK(morton_encode(UINT16_MAX, UINT16_MAX) == UINT32_MAX);
    (void)rand_range(&state, 0, UINT64_MAX);
    fisher_yates_shuffle(NULL, 0, &state);
    uint32_t items[] = {0, 1, 2, 3, 4};
    fisher_yates_shuffle(items, 5, &state);
    unsigned seen = 0;
    for (size_t i = 0; i < 5; i++) {
        CHECK(items[i] < 5);
        seen |= 1u << items[i];
    }
    CHECK(seen == 31);
    uint8_t bytes[] = {255, 0, 2, 2, 1};
    counting_sort_256(bytes, sizeof(bytes));
    CHECK(memcmp(bytes, (uint8_t[]){0, 1, 2, 2, 255}, 5) == 0);
    counting_sort_256(NULL, 0);

    xoshiro256pp_state rng;
    xoshiro256pp_init(&rng, 0);
    CHECK((rng.s[0] | rng.s[1] | rng.s[2] | rng.s[3]) != 0);
    CHECK(fnv1a_hash("hello", 5) == UINT32_C(0x4f9f2cab));
    const char unaligned[] = "xabcdefghijklmnop";
    CHECK(jenkins_hash(unaligned + 1, 16, 0) == jenkins_hash("abcdefghijklmnop", 16, 0));
    char odd[] = {'a', 'b', 'c', '\0'};
    uint16_t words[] = {0x6261, 0x0063};
    CHECK(fletcher32_string(odd) == fletcher32(words, 2));
    CHECK(is_palindrome_bit("carrace"));
    CHECK(!is_palindrome_bit("A"));
    CHECK(is_leap_year(2000) && !is_leap_year(1900));
    uint32_t msws_first = msws32();
    CHECK(msws_first == UINT32_C(0xb5ad4ece));
    CHECK(msws32() != msws_first);
    puts("Algorithm unit/property tests passed");
    return 0;
}
