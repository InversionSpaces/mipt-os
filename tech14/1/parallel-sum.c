#include <stdlib.h>
#include <stdio.h>

#include <limits.h>
#include <pthread.h>

void* count(void* arg) {
    long long part_sum = 0;
    long long tmp = 0;
    while (scanf("%d", &tmp) == 1)
        part_sum += tmp;

    pthread_exit((void*)part_sum);
}

int main(int argc, char* argv[]) {
    if (argc != 2)
        return 1;
    
    int N = atoi(argv[1]);

    if (N <= 0)
        return 1;
    
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN);
    pthread_attr_setguardsize(&attr, 0);

    pthread_t threads[N];
    for (int i = 0; i < N; ++i)
        pthread_create(threads + i, &attr, count, NULL);
    
    pthread_attr_destroy(&attr);

    long long res = 0;
    for (int i = 0; i < N; ++i) {
        void* ret = 0;
        pthread_join(threads[i], &ret);
        res += (long long)ret;
    }

    printf("%lld", res);
}