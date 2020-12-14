#include <stdlib.h>
#include <stdio.h>

#include <pthread.h>

typedef struct {
    double* arr;
    pthread_mutex_t* mutexes;
    int size;
    int times;

    int id;
} arg_t;

void* add(void* data) {
    arg_t* arg = data;

    int lid = arg->id == 0 ? arg->size - 1 : arg->id - 1;
    int rid = (arg->id + 1) % arg->size;

    for (int i = 0; i < arg->times; ++i) {
        pthread_mutex_lock(arg->mutexes + lid);
        arg->arr[lid] += 0.99;
        pthread_mutex_unlock(arg->mutexes + lid);

        pthread_mutex_lock(arg->mutexes + arg->id);
        arg->arr[arg->id] += 1.;
        pthread_mutex_unlock(arg->mutexes + arg->id);

        pthread_mutex_lock(arg->mutexes + rid);
        arg->arr[rid] += 1.01;
        pthread_mutex_unlock(arg->mutexes + rid);
    }

    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    if (argc != 3)
        return 1;
    
    int N = atoi(argv[1]);
    int k = atoi(argv[2]);

    if (N <= 0 || k <= 0)
        return 1;
    
    double arr[k];
    arg_t args[k];

    pthread_mutex_t mutexes[k];
    pthread_t threads[k];

    for (int i = 0; i < k; ++i) {
        arr[i] = 0.;
        args[i].arr = arr;
        args[i].mutexes = mutexes;
        args[i].size = k;
        args[i].times = N;
        args[i].id = i;

        pthread_mutex_init(mutexes + i, NULL);
    }
    
    for (int i = 0; i < k; ++i)
        pthread_create(threads + i, NULL, add, args + i);
    
    for (int i = 0; i < k; ++i)
        pthread_join(threads[i], NULL);
    
    for (int i = 0; i < k; ++i)
        pthread_mutex_destroy(mutexes + i);
    
    for (int i = 0; i < k; ++i)
        printf("%.10g ", arr[i]);

    return 0;
}