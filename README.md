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
    int step;                  // Number of steps (determines sleep time)
    int dir;                   // Current direction: 0=none, 1=up, 2=down
    
    int counterUpstair;        // Number of customers currently on the stairs
    int counterDownstair;      // Number of customers currently on the stairs

    int waitingUpstair;        // Waiting “up” customers
    int waitingDownstair;      // Waiting “down” customers
    
    pthread_mutex_t lock;      // Protects all variables in this struct
    pthread_cond_t cond;       // Condition variable for Single Direction threads

    int consecutive;          // Prevent Starvation by recording how many customers 
                              // have passed consecutively in the same direction
};

/**
 * Initializes the global “tunnel” struct, mutex, and condition variables.
 */
void stairInit(Tunnel *tunnel, int step);

/**
 * Function executed by “up” threads.
 */
void threadUpstair(Tunnel *tunnel, int id);

/**
 * Function executed by “down” threads.
 */
void threadDownstair(Tunnel *tunnel, int id);
```

## **3. Testing**

### **Test Strategy**

We tested the program by running it with various combinations of customer counts and step counts. The program”s log output was manually inspected to verify correct behavior (e.g., no "down" customer started while "up" customers were crossing) and to confirm that all threads eventually finished, which proves the absence of deadlock.

### **Test Cases**

We created several scenarios to trigger potential edge cases:

- **Balanced Load (5 Up, 5 Down)**

    - **Purpose:** To test for efficient switching between directions and measure a baseline average turnaround time.

    - **Result:** The simulation ran smoothly. A batch of “up” threads crossed, followed by a batch of “down” threads, and so on until all threads completed. No deadlock occurred, and switching was efficient.

- **High Contention(Alternating Arrivals)**

    - **Purpose:** Simulates customers arriving in alternating directions (up, down, up, down...) to create maximum contention and test for potential race conditions or deadlocks.

    - **Result:** The first “up” thread set the direction to “up”. The subsequent “down” thread arrived and was correctly forced to wait. When the “up” thread finished, it signaled the “down” thread, which then crossed. This process repeated without any deadlocks.

- **Deadlock Test**

    - **Purpose:** Simulate when two different customers come at same time,test whether it cause dead lock.

    - **Result:** Althrough at same time, due to the automic operation, one of the customer decide the direction and another wait to excute
 
- **Dynamic**

    - **Purpose:** Simulate if some customers arrive while other excuting, test when the system will change direction.

    - **Result:** Customer 2 arrived and wait to excute,if the number of customers entering consecutively has not reached the predetermined limit, even if customer3 arrives after customer2 but is heading in the same direction, customer3 will still enter the staircase first. Only when the limit is reached or the staircase is empty will customer2 be able to enter.
- **Starvation Test (29 Up, 1 Down)**

    - **Purpose:** A stress test to ensure the single “down” customer would eventually get a turn and not be starved by the continuous stream of “up” customers.

    - **Result:** The “up” threads crossed in one or more large batches. After the last “up” thread from the initial wave finished, our starvation-prevention logic correctly gave priority to the waiting “down” thread, which was able to cross successfully before any new “up” threads could start.
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

| Test Case           | Customer Count (Up/Down) | Steps              | Avg. Turnaround Time (sec) |
| ---------------     | ------------------------ | ------------------ | -------------------------- |
| Balanced Load       | 5 Up / 5 Down            | 5                  | 1160.40 ms                 |
| Alternating Arrivals| 5 Up / 5 Down            | 5                  | 1779.70 ms                 |
| Deadlock Test       | 1 Up / 1 Down            | 5                  | 1413.00 ms                 |
| Dynamic             | 9 Up / 6 Down            | 10                 | 3712.13 ms                 |
| Starvation Test     | 29 Up / 1 Down           | 10                 | 5938.50 ms                 |

### **"Efficient" Design**

Our implementation achieves efficiency through:

1. **Concurrent Stair Usage**: Multiple customers going the same direction can use stairs simultaneously (not one-at-a-time), maximizing throughput.

2. **Batch Processing with Threshold**: Direction switches only occur after serving a batch （5 customer）, reducing context switch.

3. **Adaptive Flow**: When one direction has no waiting customers, the other direction continues without unnecessary waits. In Test5 (29/1), not wait 1 up customer wait until other finished, but exchange direction to avoid starvation.

## **6. How to Compile, Run, and Test**

### **Compilation**

The program must be compiled with gcc and linked against the POSIX threads library (`-lpthread`).

To build the project:
```bash
make all
# if you want to build it in windows, use
make all PLATFORM=win
```

### **Running the Program**
```bash
# Usage: ./stairs then input customers and steps of the stair.

# Usage :chmod +x complete_test.sh;
./complete_test.sh

# also if you want to run the test in windows, use
bash ./complete_test.sh win
```
The program will output real-time logs for each customer thread and print the final average turnaround time at the end.

## **7. Group Contributions**

**Jinghao Zheng:**

- Implemented the core `struct tunnel` and the primary mutex/condition variable logic for deadlock prevention.

- Wrote the `threadUpstair` function and the main logic for batching customers.

- Debugged initial concurrency issues and race conditions.

**Changfeng Wang:**

- Wrote the `main` function, including the thread creation/joining loop.

- Implemented the turnaround time calculation and final performance reporting.

- Designed and ran the specific test cases.

**Tianyu Ma:**

- Implemented the starvation prevention logic using the `waiting_up` and `waiting_down` counters and the turn-based signaling.

- Wrote the `threadDownstair` function, mirroring the logic of the `up` threads.

- Wrote this `README.md` document and managed the final project submission.


