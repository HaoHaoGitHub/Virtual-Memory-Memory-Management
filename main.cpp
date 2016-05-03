//
//  main.cpp
//  OpSys_p3
//
//  Created by Chelsey on 5/2/16.
//  Copyright (c) 2016 Chelsey. All rights reserved.
//

// ======================
#include <iostream>
#include <map>
#include "pair.h"
#include <fstream>
#include <sstream>
#include <iterator>
#include <string>
#include <stdio.h>
#include <assert.h>
using namespace std;

typedef map<Proc_pair, int, Comparer> my_map;

/********************** helper function for debugging*********************/
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
    //
    return 0;
}







