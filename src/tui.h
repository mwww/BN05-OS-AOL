#pragma once

#include <stdint.h>

// sudah tapi belum
void tui_init();
void tui_exit();

// render process' state
void tui_display_process_running(uint32_t pid);
void tui_display_process_paused(uint32_t pid);
void tui_display_process_finished(uint32_t pid);

// update process list display
void tui_update_process_list(int current_idx, int count);
