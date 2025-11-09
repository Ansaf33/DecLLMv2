#include <stdlib.h> // for malloc, free, realloc
#include <string.h> // for memset, strlen, strcmp, strdup
#include <stdint.h> // for uint32_t, uintptr_t, etc.
#include <stdbool.h> // for bool, true, false

// Use standard C integer types for clarity and portability
typedef unsigned int uint;

// Decompiled constant, likely a load factor.
// Using 0.75 as a common default.
static const double HTBL_LOAD_FACTOR = 0.75;

// The hash table structure is implicitly an array of uints/uintptr_t.
// For clarity, we'll use a `uint *` pointer to access it.
// Indices and their intended types:
// htbl_ptr[0]: capacity (uint)
// htbl_ptr[1]: count (uint)
// htbl_ptr[2]: pointer to array of entry pointers (uintptr_t)
// htbl_ptr[3]: pointer to head of linked list of entries (uintptr_t)
// htbl_ptr[4]: pointer to tail of linked list of entries (uintptr_t)
// htbl_ptr[5]: pointer to free_value_func (uintptr_t)
//
// An entry itself is `char**` and contains:
// entry[0]: char* key (allocated with strdup)
// entry[1]: char* value (user-supplied, or allocated by user)
// entry[2]: char* next_entry_ptr (pointer to the next `char**` entry in the global linked list)

// Function: htbl_create
// param_1: initial_capacity
// param_2: free_value_func (function pointer to free values, or NULL if not needed)
uint * htbl_create(int initial_capacity, void (*free_value_func)(void *)) {
    // Allocate space for the main hash table structure (6 fields)
    // Using uintptr_t for all fields to ensure sufficient size for pointers on any architecture.
    uintptr_t *htbl_ptr_base = (uintptr_t *)malloc(6 * sizeof(uintptr_t));
    if (htbl_ptr_base == NULL) {
        return NULL;
    }

    memset(htbl_ptr_base, 0, 6 * sizeof(uintptr_t));

    // Store the free_value_func_ptr
    htbl_ptr_base[5] = (uintptr_t)free_value_func;

    // Set initial capacity (stored at index 0 as uint)
    if (initial_capacity < 1) {
        initial_capacity = 0x10; // Default capacity
    }
    ((uint*)htbl_ptr_base)[0] = (uint)initial_capacity; // Cast back to uint* to access as uint

    // Allocate space for the array of entry pointers (void**)
    void **entries_array = (void **)malloc(((uint*)htbl_ptr_base)[0] * sizeof(void *));
    if (entries_array == NULL) {
        free(htbl_ptr_base);
        return NULL;
    }

    // Store the pointer to the entries array
    htbl_ptr_base[2] = (uintptr_t)entries_array;
    memset(entries_array, 0, ((uint*)htbl_ptr_base)[0] * sizeof(void *)); // Initialize entry pointers to NULL

    return (uint *)htbl_ptr_base; // Return as uint* to match other functions' param_1 type
}

// Function: _htbl_hash
uint _htbl_hash(uint *htbl_ptr, char *key) {
    uint hash = 0x9e370001; // Initial seed for hash
    size_t key_len = strlen(key);

    for (size_t i = 0; i < key_len; i++) {
        hash = (hash + (uint)key[i]) * 0x20; // Original hash calculation
    }
    
    // The original final addition (0x9e370000 < local_14) * 0x61c8ffff is removed
    // as it's highly unusual and potentially a decompilation artifact.
    // Simplifying to just modulo the capacity.
    return hash % htbl_ptr[0]; // htbl_ptr[0] is capacity
}

// Function: _htbl_double_size
int _htbl_double_size(uint *htbl_ptr) {
    // Cast htbl_ptr to uintptr_t* for direct pointer field access
    uintptr_t *htbl_ptr_base = (uintptr_t *)htbl_ptr;

    // Max capacity check
    if (htbl_ptr[0] >= 0x20000000) {
        return -1; // Error: max capacity reached
    }

    uint current_capacity = htbl_ptr[0];
    uint new_capacity = current_capacity * 2;
    size_t new_array_byte_size = new_capacity * sizeof(void *);

    // Reallocate the array of entry pointers
    void **entries_array = (void **)htbl_ptr_base[2];
    void *new_entries_array = realloc(entries_array, new_array_byte_size);

    if (new_entries_array == NULL) {
        return -1; // Reallocation failed
    }

    // Zero out the newly allocated portion of the array
    memset((char *)new_entries_array + current_capacity * sizeof(void *), 0,
           current_capacity * sizeof(void *));

    // Update the hash table structure with the new array pointer and capacity
    htbl_ptr_base[2] = (uintptr_t)new_entries_array;
    htbl_ptr[0] = new_capacity;

    return 0; // Success
}

// Function: htbl_put
int htbl_put(uint *htbl_ptr, char *key, char *value) {
    if (htbl_ptr == NULL || key == NULL || value == NULL) {
        return -1; // Invalid arguments
    }

    // Cast htbl_ptr to uintptr_t* for direct pointer field access
    uintptr_t *htbl_ptr_base = (uintptr_t *)htbl_ptr;

    // Check load factor and resize if necessary
    if (HTBL_LOAD_FACTOR * htbl_ptr[0] <= htbl_ptr[1]) {
        if (_htbl_double_size(htbl_ptr) != 0) {
            return -1; // Resize failed
        }
    }

    uint current_capacity = htbl_ptr[0];
    uint hash_index = _htbl_hash(htbl_ptr, key);
    
    // Access the array of entry pointers
    void ***entries_array = (void ***)htbl_ptr_base[2];

    // Linear probing loop
    while (true) {
        char **entry = (char **)entries_array[hash_index];

        if (entry != NULL) {
            // Entry exists, check for key match
            if (entry[0] != NULL && strcmp(entry[0], key) == 0) {
                // Key found, update value
                void (*free_value_func)(void *) = (void (*)(void *))htbl_ptr_base[5];
                if (free_value_func != NULL && entry[1] != NULL) {
                    free_value_func(entry[1]); // Free old value
                }
                entry[1] = value; // Assign new value
                return 0; // Success
            }
        } else {
            // Found an empty slot, can insert here
            break;
        }

        // Move to the next slot (linear probing with wrap-around)
        hash_index = (hash_index + 1) % current_capacity;
    }

    // Key not found, create a new entry
    // An entry is `char* key, char* value, char* next_entry_ptr`
    char **new_entry = (char **)malloc(3 * sizeof(char *));
    if (new_entry == NULL) {
        return -1; // Malloc failed
    }

    char *duplicated_key = strdup(key);
    if (duplicated_key == NULL) {
        free(new_entry);
        return -1; // strdup failed
    }

    new_entry[0] = duplicated_key; // Key
    new_entry[1] = value;         // Value
    new_entry[2] = NULL;          // Next pointer for global linked list (initially NULL)

    // Add to the separate global linked list of all entries (for destruction)
    if (htbl_ptr_base[4] == (uintptr_t)NULL) { // If tail is NULL, list is empty
        htbl_ptr_base[3] = (uintptr_t)new_entry; // Head points to new entry
        htbl_ptr_base[4] = (uintptr_t)new_entry; // Tail points to new entry
    } else {
        // Append to tail
        char **tail_entry = (char **)htbl_ptr_base[4];
        tail_entry[2] = (char *)new_entry; // Old tail's next points to new entry
        htbl_ptr_base[4] = (uintptr_t)new_entry; // Tail becomes new entry
    }

    // Store the new entry in the hash table array at the calculated index
    entries_array[hash_index] = (void *)new_entry;
    htbl_ptr[1]++; // Increment count (htbl_ptr[1] is count)
    return 0; // Success
}

// Function: htbl_get
char * htbl_get(uint *htbl_ptr, char *key) {
    if (htbl_ptr == NULL || key == NULL || htbl_ptr[1] == 0) {
        return NULL; // Invalid arguments or empty table
    }

    // Cast htbl_ptr to uintptr_t* for direct pointer field access
    uintptr_t *htbl_ptr_base = (uintptr_t *)htbl_ptr;

    uint current_capacity = htbl_ptr[0];
    uint hash_index = _htbl_hash(htbl_ptr, key);
    uint probes = 0; // Counter to prevent infinite loops in case of a full table / bug

    void ***entries_array = (void ***)htbl_ptr_base[2];

    // Linear probing loop
    while (probes < current_capacity) { // Probe at most `capacity` times
        char **entry = (char **)entries_array[hash_index];

        if (entry != NULL) {
            // Entry exists, check for key match
            if (entry[0] != NULL && strcmp(entry[0], key) == 0) {
                return entry[1]; // Key found, return value
            }
        } else {
            // Found an empty slot, key is not in table
            return NULL;
        }

        // Move to the next slot (linear probing with wrap-around)
        hash_index = (hash_index + 1) % current_capacity;
        probes++;
    }

    return NULL; // Key not found after probing `capacity` times
}

// Function: htbl_destroy
void htbl_destroy(uint *htbl_ptr) {
    if (htbl_ptr == NULL) {
        return;
    }

    // Cast htbl_ptr to uintptr_t* for direct pointer field access
    uintptr_t *htbl_ptr_base = (uintptr_t *)htbl_ptr;

    // Get the free_value_func and the head of the linked list
    void (*free_value_func)(void *) = (void (*)(void *))htbl_ptr_base[5];
    char **current_entry = (char **)htbl_ptr_base[3]; // Get head of global linked list

    // Traverse the global linked list of all entries and free them
    while (current_entry != NULL) {
        char **next_entry = (char **)current_entry[2]; // Get next entry before freeing current

        if (current_entry[0] != NULL) {
            free(current_entry[0]); // Free key string (allocated by strdup)
        }
        if (free_value_func != NULL && current_entry[1] != NULL) {
            free_value_func(current_entry[1]); // Free value using provided function
        }
        free(current_entry); // Free the entry structure itself (allocated by malloc(3*sizeof(char*)))

        current_entry = next_entry; // Move to the next entry
    }

    // Free the hash table's internal array of entry pointers
    void **entries_array = (void **)htbl_ptr_base[2];
    if (entries_array != NULL) {
        free(entries_array);
        htbl_ptr_base[2] = (uintptr_t)NULL; // Clear the pointer
    }

    // Finally, free the hash table structure itself
    free(htbl_ptr_base); // Free the base pointer allocated in htbl_create
}