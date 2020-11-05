#include <sys/wait.h>
#include <unistd.h>

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
        dup2(out_fd, STDOUT_FILENO);

        // because pipefd duplicated
        // and we need to close it
        // or second process
        // will wait infinitely
        close(to_close_fd[0]);
        close(to_close_fd[1]);

        execvp(pathname, argv);
        exit(-1);
    }

    return pid;
}

int main(int argc, char* argv[])
{
    if (argc != 3)
        return 1;

    int pipefd[2];
    if (pipe(pipefd) == -1)
        return 2;

    char* cmd1 = argv[1];
    char* cmd2 = argv[2];

    char* argv1[] = {cmd1, NULL};
    pid_t pid1 = subprocess(cmd1, argv1, STDIN_FILENO, pipefd[1], pipefd);
    char* argv2[] = {cmd2, NULL};
    pid_t pid2 = subprocess(cmd2, argv2, pipefd[0], STDOUT_FILENO, pipefd);

    int status = 0;
    close(pipefd[0]);
    close(pipefd[1]);
    waitpid(pid1, &status, 0);
    waitpid(pid2, &status, 0);
}
