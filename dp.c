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
#include <stdio.h>

// init
// generate random numbers

void init() {
    //Init mutexes
    pthread_mutex_init(&mutex_rand, NULL);
    pthread_mutex_init(&mutex_lock, NULL);

    //Init position index
    rand_position = 0;

    /* seed the random number generator */
    srandom((unsigned)time(NULL));

    for (int i = 0; i < MAX_LENGTH; i++) {
        rand_numbers[i] = random() % 5;
    }

    //Init semaphores
    for (int i = 0; i < NUMBER; i++) {
        sem_init(&sem_vars[i], 0, 1);
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
        sem_post(&sem_vars[i]);
    }
}

void pickup_chopsticks(int i)
{
    sem_wait(&sem_vars[i]);
    pthread_mutex_lock(&mutex_lock);
    state[i] = HUNGRY;
    test(i);
    pthread_mutex_unlock(&mutex_lock);
    sem_post(&sem_vars[i]);
}

void return_chopsticks(int i)
{
    sem_wait(&sem_vars[i]);
    pthread_mutex_lock(&mutex_lock);
    state[i] = THINKING;
    test((i + NUMBER - 1) % NUMBER); // test LEFT
    test((i + 1) % NUMBER);          // test RIGHT
    pthread_mutex_unlock(&mutex_lock);
    sem_post(&sem_vars[i]);
}

void think() {
    sleep(get_next_number());
}

void eat() {
    sleep(get_next_number());
}

void* philosopher(void *param)
{
    int *i = (int *)param;

    int exec = 0;
    while (exec < 5) //Only cycle 5 times
    {
        think();
        printf("Stopped Thinking %d as professor %d", exec+1, *i);
        pickup_chopsticks(*i);
        printf("Eating %d as professor %d", exec+1, *i);
        eat();
        return_chopsticks(*i);
        exec++;
    }
}

int main() {
    init();
    //Make threads and do the stuff
    pthread_t tid[NUMBER];

    for(int i = 0; i < NUMBER; i++) {
        pthread_create(&tid[i], NULL, philosopher, &i);
    }

    for(int i = 0; i < NUMBER; i++) {
        pthread_join(tid[i], NULL);
    }

    return 0;
}