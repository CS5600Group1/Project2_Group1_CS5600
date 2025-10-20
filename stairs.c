#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "stairs.h"

// Initialize the Tunnel structure
void stairInit(Tunnel *tunnel, int step) {
    pthread_mutex_init(&tunnel->lock, NULL);
    pthread_cond_init(&tunnel->cond, NULL);
    tunnel->dir = NONE;
    tunnel->step = step;

    tunnel->counterUpstair = 0;
    tunnel->counterDownstair = 0;

    tunnel->waitingUpstair = 0;
    tunnel->waitingDownstair = 0;

    tunnel->consecutive = 0;
}

// Thread Functions Declaration
static void enterStair(Tunnel *tunnel, int direction) {
    printf("Customer(%s)[%lu] waiting for mutex.\n", direction == UP ? "Up" : "Down", GET_PID);
    
    // Acquire lock
    pthread_mutex_lock(&tunnel->lock);

    if (direction == UP) tunnel->waitingUpstair++;
    else tunnel->waitingDownstair++;

    // Wait until can enter
    while (1) {
        if (direction == UP) {
            if (tunnel->dir != DOWN && tunnel->counterDownstair == 0 && tunnel->consecutive <= STARVATION) break;
        } else if (direction == DOWN) {
            if (tunnel->dir != UP && tunnel->counterUpstair == 0 && tunnel->consecutive <= STARVATION) break;
        }
        printf("Customer(%s)[%lu] waiting for direction. (Upstair wait: %d, Downstair wait: %d)\n",
            direction == UP ? "Up" : "Down", GET_PID, tunnel->waitingUpstair, tunnel->waitingDownstair);
        pthread_cond_wait(&tunnel->cond, &tunnel->lock);
    }

    if (direction == UP) {
        tunnel->waitingUpstair--;
        tunnel->counterUpstair++;
    } else {
        tunnel->waitingDownstair--;
        tunnel->counterDownstair++;
    }

    printf("Customer(%s)[%lu] going up.\n", direction == UP ? "Up" : "Down", GET_PID);
    tunnel->dir = direction;
    tunnel->consecutive++;

    // Release lock
    pthread_mutex_unlock(&tunnel->lock);
}

// Thread Functions Definition
static void leaveStair(Tunnel *tunnel, int direction) {
    printf("Customer(%s)[%lu] waiting for mutex.\n", direction == UP ? "Up" : "Down", GET_PID);
    
    // Acquire lock
    pthread_mutex_lock(&tunnel->lock);

    if (direction == UP) tunnel->counterUpstair--;
    else tunnel->counterDownstair--;

    // Check if direction can change
    if ((direction == UP && (tunnel->counterUpstair == 0 || tunnel->consecutive >= STARVATION)) ||
        (direction == DOWN && (tunnel->counterDownstair == 0 || tunnel->consecutive >= STARVATION))) {
        tunnel->consecutive = 0;

        if (direction == UP && tunnel->waitingDownstair > 0) tunnel->dir = DOWN;
        else if (direction == DOWN && tunnel->waitingUpstair > 0) tunnel->dir = UP;
        else tunnel->dir = NONE;

        printf("direction can change %d\n", tunnel->dir);

        pthread_cond_broadcast(&tunnel->cond);
        }

    printf("Customer(%s)[%lu] left.\n", direction == UP ? "Up" : "Down", GET_PID);
    
    // Release lock
    pthread_mutex_unlock(&tunnel->lock);
}

// Thread Functions
void threadUpstair(Tunnel *tunnel) {
    // Enter
    enterStair(tunnel, UP);

    // upStair
    printf("Customer(Up)[%lu] crossing the stairs now.\n", GET_PID);
    for (int i = 1; i <= tunnel->step; i++) {
        printf("Customer(Up)[%lu] crossing Step %d now.\n", GET_PID, i);
        usleep(DURATION);
    }
    printf("Customer(Up)[%lu] finished stairs.\n", GET_PID);

    // Leave
    leaveStair(tunnel, UP);
}

void threadDownstair(Tunnel *tunnel) {
    // Enter
    enterStair(tunnel, DOWN);

    // upStair
    printf("Customer(Down)[%lu] crossing the stairs now.\n", GET_PID);
    for (int i = tunnel->step; i > 0; i--) {
        printf("Customer(Down)[%lu] crossing Step %d now.\n", GET_PID, i);
        usleep(DURATION);
    }
    printf("Customer(Down)[%lu] finished stairs.\n", GET_PID);

    // Leave
    leaveStair(tunnel, DOWN);
}
