#include <stdio.h>
#include <stdlib.h>

#include "../lib/gen_array.c"
#include "../lib/time_and_flush.c"
#include "../lib/print_results.c"

#define SECRET_SIZE 8
#define BUF_SIZE 13

typedef struct vars {
    char* super_secret;
    //char secret;                                                    //OPTION 1: works for some reason?
    char buf[BUF_SIZE];
    char secret;                                                  //OPTION 2: should work but does not

} vars_t;
vars_t stack;

char check_passwd(int user_idx, char user_char, char user_passwd, int secret_index) {

    stack.super_secret = "mysecret"; //TODO: just line them up here

    //bounds check should prevent overwrite. But speculatively executes
    //if (stack.user_idx < buf_size) {
        stack.buf[user_idx] = user_char;
    //}

    //secret has been (speculatively) overwritten with user char.
    if (user_passwd == stack.secret) {
        //volatile cp_t cp = flush_reload_arr[cache.secret];
        char s = stack.super_secret[secret_index];
        return s;
        //printf("super secret: '%c' (%d)\n", cp.id, cp.id);
    }

    return '?';
}

char prepare(int secret_index) {
    return check_passwd(BUF_SIZE, 's', 's', secret_index);
}

int main(int argc, char** argv) {

    printf("s secret\t%p\n", &stack.super_secret);
    printf("buf[%d] \t%p\n", BUF_SIZE, &stack.buf);
    printf("secret  \t%p\n", &stack.secret);

    char results[SECRET_SIZE];

    for (int s = 0; s < SECRET_SIZE; s++) {
        results[s] = prepare(s);
    }

    print_results_arch(results, SECRET_SIZE);

}