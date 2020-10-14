#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <limits.h>

int print_link(const char* filename) {
    char* abs_path = realpath(filename, NULL);
    if (abs_path == NULL)
        return 1;

    puts(abs_path);
    free(abs_path);

    return 0;
}

int create_link(const char* filename) {
    const char* only_name = strrchr(filename, '/');
    if (only_name == NULL)
        only_name = filename;
    else
        ++only_name;

    char buffer[PATH_MAX]; // MAX_PATH
    sprintf(buffer, "link_to_%s", only_name);

    if (symlink(filename, buffer) == -1)
        return 1;

    return 0;
}

int main() {
    char* buffer = NULL;
    size_t buffer_size = 0;

    ssize_t len = 0;
    while ((len = getline(&buffer, &buffer_size, stdin)) != -1) {
        if (len == 0)
            continue;
        if (buffer[len - 1] == '\n')
            buffer[len - 1] = 0;

        //puts(buffer);
        struct stat file_stat = {};
        if (lstat(buffer, &file_stat) == -1) {
            free(buffer);
            return 1;
        }

        if (S_ISLNK(file_stat.st_mode)) {
            //printf("Is link: %s\n", buffer);
            if (print_link(buffer) != 0) {
                free(buffer);
                return 1;
            }
        }
        else if (S_ISREG(file_stat.st_mode)) {
            //printf("Is reqular: %s\n", buffer);
            if (create_link(buffer) != 0) {
                free(buffer);
                return 1;
            }
        }
    }

    free(buffer);
    return 0;
}
