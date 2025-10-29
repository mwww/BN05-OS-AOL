#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>

#define NUM_PROCESSES 4
#define TIME_QUANTUM 1  // seconds
#define TOTAL_WORK 10   // units of work per process

typedef enum { READY, RUNNING, BLOCKED, FINISHED } State;

typedef struct {
    pid_t pid;
    int id;
    State state;
    int work_done;
    int total_work;
} Process;

Process processes[NUM_PROCESSES];
int current_process = 0;
int active_processes = NUM_PROCESSES;

// Child process - does actual CPU work
void child_work(int id) {
    srand(time(NULL) + id);
    int work_units = 0;

    while (work_units < TOTAL_WORK) {
        // Simulate CPU-intensive work
        unsigned long result = 0;
        for (long i = 0; i < 10000000; i++) {
            result += i * rand();
        }

        work_units++;
        printf("[Process %d] Completed work unit %d/%d (result: %lu)\n",
               id, work_units, TOTAL_WORK, result % 1000);

        // Simulate I/O occasionally
        if (work_units % 3 == 0) {
            printf("[Process %d] Requesting I/O...\n", id);
            kill(getpid(), SIGSTOP);  // Block self
            printf("[Process %d] I/O completed, resuming\n", id);
        }
    }

    printf("[Process %d] *** FINISHED ALL WORK ***\n", id);
    exit(0);
}

// Scheduler - runs on timer interrupt
void scheduler(int signum) {
    if (active_processes == 0) {
        printf("\n=== All processes finished ===\n");
        exit(0);
    }

    // Stop current running process
    for (int i = 0; i < NUM_PROCESSES; i++) {
        if (processes[i].state == RUNNING) {
            kill(processes[i].pid, SIGSTOP);
            processes[i].state = READY;
            printf("\n[SCHEDULER] Preempting Process %d\n", i);
            break;
        }
    }

    // Find next ready process (Round-Robin)
    int attempts = 0;
    while (attempts < NUM_PROCESSES) {
        current_process = (current_process + 1) % NUM_PROCESSES;

        if (processes[current_process].state == READY) {
            processes[current_process].state = RUNNING;
            kill(processes[current_process].pid, SIGCONT);
            printf("[SCHEDULER] Running Process %d\n\n",
                   current_process);
            break;
        }
        attempts++;
    }
}

// Handle child termination
void handle_child_exit(int signum) {
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        for (int i = 0; i < NUM_PROCESSES; i++) {
            if (processes[i].pid == pid) {
                processes[i].state = FINISHED;
                active_processes--;
                printf("\n*** Process %d terminated ***\n", i);
            }
        }
    }
}

int main() {
    printf("=== Time-Sharing Simulator Starting ===\n");
    printf("Processes: %d | Time Quantum: %ds\n\n",
           NUM_PROCESSES, TIME_QUANTUM);

    // Create child processes
    for (int i = 0; i < NUM_PROCESSES; i++) {
        pid_t pid = fork();

        if (pid == 0) {
            // Child process
            child_work(i);
            exit(0);
        } else {
            // Parent process
            processes[i].pid = pid;
            processes[i].id = i;
            processes[i].state = READY;
            processes[i].work_done = 0;
            processes[i].total_work = TOTAL_WORK;

            // Stop child immediately (they start in READY state)
            usleep(10000);  // Small delay
            kill(pid, SIGSTOP);
            printf("Process %d created (PID: %d)\n", i, pid);
        }
    }

    printf("\n=== Starting Scheduling ===\n\n");

    // Set up signal handlers
    signal(SIGALRM, scheduler);
    signal(SIGCHLD, handle_child_exit);

    // Start first process
    processes[0].state = RUNNING;
    kill(processes[0].pid, SIGCONT);
    printf("[SCHEDULER] Running Process 0\n\n");

    // Set up interval timer
    struct itimerval timer;
    timer.it_value.tv_sec = TIME_QUANTUM;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = TIME_QUANTUM;
    timer.it_interval.tv_usec = 0;

    setitimer(ITIMER_REAL, &timer, NULL);

    // Wait for all children
    while (active_processes > 0) {
        pause();
    }

    printf("\nAll processes completed!\n");
    return 0;
}