#pragma once

#include <stddef.h>
#include <stdint.h>

void tui_render_algorithm_selector(const char **options, size_t count,
                                   size_t selected_index);
// sudah tapi belum
void tui_init(const char *algorithm_name);
void tui_exit(int last_pid);

// render process' state
void tui_display_process_running(uint32_t pid);
void tui_display_process_paused(uint32_t pid);
void tui_display_process_finished(uint32_t pid);
void tui_register_process_metadata(uint32_t pid, double weight);
void tui_reset_process_states(void);

// update process list display
void tui_update_process_list(int current_idx, int count);
