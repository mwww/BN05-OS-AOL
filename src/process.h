#pragma once
#include <inttypes.h>
#include <time.h>
#include <stdbool.h>

typedef uint32_t PID;

// Process on a system, use `process_new()` to initialise
typedef struct Process {
    PID pid;
    clock_t time_created;
    clock_t time_executed;
    size_t iterations; // the amount of iterations the process is left to do 
} Process;

Process process_new(size_t iterations);
bool process_run(Process* process, double timeout);