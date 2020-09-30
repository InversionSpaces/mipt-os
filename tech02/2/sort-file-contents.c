#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

int merge_sort(const off_t l, const off_t r, const int infd, const int tmpfd) {
    if (r == l || r - l == sizeof(int32_t))
        return 0;

    const int size = 1024;
    int32_t buffer[size];
    const int byte_size = sizeof(buffer);

    if (lseek(infd, l, SEEK_SET) == -1)
        return -1;

    off_t off_less = 0;
    const off_t off_part1 = r - l;
    off_t off_greater = off_part1;
    const off_t off_part2 = 2 * off_part1;
    off_t off_equal = off_part2;

    off_t cur = l;

    char is_pivot_set = 0;
    int32_t pivot = 0;

#define READ_ATTEMPT(FID, EDGE)                             \
    const int last = EDGE - cur;                            \
    const int to_read = byte_size > last ? last : byte_size;\
    const int readed = read(FID, buffer, to_read);          \
    if (readed != to_read)                                  \
        return -1;                                          \
    cur += readed;

    while (cur < r) {
        READ_ATTEMPT(infd, r)

        int32_t less[size];
        int32_t greater[size];
        int32_t equal[size];

        if (!is_pivot_set) {
            pivot = buffer[0];
            is_pivot_set = 1;
        }

        int lcar = 0;
        int gcar = 0;
        int ecar = 0;
        for (unsigned int i = 0; i < readed / sizeof(buffer[0]); ++i) {
            if (buffer[i] < pivot)
                less[lcar++] = buffer[i];
            else if (buffer[i] > pivot)
                greater[gcar++] = buffer[i];
            else
                equal[ecar++] = buffer[i];
        }

#define WRITE_ATTEMPT(CARRET, OFFSET, ARRAY)                \
        if (CARRET > 0) {                                   \
            if (lseek(tmpfd, OFFSET, SEEK_SET) == -1)       \
                return -1;                                  \
            const int to_write = CARRET * sizeof(buffer[0]);\
            const int writed = write(                       \
                    tmpfd,                                  \
                    ARRAY,                                  \
                    to_write);                              \
            if (writed != to_write)                         \
                return -1;                                  \
            OFFSET += writed;                               \
        }

        WRITE_ATTEMPT(lcar, off_less, less)
        WRITE_ATTEMPT(gcar, off_greater, greater)
        WRITE_ATTEMPT(ecar, off_equal, equal)

#undef WRITE_ATTEMPT
    }

    if (lseek(infd, l, SEEK_SET) == -1)
        return -1;

#define MERGE_ATTEMPT(START, END)                           \
    cur = START;                                            \
    while (cur < END) {                                     \
        READ_ATTEMPT(tmpfd, END)                            \
        const int writed = write(infd, buffer, readed);     \
        if (writed != readed)                               \
            return -1;                                      \
    }

    if (lseek(tmpfd, 0, SEEK_SET) == -1)
        return -1;

    MERGE_ATTEMPT(0, off_less)

    if (lseek(tmpfd, off_part2, SEEK_SET) == -1)
        return -1;

    MERGE_ATTEMPT(off_part2, off_equal)

    if (lseek(tmpfd, off_part1, SEEK_SET) == -1)
        return -1;

    MERGE_ATTEMPT(off_part1, off_greater)

#undef MERGE_ATTEMPT
#undef READ_ATTEMPT

    if (merge_sort(l, l + off_less, infd, tmpfd) == -1)
        return -1;

    return merge_sort(l + off_less + (off_equal - off_part2), r, infd, tmpfd);
}

int main(int argc, char* argv[]) {
    if (argc != 2)
        return 1;

    const int infd = open(argv[1], O_RDWR);
    if (infd == -1)
        return 2;

    const int tmpfd = open("tmp", O_RDWR | O_CREAT | O_EXCL, 0666);
    if (tmpfd == -1) {
        close(infd);

        return 3;
    }

    const off_t start = lseek(infd, 0, SEEK_SET);
    if (start == -1) {
        close(infd);
        close(tmpfd);

        return 4;
    }

    const off_t end = lseek(infd, 0, SEEK_END);
    if (end == -1) {
        close(infd);
        close(tmpfd);

        return 4;
    }

    return merge_sort(start, end, infd, tmpfd);
}
