#ifndef TIME_AND_FLUSH
#define TIME_AND_FLUSH

#include <stdio.h>
#include <stdlib.h>

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

//NOTE: not sure if "static" works here
static inline void flush(char *addr) {
    asm __volatile__ ("mfence\nclflush 0(%0)" : : "r" (addr) :);
}

#endif  //TIME_AND_FLUSH