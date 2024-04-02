/**
 * Author: Garrett Martin, Sammy Beers
 * Course: COMP 340, Operating Systems
 * Date: 2 April 2024
 * Description: This file implements the functionality
 *              For Program 1, Task 1
 * Compile with: gcc -o dp dp.c dp.h
 * Run with: ./dp
*/
#include "dp.h"
#include <sys/time.h>

// init
// generate random numbers

void init() {
    //Init position index
    rand_position = 0;

    /* seed the random number generator */
    srandom((unsigned)time(NULL));

    for (int i = 0; i < MAX_LENGTH; i++) {
        rand_numbers[i] = random();
    }
}


int get_next_number()
{
    pthread_mutex_lock(&mutex_rand);
    int i = rand_position;
    int next_number = rand_numbers[i];
    rand_position = (i + 1) % MAX_LENGTH;
    pthread_mutex_unlock(&mutex_rand);
    return next_number;
}

void test(int i)
{
    if (state[i] == HUNGRY &&                         // want to compete
        state[(i + NUMBER - 1) % NUMBER] != EATING && // LEFT
        state[(i + 1) % NUMBER] != EATING)            // RIGHT
    {
        state[i] = EATING;
        //TODO: IDK how to signal
    }
}

void pickup_chopsticks(int i)
{
    pthread_mutex_lock(&mutex_lock);
    // TODO: needs work
    state[i] = HUNGRY;
    //TODO: IDK signal
    pthread_mutex_unlock(&mutex_lock);
}

void return_chopsticks(int i)
{
    pthread_mutex_lock(&mutex_lock);
    state[i] = THINKING;
    test((i + NUMBER - 1) % NUMBER); // test LEFT
    test((i + 1) % NUMBER);          // test RIGHT
    //TODO: IDK signal
    pthread_mutex_unlock(&mutex_lock);
}

void think() {

}

void *philosopher(void *param)
{
    int *i = (int *)param;

    int exec = 0;
    while (exec < 5) //Only cycle 5 times
    {
        // think
        pickup_chopsticks(*i);
        // eat
        return_chopsticks(*i);
        exec++;
    }
}

int main() {
    //Make threads and do the stuff
    return 0;
}