#include <stdio.h>

#include "process_pool.h"
#include "process.h"

int main() {
    ProcessPool pp = pp_new();

    pp_push(&pp, process_new(6'000'000));
    pp_push(&pp, process_new(5'000'000));
    pp_push(&pp, process_new(2'000'000));
    
    pp_run_round_robin(&pp, 1.0);
}