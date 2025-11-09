#include <stdlib.h> // For malloc, free
#include <string.h> // For memset, strncpy, strncmp
#include <stdint.h> // For uint32_t, uintptr_t

// Define uint as uint32_t for consistency with common usage in such snippets
typedef uint32_t uint;

// Custom allocate function matching the original snippet's signature and behavior.
// Returns 0 on success, -1 (which is 0xffffffff for int32_t) on failure.
static int32_t allocate(size_t size, int unknown_param, uintptr_t *out_ptr_uint, int debug_val) {
    void *ptr = malloc(size);
    if (ptr == NULL) {
        *out_ptr_uint = 0; // Ensure the pointer is null on failure
        return -1; // Return -1 for failure (0xffffffff as int32_t)
    }
    memset(ptr, 0, size); // Initialize allocated memory to zero
    *out_ptr_uint = (uintptr_t)ptr;
    return 0; // Return 0 for success
}

// Custom deallocate function matching the original snippet's signature.
static void deallocate(uintptr_t ptr_uint, size_t size) { // size is ignored by free
    free((void *)ptr_uint);
}

// Function: namespace_init
int32_t namespace_init(uint *param_1, uint param_2) {
  // param_1 is expected to point to an array of uints.
  // param_1[0] will store the capacity (param_2).
  // param_1[1] will store the allocated data pointer (as uintptr_t).
  *param_1 = param_2; // Store capacity
  
  // The original conditional `if (param_2 == (param_2 * 0xc) / 0xc)` is removed
  // as it's likely a redundant check (always true unless overflow with specific compiler behavior)
  // or a compiler artifact.
  
  // Call the custom allocate function.
  // param_1 + 1 points to the second uint in the array, which will hold the allocated pointer.
  return allocate((size_t)param_2 * 0xc, 0, (uintptr_t *)(param_1 + 1), 0x11751);
}

// Function: namespace_destroy
void namespace_destroy(uint *param_1) { // Changed param_1 type to uint* for consistency
  // param_1[1] holds the pointer to the allocated memory.
  // *param_1 holds the capacity.
  deallocate((uintptr_t)param_1[1], (size_t)*param_1 * 0xc);
  param_1[1] = 0; // Clear the stored pointer
  *param_1 = 0;   // Clear capacity
  return;
}

// Function: hash
int hash(const char *param_1) {
  int h = 0;
  for (uint i = 0; (i < 4 && param_1[i] != '\0'); ++i) {
    h = (int)param_1[i] + h * 0x34;
  }
  return h;
}

// Function: lookup_variable
char * lookup_variable(uint *param_1, const char *param_2) {
  int h_val = hash(param_2);
  uint capacity = *param_1; // Capacity is stored in param_1[0]
  // Base pointer to the allocated hash table entries, stored in param_1[1]
  char *base_ptr = (char *)(uintptr_t)param_1[1]; 

  for (uint i = 0; i < capacity; ++i) {
    // Calculate the address of the current slot using quadratic probing
    char *entry_ptr = base_ptr + ((i * i + h_val) % capacity) * 0xc;
    
    // An entry is considered empty if its 'value' field (at offset +4) is 0
    if (*(int *)(entry_ptr + 4) == 0) {
      return NULL; // Empty slot found, so variable is not in table
    }
    
    // Check if the key (first 4 bytes) matches param_2
    if (strncmp(entry_ptr, param_2, 4) == 0) {
      return entry_ptr; // Found a matching key, return the pointer to the entry
    }
  }
  return NULL; // Looped through all possible slots, variable not found
}

// Function: insert_variable
char * insert_variable(uint *param_1, const char *param_2, int param_3) {
  int h_val = hash(param_2);
  uint capacity = *param_1; // Capacity is stored in param_1[0]
  // Base pointer to the allocated hash table entries, stored in param_1[1]
  char *base_ptr = (char *)(uintptr_t)param_1[1];

  char *exact_match_ptr = NULL; // Stores pointer to an entry with matching key AND value
  char *empty_slot_ptr = NULL;  // Stores pointer to the first empty slot found during probing

  for (uint i = 0; i < capacity; ++i) {
    // Calculate the address of the current slot using quadratic probing
    char *current_slot_ptr = base_ptr + ((i * i + h_val) % capacity) * 0xc;
    
    // Check if the slot is empty (value field at offset +4 is 0)
    if (*(int *)(current_slot_ptr + 4) == 0) {
      empty_slot_ptr = current_slot_ptr; // Found an empty slot
      break; // Stop probing as we have a place to insert
    }
    
    // Slot is occupied. Check for an exact match (key AND value).
    if (param_3 == *(int *)(current_slot_ptr + 4) && strncmp(current_slot_ptr, param_2, 4) == 0) {
      exact_match_ptr = current_slot_ptr; // Found an exact match. Store it.
                                          // The original logic continues probing for an empty slot even after finding an exact match.
    }
  }

  if (exact_match_ptr != NULL) {
    return exact_match_ptr; // If an exact match was found, return it.
  }

  if (empty_slot_ptr != NULL) { // If an empty slot was found (and no exact match was returned)
    // Insert the new variable into the empty slot
    memset(empty_slot_ptr, 0, 4);      // Clear the key area (first 4 bytes)
    strncpy(empty_slot_ptr, param_2, 4); // Copy the key (first 4 bytes)
    *(int *)(empty_slot_ptr + 4) = param_3; // Store the value (at offset +4)
    return empty_slot_ptr;
  }
  
  return NULL; // No empty slot found and no exact match found after probing all slots.
}