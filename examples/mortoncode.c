/** @file mortoncode.c @brief Encode coordinates and verify their round trip. */
#include "mortoncode.h"

#include <inttypes.h>
#include <stdio.h>

#include "cli_values.h"

int main(int argc, char** argv) {
    uint64_t x = 10, y = 13;
    if (argc != 1 && (argc != 3 || !cli_u64(argv[1], UINT16_MAX, &x) || !cli_u64(argv[2], UINT16_MAX, &y))) {
        fprintf(stderr, "Usage: %s [X Y] (0..65535)\n", argv[0]);
        return 1;
    }
    uint32_t code = morton_encode((uint16_t)x, (uint16_t)y);
    uint16_t dx, dy;
    morton_decode(code, &dx, &dy);
    printf("Morton code: %" PRIu32 " (0x%08" PRIX32 ")\n", code, code);
    printf("Decoded: x=%u, y=%u\n", (unsigned)dx, (unsigned)dy);
    return dx != x || dy != y;
}
