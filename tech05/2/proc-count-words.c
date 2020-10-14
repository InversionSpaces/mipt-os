#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>

int main() {
    int res = setvbuf(stdin, NULL, _IONBF, 0);
    if (res != 0)
        return 1;

    int count = 0;

    while (1) {
        pid_t pid = fork();

        if (pid == -1)
            return 1;

        if (pid == 0) {
            char buffer[4096] = {};
            int readed = scanf("%s", buffer);
            if (readed == EOF)
                return 0;
            return 1;
        }

        int status = 0;
        wait(&status);
        if (status)
            count++;
        else
            break;
    }

    printf("%d", count);
}
