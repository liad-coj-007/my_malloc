#include <stdio.h>
#include "malloc_3.h"
using namespace std;
#include <assert.h>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <string>
class TestMalloc {
private:
    map<int, int*> allocations;
    TestMalloc() {}

    void PrintTitle(const std::string& title){
         int width = 80;
         int padding = (width - title.size()) / 2;

        std::cout << std::string(padding, ' ') << title << std::endl;
        std::cout << std::string(padding, ' ') <<
        std::string(title.size(), '=') << std::endl;
    }

public:
    static TestMalloc& getInstance() {
        static TestMalloc instance;
        return instance;
    }
    TestMalloc(const TestMalloc&) = delete;
    void operator=(const TestMalloc&) = delete;

    int addAllocation(int id,size_t size){
        if (allocations.find(id) != allocations.end()) {
            return -1; // ID already exists
        }

        int* ptr = (int*)smalloc(size*sizeof(int));
        allocations[id] = ptr;
        return 0;
    }

    int add_realloc(int oldid,size_t size,int newid){
        if (allocations.find(oldid) == allocations.end()) {
            return -1; // ID does not exist
        }
        int* ptr = (int*)srealloc(allocations[oldid], size*sizeof(int));
        allocations[newid] = ptr;
        return 0;
    }

    int freeAllocation(int id) {
        if (allocations.find(id) == allocations.end()) {
            return -1; // ID does not exist
        }
        sfree(allocations[id]);
        allocations.erase(id);
        return 0;
    }

    int scallocAllocation(int id, size_t num, size_t size) {
        if (allocations.find(id) != allocations.end()) {
            return -1; // ID already exists
        }
        int* ptr = (int*)scalloc(num, size*sizeof(int));
        allocations[id] = ptr;
        return 0;
    }

    void PrintStatistics(){
        size_t free_blocks = _num_free_blocks();
        size_t free_bytes = _num_free_bytes();
        size_t allocated_blocks = _num_allocated_blocks();
        size_t allocated_bytes = _num_allocated_bytes();
        size_t meta_data_bytes = _num_meta_data_bytes();
        PrintTitle("Memory Statistics");
        std::cout << "Free blocks: " << free_blocks << std::endl;
        std::cout << "Free bytes: " << free_bytes << std::endl;
        std::cout << "Allocated blocks: " << allocated_blocks << std::endl;
        std::cout << "Allocated bytes: " << allocated_bytes << std::endl;
        std::cout << "Meta data bytes: " << meta_data_bytes << std::endl;

    }

    int* getPtr(int id) {
        if (allocations.find(id) == allocations.end())
            return nullptr;
        return allocations[id];
    }

    void PrintArray(int id,size_t start,size_t end){
        int* ptr = getPtr(id);
        if(ptr == nullptr) {
            std::cout << "Pointer with ID " << id << " does not exist." << std::endl;
            return;
        }
        cout << "[";
        for(size_t i = start; i < end; i++) {
            cout << ptr[i] << ", ";
        }
        cout << ptr[end] << "]" << endl; 
    }

    int eqptr(int id1,int id2,int sign,bool ans){
        int* ptr1 = getPtr(id1);
        int* ptr2 = getPtr(id2);
        if(sign == 0 && ptr1 == ptr2) {
            return ans ? 0 : -1; // equal
        } else if(sign == 1 && ptr1 != ptr2) {
            return ans ? 0 : -1; // not equal
        } else if(sign == 2 && ptr1 < ptr2) {
            return ans ? 0 : -1; // less than
        } else if(sign == 3 && ptr1 > ptr2) {
            return ans ? 0 : -1; // greater than
        } else if(sign == 4 && ptr1 <= ptr2) {
            return ans ? 0 : -1; // less than or equal
        } else if(sign == 5 && ptr1 >= ptr2) {
            return ans ? 0 : -1; // greater than or equal
        }

    }

    void PutVal(int id,int idx,int src){
        int* ptr = getPtr(id);
        if(ptr == nullptr) {
            std::cout << "Pointer with ID " << id << " does not exist." << std::endl;
            return;
        }
        ptr[idx] = src;
    }
};





void runCommand(const std::string& line) {
    std::stringstream ss(line);
    std::string cmd;
    ss >> cmd;
    TestMalloc& tm = TestMalloc::getInstance();
    if (cmd == "add") {
        int id;
        size_t size;
        ss >> id >> size;
        assert(tm.addAllocation(id, size) == 0);
    } else if (cmd == "realloc") {
        int oldid, newid;
        size_t size;
        ss >> oldid >> size >> newid;
        assert(tm.add_realloc(oldid, size, newid) == 0);
    } else if (cmd == "scalloc") {
        int id;
        size_t num, size;
        ss >> id >> num >> size;
        assert(tm.scallocAllocation(id, num, size) == 0);
    }else if (cmd == "print") {
        int id;
        size_t start, end;
        ss >> id >> start >> end;
        tm.PrintArray(id, start, end);
    }else if(cmd == "eq"){
        int id1, id2, sign;
        bool ans;
        ss >> id1 >> id2 >> sign >> ans;
        assert(tm.eqptr(id1, id2, sign, ans) == 0);
    } else if (cmd == "print_stats") {
        tm.PrintStatistics();
    }else if(cmd == "put"){
        int id, idx;
        int src;
        ss >> id >> idx >> src;
        tm.PutVal(id, idx, src);
    }else if (cmd == "free") {
        int id;
        ss >> id;
        assert(tm.freeAllocation(id) == 0);
    } else {
        std::cerr << "Unknown command: " << cmd << std::endl;
    }
}


int main(int argc, char* argv[]) {
    std::istream* input = &std::cin;
    std::ifstream file;

    if (argc > 1) {
        file.open(argv[1]);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << argv[1] << std::endl;
            return 1;
        }
        input = &file;
    }

    std::string line;
    while (std::getline(*input, line)) {
        if (line.empty() || line[0] == '#') continue;  
        runCommand(line);
    }
    return 0;
}
