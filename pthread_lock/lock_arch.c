#include <stdio.h>
#include <stdlib.h>

#include "../lib/gen_array.c"
#include "../lib/time_and_flush.c"
#include "../lib/print_results.c"

#define SECRET_SIZE 9
#define SECRET "mysecret"
#define DBG 0

char victim_func(int secret_index) {
    return 'x';
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