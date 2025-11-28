#include <stdio.h>

void infinite_loop(int n) {
    printf("%d\n", n);
    infinite_loop(n + 1);
}

int main() {
    infinite_loop(1);
    return 0;
}
