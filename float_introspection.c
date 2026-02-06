#include <stdint.h>
#include <stdio.h>

typedef union {
    float f;

    struct {
        uint32_t mantissa : 23;
        uint32_t exponent : 8;
        uint32_t sign : 1;
    } bits;
} float_cast;

int main() {
    float_cast fc;
    fc.f = -3.14f;

    printf("Number: %f\n", fc.f);
    printf("Sign: %u\n", fc.bits.sign);
    printf("Exponent: %u\n", fc.bits.exponent);
    printf("Mantissa: 0x%06X\n", fc.bits.mantissa);
    return 0;
}
