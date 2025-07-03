#include <stdio.h>
#include "malloc_1.h"
using namespace std;
#include <assert.h>
#include <iostream>


int FirstTest(){
    int* ptr = (int*)smalloc(10 * sizeof(int));
    if (ptr == NULL) {
        return 0;
    }
    return 1;
}

int ErrorTest(){
    int* ptr = (int*)smalloc(0);
    if (ptr != NULL) {
        return 0; // Error: should return NULL for zero allocation
    }
    return 1;
}

int TwoArray(){
    int* ptr1 = (int*)smalloc(10 * sizeof(int));
    int* ptr2 = (int*)smalloc(20 * sizeof(int));
    
    if (ptr1 == NULL || ptr2 == NULL) {
        return 0; 
    }

    ptr1[9] = 0;
    ptr1[0] = 0;
    ptr1[9] = 300;
    if(ptr1[0] != 0 || ptr1[9] != 300){
        return 0; 
    }
    
    
    return 1;
}


int main(){
 
    assert(FirstTest() == 1);
    assert(ErrorTest() == 1);
    assert(TwoArray() == 1); 
    std::cout << "\033[32mAll tests passed!\033[0m" << std::endl;
    
    return 0;
}