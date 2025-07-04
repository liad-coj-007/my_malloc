#include <stdio.h>
#include "malloc_2.h"
using namespace std;
#include <assert.h>
#include <iostream>

int* fill_array(int size){
    int* arr = (int*)smalloc(size * sizeof(int));
    for(int i = 0; i < size; i++) {
        arr[i] = i+1;
    }
    return arr;
}

int TestReallocBig(){
    int* arr = fill_array(10);
    int* new_arr = (int*)srealloc(arr, 20 * sizeof(int));
    for(int i = 0; i < 10; i++){
        assert(new_arr[i] == i+1);
    }
    return 0;
}

int TestReallocSmall(){
    int* arr = fill_array(5);
    int* new_arr = (int*)srealloc(arr, 10 * sizeof(int));
    for(int i = 0; i < 5; i++){
        assert(new_arr[i] == i+1);
    }
    for(int i = 5; i < 10; i++){
        assert(new_arr[i] != i+1); 
    }

    return 0;
}

int main(){
    TestReallocBig();
    TestReallocSmall();
    std::cout << "\033[32mAll tests passed!\033[0m" << std::endl;
    return 0;
}