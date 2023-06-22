#ifndef GEN_ARRAY
#define GEN_ARRAY

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "header.h"

cp_t* init_flush_reload(int N_PAGES) {
    cp_t* arr = mmap(NULL, N_PAGES * sizeof(cp_t), PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    for(int i = 0; i < N_PAGES; i++) {
        cp_t* cache_page = &arr[i];
        cache_page->id = i;
    }
    return arr;
}

void free_flush_reload(cp_t* arr, int N_PAGES) {
    munmap(arr, N_PAGES * sizeof(cp_t));
}

void print_mmap_arr_vals(cp_t* arr, int N_PAGES) {

    for(int i = 0; i < N_PAGES; i++) {
        cp_t* cache_page = &arr[i];
    }
}

int*** alloc_results(int REPETITIONS, int secret_size, int N_PAGES) {
    int*** results = malloc(REPETITIONS * sizeof(void*)); //results[REPETITIONS][secret_size][N_PAGES]ints

    for(int r = 0; r < REPETITIONS; r++) {
        results[r] = malloc(secret_size * sizeof(void*));
        for (int s = 0; s < secret_size; s++) results[r][s] = malloc(N_PAGES * sizeof(int));
    }

    return results;
}

void free_results(int*** results, int REPETITIONS, int secret_size) {
    for(int r = 0; r < REPETITIONS; r++) {
        for(int s = 0; s < secret_size; s++) free(results[r][s]);
        free(results[r]);
    }
}

#endif  //GEN_ARRAY