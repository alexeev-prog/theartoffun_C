#include <stdio.h>
#include <stdint.h>

static uint32_t split_bits(uint16_t x) {
    x = (x | (x << 8)) & 0x00FF00FF;
    x = (x | (x << 4)) & 0x0F0F0F0F;
    x = (x | (x << 2)) & 0x33333333;
    x = (x | (x << 1)) & 0x55555555;
    return x;
}

uint32_t morton_encode(uint16_t x, uint16_t y) {
    return split_bits(x) | (split_bits(y) << 1);
}

void morton_decode(uint32_t code, uint16_t *x, uint16_t *y) {
    uint32_t x_code = code & 0x55555555;
    uint32_t y_code = code & 0xAAAAAAAA;

    x_code = (x_code | (x_code >> 1)) & 0x33333333;
    x_code = (x_code | (x_code >> 2)) & 0x0F0F0F0F;
    x_code = (x_code | (x_code >> 4)) & 0x00FF00FF;
    x_code = (x_code | (x_code >> 8)) & 0x0000FFFF;

    y_code = (y_code | (y_code >> 1)) & 0x33333333;
    y_code = (y_code | (y_code >> 2)) & 0x0F0F0F0F;
    y_code = (y_code | (y_code >> 4)) & 0x00FF00FF;
    y_code = (y_code | (y_code >> 8)) & 0x0000FFFF;

    *x = (uint16_t)x_code;
    *y = (uint16_t)(y_code >> 1);
}
