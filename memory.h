#include <iostream>
#include <map>
#include <vector>

using namespace std;

/* ==================== Header and functions for Memory class ======================*/
/* Memory class header  */
typedef map<int, int> mem_map;

class Memory {
public:
    /* public varables  */
    vector<char> mem;
    int num_used;
    int num_free;
    int last_index;       /* last used index          */
    mem_map index_map;    /* map with index as key    */
    mem_map space_map;    /* map with space as map    */
    map<int, pair<char, int> > char_map;    

    /* constructors */
    Memory();
    
    /* member functions */
    bool allocate(const char& p_id, const int& num_space, const string& alg);
    void deallocate(const char& p_id, const int& num_space, const string& alg);
    int defragmentation();
    void print();
    void print_map();
    void print_s_map();
    void reset();
    void set_partition(const int& s_index, const int& num_space, const string& alg);
};

/* default constructor for Memory class */
Memory::Memory() {
    /* allocate a martix of char '.' with size 8*32 */
    for (unsigned int i = 0; i < 256; ++i) {
        mem.push_back('.');
    }

    num_used = 0;
    num_free = 256;
    index_map.insert(make_pair(0,256));
    space_map.insert(make_pair(256,0));
}

/* allocate memeories   */
bool Memory::allocate(const char& p_id, const int& num_space, const string& alg) {
    if (alg == "first-fit") {
        /* loop index map to allocate memory */
        for (mem_map::iterator itr = index_map.begin(); itr != index_map.end(); ++itr) {
            if (itr->second >= num_space) {
                unsigned int index = itr->first;
                for (unsigned int i = 0; i < num_space; ++i) {
                    mem[index] = p_id;
                    ++index;
                }
                num_used += num_space;
                num_free -= num_space;
                index_map.insert(make_pair(itr->first + num_space, itr->second - num_space));
                char_map.insert(make_pair(itr->first, make_pair(p_id, num_space)));
                index_map.erase(itr);
                return true;
            }
        }
        return false;
    }
    
    if (alg == "next-fit") {
        /* loop index map from end to beginning  */
        mem_map::iterator itr = index_map.end();
        --itr;
        while (1) {
            if (itr->second >= num_space) {
                unsigned int index = itr->first;
                for (unsigned int i = 0; i < num_space; ++i) {
                    mem[index] = p_id;
                    ++index;
                }
                num_used += num_space;
                num_free -= num_space;
                index_map.insert(make_pair(itr->first + num_space, itr->second - num_space));
                char_map.insert(make_pair(itr->first, make_pair(p_id, num_space)));
                index_map.erase(itr);
                return true;
            }
            --itr;
            mem_map::iterator tmp = itr;
            if (--tmp == index_map.begin()) break;
        }
        return false;
    }

    if (alg == "best-fit") {
        /* loop space map to allocate memory    */
        for (mem_map::iterator itr = space_map.begin(); itr != space_map.end(); ++itr) {
            if (itr->first >= num_space) {
                unsigned int index = itr->second;
                for (unsigned int i = 0; i < num_space; ++i) {
                    mem[index] = p_id;
                    ++index;
                }
                num_used += num_space;
                num_free -= num_space;
                space_map.insert(make_pair(itr->first - num_space, itr->second + num_space));
                char_map.insert(make_pair(itr->second, make_pair(p_id, num_space)));
                space_map.erase(itr);
                return true;
            }
        }
        return false;
    }

    return false;
}

/* set up partition */
void Memory::set_partition(const int& s_index, const int& num_space, const string& alg) {
        mem_map* map_p;
        if (alg == "first-fit" || alg == "next-fit") map_p = &index_map;
        if (alg == "best-fit") map_p = &space_map;

        bool merge_left = false;
        bool merge_right = false;
        int front_i, front_space, end_space;
        if (s_index - 1 >= 0 && mem[s_index - 1] == '.') {
            merge_left = true;
            for (mem_map::iterator itr = (*map_p).begin(); itr != (*map_p).end(); ++itr) {
                int map_index, map_space;
                if (alg == "first-fit" || alg == "next-fit") {
                    map_index = itr->first;
                    map_space = itr->second;
                }
                if (alg == "best-fit") {
                    map_index = itr->second;
                    map_space = itr->first;
                }
                if (map_index + map_space == s_index) {
                    front_i = map_index;
                    front_space = map_space;
                    map_p->erase(itr);
                    break;
                }
            }
        }
        if (s_index + num_space + 1 < 256 && mem[s_index + num_space + 1] == '.') {
            merge_right = true;
            for (mem_map::iterator itr = (*map_p).begin(); itr != (*map_p).end(); ++itr) {
                int map_index, map_space;
                if (alg == "first-fit" || alg == "next-fit") {
                    map_index = itr->first;
                    map_space = itr->second;
                }
                if (alg == "best-fit") {
                    map_index = itr->second;
                    map_space = itr->first;
                }
                if (map_index == s_index + num_space) {
                    end_space = map_space;
                    map_p->erase(itr);
                    break;
                }
            }
        }
        int new_index;
        int new_space;
        if (merge_left && merge_right) {
            new_index = front_i;
            new_space = front_space + num_space + end_space;
        }
        else if (merge_left) {
            new_index = front_i;
            new_space = front_space + num_space;
        }    
        else if (merge_right) {
            new_index = s_index;
            new_space = num_space + end_space;
        }
        else {
            new_index = s_index;
            new_space = num_space;
        }
        if (alg == "first-fit" || "next-fit") map_p->insert(make_pair(new_index, new_space));
        if (alg == "best-fit") map_p->insert(make_pair(new_space, new_index));
}

/* deallocate memories  */
void Memory::deallocate(const char& p_id, const int& num_space, const string& alg) {
    int s_index = -1;
    for (unsigned int i = 0; i < 256; ++i) {
        if (mem[i] == p_id) {
            if (s_index == -1) s_index = i;
            mem[i] = '.';
        }
    }
    num_used -= num_space;
    num_free += num_space;

    /* combine partitions if necessary  */
    set_partition(s_index, num_space, alg);

    map<int, pair<char, int> >::iterator itr = char_map.begin();
    for (; itr != char_map.end(); ++itr) {
        if (itr->second.first == p_id) {
            char_map.erase(itr);
        }
    }
}

/* defragmentation  */
int Memory::defragmentation() {
 map<int, pair<char, int> >::iterator itr = char_map.begin();
    int index = 0, num_removed = 0;
    map<int, pair<char, int> > new_map;
    for (; itr != char_map.end(); ++itr) {
        new_map.insert(make_pair(index, itr->second));
        for (unsigned int i = 0; i < itr->second.second; ++i) {
            mem[index] = itr->second.first;
            ++index;
        }
        // check if the first block should be moved
        if (itr == char_map.begin()) {
            if (itr->first != 0)
                num_removed += itr->second.second; // if the first block should be moved
        } else num_removed += itr->second.second;
    }
//    --itr;
//    int last_index = itr->first+itr->second.second;
//    int num_removed = last_index - num_used;
    index_map.clear();
    space_map.clear();
    char_map.clear();
    index_map.insert(make_pair(num_used, num_free));
    space_map.insert(make_pair(num_free, num_used));
    char_map = new_map;
    
    return num_removed;
}

/* print out memory */
void Memory::print() {
    for (unsigned int i = 0; i < 32; ++i) cout << "=";
    cout << endl;
    unsigned int index = 0;
    for (unsigned int i = 0; i < 8; ++i) {
        for (unsigned int j = 0; j < 32; ++j) {
            cout << mem[index];
            ++index;
        }
        cout << endl;
    }
    for (unsigned int i = 0; i < 32; ++i) cout << "=";
    cout << endl;
}

/* helper functions, print index map    */
void Memory::print_map() {
    for (mem_map::iterator itr = index_map.begin(); itr != index_map.end(); ++itr) {
        cout << itr->first << " " << itr->second << endl;
    }
}

/* helper functions, print space map    */
void Memory::print_s_map() {
    for (mem_map::iterator itr = space_map.begin(); itr != space_map.end(); ++itr) {
        cout << itr->first << " " << itr->second << endl;
    }
}
/* reset Memory */
void Memory::reset() {
    mem.clear();
    index_map.clear();
    space_map.clear();
   
    /* allocate a martix of char '.' with size 8*32 */
    for (unsigned int i = 0; i < 256; ++i) {
        mem.push_back('.');
    }

    num_used = 0;
    num_free = 256;
    index_map.insert(make_pair(0,256));
    space_map.insert(make_pair(256,0));
}
