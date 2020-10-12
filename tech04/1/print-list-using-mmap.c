#include <inttypes.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct {
  int value;
  uint32_t next_pointer;
} Item;


int read_item(void* pointer, Item* item) {
    item->value = *(int*)pointer;
    item->next_pointer = *(uint32_t*)(pointer + sizeof(item->value));
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc != 2)
        return 1;

    const int fd = open(argv[1], O_RDONLY);
    if (fd == -1)
        return 2;

    struct stat file_stat = {};
    if (fstat(fd, &file_stat) == -1) {
        close(fd);
        return 4;
    }

    if (file_stat.st_size == 0) {
        close(fd);
        return 0;
    }

    void* file = mmap(
            NULL,
            file_stat.st_size,
            PROT_READ,
            MAP_PRIVATE,
            fd,
            0
        );

    close(fd);

    if (file == MAP_FAILED)
        return 5;

    void* current = file;
    Item item = {};
    while (read_item(current, &item) == 0) {
        printf("%d ", item.value);
        if (item.next_pointer == 0)
            break;
        current = file + item.next_pointer;
    }

    munmap(file, file_stat.st_size);

    return 0;
}
