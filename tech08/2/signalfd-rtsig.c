#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

volatile sig_atomic_t cmd = 0;

void handler(int sig)
{
    cmd = sig - SIGRTMIN;
}

int main(int argc, char* argv[])
{
    struct sigaction handle = {
        .sa_handler = handler, .sa_mask = (0), .sa_flags = SA_RESTART};

    for (int i = 0; i < argc; ++i)
        sigaction(SIGRTMIN + i, &handle, NULL);

    for (int i = argc; i <= SIGRTMAX - SIGRTMIN; ++i)
        signal(SIGRTMIN + i, SIG_IGN);

    while (1) {
        pause();
        sig_atomic_t cur_cmd = cmd;
        if (cur_cmd == 0)
            return 0;

        FILE* file = fopen(argv[cur_cmd], "r");
        if (file == NULL)
            return 1;

        char* line = NULL;
        size_t n = 0;

        getline(&line, &n, file);
        char* newline = strchr(line, '\n');
        if (newline)
            *newline = 0;
        puts(line);
        fflush(stdout);

        free(line);
        fclose(file);
    }
}
