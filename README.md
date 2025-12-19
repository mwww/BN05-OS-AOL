<div align="center">
  <h1> Time-Sharing System Simulation (C / TUI) </h1>
</div>

![Linux Badge](https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black)
![C Badge](https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white)
![GPL 3.0 Badge](https://img.shields.io/badge/GPL--3.0-red?style=for-the-badge)

> A terminal-based simulation of CPU time-sharing and scheduling. “Processes” are simulated workloads that run for a number of iterations, and a scheduler repeatedly selects which process to run next based on the chosen algorithm.

The program runs in an interactive TUI (raw terminal mode): you pick a scheduling algorithm, then spawn processes while the scheduler is running and observe their state transitions (running / paused / finished).

## Scheduling Algorithms

Before the simulation starts, the TUI asks you to choose which scheduler to run. Use ↑/↓ to select an algorithm and press **Enter** to begin.

- **Round Robin** (time slice / quantum = **1.0s**)
- **First Come First Serve** (runs each process until completion)
- **Completely Fair Scheduler (CFS)** (target latency = **20ms**, time slice is proportional to a process “weight”, with a minimum granularity)

## Controls

### Algorithm selector

- **↑/↓**: move selection
- **Enter**: start
- **q**: exit

### Simulation screen

- **n**: spawn a new process
- **b**: go back to algorithm selection (clears current process pool)
- **q**: quit

## Build & Run

### Requirements

- Linux / macOS (uses POSIX `termios` for raw terminal input and `pthread` for threading)
- Windows: supported via **WSL2** (recommended). Native Windows builds are not supported without porting the `termios`/`pthread` parts.
- A C compiler with C23 support (e.g., `gcc` or `clang`)
- `make`
- `pthread` (used for the scheduler + input controller threads)

### Commands

```bash
make run
```

Other useful targets:

```bash
make build
make clean
```

The binary is built to `build/main`.

## Notes

- Each press of **n** spawns a new simulated process with a fixed workload (iterations) and a randomized weight (used by CFS).
- The TUI uses ANSI escape sequences and hides the cursor while running.

## License

Licensed under GPL-3.0. See [`LICENSE`](LICENSE:1).

## Contributors:

1. Jason Saputra Ang - 2702252456
2. Louis Ruisani - 2702260994
3. Muhammad Iqbal - 2702332904
