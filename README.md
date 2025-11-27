- Added one addition input for csv (priority) instead of using the pid as the priority
- Order of flow for running processes:
1. Check if process has completed
2. Check if it has been preempted by another higher priority process
3. Check if the currently running process needs to do I/O
4. Check if quantum has been reached

Still need to go over comments and check if metrics are correct for the processes (throughput, wait time, response time etc)