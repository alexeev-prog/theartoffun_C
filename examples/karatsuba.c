#include <math.h>
#include <stdio.h>
#include <string.h>

int getMax(int a, int b) {
    return (a > b) ? a : b;
}

long long pow10(int e) {
    long long r = 1;
    for (int i = 0; i < e; i++) {
        r *= 10;
    }
    return r;
}

long long karatsuba(long long x, long long y) {
    if (x < 10 || y < 10) {
        return x * y;
    }

    int n = getMax((int)log10(x) + 1, (int)log10(y) + 1);
    int m = n / 2;

    long long power = (long long)pow10(m);

    long long a = x / power;
    long long b = x % power;
    long long c = y / power;
    long long d = y % power;

    long long ac = karatsuba(a, c);
    long long bd = karatsuba(b, d);
    long long abcd = karatsuba(a + b, c + d);

    return ac * pow10(2 * m) + (abcd - ac - bd) * power + bd;
}

int main() {
    long long x = 123456789;
    long long y = 987654321;
    long long result = karatsuba(x, y);
    printf("результат: %lld\n", result);
    return 0;
}
