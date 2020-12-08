#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>

#include <stdio.h>

#define BUFFER_SIZE (1024)
#define MAX_EVENTS  (10)
#define TIMEOUT     (1000)

size_t read_data_and_count(size_t N, int in[N]) {
    if (N == 0) return 0;

    int epollfd = epoll_create(N);
    if (epollfd == -1) {
        perror("epoll_create");

        return 0;
    }

    struct epoll_event ev = {
        .events = EPOLLIN
    };

    for (size_t i = 0; i < N; ++i) {
        ev.data.fd = in[i];
        if (epoll_ctl(epollfd, EPOLL_CTL_ADD, in[i], &ev) == -1) {
            perror("epoll_ctl");
            close(epollfd);    

            return 0;
        }
    }

    struct epoll_event events[MAX_EVENTS];
    char buffer[BUFFER_SIZE];
    size_t result = 0;
    size_t alive = N;

    while (alive) {
        int ready = epoll_wait(epollfd, events, MAX_EVENTS, TIMEOUT);
        if (ready == -1) {
            perror("epoll_wait");
            result = 0;

            break;
        }

        for (int i = 0; i < ready; ++i) {
            int fd = events[i].data.fd;
            int res = read(fd, buffer, BUFFER_SIZE);
            
            if (res == -1) {
                result = 0;
                alive = 0; // to stop

                break;
            }
            else if (res == 0) {
                alive -= 1;
                if (epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL) == -1) {
                    perror("epoll_ctl");
                    result = 0;
                    alive = 0; // to stop

                    break;
                }
            }
            else result += res;
        }
    }

    for (size_t i = 0; i < N; ++i)
        close(in[i]);
    
    return result;
}