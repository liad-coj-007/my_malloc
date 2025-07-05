#include <stdio.h>
#include "malloc_3.h"
using namespace std;
#include <assert.h>
#include <iostream>
#include <cmath>


void test_sizeof(){
    cout << "sizeof(MallocMetadata): " << sizeof(MallocMetadata) << "B" << endl;
}

int TestRealloc(){
    int* arr1 = (int*)smalloc(10 * sizeof(int));
    int* arr2 = (int*)srealloc(arr1, 20 * sizeof(int));
    assert(arr1 == arr2);
}

int TestNear(){
    int* arr = (int*)smalloc(10 * sizeof(int));
    int* arr1 = (int*)srealloc(arr,5 * sizeof(int));
    int* arr2 = (int*) smalloc(5*sizeof(int));

    MetadataPtr metadata1 = (MetadataPtr)((char*)arr1 - sizeof(MallocMetadata));
    MetadataPtr metadata2 = (MetadataPtr)((char*)arr2 - sizeof(MallocMetadata));
    cout << "meta1: " << arr1 << endl <<"size: " << metadata1->size << endl;
    cout << "meta2: " << arr2 << endl << "size: " << metadata2->size << endl;
    assert((char*)metadata1 + metadata1->size == (char*)metadata2);
    
}

int main(){
    test_sizeof();
    TestRealloc();
    TestNear();
    std::cout << "\033[1;32mPASS\033[0m" << std::endl;
    return 0;
}