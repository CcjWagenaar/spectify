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

    //volatile int n_pages = 40;
    //time_t t = time_mem_load((void*)&n_pages);
    //printf("time = %ld\n", t);
    //flush((void*)&n_pages);
    //t = time_mem_load((void*)&n_pages);
    //printf("time = %ld\n", t);

    //flush_arr((void*)arr);
    //volatile cp_t cp = arr[3];
    //printf("loaded cp 3. id = %d\n", cp.id);
    //reload(arr, N_PAGES);

    volatile int accessible_pages = N_PAGES-2;

    int *secret_data = malloc(N_PAGES * sizeof(int));
    for(int i = 0; i < N_PAGES; i++) { secret_data[i] = i; }
    secret_data[N_PAGES-1-1] = N_PAGES-1;
    for(int i = 0; i < N_PAGES; i++) { printf("secret[%d]:\t%d\n", i, secret_data[i]); }

    int x;
    volatile cp_t cp;
    for(int i = 0; i < N_PAGES; i++) {
        printf("i = %d\n",i);
        flush((void*)&accessible_pages);
        if (i < accessible_pages) {
            x = secret_data[i];
            cp = arr[x];
            printf("%d\taccessed cp=%d w val %d\n", i, x, cp.id);
        }
        //__sync_synchronize();
        if(i==N_PAGES-5) {
            printf("flushing array...\n");
            flush_arr((void*)arr);
            __sync_synchronize();
        }
        //__sync_synchronize();
    }

    printf("reloading...\n");
    reload(arr, N_PAGES);
    printf("N_PAGES     = %d\naccesible i = %d\nflushed i   = %d\n", N_PAGES, accessible_pages-1, N_PAGES-3);

    free(secret_data);
}