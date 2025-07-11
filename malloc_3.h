#include <unistd.h> 
#include <sys/mman.h>  // For mmap(), munmap()


struct MallocMetadata {
    size_t useage;
    size_t size;
    MallocMetadata* next;
    MallocMetadata* prev;
};

 
typedef MallocMetadata* MetadataPtr;
// maximum order of the buddy system
static const int MAX_ORDER = 17;
static const int SIZE_FREE_LIST = MAX_ORDER+1;

/**
 * @brief A structure to hold statistics about memory allocation.
 * It contains the total amount of free memory.
 * in bytes.
 */
struct Statistics{
    
    size_t free_bytes;
    size_t alloc_bytes;
    size_t alloc_blocks;
    size_t free_blocks;
    size_t num_metadata;

    /**
     * @brief Default constructor for the Statistics structure.
     * put zero on every field.
    */
    Statistics();

};


/**
 * @brief A class that implements a buddy memory allocator.
 * It uses a binary tree structure to manage free blocks of memory.
 * The class is a singleton, 
 * meaning only one instance can exist at a time.
 */
class MallocBuddyAllocator{
private:
    Statistics stats;
    MetadataPtr free_lists[SIZE_FREE_LIST];
    /***
     * @brief initlize the allocator
    */
    MallocBuddyAllocator();

    /**
     * @brief building the heap
    */
    void BuildHeap();

    /**
     * @brief find a free block of memory
    */
    MetadataPtr FindFreeBlock(size_t size);


    /**
     * @brief Push a metadata pointer to the free list.
     * @param metadata The metadata pointer to push.
     * @param newdata The new data to be added to the metadata.
     * metadata < newdata
    */
    void Push(MetadataPtr metadata,MetadataPtr newdata);
    /**
     * @brief Add metadata to the allocator's metadata list.
     * @param metadata The metadata pointer to add.
    */
    void AddMeta(MetadataPtr metadata);
    /**
     * @brief return true if the datas is buddies else
     * false
     * @param a The first metadata pointer.
     * @param b The second metadata pointer.
     * @return true if the two metadata pointers are buddies
     * , false otherwise.
    */
    bool isBuddy(MetadataPtr a, MetadataPtr b);
    /**
     * @brief erase the metadata from the free list.
    */
    void Erase(MetadataPtr metadata);

    /**
     * @brief connect the buddies in the high order
     * that means metadata < buddy so the the metadata is in the left
    */
    MetadataPtr SmartReallocHelper(MetadataPtr metadata,size_t size);

    /**
     * @brief This function iterates through 
     * the heap metadata and applies the provided function to each metadata block.
     * @param f A function that takes a size_t and a MallocMetadata pointer,
     * and return somthing
    */
    size_t DoFunc(size_t (*f)(size_t, MetadataPtr),MetadataPtr metadata);

public:
    /**
     * @brief build the allocator
    */
    static MallocBuddyAllocator& getInstance();

    /**
     * @brief get a free block of memory.
     * and align the mem to use the exact size.
     * @param size The size of the block to allocate.
    */
    MetadataPtr getFreeBlock(size_t size);
    /**
     * @brief free the allocation
    */
    MetadataPtr FreeData(MetadataPtr metadata);
    /**
     * @brief get the total free memory in bytes.
    */
    size_t& getFreeBytes();

    /**
     * @brief get the total free memory in blocks.
    */
    size_t& getFreeBlocks();
    /**
     * @brief get the total used memory in bytes.
    */
    size_t& getAllocBytes();
    /**
     * @brief get the total alloc memory in blocks.
    */
    size_t& getAllockBlocks();

    /**
     * @brief get the total used memory in blocks.
    */
    size_t& getMetaDataBytes();

    /**
     * @brief get the minimal metadata we need
     * @param metadata The metadata pointer to align.
     * @param size - the size of the block to align.
    */
    MetadataPtr AlignHeap(MetadataPtr metadata,size_t size);

    /**
     * @brief do smart realloc if we can allocate the oldp blocks
     * else we do smalloc
    */
    void* SmartRealloc(void* oldp, size_t size);

    /**
     * @brief This function iterates through
     * the free heap metadata 
     * and applies the provided function to each metadata block.
    */
    size_t DoFunc(size_t (*f)(size_t, MetadataPtr));
    

};



/**
 * @brief check the input size for allocation.
 * @param size The number of bytes to allocate.
 * @return 0 if the size is valid, -1 if it is invalid.
*/
int smallloc_check(size_t size);


/**
 * @brief Allocates memory of the specified size.
 * @param size The number of bytes to allocate.
 * @return A pointer to the allocated memory, 
 * or NULL if the allocation fails.
*/
void* smalloc(size_t size);

/**
 * @brief Allocates memory for an array of elements of a specified size.
 * initializes the allocated memory to zero.
 * @param num The number of elements to allocate.
 * @param size The size of each element in bytes.
*/
void* scalloc(size_t num,size_t size);

/**
 * @brief free the pointer
 * @param ptr The pointer to the memory to be freed.
*/
void sfree(void* ptr);

/**
 * @brief Reallocates memory for a pointer to a new size.
 * @param oldp The pointer to the memory to be reallocated.
*/
void* srealloc(void* oldp, size_t size);

/**
 * @brief return the number of free blocks
*/
size_t _num_free_blocks();

/**
 * @brief return the number of free bytes
*/
size_t _num_free_bytes();

/**
 * @brief return the number of allocated blocks
*/
size_t _num_allocated_blocks();
/**
 * @brief return the number of allocated bytes
*/
size_t _num_allocated_bytes();
/**
 * @brief returns the overall number of meta data bytes
*/
size_t _num_meta_data_bytes();

/**
 * @brief returns the size of single meta data
*/
size_t _size_meta_data();