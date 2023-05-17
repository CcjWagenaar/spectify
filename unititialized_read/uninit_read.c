#include <stdio.h>
#include <stdlib.h>

#include "../lib/gen_array.c"
#include "../lib/time_and_flush.c"
#include "../lib/print_results.c"

#define N_PAGES 256
#define REPETITIONS 1
#define CACHE_HIT 100
#define MAYBE_CACHE_HIT 175

int i;


int victim_main_func(initialize_index) {

        if(initialize_index  == 1)		i = 2
        else if(initialize_index == 2)	j = 3
        else{ initialize_index == 3}		k = 4

        attacker_func();

        if(initialized_process_finished[i]) victim_subfunc();

}

victim_subfunc() {
    array[i];
}

attacker_func() {
    i = 1;
}

int main(int argc, char** argv) {



}
