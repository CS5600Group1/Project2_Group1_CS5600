# Project2_Group1_CS5600
## Multi-threaded Staircase Simulation

---

### **Group 1 Members**
- Jinghao Zheng
- Changfeng Wang
- Tianyu Ma

---

## **1. Project Overview**

This project simulates customer traffic on a narrow, single-person staircase in a department store. The stairs can only support one-way traffic; if customers try to go up and down simultaneously, a deadlock occurs as neither is willing to back up.

This C program uses **POSIX threads (pthreads)**, **mutexes**, and **condition variables** to manage concurrent access to the stairs.

The solution is designed to:
- **Prevent Deadlock:** Ensure that "up" and "down" threads never get stuck waiting for each other.
- **Prevent Starvation:** Guarantee fairness so that one direction cannot indefinitely block the other.
- **Be Efficient:** Maximize throughput by allowing multiple same-direction customers simultaneously (“batching”).

Each customer is represented by a `pthread` with a unique ID and a randomly assigned direction (up or down). The program calculates individual and average turnaround times.

---

## **2. Implementation**

### **Core Logic**

Our implementation centers around a shared `tunnel` struct protected by a single mutex. It monitors the staircase state, including direction, occupancy, and waiting customers.

#### **Key Components**
- **Shared Data:** Global `tunnel` struct for all shared states.
- **Mutex Lock:** `pthread_mutex_t lock` protects access to `tunnel`.
- **Direction Control:** Integer `dir` tracks direction:
    - `0` → Neutral
    - `1` → Up
    - `-1` → Down
- **Waiting Mechanism:** Two condition variables `upstair` and `downstair` manage sleeping threads efficiently.

If an “up” thread arrives while `dir == 1`, it waits on `upstair`. Similarly, “down” threads wait on `downstair` when `dir == -1`.

#### **Batching Mechanism**
- Threads moving in the same direction increment a shared counter and cross simultaneously.
- The last thread of a batch resets `dir = 0` and signals the opposite direction.

---

### **Functions and Data Structures**

```c
#include <pthread.h>

// Represents the shared staircase resource
struct tunnel {
    int step;           // Number of steps (determines sleep time)
    int dir;            // Current direction: 0=none, 1=up, 2=down
    int counter;        // Number of customers currently on the stairs

    int waiting_up;     // Waiting “up” customers
    int waiting_down;   // Waiting “down” customers
    
    pthread_mutex_t lock;      // Protects all variables in this struct
    pthread_cond_t upstair;    // Condition variable for “up” threads
    pthread_cond_t downstair;  // Condition variable for “down” threads
};

/**
 * Initializes the global “tunnel” struct, mutex, and condition variables.
 */
void tunnelInit(int step);

/**
 * Function executed by “up” threads.
 */
void *threadUpstair(void *arg);

/**
 * Function executed by “down” threads.
 */
void *threadDownstair(void *arg);

/**
 * Main entry point. Handles initialization, thread creation, and result computation.
 */
int main(int argc, char *argv[]);
```

## **3. Testing**

### **Test Strategy**

We tested the program by running it with various combinations of customer counts and step counts. The program”s log output was manually inspected to verify correct behavior (e.g., no "down" customer started while "up" customers were crossing) and to confirm that all threads eventually finished, which proves the absence of deadlock.

### **Test Cases**

We created several scenarios to trigger potential edge cases:

- **Balanced Load (10 Up, 10 Down)**

    - **Purpose:** To test for efficient switching between directions and measure a baseline average turnaround time.

    - **Result:** The simulation ran smoothly. A batch of “up” threads crossed, followed by a batch of “down” threads, and so on until all threads completed. No deadlock occurred, and switching was efficient.

- **Starvation Test (29 Up, 1 Down)**

    - **Purpose:** A stress test to ensure the single “down” customer would eventually get a turn and not be starved by the continuous stream of “up” customers.

    - **Result:** The “up” threads crossed in one or more large batches. After the last “up” thread from the initial wave finished, our starvation-prevention logic correctly gave priority to the waiting “down” thread, which was able to cross successfully before any new “up” threads could start.

- **High Contention / Deadlock Test (Alternating Arrivals)**

    - **Purpose:** Simulates customers arriving in alternating directions (up, down, up, down...) to create maximum contention and test for potential race conditions or deadlocks.

    - **Result:** The first “up” thread set the direction to “up”. The subsequent “down” thread arrived and was correctly forced to wait. When the “up” thread finished, it signaled the “down” thread, which then crossed. This process repeated without any deadlocks.

## **4. Guarantees**

### **Deadlock Prevention**

Deadlock is prevented by adhering to a strict lock acquisition order and by ensuring no circular waits can occur.

- **Single Global Lock:** There is only one mutex (`lock`) that governs access to the shared state (`tunnel`). All threads must acquire this single lock before making a decision. This eliminates the possibility of a thread holding one resource while waiting for another.

- **No Lock Holding While Waiting:** When a thread must wait (e.g., an “up” thread when the dir is “down”), it calls `pthread_cond_wait()`. This function atomically releases the mutex before putting the thread to sleep. Since the thread does not hold any locks while waiting, it cannot be part of a deadlock chain.

### **Starvation Prevention**

Starvation could occur if new “up” threads keep arriving and using the stairs while “down” threads are waiting. As soon as the last “up” thread leaves, a new “up” thread might immediately arrive and grab the lock, resetting the dir to “up” and starving the “down” direction indefinitely.

We prevent this by using the `waiting_up` and `waiting_down` counters to enforce a turn-based system:

When a thread arrives, it increments the appropriate waiting counter before checking if it can proceed.

When the last thread of a batch finishes (e.g., an “up” thread sees `counter == 0`), it checks if `waiting_down > 0`.

If there are threads waiting in the opposite direction, it exclusively signals the other direction (`pthread_cond_broadcast(&downstair)`). It does not signal its own direction.
This "turn-based" handoff ensures that a waiting group of threads will always get the next turn, preventing one direction from monopolizing the stairs.

## **5. Performance and Efficiency**

Turnaround time is measured from the moment a thread is created to the moment it successfully finishes crossing the stairs and exits its thread function.

### **Average Turnaround Time**

| Test Case       | Customer Count (Up/Down) | Steps (Sleep Time) | Avg. Turnaround Time (sec) |
| --------------- | ------------------------ | ------------------ | -------------------------- |
| Balanced Load   | 10 Up / 10 Down          | 10                 | `[Fill in your result]`    |
| Starvation Test | 29 Up / 1 Down           | 10                 | `[Fill in your result]`    |
| High Contention | 15 Up / 15 Down          | 5                  | `[Fill in your result]`    |

### **"Efficient" Design**

Our definition of "efficient" is maximizing throughput by allowing multiple customers on the stairs at once, as long as they are moving in the same direction.

This is achieved with our counter variable. If a customer wants to go "up" and the stairs are already in use for "up" (`dir == 1`), they do not wait. They simply increment counter, release the lock, and cross immediately in parallel with others. This "batching" strategy is far more efficient than a naive design that allows only one person on the stairs at a time, which would serialize all customers regardless of direction. Our design only serializes the groups of opposing traffic, not the individuals within those groups.

## **6. How to Compile, Run, and Test**

### **Compilation**

The program must be compiled with gcc and linked against the POSIX threads library (`-lpthread`).
```c
# Compile the program (assuming your file is main.c)
gcc -o stairs_sim main.c -lpthread -std=c11
```

### **Running the Program**
```c
# Usage: ./stairs_sim [NUMBER_OF_CUSTOMERS] [NUMBER_OF_STEPS]

# Example: Run simulation with 20 customers and 10 steps
./stairs_sim 20 10
```
The program will output real-time logs for each customer thread and print the final average turnaround time at the end.

## **7. Group Contributions**

**Jinghao Zheng:**

- Implemented the core `struct tunnel` and the primary mutex/condition variable logic for deadlock prevention.

- Wrote the `threadUpstair` function and the main logic for batching customers.

- Debugged initial concurrency issues and race conditions.

**Changfeng Wang:**

- Wrote the `main` function, including command-line argument parsing and the thread creation/joining loop.

- Implemented the turnaround time calculation and final performance reporting.

- Designed and ran the specific test cases for balanced loads and high contention.

**Tianyu Ma:**

- Implemented the starvation prevention logic using the `waiting_up` and `waiting_down` counters and the turn-based signaling.

- Wrote the `threadDownstair` function, mirroring the logic of the `up` threads.

- Wrote this `README.md` document and managed the final project submission.


