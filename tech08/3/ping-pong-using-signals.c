#include <sched.h>
#include <signal.h>
#include <unistd.h>

volatile sig_atomic_t stop = 0;

void handler(int sig, siginfo_t* info, void* context)
{
    union sigval n = info->si_value;
    if (n.sival_int == 0)
        stop = 1;
    else {
        --n.sival_int;
        sigqueue(info->si_pid, SIGRTMIN, n);
    }
}

void simple_handler(int sig)
{
    stop = 1;
}

int main()
{
    struct sigaction handle = {
        .sa_sigaction = handler, .sa_mask = 0, .sa_flags = SA_SIGINFO};

    struct sigaction simple_handle = {
        .sa_handler = simple_handler, .sa_mask = 0, .sa_flags = 0};

    sigaction(SIGTERM, &simple_handle, NULL);
    sigaction(SIGRTMIN, &handle, NULL);

    while (!stop)
        sched_yield();
}
