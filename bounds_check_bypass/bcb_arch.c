#include <stdio.h>

#include "../lib/gen_array.c"
#include "../lib/time_and_flush.c"
#include "../lib/print_results.c"

#define SECRET      "mysecret"
#define BUF         "public"
#define SECRET_SIZE 9
int BUF_SIZE = 7;

char victim_func(int index) {
    //if (index < BUF_SIZE) {
        return BUF[index];
    //} else return '?';
}

char prepare(int secret_index) {
    return victim_func(secret_index);
}

int main(int argc, char* argv[]) {

    printf("SECRET \t%p\tsize = %d\nBUF  \t%p\tsize = %d\nBUF[%d] \t%p\n",
           &SECRET, SECRET_SIZE, &BUF, BUF_SIZE, BUF_SIZE, &BUF[BUF_SIZE]);
    char results[SECRET_SIZE];

    for (int s = 0; s < SECRET_SIZE; s++) {
        results[s] = prepare(BUF_SIZE + s);
    }

    print_results_arch(results, SECRET_SIZE);
}
