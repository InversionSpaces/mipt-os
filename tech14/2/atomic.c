#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <limits.h>
#include <pthread.h>
#include <stdatomic.h>

typedef struct Item {
  struct Item *next;
  int64_t value;
} item_t;

// Not sure if i can put this in struct
_Atomic (item_t*) head;

void prepend(const int64_t val) {
    item_t* prepended = malloc(sizeof(item_t));
    prepended->value = val;
    prepended->next = atomic_exchange(&head, prepended);
}

typedef struct {
    int id;
    int64_t k;
} arg_t;

void* job(void* data) {
    arg_t* arg = data;
    for (int64_t i = 0; i < arg->k; ++i)
        prepend(arg->k * arg->id + i);
    
    pthread_exit(0);
}

int main(int argc, char* argv[]) {
    if (argc != 3)
        return 1;
    
    int N = atoi(argv[1]);
    int64_t k = atoll(argv[2]);

    if (N <= 0 || k <= 0)
        return 1;

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN);
    pthread_attr_setguardsize(&attr, 0);

    arg_t args[N];
    pthread_t threads[N];
    for (int i = 0; i < N; ++i) {
        args[i].id = i;
        args[i].k = k;

        pthread_create(threads + i, &attr, job, args + i);
    }

    pthread_attr_destroy(&attr);

    for (int i = 0; i < N; ++i)
        pthread_join(threads[i], NULL);
    
    item_t* cur = atomic_exchange(&head, NULL);
    while (cur != NULL) {
        printf("%ld ", cur->value);
        item_t* tmp = cur;
        cur = cur->next;
        free(tmp);
    }
}