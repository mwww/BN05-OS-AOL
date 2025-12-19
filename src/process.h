#pragma once
#include <inttypes.h>
#include <stdbool.h>
#include <time.h>

typedef uint32_t PID;

// Process on a system, use `process_new()` to initialise
typedef struct Process {
  PID pid;
  clock_t time_created;
  clock_t time_executed;
  double priority_weight;
  size_t iterations; // the amount of iterations the process is left to do
} Process;

Process process_new(size_t iterations);
bool process_run(Process *process, double timeout);
void process_reset_pid_counter(void);
