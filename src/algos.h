#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <stdint.h>
#include <stdlib.h>

typedef struct {
    uint64_t state;
    uint64_t inc;
} pcg32_random_t;

typedef struct {
    uint32_t state[4];
} tinymt32_t;

/**
 * @brief xorshiro256pp generator 256bit state
 *
 **/
typedef struct {
    uint64_t s[4];
} xoshiro256pp_state;

/**
 * @brief xorshift64 pseudorandom generator
 *
 * @param state seed
 * @return uint64_t
 **/
uint64_t xorshift64(uint64_t* state);

/**
 * @brief Generate pseudorandom number from range by xorshift64
 *
 * @param state seed
 * @param min minimal num
 * @param max maximum num
 * @return uint64_t
 **/
uint64_t rand_range(uint64_t* state, uint64_t min, uint64_t max);

/**
 * @brief Generate pseudorandom double number by xorshift64
 *
 * @param state seed
 * @return double
 **/
double rand_double(uint64_t* state);

/**
 * @brief quick rsqrt from Quake III
 *
 * @param number number
 * @return float
 **/
float Q_rsqrt(float number);

/**
 * @brief Generate pseudo random number by lehmer64
 *
 * @return uint64_t
 **/
uint64_t lehmer64(void);

/**
 * @brief set lehmer64 seed
 *
 * @param seed
 **/
void lehmer64_seed(uint64_t seed);

/**
 * @brief generate next pseudo-random number by xorshiro256pp
 *
 * @param state
 * @return uint64_t
 **/
uint64_t xoshiro256pp_next(xoshiro256pp_state* state);

/**
 * @brief initialize a xorshiro256pp pseudorandom generator
 *
 * @param state xorshiro256pp 256bit state
 * @param seed seed
 **/
void xoshiro256pp_init(xoshiro256pp_state* state, uint64_t seed);

/**
 * @brief function for fast dividing by 3
 *
 * @param x
 * @return uint32_t
 **/
uint32_t div3(uint32_t x);

/**
 * @brief XOR swapping variable values without third temp var
 *
 * @param a
 * @param b
 **/
void xor_swap(int* a, int* b);

/**
 * @brief PCG32 PRNG
 *
 * @param rng
 * @return uint32_t
 **/
uint32_t pcg32_random_r(pcg32_random_t* rng);

/**
 * @brief trick of bob jenkins for fast hashing (lookup3)
 *
 * @param a
 * @param b
 * @param c
 **/
void jenkins_mix(uint32_t* a, uint32_t* b, uint32_t* c);

/**
 * @brief trick of bob jenkins for fast hashing, final part
 *
 * @param a
 * @param b
 * @param c
 **/
void jenkins_final(uint32_t* a, uint32_t* b, uint32_t* c);

/**
 * @brief jenkkins hashing function
 *
 * @param a
 * @param b
 * @param c
 **/
uint32_t jenkins_hash(const void* key, size_t length, uint32_t initval);

/**
 * @brief Fast module 2^n
 *
 * @param x
 * @param mod
 * @return uint32_t
 **/
uint32_t fast_mod(uint32_t x, uint32_t mod);

/**
 * @brief Integer square root of x
 *
 * @param x
 * @return uint32_t
 **/
uint32_t isqrt(uint32_t x);

/**
 * @brief Convert to Gray Code
 *
 * @param n
 * @return uint32_t
 **/
uint32_t to_gray(uint32_t n);

/**
 * @brief Convert from Gray Code
 *
 * @param gray
 * @return uint32_t
 **/
uint32_t from_gray(uint32_t gray);

/**
 * @brief Counting sort for arrays with small range
 *
 * @param arr
 * @param n
 **/
void counting_sort_256(uint8_t* arr, size_t n);

/**
 * @brief PRNG wyrand
 *
 * @return uint64_t
 **/
uint64_t wyrand();

/**
 * @brief PRNG msws32
 *
 * @return uint32_t
 **/
uint32_t msws32();

/**
 * @brief PRNG RomuDuo
 *
 * @return uint64_t
 **/
uint64_t romu_duo();

/**
 * @brief Count trailing zeros
 *
 * @param x
 * @return int
 **/
int count_trailing_zeros(unsigned int x);

/**
 * @brief Count trailing zeros by Kernighan
 *
 * @param x
 * @return int
 **/
int count_trailing_zeros_kernighan(unsigned int x);

/**
 * @brief Zellers formule for working with weeks
 *
 * @param day
 * @param month
 * @param year
 * @return int
 **/
int zellers_congruence(int day, int month, int year);

/**
 * @brief Check is leap year
 *
 * @param year
 * @return int
 **/
int is_leap_year(int year);

/**
 * @brief Check is palindrome with bit method
 *
 * @param str
 * @return int
 **/
int is_palindrome_bit(const char* str);

/**
 * @brief PRNG based on sha1
 *
 * @param state
 * @return uint32_t
 **/
uint32_t sha1_prng(uint32_t* state);

/**
 * @brief Get next power of two number
 *
 * @param x
 * @return uint32_t
 **/
uint32_t next_power_of_two(uint32_t x);

/**
 * @brief Fisher-Yates Shuffle
 *
 * @param arr
 * @param n
 * @param seed
 **/
void fisher_yates_shuffle(uint32_t* arr, size_t n, uint64_t* seed);

/**
 * @brief SFC PRNG
 *
 * @return uint32_t
 **/
uint32_t sfc32();

/**
 * @brief Init for TinyMT32 PRNG
 *
 * @param tmt
 * @param seed
 **/
void tinymt32_init(tinymt32_t* tmt, uint32_t seed);

/**
 * @brief Generate number by TinyMT32 PRNG
 *
 * @param tmt
 * @return uint32_t
 **/
uint32_t tinymt32_generate(tinymt32_t* tmt);

/**
 * @brief Fletcher 32 checksum-algorithm
 *
 * @param data
 * @param len
 * @return uint32_t
 **/
uint32_t fletcher32(const uint16_t* data, size_t len);

/**
 * @brief Get string checksum by fletcher32-algo
 *
 * @param str
 * @return uint32_t
 **/
uint32_t fletcher32_string(const char* str);

/**
 * @brief Print hex data
 *
 * @param data
 * @param len
 **/
void print_hex(const uint8_t* data, size_t len);

/**
 * @brief Reverse bit algorithm
 *
 * @param x
 * @return uint32_t
 **/
uint32_t reverse_bits(uint32_t x);

#endif
