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
    pp->processes[index] = pp->processes[--pp->length];
    pthread_mutex_unlock(&pp->mutex);
}

void pp_run_round_robin(ProcessPool* pp, double timeout) {
    size_t current_process = 0;
    while (true) {
        pthread_mutex_lock(&pp->mutex);
        bool is_complete = process_run(&pp->processes[current_process], timeout);
        pthread_mutex_unlock(&pp->mutex);

        if (is_complete) {
            pp_remove(pp, current_process);
        } else {
            current_process += 1;
        }
        pthread_mutex_lock(&pp->mutex);
        if (pp->length == 0) {
            pthread_mutex_unlock(&pp->mutex);
            return;
        }
        current_process %= pp->length;
        pthread_mutex_unlock(&pp->mutex);
    }
}