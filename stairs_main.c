#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

#include "stairs.h"

#ifndef DURATION
#define DURATION 0
#endif

// Thread Info Structure
typedef struct {
    int id;
    int dir;
    int delay;
    Tunnel *tunnel;
} ThreadINFO;

// Global Variables
pthread_t threads[100];
ThreadINFO infos[100];
Tunnel tunnel;
long turnaroundTime[100];

// Get current time in milliseconds
long now_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

// Thread Functions
void *threadA(void* arg)
{
    ThreadINFO *info = arg;
    Tunnel *tunnel = info->tunnel;
    usleep(info->delay * 1000);

    long actual_start= now_ms();

    threadUpstair(tunnel, info->id);

    long actual_end = now_ms();
    turnaroundTime[info->id] = actual_end-actual_start;

    return NULL;
}

void *threadB(void* arg)
{
    ThreadINFO *info = arg;
    Tunnel *tunnel = info->tunnel;
    usleep(info->delay * 1000);

    long actual_start= now_ms();

    threadDownstair(tunnel, info->id);

    long actual_end = now_ms();
    turnaroundTime[info->id] = actual_end-actual_start;

    return NULL;
}

// Main Function
int main()
{
    srand(time(NULL));

    // Input Handler
    int customer, step;
    scanf("%d %d", &customer, &step);

    stairInit(&tunnel, step);
    
    // create threads
    for (int i = 0; i < customer; i++) {
        int id, dir, delay;
        scanf("%d %d %d", &id, &dir, &delay);
        infos[i].id = id;
        infos[i].dir = dir;
        infos[i].delay = delay;
        infos[i].tunnel = &tunnel;
    }

    for (int i = 0; i < customer; i++) {
        if (infos[i].dir == UP) {
            pthread_create(threads+i, NULL, threadA, infos+i);
        }
        else if (infos[i].dir == DOWN) {
            pthread_create(threads+i, NULL, threadB, infos+i);
        }
    }

    // join
    for (int i = 0; i < customer; i++)
        pthread_join(threads[i], NULL);


    // average turnaround time
    long sum = 0;
    for(int i=0; i<customer; i++) {
        sum += turnaroundTime[i];
    }
    printf("Average Turnaround: %.2f ms\n", sum*1.0/customer);

    return 0;
}