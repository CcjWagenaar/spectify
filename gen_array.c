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

cp_t** malloc_arr_cache_pages() {
    cp_t** arr = malloc(N_PAGES * sizeof(void*));
    printf("alloc'ed %d * %ld\n", N_PAGES, sizeof(void*));

    for(int i = 0; i < N_PAGES; i++) {
        cp_t* cache_page = malloc(sizeof(cp_t));
        arr[i] = cache_page;
        arr[i]->id = i;
    }

    return arr;
}

cp_t* mmap_arr_cache_pages() {
    cp_t* arr = mmap(NULL, N_PAGES * sizeof(cp_t), PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    printf("arr = 0x%p\n", arr);
    for(int i = 0; i < N_PAGES; i++) {
        cp_t* cache_page = arr + i; // or &arr[i]
        cache_page->id = i;
    }
    return arr;
}

void print_malloc_arr_vals(cp_t** arr) {

    for(int i = 0; i < N_PAGES; i++) {
        printf("val %d = %d\n", i, arr[i]->id);
    }

}

void print_mmap_arr_vals(cp_t* arr) {

    for(int i = 0; i < N_PAGES; i++) {
        cp_t* cache_page = &arr[i];
    }
}

void free_arr(cp_t** arr) {
    for(int i = 0; i < N_PAGES; i++) {
        free(arr[i]);
    }
    free(arr);
}

#endif  //GEN_ARRAY