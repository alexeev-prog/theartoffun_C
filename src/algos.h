#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <stdint.h>
#include <stdlib.h>

typedef struct {
    uint64_t state;
    uint64_t inc;
} pcg32_random_t;

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
 * @brief A quick check for the power of two
 *
 * @param x
 * @return int8_t
 **/
int8_t is_power_of_two(uint32_t x);

/**
 * @brief Fast module 2^n
 *
 * @param x
 * @param mod
 * @return uint32_t
 **/
uint32_t fast_mod(uint32_t x, uint32_t mod);

#endif
