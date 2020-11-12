#include <signal.h>
#include <unistd.h>

#include <stdio.h>

enum { INC, NEG, STOP };

volatile sig_atomic_t action = -1;

void sig_handler(int sig)
{
    switch (sig) {
    case SIGUSR1:
        action = INC;
        break;
    case SIGUSR2:
        action = NEG;
        break;
    case SIGTERM:
        action = STOP;
        break;
    }
}

int main()
{
    struct sigaction sa = {.sa_handler = sig_handler, .sa_flags = SA_RESTART};

    if (sigaction(SIGTERM, &sa, NULL) == -1)
        return 1;
    if (sigaction(SIGUSR1, &sa, NULL) == -1)
        return 1;
    if (sigaction(SIGUSR2, &sa, NULL) == -1)
        return 1;

    int counter = 0;
    scanf("%d", &counter);
    printf("%d\n", getpid());
    fflush(stdout);

    int stop = 0;
    while (!stop) {
        pause();
        switch (action) {
        case INC:
            ++counter;
            printf("%d\n", counter);
            fflush(stdout);
            break;
        case NEG:
            counter *= -1;
            printf("%d\n", counter);
            fflush(stdout);
            break;
        case STOP:
            stop = 1;
            break;
        }
    }
}
