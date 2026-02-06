#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

uint64_t gjf_state[2];

uint64_t gjf64(void) {
    uint64_t s0 = gjf_state[0];
    uint64_t s1 = gjf_state[1];
    gjf_state[0] = s1;
    s0 ^= s0 << 23;
    gjf_state[1] = s0 ^ s1 ^ (s0 >> 17) ^ (s1 >> 26);
    return gjf_state[1] + s1;
}

void gjf_seed(uint64_t seed1, uint64_t seed2) {
    gjf_state[0] = seed1;
    gjf_state[1] = seed2;
    for (int i = 0; i < 10; i++) {
        gjf64();
    }
}

int main() {
    uint64_t seed1 = (uint64_t)time(NULL);
    uint64_t seed2 = (uint64_t)getpid() ^ (seed1 << 32);
    gjf_seed(seed1, seed2);

    printf("Seed: %llu, %llu\n", seed1, seed2);
    for (int i = 0; i < 5; i++) {
        printf("%016llx\n", gjf64());
    }
    return 0;
}
