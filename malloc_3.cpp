#include "malloc_3.h"
#include <cstring>
#include <cmath>
#include <cstdint>


const int heap_size = 4 * 1024 * 1024; // 4MB
MallocBuddyAllocator::MallocBuddyAllocator() : stats() {
     
    for(int i = 0; i < SIZE_FREE_LIST; i++) {
        free_lists[i] = NULL;
    }
    BuildHeap();    
}

MetadataPtr MallocBuddyAllocator::FindFreeBlock(size_t size) {
    size_t order = log2(size+sizeof(MallocMetadata));
    for(size_t i = order; i < SIZE_FREE_LIST; i++){
        if(free_lists[i] == NULL) {
            continue; // No free block of this size
        }
        return free_lists[i];
    }

    return NULL; // No suitable free block found  
}
MetadataPtr MallocBuddyAllocator::AlignHeap(MetadataPtr metadata,size_t size) {
    if(metadata->size/2 < size){
        return metadata;
    }
    metadata->size /= 2; 
    MetadataPtr buddy = (MetadataPtr)((char*)metadata + metadata->size);
    AddMeta(buddy);
    return AlignHeap(metadata, size);
}

void MallocBuddyAllocator::Pop(MetadataPtr metadata) {
    if(metadata == NULL){
        return; 
    }
    size_t order = log2(metadata->size);
    free_lists[order] = metadata->next;
    MetadataPtr next = metadata->next;
    if(next != NULL) {
        next->prev = metadata->prev;
    }
    metadata->next = NULL;
}

void MallocBuddyAllocator::AddMeta(MetadataPtr metadata){
    size_t order = log2(metadata->size);
    metadata->next = NULL;
    metadata->prev = NULL;
    if(free_lists[order] == NULL) {
        free_lists[order] = metadata; // Add to the free list
        return;
    }

    MetadataPtr current = free_lists[order];
    MetadataPtr prev = NULL;
    while(current != NULL){
        prev = current;
        if(current > metadata){
            Push(current,metadata);
            return;
        }
        current = current->next;
    }

    prev->next = metadata;
    metadata->prev = prev;
}

MetadataPtr MallocBuddyAllocator::getFreeBlock(size_t size){
    MetadataPtr metadata = FindFreeBlock(size + sizeof(MallocMetadata));
    Pop(metadata);
    metadata = AlignHeap(metadata, size + sizeof(MallocMetadata));
    stats.total_free -= metadata->size;
    stats.total_used += metadata->size;
    return metadata;
}



void MallocBuddyAllocator::Push(MetadataPtr metadata,MetadataPtr newdata) {
   //newdata < metadata
   // low add -> high add
   MetadataPtr prev = metadata->prev;
   metadata->prev = newdata;
   newdata->next = metadata;
   newdata->prev = prev;
   if(prev != NULL){
       prev->next = newdata;
   }
}

void* getHeap(){
    void* current_brk = sbrk(0);
    uintptr_t aligned_brk = ((uintptr_t)current_brk + heap_size - 1) & ~(heap_size - 1);

    size_t padding = aligned_brk - (uintptr_t)current_brk;
    if (padding > 0) {
        void* tmp = sbrk(padding); 
        if (tmp == (void*)-1) {
            return NULL;
        }
    }

    void* heap_start = sbrk(heap_size); 
    if (heap_start == (void*)-1) {
        return NULL;
    }
    return heap_start;
}

void MallocBuddyAllocator::BuildHeap() {
    // Initialize the heap with a single large free block
    void* ptr = getHeap();
    if (ptr == NULL) {
        // Handle error
        return;
    }
    const int num_blocks = 32;
    const int size = heap_size / num_blocks;
    MetadataPtr current = nullptr;
    for(int i = 0; i < num_blocks;i++){
        MetadataPtr metadata = (MetadataPtr)ptr;
        metadata->size = size;
        metadata->next = NULL;
        metadata->prev = current;
        if(current != NULL) {
            current->next = metadata;
        }
        ptr = (void*)((char*)ptr  + size);
        current = metadata;
    }

    //for start in the lowest address
    for(int i = 0; i < num_blocks-1; i++){
        current = current->prev;
    }
    free_lists[MAX_ORDER] = current;
}

MallocBuddyAllocator& MallocBuddyAllocator::getInstance() {
    static MallocBuddyAllocator instance;
    return instance;
}



MallocMetadata* heap_metadata = NULL;
Statistics statistics;

Statistics::Statistics() : 
total_free(heap_size), total_used(heap_size) {}

int smallloc_check(size_t size){
    size_t max_size = 100000000;
    if(size > max_size || size <= 0){
        return -1; 
    }
    return 0; 
}

void UpdateMetaData(MallocMetadata* metadata, size_t size) {
    metadata->size = size;
}

void AddMetaData(void* ptr,size_t size){
    MallocMetadata* metadata = (MallocMetadata*)ptr;
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
    next_metadata->next = metadata->next;
    next_metadata->prev = metadata;
    metadata->next = next_metadata;
    metadata->size = size;
}



void* smalloc(size_t size){
    if(smallloc_check(size) == -1){
        return NULL; 
    }

    MallocBuddyAllocator& allocator = 
    MallocBuddyAllocator::getInstance();
    MetadataPtr metadata = allocator.getFreeBlock(size);
    return (void*)((char*)metadata + sizeof(MallocMetadata));
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
    if(right == NULL ){
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
    return  metadata == NULL;
}

void sfree(void* ptr){
    if(ptr == NULL){
        return; 
    }
    MallocMetadata* metadata = (MallocMetadata*)((char*)ptr - sizeof(MallocMetadata));

    statistics.total_free += metadata->size;
    statistics.total_used -= metadata->size;
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
    return 0;
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

