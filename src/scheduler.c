#include "scheduler.h"

#include "process_pool.h"

static const SchedulerAlgorithm SCHEDULER_ALGORITHMS[] = {
    {"Round Robin", pp_run_round_robin, 1.0},
    {"First Come First Serve", pp_run_fcfs, 0.0},
    {"Completely Fair Scheduler", pp_run_cfs, 0.02},
};

const SchedulerAlgorithm *scheduler_algorithms(size_t *count) {
  if (count) {
    *count = sizeof(SCHEDULER_ALGORITHMS) / sizeof(*SCHEDULER_ALGORITHMS);
  }
  return SCHEDULER_ALGORITHMS;
}
