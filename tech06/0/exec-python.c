#include <stdio.h>
#include <stdlib.h>

#include <sys/stat.h>
#include <unistd.h>

int main()
{
    struct stat file_stat = {};
    if (fstat(fileno(stdin), &file_stat) == -1)
        return 1;

    char* buffer = malloc(file_stat.st_size + 1);
    size_t read = 0, total = 0;
    while (
        (read = fread(
             buffer + total, sizeof(char), file_stat.st_size - total, stdin)) >
        0) {
        total += read;
    }

    if (read < 0) {
        free(buffer);
        return 2;
    }

    buffer[total] = 0;
    const size_t program_len = total + 64;
    char* program = malloc(program_len);
    snprintf(program, program_len, "print(%s)", buffer);

    execlp("python3", "python3", "-c", program, NULL);

    return 0;
}
