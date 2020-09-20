#include <sys/syscall.h>

extern long syscall(long number, ...);

static const char hello[] = "Hello, World!";

void _start() {
    syscall(SYS_write, 1, hello, sizeof(hello) - 1);
    syscall(SYS_exit, 0);
}
