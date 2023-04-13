#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#define PAGESIZE 4096
#define N_PAGES 100

typedef struct cache_page {
    int id __attribute__(( aligned(PAGESIZE) ));
} cp_t;

cp_t** malloc_arr_cache_pages() {
    cp_t** arr = malloc(N_PAGES * sizeof(void*));
    printf("alloc'ed %d * %ld\n", N_PAGES, sizeof(void*));

    for(int i = 0; i < N_PAGES; i++) {
        cp_t* cache_page = malloc(sizeof(cp_t));
        arr[i] = cache_page;

        //printf("assigning i=%d\n",i);
        arr[i]->id = i;
    }

    return arr;
}

cp_t* mmap_arr_cache_pages() {
    //cp_t** arr = mmap(NULL, 100 * sizeof(cp_t), PROT_READ | PROT_WRITE,
    //                  MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    cp_t* arr = mmap(NULL, N_PAGES * sizeof(cp_t), PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    //void* arr = arr - N_PAGES * sizeof(cp_t);
    printf("arr = 0x%p\n", arr);

    //cp_t* cache_page = arr + 100 * sizeof(cp_t);
    //printf("cache page = 0x%p\n", cache_page);
    //printf("cache id = %d\n", cache_page->id);
    //cache_page->id = 1;
    //printf("cache id = %d\n", cache_page->id);
    for(int i = 0; i < N_PAGES; i++) {
        cp_t* cache_page = arr + i;//*sizeof(cp_t);
        cache_page->id = i;
        //printf("assigning i=%d\n",i);
        //arr[i]->id = i;
    }

    //cp_t* cache_page = arr + 56 * sizeof(cp_t);
    //printf("cache id = %d\n", cache_page->id);

    return arr;
}

void print_malloc_arr_vals(cp_t** arr) {

    for(int i = 0; i < N_PAGES; i++) {
        printf("val %d = %d\n", i, arr[i]->id);
    }

}

void print_mmap_arr_vals(cp_t* arr) {

    for(int i = 0; i < N_PAGES; i++) {
        //cp_t* cache_page = arr + i;
        cp_t* cache_page = &arr[i];
        printf("%p\tval %d = ", cache_page, i);
        printf("%d", cache_page->id);
        printf("\n");
    }
}

void free_arr(cp_t** arr) {

    for(int i = 0; i < N_PAGES; i++) {
        free(arr[i]);
    }
    free(arr);

}

int main(int argc, char* argv[]) {

    printf("hello, world!\n");

    //cp_t** arr = malloc_arr_cache_pages();
    //free_arr(arr);

    cp_t* arr = mmap_arr_cache_pages();
    printf("sizeof(arr) = %ld\n", sizeof(arr));
    print_mmap_arr_vals(arr);



}