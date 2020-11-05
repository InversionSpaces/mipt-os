#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

#include <stdio.h>

int main(int argc, char* argv[])
{
    if (argc != 3)
        return 1;

    char* cmd = argv[1];
    char* in = argv[2];

    int fd = open(in, O_RDONLY);
    if (fd == -1)
        return 2;

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        close(fd);
        return 3;
    }

    pid_t pid = fork();
    if (pid == -1) {
        close(fd);
        return 4;
    }

    if (pid == 0) {
        dup2(fd, STDIN_FILENO);
        dup2(pipefd[1], STDOUT_FILENO);
        execlp(cmd, cmd, NULL);
        perror("exec error");
        return 5;
    }

    int status = 0;
    wait(&status);

    close(fd);
    close(pipefd[1]);

    char buffer[1024];
    int len = 0, total = 0;
    while ((len = read(pipefd[0], buffer, sizeof(buffer))) > 0)
        total += len;

    close(pipefd[0]);

    printf("%d", total);

    return 0;
}
