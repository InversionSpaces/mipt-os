#include <signal.h>
#include <unistd.h>

#include <stdio.h>

volatile sig_atomic_t action = 0;
volatile sig_atomic_t stop = 0;

void sig_handler(int sig)
{
    if (sig == SIGTERM)
        stop = 1;
    else if (sig == SIGINT)
        action = 1;
}

int main()
{
    struct sigaction sa = {.sa_handler = sig_handler, .sa_flags = SA_RESTART};

    if (sigaction(SIGTERM, &sa, NULL) == -1)
        return 1;
    if (sigaction(SIGINT, &sa, NULL) == -1)
        return 1;

    printf("%d\n", getpid());
    fflush(stdout);

    while (stop == 0)
        pause();

    printf("%d", counter);
}
