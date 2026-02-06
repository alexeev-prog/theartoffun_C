#include <stdint.h>
#include <stdio.h>
#include <time.h>

typedef struct {
    uint32_t a, b, c, d;
} TycheState;

uint32_t tyche_next(TycheState* s) {
    uint32_t t = s->b;
    s->b = s->c;
    s->c = s->d;
    s->a = (s->a ^ (s->a << 11)) ^ (t ^ (t >> 19));
    s->d = s->a ^ s->b ^ s->c;
    return s->d;
}

void tyche_init(TycheState* s, uint32_t seed) {
    s->a = 0x6C078965 * (seed ^ (seed >> 30)) + 1;
    s->b = s->a + 0x6C078965;
    s->c = s->b + 0x6C078965;
    s->d = s->c + 0x6C078965;

    for (int i = 0; i < 20; i++) {
        tyche_next(s);
    }
}

int main() {
    TycheState rng;
    tyche_init(&rng, time(NULL));

    printf("рандомне 16битные числа:\n");
    for (int i = 0; i < 10; i++) {
        printf("0x%08X\n", tyche_next(&rng));
    }

    printf("\nВ диапазоне 0..99:\n");
    for (int i = 0; i < 10; i++) {
        printf("%d ", tyche_next(&rng) % 100);
    }
    printf("\n");

    return 0;
}
