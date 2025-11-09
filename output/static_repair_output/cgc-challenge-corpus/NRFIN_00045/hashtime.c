#include <string.h> // For memcpy, memset
#include <stdint.h> // For uint8_t, uint32_t
#include <stdio.h>  // For a minimal main function

// Global variable definition for ACCOUNTS.
// The size is an estimation based on access patterns: ACCOUNTS + param_1 * 0x1a4 + 4.
// Assuming param_1 (read from param_5+0xc) can be an index.
#define MAX_ACCOUNTS_INDEX 100 // Example maximum index for param_1
#define ACCOUNT_DATA_SIZE 0x1a4 // Size of each account data segment
#define ACCOUNTS_ARRAY_BASE_OFFSET 4 // Offset within ACCOUNTS array
// The XOR loop accesses up to 0x20 bytes from `accounts_segment_ptr`.
#define REQUIRED_SEGMENT_LENGTH 0x20
#define ACCOUNTS_STORAGE_SIZE (MAX_ACCOUNTS_INDEX * ACCOUNT_DATA_SIZE + ACCOUNTS_ARRAY_BASE_OFFSET + REQUIRED_SEGMENT_LENGTH)
uint8_t ACCOUNTS[ACCOUNTS_STORAGE_SIZE];

// Function: checksum
// Fixed types:
// param_1: int (its initial value is overwritten from param_5+0xc)
// param_2: int (its initial value is overwritten from param_5+8)
// param_3: const uint8_t* (pointer to source data)
// param_4: uint32_t (value, unused)
// param_5: uint8_t* (pointer to destination data)
void checksum(int param_1, int param_2, const uint8_t* param_3, uint32_t param_4, uint8_t* param_5) {
  // Intermediate variables reduced:
  // local_10, local_14, local_18 replaced by loop variable 'i'.
  // local_1c (0x18) was unused and is removed.
  // local_20 (0x10) is a constant and is inlined.
  // local_28 was computed, immediately overwritten, and never used; thus, removed.

  // Initialize 0x20 bytes at param_5 to 0
  memset(param_5, 0, 0x20);

  // Copy 8 bytes from param_3 to param_5 and param_5 + 0x18
  memcpy(param_5, param_3, 8);
  memcpy(param_5 + 0x18, param_3, 8);

  // Read values into param_2 and param_1 from param_5.
  // These are the values used in subsequent calculations; the initial parameters
  // passed to the function are effectively discarded.
  memcpy(&param_2, param_5 + 8, 4);   // param_2 now holds value from param_5 + 8
  memcpy(&param_1, param_5 + 0xc, 4); // param_1 now holds value from param_5 + 0xc

  // Calculate pointer to ACCOUNTS data based on the updated param_1 value.
  // Cast param_1 to unsigned int for robust pointer arithmetic.
  uint8_t* accounts_segment_ptr = ACCOUNTS + (unsigned int)param_1 * ACCOUNT_DATA_SIZE + ACCOUNTS_ARRAY_BASE_OFFSET;

  // Copy 4 bytes from param_5 + 0x14 (which is param_5 + 0x10 + 4) to accounts_segment_ptr
  memcpy(accounts_segment_ptr, param_5 + 0x14, 4);

  // XOR operation over 0x20 bytes
  for (int i = 0; i < 0x20; ++i) {
    param_5[i] ^= accounts_segment_ptr[i];
  }
}

// Minimal main function to make the code Linux compilable.
int main() {
    // Example usage:
    uint8_t source_buffer[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    uint8_t dest_buffer[0x20]; // 32 bytes

    // Initialize ACCOUNTS array with dummy data for demonstration.
    for (int i = 0; i < ACCOUNTS_STORAGE_SIZE; ++i) {
        ACCOUNTS[i] = (uint8_t)(i % 256);
    }

    // Populate dest_buffer with example values that will be read into param_1 and param_2
    // and used by the checksum logic.
    int example_val_for_param2 = 0x100; // Value for param_2 after memcpy
    int example_val_for_param1 = 0x1;   // Value for param_1 (used as an index for ACCOUNTS)
    int example_val_for_accounts_copy = 0xAABBCCDD; // Value copied into ACCOUNTS

    memcpy(dest_buffer + 8, &example_val_for_param2, 4);
    memcpy(dest_buffer + 0xc, &example_val_for_param1, 4);
    memcpy(dest_buffer + 0x14, &example_val_for_accounts_copy, 4);

    // Call checksum with dummy initial values for param_1 and param_2,
    // as they are overwritten from dest_buffer. param_4 is unused.
    checksum(0, 0, source_buffer, 0, dest_buffer);

    // Optional: Print the resulting dest_buffer for verification.
    // printf("Resulting destination_data:\n");
    // for (int i = 0; i < 0x20; ++i) {
    //     printf("%02x ", dest_buffer[i]);
    // }
    // printf("\n");

    return 0;
}