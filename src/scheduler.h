#pragma once

#include <stddef.h>

#include "process_pool.h"

typedef void (*SchedulerExecuteFn)(ProcessPool* pp, double timeout);

typedef struct SchedulerAlgorithm {
    const char* name;
    SchedulerExecuteFn execute;
    double quantum;
} SchedulerAlgorithm;

const SchedulerAlgorithm* scheduler_algorithms(size_t* count);
