#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define DELTA 0x9e3779b9
#define MX (((z >> 5 ^ y << 2) + (y >> 3 ^ z << 4)) ^ ((sum ^ y) + (key[(p & 3) ^ e] ^ z)))

void xxtea_encrypt(uint32_t* v, int n, uint32_t const key[4]) {
    uint32_t y, z, sum;
    unsigned p, rounds, e;
    if (n < 1) {
        return;
    }
    rounds = 6 + 52 / n;
    sum = 0;
    z = v[n - 1];
    do {
        sum += DELTA;
        e = (sum >> 2) & 3;
        for (p = 0; p < n - 1; p++) {
            y = v[p + 1];
            z = v[p] += MX;
        }
        y = v[0];
        z = v[n - 1] += MX;
    } while (--rounds);
}

void xxtea_decrypt(uint32_t* v, int n, uint32_t const key[4]) {
    uint32_t y, z, sum;
    unsigned p, rounds, e;
    if (n < 1) {
        return;
    }
    rounds = 6 + 52 / n;
    sum = rounds * DELTA;
    y = v[0];
    do {
        e = (sum >> 2) & 3;
        for (p = n - 1; p > 0; p--) {
            z = v[p - 1];
            y = v[p] -= MX;
        }
        z = v[n - 1];
        y = v[0] -= MX;
        sum -= DELTA;
    } while (--rounds);
}

int main() {
    uint32_t data[] = { 0x12345678, 0x9ABCDEF0 };
    uint32_t key[4] = { 0xA1B2C3D4, 0x5E6F7A8B, 0x9C0D1E2F, 0x3A4B5C6D };

    printf("Original:      0x%08X 0x%08X\n", data[0], data[1]);

    xxtea_encrypt(data, 2, key);
    printf("After encrypt: 0x%08X 0x%08X\n", data[0], data[1]);

    xxtea_decrypt(data, 2, key);
    printf("After decrypt: 0x%08X 0x%08X\n", data[0], data[1]);

    return 0;
}
