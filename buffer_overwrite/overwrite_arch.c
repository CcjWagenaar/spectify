#include <stdio.h>
#include <stdlib.h>

#include "../lib/gen_array.c"
#include "../lib/time_and_flush.c"
#include "../lib/print_results.c"

#define SECRET_SIZE 8
#define BUF_SIZE 16
#define SECRET "mysecret"

typedef struct vars {
    char buf[BUF_SIZE];
    char password;
} vars_t;
vars_t stack;

char victim_func(int user_id, char user_char, char user_password, int secret_index) {
    stack.password = 'x';
    //if (user_id < BUF_SIZE) {
        stack.buf[user_id] = user_char;
    //}

    if (user_password == stack.password) {
        return SECRET[secret_index];
    } else return '?';
}

char prepare(int secret_index) {
    return victim_func(BUF_SIZE, 's', 's', secret_index);
}

int main(int argc, char** argv) {

    printf("buf      = %p\tbuf[last] = %p\npassword = %p\toverwrite = %p\n",
           &stack.buf, &stack.buf[BUF_SIZE-1], &stack.password, &stack.buf[BUF_SIZE]);

    char results[SECRET_SIZE];

    for (int s = 0; s < SECRET_SIZE; s++) {
        results[s] = prepare(s);
    }

    print_results_arch(results, SECRET_SIZE);

}