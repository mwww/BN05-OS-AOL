#include "process_pool.h"

#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
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

Process pp_get(ProcessPool* pp, size_t index) {
    pthread_mutex_lock(&pp->mutex);
    Process process = pp->processes[index];
    pthread_mutex_unlock(&pp->mutex);
    return process;
}

void pp_push(ProcessPool* pp, Process p) {
    pthread_mutex_lock(&pp->mutex);
    if (pp->length == pp->capacity) {
        pp->capacity *= 2;
        pp->processes = realloc(pp->processes, pp->capacity * sizeof(Process));
    }

    pp->processes[pp->length++] = p;
    pthread_mutex_unlock(&pp->mutex);
}

void pp_remove(ProcessPool* pp, size_t index) {
    pthread_mutex_lock(&pp->mutex);
    size_t len = pp->length;
    for (size_t i = index + 1; i < len; i++) {
        pp->processes[i - 1] = pp->processes[i];
    }
    pp->length -= 1;
    pthread_mutex_unlock(&pp->mutex);
}

size_t pp_total_created() {
    extern PID _process_last_pid;
    return _process_last_pid;
}

PID pp_current_process_pid(ProcessPool* pp) {
    pthread_mutex_lock(&pp->mutex);
    PID pid = pp->current_process_pid;
    pthread_mutex_unlock(&pp->mutex);
    return pid;
}

void pp_run_round_robin(ProcessPool* pp, double timeout) {
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

        // wait for a bit of time to let insertion happen
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
