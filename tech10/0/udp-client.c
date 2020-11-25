#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    if (argc != 2)
        return 1;

    int port = atoi(argv[1]);
    if (port == 0)
        return 1;

    struct sockaddr_in addr = {.sin_family = AF_INET, .sin_port = htons(port)};
    if (inet_aton("127.0.0.1", &addr.sin_addr) != 1)
        return 1;

    int sfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sfd == -1)
        return 1;

    if (connect(sfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        close(sfd);
        return 1;
    }

    for (;;) {
        int num = 0;
        if (scanf("%d", &num) != 1)
            break;

        int res = write(sfd, &num, sizeof(num));
        if (res == -1)
            break;

        res = read(sfd, &num, sizeof(num));
        if (res == -1)
            break;

        printf("%d\n", num);
    }

    close(sfd);

    return 0;
}
