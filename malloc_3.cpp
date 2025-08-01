#include "malloc_3.h"
#include <cstring>
#include <cmath>
#include <cstdint>
#include <cassert>
#include <iomanip>
#include <assert.h>

/**
 * @brief a global variable that holds the
 *  heap - buddy size. 
*/
const int heap_size = 4 * 1024 * 1024; // 4MB
const int max_alloc = pow(2, 17); // 128KB
const int num_blocks = 32;

MallocBuddyAllocator::MallocBuddyAllocator() : stats() {
     
    for(int i = 0; i < SIZE_FREE_LIST; i++) {
        free_lists[i] = NULL;
    }
    BuildHeap();    
}

size_t& MallocBuddyAllocator::getFreeBlocks(){
    return stats.free_blocks;
}

size_t& MallocBuddyAllocator::getFreeBytes(){
    return stats.free_bytes;
}

size_t& MallocBuddyAllocator::getAllockBlocks(){
    return stats.alloc_blocks;
}

size_t& MallocBuddyAllocator::getAllocBytes(){
    return stats.alloc_bytes;
}

size_t& MallocBuddyAllocator::getMetaDataBytes(){
    return stats.num_metadata;
}

MetadataPtr MallocBuddyAllocator::FindInList(MetadataPtr list){
    MetadataPtr head = list;
    while(head != nullptr){
        if(head->useage == 0){
            return head;
        }
        head = head->next;
    }
    return NULL;
}

size_t my_log(size_t size){
    double log_value = log(size) / log(2);  
    
    if (log_value != floor(log_value)) {
        return (size_t)ceil(log_value);
    }
    
    return (size_t)log_value;
}

MetadataPtr MallocBuddyAllocator::FindFreeBlock(size_t size) {
    size_t order = my_log(size);
    for(size_t i = order; i < SIZE_FREE_LIST; i++){
        MetadataPtr metadata = FindInList(free_lists[i]);
        if(metadata != nullptr) {
            return metadata; 
        }
    }

    return NULL;  
}

bool MallocBuddyAllocator::isBuddy(MetadataPtr a, MetadataPtr b) {
  

    if(a == NULL || b == NULL) {
        return false; 
    }


    if(a->useage != 0 || b->useage != 0){
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
    buddy->next = nullptr;
    buddy->prev = nullptr;
    FreeData(buddy);
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
    if(metadata == NULL){
        return metadata;
    }
    Erase(metadata);
    metadata = AlignHeap(metadata, size + sizeof(MallocMetadata));
    AddMeta(metadata);
    return metadata;
}



void MallocBuddyAllocator::Push(MetadataPtr metadata,MetadataPtr newdata) {
   //newdata < metadata
   // low add -> high add
   MetadataPtr prev = metadata->prev;
   if(prev != NULL){
        prev->next = newdata;
        newdata->prev = prev;
   }else{
      size_t order = log2(metadata->size);
      free_lists[order] = newdata;
      newdata->prev = NULL;
   }
   newdata->next = metadata;
   metadata->prev = newdata;
   
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

    MetadataPtr current = nullptr;
    for(int i = 0; i < num_blocks;i++){
        MetadataPtr metadata = (MetadataPtr)ptr;
        metadata->size = max_alloc;
        metadata->next = NULL;
        metadata->prev = current;
        metadata->useage = 0;
        if(current != NULL) {
            current->next = metadata;
        }
        ptr = (void*)((char*)ptr  + max_alloc);
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




Statistics::Statistics(){
    free_bytes =  0;
    alloc_bytes = 0;
    alloc_blocks = 0;
    free_blocks = 0;
    num_metadata = 0;
}

int smallloc_check(size_t size){
    size_t max_size = 100000000;
    if(size <= 0 || size > max_size){
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
    allocator.getAllocBytes() += metadata->size;
    allocator.getAllockBlocks() += 1;
    allocator.getMetaDataBytes() += sizeof(MallocMetadata);
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
    if(metadata == NULL){
        return NULL;
    }
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
    MetadataPtr metadata = (MetadataPtr)ptr;
    MallocBuddyAllocator& allocator = 
    MallocBuddyAllocator::getInstance();
    allocator.getAllocBytes() -= metadata->useage + sizeof(MallocMetadata);
    allocator.getAllockBlocks() -= 1;
    allocator.getMetaDataBytes() -= sizeof(MallocMetadata);
    munmap(ptr, metadata->size);
    ptr = NULL;
}

void sfree(void* ptr){
    if(ptr == nullptr){
        return;
    }

    MetadataPtr metadata = 
    (MetadataPtr)((char*)ptr - sizeof(MallocMetadata));
    if(is_free(metadata)) {
        return; // Already freed
    }
    if(metadata->size > max_alloc){
        FreeBigAlloc(metadata);
        return ;
    }
    MallocBuddyAllocator& allocator 
    =  MallocBuddyAllocator::getInstance();
    allocator.Erase(metadata);
    metadata->useage = 0;
    MetadataPtr real_block =  allocator.FreeData(metadata);
    real_block->useage = 0;

}

void* realloc_update_bigAlloc(MallocMetadata* metadata, size_t size, void* oldp) {
    MallocBuddyAllocator& allocator =  MallocBuddyAllocator::getInstance();
    allocator.getAllocBytes() -= metadata->useage - size;
    metadata->useage = size;
    return oldp;
}

void* relloc_update_meta(MallocMetadata* metadata,size_t size,void* oldp){
    if(metadata->size > max_alloc){
        return realloc_update_bigAlloc(metadata, size, oldp);
    }
    metadata->useage = size; 
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
    size_t meta_usage = metadata->useage;
    metadata->useage = 0;
    Erase(metadata);
    MetadataPtr real_block = SmartReallocHelper(metadata, size + sizeof(MallocMetadata));
    if(real_block->size >= size + sizeof(MallocMetadata)){
        AddMeta(real_block);
        return oldp;
    }
    Erase(real_block);
    AlignHeap(real_block, metadata->size);
    AddMeta(real_block);

    real_block->useage = meta_usage;
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
    void* ptr = NULL;
    if(size + sizeof(MallocMetadata) <= max_alloc){
        MallocBuddyAllocator& allocator = MallocBuddyAllocator::getInstance();
        ptr = allocator.SmartRealloc(oldp, size);
    }
    
    if(ptr != NULL) {
        return ptr; 
    }
    ptr = smalloc(size);
    if(ptr == NULL){
        return NULL;
    }
    std::memmove(ptr, oldp, metadata->useage);
    sfree(oldp);
    return ptr;
}


size_t MallocBuddyAllocator::DoFunc(size_t (*f)(size_t, MetadataPtr), 
MetadataPtr metadata) {
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


size_t _num_free_blocks(){
    auto func = [](size_t count , MetadataPtr metadata){
        return count + (metadata->useage == 0);
    };
    MallocBuddyAllocator& allocator = MallocBuddyAllocator::getInstance();
    return allocator.DoFunc(func)+allocator.getFreeBlocks();
}

size_t _num_free_bytes(){
    auto func = [](size_t count , MetadataPtr metadata){
        if(is_free(metadata) == false){
            return count;
        }
        return count + metadata->size;
    };
    MallocBuddyAllocator& allocator = MallocBuddyAllocator::getInstance();
    return allocator.DoFunc(func)+allocator.getFreeBytes();
}

size_t _num_allocated_blocks(){
    auto func = [](size_t count , MetadataPtr metadata){
        return count+1;
    };
    MallocBuddyAllocator& allocator = MallocBuddyAllocator::getInstance();
    return allocator.DoFunc(func) + allocator.getAllockBlocks();
}

size_t _num_allocated_bytes(){
    auto func = [](size_t count , MetadataPtr metadata){
        return count +metadata->size;
    };
    MallocBuddyAllocator& allocator = MallocBuddyAllocator::getInstance();
    return allocator.DoFunc(func)+ allocator.getAllocBytes();
}

size_t _num_meta_data_bytes(){
    auto func = [](size_t count , MetadataPtr metadata){
        return count + sizeof(MallocMetadata);
    };
    MallocBuddyAllocator& allocator = MallocBuddyAllocator::getInstance();
    return allocator.DoFunc(func)+allocator.getMetaDataBytes();
}

size_t _size_meta_data(){
    return sizeof(MallocMetadata);
}


bool MallocBuddyAllocator::is_on_list(MetadataPtr metadata){
    for(int i = 0; i < SIZE_FREE_LIST;i++){
        MetadataPtr metadata2 = free_lists[i];
        while(metadata2 != nullptr){
            // cout << metadata2 << endl;
            if(metadata2 == metadata ){
                return true;
            }
            metadata2 = metadata2->next;
        }
    }
    return false;
}
