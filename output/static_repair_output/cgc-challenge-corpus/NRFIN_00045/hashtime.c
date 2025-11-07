#include <stdint.h> // For uint8_t, uint32_t
#include <string.h> // For memcpy

// Define ACCOUNTS for compilation. Assuming it's a global buffer.
// The size is arbitrary but chosen to be large enough for typical use cases
// where param_1 is a non-negative index. 0x1a4 is 420 bytes.
// Example: If param_1 can go up to 10, 10 * 420 + 4 + 0x20 = 4200 + 36 = 4236 bytes.
// A size of 4096 bytes (0x1000) is used as a placeholder.
#define ACCOUNTS_GLOBAL_SIZE 4096
static uint8_t ACCOUNTS[ACCOUNTS_GLOBAL_SIZE];

// Function: checksum
// Original signature: void checksum(int param_1,int param_2,int param_3,undefined4 param_4,int param_5)
//
// param_1 (int), param_2 (int): These are passed by value, but their stack copies are modified by memcpy.
// param_3 (int in original): Interpreted as a pointer to 8 bytes of source data.
// param_4 (undefined4 in original): Interpreted as uint32_t, but it is unused in the function body.
// param_5 (int in original): Interpreted as a pointer to a 0x20-byte scratch buffer.
void checksum(int param_1, int param_2, const uint8_t *param_3, uint32_t param_4_unused, uint8_t *param_5) {
  int i; // Loop counter, replacing local_10, local_14, local_18
  
  // Initialize the 0x20 byte buffer pointed to by param_5 to zeros
  for (i = 0; i < 0x20; i++) {
    param_5[i] = 0;
  }
  
  // Copy 8 bytes from param_3 to the start of param_5 and to param_5 + 0x18
  for (i = 0; i < 8; i++) {
    param_5[i] = param_3[i];
    param_5[i + 0x18] = param_3[i];
  }
  
  // Read 4 bytes from param_5 + 8 into the stack location of param_2
  memcpy(&param_2, param_5 + 8, 4);
  
  // Read 4 bytes from param_5 + 0xc into the stack location of param_1
  memcpy(&param_1, param_5 + 0xc, 4);
  
  // Declare a temporary variable to hold the 4 bytes read from param_5 + 0x10.
  // The original code had a dead assignment to local_28 before this memcpy, which has been removed.
  int temp_val_from_param5_10;
  memcpy(&temp_val_from_param5_10, param_5 + 0x10, 4);
  
  // Calculate the destination address in the global ACCOUNTS buffer.
  // Use uintptr_t for pointer arithmetic with an integer index to ensure correctness
  // and avoid potential compiler warnings on some architectures.
  uint8_t *account_ptr = ACCOUNTS + (uintptr_t)param_1 * 0x1a4 + 4;
  
  // Copy 4 bytes from param_5 + 0x14 (which is param_5 + 0x10 + 4) into the calculated account_ptr location.
  memcpy(account_ptr, param_5 + 0x14, 4);
  
  // XOR the 0x20 bytes in param_5 with bytes from the ACCOUNTS buffer starting at account_ptr.
  for (i = 0; i < 0x20; i++) {
    param_5[i] ^= account_ptr[i];
  }
  return;
}