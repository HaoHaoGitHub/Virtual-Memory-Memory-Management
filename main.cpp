// Virtual Memory & Memory management 

#include <iostream>
#include <map>
#include "pair.h"
#include "memory.h"
#include <fstream>
#include <sstream>
#include <iterator>
#include <string>
#include <stdio.h>
#include <assert.h>
using namespace std;

# define t_memmove 1 /* time of moving each unit in defragmentation  */
int final_t;
int defrag_t;


typedef map<Proc_pair, int, Comparer> my_map;

/*======================== helper function for debugging====================================*/
void print_event_map(const my_map& event_map) {
    for (my_map::const_iterator itr = event_map.begin(); itr != event_map.end(); ++itr) {
        cout << " | " << itr->first.time << " " << itr->first.proc.p_id << " " << itr->first.proc.p_mem
        << " | " << itr->second << " | " << endl;
        cout << "-------------" << endl;
    }
}


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
            event_map.insert(make_pair(tmp_key, 0)); // 0 means start use memory, 1 means finish using memory
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


int main(int argc, const char * argv[]) {
    my_map event_map;
    ifstream in_str(argv[1]);
    if (!in_str.good()) {
        cerr << "Can't open " << argv[1] << "to read.\n";
        exit(1);
    }
    read_file(in_str, event_map);
    print_event_map(event_map);

    Memory memory;
    memory.reset();
    defrag_t = 0;
    
    /* start simulator for each alg */
    cout << "time 0ms: Simulator started (Contiguous -- First-Fit)" << endl;

    for (my_map::iterator itr = event_map.begin(); itr != event_map.end(); ++itr) {
        Process p = itr->first.proc;
        int temp_t = itr->first.time;
        char process_id = p.p_id;
        int mem = p.p_mem;
        int arriving_status = itr->second;
        
  
        /* First we need to check the arriving status. */
        if (arriving_status == 0) { // If arrive: 

            // If the number of free memory frames is less than memmory frames needed: skipping:  
            if (memory.num_free < mem) {
                cout << "time " << temp_t << "ms: Process " << process_id << " arrived " <<
                    "(requires " << mem << " frames of physical memory)" << endl;
                cout << "time " << temp_t << "ms: Cannot place process " << process_id << 
                    " -- skipping process " << process_id << endl;  
                memory.print();
                // Erase this process record from event_map (including both arriving and leaving one):
                my_map::iterator temp_itr = itr;
                temp_itr++;
                for (; temp_itr != event_map.end(); ++temp_itr) {
                    Process temp_p = temp_itr->first.proc;
                    char temp_id = temp_p.p_id;
                    if (temp_id == process_id) event_map.erase(temp_itr);
                }

            } 
            // If not skipping: 
            else { 

                if (memory.allocate(process_id, mem, "first-fit") == false) {
                    /* defragmentation */
                    /* ---------------------------------------------------- */
                    cout << "time " << temp_t << "ms: Process " << process_id << " arrived " <<
                        "(requires " << mem << " frames of physical memory)" << endl;
                    cout << "time " << temp_t << "ms: Cannot place process " << process_id << 
                        " -- starting defragmentation " << endl;

                    int num_move = memory.defragmentation();
                    int move_time = num_move * t_memmove;
                    int new_time = temp_t + move_time;
                    
                    cout << "time " << new_time << "ms: Defragmentation complete (move " << num_move
                         << " frames:    " << endl;
                    
                    memory.print();

                    /* for those 'suspending' events, add defragmentation time to related events */
                    my_map::iterator temp_itr = itr;
                    temp_itr++;
                    my_map new_event_map;

                    for (; temp_itr != event_map.end(); ++temp_itr) {
                        Process p1 = temp_itr->first.proc;
                        int temp_t1 = temp_itr->first.time;
                        char process_id1 = p1.p_id;
                        int mem1 = p1.p_mem;
                        int arriving_status1 = temp_itr->second;
                        // cout << "process id is : " << temp_t1 << " " << process_id1 << endl;
                        int new_t = temp_t1 + move_time;


                        Process temp_p(process_id1, mem1);
                        Proc_pair tmp_key(new_t, temp_p);
                        new_event_map.insert(make_pair(tmp_key, arriving_status1));
                    }
                    event_map = new_event_map;
                    itr = event_map.begin();
                    print_event_map(event_map);

                } else { // allocate successfully
                    cout << "time " << temp_t << "ms: Process " << process_id << " arrived " <<
                         "(requires " << mem << " frames of physical memory)" << endl;
                    cout << "time " << temp_t << "ms: Placed process " << process_id << " in memory:" << endl;
                    memory.print();
                }
            }

        } else { // If leave: 
            cout << "time " << temp_t << "ms: Process " << process_id << " removed from physical memory" << endl;
            memory.deallocate(process_id, mem, "first-fit");
            memory.print();
        }

    }





    


    return 0;
}







