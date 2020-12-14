#include <arpa/inet.h>
#include <fcntl.h>
#include <limits.h>
#include <linux/limits.h>
#include <signal.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/signalfd.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>

#define QUEUE_SIZE (100)
#define HEADER_SIZE (PATH_MAX + 256)
#define BUFFER_SIZE (1024)

int answer_simple(int fd, const char* answer)
{
    write(fd, answer, strlen(answer));

    return 0;
}

int answer_content(int fd, const char* path)
{
    int file = open(path, O_RDONLY);
    int size = lseek(file, 0, SEEK_SET);
    size = lseek(file, 0, SEEK_END) - size;
    lseek(file, 0, SEEK_SET);

    char answer[size + HEADER_SIZE];
    int header = snprintf(
        answer,
        sizeof(answer),
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: %d\r\n\r\n",
        size);

    read(file, answer + header, size);
    write(fd, answer, header + size);

    return 0;
}

int answer_execution(int fd, const char* path)
{
    int pipefd[2];
    if (pipe(pipefd) == -1)
        return 1;

    pid_t pid = fork();
    if (pid == -1)
        return -1;

    if (pid == 0) {
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        execl(path, path, NULL);
        exit(1);
    }

    close(pipefd[1]);
    wait(NULL);

    char buffer[BUFFER_SIZE];
    int res = 0;
    while ((res = read(pipefd[0], buffer, sizeof(buffer))) > 0)
        write(fd, buffer, res);

    close(pipefd[0]);

    return 0;
}

int main(int argc, char* argv[])
{
    if (argc != 3)
        return 1;

    char* path_pref = argv[2];
    int len = strlen(path_pref);
    if (len && path_pref[len - 1] == '/')
        path_pref[len - 1] = 0;

    const int port = atoi(argv[1]);
    if (port == 0)
        return 1;

    struct sockaddr_in addr = {.sin_family = AF_INET, .sin_port = htons(port)};

    if (inet_aton("127.0.0.1", &addr.sin_addr) == 0)
        return 1;

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1)
        return 1;

    if (bind(socket_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        close(socket_fd);

        return 1;
    }

    if (listen(socket_fd, QUEUE_SIZE) == -1) {
        close(socket_fd);

        return 1;
    }

    int epoll_fd = epoll_create(2);
    if (epoll_fd == -1) {
        close(socket_fd);

        return 1;
    }

    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGINT);

    sigprocmask(SIG_BLOCK, &mask, NULL);
    int signal_fd = signalfd(-1, &mask, 0);
    if (signal_fd == -1) {
        close(socket_fd);
        close(epoll_fd);

        return 1;
    }

    struct epoll_event ev;
    ev.events = EPOLLIN;

    ev.data.fd = socket_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &ev) == -1) {
        close(socket_fd);
        close(signal_fd);
        close(epoll_fd);

        return 1;
    }

    ev.data.fd = signal_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, signal_fd, &ev) == -1) {
        close(socket_fd);
        close(signal_fd);
        close(epoll_fd);

        return 1;
    }

    int stop = 0;
    while (!stop) {
        struct epoll_event events[2];
        int ready = epoll_wait(epoll_fd, events, 2, -1);
        for (int i = 0; i < ready; ++i) {
            ev = events[i];

            if (ev.data.fd == signal_fd || ev.data.fd != socket_fd) {
                stop = 1;
                break;
            }

            int fd = accept(socket_fd, NULL, NULL);

            char request[HEADER_SIZE];
            read(fd, request, HEADER_SIZE);

            strtok(request, " ");
            char* fname = strtok(NULL, " ");

            if (!fname || strlen(fname) == 0) {
                shutdown(fd, SHUT_RDWR);

                continue;
            }

            if (*fname == '/')
                ++fname;

            char path[PATH_MAX];
            snprintf(path, PATH_MAX, "%s/%s", path_pref, fname);

            if (access(path, F_OK))
                answer_simple(fd, "HTTP/1.1 404 Not Found\r\n\r\n");
            else if (access(path, R_OK))
                answer_simple(fd, "HTTP/1.1 403 Forbidden\r\n\r\n");
            else if (!access(path, X_OK))
                answer_execution(fd, path);
            else
                answer_content(fd, path);

            shutdown(fd, SHUT_RDWR);
            close(fd);
        }
    }

    shutdown(socket_fd, SHUT_RDWR);
    close(socket_fd);
    close(signal_fd);
    close(epoll_fd);

    return 0;
}