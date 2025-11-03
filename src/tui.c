#include "tui.h"
#include "ansi_colors.h"
#include <stdio.h>

void tui_init() {
    printf("\x1b[2J");
    printf("\x1b[?25l");
    printf(
        "\x1b[H"
        "╭──── Round-robin ────╮\n"
        "│ No active process.  │\n"
        "│                     │\n"
        "│ Press `n` to spawn  │\n"
        "│ new process...      │\n"
        "╰─────── [n]ew [q]uit ╯\n"
    );
}

void tui_exit() {
    printf(RESET);
    printf("╰─────────────────────╯\n");
    printf(
        "╭─────────── OS Group 8 ───────────╮\n"
        "│ - Jason Saputra Ang (2702252456) │\n"
        "│ - Louis Ruisani (2702260994)     │\n"
        "│ - Muhammad Iqbal (2702332904)    │\n"
        "╰──────────────────────────────────╯\n"
    );
    printf("\x1b[?25h");
}

void tui_display_process_running(uint32_t pid) {
    printf("\x1b[%u;1H│"YELLOW"[%2u] running         "RESET"│\n", pid + 2, pid);
}

void tui_display_process_paused(uint32_t pid) {
    printf("\x1b[%u;1H│[%-2u] paused          │\n", pid + 2, pid);
}

void tui_display_process_finished(uint32_t pid) {
    printf(GREEN"\x1b[%u;2H[%-2u] finished\n"RESET, pid + 2, pid);
}

void tui_update_process_list(int current_idx, int count) {
    if (current_idx < count) {
        for (int i = current_idx; i < count; ++i) {
            printf("\x1b[%d;1H│%*s│\n", i + 4, 21, "");
        }
    }

    const int delta = (4 - count) < 0 ? 0 : (4 - count);

    if (delta > 0) {
        for (int i = 1; i < delta + 2; ++i) {
            printf("\x1b[%d;1H│%*s│\n", count + i, 21, "");
        }
    }

    printf("\x1b[%d;1H╰─────── [n]ew [q]uit ╯\n", delta + count + 2);
    printf("                       \n");
}
