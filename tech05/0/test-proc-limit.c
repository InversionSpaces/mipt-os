#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>

int main() {
    unsigned long count = 1;
    pid_t pid = 0;
    while ((pid = fork()) != -1) {
        if (pid == 0) return 0;
        count++;
    }

    int status = 0;
    for (int i = 0; i < count; ++i)
        wait(&status);

    printf("%d", count);
}
