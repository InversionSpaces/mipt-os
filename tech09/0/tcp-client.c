#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    if (argc != 3)
        return 1;

    const int port = atoi(argv[2]);
    if (port == 0)
        return 1;

    struct sockaddr_in addr = {.sin_family = AF_INET, .sin_port = htons(port)};

    if (inet_aton(argv[1], &addr.sin_addr) == 0)
        return 1;

    int sfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sfd == -1)
        return 1;

    if (connect(sfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        close(sfd);
        return 1;
    }

    for (;;) {
        int num = 0;

#define ABORT                                                                  \
    {                                                                          \
        shutdown(sfd, SHUT_RDWR);                                              \
        close(sfd);                                                            \
        return 1;                                                              \
    }

        if (scanf("%d", &num) == EOF)
            break;

        if (write(sfd, &num, sizeof(num)) == -1)
            ABORT

        int ret = read(sfd, &num, sizeof(num));
        if (ret <= 0)
            break;

        printf("%d\n", num);
        fflush(stdout);
#undef ABORT
    }

    shutdown(sfd, SHUT_RDWR);
    close(sfd);

    return 0;
}
