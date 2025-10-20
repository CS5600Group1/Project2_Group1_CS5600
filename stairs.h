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

typedef struct {
    int step;
    int dir; // (-1: down, 0: none, 1: up)

    pthread_mutex_t lock;
    pthread_cond_t cond;

    int counterUpstair;
    int counterDownstair;

    int waitingUpstair;
    int waitingDownstair;

    int consecutive;

} Tunnel;

void stairInit(Tunnel *tunnel, int step);
void threadUpstair(Tunnel *tunnel);
void threadDownstair(Tunnel *tunnel);

#endif //TUNNEL_H
