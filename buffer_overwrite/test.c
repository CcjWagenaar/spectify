#include <stdio.h>
#include <stdlib.h>

//#include "../lib/gen_array.c"
#include "../lib/time_and_flush.c"
//#include "../lib/print_results.c"

#define N_PAGES 256
#define REPETITIONS 1
#define CACHE_HIT 100
#define MAYBE_CACHE_HIT 175

volatile char super_secret = 97;
volatile int  buf_size     = 16;
int main(int argc, char** argv) {

    volatile int buf_size2 = buf_size;
    volatile int  n_training   = 10;
    volatile long long test = 10;
    volatile long long test2 = 20;
    volatile long long test3 = 30;
    volatile long long test4 = 40;
    volatile long long test5 = 50;
    volatile long long test6 = 60;
    volatile long long test7 = 70;
    volatile long long test8 = 80;
    volatile long long test9 = 80;
    volatile long long test10 = 80;
    volatile long long test11 = 80;
    volatile long long test12 = 80;
    volatile long long test13 = 80;
    volatile long long test14 = 80;
    volatile long long test15 = 80;
    volatile long long test16 = 80;
    volatile int  secret_size  = 1;

    //printf("random print: %d %d %lld %lld %lld %lld %lld %lld %lld %lld lld %lld %lld %lld %lld %lld %lld %lld %lld %d %d\n",
    //       buf_size, n_training, test, test2, test3, test4, test5, test6, test7, test8, test9, test10, test11, test12, test13, test14, test15, test16, secret_size, super_secret);

    printf("super_secret time = %d\n", time_mem_load((void*)&super_secret));
    __sync_synchronize();
    printf("buf_size time = %d\n", time_mem_load((void*)&buf_size));
    __sync_synchronize();
    flush((void*)&buf_size);
    __sync_synchronize();
    //volatile int bro = super_secret;
    //printf("super_secret time = %d\n", time_mem_load((void*)&super_secret));
    __sync_synchronize();
    printf("buf_size time = %d\n", time_mem_load((void*)&buf_size));
}