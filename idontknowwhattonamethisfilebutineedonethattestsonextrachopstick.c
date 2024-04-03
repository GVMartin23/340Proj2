/**
 * Author: Garrett Martin, Sammy Beers
 * Course: COMP 340, Operating Systems
 * Date: 2 April 2024
 * Description: This file implements the functionality
 *              For Program 1, Task 2
 * Compile with: gcc -o dpp1 [nameoffile].c dpp1.h
 * Run with: ./dpp1
 */
#include "dpp1.h"
#include <stdio.h>

// init
// generate random numbers

void init(char *filename)
{
    // Init mutexes
    pthread_mutex_init(&mutex_rand, NULL);
    pthread_mutex_init(&middle_chop, NULL);

    // Init position index
    rand_position = 0;

    // Init middle chopstick
    middleAvailable = true;

    FILE *myFile;
    myFile = fopen(filename, "r");

    for (int i = 0; i < MAX_LENGTH; i++)
    {
        fscanf(myFile, "%d", &rand_numbers[i]);
    }

    fclose(myFile);

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
    pthread_mutex_lock(&middle_chop);
    if (state[i] == HUNGRY &&                         // want to compete
        state[(i + NUMBER - 1) % NUMBER] != EATING && // LEFT
        state[(i + 1) % NUMBER] != EATING)            // RIGHT
    {
        state[i] = EATING;
        sem_post(&sem_vars[i]);
    }
    else if (state[i] == HUNGRY &&
                 (state[(i + NUMBER - 1) % NUMBER] != EATING &&
                  middleAvailable) ||
             (state[(i + 1) % NUMBER] != EATING &&
              middleAvailable))
    {
        state[i] = EATING;
        sem_post(&sem_vars[i]);
        middleAvailable = false;
    }
    pthread_mutex_unlock(&middle_chop);
}

int pickup_chopsticks(int i)
{
    int success = 0;
    sem_wait(&sem_vars[i]);
    pthread_mutex_lock(&mutex_lock);
    state[i] = HUNGRY;
    test(i);
    if (state[i] == EATING)
    {
        success = 1;
    }
    pthread_mutex_unlock(&mutex_lock);
    sem_post(&sem_vars[i]);
    return success;
}

void return_chopsticks(int i)
{
    sem_wait(&sem_vars[i]);
    pthread_mutex_lock(&middle_chop);
    middleAvailable = true;
    pthread_mutex_unlock(&middle_chop);
    pthread_mutex_lock(&mutex_lock);
    state[i] = THINKING;
    for (int j = 0; j < NUMBER; j++)
    {
        if (j != i)
            test(j);
    }
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
    while (exec < 5) // Only cycle 5 times
    {
        think();
        if (pickup_chopsticks(*i))
        {
            eat();
            return_chopsticks(*i);
            exec++;
        }
    }
    printf("Professor %d is done eating\n", *i);
}

int main()
{
    init("sample1.out");
    // Make threads and do the stuff
    pthread_t tid[NUMBER];
    int *nums[NUMBER];

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

    return 0;
}