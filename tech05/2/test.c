#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>

int main() {
    int fd = fileno(stdin);
    printf("Begin pos: %ld\n", lseek(fd, 0, SEEK_CUR));
    pid_t pid = fork();

    if (pid == 0) {
        char buffer[4096] = {};
        int readed = scanf("%s", buffer);
        printf("Child readed: %d, pos: %ld\n", readed, lseek(fd, 0, SEEK_CUR));
        return 0;
    }

    int status = 0;
    wait(&status);
    printf("Parrent pos: %ld\n", lseek(fd, 0, SEEK_CUR));

    return 0;
}
