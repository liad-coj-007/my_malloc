#include "malloc_1.h"

void* smalloc(size_t size){
    size_t max_size = 100000000;
    if(size > max_size || size <= 0){
        return NULL; 
    }
    void* ptr = sbrk(size);
    if(ptr == (void*)-1){
        return NULL; 
    }
    return ptr;
}