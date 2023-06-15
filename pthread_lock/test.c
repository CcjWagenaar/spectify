#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

pthread_mutex_t k;

extern inline void cpuid() {
    asm volatile ("cpuid\n":::);
}
void* thread1(void* data) {

    char* c = (char*)data;
    printf("t%c\tstarted\n", *c);

    int ret;
    ret = pthread_mutex_lock(&k);
    printf("t%c\tlocked:\t\t%d\n", *c, ret);

    printf("t%c\tsleeping\n", *c);
    sleep(3);

    ret = pthread_mutex_unlock(&k);
    printf("t%c\tunlocked:\t%d\n", *c, ret);

    return NULL;
}
void* thread(void* data) {
    char* c = (char*)data;
    printf("thread %c\n", *c);
    return NULL;
}

int main(int argc, char** argv) {
    //shows deadlock on 2 separate threads
    int ret;
    ret = pthread_mutex_init(&k, PTHREAD_MUTEX_DEFAULT);
    pthread_t t1, t2;
    pthread_create(&t1, NULL, thread1, "1");
    pthread_create(&t2, NULL, thread1, "2");

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("\n\n");

    //shows deadlock on 1 thread
    printf("made mutex:\t%d\n", ret);
    cpuid();
    ret = pthread_mutex_lock(&k);
    printf("locked once:\t%d\n", ret);
    cpuid();
    ret = pthread_mutex_lock(&k);
    printf("locked twice\t%d\n", ret);
    cpuid();
    ret = pthread_mutex_destroy(&k);
    printf("destroyed mutex\t%d\n", ret);
    return 0;
}