#include <stdio.h>

#include "tui.h"
#include "process.h"
#include "process_pool.h"
#include "terminal.h"

ProcessPool pp;

void* scheduler(void* _);
void* controller(void* _);

int main() {
    tui_init();

    pp = pp_new();

    pthread_t scheduler_thread;
    pthread_t control_thread;

    pthread_create(&scheduler_thread, NULL, scheduler, NULL);
    pthread_create(&control_thread, NULL, controller, NULL);

    pthread_join(control_thread, NULL);
    pthread_cancel(scheduler_thread);

    tui_exit();

    return 0;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
void* scheduler(void* _) {
    while (true) pp_run_round_robin(&pp, 1.0);
}

void* controller(void* _) {
    terminal_init_raw();
    while (true) {
        const char c = (char)getchar();
        if (c == 'n') {
            pp_push(&pp, process_new(8'000'000));

            const int count = (int)pp_total_created();
            const int current_idx = (int)pp_current_process_pid(&pp);
            tui_update_process_list(current_idx, count);

        } else if (c == 'q') {
            terminal_deinit_raw();
            return NULL;
        }
    }
}
#pragma GCC diagnostic pop