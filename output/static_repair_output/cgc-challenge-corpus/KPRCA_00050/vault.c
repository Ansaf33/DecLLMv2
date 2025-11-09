#include <stdint.h> // For uint8_t, uint16_t, uint32_t, uintptr_t
#include <stdlib.h> // For malloc, free
#include <string.h> // For memcpy
#include <stdio.h>  // For a dummy write_bytes

// --- Type Definitions ---
typedef uint8_t  undefined;
typedef uint16_t undefined2;
typedef uint32_t undefined4;

// --- Global Variables (placeholders if not defined elsewhere) ---
uint32_t *the_vault = NULL;
uint32_t current_time = 0; // Placeholder for a global timestamp
uint32_t DAT_0001600c = 0; // Placeholder for another global timestamp/limit

// --- External Functions (placeholders if not defined elsewhere) ---
// Dummy implementation for compilation. In a real system, this would interact with I/O.
void write_bytes(void *data, size_t size) {
    // Example: fwrite(data, 1, size, stdout);
}

// --- Byte Swap Functions (assuming these are intended as generic byte swaps) ---
static inline uint16_t swap16(uint16_t val) {
    return (val << 8) | (val >> 8);
}

static inline uint32_t swap32(uint32_t val) {
    return ((val << 24) & 0xFF000000) |
           ((val << 8)  & 0x00FF0000) |
           ((val >> 8)  & 0x0000FF00) |
           ((val >> 24) & 0x000000FF);
}

// Dummy CARRY4 function - this is a common pattern for detecting carry-out from 32-bit addition.
static inline uint32_t CARRY4(uint32_t a, uint32_t b) {
    return (uint32_t)(a + b < a); // Returns 1 if a+b overflows, 0 otherwise
}

// Function: write_msg
void write_msg(uint8_t msg_id, void *data_ptr, int data_len) {
  uint32_t total_len_field = data_len + 1; // +1 for the msg_id itself
  uint8_t header_buf[7]; 
  uint16_t header_type_val = swap16(2);
  memcpy(header_buf, &header_type_val, 2);

  if (total_len_field < 0x8000) {
    uint16_t short_len_val = swap16(total_len_field & 0xffff);
    memcpy(header_buf + 2, &short_len_val, 2);
    header_buf[4] = msg_id;
    write_bytes(header_buf, 5);
  } else {
    uint32_t long_len_val = swap32(total_len_field | 0x80000000);
    memcpy(header_buf + 2, &long_len_val, 4);
    header_buf[6] = msg_id;
    write_bytes(header_buf, 7);
  }
  write_bytes(data_ptr, data_len);
}

// Function: init_vault
void init_vault(void) {
  the_vault = (uint32_t *)malloc(0xfb0);
  if (the_vault == NULL) {
      // Handle allocation error, e.g., print error and exit
      return;
  }
  the_vault[0] = 0xd0856180;
  the_vault[1] = 0x34c;
  the_vault[2] = 3600000;
  the_vault[0x3eb] = 0; // This element stores the count of items in the vault
}

// Function: store_in_vault
// param_1: If not NULL, it's a pointer to an existing vault entry (uint32_t* to data_ptr/size pair)
// param_2: The data to store (void*)
// param_3: The size of the data (size_t, but stored as uint32_t)
uint32_t *store_in_vault(uint32_t *param_1, void *param_2, size_t param_3) {
  uint32_t *entry_ptr;
  
  if (param_1 == NULL) { // Allocate new entry
    if (the_vault[0x3eb] >= 499) { // Check if vault is full (max 499 entries)
      return NULL;
    }
    entry_ptr = &the_vault[3 + the_vault[0x3eb] * 2]; // Point to the next available slot
    the_vault[0x3eb]++; // Increment item count
  } else { // Update existing entry
    entry_ptr = param_1;
    // Check if entry_ptr is within valid bounds of the vault entries
    if (entry_ptr < &the_vault[3] || entry_ptr >= &the_vault[3 + the_vault[0x3eb] * 2]) {
      return NULL; // Invalid entry pointer
    }
    // Free existing data if it exists before overwriting
    if (entry_ptr[0] != 0) {
        free((void*)(uintptr_t)entry_ptr[0]);
    }
  }

  void *__dest = malloc(param_3);
  if (__dest == NULL) {
    if (param_1 != NULL) { // If updating an existing entry, and malloc fails, clear it.
      entry_ptr[0] = 0;
      entry_ptr[1] = 0;
    } else { // If new entry, decrement count as it wasn't successfully added
        the_vault[0x3eb]--;
    }
    return NULL; // Allocation failed
  }
  
  memcpy(__dest, param_2, param_3);
  
  // Store the allocated data pointer and its size
  entry_ptr[0] = (uint32_t)(uintptr_t)__dest; // Store data pointer
  entry_ptr[1] = (uint32_t)param_3; // Store size
  
  // Return the address of the data pointer within the vault entry
  return entry_ptr;
}

// Function: retrieve_from_vault
// param_1: Pointer to the vault entry (uint32_t* to data_ptr/size pair)
// param_2: Pointer to store the retrieved size (uint32_t*)
uint32_t retrieve_from_vault(uint32_t *param_1, uint32_t *param_2) {
  // Translate the complex condition from the original snippet.
  // The expression `(uint)(boolean_expression)` casts boolean to int (0 or 1).
  uint32_t condition_val1 = (uint32_t)(current_time < the_vault[0]);
  uint32_t uVar2_val = the_vault[1] + CARRY4(the_vault[0], the_vault[2]);
  uint32_t condition_val2 = (uint32_t)(current_time < (the_vault[0] + the_vault[2]));

  if ((DAT_0001600c < the_vault[1] || (DAT_0001600c - the_vault[1]) < condition_val1) ||
      (uVar2_val <= DAT_0001600c && condition_val2 <= (DAT_0001600c - uVar2_val))) {
    return 0;
  } else {
    // Additionally, check if the entry itself is valid (not null/cleared)
    if (param_1 == NULL || param_1[0] == 0) {
        return 0; // Invalid entry
    }

    uint32_t data_ptr = param_1[0];
    *param_2 = param_1[1];
    param_1[0] = 0; // Clear data pointer in vault
    param_1[1] = 0; // Clear size in vault
    return data_ptr;
  }
}

// Function: handle_msg_vault
undefined4 handle_msg_vault(char *param_1, int param_2) {
  if (param_2 == 0) {
    return 0;
  }

  uint8_t msg_type = param_1[0];
  uint32_t data_len = param_2 - 1;
  uint32_t *data_ptr_in_msg = (uint32_t *)(param_1 + 1);
  undefined4 result = 1; // Default return value for success paths

  if (msg_type == 0x01) {
    uint32_t count = the_vault[0x3eb];
    size_t total_size = count * 8; // Each entry is 8 bytes (data_ptr + size)
    void *allocated_data = malloc(total_size);
    if (allocated_data == NULL) {
      write_msg(1, NULL, 0);
    } else {
      for (uint32_t i = 0; i < count; ++i) {
        ((uint32_t*)allocated_data)[i * 2] = swap32(the_vault[3 + i * 2]);
        ((uint32_t*)allocated_data)[i * 2 + 1] = swap32(the_vault[3 + i * 2 + 1]);
      }
      write_msg(1, allocated_data, total_size);
      free(allocated_data);
    }
  } else if (msg_type == 0x02) {
    uint32_t *stored_entry_ptr = store_in_vault(NULL, data_ptr_in_msg, data_len);
    uint32_t response_val = stored_entry_ptr ? swap32((uint32_t)(uintptr_t)stored_entry_ptr) : 0;
    write_msg(2, &response_val, 4);
  } else if (msg_type == 0x03 && data_len > 3) {
    uint32_t target_entry_val = swap32(*data_ptr_in_msg);
    uint32_t *entry_address = (uint32_t *)(uintptr_t)target_entry_val;
    uint32_t *stored_entry_ptr = store_in_vault(entry_address, param_1 + 5, param_2 - 5);
    uint32_t response_val = stored_entry_ptr ? swap32((uint32_t)(uintptr_t)stored_entry_ptr) : 0;
    write_msg(3, &response_val, 4);
  } else if (msg_type == 0x04 && data_len > 3) {
    uint32_t target_entry_val = swap32(*data_ptr_in_msg);
    uint32_t retrieved_size;
    uint32_t *entry_address = (uint32_t *)(uintptr_t)target_entry_val;
    uint32_t retrieved_data_ptr = retrieve_from_vault(entry_address, &retrieved_size);
    
    if (retrieved_data_ptr == 0) {
      write_msg(4, NULL, 0);
    } else {
      write_msg(4, (void*)(uintptr_t)retrieved_data_ptr, retrieved_size);
      free((void*)(uintptr_t)retrieved_data_ptr);
    }
  } else {
      // Unhandled message type or invalid length for type 3/4
      // The original code implies result is still 1 here even if no action matched.
  }
  return result;
}