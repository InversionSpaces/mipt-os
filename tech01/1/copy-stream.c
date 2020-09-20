#include <sys/syscall.h>

extern long syscall(long number, ...);

#define BUFFER_SIZE (1024)

void _start() {
    char buffer[BUFFER_SIZE];
    long ret = 0;
    while (ret = syscall(SYS_read, 0, buffer, sizeof(buffer))) {
        if (ret < 0) syscall(SYS_exit, 1);
        syscall(SYS_write, 1, buffer, (unsigned long)ret);
    }
    syscall(SYS_exit, 0);
}
