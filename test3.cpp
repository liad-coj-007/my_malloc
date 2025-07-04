#include <stdio.h>
#include "malloc_3.h"
using namespace std;
#include <assert.h>
#include <iostream>
#include <cmath>


int main(){
    int* arr1 = (int*)smalloc(10 * sizeof(int));
    int* arr2 = (int*)srealloc(arr1, 5 * sizeof(int));
    MetadataPtr metadata = (MetadataPtr)((char*)arr1 - sizeof(MallocMetadata));

    assert(metadata->size == 64);
    cout << "success " << endl;
    return 0;
}