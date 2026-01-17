#include <stdint.h>

uint32_t morton_encode(uint16_t x, uint16_t y);

void morton_decode(uint32_t code, uint16_t *x, uint16_t *y);
