#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <pthread.h>

#include "process.h"

typedef struct ProcessPool {
    Process* processes;
    size_t length;
    size_t capacity;
    pthread_mutex_t mutex;
} ProcessPool;

ProcessPool pp_new();
Process pp_get(ProcessPool* pp, size_t index);
void pp_push(ProcessPool* pp, Process p);
void pp_remove(ProcessPool* pp, size_t index);
void pp_run_round_robin(ProcessPool* pp, double timeout);
