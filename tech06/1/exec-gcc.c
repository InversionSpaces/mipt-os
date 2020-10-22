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
    char buffer[5 * 1024] = {};

    const int rres = read(infd, buffer, sizeof(buffer) - 1);
    if (rres == -1)
        return 1;
    buffer[rres] = 0;

    char program[1024 * 1024] = {};

    snprintf(
        program,
        sizeof(program),
        "#include <stdio.h>\n"
        "int main() {\n"
        "   int res = (%s);\n"
        "   printf(\"%%d\", res);\n"
        "   return 0;\n"
        "}\n",
        buffer);

    int srcfd = open("src.c", O_CREAT | O_EXCL | O_WRONLY, 0640);
    if (srcfd == -1)
        return 5;

    const int wres = write(srcfd, program, strlen(program));
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

    pid = fork();
    if (pid == -1)
        return 12;

    if (pid == 0) {
        execlp("./megaprog", "megaprog", NULL);
        return 13;
    }

    wait(&status);
    if (status != 0)
        return 14;

    if (unlink("megaprog") == -1)
        return 10;

    return 0;
}
