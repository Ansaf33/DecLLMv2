#include <stddef.h>  // For size_t
#include <stdint.h>  // For uintptr_t
#include <string.h>  // For memset (used in calloc)

// --- External Declarations ---
// Assuming allocate returns 0 on success, non-zero on failure.
// The allocated address is written to the `out_address` pointer.
extern int allocate(size_t size, uintptr_t *out_address);
extern void _terminate(void);
// extern void *memset(void *s, int c, size_t n); // Already in <string.h>

// --- Chunk Structure Definition ---
// This structure assumes a 32-bit system where pointers and size_t are 4 bytes.
// The header size is 12 bytes (3 * 4 bytes).
typedef struct chunk {
    struct chunk *next;
    struct chunk *prev;
    size_t size; // Size of the entire chunk including header
} chunk_t;

// --- Global Variables ---
// These act as sentinel nodes for the free and allocated lists.
// Their `next` and `prev` pointers are initialized to point to themselves,
// indicating an empty list. Their `size` is usually 0 for sentinel nodes.
static chunk_t freel = {&freel, &freel, 0};
static chunk_t heapl = {&heapl, &heapl, 0};

// Global variables for page management
static uintptr_t page = 0;
static size_t page_remaining = 0;

// A global pointer used as a one-time initialization flag.
// If it's NULL, the lists are initialized.
static chunk_t *g_init_flag_chunk = NULL;
// Another global pointer, possibly related to heap list initialization.
static chunk_t *g_heap_list_init_ptr = NULL;

// Constant for the chunk header size, derived from original code's 0xc (12 bytes).
// This assumes a 32-bit architecture where pointers and size_t are 4 bytes.
const size_t CHUNK_HEADER_SIZE = sizeof(chunk_t);

// Function: _addchunk
// Inserts 'new_chunk' immediately after 'list_node' in a doubly linked list.
void _addchunk(chunk_t *new_chunk, chunk_t *list_node) {
    new_chunk->next = list_node->next;
    new_chunk->prev = list_node;
    list_node->next->prev = new_chunk;
    list_node->next = new_chunk;
}

// Function: _rmchunk
// Removes 'chunk_to_remove' from its doubly linked list.
void _rmchunk(chunk_t *chunk_to_remove) {
    chunk_to_remove->next->prev = chunk_to_remove->prev;
    chunk_to_remove->prev->next = chunk_to_remove->next;
}

// Function: malloc
void *malloc(size_t __size) {
    // Handle zero-size allocation as per C standard.
    if (__size == 0) {
        return NULL;
    }

    // One-time initialization of global list heads if not already done.
    if (g_init_flag_chunk == NULL) {
        g_init_flag_chunk = &freel;
        g_heap_list_init_ptr = &heapl;
    }

    // Calculate the total required size, including the chunk header.
    const size_t required_size = __size + CHUNK_HEADER_SIZE;

    // Search for a suitable free chunk in the freelist.
    chunk_t *current_chunk = freel.next; // Start from the first actual free chunk
    while (current_chunk != &freel) { // Iterate until we loop back to the sentinel
        if (required_size <= current_chunk->size) {
            // Found a suitable chunk. Remove it from freelist and add to heaplist.
            _rmchunk(current_chunk);
            _addchunk(current_chunk, &heapl);
            // Return pointer to the user data area (after the header).
            return (void *)((uintptr_t)current_chunk + CHUNK_HEADER_SIZE);
        }
        current_chunk = current_chunk->next;
    }

    // If no suitable free chunk was found, try to use remaining page space.
    if (page != 0) {
        if (required_size <= page_remaining) {
            // Use space from the current page.
            chunk_t *new_chunk_from_page = (chunk_t *)page;
            new_chunk_from_page->size = required_size;
            page += required_size;
            page_remaining -= required_size;
            _addchunk(new_chunk_from_page, &heapl);
            return (void *)((uintptr_t)new_chunk_from_page + CHUNK_HEADER_SIZE);
        }

        // If remaining page space is too small for a chunk header, discard it.
        // Otherwise, add the remaining space as a free chunk.
        if (page_remaining < CHUNK_HEADER_SIZE) {
            page = 0;
        } else {
            chunk_t *remaining_page_chunk = (chunk_t *)page;
            remaining_page_chunk->size = page_remaining;
            _addchunk(remaining_page_chunk, &freel);
            page = 0;
        }
    }

    // No space found, request new memory from the system via `allocate`.
    uintptr_t allocated_address;
    if (allocate(required_size, &allocated_address) != 0) {
        // If allocation failed, terminate the program.
        _terminate();
    }

    // Calculate the `page` and `page_remaining` for the newly allocated block.
    // `page` points to the start of the unused portion of the block.
    // `page_remaining` is the size of that unused portion up to the next 4KB boundary.
    uintptr_t current_block_end = allocated_address + required_size;
    uintptr_t next_page_boundary = (current_block_end + 0xFFF) & ~0xFFF; // Round up to next 4KB page boundary
    page_remaining = next_page_boundary - current_block_end;
    page = current_block_end;

    // If the remaining part is too small to form a valid chunk, discard it.
    if (page_remaining < CHUNK_HEADER_SIZE) {
        page = 0;
    }

    // Initialize the newly allocated chunk and add it to the heaplist.
    chunk_t *newly_allocated_chunk = (chunk_t *)allocated_address;
    newly_allocated_chunk->size = required_size; // Store the total chunk size.
    _addchunk(newly_allocated_chunk, &heapl);

    return (void *)((uintptr_t)newly_allocated_chunk + CHUNK_HEADER_SIZE);
}

// Function: free
void free(void *__ptr) {
    if (__ptr == NULL) {
        return; // Freeing a NULL pointer is a no-op.
    }

    // Get the chunk header pointer from the user pointer.
    chunk_t *chunk_to_free = (chunk_t *)((uintptr_t)__ptr - CHUNK_HEADER_SIZE);

    // Remove the chunk from the heaplist and add it to the freelist.
    _rmchunk(chunk_to_free);
    _addchunk(chunk_to_free, &freel);
}

// Function: calloc
void *calloc(size_t __nmemb, size_t __size) {
    // Calculate total size, checking for potential overflow.
    size_t total_size;
    // Check if __nmemb is non-zero to prevent division by zero for overflow check
    // and to handle cases where __size is 0 but __nmemb is large (should result in 0 total_size).
    if (__nmemb == 0 || __size == 0) {
        total_size = 0;
    } else {
        total_size = __nmemb * __size;
        // Check for overflow: if total_size divided by __nmemb is not __size, overflow occurred.
        if (total_size / __nmemb != __size) {
            return NULL; // Overflow, return NULL.
        }
    }

    // Allocate memory using malloc.
    void *__s = malloc(total_size);

    // If allocation was successful, zero out the memory.
    if (__s != NULL) {
        memset(__s, 0, total_size);
    }
    return __s;
}