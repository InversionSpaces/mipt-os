#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>

int main() {
    pid_t pid = fork();

    if (pid == -1)
        return 1;

    if (pid == 0) {
        while (1) {
            char buffer[4096];
            int read = scanf("%s", buffer);

            if (read == EOF)
                return 0;

            pid_t pid = fork();

            if (pid == -1)
                return 0;

            if (pid == 0)
                continue;

            int status = 0;
            wait(&status);

            return (WEXITSTATUS(status) + 1);
        }
    }

    int status = 0;
    wait(&status);

    printf("%d", WEXITSTATUS(status));
}
