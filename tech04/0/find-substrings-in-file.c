#include <stdio.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char* argv[]) {
    if (argc != 3)
        return 1;

    char* little = argv[2];
    int lsize = strlen(little);
    if (lsize == 0)
        return 0;

    const int fd = open(argv[1], O_RDWR);
    if (fd == -1)
        return 2;

    struct stat file_stat = {};
    if (fstat(fd, &file_stat) == -1) {
        close(fd);
        return 4;
    }

    if (file_stat.st_size == 0) {
        close(fd);
        return 0;
    }

    char* big = mmap(
            NULL,
            file_stat.st_size + 1,
            PROT_READ | PROT_WRITE,
            MAP_PRIVATE,
            fd,
            0
        );

    close(fd);

    if (big == MAP_FAILED)
        return 5;

    *(big + file_stat.st_size) = 0;

    char* current = big;
    while ((current = strstr(current, little)) != NULL) {
        printf("%ld\n", current - big);
        ++current;
    }

    munmap(big, file_stat.st_size + 1);

    return 0;
}
