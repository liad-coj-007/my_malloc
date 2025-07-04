#include <stdio.h>
#include "malloc_3.h"
using namespace std;
#include <assert.h>
#include <iostream>


int main(){
    int* arr = (int*)smalloc(10 * sizeof(int));
    MetadataPtr metadata = (MetadataPtr)((char*)arr - sizeof(MallocMetadata));
    cout << "size of metadata: " << sizeof(MallocMetadata) << endl;
    cout << metadata->size << endl;
    return 0;
}