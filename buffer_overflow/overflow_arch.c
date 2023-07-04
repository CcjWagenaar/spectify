#include <stdio.h>

#include "../lib/gen_array.c"
#include "../lib/time_and_flush.c"
#include "../lib/print_results.c"

#define SECRET_SIZE 9
#define BUF_SIZE    16
#define SECRET      "mysecret"

typedef struct vars {
    char buf[BUF_SIZE];
    char password;
} vars_t;
vars_t wrapper;

char victim_func(int user_id, char user_char,
                 char user_password, int secret_index) {
    wrapper.password = 'x';
    //if (user_id < BUF_SIZE) {
        wrapper.buf[user_id] = user_char;
    //}

    if (user_password == wrapper.password) {
        return SECRET[secret_index];
    } else return '?';
}

char prepare(int secret_index) {
    return victim_func(BUF_SIZE, 's', 's', secret_index);
}

int main(int argc, char** argv) {

    printf("buf      = %p\tbuf[last] = %p\npassword = %p\toverwrite = %p\n",
           &wrapper.buf, &wrapper.buf[BUF_SIZE-1], &wrapper.password, &wrapper.buf[BUF_SIZE]);

    char results[SECRET_SIZE];

    for (int s = 0; s < SECRET_SIZE; s++) {
        results[s] = prepare(s);
    }

    print_results_arch(results, SECRET_SIZE);

}