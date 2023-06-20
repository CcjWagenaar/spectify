#include <stdio.h>
#include <stdlib.h>

#include "../lib/gen_array.c"
#include "../lib/time_and_flush.c"
#include "../lib/print_results.c"

#define SECRET_SIZE 9
#define SECRET "mysecret"
#define MALLOC_SIZE 4096
#define DBG 1

void* attack_func(int secret_index) {
    int* var1_dupe = malloc(MALLOC_SIZE);
    *var1_dupe = secret_index;
    return var1_dupe;
}

char victim_func(int secret_index) {

    int* var1 = calloc(1, MALLOC_SIZE);
    free(var1);

    int* var1_dupe = attack_func(secret_index);

    if(DBG)printf("var1:    \t%p\nvar1_dupe:\t%p\n", var1, var1_dupe);

    char secret_char = SECRET[*var1];
    free(var1_dupe);
    return secret_char;
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
