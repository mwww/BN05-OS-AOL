#include <stdio.h>

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
        "╭──── Round-robin ────╮\n"
        "│ No active process.  │\n"
        "│                     │\n"
        "│ Press `n` to spawn  │\n"
        "│ new process...      │\n"
        "╰─────── [n]ew [q]uit ╯\n"
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
    printf("╰─────────────────────╯\n");
    printf(
        "╭─────────── OS Group 8 ───────────╮\n"
        "│ - Jason Saputra Ang (2702252456) │\n"
        "│ - Louis Ruisani (2702260994)     │\n"
        "│ - Muhammad Iqbal (2702332904)    │\n"
        "╰──────────────────────────────────╯\n"
    );
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
            size_t count = pp_total_created();
            size_t current_idx = pp_current_process_pid(&pp);

            if (current_idx < count) {
                for (int i = (int)current_idx; i < (int)count; ++i) {
                    printf("\x1b[%d;1H│%*s│\n", i + 4, 21, "");
                }
            }

            // int delta = 4 - (int)count;
            const int delta = (4 - (int)count) < 0 ? 0 : (4 - (int)count); // delta cant be < 0 or things brokie.

            if (delta > 0) {
                for (int i = 1; i < delta+2; ++i) {
                    printf("\x1b[%d;1H│%*s│\n", (int)count + i, 21, "");
                }
            }

            printf("\x1b[%d;1H╰─────── [n]ew [q]uit ╯\n", delta + (int)count + 2);
            printf("                       \n");

        } else if (c == 'q') {
            terminal_deinit_raw();
            return NULL;
        }
    }
}
#pragma GCC diagnostic pop