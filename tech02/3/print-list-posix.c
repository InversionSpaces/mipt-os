#include <inttypes.h>
#include <stdio.h>
#include <windows.h>

typedef struct {
  int value;
  uint32_t next_pointer;
} Item;

int read_val(HANDLE fd, void* val, const size_t size) {
    size_t total = 0;
    DWORD readed = 0;
    if (!ReadFile(fd, val + total, size - total, &readed, NULL))
        return -1;
    if (readed != size)
        return -1;

    return 0;
}

int read_item(HANDLE fd, Item* item) {
    int res = read_val(fd, &(item->value), sizeof(item->value));
    if (res == -1)
        return -1;

    res = read_val(fd, &(item->next_pointer), sizeof(item->next_pointer));
    return res;
}

int main(int argc, char* argv[]) {
    if (argc != 2)
        return 1;

    HANDLE fd = CreateFile(argv[1],
            GENERIC_READ,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );
    if (fd == INVALID_HANDLE_VALUE)
        return 2;

    Item item = {};
    while (read_item(fd, &item) == 0) {
        printf("%d ", item.value);
        if (item.next_pointer == 0)
            break;
        if (!SetFilePointer(fd, item.next_pointer, NULL, FILE_BEGIN))
            return 3;
    }

    CloseHandle(fd);

    return 0;
}
