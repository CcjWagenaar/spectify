#ifndef PRINT_RESULTS
#define PRINT_RESULTS

//#include "../bounds_check_bypass/config.h"
#include "header.h"
/* USES
 *  REPETITIONS
 *  secret_size
 *  N_PAGES
 *  CACHE_HIT
 */

int cmpfunc (const void * a, const void * b) {
    return ( *(int*)a - *(int*)b );
}

void print_results(int*** results, int REPETITIONS, int secret_size, int N_PAGES, int CACHE_HIT) {

    for(int r = 0; r < REPETITIONS; r++) {
        printf("Repetition %d\n", r);
        for (int s = 0; s < secret_size; s++) {
            printf("char: %3d\t", s);
            for (int p = 0; p < N_PAGES; p++) {
                int t = results[r][s][p];
                if (t < CACHE_HIT) printf("found: '%c'\t", p);
            }
            printf("\n");
        }
    }

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
        printf("\n\nCHAR %d\n", s);
        for(int p = 0; p < N_PAGES; p++) {
            int t = character_medians[s][p];
            if (t < CACHE_HIT)  printf("[[%3d:%1c:%4d]]", p, p, t);
            else                printf("  %3d:%1c:%4d  ", p, p, t);
            if(p%8==0 && p!=0) printf("\n");
        }
    }
    printf("\n\n");

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

        printf("char%3d:\t'%c'\t(%3d)\n", s, index_of_min, index_of_min);


    }

    printf("\n\nleaked message: %s\n", secret_message);

}

#endif //PRINT_RESULTS