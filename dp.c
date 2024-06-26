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
#include <stdio.h>
#include <sys/time.h>

struct timeval before, after;

// init
// generate random numbers
void init(char *filename)
{
    // Init mutexes
    pthread_mutex_init(&mutex_rand, NULL);
    pthread_mutex_init(&mutex_lock, NULL);
    pthread_mutex_init(&mutex_time, NULL);

    // Init position index
    rand_position = 0;

    FILE *myFile;
    myFile = fopen(filename, "r");

    for (int i = 0; i < MAX_LENGTH; i++)
    {
        fscanf(myFile, "%d", &rand_numbers[i]);
    }

    fclose(myFile);

    // Init timing vars
    pthread_mutex_lock(&mutex_time);
    totalEatingTime = 0;
    maxEatingTime = -1;
    pthread_mutex_unlock(&mutex_time);

    // Init semaphores
    for (int i = 0; i < NUMBER; i++)
    {
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

    // Continually try and eat untill succesfull
    while (state[i] == HUNGRY)
    {
        test(i);

        // Check if failed
        if (state[i] == HUNGRY)
        {
            // Giveup lock until signalled then take it back
            pthread_mutex_unlock(&mutex_lock);
            sem_wait(&sem_vars[i]);
            pthread_mutex_lock(&mutex_lock);
        }
    }

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

void think()
{
    sleep((unsigned)get_next_number());
}

void eat()
{
    sleep((unsigned)get_next_number());
}

void *philosopher(void *param)
{
    int *i = (int *)param;

    int exec = 0;
    double waitTime;
    while (exec < 5) // Only cycle 5 times
    {
        think();
        gettimeofday(&before, NULL);
        pickup_chopsticks(*i);
        gettimeofday(&after, NULL);
        waitTime = ((after.tv_usec - before.tv_usec) / 1000.0) +
                   ((after.tv_sec - before.tv_sec) * 1000.0);

        // Timing stuff
        pthread_mutex_lock(&mutex_time);
        totalEatingTime += waitTime;
        if (waitTime > maxEatingTime)
        {
            maxEatingTime = waitTime;
        }
        pthread_mutex_unlock(&mutex_time);

        eat();
        return_chopsticks(*i);
        exec++;
    }
    printf("Philosopher %d is done eating\n", *i);
}

int main()
{
    init("sample1.out");
    // Make threads and do the stuff
    pthread_t tid[NUMBER];
    int *nums[NUMBER];

    int exec = 0;
    while (exec < 5)//Runs 5 times for timing
    {
        for (int i = 0; i < NUMBER; i++)
        {
            nums[i] = (int *)malloc(sizeof(int));
            *nums[i] = i;
            pthread_create(&tid[i], NULL, philosopher, nums[i]);
        }

        for (int i = 0; i < NUMBER; i++)
        {
            pthread_join(tid[i], NULL);
        }

        for (int i = 0; i < NUMBER; i++)
        {
            free(nums[i]);
        }

        exec++;
    }

    pthread_mutex_lock(&mutex_time);
    printf("Avg eating time %f\n", totalEatingTime / (NUMBER * 5.0 * 5));
    printf("Max eating time %f\n", maxEatingTime);
    pthread_mutex_unlock(&mutex_time);

    return 0;
}