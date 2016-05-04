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
#include <vector>
#include "virtual_memory.h"
#include <list>
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
//================================================Virtual Memory==================================
void read_file_2(ifstream &in_str, vector<int> &page_reference, map<int,list<int> > &page_index)
{
    int count = 0;
    while(in_str)
    {
        int tmp;
        in_str>>tmp;
        page_reference.push_back(tmp);
        page_index[tmp].push_back(count);
        count++;
    }
}

void print_frame( int frame[])
{
    cout<<"[mem: ";
    for(int i = 0 ; i < 3;i++)
    {
        if(frame[i] > 0)
        {
            cout<<frame[i]<<" ";
        }
        else
        {
            cout<<". ";
        }
    }
    cout<<"]";
}

//------------------------------------------------OPT replacement algo----------------------------
void OPT(vector<int> page_reference, map<int,list<int> > page_index, Virtual_memory virtual_memory)
{
    int count_fault = 0;
    cout<<"Simulating OPT with fixed frame size of 3"<<endl;
    for(int i = 0; i < page_reference.size();i++)
    {
        bool is_find = false;
        for(int l = 0 ; l < 3;l++)
        {
            if(page_reference[i] == virtual_memory.frame[l])
            {
                is_find = true;
                break;
            }
        }
        if( !is_find)
        {
            if(virtual_memory.get_left_unused() > 0)
            {
                    int j = 0;
                    while(virtual_memory.frame[j] > 0)
                    {
                        j++;
                    }
                    virtual_memory.frame[j] = page_reference[i];
                    page_index[page_reference[i]].pop_front();
                    int tmp = virtual_memory.get_left_unused();
                    virtual_memory.set_left_unused(--tmp);
                    cout<<"referencing page "<<page_reference[i]<<" ";
                    print_frame(virtual_memory.frame);
                    cout<<" PAGE FAULT (no victim page)"<<endl;
                    count_fault++;
            }
            else
            {
                int replace_page = -1;
                int replace_page_index = -1;
                int replace_page_frame_index = -1;
                for(int m = 0 ; m < 3; m++)
                {
                    int tmp;
                    list<int>::iterator itr = page_index[virtual_memory.frame[m]].begin();
                    if(itr != page_index[virtual_memory.frame[m]].end())
                    {
                        tmp = *itr;
                    }
                    if(tmp > replace_page_index)
                    {
                        replace_page = virtual_memory.frame[m];
                        replace_page_frame_index = m;
                        replace_page_index = tmp;
                    }
                }
                int victim_page = virtual_memory.frame[replace_page_frame_index];
                virtual_memory.frame[replace_page_frame_index] = page_reference[i];
                page_index[page_reference[i]].pop_front();
                cout<<"referencing page "<<page_reference[i]<<" ";
                print_frame(virtual_memory.frame);
                cout<<" PAGE FAULT (victim page "<<victim_page<<")"<<endl;
                count_fault++;

            }
        }
        else
        {
            cout<<"referencing page "<<page_reference[i]<<" ";
            print_frame(virtual_memory.frame);
            cout<<endl;
            page_index[page_reference[i]].pop_front();
        }
    }
    cout<<"End of OPT simulation ("<<count_fault<<" page faults)"<<endl;
}

//================================================Virtual Memory==================================


int main(int argc, const char * argv[]) {
    my_map event_map;
    ifstream in_str(argv[1]);
    if (!in_str.good()) {
        cerr << "Can't open " << argv[1] << "to read.\n";
        exit(1);
    }
    read_file(in_str, event_map);
    print_event_map(event_map);
//================================================Virtual Memory==================================
    ifstream in_str_2(argv[2]);
    map<int, list<int> > page_index;
    vector<int> page_reference;
    read_file_2(in_str_2,page_reference,page_index);
    Virtual_memory virtual_memory;
    OPT(page_reference,page_index,virtual_memory);


    return 0;
}







