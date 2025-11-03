#include "process.h"
#include "tui.h"

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
    tui_display_process_running(process->pid);

    // int64_t randsum = 0;
    while (process->iterations-- > 0) {
        // randsum += rand() % 2; // @jason ini buat apa?
        double secs_since_start = ((double)(clock() - start)) / CLOCKS_PER_SEC;
        if (secs_since_start > timeout) {
            tui_display_process_paused(process->pid);
            return false;
        }
    }

    tui_display_process_finished(process->pid);
    process->time_executed += clock() - start;
    return true;
}