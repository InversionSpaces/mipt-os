#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

int main() {
    char* buffer = NULL;
    size_t buf_size = 0;
    off_t res_size = 0;

    int len = 0;
    while ((len = getline(&buffer, &buf_size, stdin)) != -1) {
        if (len == 0)
            continue;

        if (buffer[len - 1] == '\n')
            buffer[len - 1] = 0;

        struct stat file_stat = {};
        if (lstat(buffer, &file_stat) == -1) {
            free(buffer);
            return 1;
        }

        if (S_ISREG(file_stat.st_mode))
            res_size += file_stat.st_size;
    }

    free(buffer);
    printf("%ld", res_size);
}
