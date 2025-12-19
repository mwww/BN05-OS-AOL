#include "tui.h"
#include "ansi_colors.h"

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const int TUI_PROCESS_ROW_START = 3;
static const int TUI_MIN_PROCESS_ROWS = 4;
static int tui_footer_row = 7;

typedef enum {
  TUI_STATE_WAITING,
  TUI_STATE_RUNNING,
  TUI_STATE_PAUSED,
  TUI_STATE_FINISHED,
} TuiProcessState;

static TuiProcessState *tui_process_states = NULL;
static size_t tui_process_states_capacity = 0;
static pthread_mutex_t tui_state_mutex = PTHREAD_MUTEX_INITIALIZER;
static double *tui_process_weight = NULL;
static size_t tui_process_weight_capacity = 0;
static bool tui_show_weight = false;

static int tui_clamp_rows(int count) {
  // if count is 0 then stay 0, else if it is less than TUI_MIN_PROCESS_ROWS
  // then set to TUI_MIN_PROCESS_ROWS
  if (count == 0)
    return TUI_PROCESS_ROW_START;
  return count < TUI_MIN_PROCESS_ROWS ? TUI_MIN_PROCESS_ROWS : count;
}

static bool tui_ensure_state_capacity(size_t count) {
  if (count <= tui_process_states_capacity)
    return true;

  size_t new_cap = count;
  TuiProcessState *new_states =
      realloc(tui_process_states, new_cap * sizeof(*new_states));
  double *new_weights =
      realloc(tui_process_weight, new_cap * sizeof(*new_weights));

  if (new_states == NULL || new_weights == NULL) {
    return false;
  }

  for (size_t i = tui_process_states_capacity; i < new_cap; ++i) {
    new_states[i] = TUI_STATE_WAITING;
    new_weights[i] = -1.0;
  }

  tui_process_states = new_states;
  tui_process_states_capacity = new_cap;
  tui_process_weight = new_weights;
  tui_process_weight_capacity = new_cap;
  return true;
}

static void tui_render_state_line(uint32_t pid, TuiProcessState state) {
  double weight =
      (pid < tui_process_weight_capacity) ? tui_process_weight[pid] : -1.0;
  switch (state) {
  case TUI_STATE_RUNNING:
    if (tui_show_weight && weight >= 0.0) {
      printf("\x1b[%u;1H│" YELLOW "[%2u] running  w=%-4.2f" RESET
             "               │\n",
             pid + TUI_PROCESS_ROW_START, pid, weight);
    } else {
      printf("\x1b[%u;1H│" YELLOW "[%2u] running                       " RESET
             "│\n",
             pid + TUI_PROCESS_ROW_START, pid);
    }
    break;
  case TUI_STATE_FINISHED:
    if (tui_show_weight && weight >= 0.0) {
      printf("\x1b[%u;1H│" GREEN "[%-2u] finished w=%-4.2f               " RESET
             "│\n",
             pid + TUI_PROCESS_ROW_START, pid, weight);
    } else {
      printf(GREEN "\x1b[%u;2H[%-2u] finished\n" RESET,
             pid + TUI_PROCESS_ROW_START, pid);
    }
    break;
  case TUI_STATE_PAUSED:
  case TUI_STATE_WAITING:
  default:
    if (tui_show_weight && weight >= 0.0) {
      printf("\x1b[%u;1H│[%-2u] paused   w=%-4.2f               │\n",
             pid + TUI_PROCESS_ROW_START, pid, weight);
    } else {
      printf("\x1b[%u;1H│[%-2u] paused                        │\n",
             pid + TUI_PROCESS_ROW_START, pid);
    }
    break;
  }
}

static void tui_render_footer(void) {
  const int row = tui_footer_row > 0
                      ? tui_footer_row
                      : (TUI_PROCESS_ROW_START + TUI_MIN_PROCESS_ROWS);
  printf("\x1b[%d;1H╰────────────── [b]ack [n]ew [q]uit ╯\n", row);
}

void tui_reset_process_states(void) {
  pthread_mutex_lock(&tui_state_mutex);
  for (size_t i = 0; i < tui_process_states_capacity; ++i) {
    tui_process_states[i] = TUI_STATE_WAITING;
  }
  for (size_t i = 0; i < tui_process_weight_capacity; ++i) {
    tui_process_weight[i] = -1.0;
  }
  tui_footer_row = TUI_PROCESS_ROW_START + TUI_MIN_PROCESS_ROWS;
  pthread_mutex_unlock(&tui_state_mutex);
}

void tui_render_algorithm_selector(const char **options, size_t count,
                                   size_t selected_index) {
  printf("\x1b[2J");
  printf("\x1b[?25l");
  printf("\x1b[H╭─────── Select scheduler ───────╮\n");
  printf("│                                │\n");
  for (size_t i = 0; i < count; ++i) {
    if (i == selected_index) {
      printf("│ " YELLOW "▶ %-28s" RESET " │\n", options[i]);
    } else {
      printf("│   %-28s │\n", options[i]);
    }
  }
  printf("│                                │\n");
  printf("╰───────────────── [Enter] start ╯\n");
  printf("  Use ↑/↓ to navigate\n");
  printf("  Press q to exit\n");
}

void tui_init(const char *algorithm_name) {
  printf("\x1b[2J");
  printf("\x1b[?25l");
  printf("\x1b[H");
  printf("Scheduler: %-14s \n", algorithm_name);
  tui_show_weight = strstr(algorithm_name, "Fair") != NULL;
  printf("╭─────── Time-sharing system ───────╮\n");
  printf("│ No active process.                │\n");
  tui_update_process_list(0, 0);
}

void tui_update_process_list(int current_idx, int count) {
  (void)current_idx;
  pthread_mutex_lock(&tui_state_mutex);
  const int rows = tui_clamp_rows(count);

  if (count == 0) {
    const int footer_row = rows;
    printf("\x1b[%d;1H│                                   │\n", footer_row + 1);
    printf("\x1b[%d;1H│ Press `n` to spawn                │\n", footer_row + 2);
    printf("\x1b[%d;1H│ new processes...                  │\n", footer_row + 3);
    tui_footer_row = footer_row + 4;
    tui_render_footer();
    printf("\x1b[%d;1H%*s\n", footer_row + 5, 23, "");
    pthread_mutex_unlock(&tui_state_mutex);
    return;
  }

  if (!tui_ensure_state_capacity((size_t)count)) {
    pthread_mutex_unlock(&tui_state_mutex);
    return;
  }

  for (size_t pid = 0; pid < (size_t)count; ++pid) {
    tui_render_state_line((uint32_t)pid, tui_process_states[pid]);
  }

  for (int row = count; row < rows; ++row) {
    printf("\x1b[%d;1H│                                   │\n",
           TUI_PROCESS_ROW_START + row);
  }

  tui_footer_row = rows + 3;
  tui_render_footer();
  pthread_mutex_unlock(&tui_state_mutex);
}

void tui_display_process_running(uint32_t pid) {
  pthread_mutex_lock(&tui_state_mutex);
  if (!tui_ensure_state_capacity(pid + 1)) {
    pthread_mutex_unlock(&tui_state_mutex);
    return;
  }
  tui_process_states[pid] = TUI_STATE_RUNNING;
  tui_render_state_line(pid, TUI_STATE_RUNNING);
  tui_render_footer();
  pthread_mutex_unlock(&tui_state_mutex);
}

void tui_display_process_paused(uint32_t pid) {
  pthread_mutex_lock(&tui_state_mutex);
  if (!tui_ensure_state_capacity(pid + 1)) {
    pthread_mutex_unlock(&tui_state_mutex);
    return;
  }
  if (tui_process_states[pid] != TUI_STATE_FINISHED) {
    tui_process_states[pid] = TUI_STATE_PAUSED;
  }
  tui_render_state_line(pid, tui_process_states[pid]);
  tui_render_footer();
  pthread_mutex_unlock(&tui_state_mutex);
}

void tui_display_process_finished(uint32_t pid) {
  pthread_mutex_lock(&tui_state_mutex);
  if (!tui_ensure_state_capacity(pid + 1)) {
    pthread_mutex_unlock(&tui_state_mutex);
    return;
  }
  tui_process_states[pid] = TUI_STATE_FINISHED;
  tui_render_state_line(pid, TUI_STATE_FINISHED);
  tui_render_footer();
  pthread_mutex_unlock(&tui_state_mutex);
}

void tui_register_process_metadata(uint32_t pid, double weight) {
  pthread_mutex_lock(&tui_state_mutex);
  if (!tui_ensure_state_capacity(pid + 1)) {
    pthread_mutex_unlock(&tui_state_mutex);
    return;
  }
  tui_process_weight[pid] = weight;

  if (tui_show_weight && tui_process_states[pid] == TUI_STATE_WAITING) {
    tui_render_state_line(pid, tui_process_states[pid]);
  }

  pthread_mutex_unlock(&tui_state_mutex);
}

void tui_exit(int last_pid) {
  printf(RESET);
  const int rows = last_pid > 0 ? last_pid : 2;
  const int summary_row = TUI_PROCESS_ROW_START + rows + 1;

  // print " " for 50 times side by side
  // printf("%*s\n", 50, " ");

  if (last_pid == 0) {
    printf("\x1b[%d;1H│                                   │\n",
           summary_row - 3);
    printf("\x1b[%d;1H│     No processes were started     │\n",
           summary_row - 2);
    printf("\x1b[%d;1H│                                   │\n",
           summary_row - 1);
    printf("\x1b[%d;1H╰───────────────────────────────────╯\n",
           summary_row - 0);
  } else {
    printf("\x1b[%d;1H╰───────────────────────────────────╯\n",
           summary_row - 1);
    printf("\x1b[%d;1H╭───────────────────────────────────╮\n", summary_row);
    //   printf("\x1b[%d;1H│ No processes were               │\n", summary_row +
    //   1); printf("\x1b[%d;1H│ started.                        │\n",
    //   summary_row + 2);
    // } else {
    printf("\x1b[%d;1H│ Completed %d process              %s│\n",
           summary_row + 1, last_pid, (last_pid >= 10) ? "" : " ");
    printf("\x1b[%d;1H│ Thank you.                        │\n",
           summary_row + 2);
    printf("\x1b[%d;1H╰───────────────────────────────────╯\n",
           summary_row + 3);
  }
  printf("╭─────────── OS Group 08 ───────────╮\n"
         "│ - Jason Saputra Ang (2702252456)  │\n"
         "│ - Louis Ruisani (2702260994)      │\n"
         "│ - Muhammad Iqbal (2702332904)     │\n"
         "╰───────────────────────────────────╯\n");

  // ╰───────────────────────────────────╯
  // ╭──── Operating System Group 8 ─────╮
  printf("\nhttps://github.com/mwww/BN05-OS-AOL/\n");
  printf("\x1b[?25h");
}
