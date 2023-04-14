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
    print_mmap_arr_vals(arr);

    volatile int n_pages = 100; //afraid that constants will be propagated consts by the compiler, hence not in mem

    //time_t t = time_mem_load((void*)&n_pages);
    //printf("time = %ld\n", t);
    //flush((void*)&n_pages);
    //t = time_mem_load((void*)&n_pages);
    //printf("time = %ld\n", t);

    flush_arr((void*)arr);

    volatile cp_t cp = arr[3];
    printf("loaded cp 3. id = %d\n", cp.id);

    reload(arr, n_pages);

}