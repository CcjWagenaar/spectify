#include <stdio.h>
#include <stdlib.h>

#include "../lib/print_results.c"
#include "../lib/time_and_flush.c"
#include "../lib/print_results.c"

#define SECRET_SIZE 25
//TODO: update based on updated bcb
#define SECRET "zhis is a secret message"
#define DATA "notasecretnotasecretnotasecretnotasecretnotasecretnotasecretnotasecretnotasecretnotasecretnotasecre"
int accessible = 100; //TODO: maybe make this a #define?

char victim_func(int index, unsigned char* data) {

    //if (index < accessible) {
        unsigned char x = data[index];
    //}
    return x;
}

char prepare(int secret_index) {
    return victim_func(secret_index, DATA);
}

int main(int argc, char* argv[]) {

    printf("SECRET \t%p\tsize = 25\nDATA \t%p\tsize = 125\n", &SECRET, &DATA);

    char results[SECRET_SIZE];

    for (int s = 0; s < SECRET_SIZE; s++) {
        results[s] = prepare(accessible + s);
    }

    print_results_arch(results, SECRET_SIZE);
}
