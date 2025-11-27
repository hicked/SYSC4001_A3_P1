/**
 * @file interrupts_101295764_101306299.hpp
 * @author Antoine Hickey & Enzo Chen
 * @brief Header file for os schedulers 4001
 *
 */

#ifndef INTERRUPTS_HPP_
#define INTERRUPTS_HPP_

#include<iostream>
#include<fstream>
#include<string>
#include<vector>
#include<tuple>
#include<random>
#include<utility>
#include<sstream>
#include<iomanip>
#include<algorithm>
#include <map>

//An enumeration of states to make assignment easier
enum states {
    NEW,
    READY,
    RUNNING,
    WAITING,
    TERMINATED,
    NOT_ASSIGNED
};
std::ostream& operator<<(std::ostream& os, const enum states& s) { //Overloading the << operator to make printing of the enum easier

	std::string state_names[] = {
                                "NEW",
                                "READY",
                                "RUNNING",
                                "WAITING",
                                "TERMINATED",
                                "NOT_ASSIGNED"
    };
    return (os << state_names[s]);
}

struct memory_partition{
    unsigned int    partition_number;
    unsigned int    size;
    int             occupied;
} memory_paritions[] = {
    {1, 40, -1},
    {2, 25, -1},
    {3, 15, -1},
    {4, 10, -1},
    {5, 8, -1},
    {6, 2, -1}
};

struct PCB{
    int             PID;
    unsigned int    size;
    unsigned int    arrival_time;
    int             start_time;
    unsigned int    processing_time;
    unsigned int    remaining_time;
    int             partition_number;
    enum states     state;
    unsigned int    io_freq;
    unsigned int    io_duration;

    unsigned int    priority;
};

//------------------------------------HELPER FUNCTIONS FOR THE SIMULATOR------------------------------
// Following function was taken from stackoverflow; helper function for splitting strings
std::vector<std::string> split_delim(std::string input, std::string delim) {
    std::vector<std::string> tokens;
    std::size_t pos = 0;
    std::string token;
    while ((pos = input.find(delim)) != std::string::npos) {
        token = input.substr(0, pos);
        tokens.push_back(token);
        input.erase(0, pos + delim.length());
    }
    tokens.push_back(input);

    return tokens;
}

//Function that takes a queue as an input and outputs a string table of PCBs
std::string print_PCB(std::vector<PCB> _PCB) {
    const int tableWidth = 83;

    std::stringstream buffer;

    // Print top border
    buffer << "+" << std::setfill('-') << std::setw(tableWidth) << "+" << std::endl;

    // Print headers
    buffer << "|"
              << std::setfill(' ') << std::setw(4) << "PID"
              << std::setw(2) << "|"
              << std::setfill(' ') << std::setw(11) << "Partition"
              << std::setw(2) << "|"
              << std::setfill(' ') << std::setw(5) << "Size"
              << std::setw(2) << "|"
              << std::setfill(' ') << std::setw(13) << "Arrival Time"
              << std::setw(2) << "|"
              << std::setfill(' ') << std::setw(11) << "Start Time"
              << std::setw(2) << "|"
              << std::setfill(' ') << std::setw(14) << "Remaining Time"
              << std::setw(2) << "|"
              << std::setfill(' ') << std::setw(11) << "State"
              << std::setw(2) << "|" << std::endl;

    // Print separator
    buffer << "+" << std::setfill('-') << std::setw(tableWidth) << "+" << std::endl;

    // Print each PCB entry
    for (const auto& program : _PCB) {
        buffer << "|"
                  << std::setfill(' ') << std::setw(4) << program.PID
                  << std::setw(2) << "|"
                  << std::setw(11) << program.partition_number
                  << std::setw(2) << "|"
                  << std::setw(5) << program.size
                  << std::setw(2) << "|"
                  << std::setw(13) << program.arrival_time
                  << std::setw(2) << "|"
                  << std::setw(11) << program.start_time
                  << std::setw(2) << "|"
                  << std::setw(14) << program.remaining_time
                  << std::setw(2) << "|"
                  << std::setw(11) << program.state
                  << std::setw(2) << "|" << std::endl;
    }

    // Print bottom border
    buffer << "+" << std::setfill('-') << std::setw(tableWidth) << "+" << std::endl;

    return buffer.str();
}

//Overloaded function that takes a single PCB as input
std::string print_PCB(PCB _PCB) {
    std::vector<PCB> temp;
    temp.push_back(_PCB);
    return print_PCB(temp);
}

std::string print_exec_header() {

    const int tableWidth = 49;

    std::stringstream buffer;

    // Print top border
    buffer << "+" << std::setfill('-') << std::setw(tableWidth) << "+" << std::endl;

    // Print headers
    buffer  << "|"
            << std::setfill(' ') << std::setw(18) << "Time of Transition"
            << std::setw(2) << "|"
            << std::setfill(' ') << std::setw(3) << "PID"
            << std::setw(2) << "|"
            << std::setfill(' ') << std::setw(10) << "Old State"
            << std::setw(2) << "|"
            << std::setfill(' ') << std::setw(10) << "New State"
            << std::setw(2) << "|" << std::endl;

    // Print separator
    buffer << "+" << std::setfill('-') << std::setw(tableWidth) << "+" << std::endl;

    return buffer.str();

}

std::string print_exec_status(unsigned int current_time, int PID, states old_state, states new_state) {

    const int tableWidth = 49;

    std::stringstream buffer;

    buffer  << "|"
            << std::setfill(' ') << std::setw(18) << current_time
            << std::setw(2) << "|"
            << std::setw(3) << PID
            << std::setw(2) << "|"
            << std::setw(10) << old_state
            << std::setw(2) << "|"
            << std::setw(10) << new_state
            << std::setw(2) << "|" << std::endl;

    return buffer.str();
}

std::string print_exec_footer() {
    const int tableWidth = 49;
    std::stringstream buffer;

    // Print bottom border
    buffer << "+" << std::setfill('-') << std::setw(tableWidth) << "+" << std::endl;

    return buffer.str();
}

//Synchronize the process in the process queue
void sync_queue(std::vector<PCB> &process_queue, PCB _process) {
    for(auto &process : process_queue) {
        if(process.PID == _process.PID) {
            process = _process;
        }
    }
}

//Writes a string to a file
void write_output(std::string execution, const char* filename) {
    std::ofstream output_file(filename);

    if (output_file.is_open()) {
        output_file << execution;
        output_file.close();  // Close the file when done
        std::cout << "File content overwritten successfully." << std::endl;
    } else {
        std::cerr << "Error opening file!" << std::endl;
    }

    std::cout << "Output generated in " << filename << ".txt" << std::endl;
}

//--------------------------------------------FUNCTIONS FOR THE "OS"-------------------------------------

//Assign memory partition to program
bool assign_memory(PCB &program) {
    int size_to_fit = program.size;
    int available_size = 0;

    for(int i = 5; i >= 0; i--) {
        available_size = memory_paritions[i].size;

        if(size_to_fit <= available_size && memory_paritions[i].occupied == -1) {
            memory_paritions[i].occupied = program.PID;
            program.partition_number = memory_paritions[i].partition_number;
            return true;
        }
    }

    return false;
}

//Free a memory partition
bool free_memory(PCB &program){
    for(int i = 5; i >= 0; i--) {
        if(program.PID == memory_paritions[i].occupied) {
            memory_paritions[i].occupied = -1;
            program.partition_number = -1;
            return true;
        }
    }
    return false;
}

//Convert a list of strings into a PCB
PCB add_process(std::vector<std::string> tokens) {
    PCB process;
    process.PID = std::stoi(tokens[0]);
    process.size = std::stoi(tokens[1]);
    process.arrival_time = std::stoi(tokens[2]);
    process.processing_time = std::stoi(tokens[3]);
    process.remaining_time = std::stoi(tokens[3]);
    process.io_freq = std::stoi(tokens[4]);
    process.io_duration = std::stoi(tokens[5]);

    // added priority for some algorithms
    process.priority = std::stoi(tokens[6]);

    process.start_time = -1;
    process.partition_number = -1;
    process.state = NOT_ASSIGNED;

    return process;
}

//Returns true if all processes in the queue have terminated
bool all_process_terminated(std::vector<PCB> processes) {

    for(auto process : processes) {
        if(process.state != TERMINATED) {
            return false;
        }
    }

    return true;
}

//Terminates a given process
void terminate_process(PCB &running, std::vector<PCB> &job_queue) {
    running.remaining_time = 0;
    running.state = TERMINATED;
    free_memory(running);
    sync_queue(job_queue, running);
}

//set the process in the ready queue to runnning
void run_process(PCB &running, std::vector<PCB> &job_queue, std::vector<PCB> &ready_queue, unsigned int current_time) {
    running = ready_queue.back();
    ready_queue.pop_back();
    running.start_time = current_time;
    running.state = RUNNING;
    sync_queue(job_queue, running);
}

void idle_CPU(PCB &running) {
    running.start_time = 0;
    running.processing_time = 0;
    running.remaining_time = 0;
    running.arrival_time = 0;
    running.io_duration = 0;
    running.io_freq = 0;
    running.partition_number = 0;
    running.size = 0;
    running.state = NOT_ASSIGNED;
    running.PID = -1;
}


//========================================================================================
//             CUSTOM FUNCTION FOR MEMORY AND TRANSITION ANALYSIS (BONUS)
//========================================================================================
// Transition record (per tick)
struct Event {
    unsigned int time;
    int pid;
    states old_state;
    states new_state;
};

// Build consolidated memory + transition and queue report for one time tick
std::string parseEvents(unsigned int current_time,
                        const std::vector<Event>& transitions,
                        const std::vector<PCB>& job_list,
                        const std::vector<PCB>& ready_queue,
                        const std::vector<PCB>& wait_queue,
                        const PCB& running) {
    if (transitions.empty()) {return std::string();}

    // This is basically a short summary of what transitions happened at this time frame
    std::string output;
    output += "\n--- Memory / Events at Time " + std::to_string(current_time) + " ---\n";
    output += "Transitions this tick:\n";
    output += "| PID | Old -> New\n";
    output += "|-----|-------------\n";

    // turn transition ENUM into string for printing
    for (auto &t : transitions) {
        output += (t.pid == -1 || t.pid > 9 ? "| " : "|  ") + std::to_string(t.pid) + "  | " +
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
    // This will show the states of each pid, as well as the order they arrive in (FIFO)
    // Note that they will not necessarily leave the state in FIFO, that will depend on the scheduling algorithm
    output += "\nProcess State:\n";
    output += "| NEW | READY | WAITING | RUNNING | TERMINATED |\n";
    output += "|-----|-------|---------|---------|------------|";

    // Collect PIDs for each state
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

    // Find max rows needed
    // vector.size() returns size_t so we need to use that
    size_t max_rows = std::max({new_pids.size(),
                                ready_pids.size(),
                                waiting_pids.size(),
                                (size_t)(running_pid != -1 ? 1 : 0),
                                terminated_pids.size()});
    if (max_rows == 0) {max_rows = 1;} // At least one row

    // Print rows
    for (size_t i = 0; i < max_rows; i++) {
        output += "\n|";

        // NEW column
        if (i < new_pids.size()) {
            output += "  " + std::to_string(new_pids[i]) + (new_pids[i] < 10 ? "  |" : " |");
        } else {
            output += "  -  |";
        }

        // READY column
        if (i < ready_pids.size()) {
            output += "   " + std::to_string(ready_pids[i]) + (ready_pids[i] < 10 ? "   |" : "  |");
        } else {
            output += "   -   |";
        }

        // WAITING column
        if (i < waiting_pids.size()) {
            output += "    " + std::to_string(waiting_pids[i]) + (waiting_pids[i] < 10 ? "    |" : "   |");
        }else {
            output += "    -    |";
        }

        // RUNNING column
        if (i == 0) {
            if (running_pid != -1) {
                output += "    " + std::to_string(running_pid) + (running_pid < 10 ? "    |" : "   |");
            } else {
                output += "    -    |";
            }
        } else {
            output += "    -    |";
        }

        // TERMINATED column
        if (i < terminated_pids.size()) {
            output += "     " + std::to_string(terminated_pids[i]) + (terminated_pids[i] < 10 ? "      |" : "     |");
        } else {
            output += "     -      |";
        }
    }
    output += "\n";

    // Final table: Shows the usage of all partitions, as well as by which PID
    output += "\nPartition Usage:\n";
    output += "| Part | Size | Used | Unused | PID |\n";
    output += "|------|------|------|--------|-----|\n";
    for (int i = 0; i < 6; i++) {
        const memory_partition &part = memory_paritions[i];
        int pid = part.occupied;
        unsigned int used = 0;
        states proc_state = NOT_ASSIGNED;

        if (pid != -1) {
            for (size_t j = 0; j < job_list.size(); j++) {
                if (job_list[j].PID == pid) {
                    used = job_list[j].size;
                    proc_state = job_list[j].state;
                    break;
                }
            }
        }

        unsigned int unused = (pid == -1) ? part.size : (part.size > used ? part.size - used : 0);
        // Build row without state
        output += "|  " + std::to_string(part.partition_number) + "   |  " +
                std::to_string(part.size) +
                    (part.size < 10 ? "   |  " : "  |  ") +
                std::to_string(used) +
                    (used < 10 ? "   |   " : "  |   ") +
                std::to_string(unused) +
                    (unused < 10 ? "    | " : "   | ") +
                (pid <= 9 && pid > -1 ? " " : "") +
                (pid == -1 ? std::string("-1") : std::to_string(pid)) +
                    (pid == -1 || pid > 9 ? "  |\n" : "  |\n");
    }

    output += "\n" + std::string(70, '=') + "\n";

    return output;
}

// Calculates and returns the scheduling metrics string
inline std::string calculate_metrics(const std::vector<PCB>& list_processes,
                                        const std::map<int, unsigned int>& completion_times,
                                        const std::map<int, unsigned int>& first_run_times,
                                        unsigned int current_time) {
    unsigned int total_turnaround = 0;
    unsigned int total_wait = 0;
    unsigned int total_response = 0;
    unsigned int num_processes = list_processes.size();

    for (const auto& process : list_processes) {
        unsigned int turnaround = completion_times.at(process.PID) - process.arrival_time;
        unsigned int response = first_run_times.at(process.PID) - process.arrival_time;
        unsigned int wait = turnaround - process.processing_time;

        total_turnaround += turnaround;
        total_response += response;
        total_wait += wait;
    }

    double avg_turnaround = (double)total_turnaround / num_processes;
    double avg_wait = (double)total_wait / num_processes;
    double avg_response = (double)total_response / num_processes;
    double throughput =  current_time/(double)num_processes;

    std::string metrics = "\n\n========== Scheduling Metrics ==========";
    metrics += "\nThroughput:              " + std::to_string(throughput) + " ms/process";
    metrics += "\nAverage Turnaround Time: " + std::to_string(avg_turnaround) + " ms";
    metrics += "\nAverage Wait Time:       " + std::to_string(avg_wait) + " ms";
    metrics += "\nAverage Response Time:   " + std::to_string(avg_response) + " ms";
    metrics += "\n========================================\n";
    return metrics;
}

#endif