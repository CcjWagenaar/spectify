#include <stdio.h>
#include <stdlib.h>

#include "../lib/gen_array.c"
#include "../lib/time_and_flush.c"
#include "../lib/print_results.c"

#define SECRET_SIZE 9
#define SECRET "mysecret"
#define MALLOC_SIZE 4096
#define false 0
#define true  1
#define DBG 1

void touch_secret(int secret_index) {
    volatile char s = SECRET[secret_index];
    if(DBG)printf("secret\t%p:\t%d\n", &s, s);
}

char uninit_func() {
    volatile char x;
    if(DBG)printf("uninit\t%p:\t%d\n", &x, x);
    return x;
}

char victim_func(char init_bool, int secret_index) {
    touch_secret(secret_index);
    return uninit_func();

}

char prepare(int secret_index) {
    return victim_func(false, secret_index);
}

int main(int argc, char** argv) {

    char results[SECRET_SIZE];


    for (int s = 0; s < SECRET_SIZE; s++) {
        results[s] = prepare(s);
    }

    print_results_arch(results, SECRET_SIZE);


}