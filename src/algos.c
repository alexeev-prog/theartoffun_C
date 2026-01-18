#include "algos.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define Q_RSQRT_MAGIC_NUMBER 0x5f3759df
#define HALF_FLOAT 0.5F
#define LEHMER64_MAGIC_NUMBER 0xda942042e4dd58b5ULL
#define XORSHIRO256PP_MAGIC_FIRST_NUMBER 0xbf58476d1ce4e5b9
#define XORSHIRO256PP_MAGIC_SECOND_NUMBER 0x94d049bb133111eb
#define DIV3_MAGIC_NUMBER 0xAAAAAAABULL

#define rot(x, k) (((x) << (k)) | ((x) >> (32 - (k))))

static __uint128_t g_lehmer64_state;
uint64_t wyrand_state = 0xa55a5a5a5a5a5a5a;
uint64_t ranq1_state;

uint64_t msws_x = 0, msws_w = 0;

uint64_t romu_duo_state1 = 0x1234567890abcdef;
uint64_t romu_duo_state2 = 0xfedcba0987654321;

uint32_t sfc32_state[4] = {0x12345678, 0x9ABCDEF0, 0x13579BDF, 0x2468ACE0};

uint32_t jsf32_state[4] = {0x12345678, 0x9ABCDEF0, 0x13579BDF, 0x2468ACE0};

typedef struct {
    uint32_t a, b, c, d;
} jsf32_state_t;

static jsf32_state_t jsf32_global = {0x12345678, 0x9ABCDEF0, 0x13579BDF, 0x2468ACE0};

uint32_t jsf32() {
    jsf32_state_t* s = &jsf32_global;
    uint32_t e = s->a - ((s->b << 27) | (s->b >> 5));
    s->a = s->b ^ ((s->c << 17) | (s->c >> 15));
    s->b = s->c + s->d;
    s->c = s->d + e;
    s->d = e + s->a;
    return s->d;
}

uint64_t xorshift64(uint64_t* state) {
    uint64_t x = *state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    *state = x;
    return x;
}

uint64_t rand_range(uint64_t* state, uint64_t min, uint64_t max) {
    return min + (xorshift64(state) % (max - min + 1));
}

double rand_double(uint64_t* state) {
    return (xorshift64(state) >> 11) * (1.0 / (UINT64_C(1) << 53));
}

float Q_rsqrt(float number) {
    int32_t i;
    float x2;
    float y;
    const float THREEHALFS = 1.5F;

    x2 = number * HALF_FLOAT;
    y = number;
    i = *(int32_t*)&y;
    i = Q_RSQRT_MAGIC_NUMBER - (i >> 1);
    y = *(float*)&i;
    y = y * (THREEHALFS - (x2 * y * y));
    y = y * (THREEHALFS - (x2 * y * y));
    return y;
}

uint64_t lehmer64(void) {
    g_lehmer64_state *= LEHMER64_MAGIC_NUMBER;
    return g_lehmer64_state >> 64;
}

void lehmer64_seed(uint64_t seed) {
    g_lehmer64_state = seed;
}

static inline uint64_t rotl(const uint64_t x, int k) {
    return (x << k) | (x >> (64 - k));
}

uint64_t xoshiro256pp_next(xoshiro256pp_state* state) {
    uint64_t* s = state->s;
    uint64_t result = rotl(s[0] + s[3], 23) + s[0];

    uint64_t t = s[1] << 17;
    s[2] ^= s[0];
    s[3] ^= s[1];
    s[1] ^= s[2];
    s[0] ^= s[3];
    s[2] ^= t;
    s[3] = rotl(s[3], 45);

    return result;
}

void xoshiro256pp_init(xoshiro256pp_state* state, uint64_t seed) {
    uint64_t tmp = seed;
    for (int i = 0; i < 4; i++) {
        tmp ^= tmp >> 30;
        tmp *= XORSHIRO256PP_MAGIC_FIRST_NUMBER;
        tmp ^= tmp >> 27;
        tmp *= XORSHIRO256PP_MAGIC_SECOND_NUMBER;
        tmp ^= tmp >> 31;
        state->s[i] = tmp;
    }
}

uint32_t div3(uint32_t x) {
    return (uint32_t)(((uint64_t)x * 0xAAAAAAABULL) >> 33);
}

void xor_swap(int* a, int* b) {
    if (a != b) {
        *a ^= *b;
        *b ^= *a;
        *a ^= *b;
    }
}

int count_trailing_zeros(unsigned int x) {
    if (x == 0) {
        return sizeof(x) * 8;
    }

    int count = 0;
    while ((x & 1) == 0) {
        count++;
        x >>= 1;
    }
    return count;
}

int count_trailing_zeros_kernighan(unsigned int x) {
    if (x == 0) {
        return sizeof(x) * 8;
    }

    unsigned int lowest_set_bit = x & -x;

    int count = 0;
    while (lowest_set_bit > 1) {
        count++;
        lowest_set_bit >>= 1;
    }
    return count;
}

uint32_t pcg32_random_r(pcg32_random_t* rng) {
    uint64_t oldstate = rng->state;
    rng->state = oldstate * 6364136223846793005ULL + (rng->inc | 1);
    uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    uint32_t rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

void jenkins_mix(uint32_t* a, uint32_t* b, uint32_t* c) {
    *a -= *c;
    *a ^= rot(*c, 4);
    *c += *b;
    *b -= *a;
    *b ^= rot(*a, 6);
    *a += *c;
    *c -= *b;
    *c ^= rot(*b, 8);
    *b += *a;
    *a -= *c;
    *a ^= rot(*c, 16);
    *c += *b;
    *b -= *a;
    *b ^= rot(*a, 19);
    *a += *c;
    *c -= *b;
    *c ^= rot(*b, 4);
    *b += *a;
}

void jenkins_final(uint32_t* a, uint32_t* b, uint32_t* c) {
    *c ^= *b;
    *c -= rot(*b, 14);
    *a ^= *c;
    *a -= rot(*c, 11);
    *b ^= *a;
    *b -= rot(*a, 25);
    *c ^= *b;
    *c -= rot(*b, 16);
    *a ^= *c;
    *a -= rot(*c, 4);
    *b ^= *a;
    *b -= rot(*a, 14);
    *c ^= *b;
    *c -= rot(*b, 24);
}

uint32_t jenkins_hash(const void* key, size_t length, uint32_t initval) {
    uint32_t a, b, c;
    const uint8_t* k = (const uint8_t*)key;
    uint32_t* blocks = (uint32_t*)k;

    a = b = c = 0xdeadbeef + ((uint32_t)length) + initval;

    size_t len = length;
    while (len >= 12) {
        a += blocks[0];
        b += blocks[1];
        c += blocks[2];
        jenkins_mix(&a, &b, &c);
        blocks += 3;
        len -= 12;
    }

    uint8_t* tail = (uint8_t*)blocks;
    switch (len) {
        case 11:
            c += ((uint32_t)tail[10]) << 16;
        case 10:
            c += ((uint32_t)tail[9]) << 8;
        case 9:
            c += tail[8];
        case 8:
            b += ((uint32_t)tail[7]) << 24;
        case 7:
            b += ((uint32_t)tail[6]) << 16;
        case 6:
            b += ((uint32_t)tail[5]) << 8;
        case 5:
            b += tail[4];
        case 4:
            a += ((uint32_t)tail[3]) << 24;
        case 3:
            a += ((uint32_t)tail[2]) << 16;
        case 2:
            a += ((uint32_t)tail[1]) << 8;
        case 1:
            a += tail[0];
    }

    jenkins_final(&a, &b, &c);

    return c;
}

uint32_t fast_mod(uint32_t x, uint32_t mod) {
    return x & (mod - 1);
}

uint32_t isqrt(uint32_t x) {
    uint32_t res = 0;
    uint32_t bit = 1 << 30;

    while (bit > x) {
        bit >>= 2;
    }

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

uint32_t to_gray(uint32_t n) {
    return n ^ (n >> 1);
}

uint32_t from_gray(uint32_t gray) {
    gray ^= (gray >> 16);
    gray ^= (gray >> 8);
    gray ^= (gray >> 4);
    gray ^= (gray >> 2);
    gray ^= (gray >> 1);
    return gray;
}

uint64_t wyrand() {
    wyrand_state += 0xa0761d6478bd642f;
    __uint128_t t = (__uint128_t)wyrand_state * (wyrand_state ^ 0xe7037ed1a0b428db);
    return (t >> 64) ^ t;
}

void counting_sort_256(uint8_t* arr, size_t n) {
    size_t count[256] = {0};

    for (size_t i = 0; i < n; i++) {
        count[arr[i]]++;
    }

    size_t idx = 0;
    for (size_t i = 0; i < 256; i++) {
        while (count[i]--) {
            arr[idx++] = i;
        }
    }
}

uint32_t msws32() {
    msws_x *= msws_x;
    msws_x += (msws_w += 0xb5ad4eceda1ce2a9);
    return (msws_x = (msws_x >> 32)) | (msws_x << 32);
}

uint64_t romu_duo() {
    uint64_t xp = romu_duo_state1;
    romu_duo_state1 = 15241094284759029579u * romu_duo_state2;
    romu_duo_state2 = romu_duo_state2 - xp;
    romu_duo_state2 = (romu_duo_state2 << 32) | (romu_duo_state2 >> 32);
    return xp;
}

int zellers_congruence(int day, int month, int year) {
    if (month < 3) {
        month += 12;
        year--;
    }
    int k = year % 100;
    int j = year / 100;
    return (day + 13 * (month + 1) / 5 + k + k / 4 + j / 4 + 5 * j) % 7;
}

int is_leap_year(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int is_palindrome_bit(const char* str) {
    uint64_t mask = 0;
    for (const char* p = str; *p; p++) {
        mask ^= (1ULL << (*p - 'a'));
    }
    return (mask & (mask - 1)) == 0;
}

uint32_t sha1_prng(uint32_t* state) {
    uint32_t a = state[0];
    uint32_t b = state[1];
    uint32_t c = state[2];
    uint32_t d = state[3];
    uint32_t e = state[4];

    for (int i = 0; i < 20; i++) {
        uint32_t f = (b & c) | ((~b) & d);
        uint32_t temp = ((a << 5) | (a >> 27)) + f + e + 0x5A827999 + state[i % 16];
        e = d;
        d = c;
        c = (b << 30) | (b >> 2);
        b = a;
        a = temp;
    }

    state[0] = a;
    state[1] = b;
    state[2] = c;
    state[3] = d;
    state[4] = e;
    return a ^ b ^ c ^ d ^ e;
}

uint32_t next_power_of_two(uint32_t x) {
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x + 1;
}

void fisher_yates_shuffle(uint32_t* arr, size_t n, uint64_t* seed) {
    for (size_t i = n - 1; i > 0; i--) {
        size_t j = xorshift64(seed) % (i + 1);
        uint32_t temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

uint32_t sfc32() {
    uint32_t result = sfc32_state[0] + sfc32_state[1] + sfc32_state[3]++;
    sfc32_state[0] = sfc32_state[1] ^ (sfc32_state[1] >> 9);
    sfc32_state[1] = sfc32_state[2] + (sfc32_state[2] << 3);
    sfc32_state[2] = ((sfc32_state[2] << 21) | (sfc32_state[2] >> 11)) + result;
    return result;
}

void tinymt32_init(tinymt32_t* tmt, uint32_t seed) {
    tmt->state[0] = seed;
    tmt->state[1] = 0x8f7011ee;
    tmt->state[2] = 0xfc78ff1f;
    tmt->state[3] = 0x3793fdff;

    for (int i = 1; i < 8; i++) {
        tmt->state[i & 3] ^= i + 1812433253 * (tmt->state[(i - 1) & 3] ^ (tmt->state[(i - 1) & 3] >> 30));
    }
}

uint32_t tinymt32_generate(tinymt32_t* tmt) {
    uint32_t x = (tmt->state[0] & 0x7fffffff) ^ tmt->state[1] ^ tmt->state[2];
    x ^= x << 1;
    tmt->state[0] = tmt->state[1];
    tmt->state[1] = tmt->state[2];
    tmt->state[2] = tmt->state[3] ^ (x >> 1);
    tmt->state[3] = x;
    return tmt->state[3];
}

uint32_t fletcher32(const uint16_t* data, size_t len) {
    uint32_t sum1 = 0xffff;
    uint32_t sum2 = 0xffff;

    while (len) {
        size_t tlen = len > 360 ? 360 : len;
        len -= tlen;

        do {
            sum1 += *data++;
            sum2 += sum1;
        } while (--tlen);

        sum1 = (sum1 & 0xffff) + (sum1 >> 16);
        sum2 = (sum2 & 0xffff) + (sum2 >> 16);
    }

    sum1 = (sum1 & 0xffff) + (sum1 >> 16);
    sum2 = (sum2 & 0xffff) + (sum2 >> 16);
    return sum2 << 16 | sum1;
}

uint32_t fletcher32_string(const char* str) {
    size_t len = strlen(str);
    size_t padded_len = (len + 1) / 2;
    uint16_t* data = (uint16_t*)str;
    return fletcher32(data, padded_len);
}

void print_hex(const uint8_t* data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        printf("%02x ", data[i]);
    }
    printf("\n");
}

uint32_t fnv1a_hash(const void* data, size_t len) {
    const uint8_t* bytes = (const uint8_t*)data;
    uint32_t hash = 2166136261u;

    for (size_t i = 0; i < len; i++) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

uint32_t reverse_bits(uint32_t x) {
    x = ((x >> 1) & 0x55555555) | ((x & 0x55555555) << 1);
    x = ((x >> 2) & 0x33333333) | ((x & 0x33333333) << 2);
    x = ((x >> 4) & 0x0F0F0F0F) | ((x & 0x0F0F0F0F) << 4);
    x = ((x >> 8) & 0x00FF00FF) | ((x & 0x00FF00FF) << 8);
    x = (x >> 16) | (x << 16);
    return x;
}

uint8_t micro_rand(uint8_t* state) {
    *state = 29 * (*state) + 217;
    return *state;
}

void rc4_init(RC4_ctx* ctx, const uint8_t* key, int key_len) {
    for (int i = 0; i < 256; i++) {
        ctx->S[i] = i;
    }

    int j = 0;
    for (int i = 0; i < 256; i++) {
        j = (j + ctx->S[i] + key[i % key_len]) & 0xFF;
        uint8_t tmp = ctx->S[i];
        ctx->S[i] = ctx->S[j];
        ctx->S[j] = tmp;
    }
    ctx->i = ctx->j = 0;
}

uint8_t rc4_byte(RC4_ctx* ctx) {
    ctx->i = (ctx->i + 1) & 0xFF;
    ctx->j = (ctx->j + ctx->S[ctx->i]) & 0xFF;

    uint8_t tmp = ctx->S[ctx->i];
    ctx->S[ctx->i] = ctx->S[ctx->j];
    ctx->S[ctx->j] = tmp;

    return ctx->S[(ctx->S[ctx->i] + ctx->S[ctx->j]) & 0xFF];
}

void xor_list_add(xor_node* prev, xor_node* node, xor_node* next) {
    node->link = (uintptr_t)prev ^ (uintptr_t)next;
    prev->link ^= (uintptr_t)next ^ (uintptr_t)node;
    next->link ^= (uintptr_t)prev ^ (uintptr_t)node;
}

xor_node* xor_list_next(xor_node* prev, xor_node* current) {
    return (xor_node*)(current->link ^ (uintptr_t)prev);
}

uint32_t mulberry32(uint32_t* state) {
    uint32_t z = (*state += 0x6D2B79F5);
    z = (z ^ (z >> 15)) * (z | 1);
    z ^= z + (z ^ (z >> 7)) * (z | 1);
    return z ^ (z >> 14);
}

uint64_t ranq1() {
    ranq1_state ^= ranq1_state >> 21;
    ranq1_state ^= ranq1_state << 35;
    ranq1_state ^= ranq1_state >> 4;
    return ranq1_state * 2685821657736338717ULL;
}

static inline uint64_t rotl64(uint64_t x, int8_t r) {
    return (x << r) | (x >> (64 - r));
}

static inline uint64_t fmix64(uint64_t k) {
    k ^= k >> 33;
    k *= 0xff51afd7ed558ccdULL;
    k ^= k >> 33;
    k *= 0xc4ceb9fe1a85ec53ULL;
    k ^= k >> 33;
    return k;
}

void murmur3_prng_init(murmur3_prng_t* prng, uint64_t seed) {
    prng->seed = seed;
    prng->counter = 0;
}

uint64_t murmur3_prng_next(murmur3_prng_t* prng) {
    uint64_t h1 = prng->seed;
    uint64_t k1 = prng->counter++;

    const uint64_t c1 = 0x87c37b91114253d5ULL;
    const uint64_t c2 = 0x4cf5ad432745937fULL;

    k1 *= c1;
    k1 = rotl64(k1, 31);
    k1 *= c2;

    h1 ^= k1;
    h1 = rotl64(h1, 27);
    h1 = h1 * 5 + 0x52dce729;

    h1 ^= 4;
    h1 = fmix64(h1);

    return h1;
}

double calculate_pi_leibniz(long long iterations) {
    double pi = 1.0;
    long long i;
    int sign = -1;

#pragma omp parallel for reduction(+ : pi) private(sign)
    for (i = 1; i < iterations; i++) {
        sign = (i % 2 == 0) ? 1 : -1;
        pi += sign / (2.0 * i + 1.0);
    }

    return pi * 4;
}
