#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>

struct alloc_unit_t {
    size_t size;
    char is_free;
    struct alloc_unit_t* next;
};

static struct alloc_unit_t* head = NULL;
size_t size = 0;

int malloc_error = 0;

void* my_malloc(size_t size) {
    struct alloc_unit_t* retval = NULL;
    for (struct alloc_unit_t* current = head; current; current = current->next) {
        if (    current->is_free &&
                current->size >= size &&
                (!retval || retval->size > current->size)   )
            retval = current;
    }

    if (!retval)
        return NULL;

    if (retval->size - size > sizeof(struct alloc_unit_t)) {
        struct alloc_unit_t* inserted = ((void*)(retval + 1)) + size;

        inserted->next = retval->next;
        inserted->size = retval->size - size - sizeof(struct alloc_unit_t);
        inserted->is_free = 1;

        retval->next = inserted;
        retval->is_free = 0;
    }

    return retval;
}

void my_free(void *ptr) {

}

void myalloc_initialize(int fd) {
    struct stat file_stat = {};
    if (fstat(fd, &file_stat) == -1) {
        malloc_error = 1;
        return;
    }

    size = file_stat.st_size;

    if (size < sizeof(struct alloc_unit_t)) {
        malloc_error = 2;
        return;
    }

    head = mmap(
            NULL,
            size,
            PROT_READ | PROT_WRITE,
            MAP_SHARED,
            fd,
            0
        );

    if (head == MAP_FAILED) {
        malloc_error = 3;
        return;
    }

    head->size = size - sizeof(struct alloc_unit_t);
    head->is_free = 1;
    head->next = NULL;
}

void myalloc_finalize() {
    munmap(head, size);
}

int main() {

}
