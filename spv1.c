#include <stdio.h>
#include <stdlib.h>

#include "gen_array.c"
#include "time_and_flush.c"

int main(int argc, char* argv[]) {

    printf("hello, world!\n");

    cp_t* arr = mmap_arr_cache_pages();
    print_mmap_arr_vals(arr);

    //last two pages will not be accessible.
    volatile int accessible_pages = N_PAGES-2;

    //array [0,1,2,3,4...]. one-but-last points to last.
    int *secret_data = malloc(N_PAGES * sizeof(int));
    for(int i = 0; i < N_PAGES; i++) { secret_data[i] = i; }
    secret_data[N_PAGES-1-1] = N_PAGES-1;

    int x;
    volatile cp_t cp;
    for(int i = 0; i < N_PAGES; i++) {
        printf("i = %d\n",i);
        flush((void*)&accessible_pages);
        if (i < accessible_pages) {
            x = secret_data[i];
            cp = arr[x];
        }
        if(i==N_PAGES-3) {
            printf("flushing array...\n");
            flush_arr((void*)arr);
        }
    }

    reload(arr, N_PAGES);

    free(secret_data);
}