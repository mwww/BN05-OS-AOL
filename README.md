<div align="center">
  <h1> Time-Sharing System Simulation Using C </h1>
</div>

![Linux Badge](https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black)
![VIM Badge](https://img.shields.io/badge/VIM-%2311AB00.svg?&style=for-the-badge&logo=vim&logoColor=white)
![C Badge](https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white)
![GPL 3.0 Badge](https://img.shields.io/badge/GPL--3.0-red?style=for-the-badge)


> This program demonstrates how time-sharing works in a multi-tasking environment by using timers and signals. Each task will be given an equal set amount of time to be processed before being paused and switched to the next task. This process is looped until each task has been completed, simulating a round-robin CPU scheduler. This however, happens in the span of miliseconds in real life, so in order to make the process observable, we slowed the time slice down to ~1 second per task.

## Build & Run
To build and run this program, you need to have **make** installed

```bash
make run
```

## Contributors:
1. Jason Saputra Ang - 2702252456
2. Louis Ruisani - 2702260994
3. Muhammad Iqbal - 2702332904
