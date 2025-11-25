/**
 * @file interrupts.cpp
 * @author Sasisekhar Govind
 * @brief template main.cpp file for Assignment 3 Part 1 of SYSC4001
 *
 */


#include<interrupts_student1_student2.hpp>
#include<map>

#define QUANTUM         100

// Transition record (per tick)
struct Event {
    unsigned int time;
    int pid;
    states old_state;
    states new_state;
};

// Build consolidated memory + transition report for one time tick (string concatenation version)

std::string parseMemoryEvents(unsigned int current_time,
                              const std::vector<Event>& transitions,
                              const std::vector<PCB>& job_list,
                              const std::vector<PCB>& ready_queue,
                              const std::vector<PCB>& wait_queue,
                              const PCB& running) {
    if (transitions.empty()) return std::string();

    std::string output;
    output += "\n--- Memory / Events at Time " + std::to_string(current_time) + " ---\n";
    output += "Transitions this tick:\n";
    output += "| PID | Old -> New\n";
    output += "|-----|-------------\n";
    for (auto &t : transitions) {
        output += "|  " + std::to_string(t.pid) + "  | " +
            std::string(t.old_state == NEW ? "NEW" :
                        t.old_state == READY ? "READY" :
                        t.old_state == RUNNING ? "RUNNING" :
                        t.old_state == WAITING ? "WAITING" :
                        t.old_state == TERMINATED ? "TERMINATED" : "NOT_ASSIGNED") +
            " -> " +
            std::string(t.new_state == NEW ? "NEW" :
                        t.new_state == READY ? "READY" :
                        t.new_state == RUNNING ? "RUNNING" :
                        t.new_state == WAITING ? "WAITING" :
                        t.new_state == TERMINATED ? "TERMINATED" : "NOT_ASSIGNED") + "\n";
    }

    // Process State table
    output += "\nProcess State:\n";
    output += "| NEW | READY | WAITING | RUNNING | TERMINATED |\n";
    output += "|-----|-------|---------|---------|------------|";

    std::vector<int> new_pids, ready_pids, waiting_pids, terminated_pids;
    int running_pid = -1;
    for (auto &job : job_list) {
        if (job.state == NEW) {
            new_pids.push_back(job.PID);
        } else if (job.state == TERMINATED) {
            terminated_pids.push_back(job.PID);
        }
    }
    for (const auto &p : ready_queue) {
        ready_pids.push_back(p.PID);
    }
    for (const auto &p : wait_queue) {
        waiting_pids.push_back(p.PID);
    }
    if (running.state == RUNNING) {
        running_pid = running.PID;
    }
    size_t max_rows = std::max({new_pids.size(), ready_pids.size(), waiting_pids.size(), (size_t)(running_pid != -1 ? 1 : 0), terminated_pids.size()});
    if (max_rows == 0) {max_rows = 1;}
    for (size_t i = 0; i < max_rows; i++) {
        output += "\n|";
        if (i < new_pids.size()) {
            output += "  " + std::to_string(new_pids[i]) + (new_pids[i] < 10 ? "  |" : " |");
        } else {
            output += "  -  |";
        }
        if (i < ready_pids.size()) {
            output += "   " + std::to_string(ready_pids[i]) + (ready_pids[i] < 10 ? "   |" : "  |");
        } else {
            output += "   -   |";
        }
        if (i < waiting_pids.size()) {
            output += "    " + std::to_string(waiting_pids[i]) + (waiting_pids[i] < 10 ? "    |" : "   |");
        }else {
            output += "    -    |";
        }
        if (i == 0) {
            if (running_pid != -1) {
                output += "    " + std::to_string(running_pid) + (running_pid < 10 ? "    |" : "   |");
            } else {
                output += "    -    |";
            }
        } else {
            output += "    -    |";
        }
        if (i < terminated_pids.size()) {
            output += "     " + std::to_string(terminated_pids[i]) + (terminated_pids[i] < 10 ? "      |" : "     |");
        } else {
            output += "     -      |";
        }
    }
    output += "\n";

    // Partition Usage
    output += "\nPartition Usage:\n";
    output += "| Part | Size | Used | Unused | PID |\n";
    output += "|------|------|------|--------|-----|\n";
    for (int i = 0; i < 6; i++) {
        const memory_partition &part = memory_paritions[i];
        int pid = part.occupied;
        unsigned int used = 0;
        if (pid != -1) {
            for (size_t j = 0; j < job_list.size(); j++) {
                if (job_list[j].PID == pid) {
                    used = job_list[j].size;
                    break;
                }
            }
        }
        unsigned int unused = (pid == -1) ? part.size : (part.size > used ? part.size - used : 0);
        output += "|  " + std::to_string(part.partition_number) + "   |  " +
                std::to_string(part.size) +
                    (part.size < 10 ? "   |  " : "  |  ") +
                std::to_string(used) +
                    (used < 10 ? "   |   " : "  |   ") +
                std::to_string(unused) +
                    (unused < 10 ? "    | " : "   | ") +
                (pid == -1 ? std::string("-1") : " " + std::to_string(pid)) +
                    (pid == -1 || pid > 9 ? "  |\n" : "  |\n");
    }
    output += "\n" + std::string(70, '=') + "\n";
    return output;
}


// Sort ready queue by priority (lower priority number = higher priority)
void sort_by_priority(std::vector<PCB> &ready_queue) {
    std::sort(
        ready_queue.begin(),
        ready_queue.end(),
        [](const PCB &first, const PCB &second) {
            return (first.priority > second.priority); // ascending: lowest number first
        }
    );
}

std::tuple<std::string, std::string> run_simulation(std::vector<PCB> list_processes) {

    std::vector<PCB> ready_queue;   //The ready queue of processes
    std::vector<PCB> wait_queue;    //The wait queue of processes
    std::vector<PCB> job_list;      //A list to keep track of all the processes. This is similar
                                    //to the "Process, Arrival time, Burst time" table that you
                                    //see in questions. You don't need to use it, I put it here
                                    //to make the code easier :).

    unsigned int current_time = 0;
    PCB running;

    //Initialize an empty running process (for when when CPU is idle)
    // Need to do this at the end as well
    idle_CPU(running);

    std::string execution_status;
    std::string memory_status;

    //make the output table (the header row)
    execution_status = print_exec_header();


    while(!all_process_terminated(list_processes)) {
        std::vector<Event> memory_transitions;

        // 1. Admit new arrivals
        bool new_arrival = false;
        for(auto &process : list_processes) {
            if (process.state == NOT_ASSIGNED && process.arrival_time <= current_time) {
                if (assign_memory(process)) {
                    process.state = READY;
                    ready_queue.push_back(process);
                    job_list.push_back(process);
                    new_arrival = true;
                    execution_status += print_exec_status(current_time, process.PID, NEW, READY);
                    memory_transitions.push_back({current_time, process.PID, NEW, READY});
                }
            }
        }

        // 2. Move completed I/Os back to ready
        bool io_completed = false;
        for (int i = 0; i < wait_queue.size(); i++) {
            if (current_time - wait_queue[i].start_time >= wait_queue[i].io_duration) {
                wait_queue[i].state = READY;
                ready_queue.push_back(wait_queue[i]);
                sync_queue(job_list, wait_queue[i]);
                io_completed = true;
                execution_status += print_exec_status(current_time, wait_queue[i].PID, WAITING, READY);
                memory_transitions.push_back({current_time, wait_queue[i].PID, WAITING, READY});
                wait_queue.erase(wait_queue.begin() + i);
            }
        }

        // Always sort ready queue by priority after arrivals and I/O completions
        if (!ready_queue.empty()) {
            sort_by_priority(ready_queue);
        }

        // Update list_processes with states from job_list (keep them synced)
        sync_queue(list_processes, running);
        for (auto &job : job_list) {
            sync_queue(list_processes, job);
        }

        // 3. Preemption: If a higher priority process is ready, preempt running
        bool preempted = false;
        if ((new_arrival || io_completed) && running.state == RUNNING && !ready_queue.empty()) {
            if (ready_queue.front().priority < running.priority) {
                preempted = true;
            }
        }

        // 4. If CPU is idle and has stuff in ready, start running next process
        if (running.state == NOT_ASSIGNED && !ready_queue.empty()) {
            running = ready_queue.front();
            ready_queue.erase(ready_queue.begin());
            running.start_time = current_time;
            running.state = RUNNING;
            sync_queue(job_list, running);
            execution_status += print_exec_status(current_time, running.PID, READY, RUNNING);
            memory_transitions.push_back({current_time, running.PID, READY, RUNNING});
        }

        // 5. Make sure CPU isn't idle before checking these things
        if (running.state == RUNNING) {
            unsigned int elapsed = current_time - running.start_time;

            // if process has completed
            if (running.remaining_time <= elapsed) {
                running.state = TERMINATED;
                running.remaining_time = 0;
                memory_paritions[running.partition_number - 1].occupied = -1;
                running.partition_number = -1;
                sync_queue(job_list, running);
                execution_status += print_exec_status(current_time, running.PID, RUNNING, TERMINATED);
                memory_transitions.push_back({current_time, running.PID, RUNNING, TERMINATED});
                idle_CPU(running);
            }
            // if process needs to do I/O (check before quantum)
            else if (running.io_freq > 0 && elapsed > 0 && elapsed % running.io_freq == 0) {
                running.remaining_time -= elapsed;
                running.start_time = current_time;
                running.state = WAITING;
                wait_queue.push_back(running);
                sync_queue(job_list, running);
                execution_status += print_exec_status(current_time, running.PID, RUNNING, WAITING);
                memory_transitions.push_back({current_time, running.PID, RUNNING, WAITING});
                idle_CPU(running);
            }
            // if process has exceeded its allowed time (quantum)
            else if (elapsed >= QUANTUM) {
                running.remaining_time -= QUANTUM;
                running.state = READY;
                ready_queue.push_back(running);
                sync_queue(job_list, running);
                execution_status += print_exec_status(current_time, running.PID, RUNNING, READY);
                memory_transitions.push_back({current_time, running.PID, RUNNING, READY});
                idle_CPU(running);
            }
            // if preempted by higher priority process
            else if (preempted) {
                running.remaining_time -= elapsed;
                running.state = READY;
                ready_queue.push_back(running);
                sync_queue(job_list, running);
                execution_status += print_exec_status(current_time, running.PID, RUNNING, READY);
                memory_transitions.push_back({current_time, running.PID, RUNNING, READY});
                idle_CPU(running);
            }
        }
        if(!memory_transitions.empty()) {
            memory_status += parseMemoryEvents(current_time, memory_transitions, job_list, ready_queue, wait_queue, running);
        }
        current_time += 1;
    }


    //Close the output table
    execution_status += print_exec_footer();

    // Calculate and format metrics
    unsigned int total_turnaround = 0;
    unsigned int total_wait = 0;
    unsigned int total_response = 0;
    unsigned int num_processes = list_processes.size();

    // For metrics, we need completion and first run times
    std::map<int, unsigned int> completion_times;
    std::map<int, unsigned int> first_run_times;
    for (const auto& job : job_list) {
        if (job.state == TERMINATED) {
            completion_times[job.PID] = current_time;
        }
    }
    for (const auto& job : job_list) {
        if (job.state == RUNNING || job.state == READY || job.state == WAITING || job.state == TERMINATED) {
            if (first_run_times.find(job.PID) == first_run_times.end()) {
                first_run_times[job.PID] = job.start_time;
            }
        }
    }
    for (const auto& process : job_list) {
        if (completion_times.find(process.PID) != completion_times.end() && first_run_times.find(process.PID) != first_run_times.end()) {
            unsigned int turnaround = completion_times[process.PID] - process.arrival_time;
            unsigned int response = first_run_times[process.PID] - process.arrival_time;
            unsigned int wait = turnaround - process.processing_time;
            total_turnaround += turnaround;
            total_response += response;
            total_wait += wait;
        }
    }
    double avg_turnaround = num_processes ? (double)total_turnaround / num_processes : 0.0;
    double avg_wait = num_processes ? (double)total_wait / num_processes : 0.0;
    double avg_response = num_processes ? (double)total_response / num_processes : 0.0;
    double throughput = current_time ? (double)num_processes / current_time : 0.0;

    std::string metrics = "\n\n========== Scheduling Metrics ==========";
    metrics += "\nThroughput:              " + std::to_string(throughput) + " processes/time unit";
    metrics += "\nAverage Turnaround Time: " + std::to_string(avg_turnaround) + " time units";
    metrics += "\nAverage Wait Time:       " + std::to_string(avg_wait) + " time units";
    metrics += "\nAverage Response Time:   " + std::to_string(avg_response) + " time units";
    metrics += "\n========================================\n";

    execution_status += metrics;

    return std::make_tuple(execution_status, memory_status);
}


int main(int argc, char** argv) {

    //Get the input file from the user
    if(argc != 2) {
        std::cout << "ERROR!\nExpected 1 argument, received " << argc - 1 << std::endl;
        std::cout << "To run the program, do: ./interrutps <your_input_file.txt>" << std::endl;
        return -1;
    }

    //Open the input file
    auto file_name = argv[1];
    std::ifstream input_file;
    input_file.open(file_name);

    //Ensure that the file actually opens
    if (!input_file.is_open()) {
        std::cerr << "Error: Unable to open file: " << file_name << std::endl;
        return -1;
    }

    //Parse the entire input file and populate a vector of PCBs.
    //To do so, the add_process() helper function is used (see include file).
    std::string line;
    std::vector<PCB> list_process;
    while(std::getline(input_file, line)) {
        auto input_tokens = split_delim(line, ", ");
        auto new_process = add_process(input_tokens);
        list_process.push_back(new_process);
    }
    input_file.close();

    //With the list of processes, run the simulation
    auto [exec, mem_report] = run_simulation(list_process);

    write_output(exec, "execution.txt");
    write_output(mem_report, "memory_analysis.txt");

    return 0;
}