#include <stdio.h>
#include <stdlib.h>
#include "../lib/gen_array.c"
#include "../lib/time_and_flush.c"
#include "../lib/print_results.c"

int main(int argc, char** argv) {
    return 0;
}

cp_t* leak_array;
int* arr;
int iter;
#define true    1
#define false   0
#define I_INDEX 0
#define J_INDEX 1
#define K_INDEX 2
#define MALLOC_SIZE 4096
#define SECRET "secret"

int i[1000];
int j[1000];
int k[1000];
//training: initialize_index=0 print_index=0
//attack:   initialize_index=2 print_index={iterate through array i}
void uninitialized_read(int initialize_index, int print_index, int* i) {
    char initialized[3];

    //of course, I'd rework this without the branches
    //if      (initialize_index == I_INDEX)	{i[iter] = 1/*[1,4,7...2998]*/; initialized[I_INDEX] = true;}
    //else if (initialize_index == J_INDEX)	{j[iter] = 2/*[2,5,8...2999]*/; initialized[J_INDEX] = true;}
    //else if (initialize_index == K_INDEX)	{k[iter] = 3/*[3,6,9...3000]*/; initialized[K_INDEX] = true;}

    //flush(initilized[I_INDEX]);

    //if(initialized[I_INDEX]) leak_array[i[print_index]];

    //leak_array();//leak_array[i[print_index]];

}

//training: free_index=1 print_index=0
//attack:   free_index=0 print_index={iterate through string SECRET}
void use_after_free(int free_index, int secret_index) {
    char freed[3];

    int* i = calloc(1, MALLOC_SIZE);
    int* j = calloc(1, MALLOC_SIZE);
    int* k = calloc(1, MALLOC_SIZE);

    if      (free_index == I_INDEX)	{free(i); freed[I_INDEX] = true;}
    else if (free_index == J_INDEX)	{free(j); freed[J_INDEX] = true;}
    else if (free_index == K_INDEX)	{free(k); freed[K_INDEX] = true;}


    int* i_dupe = malloc(MALLOC_SIZE);
    *i_dupe = secret_index;

    printf("    \n");
    if(!freed[I_INDEX]) {
        cp_t cp = leak_array[SECRET[*i]];
    }

}//*/