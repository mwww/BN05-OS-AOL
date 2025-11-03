#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ansi_colors.h"
#include "process.h"
#include "process_pool.h"
#include "terminal.h"

ProcessPool pp;

void* scheduler(void* _);
void* controller(void* _);

int main() {
    printf("\x1b[2J");    // clear screen
    printf("\x1b[?25l");  // hide cursor
    printf(
        "\x1b[H"
        "╭ Round-robin ─╮\n"
        "│              │\n"
        "│              │\n"
        "│              │\n"
        "│              │\n"
        "│              │\n"
        "│              │\n"
        "│              │\n"
        "│              │\n"
        "│              │\n"
        "│              │\n"
        "│              │\n"
        "│              │\n"
        "╰ [n]ew [q]uit ╯\n"
    );
    pp = pp_new();

    pthread_t scheduler_thread;
    pthread_t control_thread;

    pthread_create(&scheduler_thread, NULL, scheduler, NULL);
    pthread_create(&control_thread, NULL, controller, NULL);

    pthread_join(control_thread, NULL);
    pthread_cancel(scheduler_thread);
    printf(RESET);        // reset color
    printf("\x1b[?25h");  // show cursor
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
void* scheduler(void* _) {
    while (true) pp_run_round_robin(&pp, 1.0);
}

void* controller(void* _) {
    terminal_init_raw();
    while (true) {
        char c = getchar();
        if (c == 'n') {
            pp_push(&pp, process_new(8'000'000));
        } else if (c == 'q') {
            terminal_deinit_raw();
            return NULL;
        }
    }
}
#pragma GCC diagnostic pop