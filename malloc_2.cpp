#include "malloc_2.h"

MallocMetadata* heap_metadata = NULL;

int smallloc_check(size_t size){
    size_t max_size = 100000000;
    if(size > max_size || size <= 0){
        return -1; 
    }
    return 0; 
}

size_t Min(size_t a,size_t b){
    if(a == 0){
        return b;
    }

    size_t min = a < b ? a : b;
    return min;
}

void* find_free_block(size_t size) {
    // This function would typically search through a linked list of free blocks
    // and return a pointer to a suitable block if found.
    // For now, we will return NULL to indicate no free block found.
    MallocMetadata* current = heap_metadata;
    void* ptr = NULL;
    size_t min_size = 0;
    while(current != NULL){
        if(current->is_free && current->size >= size 
        && Min(min_size, current->size) == current->size){
            ptr = (void*) current;
            min_size = current->size;
        }
        current = current->prev;
    }

    return ptr;

}

void* inc_program_break(size_t size){
   
    void* ptr = sbrk(size+sizeof(MallocMetadata));
    if(ptr == (void*)-1){
        return NULL; 
    }

    MallocMetadata* metadata = (MallocMetadata*)ptr;
    metadata->size = size;
    metadata->is_free = false;
    metadata->next = NULL;
    metadata->prev = heap_metadata;
    heap_metadata = metadata;
    return (void*)((char*)ptr + sizeof(MallocMetadata));
}

void AddMetaData(void* ptr,size_t size){
    MallocMetadata* metadata = (MallocMetadata*)ptr;
    metadata->is_free = false;
    MallocMetadata* next_metadata = (MallocMetadata*)((char*)ptr + sizeof(MallocMetadata) + size);
    if(next_metadata == metadata->next){
        return;
    }
    next_metadata->size = metadata->size - size - sizeof(MallocMetadata);
    next_metadata->is_free = true;
    next_metadata->next = metadata->next;
    next_metadata->prev = metadata;
    metadata->next = next_metadata;
    metadata->size = size;
}

void* smalloc(size_t size){
    if(smallloc_check(size) == -1){
        return NULL; 
    }

    void* ptr;
    if(heap_metadata == NULL || (ptr = find_free_block(size+sizeof(MallocMetadata))) == NULL){
        return inc_program_break(size);
    }
    AddMetaData(ptr, size);
    return (void*)((char*)ptr + sizeof(MallocMetadata));

}

