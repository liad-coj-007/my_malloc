#include <unistd.h> 


struct MallocMetadata {
    size_t size;
    bool is_free;
    MallocMetadata* next;
    MallocMetadata* prev;
};

/**
 * @brief A structure to hold statistics about memory allocation.
 * It contains the total amount of free memory.
 * in bytes.
 */
struct Statistics{
    
    size_t total_free;
    size_t total_used;

    /**
     * @brief Default constructor for the Statistics structure.
     * put zero on every field.
    */
    Statistics();

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