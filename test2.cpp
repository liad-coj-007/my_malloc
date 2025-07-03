#include <stdio.h>
#include "malloc_2.h"
using namespace std;
#include <assert.h>
#include <iostream>

int main(){
    int* ptr = (int*)scalloc(10,sizeof(int));
    if(ptr [9] == 0){
        std::cout << "\033[32mAll tests passed!\033[0m" << std::endl;       
    }else{
        std::cout << "\033[31mTest failed!\033[0m" << std::endl;
        return 1; // Test failed
    }

    return 0;
}