#ifndef TIME_AND_FLUSH
#define TIME_AND_FLUSH

#include <stdio.h>
#include <stdlib.h>

#include "gen_array.c"

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

static inline void flush(char *addr) {
    asm __volatile__ ("mfence\nclflush 0(%0)" : : "r" (addr) :);
}

void flush_arr(cp_t* arr) {

    for(int i = 0; i < N_PAGES; i++) {
        flush((void*)&arr[i]);
    }

}

static inline void reload(cp_t *arr, int n_pages) {

    for(int i = 0; i < n_pages; i++) {

        time_t t = time_mem_load(&arr[i]);

        if (t < 175)
            printf("0x%p\ti=%d  (%d)\ttime = %ld cycles\n", &arr[i], i, arr[i].id, t);

        if      (t < 75)  printf("\t\t\t\t\t\t\t\tCACHE HIT\n");
        else if (t < 175) printf("\t\t\t\t\t\t\t\tmss?\n");
    }

}

#endif  //TIME_AND_FLUSH