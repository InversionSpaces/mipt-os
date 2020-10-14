#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <limits.h>

typedef enum {
    ERROR,
    VALID,
    INVALID
} CHECK_RESULT;

CHECK_RESULT is_invalid_file(const int fd) {
    struct stat file_stat = {};
    if (fstat(fd, &file_stat) == -1)
        return ERROR;

    if (!(S_IXUSR & file_stat.st_mode))
        return VALID;

    char buffer[PATH_MAX]; // MAX_PATH
    int readed = read(fd, buffer, sizeof(buffer));
    if (readed == -1)
        return ERROR;

    const char ELF[] = {0x7f, 'E', 'L', 'F'};
    if (    (size_t)readed >= sizeof(ELF) &&
            (memcmp(buffer, ELF, sizeof(ELF)) == 0) )
        return VALID;

    if (readed > 2 && buffer[0] == '#' && buffer[1] == '!') {
        for (int i = 2; i < readed; ++i)
            if (buffer[i] == '\n') {
                buffer[i] = 0;
                break;
            }

        if (lstat(buffer + 2, &file_stat) == 0
                && (S_IXUSR & file_stat.st_mode))
            return VALID;
    }

    return INVALID;
}

int main() {
    char* buffer = NULL;
    size_t buf_size = 0;

    int len = 0;
    while ((len = getline(&buffer, &buf_size, stdin)) != -1) {
        if (len == 0)
            continue;

        if (buffer[len - 1] == '\n')
            buffer[len - 1] = 0;

        int fd = open(buffer, O_RDONLY);
        if (fd == -1) {
            free(buffer);
            return 1;
        }

        switch (is_invalid_file(fd)) {
            case ERROR:
                close(fd);
                free(buffer);
                return 1;
            case INVALID:
                puts(buffer);
                break;
            case VALID:
                break;
        }

        close(fd);
    }

    free(buffer);
    return 0;
}
