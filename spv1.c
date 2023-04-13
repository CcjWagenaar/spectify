#include <stdio.h>
#include <stdlib.h>

#include "gen_array.c"
#include "time_and_flush.c"

int main(int argc, char* argv[]) {

    printf("hello, world!\n");

    //cp_t** arr = malloc_arr_cache_pages();
    //free_arr(arr);

    cp_t* arr = mmap_arr_cache_pages();
    printf("sizeof(arr) = %ld\n", sizeof(arr));
    //print_mmap_arr_vals(arr);

    volatile int _n_pages = 100; //afraid that constants will be propagated consts by the compiler, hence not in mem

    time_t t = time_mem_load((void*)&_n_pages);
    printf("time = %ld\n", t);
    flush((void*)&_n_pages);
    t = time_mem_load((void*)&_n_pages);
    printf("time = %ld\n", t);



}