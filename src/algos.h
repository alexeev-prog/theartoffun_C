#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <stdint.h>

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

#endif
