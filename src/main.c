#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
// #include <unistd.h>

#include "process.h"
#include "process_pool.h"
#include "scheduler.h"
#include "terminal.h"
#include "tui.h"

ProcessPool pp;

static volatile bool should_stop_scheduler = false;
static volatile bool request_back = false;
static volatile bool request_quit = false;

typedef struct {
  ProcessPool *pool;
  const SchedulerAlgorithm *algorithm;
} SchedulerContext;

void *scheduler(void *_);
void *controller(void *_);

static bool select_scheduler(size_t count, const SchedulerAlgorithm *algorithms,
                             size_t *selected_index);

int main() {
  // setvbuf(stdout, NULL, _IONBF, 0);
  size_t algorithm_count = 0;
  const SchedulerAlgorithm *algorithms = scheduler_algorithms(&algorithm_count);

  terminal_init_raw();

  while (true) {
    process_reset_pid_counter();
    tui_reset_process_states();

    size_t selected_algorithm = 0;
    if (!select_scheduler(algorithm_count, algorithms, &selected_algorithm)) {
      printf("\x1b[?25h");
      terminal_deinit_raw();
      return 0;
    }

    request_back = false;
    request_quit = false;
    should_stop_scheduler = false;

    pp = pp_new();
    tui_init(algorithms[selected_algorithm].name);

    pthread_t scheduler_thread;
    pthread_t control_thread;

    SchedulerContext ctx = {
        .pool = &pp,
        .algorithm = &algorithms[selected_algorithm],
    };

    pthread_create(&scheduler_thread, NULL, scheduler, &ctx);
    pthread_create(&control_thread, NULL, controller, NULL);

    pthread_join(control_thread, NULL);

    should_stop_scheduler = true;
    pp_clear(&pp);
    pthread_join(scheduler_thread, NULL);

    if (request_quit) {
      terminal_deinit_raw();
      tui_exit((int)pp_total_created());
      pp_destroy(&pp);
      return 0;
    }

    pp_destroy(&pp);

    if (!request_back) {
      break;
    }
  }

  printf("\x1b[?25h");
  terminal_deinit_raw();
  return 0;
}

void *scheduler(void *ctx) {
  SchedulerContext *context = ctx;
  while (!should_stop_scheduler) {
    context->algorithm->execute(context->pool, context->algorithm->quantum);
  }
  return NULL;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
void *controller(void *_) {
  int irun = 0;
  while (true) {
    irun++;

    // printf("\x1b[1;1H%*s\n", 23, "");
    // printf("\x1b[1;1H%d\n", irun);
    // fflush(stdout);

    // char buf[16];
    // int len =
    //     snprintf(buf, sizeof buf, "\x1b[1;1H%*s\n\x1b[1;1H%d\n", 23, "",
    //     irun);
    // write(STDOUT_FILENO, buf, len);

    const char c = (char)getchar();
    if (c == 'n') {
      Process p = process_new(8000000);
      tui_register_process_metadata(p.pid, p.priority_weight);
      pp_push(&pp, p);

      const int count = (int)pp_total_created();
      const int current_idx = (int)pp_current_process_pid(&pp);

      // print count to the top left most
      //   printf("\x1b[1;1H%*s\n", 23, "");
      //   printf("\x1b[1;1H%d", count);

      tui_update_process_list(current_idx, count);

    } else if (c == 'b') {
      should_stop_scheduler = true;
      request_back = true;
      pp_clear(&pp);
      return NULL;

    } else if (c == 'q') {
      should_stop_scheduler = true;
      request_quit = true;
      pp_clear(&pp);
      return NULL;
    }
  }
}
#pragma GCC diagnostic pop

static bool select_scheduler(size_t count, const SchedulerAlgorithm *algorithms,
                             size_t *selected_index) {
  if (count == 0 || selected_index == NULL) {
    return false;
  }

  const char **names = calloc(count, sizeof(*names));
  if (names == NULL) {
    return false;
  }

  for (size_t i = 0; i < count; ++i) {
    names[i] = algorithms[i].name;
  }

  *selected_index = 0;
  tui_render_algorithm_selector(names, count, *selected_index);

  bool should_start = false;
  while (true) {
    int c = getchar();
    if (c == EOF) {
      break;
    }

    if (c == '\x1b') {
      int first = getchar();
      int second = getchar();
      if (first == '[') {
        if (second == 'A') {
          *selected_index = (*selected_index + count - 1) % count;
          tui_render_algorithm_selector(names, count, *selected_index);
        } else if (second == 'B') {
          *selected_index = (*selected_index + 1) % count;
          tui_render_algorithm_selector(names, count, *selected_index);
        }
      }
      continue;
    }

    if (c == '\n' || c == '\r') {
      should_start = true;
      break;
    }

    if (c == 'q') {
      break;
    }
  }

  free(names);
  return should_start;
}
