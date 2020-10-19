#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main()
{
    const int infd = fileno(stdin);
    struct stat file_stat = {};
    if (fstat(infd, &file_stat) == -1)
        return 1;

    const size_t buffer_size = file_stat.st_size + 1;
    char* buffer = malloc(buffer_size);
    if (buffer == NULL)
        return 2;

    const int rres = read(infd, buffer, file_stat.st_size);
    if (rres == -1) {
        free(buffer);
        return 3;
    }

    buffer[rres] = 0;

    const size_t program_size = file_stat.st_size + 512;
    char* program = malloc(program_size);

    if (program == NULL) {
        free(buffer);
        return 4;
    }

    snprintf(
        program,
        program_size,
        "#include <stdio.h>\n"
        "int main() {\n"
        "   int res = (%s);\n"
        "   printf(\"%%d\", res);\n"
        "   return 0;\n"
        "}\n",
        buffer);
    free(buffer);

    int srcfd = open("src.c", O_CREAT | O_EXCL | O_WRONLY, 0640);
    if (srcfd == -1) {
        free(program);
        return 5;
    }

    const int wres = write(srcfd, program, strlen(program));
    free(program);
    if (wres == -1)
        return 6;

    close(srcfd);

    pid_t pid = fork();

    if (pid == -1)
        return 7;

    if (pid == 0) {
        execlp("gcc", "gcc", "src.c", "-o", "megaprog", NULL);
        return 8;
    }

    int status = 0;
    wait(&status);

    if (status != 0)
        return 9;

    if (unlink("src.c") == -1)
        return 10;

    if (chmod("./megaprog", 0750) == -1)
        return 11;

    execlp("./megaprog", "megaprog", NULL);

    return 12;
}
