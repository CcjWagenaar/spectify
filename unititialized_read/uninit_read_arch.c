#include <stdio.h>

#include "../lib/gen_array.c"
#include "../lib/time_and_flush.c"
#include "../lib/print_results.c"

#define SECRET_SIZE 9
#define SECRET      "mysecret"
#define DBG         0

void touch_secret(int secret_index) {
    volatile char s = SECRET[secret_index];
    if(DBG)printf("secret\t%p:\t%d\n", &s, s);
}

void init_func(int val) {
    volatile char init = val;
    if(DBG)printf("init  \t%p:\t%d\n", &init, init);
}

char uninit_func() {
    volatile char uninit;
    //volatile char uninit = 'u';
    if(DBG)printf("uninit\t%p:\t%d\n", &uninit, uninit);
    return uninit;
}

char victim_func(int secret_index) {
    touch_secret(secret_index);
    //init_func('i');
    return uninit_func();
}

char prepare(int secret_index) {
    return victim_func(secret_index);
}

int main(int argc, char** argv) {

    char results[SECRET_SIZE];


    for (int s = 0; s < SECRET_SIZE; s++) {
        results[s] = prepare(s);
    }

    print_results_arch(results, SECRET_SIZE);


}