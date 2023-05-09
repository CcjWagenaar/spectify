#ifndef HEADER
#define HEADER

#define PAGESIZE 4096

typedef struct cache_page {
    int id __attribute__(( aligned(PAGESIZE) ));
} cp_t;

#endif //HEADER