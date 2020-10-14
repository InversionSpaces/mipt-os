#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int N = 0, W = 0;
int line_width = 0;

typedef enum {
    UP = 0,
    DOWN = 1,
    LEFT = 2,
    RIGHT = 3
} dir_t;

const dir_t next_dir[] = {RIGHT, LEFT, UP, DOWN};

char* next_address(char* current, dir_t dir) {
    switch (dir) {
        case UP:
            return current - line_width;
        case DOWN:
            return current + line_width;
        case RIGHT:
            return current + W;
        case LEFT:
            return current - W;
    }
}

void printnum(char* addr, int num) {
    char buffer[16] = {};

    snprintf(buffer, sizeof(buffer), "%*d", W, num);
    memcpy(addr, buffer, W);
}

void gen_spiral(char* file) {
    dir_t dir = RIGHT;
    char* carret = file;
    int num = 1;

    //sprintf(carret, "%*d\n", W, num);
    for (int i = 0; i < N - 1; ++i) {
        printnum(carret, num++);
        carret = next_address(carret, dir);
    }
    printnum(carret, num++);

    dir = next_dir[dir];
    carret = next_address(carret, dir);

    int length = N - 1;
    char flipper = 0;
    while (length) {
        for (int i = 0; i < length - 1; ++i) {
            printnum(carret, num++);
            carret = next_address(carret, dir);
        }
        printnum(carret, num++);

        dir = next_dir[dir];
        carret = next_address(carret, dir);

        if (flipper) {
            length -= 1;
            flipper = 0;
        }
        else flipper = 1;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4)
        return 1;

    N = atoi(argv[2]);
    W = atoi(argv[3]);

    if (!N || !W)
        return 2;

    line_width = N * W + 1;

    const int fd = open(argv[1], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1)
        return 3;

    const int size = N * line_width;
    ftruncate(fd, size);

    char* file = mmap(
            NULL,
            size,
            PROT_WRITE,
            MAP_SHARED,
            fd,
            0
        );

    close(fd);

    if (file == MAP_FAILED)
        return 4;

    gen_spiral(file);

    for (int i = 0; i < N; ++i)
        *(file + line_width * i + line_width - 1) = '\n';

    munmap(file, size);

    return 0;
}
