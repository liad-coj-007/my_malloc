#include <stdio.h>
#include "malloc_2.h"
using namespace std;
#include <assert.h>
#include <iostream>

int main(){
    int* ptr1 = (int*)scalloc(10,sizeof(int));
    int* ptr2 = (int*)scalloc(10,sizeof(int));
    int* ptr3 = (int*)scalloc(10,sizeof(int));
    if(ptr1 == NULL || ptr2 == NULL || ptr3 == NULL){
        return 1; 
    }
    sfree(ptr1);
    sfree(ptr2);
    sfree(ptr3);
    void* heap_ptr = sbrk(0);
    int* ptr4 = (int*)scalloc(30,sizeof(int));
    if(heap_ptr != sbrk(0)){
        cout << "Error: heap size changed after freeing memory." << endl;
        return 1;
    }

    std::cout << "\033[32mAll tests passed!\033[0m" << std::endl;
    return 0;
}