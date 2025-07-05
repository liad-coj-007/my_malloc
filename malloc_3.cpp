#include "malloc_3.h"
#include <cstring>
#include <cmath>
#include <cstdint>


const int heap_size = 4 * 1024 * 1024; // 4MB
const int max_alloc = pow(2, 17); // 128KB
MallocBuddyAllocator::MallocBuddyAllocator() : stats() {
     
    for(int i = 0; i < SIZE_FREE_LIST; i++) {
        free_lists[i] = NULL;
    }
    BuildHeap();    
}

size_t& MallocBuddyAllocator::getTotalFree() {
    return stats.total_free;
}

size_t& MallocBuddyAllocator::getTotalUsed() {
    return stats.total_used;
}

MetadataPtr MallocBuddyAllocator::FindFreeBlock(size_t size) {
    size_t order = log2(size+sizeof(MallocMetadata));
    for(size_t i = order; i < SIZE_FREE_LIST; i++){
        if(free_lists[i] == NULL) {
            continue; 
        }
        return free_lists[i];
    }

    return NULL;  
}

bool MallocBuddyAllocator::isBuddy(MetadataPtr a, MetadataPtr b) {
    if(a == NULL || b == NULL) {
        return false; 
    }
    if(a->size != b->size) {
        return false; 
    }
    uintptr_t a_addr = (uintptr_t)a;
    uintptr_t b_addr = (uintptr_t)b;
    return (a_addr ^ b_addr) == a->size; 
}

void MallocBuddyAllocator::Erase(MetadataPtr metadata){
    if(metadata == NULL) {
        return; 
    }
    MetadataPtr prev = metadata->prev;
    MetadataPtr next = metadata->next;
    metadata->next = NULL;
    metadata->prev = NULL;
    if(prev != NULL){
        prev->next = next; 
    }else{
        size_t idx = log2(metadata->size);
        free_lists[idx] = next; 
    }

    if(next != NULL) {
        next->prev = prev; 
    }
}

MetadataPtr MallocBuddyAllocator::FreeData(MetadataPtr metadata) {
    AddMeta(metadata);

    MetadataPtr buddy = NULL;
    if(isBuddy(metadata, metadata->next)) {
        buddy = metadata->next;
    } else if(isBuddy(metadata, metadata->prev)) {
        buddy = metadata->prev;
    }
    size_t idx = log2(metadata->size);
    if(buddy == NULL || idx == MAX_ORDER) {
        return metadata;
    }
    Erase(metadata);
    Erase(buddy);
    MetadataPtr new_metadata = metadata > buddy ? buddy: metadata;
    new_metadata->size *= 2; 
    return FreeData(new_metadata); 
}




MetadataPtr MallocBuddyAllocator::AlignHeap(MetadataPtr metadata,size_t size) {
    if(metadata->size/2 < size){
        return metadata;
    }
    metadata->size /= 2; 
    MetadataPtr buddy = (MetadataPtr)((char*)metadata + metadata->size);
    buddy->size = metadata->size;
    buddy->useage = 0;
    AddMeta(buddy);
    return AlignHeap(metadata, size);
}

void MallocBuddyAllocator::AddMeta(MetadataPtr metadata){
    size_t order = log2(metadata->size);
    metadata->next = NULL;
    metadata->prev = NULL;
    if(free_lists[order] == NULL) {
        free_lists[order] = metadata; 
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
    Erase(metadata);
    metadata = AlignHeap(metadata, size + sizeof(MallocMetadata));
    stats.total_free -= metadata->size;
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
        metadata->useage = 0;
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


void* BigAlloc(size_t size){
    void* ptr = mmap(NULL, size + sizeof(MallocMetadata), 
    PROT_READ | PROT_WRITE,
                     MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if(ptr == MAP_FAILED) {
        return NULL; // Allocation failed
    }
    MetadataPtr metadata = (MetadataPtr)ptr;
    metadata->size = size + sizeof(MallocMetadata);
    metadata->prev = NULL;
    metadata->next = NULL;
    metadata->useage = size;
    MallocBuddyAllocator& allocator = 
    MallocBuddyAllocator::getInstance();
    allocator.getTotalUsed() += size;
    return (void*)((char*)ptr + sizeof(MallocMetadata));
}

void* smalloc(size_t size){
    MallocBuddyAllocator& allocator = 
    MallocBuddyAllocator::getInstance();
    if(smallloc_check(size) == -1){
        return NULL; 
    }

    if(size +sizeof(MallocMetadata) > max_alloc){
        return BigAlloc(size);
    }
  
    MetadataPtr metadata = allocator.getFreeBlock(size);
    metadata->useage = size;
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





bool is_free(MallocMetadata* metadata) {
    return  metadata == NULL || metadata->useage == 0;
}

void FreeBigAlloc(void *ptr){
    MallocMetadata* metadata = (MallocMetadata*)ptr;
    MallocBuddyAllocator& allocator = 
    MallocBuddyAllocator::getInstance();
    allocator.getTotalUsed() -= metadata->useage;
    munmap(ptr, metadata->size);
}

void sfree(void* ptr){
    MetadataPtr metadata = 
    (MetadataPtr)((char*)ptr - sizeof(MallocMetadata));
    if(is_free(metadata)) {
        return; // Already freed
    }
    if(metadata->size > max_alloc){
        FreeBigAlloc(ptr);
        return ;
    }
    MallocBuddyAllocator& allocator 
    =  MallocBuddyAllocator::getInstance();
    MetadataPtr real_block =  allocator.FreeData(metadata);
    real_block->useage = 0;
    allocator.getTotalFree() += metadata->size;
}

void* realloc_update_bigAlloc(MallocMetadata* metadata, size_t size, void* oldp) {
    MallocBuddyAllocator& allocator =  MallocBuddyAllocator::getInstance();
    allocator.getTotalUsed() -= metadata->useage - size;
    metadata->useage = size;
    return oldp;
}

void* relloc_update_meta(MallocMetadata* metadata,size_t size,void* oldp){
    if(metadata->size > max_alloc){
        return realloc_update_bigAlloc(metadata, size, oldp);
    }
    MallocBuddyAllocator& allocator =  MallocBuddyAllocator::getInstance();
    MetadataPtr real_block = allocator.AlignHeap(metadata, size + sizeof(MallocMetadata));
    real_block->useage = size;
    allocator.getTotalFree() += metadata->size - real_block->size;
    return oldp;
}

MetadataPtr MallocBuddyAllocator::SmartReallocHelper(MetadataPtr metadata,size_t size){
    if(metadata->size >= size){
        metadata->useage = size - sizeof(MallocMetadata);
        return metadata;
    }
    AddMeta(metadata);
    size_t idx = log2(metadata->size);

    if(!isBuddy(metadata,metadata->next) || idx == MAX_ORDER) {
        return metadata;
    }
    MetadataPtr buddy = metadata->next;
    Erase(metadata);
    Erase(buddy);
    metadata->size *= 2;
    return SmartReallocHelper(metadata, size);
}


void* MallocBuddyAllocator::SmartRealloc(void* oldp, size_t size) {
    MetadataPtr metadata = 
    (MetadataPtr)((char*)oldp - sizeof(MallocMetadata));
    MetadataPtr real_block = SmartReallocHelper(metadata, size + sizeof(MallocMetadata));
    if(real_block->size >= size + sizeof(MallocMetadata)){
        return oldp;
    }
    AlignHeap(real_block, metadata->size);
    return NULL;
}

void* srealloc(void* oldp, size_t size){
    if(smallloc_check(size) == -1) {
        return NULL; 
    }

    if (oldp == NULL) {
        return smalloc(size);
    }

    MallocMetadata* metadata = (MallocMetadata*)((char*)oldp - sizeof(MallocMetadata));
    if (metadata->size >= size + sizeof(MallocMetadata)){
        return relloc_update_meta(metadata, size, oldp);
    }
    MallocBuddyAllocator& allocator = MallocBuddyAllocator::getInstance();
    void* ptr = allocator.SmartRealloc(oldp, size);
    if(ptr != NULL) {
        return ptr; 
    }
    ptr = smalloc(size);
    std::memmove(ptr, oldp, metadata->useage);
    return ptr;
}


size_t MallocBuddyAllocator::DoFunc(size_t (*f)(size_t, MallocMetadata*), MallocMetadata* metadata) {
    size_t result = 0;
    while(metadata != NULL) {
        result = f(result, metadata);
        metadata = metadata->next;
    }
    return result;
}

size_t MallocBuddyAllocator::DoFunc(size_t (*f)(size_t, MetadataPtr)){
    size_t result = 0;
    for(int i = 0; i < SIZE_FREE_LIST; i++) {
        result += DoFunc(f, free_lists[i]);
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
    return 0;
}

size_t _num_allocated_bytes(){
    return statistics.total_free + statistics.total_used;
}

size_t _num_meta_data_bytes(){
    return 0;
}

size_t _size_meta_data_bytes(){
    return sizeof(MallocMetadata);
}

