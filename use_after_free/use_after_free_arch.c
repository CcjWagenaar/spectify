#include <stdio.h>
#include <stdlib.h>

#include "../lib/gen_array.c"
#include "../lib/time_and_flush.c"
#include "../lib/print_results.c"

#define SECRET_SIZE 8
#define SECRET "mysecret"
#define MALLOC_SIZE 4096

void* attack_func(int secret_index) {
    int* k_dupe = malloc(MALLOC_SIZE);
    *k_dupe = secret_index;
    //if(DBG)printf("k dupe addr: %p\n", k_dupe);
    return k_dupe;
}

char victim_func(int secret_index) {

    int* k = calloc(1, MALLOC_SIZE);
    free(k);

    int* k_dupe = attack_func(secret_index);

    //printf("k:    \t%p\nk_dupe:\t%p\n", k, k_dupe);

    char secret_char = SECRET[*k];
    free(k_dupe);
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
