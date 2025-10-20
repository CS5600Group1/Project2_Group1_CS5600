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
static void enterStair(Tunnel *tunnel, int direction, int id) {
    printf("Customer(%s)[%d] waiting for mutex.\n", direction == UP ? "Up" : "Down", id);
    
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
        printf("Customer(%s)[%d] waiting for direction. (Upstair wait: %d, Downstair wait: %d)\n",
            direction == UP ? "Up" : "Down", id, tunnel->waitingUpstair, tunnel->waitingDownstair);
        pthread_cond_wait(&tunnel->cond, &tunnel->lock);
    }

    if (direction == UP) {
        tunnel->waitingUpstair--;
        tunnel->counterUpstair++;
    } else {
        tunnel->waitingDownstair--;
        tunnel->counterDownstair++;
    }

    printf("Customer(%s)[%d] going up.\n", direction == UP ? "Up" : "Down", id);
    tunnel->dir = direction;
    tunnel->consecutive++;

    // Release lock
    pthread_mutex_unlock(&tunnel->lock);
}

// Thread Functions Definition
static void leaveStair(Tunnel *tunnel, int direction, int id) {
    printf("Customer(%s)[%d] waiting for mutex.\n", direction == UP ? "Up" : "Down", id);
    
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

    printf("Customer(%s)[%d] left.\n", direction == UP ? "Up" : "Down", id);
    
    // Release lock
    pthread_mutex_unlock(&tunnel->lock);
}

// Thread Functions
void threadUpstair(Tunnel *tunnel, int id) {
    // Enter
    enterStair(tunnel, UP, id);

    // upStair
    printf("Customer(Up)[%d] crossing the stairs now.\n", id);
    for (int i = 1; i <= tunnel->step; i++) {
        printf("Customer(Up)[%d] crossing Step %d now.\n", id, i);
        usleep(DURATION);
    }
    printf("Customer(Up)[%d] finished stairs.\n", id);

    // Leave
    leaveStair(tunnel, UP, id);
}

void threadDownstair(Tunnel *tunnel, int id) {
    // Enter
    enterStair(tunnel, DOWN, id);

    // upStair
    printf("Customer(Down)[%d] crossing the stairs now.\n", id);
    for (int i = tunnel->step; i > 0; i--) {
        printf("Customer(Down)[%d] crossing Step %d now.\n", id, i);
        usleep(DURATION);
    }
    printf("Customer(Down)[%d] finished stairs.\n", id);

    // Leave
    leaveStair(tunnel, DOWN, id);
}
