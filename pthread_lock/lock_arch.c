#include <stdio.h>
#include <stdlib.h>

#include "../lib/gen_array.c"
#include "../lib/time_and_flush.c"
#include "../lib/print_results.c"

#define SECRET_SIZE 9
#define SECRET      "mysecret"
#define DBG         0

void attack_func(/*pthread_mutex_t* lock_ptr,*/ int* lock_var_ptr, int secret_index) {
    //if(pthread_mutex_trylock(lock_ptr) != 0) return;
    *lock_var_ptr = secret_index;
    //pthread_mutex_unlock(lock_ptr);
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
    //pthread_mutex_t lock;
    //pthread_mutex_init(&lock, FLAG);
    int lock_var;

    //pthread_mutex_lock(&lock0);
    lock_var = 0;

    attack_func(/*&lock,*/ &lock_var, secret_index);

    char s = SECRET[lock_var];
    //pthread_mutex_unlock(&lock);
    //pthread_mutex_destroy(&lock);
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