#pragma once

#include <pthread.h>
#include <stddef.h>

#include "process.h"

typedef struct ProcessPool {
  Process *processes;
  size_t length;
  size_t capacity;
  PID current_process_pid;
  pthread_mutex_t mutex;
} ProcessPool;

ProcessPool pp_new();
Process pp_get(ProcessPool *pp, size_t index);
void pp_push(ProcessPool *pp, Process p);
void pp_remove(ProcessPool *pp, size_t index);
void pp_clear(ProcessPool *pp);
void pp_destroy(ProcessPool *pp);
size_t pp_total_created();
PID pp_current_process_pid(ProcessPool *pp);
void pp_run_round_robin(ProcessPool *pp, double timeout);
void pp_run_fcfs(ProcessPool *pp, double timeout);
void pp_run_cfs(ProcessPool *pp, double timeout);
