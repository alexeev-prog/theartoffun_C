#include <math.h>
#include <stdbool.h>
#include <stdio.h>

bool fuzzy_float_eq(float a, float b) {
    float abs_diff = fabsf(a - b);
    float max_abs = fmaxf(fabsf(a), fabsf(b));
    return abs_diff <= max_abs * 1e-6f || abs_diff < 1e-12f;
}

int main() {
    float num1 = 1.0000009901f;
    float num2 = 1.0000009902f;

    if (fuzzy_float_eq(num1, num2)) {
        printf("Числа считаются равными\n");
    } else {
        printf("Числа различны\n");
    }

    return 0;
}
