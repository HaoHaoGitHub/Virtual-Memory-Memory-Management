// Virtual Memory & Memory management 
/* Compile as g++ main.cpp 
   Run as ./a.out test.txt
   Given input file name" test.txt */

#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>
#include <string>
#include <map>
#include <stdio.h>
#include <assert.h>
#include "pair.h"
#include "memory.h"
using namespace std;

# define t_memmove 1 /* time of moving each unit in defragmentation  */
typedef map<Proc_pair, int, Comparer> my_map; /* <[time_value, (process_id, memory units needed)], event_type> */
int final_t;

/* helper function for debugging */
void print_event_map(const my_map& event_map) {
    for (my_map::const_iterator itr = event_map.begin(); itr != event_map.end(); ++itr) {
        cout << " | " << itr->first.time << " " << itr->first.proc.p_id << " " << itr->first.proc.p_mem
        << " | " << itr->second << " | " << endl;
        cout << "-------------" << endl;
    }
}

/* parse each line of input and build the event_map */
void parse_a_line(string line, my_map& event_map){
    
    //get the process_id and the number of memory needed, then make a process
    char symbol = line[0];
    string mem_num = "";
    int i = 2;
    while (line[i] != ' ') mem_num += line[i++];
    Process tmp_p(symbol, stoi(mem_num));

    //continue parse the following time interval part
    i++;
    string tmp = "", start, end;
    for (; i < line.size(); ++i) {
        if (line[i] == '-') {
            start = tmp;
            // make a pair and insert to event_map
            Proc_pair tmp_key(stoi(start), tmp_p);
            event_map.insert(make_pair(tmp_key, 0)); // 0 means start using memory, 1 means finish using memory
            tmp = "";
        } else if (line[i] == ' ') {
            end = tmp;
            // make a pair and insert to event_map
            Proc_pair tmp_key(stoi(end), tmp_p);
            event_map.insert(make_pair(tmp_key, 1));
            tmp = "";
        } else {
            tmp += line[i];
            if (i == line.size() - 1) {
                end = tmp;
                // make a pair and insert to event_map
                Proc_pair tmp_key(stoi(end), tmp_p);
                event_map.insert(make_pair(tmp_key, 1));
            }
        }
    }
}

/* Read in input file*/
void read_file(ifstream &in_str, my_map& event_map) {
    string line;
    unsigned int num;
    getline(in_str, line);
    num = stoi(line);
    assert(num <=26); // num of process is less than 26
    for (; num > 0; --num){
        getline(in_str, line);
        parse_a_line(line, event_map);
    }
}
/* simulate each algorithm */
void start_simulate(my_map event_map, Memory memory, const string& alg) {
    // reset
    memory.reset();
    final_t = 0;
    bool defrag = false;

    cout << "time 0ms: Simulator started (";
    if(alg != "Non-contiguous") cout << "Contiguous -- ";
    cout << alg << ")" << endl;
    
    for (my_map::iterator itr = event_map.begin(); itr != event_map.end(); ++itr) {
        // if event_map is switched, return the itr to event_map.begin()
        if(defrag == true) {
            itr--;
            defrag = false;
        }
        // Process p = itr->first.proc;
        int tmp_t = itr->first.time;
        final_t = tmp_t;
        char p_id = itr->first.proc.p_id;
        int mem_num = itr->first.proc.p_mem;
        int event_type= itr->second;
        
  
        // if event type is 0, allocate memory
        if (event_type == 0) { 

            cout << "time " << tmp_t << "ms: Process " << p_id << " arrived " <<
                    "(requires " << mem_num << " frames of physical memory)" << endl;

            // If free memory units are not enough (num_free < mem_num): skipping:  
            if (memory.num_free < mem_num) {
                cout << "time " << tmp_t << "ms: Cannot place process " << p_id << 
                    " -- skipping process " << p_id << endl;  
                memory.print();

                // Erase this process record from event_map (including both arriving and leaving one):
                my_map::iterator tmp_itr = itr;
                tmp_itr++;
                for (; tmp_itr != event_map.end();) {
                    char tmp_id = tmp_itr->first.proc.p_id;
                    if (tmp_id == p_id) event_map.erase(tmp_itr++);
                    else tmp_itr++;
                }
            } 
            // If free memory units are enough (num_free >= mem_num): try allocating
            else { 

                // if cannot allocate directly, start defragmentation
                if (memory.allocate(p_id, mem_num, alg) == false) {

                    cout << "time " << tmp_t << "ms: Cannot place process " << p_id << 
                        " -- starting defragmentation " << endl;

                    vector<char>removed_chars;
                    int num_move = memory.defragmentation(removed_chars);
                    int time_move = num_move * t_memmove;
                    int new_time = tmp_t + num_move * t_memmove;;
                    
                    cout << "time " << new_time << "ms: Defragmentation complete (moved " << num_move
                         << " frames:";
                    for (unsigned int i = 0; i < removed_chars.size(); ++i) 
                        cout << " " << removed_chars[i];
                    cout << ")" << endl;
                    memory.print();

                    // place the process which triggered the defragmentation
                    cout << "time " << new_time << "ms: Placed process " << p_id << " in memory:" << endl;
                    memory.allocate(p_id, mem_num, alg);
                    memory.print();

                    /* for those 'suspending' events, add defragmentation time to related events */
                    my_map::iterator tmp_itr = itr;
                    tmp_itr++;
                    my_map new_event_map;

                    for (; tmp_itr != event_map.end(); ++tmp_itr) {
                        int new_t = tmp_itr->first.time + time_move;
                        Proc_pair tmp_key(new_t, tmp_itr->first.proc);
                        new_event_map.insert(make_pair(tmp_key, tmp_itr->second));
                    }
                    event_map = new_event_map;
                    defrag = true;
                    itr = event_map.begin();
                } 
                // if allocate successfully
                else { 
                    cout << "time " << tmp_t << "ms: Placed process " << p_id << " in memory:" << endl;
                    memory.print();
                }
            }
        } 
        // if event type is 1, remove memory
        else { 
            cout << "time " << tmp_t << "ms: Process " << p_id << " removed from physical memory" << endl;
            memory.deallocate(p_id, mem_num, alg);
            memory.print();
        }
    }
    cout << "time " << final_t << "ms: Simulator ended (";
    if(alg != "Non-contiguous") cout << "Contiguous -- ";
    cout << alg << ")\n\n" << endl;
}


int main(int argc, const char * argv[]) {

    my_map event_map; 
    ifstream in_str(argv[1]);
    if (!in_str.good()) {
        cerr << "Can't open " << argv[1] << "to read.\n";
        exit(1);
    }

    read_file(in_str, event_map);
    // print_event_map(event_map);

    Memory memory;
    // Start simulation
    start_simulate(event_map, memory, "First-Fit");
    start_simulate(event_map, memory, "Next-Fit");
    start_simulate(event_map, memory, "Best-Fit");
    start_simulate(event_map, memory, "Non-contiguous");
    return 0;
}









