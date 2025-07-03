#include <stdio.h>
#include "malloc_2.h"
using namespace std;
#include <assert.h>
#include <iostream>

int main(){
    int* ptr = (int*)smalloc(10 * sizeof(int));
    int* ptr2 = (int*)smalloc(10 * sizeof(int));
    ptr[9] = 1;
    ptr2[0] = 3;
    if(ptr[9] == 1 && ptr2[0] == 3){
        std::cout << "\033[32mAll tests passed!\033[0m" << std::endl;
    } else {
        std::cout << "\033[31mTest failed!\033[0m" << std::endl;
    }
    return 0;
}