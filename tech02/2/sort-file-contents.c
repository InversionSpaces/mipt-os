#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    if (argc != 2)
        return 1;

    const int infd = open(argv[1], O_RDWR);
    if (infd == -1)
        return 2;

    const int lessfd = open("./", __O_TMPFILE | O_EXCL | O_RDWR);
    if (lessfd == -1) {
        close(infd);

        return 3;
    }

    const int greaterfd = open("./", __O_TMPFILE | O_EXCL | O_RDWR);
    if (greaterfd == -1) {
        close(infd);
        close(lessfd);

        return 3;
    }

    const off_t start = lseek(infd, 0, SEEK_SET);
    if (start == -1) {
        close(infd);
        close(lessfd);
        close(greaterfd);

        return 4;
    }

    const off_t end = lseek(infd, 0, SEEK_END);
    if (end == -1) {
        close(infd);
        close(lessfd);
        close(greaterfd);

        return 4;
    }

    merge_sort(start, end, 
}
