#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

sem_t semaphore;
#define SEM_COUNT 2

extern inline void cpuid() {
    asm volatile ("cpuid\n":::);
}
void* thread(void* data) {

    char* c = (char*)data;
    printf("t%c\tstarted\n", *c);

    int ret;
    ret = sem_wait(&semaphore);
    printf("t%c\tsem wait:\t%d\n", *c, ret);

    printf("t%c\tsleeping\n", *c);
    sleep(3);

    ret = sem_post(&semaphore);
    printf("t%c\tsem post:\t%d\n", *c, ret);

    return NULL;
}

int main(int argc, char** argv) {

    int ret;
    ret = sem_init(&semaphore, 0, SEM_COUNT);

    //shows deadlock on 2 separate threads
    pthread_t t1, t2, t3;
    pthread_create(&t1, NULL, thread, "1");
    pthread_create(&t2, NULL, thread, "2");
    pthread_create(&t3, NULL, thread, "3");

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    printf("\n\n");

    //shows deadlock on 1 thread
    printf("made semaphore:\t%d\n", ret);
    cpuid();
    ret = sem_wait(&semaphore);
    printf("sem wait once:\t%d\n", ret);
    cpuid();
    ret = sem_wait(&semaphore);
    printf("sem wait twice:\t%d\n", ret);
    cpuid();
    ret = sem_wait(&semaphore);
    printf("sem wait 3x:\t%d\n", ret);
    cpuid();
    ret = sem_post(&semaphore);
    ret = sem_post(&semaphore);
    ret = sem_post(&semaphore);
    ret = sem_destroy(&semaphore);
    printf("destroyed sem\t%d\n", ret);
    return 0;
}