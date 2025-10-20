//
// Created by zheng on 2025/10/19.
//

#ifndef TUNNEL_H
#define TUNNEL_H

#include <pthread.h>

#define DOWN -1
#define NONE 0
#define UP 1

#define STARVATION 5

#define GET_PID (long unsigned int)pthread_self()

#define DURATION 100000 + rand() % 200000

// Tunnel Structure Definition
typedef struct {
    int step;                   // Number of steps (determines sleep time)
    int dir;                    // Current direction: 0=none, 1=up, 2=down

    int counterUpstair;         // Number of customers currently on the stairs
    int counterDownstair;       // Number of customers currently on the stairs

    int waitingUpstair;         // Waiting “up” customers
    int waitingDownstair;       // Waiting “down” customers

    pthread_mutex_t lock;       // Protects all variables in this struct
    pthread_cond_t cond;        // Condition variable for Single Direction threads

    int consecutive;            // Prevent Starvation by recording how many customers
                                // have passed consecutively in the same direction
} Tunnel;

void stairInit(Tunnel *tunnel, int step);
void threadUpstair(Tunnel *tunnel, int id);
void threadDownstair(Tunnel *tunnel, int id);

#endif //TUNNEL_H
