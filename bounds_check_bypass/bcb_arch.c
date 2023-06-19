#include <stdio.h>
#include <stdlib.h>

#include "../lib/print_results.c"
#include "../lib/time_and_flush.c"
#include "../lib/print_results.c"


#define SECRET "mysecret"
#define DATA "public"
#define SECRET_SIZE 9
int DATA_SIZE = 7;

char victim_func(int index) {
    //if (index < DATA_SIZE) {
        unsigned char x = DATA[index];
        return x;
    //} else return '?';
}

char prepare(int secret_index) {
    return victim_func(secret_index);
}

int main(int argc, char* argv[]) {

    printf("SECRET \t%p\tsize = %d\nDATA \t%p\tsize = %d\nDATA[%d]\t%p\n",
           &SECRET, SECRET_SIZE, &DATA, DATA_SIZE, DATA_SIZE, &DATA[DATA_SIZE]);
    char results[SECRET_SIZE];

    for (int s = 0; s < SECRET_SIZE; s++) {
        results[s] = prepare(DATA_SIZE + s);
    }

    print_results_arch(results, SECRET_SIZE);
}
