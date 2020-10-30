#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>

struct alloc_unit_t {
    size_t size;
    char is_free;
    struct alloc_unit_t* next;
};

static struct alloc_unit_t* head = NULL;
static size_t size = 0;

int malloc_error = 0;

void* my_malloc(size_t size) {
    if (malloc_error)
        return NULL;

    struct alloc_unit_t* retval = NULL;
    for (struct alloc_unit_t* current = head; current; current = current->next) {
        if (    current->is_free &&
                current->size >= size &&
                (!retval || retval->size > current->size)   )
            retval = current;
    }

    if (!retval)
        return NULL;

    if (retval->size > sizeof(struct alloc_unit_t) + size) {
        struct alloc_unit_t* inserted = ((void*)(retval + 1)) + size;

        inserted->next = retval->next;
        inserted->size = retval->size - size - sizeof(struct alloc_unit_t);
        inserted->is_free = 1;

        retval->next = inserted;
        retval->size = size;
    }

    retval->is_free = 0;
    return retval + 1;
}

void my_free(void *ptr) {
    if (malloc_error)
        return;

    struct alloc_unit_t* cur = ((struct alloc_unit_t*)ptr) - 1;

    struct alloc_unit_t* prev = (cur == head ? NULL : head);
    while (prev && prev->next != cur)
        prev = prev->next;

    struct alloc_unit_t* next = cur->next;

    if (prev && prev->is_free) {
        prev->size += cur->size + sizeof(struct alloc_unit_t);
        prev->next = next;
        cur = prev;
    }

    if (next && next->is_free) {
        cur->size += next->size + sizeof(struct alloc_unit_t);
        cur->next = next->next;
    }

    cur->is_free = 1;
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
    if (malloc_error)
        return;
    munmap(head, size);
}

/*
#include <stdio.h>

void print_list() {
    for (struct alloc_unit_t* cur = head; cur; cur = cur->next) {
        printf("(size: %ld, is_free: %d)", cur->size, (int)cur->is_free);
        if (cur->next)
            printf(" -> ");
    }

    printf("\n\n");
}

int main(int argc, char* argv[]) {
    if (argc != 2)
        return 1;

    int fd = open(argv[1], O_RDWR);

    if (fd == -1)
        return 1;

    myalloc_initialize(fd);

    print_list();

    char* ptr1 = my_malloc(1);
    char* ptr2 = my_malloc(2);
    char* ptr3 = my_malloc(3);
    char* ptr4 = my_malloc(4);
    char* ptr5 = my_malloc(5);

    print_list();

    if (!ptr1 || !ptr2 || !ptr3 || !ptr4 || !ptr5) {
        printf("1 error\n");
        return 1;
    }

    my_free(ptr4);
    print_list();
    my_free(ptr2);
    print_list();
    my_free(ptr3);
    print_list();
    my_free(ptr1);
    print_list();
    my_free(ptr5);
    print_list();

    char* ptr = my_malloc(100);

    if (!ptr) {
        printf("2 error\n");
        return 1;
    }

    myalloc_finalize();

    close(fd);
}
*/
