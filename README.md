# CPU Scheduling Simulator

## Overview
This program simulates a Multi-Level Feedback Queue (MLFQ) scheduling policy on a multi-CPU system. It's designed to emulate the behavior of scheduling tasks on multiple CPUs using threads, where each CPU is represented by a thread. The program compares the performance of different scheduling policies using this simulation.

## Features
- Implements MLFQ scheduling policies.
- Simulates multi-CPU system using threads.
- Each "CPU" thread handles one task at a time.
- Supports I/O-bound and CPU-bound tasks. 
- Includes functionality to simulate I/O operations and task sleeping.
- Priority levels and quantum lengths are configurable.
- Measures and reports average turnaround time and response time for each task type.

## Requirements
- C compiler (GCC recommended)
- POSIX compliant environment
- `sys/queue.h` for queue operations (included in most Unix-like systems)

## Configuration
- **Priority Levels**: 3
- **Quantum Length**: 50 microseconds
- **Time Allotment**: 200 microseconds before lowering a task's priority
- **S Value**: Time value for moving jobs to the topmost queue (modifiable for system analysis)

## Compilation
Compile the program with GCC or a similar C compiler. For example:
```bash
gcc -o cpuScheduling cpuScheduling.c -lpthread
```
or with make:
```bash
make
```

## USAGE
The program requires three command-line arguments:

1. **Number of CPUs:** Number of simulated CPUs (threads).
2. **S Value:** Time in milliseconds between boosting all jobs to the topmost queue.
3. **Task File Name:** Name of the file containing the workload for the simulation.

## Example Command
```bash
./cpuScheduling 4 5000 tasks.txt
```

## Make Run

With 1,2,3,4 CPUs, and 1000ms, 3000ms, 5000ms, 7000ms boost time, on tasks.txt:
```bash
or, with make:
```bash
make run
```



This runs the simulator with 4 CPU threads, a boost time of 5000ms, and tasks loaded from tasks.txt.

## Input File Format
The workload file should contain tasks with the following space-delimited format:

```
task_name task_type task_length odds_of_IO
```

Where:
* **task_name:** Name of the task.
* **task_type:** Type of the task (0 for short, 1 for medium, 2 for long, 3 for I/O tasks).
* **task_length:** Length of the task in microseconds.
* **odds_of_IO:** Percentage chance of the task performing an I/O operation.


## DELAY Command
Use DELAY <milliseconds> to introduce delays in task processing.

Output
The program outputs the average turnaround time and response time for each type of task upon completion.

## Report

## Report and Analysis

Compute an average turnaround and response time for each type of task for each scheduling policy for these settings.

### CPU Count: 1

| S Value | Type | Avg Turnaround | Avg Response |
|---------|------|----------------|--------------|
| 1000    | 0    | 211573.63      | 211800.37    |
| 1000    | 1    | 216954.19      | 217151.24    |
| 1000    | 2    | 187531.72      | 187372.31    |
| 1000    | 3    | 263531.32      | 263551.00    |
|---------|------|----------------|--------------|
| 3000    | 0    | 213752.21      | 214068.42    |
| 3000    | 1    | 215071.48      | 215352.52    |
| 3000    | 2    | 196871.53      | 196866.75    |
| 3000    | 3    | 258580.23      | 258657.68    |
|---------|------|----------------|--------------|
| 5000    | 0    | 242511.11      | 242807.47    |
| 5000    | 1    | 244163.57      | 244430.76    |
| 5000    | 2    | 229267.22      | 229254.97    |
| 5000    | 3    | 300189.14      | 300312.59    |
|---------|------|----------------|--------------|
| 7000    | 0    | 207677.32      | 207915.79    |
| 7000    | 1    | 222806.52      | 223016.76    |
| 7000    | 2    | 191589.69      | 191515.25    |
| 7000    | 3    | 255252.59      | 255304.59    |

---

### CPU Count: 2

| S Value | Type | Avg Turnaround | Avg Response |
|---------|------|----------------|--------------|
| 1000    | 0    | 431768.83      | 545632.74    |
| 1000    | 1    | 464672.93      | 641883.67    |
| 1000    | 2    | 490125.16      | 581324.88    |
| 1000    | 3    | 491415.29      | 624997.64    |
|---------|------|----------------|--------------|
| 3000    | 0    | 383883.00      | 545735.63    |
| 3000    | 1    | 433415.32      | 639973.76    |
| 3000    | 2    | 448103.69      | 489432.31    |
| 3000    | 3    | 524434.36      | 557662.68    |
|---------|------|----------------|--------------|
| 5000    | 0    | 379836.21      | 480066.89    |
| 5000    | 1    | 375611.95      | 393706.33    |
| 5000    | 2    | 392070.11      | 428059.16    |
| 5000    | 3    | 486757.39      | 508347.09    |
|---------|------|----------------|--------------|
| 7000    | 0    | 366202.96      | 462865.84    |
| 7000    | 1    | 394342.41      | 507255.33    |
| 7000    | 2    | 462641.64      | 594366.91    |
| 7000    | 3    | 515516.71      | 561924.32    |

---

### CPU Count: 3

| S Value | Type | Avg Turnaround | Avg Response |
|---------|------|----------------|--------------|
| 1000    | 0    | 388006.74      | 633399.68    |
| 1000    | 1    | 419197.79      | 758816.90    |
| 1000    | 2    | 463401.57      | 679820.75    |
| 1000    | 3    | 476238.34      | 887144.41    |
|---------|------|----------------|--------------|
| 3000    | 0    | 424029.16      | 712738.95    |
| 3000    | 1    | 399145.12      | 627539.05    |
| 3000    | 2    | 455480.15      | 613403.09    |
| 3000    | 3    | 513770.23      | 700342.05    |
|---------|------|----------------|--------------|
| 5000    | 0    | 433288.04      | 547534.42    |
| 5000    | 1    | 523111.63      | 797303.90    |
| 5000    | 2    | 528513.28      | 658772.19    |
| 5000    | 3    | 491667.90      | 669676.36    |
|---------|------|----------------|--------------|
| 7000    | 0    | 460779.39      | 873390.74    |
| 7000    | 1    | 414362.86      | 848734.24    |
| 7000    | 2    | 482593.55      | 798375.84    |
| 7000    | 3    | 523083.25      | 570250.95    |

---

### CPU Count: 4

| S Value | Type | Avg Turnaround | Avg Response |
|---------|------|----------------|--------------|
| 1000    | 0    | 442996.37      | 1259304.95   |
| 1000    | 1    | 482773.17      | 921597.38    |
| 1000    | 2    | 437272.31      | 873597.25    |
| 1000    | 3    | 505211.70      | 1055764.00   |
|---------|------|----------------|--------------|
| 3000    | 0    | 479745.77      | 1187141.37   |
| 3000    | 1    | 363049.77      | 761021.10    |
| 3000    | 2    | 413971.05      | 730420.94    |
| 3000    | 3    | 510378.34      | 881271.36    |
|---------|------|----------------|--------------|
| 5000    | 0    | 367537.21      | 708776.42    |
| 5000    | 1    | 413579.12      | 904013.00    |
| 5000    | 2    | 423913.44      | 846142.66    |
| 5000    | 3    | 485275.77      | 820212.45    |
|---------|------|----------------|--------------|
| 7000    | 0    | 401477.38      | 845619.95    |
| 7000    | 1    | 424323.40      | 606522.95    |
| 7000    | 2    | 465334.50      | 901456.47    |
| 7000    | 3    | 467999.74      | 851627.55    |



