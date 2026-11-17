
[200~# CampusCompute_OS
Intelligent Job Scheduling and Resource Management

## Implemented Features

### FR1 - Job Management
- Create and display jobs.
- Start, pause, and complete jobs.
- Track job status and lifecycle events.

### FR2 - CPU Scheduling
- First Come First Serve (FCFS).
- Round Robin (RR).
- Non-preemptive Priority Scheduling.
- Display execution order and scheduling metrics.

### FR3 - Concurrent Activity Logger
- POSIX threads for producer-consumer operation.
- Bounded circular queue.
- Mutex and condition variables for synchronization.
- Timestamped activity logging.
- Clean thread termination.

### FR4 - Inter-Process Communication
- POSIX shared memory.
- Named semaphore synchronization.
- Parent-child communication using fork().
- IPC resource cleanup.

## Requirements
- GCC compiler
- POSIX threads
- POSIX shared memory and semaphores
- Linux environment

## Compilation

```bash
gcc -Wall -Wextra -std=c11 -pthread main.c job.c scheduler.c logger.c ipc.c -o campuscompute -lrt~
