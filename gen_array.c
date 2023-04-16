#ifndef GEN_ARRAY
#define GEN_ARRAY

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#define PAGESIZE 4096
#define N_PAGES 1000

typedef struct cache_page {
    int id __attribute__(( aligned(PAGESIZE) ));
} cp_t;

cp_t* mmap_arr_cache_pages() {
    cp_t* arr = mmap(NULL, N_PAGES * sizeof(cp_t), PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    printf("arr = 0x%p\n", arr);
    for(int i = 0; i < N_PAGES; i++) {
        cp_t* cache_page = &arr[i];
        cache_page->id = i;
    }
    return arr;
}

void print_mmap_arr_vals(cp_t* arr) {

    for(int i = 0; i < N_PAGES; i++) {
        cp_t* cache_page = &arr[i];
    }
}

#endif  //GEN_ARRAY