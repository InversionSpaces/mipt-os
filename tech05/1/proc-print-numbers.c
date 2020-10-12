#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    if (argc != 2)
        return 1;

    int N = atoi(argv[1]);
    int start = N;
    while (N > 1) {
        pid_t pid = fork();
        if (pid == -1)
            return 1;

        if (pid == 0) {
            N -= 1;
            continue;
        }

        int status = 0;
        wait(&status);

        if (status == 1)
            return status;

        break;
    }

    if (N == start)
        printf("%d\n", N);
    else
        printf("%d ", N);

    return 0;
}
