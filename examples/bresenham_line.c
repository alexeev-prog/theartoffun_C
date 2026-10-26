#include <stdio.h>
#include <stdlib.h>

#define WIDTH 20
#define HEIGHT 20

char buffer[HEIGHT][WIDTH];

void clear_buffer() {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            buffer[y][x] = ' ';
        }
    }
}

void plot(int x, int y) {
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
        buffer[y][x] = '*';
    }
}

void bresenham_line(int x0, int y0, int x1, int y1) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        plot(x0, y0);
        if (x0 == x1 && y0 == y1) {
            break;
        }
        int e2 = err * 2;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void print_buffer() {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            printf("%c", buffer[y][x]);
        }
        printf("\n");
    }
}

int main() {
    clear_buffer();
    bresenham_line(2, 3, 15, 10);
    bresenham_line(0, 19, 10, 5);
    print_buffer();
    return 0;
}
