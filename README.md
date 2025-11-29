# Assignment 3 Part 1

## Description
This project simulates process scheduling algorithms for an operating system, including EP, EP_RR, and RR. It models process states, memory partitioning, and tracks key metrics such as throughput, turnaround time, wait time, and response time.

## Features
- Multiple scheduling algorithms (EP, EP_RR, RR)
- Simulates process state transitions and memory allocation
- Calculates and outputs scheduling metrics
- Handles I/O and priority-based scheduling

## Usage
1. Build executables using `build.sh`:
	```bash
	./build.sh
	```
2. Run a simulation:
	```bash
	./bin/interrupts_RR input.txt
	```
	Replace `interrupts_RR` with the desired algorithm and `input.txt` with your input file.

## Input Format
Each line in the input file should be:
```
PID, size, arrival_time, burst_time, io_freq, io_duration, priority
```
**Ensure to use this format, or you may experience unexpected behaviour or `Segmentation Faults`**

## Output
- `execution.txt`:          Process state transitions and metrics
- `memory_analysis.txt`:    Memory and event analysis per tick (see header)

## Notes/Assumptions
- Priority is a separate input (not PID-based)
- Metrics are tracked during execution, and then calculated (see header) at the end of each simulation (output in `execution.txt`)
- Ignored any context switch times
- Below is the approach we took in the case of race conditions. The code will execute in this order, and therefore the first check has higher priority than the last.

For Running Processes:
1. Check if the currently running process is finished
2. Check if the currently running process needs to do I/O
3. Check if it has been preempted by another higher priority process
4. Check if quantum has been reached
