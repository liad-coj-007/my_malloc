#include <iostream>
#include <string>
#include <cmath>
#include "malloc_3.h"
void* smalloc(size_t size);
void* scalloc(size_t num, size_t size);
void sfree(void* p);
void* srealloc(void* oldp, size_t size);

size_t _num_free_blocks();
size_t _num_free_bytes();
size_t _num_allocated_blocks();
size_t _num_allocated_bytes();
size_t _num_meta_data_bytes();
size_t _size_meta_data();


using std::cout;
using std::cin;
using std::endl;
using std::string;



bool expected_stats(size_t free_block, size_t free_byte, size_t allocated_block, size_t allocated_byte, string msg = "");


bool smalloc_size_zero();
bool smalloc_size_too_big();
bool scalloc_size_zero();
bool scalloc_size_too_big();
bool srealloc_size_zero();
bool srealloc_size_too_big();
bool sfree_nullptr();
bool size_check();

bool ten_to_zero_and_back();
bool full_blocks_X(int x);

bool scalloc_reset_check();

bool srealloc_small_no_change();
bool srealloc_small_merge_blocks();
bool srealloc_small_different_block();
bool srealloc_small_fail();
bool srealloc_small_copy();
bool srealloc_big_dec();
bool srealloc_big_stay();
bool srealloc_big_enc();






int main(){
    cout << _size_meta_data() << endl;
    cout << "Pack 1: fails" << endl;
    cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    cout << "smalloc fail when 0: " << smalloc_size_zero() << endl;
    cout << "smalloc fail when over 1e8: " << smalloc_size_too_big() << endl;
    cout << "scalloc fail when 0: " << scalloc_size_zero() << endl;
    cout << "scalloc fail when over 1e8: " << scalloc_size_too_big() << endl;
    cout << "srealloc fail when 0: " << srealloc_size_zero() << endl;
    cout << "srealloc fail when over 1e8: " << srealloc_size_too_big() << endl;
    cout << "sfree nullptr: " << sfree_nullptr() << endl;
    cout << "size check: " << size_check() << endl;


    cout << endl << "Pack 2: malloc + free usage" << endl;
    cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    cout << "ten to zero and back: " << ten_to_zero_and_back() << endl;
    cout << "full blocks 10: " << full_blocks_X(10) << endl;
    cout << "full blocks 9: " << full_blocks_X(9) << endl;
    cout << "full blocks 8: " << full_blocks_X(8) << endl;
    cout << "full blocks 7: " << full_blocks_X(7) << endl;
    cout << "full blocks 4: " << full_blocks_X(4) << endl;
    cout << "full blocks 6: " << full_blocks_X(6) << endl;
    cout << "full blocks 5: " << full_blocks_X(5) << endl;
    cout << "full blocks 3: " << full_blocks_X(3) << endl;
    cout << "full blocks 2: " << full_blocks_X(2) << endl;
    cout << "full blocks 1: " << full_blocks_X(1) << endl;
    cout << "full blocks 0: " << full_blocks_X(0) << endl;


    cout << endl << "Pack 3: scalloc resetting" << endl;
    cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    cout << "only test: " << scalloc_reset_check() << endl;


    cout << endl << "Pack 4: srealloc" << endl;
    cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    cout << "srealloc small no change: " << srealloc_small_no_change() << endl;
    cout << "srealloc small merge blocks: " << srealloc_small_merge_blocks() << endl;
    cout << "srealloc small different block: " << srealloc_small_different_block() << endl;
    cout << "srealloc small fail: " << srealloc_small_fail() << endl;
    cout << "srealloc small copy: " << srealloc_small_copy() << endl;
    cout << "srealloc big dec: " << srealloc_big_dec() << endl;
    cout << "srealloc big stay: " << srealloc_big_stay() << endl;
    cout << "srealloc big enc: " << srealloc_big_enc() << endl;
    cout << endl << "Tests over, go home!" << endl;
}


bool expected_stats(size_t free_block, size_t free_byte, size_t allocated_block, size_t allocated_byte, string msg){
    string wrong = "";
    bool bad = false;
    if(free_block != _num_free_blocks()){
        wrong += "\n-free blocks" + std::to_string(free_block) + "/" + std::to_string(_num_free_blocks());
        bad = true;
    }
    if(free_byte != _num_free_bytes()){
        wrong += "\n-free bytes" + std::to_string(free_byte) + "/" + std::to_string(_num_free_bytes());
        bad = true;
    }
    if(allocated_block != _num_allocated_blocks()){
        wrong += "\n-allocated blocks" + std::to_string(allocated_block) + "/" + std::to_string(_num_allocated_blocks());
        bad = true;
    }
    if(allocated_byte != _num_allocated_bytes()){
        wrong += "\n-allocated bytes" + std::to_string(allocated_byte) + "/" + std::to_string(_num_allocated_bytes());
        bad = true;
    }
    if(bad){
        cout << msg << "wrong inputs [expected] / [function]:" << wrong << endl;
    }
    return !bad;
}



bool smalloc_size_zero(){
    void* ptr = smalloc(0);
    return ptr == nullptr && expected_stats(32, 128*1024*32, 32, 128*1024*32);
}

bool smalloc_size_too_big() {
    void* ptr = smalloc(1e8 + 1);
    return ptr == nullptr && expected_stats(32, 128*1024*32, 32, 128*1024*32);
}

bool scalloc_size_zero(){
    void* ptr = scalloc(0, 0);
    return ptr == nullptr && expected_stats(32, 128*1024*32, 32, 128*1024*32);
}

bool scalloc_size_too_big(){
    void* ptr = scalloc(1e8 + 1, 1);
    return ptr == nullptr && expected_stats(32, 4194304, 32, 4194304);
}

bool srealloc_size_zero(){
    int a;
    void* ptr = srealloc(&a, 0);
    return ptr == nullptr && expected_stats(32, 128*1024*32, 32, 128*1024*32);
}

bool srealloc_size_too_big(){
    int a;
    void* ptr = srealloc(&a, 1e8 + 1);
    return ptr == nullptr && expected_stats(32, 128*1024*32, 32, 128*1024*32);
}

bool sfree_nullptr(){
    sfree(nullptr);
    return expected_stats(32, 128*1024*32, 32, 128*1024*32);
}

bool size_check(){
    return _size_meta_data() <= 64;
}


bool ten_to_zero_and_back(){
    bool all_good = expected_stats(32, 128*1024*32, 32, 128*1024*32);
    void* ptr = smalloc(1);
    all_good = all_good && expected_stats(41, 128*1024*32 - 128, 42, 128*1024*32);

    sfree(ptr);
    return all_good && expected_stats(32, 128*1024*32, 32, 128*1024*32);
}

bool full_blocks_X(int x){
    if(x < 0 || x > 10){
        return false;
    }
    int order_to_min_size[] = {1, 97, 225, 481, 993, 2017, 4065, 8161, 16353, 32737, 65505};
    int size = pow(2, x) * 128, amount = pow(2, 10-x);

    bool all_good = expected_stats(32, 128*1024*32, 32, 128*1024*32);
    void* origin = smalloc(1);
    sfree(origin);
    all_good = all_good && expected_stats(32, 128*1024*32, 32, 128*1024*32);

    void* ptr;
    for(int i = 0; i < amount * 32; i++){
        ptr = smalloc(order_to_min_size[x]);
        if(ptr != (void*)((char*)origin + size*i)){
            all_good = false;
            cout << "on smalloc " << i << " pointer is not expected (" << (void*)ptr << " instead of ";
            cout << (void*)((char*)origin + size*i) << ") - real = expected + ";
            cout << (long)ptr - (long)((char*)origin + size*i) << endl;
        }
    }
    all_good = all_good && expected_stats(0, 0, amount * 32, 128*1024*32);

    for(int i = 0; i < amount * 32; i++){
        sfree((char*)origin + size*i);
    }
    return all_good && expected_stats(32, 128*1024*32, 32, 128*1024*32);
}


bool scalloc_reset_check(){
    bool all_good = expected_stats(32, 128*1024*32, 32, 128*1024*32);
    char* low = (char*)smalloc(1);
    char* high = (char*)smalloc(30);
    all_good = all_good && expected_stats(40, 128*1024*32 - 128*2, 42, 128*1024*32);

    char* ptr = high + 25;
    ptr[0] = 'a';
    ptr[1] = 'b';
    ptr[2] = 'c';
    ptr[3] = 'd';
    ptr[4] = 'e';
    unsigned long diff = (unsigned long)(ptr) - (unsigned long)(low);

    sfree(low);
    sfree(high);
    all_good = all_good && expected_stats(32, 128*1024*32, 32, 128*1024*32);
    char* both = (char*)scalloc(diff + 2, 1);
    all_good = all_good && expected_stats(40, 128*1024*32 - 128*2, 41, 128*1024*32);
    sfree(both);
    all_good = all_good && expected_stats(32, 128*1024*32, 32, 128*1024*32);
    return all_good && ptr[0] == 0 && ptr[1] == 0&& ptr[2] == 'c' && ptr[3] == 'd' && ptr[4] == 'e';
}


bool srealloc_small_no_change(){
    bool all_good = expected_stats(32, 128*1024*32, 32, 128*1024*32);
    void* ptr = (void*)smalloc(100);
    all_good = all_good && expected_stats(40, 128*1024*32 - 128*2, 41, 128*1024*32);
    if(!all_good){
        cout << "1" << endl;
        exit(1);
    }
    void* same_ptr = srealloc(ptr, 1);
    all_good = all_good && expected_stats(40, 128*1024*32 - 128*2, 41, 128*1024*32);
    if(!all_good){
        cout << "2" << endl;
        exit(1);
    }
    void* identical_ptr = srealloc(ptr, 1);
    all_good = all_good && expected_stats(40, 128*1024*32 - 128*2, 41, 128*1024*32);
    if(!all_good){
        cout << "3" << endl;
        exit(1);
    }
    sfree(identical_ptr);
    all_good = all_good && expected_stats(32, 128*1024*32, 32, 128*1024*32);
    if(!all_good){
        cout << "4" << endl;
        exit(1);
    }
    return all_good && ptr == same_ptr && same_ptr == identical_ptr;
}

bool srealloc_small_merge_blocks(){
    bool all_good = expected_stats(32, 128*1024*32, 32, 128*1024*32);
    void* ptr = (void*)smalloc(100);
    all_good = all_good && expected_stats(40, 128*1024*32 - 128*2, 41, 128*1024*32);
    void* same_ptr = srealloc(ptr, 1000);
    all_good = all_good && expected_stats(37, 128*1024*32 - 128*pow(2, 4), 38, 128*1024*32);
    sfree(same_ptr);
    all_good = all_good && expected_stats(32, 128*1024*32, 32, 128*1024*32);
    return all_good && ptr == same_ptr;
}

bool srealloc_small_different_block(){
    bool all_good = expected_stats(32, 128*1024*32, 32, 128*1024*32);
    void* resize_ptr = smalloc(1);
    all_good = all_good && expected_stats(41, 128*1024*32 - 128, 42, 128*1024*32);
    void* blocker_ptr = smalloc(1);
    all_good = all_good && expected_stats(40, 128*1024*32 - 128 * 2, 42, 128*1024*32);

    void* different = srealloc(resize_ptr, 100);
    all_good = all_good && expected_stats(40, 128*1024*32 - 128 * 3, 42, 128*1024*32);
    if(!all_good){
        cout << "1" << endl;
        exit(1);
    }
    void* calc = (void*)((char*)resize_ptr + 128*2);

    sfree(different);
    all_good = all_good && expected_stats(41, 128*1024*32 - 128, 42, 128*1024*32);
    if(!all_good){
        cout << "2" << endl;
        exit(1);
    }
    sfree(blocker_ptr);
    all_good = all_good && expected_stats(32, 128*1024*32, 32, 128*1024*32);
    if(!all_good){
        cout << "3" << endl;
        exit(1);
    }
    return all_good && calc == different;
}

bool srealloc_small_fail(){
    bool all_good = expected_stats(32, 128*1024*32, 32, 128*1024*32);
    void* resize_ptr = smalloc(1);
    all_good = all_good && expected_stats(41, 128*1024*32 - 128, 42, 128*1024*32);
    void* blocker_ptr = smalloc(1);
    all_good = all_good && expected_stats(40, 128*1024*32 - 128 * 2, 42, 128*1024*32);
    if(!all_good){
        cout << "smalloc goes wrong2!" << endl;
        exit(1);
    }
    void* occupied[31];
    for(int i = 0; i < 31; i++){
        occupied[i] = smalloc(70000);
        
    }
    all_good = all_good && expected_stats(9, 128*1024 - 128 * 2, 42, 128*1024*32);
    if(!all_good){
        cout << "smalloc goes wrong3!" << endl;
        exit(1);
    }
    void* isnull = srealloc(resize_ptr, 70000);
    all_good = all_good && expected_stats(9, 128*1024 - 128 * 2, 42, 128*1024*32);
    if(!all_good ){
        cout << "1" << endl;
        exit(1);
    }

    for(int i = 0; i < 31; i++){
        sfree(occupied[i]);
    }
    all_good = all_good && expected_stats(40, 128*1024*32 - 128 * 2, 42, 128*1024*32);
    sfree(blocker_ptr);
    all_good = all_good && expected_stats(41, 128*1024*32 - 128, 42, 128*1024*32);
    sfree(resize_ptr);
    all_good = all_good && expected_stats(32, 128*1024*32, 32, 128*1024*32);
    return all_good && isnull == nullptr;
}

bool srealloc_small_copy() {
    bool all_good = expected_stats(32, 128*1024*32, 32, 128*1024*32);
    void* resize_ptr = smalloc(5);
    all_good = all_good && expected_stats(41, 128*1024*32 - 128, 42, 128*1024*32);
    void* blocker_ptr = smalloc(5);
    all_good = all_good && expected_stats(40, 128*1024*32 - 128 * 2, 42, 128*1024*32);

    ((char*)resize_ptr)[0] = 'a';
    ((char*)resize_ptr)[1] = 'b';
    ((char*)resize_ptr)[2] = 'c';
    ((char*)resize_ptr)[3] = 'd';
    ((char*)resize_ptr)[4] = 'e';

    void* different = srealloc(resize_ptr, 100);
    all_good = all_good && expected_stats(40, 128*1024*32 - 128 * 3, 42, 128*1024*32);
    void* calc = (void*)((char*)resize_ptr + 128*2);

    all_good = all_good && ((char*)different)[0] == 'a' && ((char*)different)[1] == 'b' &&
            ((char*)different)[2] == 'c' && ((char*)different)[3] == 'd' && ((char*)different)[4] == 'e';

    sfree(different);
    all_good = all_good && expected_stats(41, 128*1024*32 - 128, 42, 128*1024*32);
    sfree(blocker_ptr);
    all_good = all_good && expected_stats(32, 128*1024*32, 32, 128*1024*32);
    return all_good && calc == different;
}

bool srealloc_big_dec() {
    bool all_good = expected_stats(32, 128*1024*32, 32, 128*1024*32);
    void* old_ptr = smalloc(128*1024+2);
    all_good = all_good && expected_stats(32, 128*1024*32, 33, 128*1024*32 + 128*1024+2 + _size_meta_data());
    if(!all_good){
        cout << "1" << endl;
        exit(1);
    }
    for (int i = 0; i < 128*1024+2; i++) {
        ((char*)old_ptr)[i] = 'a';
    }

    void* new_ptr = srealloc(old_ptr, 128*1024+1);
    all_good = all_good && expected_stats(32, 128*1024*32, 33, 128*1024*32 + 128*1024+1 + _size_meta_data());
    if(!all_good){
        cout << "2" << endl;
        exit(1);
    }
    for (int i = 0; i < 128*1024+1; i++) {
        all_good = all_good && ((char*)new_ptr)[i] == 'a';
    }

    sfree(new_ptr);
    all_good = all_good && expected_stats(32, 128*1024*32, 32, 128*1024*32);
    if(!all_good){
        cout << "3" << endl;
        exit(1);
    }
    bool save =  new_ptr != old_ptr;
    return all_good ;
}

bool srealloc_big_stay() {
    bool all_good = expected_stats(32, 128*1024*32, 32, 128*1024*32);
    void* stay_ptr = smalloc(128*1024+1);
    all_good = all_good && expected_stats(32, 128*1024*32, 33, 128*1024*32 + 128*1024+1 + _size_meta_data());

    void* same_ptr = srealloc(stay_ptr, 128*1024+1);
    all_good = all_good && expected_stats(32, 128*1024*32, 33, 128*1024*32 + 128*1024+1 + _size_meta_data());
    sfree(stay_ptr);

    all_good = all_good && expected_stats(32, 128*1024*32, 32, 128*1024*32);
    return all_good && stay_ptr == same_ptr;
}

bool srealloc_big_enc() {
    bool all_good = expected_stats(32, 128*1024*32, 32, 128*1024*32);
    void* small_ptr = smalloc(128*1024+1);
    all_good = all_good && expected_stats(32, 128*1024*32, 33, 128*1024*32 + 128*1024+1 + _size_meta_data());
    for (int i = 0; i < 128*1024+1; i++) {
        ((char*)small_ptr)[i] = 'a';
    }

    void* big_ptr = srealloc(small_ptr, 128*1024+2);
    all_good = all_good && expected_stats(32, 128*1024*32, 33, 128*1024*32 + 128*1024+2 + _size_meta_data());
    for (int i = 0; i < 128*1024+1; i++) {
        all_good = all_good && ((char*)big_ptr)[i] == 'a';
    }

    sfree(big_ptr);
    all_good = all_good && expected_stats(32, 128*1024*32, 32, 128*1024*32);
    return all_good && small_ptr != big_ptr;
}


