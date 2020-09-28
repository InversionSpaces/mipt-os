#include <inttypes.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct {
  int value;
  uint32_t next_pointer;
} Item;

int read_val(const int fd, void* val, const size_t size) {
    size_t total = 0;
    int readed = 0;
    while ((readed = read(fd, val + total, size - total)) > 0)
        total += readed;

    if (readed == -1)
        return -1;

    return total == size ? 0 : -1;
}

int read_item(const int fd, Item* item) {
    int res = read_val(fd, &(item->value), sizeof(item->value));
    if (res == -1)
        return -1;
    res = read_val(fd, &(item->next_pointer), sizeof(item->next_pointer));
    return res;
}

int main(int argc, char* argv[]) {
    if (argc != 2)
        return 1;

    const int fd = open(argv[1], O_RDONLY);
    if (fd == -1)
        return 2;

    Item item = {};
    while (read_item(fd, &item) == 0) {
        printf("%d ", item.value);
        if (item.next_pointer == 0)
            break;
        if (lseek(fd, item.next_pointer, SEEK_SET) == -1)
            return -1;
    }

    close(fd);

    return 0;
}
