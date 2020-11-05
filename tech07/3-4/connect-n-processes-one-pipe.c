#include <sys/wait.h>
#include <unistd.h>

#include <stdlib.h>

int subprocess(char* pathname, int in_fd, int out_fd, int to_close_fd[2])
{
    pid_t pid = fork();

    if (pid == -1)
        return pid;

    if (pid == 0) {
        dup2(in_fd, STDIN_FILENO);
        dup2(out_fd, STDOUT_FILENO);

        // because pipefd duplicated
        // and we need to close it
        // or second process
        // will wait infinitely
        close(to_close_fd[0]);
        close(to_close_fd[1]);

        char* argv[] = {pathname, NULL};
        execvp(pathname, argv);
        exit(-1);
    }

    return pid;
}

pid_t piped_subprocess(char* cmd, int pipe_in[2], int pipe_out[2], int prev_pid)
{
    pid_t retval = subprocess(cmd, pipe_in[0], pipe_out[1], pipe_in);

    close(pipe_in[0]);
    close(pipe_in[1]);

    int status = 0;
    waitpid(prev_pid, &status, 0);

    return retval;
}

int main(int argc, char* argv[])
{
    if (argc == 1)
        return 0;

    if (argc == 2) {
        execlp(argv[1], argv[1], NULL);

        return 1;
    }

    int pipe_one[2] = {-1, -1};
    int pipe_two[2] = {-1, -1};
    if (pipe(pipe_one) == -1)
        return 2;

    pid_t pid1 = subprocess(argv[1], STDIN_FILENO, pipe_one[1], pipe_one);
    pid_t pid2 = -1;

    if (pid1 == -1) {
        close(pipe_one[0]);
        close(pipe_one[1]);

        return 3;
    }

    char flipper = 0;
    for (int i = 2; i < argc - 1; ++i) {
        int prev_pid = flipper ? pid2 : pid1;
        int* cur_pid = flipper ? &pid1 : &pid2;
        int* pipe_in = flipper ? pipe_two : pipe_one;
        int* pipe_out = flipper ? pipe_one : pipe_two;

        if (pipe(pipe_out) == -1) {
            int status = 0;
            waitpid(prev_pid, &status, 0);
            return 2;
        }

        *cur_pid = piped_subprocess(argv[i], pipe_in, pipe_out, prev_pid);

        if (*cur_pid == -1) {
            close(pipe_out[0]);
            close(pipe_out[1]);

            return 3;
        }

        flipper ^= 1;
    }

    int* last_pipe = flipper ? pipe_two : pipe_one;
    pid_t pid =
        subprocess(argv[argc - 1], last_pipe[0], STDOUT_FILENO, last_pipe);

    close(last_pipe[0]);
    close(last_pipe[1]);

    int prev_pid = flipper ? pid2 : pid1;
    int status = 0;
    waitpid(prev_pid, &status, 0);

    if (pid == -1)
        return 3;

    waitpid(pid, &status, 0);

    return 0;
}
