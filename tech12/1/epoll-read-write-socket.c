#include <arpa/inet.h>
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/signal.h>
#include <sys/signalfd.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

typedef struct {
    int fd;
    char* buffer;
    size_t size;
    size_t pos;
} data_t;

#define MAX_EVENTS (100)
#define QUEUE_SIZE (100)
#define TIMEOUT (100)
#define CHUNK_SIZE (4096 * 8)

int accept_connection(int epoll_fd, int socket_fd) {
    //printf("accepting connection\n");

    int fd = accept(socket_fd, NULL, NULL);
    if (fd == -1) return fd;

    int flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    
    data_t* fd_data = malloc(sizeof(data_t));
    fd_data->fd = fd;
    fd_data->size = CHUNK_SIZE;
    fd_data->buffer = malloc(fd_data->size);
    fd_data->pos = 0;
    
    struct epoll_event ev;
    ev.data.ptr = fd_data;
    ev.events = EPOLLIN | EPOLLRDHUP;

    return epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev);
}

int disconnect(int epoll_fd, data_t* data) {
    //printf("closing connection\n");

    int res = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, data->fd, NULL);
    
    shutdown(data->fd, SHUT_RDWR);
    close(data->fd);
    free(data->buffer);
    free(data);

    return res;
}

int update(int epoll_fd, data_t* data) {
    struct epoll_event ev;
    ev.data.ptr = data;
    ev.events = EPOLLIN | EPOLLRDHUP;
    if (data->pos > 0)
        ev.events |= EPOLLOUT;

    return epoll_ctl(epoll_fd, EPOLL_CTL_MOD, data->fd, &ev);
}

int process_read(int epoll_fd, data_t* data) {
    //printf("reading\n");

    if (data->pos == data->size) {
        data->size += CHUNK_SIZE;
        data->buffer = realloc(data->buffer, data->size);
    }

    char* read_pos = data->buffer + data->pos;
    int read_size = data->size - data->pos;

    int res = read(data->fd, read_pos, read_size);
    // if (res == -1 || res == 0) disconnect(epoll_fd, data);
    if (res > 0) {
        data->pos += res;

        char* end_pos = read_pos + res;
        for (char* pos = read_pos; pos != end_pos; ++pos)
            *pos = toupper(*pos);
    }

    return update(epoll_fd, data);
}

int process_write(int epoll_fd, data_t* data) {
    //printf("writing\n");

    int res = write(data->fd, data->buffer, data->pos);
    //if (res == -1 || res == 0) disconnect(epoll_fd, data);

    if (res > 0) {
        memmove(data->buffer, data->buffer + res, data->pos - res);
        data->pos -= res;
    }

    return update(epoll_fd, data);
}

int main(int argc, char* argv[])
{
    if (argc != 2)
        return 1;

    const int port = atoi(argv[1]);
    if (port == 0)
        return 1;

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port)
    };

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

    sigprocmask(SIG_BLOCK, &mask, NULL);
    int signal_fd = signalfd(-1, &mask, 0);
    if (signal_fd == -1) {
        close(socket_fd);
        close(epoll_fd);

        return 1;
    }

    struct epoll_event ev;
    
    data_t* sock_data = malloc(sizeof(data_t));
    sock_data->fd = socket_fd;
    sock_data->buffer = NULL;
    sock_data->pos = 0;
    sock_data->size = 0;

    ev.data.ptr = sock_data;
    ev.events = EPOLLIN;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &ev) == -1) {
        close(socket_fd);
        close(signal_fd);
        close(epoll_fd);

        return 1;       
    }

    data_t* signal_data = malloc(sizeof(data_t));
    signal_data->fd = signal_fd;
    signal_data->buffer = NULL;
    signal_data->pos = 0;
    signal_data->size = 0;

    ev.data.ptr = signal_data;
    ev.events = EPOLLIN;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, signal_fd, &ev) == -1) {
        close(socket_fd);
        close(signal_fd);
        close(epoll_fd);

        return 1;       
    }

    int stop = 0;
    while (!stop) {
        struct epoll_event events[MAX_EVENTS];
        int ready = epoll_wait(epoll_fd, events, MAX_EVENTS, TIMEOUT);
        
        for (int i = 0; i < ready; ++i) {
            ev = events[i];
            data_t* data = (data_t*)ev.data.ptr;
            
            if (data->fd == signal_fd) {
                //printf("shutting down\n");
                stop = 1; // TODO
                break;
            }
            else if (data->fd == socket_fd) accept_connection(epoll_fd, socket_fd);
            else if (ev.events & EPOLLRDHUP) disconnect(epoll_fd, data);
            else {
                if (ev.events & EPOLLIN) process_read(epoll_fd, data);
                if (ev.events & EPOLLOUT) process_write(epoll_fd, data);
            }
        }
    }

    close(signal_fd);
    close(epoll_fd);
    shutdown(socket_fd, SHUT_RDWR);
    close(socket_fd);

    return 0;
}