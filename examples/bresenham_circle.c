#include <stdio.h>
#define WIDTH 80
#define HEIGHT 40

char buffer[HEIGHT][WIDTH];

void plot(int x, int y) {
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
        buffer[y][x] = '*';
    }
}

void clear_buffer() {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            buffer[y][x] = ' ';
        }
    }
}

void print_buffer() {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            putchar(buffer[y][x]);
        }
        putchar('\n');
    }
}

void bresenham_circle(int x0, int y0, int radius) {
    int x = 0;
    int y = radius;
    int d = 3 - 2 * radius;

    while (x <= y) {
        plot(x0 + x, y0 + y);
        plot(x0 - x, y0 + y);
        plot(x0 + x, y0 - y);
        plot(x0 - x, y0 - y);
        plot(x0 + y, y0 + x);
        plot(x0 - y, y0 + x);
        plot(x0 + y, y0 - x);
        plot(x0 - y, y0 - x);

        if (d < 0) {
            d = d + 4 * x + 6;
        } else {
            d = d + 4 * (x - y) + 10;
            y = y - 1;
        }
        x = x + 1;
    }
}

int main() {
    clear_buffer();
    bresenham_circle(WIDTH / 2, HEIGHT / 2, 15);
    print_buffer();
    return 0;
}
