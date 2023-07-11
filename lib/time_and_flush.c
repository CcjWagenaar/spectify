#ifndef TIME_AND_FLUSH
#define TIME_AND_FLUSH

#include <stdio.h>
#include <stdlib.h>

#include "header.h"


//NOTE: not sure if "static" works here
static inline int time_mem_load(void *addr) {
    volatile unsigned long time;

    asm __volatile__ (
            "  mfence             \n"
            "  lfence             \n"
            "  rdtsc              \n"
            "  lfence             \n"
            "  movl %%eax, %%esi  \n"
            "  movl (%1), %%eax   \n"
            "  lfence             \n"
            "  rdtsc              \n"
            "  subl %%esi, %%eax  \n"
            "  clflush 0(%1)      \n"
            : "=a" (time)
            : "c" (addr)
            :  "%esi", "%edx");
    return time;
}

extern inline void flush(void *addr) {
    asm __volatile__ ("mfence\nclflush 0(%0)" : : "r" (addr) :);
}

extern inline void cpuid() {
    asm volatile ("cpuid\n":::);
}

extern inline void nop_20() {
    asm volatile (
            "nop\n"
            "nop\n"
            "nop\n"
            "nop\n"
            "nop\n"
            "nop\n"
            "nop\n"
            "nop\n"
            "nop\n"
            "nop\n"
            "nop\n"
            "nop\n"
            "nop\n"
            "nop\n"
            "nop\n"
            "nop\n"
            "nop\n"
            "nop\n"
            "nop\n"
            "nop\n"
            :::
            );
}

void flush_arr(cp_t* arr, int N_PAGES) {

    for(int i = 0; i < N_PAGES; i++) {
        flush((void*)&arr[i]);
    }

}

static inline int* reload(cp_t *arr, int N_PAGES, int CACHE_HIT) {
    int* times = malloc(N_PAGES * sizeof(int));

    for(int i = 0; i < N_PAGES; i++) {

        int t = (int)time_mem_load(&arr[i]);
        times[i] = t;

    }
    for(int i = 0; i < N_PAGES; i++) {
        int t = times[i];
        if (t < CACHE_HIT) {
            //printf("0x%p\ti=%03d  (%c)\ttime = %d\tCACHE HIT\n", &arr[i], i, arr[i].id, t);
        }//*/
    }


    return times;
}

#endif  //TIME_AND_FLUSH