#include <sys/wait.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>

int subprocess(
    const char* pathname,
    char* const argv[],
    int in_fd,
    int out_fd,
    int to_close_fd[2])
{
    pid_t pid = fork();

    if (pid == -1)
        return pid;

    if (pid == 0) {
        dup2(in_fd, STDIN_FILENO);
        dup2(out_fd, STDERR_FILENO);

        // because pipefd duplicated
        // and we need to close it
        // or second process
        // will wait infinitely
        close(to_close_fd[0]);
        close(to_close_fd[1]);

        execvp(pathname, argv);
        exit(1);
    }

    return pid;
}

int main(int argc, char* argv[])
{
    if (argc != 2)
        return 1;

    int pipefd[2];
    if (pipe(pipefd) == -1)
        return 2;

    char* gcc_argv[] = {"gcc", argv[1], NULL};
    pid_t pid = subprocess("gcc", gcc_argv, STDIN_FILENO, pipefd[1], pipefd);

    close(pipefd[1]);

    int status = 0;
    waitpid(pid, &status, 0);

    if (dup2(pipefd[0], STDIN_FILENO) == -1)
        return 3;

    close(pipefd[0]);

    int last_error_line = 0;
    int last_warning_line = 0;
    int errors = 0, warnings = 0;
    for (;;) {
        int line_number = 0;
        int res = scanf("%*[^:]:%d:%*d:", &line_number);

        if (res == EOF)
            break;

        if (res == 0) {
            scanf(" %*s ");
            continue;
        }

        char c = 0;
        if (scanf(" error%c", &c) == 1 && c == ':') {
            if (line_number != last_error_line)
                ++errors;
            last_error_line = line_number;
        } else if (scanf(" warning%c", &c) == 1 && c == ':') {
            if (line_number != last_warning_line)
                ++warnings;
            last_warning_line = line_number;
        }
    }

    printf("%d %d\n", errors, warnings);
}
