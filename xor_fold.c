#include <stdint.h>
#include <stdio.h>

uint32_t xor_fold(const uint32_t* data, size_t len) {
    uint32_t result = 0;
    for (size_t i = 0; i < len; i++) {
        result ^= data[i];
    }
    return result;
}

int main() {
    uint32_t array1[] = {0x12345678, 0x9ABCDEF0, 0x13579BDF};
    uint32_t array2[] = {0x12345678, 0x9ABCDEF0, 0x13579BDF};
    uint32_t array3[] = {0x12345678, 0x9ABCDEF0, 0x13579BDE};    // Changed

    printf("Hash array1: %08X\n", xor_fold(array1, 3));
    printf("Hash array2: %08X\n", xor_fold(array2, 3));
    printf("Hash array3: %08X\n", xor_fold(array3, 3));
    return 0;
}
