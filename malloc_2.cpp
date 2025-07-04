#include "malloc_2.h"
#include <cstring>


MallocMetadata* heap_metadata = NULL;
Statistics statistics;

Statistics::Statistics() : total_free(0), total_used(0) {}

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
    if(heap_metadata != NULL) {
        heap_metadata->next = metadata;
    }
    heap_metadata = metadata;
    
    return (void*)((char*)ptr + sizeof(MallocMetadata));
}


void UpdateMetaData(MallocMetadata* metadata, size_t size) {
    metadata->size = size;
    metadata->is_free = false;
}

void AddMetaData(void* ptr,size_t size){
    MallocMetadata* metadata = (MallocMetadata*)ptr;
    metadata->is_free = false;
    if(metadata->next == NULL && metadata->size == size +sizeof(MallocMetadata)){
        //edge case
        UpdateMetaData(metadata, size);
        return;
    }

    MallocMetadata* next_metadata = 
    (MallocMetadata*)((char*)ptr + sizeof(MallocMetadata) + size);
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
    statistics.total_used += size;
    void* ptr;
    if(heap_metadata == NULL || (ptr = find_free_block(size+sizeof(MallocMetadata))) == NULL){
        return inc_program_break(size);
    }
    AddMetaData(ptr, size);
    statistics.total_free -= size;
    return (void*)((char*)ptr + sizeof(MallocMetadata));

}

void* scalloc(size_t num,size_t size){
    size_t total_size = num * size;
    void* ptr = smalloc(total_size);
    if(ptr == NULL){
        return NULL; 
    }

    std::memset(ptr, 0,total_size);


    return ptr;  
}



void merge_right(MallocMetadata*& metadata,MallocMetadata*& right){
    if(right == NULL || !right->is_free){
        return; 
    }

    if(metadata == NULL){
        metadata = right;
        return; 
    }

    metadata->size += right->size + sizeof(MallocMetadata);
    metadata->next = right->next;
    if(right->next != NULL) {
        right->next->prev = metadata;
    }

    if(right == heap_metadata) {
        heap_metadata = metadata;
    }
    
}

void merge(MallocMetadata*& metadata , MallocMetadata*& left,MallocMetadata*& right){
    merge_right(metadata,right);
    merge_right(left,metadata);
}



bool is_free(MallocMetadata* metadata) {
    return  metadata == NULL || metadata->is_free;
}

void sfree(void* ptr){
    if(ptr == NULL){
        return; 
    }
    MallocMetadata* metadata = (MallocMetadata*)((char*)ptr - sizeof(MallocMetadata));
    if(metadata->is_free) {
        return; // Memory is already freed
    }
    statistics.total_free += metadata->size;
    statistics.total_used -= metadata->size;
    metadata->is_free = true;
    MallocMetadata* right = metadata->next;
    MallocMetadata* left = metadata->prev;
    if(!is_free(left)){
        merge_right(metadata,right);
        return;
    }

    if(!is_free(right)){
        merge_right(left,metadata);
        return;
    }
    merge(metadata, left, right);
}



void* relloc_update_meta(MallocMetadata* metadata,size_t size,void* oldp){
    metadata->is_free = false;
    statistics.total_used -= metadata->size - size;
    statistics.total_free += metadata->size - size;
    AddMetaData(metadata, size);
    return oldp;
}


void* srealloc(void* oldp, size_t size){
    if(smallloc_check(size) == -1) {
        return NULL; 
    }

    if (oldp == NULL) {
        return smalloc(size);
    }

    MallocMetadata* metadata = (MallocMetadata*)((char*)oldp - sizeof(MallocMetadata));
    if (metadata->size >= size) {
        return relloc_update_meta(metadata, size, oldp);
    }

    void* ptr = smalloc(size); 
    std::memmove(ptr, oldp, metadata->size);
    return ptr;
}
/**
 * @brief This function iterates through 
 * the heap metadata and applies the provided function to each metadata block.
 * @param f A function that takes a size_t and a MallocMetadata pointer,
 * and return somthing
*/
size_t DoFunc(size_t (*f)(size_t, MallocMetadata*)){
    size_t result = 0;
    MallocMetadata* current = heap_metadata;
    while(current != NULL){
        result = f(result, current);
        current = current->prev;
    }
    return result;
}

size_t  _num_free_blocks(){
    return DoFunc([](size_t count, MallocMetadata* metadata) {
        if (metadata->is_free) {
            return count + 1;
        }
        return count;
    });
}

size_t _num_free_bytes(){
    return statistics.total_free;
}

size_t _num_allocated_blocks(){
    return DoFunc([](size_t count, MallocMetadata* metadata) {
        return count + 1;
    });
}

size_t _num_allocated_bytes(){
    return statistics.total_free + statistics.total_used;
}

size_t _num_meta_data_bytes(){
    return DoFunc([](size_t count, MallocMetadata* metadata) {
        return count + sizeof(MallocMetadata);
    });
}

size_t _size_meta_data_bytes(){
    return sizeof(MallocMetadata);
}

