#ifndef PRINT_RESULTS
#define PRINT_RESULTS

#include "header.h"
#include <stdio.h>
#include <stdlib.h>

char** list_cache_hits(int*** results, int REPETITIONS, int SECRET_SIZE, int N_PAGES, int CACHE_HIT) {
    char** cache_hits = malloc(REPETITIONS * sizeof(char*)); //cache_hits[REPETITIONS][SECRET_SIZE]
    for(int r = 0; r < REPETITIONS; r++) {
        cache_hits[r] = malloc(SECRET_SIZE+1 * sizeof(char));
        cache_hits[r][SECRET_SIZE] = '\0'; //ensure ending on zero byte
        printf("Repetition %d\n", r);
        for (int s = 0; s < SECRET_SIZE; s++) {
            printf("char: %3d\t", s);
            int cache_hits_for_this_char = 0;
            for (int p = 0; p < N_PAGES; p++) {
                int t = results[r][s][p];
                if (t < CACHE_HIT) {
                    cache_hits_for_this_char++;
                    cache_hits[r][s] = p;
                    printf("found: '%c'\t", p);
                }
            }
            if(cache_hits_for_this_char != 1) cache_hits[r][s] = '?';
            printf("\n");
        }
        printf("\t\tfound: '%s'\n", cache_hits[r]);
    }
    return cache_hits;
}

int** calculate_character_medians(int*** results, int REPETITIONS, int SECRET_SIZE, int N_PAGES, int CACHE_HIT) {
    int** character_medians = malloc(SECRET_SIZE * sizeof(int*)); //character_medians[SECRET_SIZE][N_PAGES]
    for(int s = 0; s < SECRET_SIZE; s++) {
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
    return character_medians;
}

void print_all_medians(int** character_medians, int SECRET_SIZE, int N_PAGES, int CACHE_HIT) {
    //Print all median loading times using character_medians, also of non cache hits
    for(int s = 0; s < SECRET_SIZE; s++) {
        printf("\n\nCHAR %d\n", s);
        for(int p = 0; p < N_PAGES; p++) {
            int t = character_medians[s][p];
            if (t < CACHE_HIT)  printf("[[%3d:%1c:%4d]]", p, p, t);
            else                printf("  %3d:%1c:%4d  ", p, p, t);
            if(p%8==0 && p!=0) printf("\n");
        }
    }
    printf("\n\n");
}

char* get_leaked_message(int** character_medians, int SECRET_SIZE, int N_PAGES, int CACHE_HIT) {
    //find the lowest median per repetition. If lowest median is
    //too slow to be a cache hit, it is replaced with a '?'.
    //SECRET_SIZE+1 to ensure the string ends on a null-byte.
    char* leaked_message = malloc((SECRET_SIZE+1) * sizeof(char));
    leaked_message[SECRET_SIZE] = 0;
    for(int s = 0; s < SECRET_SIZE; s++) {
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
        leaked_message[s] = index_of_min;

        printf("char%3d:\t'%c'\t(%3d)\n", s, index_of_min, index_of_min);
    }
    return leaked_message;
}

int calculate_median_accuracy(char* leaked, char* SECRET, int SECRET_SIZE) {
    int matches = 0;
    for(int s = 0; s < SECRET_SIZE; s++) {
        if(leaked[s] == SECRET[s]) matches++;
    }
    return matches;
}

int calculate_total_accuracy(char** cache_hits, int REPETITIONS, char* SECRET, int SECRET_SIZE) {
    int matches = 0;
    int total = REPETITIONS * SECRET_SIZE;
    for(int r = 0; r < REPETITIONS; r++) {
        for(int s = 0; s < SECRET_SIZE; s++) {
            if(cache_hits[r][s] == SECRET[s]) matches++;
        }
    }
    return matches;
}

void print_results(int*** results, int REPETITIONS, char* SECRET, int SECRET_SIZE, int N_PAGES, int CACHE_HIT, double measured_time) {

    printf("\nPrinting results...\n");

    //list all cache hits, store in cache_hits.
    //cache_hits[REPETITIONS][SECRET_SIZE]
    char** cache_hits = list_cache_hits(results, REPETITIONS, SECRET_SIZE, N_PAGES, CACHE_HIT);


    //calculate the median loading time per character in character_medians
    //character_medians[SECRET_SIZE][N_PAGES]
    int** character_medians = calculate_character_medians(results, REPETITIONS, SECRET_SIZE, N_PAGES, CACHE_HIT);

    //print_all_medians(character_medians, SECRET_SIZE, N_PAGES, CACHE_HIT);

    //find the lowest median per repetition. If lowest median is
    //too slow to be a cache hit, it is replaced with a '?'.
    char* leaked_message = get_leaked_message(character_medians, SECRET_SIZE, N_PAGES, CACHE_HIT);

    int total_bytes = REPETITIONS*SECRET_SIZE;
    int median_accuracy = calculate_median_accuracy(leaked_message, SECRET, SECRET_SIZE);
    int total_accuracy = calculate_total_accuracy(cache_hits, REPETITIONS, SECRET, SECRET_SIZE);
    long double median_accuracy_percentage = (median_accuracy/(long double)SECRET_SIZE)*100;
    long double total_accuracy_percentage = (total_accuracy/((long double)REPETITIONS*SECRET_SIZE))*100;
    long double byte_per_second = total_bytes / measured_time;
    long double leaked_byte_per_second = total_accuracy / measured_time;

    //grep print
    printf("\nGREP PRINT\n");
    printf("grep_repetitions %d\n", REPETITIONS);
    printf("grep_secret %s\n", SECRET);
    printf("grep_secret size %d\n", SECRET_SIZE);
    printf("grep_measured_time %f\n", measured_time);
    printf("grep_total_bytes %d\n", total_bytes);
    printf("grep_median_accuracy %d\n", median_accuracy);
    printf("grep_total_accuracy %d\n", total_accuracy);
    printf("grep_median_accuracy_percentage %Lf\n", median_accuracy_percentage);
    printf("grep_total_accuracy_percentage %Lf\n", total_accuracy_percentage);
    printf("grep_byte_per_second %Lf\n", byte_per_second);
    printf("grep_leaked_byte_per_second %Lf\n", leaked_byte_per_second);
    printf("END GREP PRINT\n");
    //end grep print*/

    printf("\n\n");
    printf("repetitions:      \t%d\n", REPETITIONS);
    printf("Actual secret:    \t%s\n", SECRET);
    printf("Leaked secret:    \t%s\n", leaked_message);
    printf("Median accuracy:  \t%d/%d\t\t%Lf%%\n", median_accuracy, SECRET_SIZE, median_accuracy_percentage);
    printf("Total accuracy:   \t%d/%d\t\t%Lf%%\n", total_accuracy, total_bytes, total_accuracy_percentage);
    printf("Total time:       \t%f \tseconds\n", measured_time);
    printf("bytes per second: \t%Lf \tbytes / second\n", byte_per_second);
    //printf("                  \t%d \t\tmicroseconds / byte\n", (int)(time_per_byte*1e6));                   //clock() of time.h has microsecond precision only, hence (int)
    printf("Leaked byte per\nsecond (leakage rate):\t%Lf \tbytes per second\n", leaked_byte_per_second);
    //printf("                  \t%d \t\tmicroseconds / byte\n", (int)(time_per_correctly_leaked_byte*1e6));  //clock() of time.h has microsecond precision only, hence (int)

    for(int r = 0; r < REPETITIONS; r++) free(cache_hits[r]);
    for(int s = 0; s < SECRET_SIZE; s++) free(character_medians[s]);
    free(cache_hits);
    free(character_medians);
}

void print_results_arch(char* results, int SECRET_SIZE) {

    for(int r = 0; r < SECRET_SIZE; r++) {
        printf("char%3d:\t'%c'\t(%3d)\n", r, results[r], results[r]);
    }

    printf("\n\nleaked message: %s\n", results);

}

#endif //PRINT_RESULTS