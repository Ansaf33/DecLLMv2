#include <stdlib.h> // For malloc, free, EXIT_SUCCESS, EXIT_FAILURE
#include <string.h> // For memset, strlen
#include <stdint.h> // For uintptr_t
#include <stdio.h>  // For printf, fprintf

// Type definitions
// undefined4 appears to be an unsigned 4-byte integer.
typedef unsigned int undefined4;

// code appears to be a function pointer: unsigned int (*)(char*, size_t)
typedef unsigned int (*hash_func_ptr_t)(char*, size_t);

// --- Dummy Hash Functions for demonstration ---
// A simple hash function (DJB2)
unsigned int hash_djb2(char *str, size_t len) {
    unsigned int hash = 5381;
    for (size_t i = 0; i < len; ++i) {
        hash = ((hash << 5) + hash) + str[i]; // hash * 33 + c
    }
    return hash;
}

// Another simple hash function (SDBM)
unsigned int hash_sdbm(char *str, size_t len) {
    unsigned int hash = 0;
    for (size_t i = 0; i < len; ++i) {
        hash = str[i] + (hash << 6) + (hash << 16) - hash;
    }
    return hash;
}

// A third simple hash function (Lose-lose hash)
unsigned int hash_lose_lose(char *str, size_t len) {
    unsigned int hash = 0;
    for (size_t i = 0; i < len; ++i) {
        hash += str[i];
    }
    return hash;
}
// --- End Dummy Hash Functions ---


// Function: bloomy_new
// param_1 (capacity_bits) is the total number of bits in the bloom filter.
// param_2, param_3, param_4 are pointers to hash functions.
void ** bloomy_new(unsigned int capacity_bits, hash_func_ptr_t hash_func1, hash_func_ptr_t hash_func2, hash_func_ptr_t hash_func3) {
  if (capacity_bits == 0) {
    return NULL;
  }

  // Allocate space for 5 void* pointers: [bits_array, capacity_bits, hash_func1, hash_func2, hash_func3]
  // This structure mimics the original code's use of void** as a handle.
  void **bf_handle = (void **)malloc(5 * sizeof(void*));
  if (bf_handle == NULL) {
    return NULL;
  }
  memset(bf_handle, 0, 5 * sizeof(void*));

  // Calculate number of bytes needed for the bit array
  // (capacity_bits + 7) / 8 ensures enough bytes even if capacity_bits is not a multiple of 8
  size_t num_bytes = (capacity_bits + 7) / 8;
  unsigned char *bits_array = (unsigned char *)malloc(num_bytes);
  if (bits_array == NULL) {
    free(bf_handle); // Clean up previously allocated handle if bit array allocation fails
    return NULL;
  }
  memset(bits_array, 0, num_bytes); // Initialize all bits to 0

  // Store the allocated bit array and other parameters in the handle
  bf_handle[0] = bits_array;
  // Store capacity_bits as a uintptr_t in a void* slot.
  // This relies on sizeof(unsigned int) <= sizeof(void*) and is a common, though not strictly portable, C pattern.
  bf_handle[1] = (void*)(uintptr_t)capacity_bits;
  bf_handle[2] = (void*)hash_func1;
  bf_handle[3] = (void*)hash_func2;
  bf_handle[4] = (void*)hash_func3;

  return bf_handle;
}

// Function: bloomy_free
// Frees the memory associated with a Bloom filter handle.
void bloomy_free(void **bf_handle) {
  if (bf_handle != NULL) {
    // bf_handle[0] points to the actual bit array
    if (bf_handle[0] != NULL) {
      free(bf_handle[0]);
    }
    // Free the handle itself
    free(bf_handle);
  }
}

// Function: bloomy_check
// Checks if an item is possibly present in the Bloom filter.
// Returns 1 if likely present, 0 if definitely not present, (undefined4)-1 on error.
undefined4 bloomy_check(void **bf_handle, char *item) {
  if (bf_handle == NULL || item == NULL) {
    return (undefined4)-1; // Indicate error with 0xffffffff
  }

  // Extract components from the handle, casting back to their original types
  unsigned char *bits = (unsigned char *)bf_handle[0];
  unsigned int capacity_bits = (unsigned int)(uintptr_t)bf_handle[1];
  
  // A capacity of 0 indicates an uninitialized or invalid filter
  if (capacity_bits == 0) {
      return (undefined4)-1;
  }

  size_t item_len = strlen(item);

  // Iterate through the 3 hash functions
  for (int i = 0; i < 3; ++i) {
    hash_func_ptr_t current_hash_func = (hash_func_ptr_t)bf_handle[2 + i];
    if (current_hash_func != NULL) {
      unsigned int hash_val = current_hash_func(item, item_len);
      unsigned int bit_index = hash_val % capacity_bits;
      
      // Check if the corresponding bit is set
      // (bit_index / 8) gives the byte index
      // (bit_index % 8) gives the bit position within that byte
      if (!((bits[bit_index / 8] >> (bit_index % 8)) & 1)) {
        return 0; // At least one bit is not set, so item is definitely not in the filter
      }
    }
  }
  return 1; // All relevant bits are set, item is likely in the filter (false positives are possible)
}

// Function: bloomy_add
// Adds an item to the Bloom filter by setting the corresponding bits.
void bloomy_add(void **bf_handle, char *item) {
  if (bf_handle == NULL || item == NULL) {
    return;
  }

  // Extract components from the handle
  unsigned char *bits = (unsigned char *)bf_handle[0];
  unsigned int capacity_bits = (unsigned int)(uintptr_t)bf_handle[1];
  
  if (capacity_bits == 0) {
      return;
  }

  size_t item_len = strlen(item);

  // Iterate through the 3 hash functions
  for (int i = 0; i < 3; ++i) {
    hash_func_ptr_t current_hash_func = (hash_func_ptr_t)bf_handle[2 + i];
    if (current_hash_func != NULL) {
      unsigned int hash_val = current_hash_func(item, item_len);
      unsigned int bit_index = hash_val % capacity_bits;
      
      // Set the corresponding bit
      // (bit_index / 8) gives the byte index
      // (bit_index % 8) gives the bit position within that byte
      bits[bit_index / 8] |= (1 << (bit_index % 8));
    }
  }
}

// Main function for demonstration
int main() {
    // Define capacity for the Bloom filter (e.g., 1000 bits)
    unsigned int capacity = 1000;

    // Create a new Bloom filter instance with three hash functions
    void **my_bloom_filter = bloomy_new(capacity, hash_djb2, hash_sdbm, hash_lose_lose);

    if (my_bloom_filter == NULL) {
        fprintf(stderr, "Failed to create Bloom filter.\n");
        return EXIT_FAILURE;
    }

    printf("Bloom filter created with %u bits.\n", capacity);

    // Items to add
    char *item1 = "apple";
    char *item2 = "banana";
    char *item3 = "cherry";
    char *item4 = "date"; // Will be checked, not added
    char *item5 = "grape";
    char *item6 = "kiwi";

    // Add items to the Bloom filter
    printf("\nAdding items: \"%s\", \"%s\", \"%s\"\n", item1, item2, item3);
    bloomy_add(my_bloom_filter, item1);
    bloomy_add(my_bloom_filter, item2);
    bloomy_add(my_bloom_filter, item3);

    // Check for items
    printf("\nChecking for items:\n");
    printf("Is \"%s\" present? %s\n", item1, bloomy_check(my_bloom_filter, item1) == 1 ? "Yes" : "No (or error)");
    printf("Is \"%s\" present? %s\n", item2, bloomy_check(my_bloom_filter, item2) == 1 ? "Yes" : "No (or error)");
    printf("Is \"%s\" present? %s\n", item3, bloomy_check(my_bloom_filter, item3) == 1 ? "Yes" : "No (or error)");
    printf("Is \"%s\" present? %s\n", item4, bloomy_check(my_bloom_filter, item4) == 1 ? "Yes (false positive possible)" : "No");
    printf("Is \"%s\" present? %s\n", item5, bloomy_check(my_bloom_filter, item5) == 1 ? "Yes (false positive possible)" : "No");
    printf("Is \"%s\" present? %s\n", item6, bloomy_check(my_bloom_filter, item6) == 1 ? "Yes (false positive possible)" : "No");


    // Test edge cases or error handling for bloomy_check
    printf("\nTesting error cases for bloomy_check:\n");
    // Passing NULL for bloom filter handle
    printf("Check with NULL filter: %d (expected -1)\n", bloomy_check(NULL, item1));
    // Passing NULL for item string
    printf("Check with NULL item: %d (expected -1)\n", bloomy_check(my_bloom_filter, NULL));
    
    // Free the Bloom filter
    bloomy_free(my_bloom_filter);
    printf("\nBloom filter freed.\n");

    return EXIT_SUCCESS;
}