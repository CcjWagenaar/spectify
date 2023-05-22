/*#include <stdio.h>


#define I_INDEX 0
#define J_INDEX 1
#define K_INDEX 2

int i[1000]
int j[1000]
int k[1000]
//training: initialize_index=0 print_index=0
//attack:   initialize_index=2 print_index={iterate through array i}
void uninitialized_read(int initialize_index, int print_index) {

    //of course, I'd rework this without the branches
    if      (initialize_index == I_INDEX)	{i = [1,4,7...2998]; initialized[I_INDEX] = true;}
    else if (initialize_index == J_INDEX)	{j = [2,5,8...2999]; initialized[J_INDEX] = true;}
    else if (initialize_index == K_INDEX}	{k = [3,6,9...3000]; initialized[K_INDEX] = true;}

    if(initialized[I_INDEX]) leak_array[i[print_index]];

}



#define MALLOC_SIZE 4096
//training: free_index=1 print_index=0
//attack:   free_index=0 print_index={iterate through string SECRET}
void use_after_free(free_index, secret_index) {

    int* i = malloc(MALLOC_SIZE);
    int* j = malloc(MALLOC_SIZE);
    int* k = malloc(MALLOC_SIZE);

    if      (free_index == I_INDEX)	{free(i); freed[I_INDEX] = true;}
    else if (free_index == J_INDEX)	{free(j); freed[J_INDEX] = true;}
    else if (free_index == K_INDEX)	{free(k); freed[K_INDEX] = true;}

    if(!freed[I_INDEX]) {
        int* i_dupe = malloc(MALLOC_SIZE);
        *i_dupe = secret_index;
    }

    cp_t cp = leak_array[SECRET[*i]];


}*/