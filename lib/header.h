#ifndef HEADER
#define HEADER

#define PAGESIZE 4096

typedef struct cache_page {
    int id __attribute__(( aligned(PAGESIZE) ));
} cp_t;

//gen_array.c
cp_t* init_flush_reload(int N_PAGES);
void unmap_cache_pages(cp_t* arr, int N_PAGES);
void print_mmap_arr_vals(cp_t* arr, int N_PAGES);
int*** alloc_results(int REPETITIONS, int secret_size, int N_PAGES);
void free_results(int*** results, int REPETITIONS, int secret_size);

//time_and_flush.c
static inline int time_mem_load(void *addr);
extern inline void flush(void *addr);
extern inline void cpuid();
extern inline void nop_20();
void flush_arr(cp_t* arr, int N_PAGES);
static inline int* reload(cp_t *arr, int N_PAGES, int CACHE_HIT, int MAYBE_CACHE_HIT);

//print_results.c
int cmpfunc (const void * a, const void * b) {return ( *(int*)a - *(int*)b ); }
void print_results(int*** results, int REPETITIONS, int secret_size, int N_PAGES, int CACHE_HIT);


#endif //HEADER