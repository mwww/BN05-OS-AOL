#include "process.h"

#include <stdio.h>
#include <stdlib.h>

PID _process_last_pid = 0;

Process process_new(size_t iterations) {
    return (Process){
        .pid = _process_last_pid++,
        .iterations = iterations,
        .time_created = clock(),
        .time_executed = 0,
    };
}

bool process_run(Process* process, double timeout) {
    clock_t start = clock();
    printf("[%u] started\n", process->pid);

    int64_t randsum = 0;
    while (process->iterations-- > 0) {
        randsum += rand() % 2;
        double secs_since_start = ((double)(clock() - start)) / CLOCKS_PER_SEC;
        if (secs_since_start > timeout) {
            printf("[%u] paused\n", process->pid);
            return false;
        }
    }

    printf("[%u] finished\n", process->pid);
    process->time_executed += clock() - start;
    return true;
}