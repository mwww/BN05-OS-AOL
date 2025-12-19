#include "process_pool.h"

#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#include "process.h"

ProcessPool pp_new() {
  ProcessPool pp = (ProcessPool){
      .processes = calloc(1, sizeof(Process)),
      .length = 0,
      .capacity = 1,
      .current_process_pid = 0,
  };
  pthread_mutex_init(&pp.mutex, NULL);
  return pp;
}

Process pp_get(ProcessPool *pp, size_t index) {
  pthread_mutex_lock(&pp->mutex);
  Process process = pp->processes[index];
  pthread_mutex_unlock(&pp->mutex);
  return process;
}

void pp_push(ProcessPool *pp, Process p) {
  pthread_mutex_lock(&pp->mutex);
  if (pp->length == pp->capacity) {
    pp->capacity *= 2;
    pp->processes = realloc(pp->processes, pp->capacity * sizeof(Process));
  }

  pp->processes[pp->length++] = p;
  pthread_mutex_unlock(&pp->mutex);
}

void pp_remove(ProcessPool *pp, size_t index) {
  pthread_mutex_lock(&pp->mutex);
  size_t len = pp->length;
  for (size_t i = index + 1; i < len; i++) {
    pp->processes[i - 1] = pp->processes[i];
  }
  pp->length -= 1;
  pthread_mutex_unlock(&pp->mutex);
}

void pp_clear(ProcessPool *pp) {
  pthread_mutex_lock(&pp->mutex);
  pp->length = 0;
  pp->current_process_pid = 0;
  pthread_mutex_unlock(&pp->mutex);
}

void pp_destroy(ProcessPool *pp) {
  if (pp == NULL) {
    return;
  }

  pthread_mutex_destroy(&pp->mutex);
  free(pp->processes);
  pp->processes = NULL;
  pp->length = 0;
  pp->capacity = 0;
  pp->current_process_pid = 0;
}

size_t pp_total_created() {
  extern PID _process_last_pid;
  return _process_last_pid;
}

PID pp_current_process_pid(ProcessPool *pp) {
  pthread_mutex_lock(&pp->mutex);
  PID pid = pp->current_process_pid;
  pthread_mutex_unlock(&pp->mutex);
  return pid;
}

void pp_run_round_robin(ProcessPool *pp, double timeout) {
  size_t current_process = 0;
  while (true) {
    pthread_mutex_lock(&pp->mutex);
    if (pp->length == 0) {
      pthread_mutex_unlock(&pp->mutex);
      return;
    }
    pp->current_process_pid = pp->processes[current_process].pid;
    bool is_complete = process_run(&pp->processes[current_process], timeout);
    pthread_mutex_unlock(&pp->mutex);

    // wait for a lil bit of time to let insertion happen
    for (size_t i = 0; i < 10000; i++) {
      rand();
    }

    if (is_complete) {
      pp_remove(pp, current_process);
      if (pp->length == 0) {
        return;
      }
    } else {
      current_process += 1;
    }
    pthread_mutex_lock(&pp->mutex);
    current_process %= pp->length;
    pthread_mutex_unlock(&pp->mutex);
  }
}

void pp_run_fcfs(ProcessPool *pp, double timeout) {
  while (true) {
    pthread_mutex_lock(&pp->mutex);
    if (pp->length == 0) {
      pthread_mutex_unlock(&pp->mutex);
      return;
    }
    pp->current_process_pid = pp->processes[0].pid;
    bool is_complete = process_run(&pp->processes[0], timeout);
    pthread_mutex_unlock(&pp->mutex);

    if (is_complete) {
      pp_remove(pp, 0);
    }
  }
}

void pp_run_cfs(ProcessPool *pp, double target_latency) {
  const double MIN_GRANULARITY = 0.002; // default target latency in seconds

  if (target_latency <= 0) {
    target_latency = 0.02; // INGET BAL INI SECOND NOT MS NOT MINUTES PLS JANGAN LUPA!!!!!!
  }

  while (true) {
    pthread_mutex_lock(&pp->mutex);
    size_t len = pp->length;
    if (len == 0) {
      pthread_mutex_unlock(&pp->mutex);
      return;
    }

    double total_weight = 0.0;
    for (size_t i = 0; i < len; ++i) {
      total_weight += pp->processes[i].priority_weight;
    }

    // pick process with the smallest weighted virtual runtime (time_executed / weight)
    // ts lowkey cool af, jadi si schedulernya bakal "prioritize" each process based on their weight!!!
    size_t next_index = 0;
    double min_vruntime = (double)pp->processes[0].time_executed /
                          pp->processes[0].priority_weight;
    for (size_t i = 1; i < len; ++i) {
      double vruntime = (double)pp->processes[i].time_executed /
                        pp->processes[i].priority_weight;
      if (vruntime < min_vruntime ||
          (vruntime == min_vruntime &&
           pp->processes[i].priority_weight >
               pp->processes[next_index].priority_weight) ||
          (vruntime == min_vruntime &&
           pp->processes[i].priority_weight ==
               pp->processes[next_index].priority_weight &&
           pp->processes[i].time_created <
               pp->processes[next_index].time_created)) {
        min_vruntime = vruntime;
        next_index = i;
      }
    }

    double time_slice =
        target_latency *
        (pp->processes[next_index].priority_weight / total_weight);
    if (time_slice < MIN_GRANULARITY) {
      time_slice = MIN_GRANULARITY;
    }

    pp->current_process_pid = pp->processes[next_index].pid;
    bool is_complete = process_run(&pp->processes[next_index], time_slice);
    pthread_mutex_unlock(&pp->mutex);

    if (is_complete) {
      pp_remove(pp, next_index);
    }
  }
}
