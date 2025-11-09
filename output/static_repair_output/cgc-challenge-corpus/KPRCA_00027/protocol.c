#include <string.h>  // For memcmp, memcpy, memset
#include <stdint.h>  // For uint32_t, intptr_t
#include <stdlib.h>  // For rand

// Mock declarations for external functions and types
// Assuming 'uint' in original snippet refers to 'unsigned int'
typedef unsigned int uint;

// transmit expects a file descriptor, a buffer, a count, and flags.
void transmit(int fd, const void *buf, size_t count, int flags);

// permit_new: arg1 is a pointer, arg2 and arg3 are 4-byte values.
int permit_new(void *arg1, uint32_t arg2, uint32_t arg3);

// permit_test: arg1 is a pointer, arg2 is a 4-byte value, arg3 is a pointer.
int permit_test(void *arg1, uint32_t arg2, void *arg3);

// pring_refactor: arg1 is a pointer.
int pring_refactor(void *arg1);

// destroy_permit_ring: arg1 is an integer (likely an ID or pointer).
void destroy_permit_ring(int arg1);

// pring_test: all three args are pointers.
int pring_test(void *arg1, void *arg2, void *arg3);

// Global variables
unsigned char g_mkey[16] = {0}; // Master key, initialized to zeros
int g_auth = 0;                 // Authentication status
unsigned char g_session_key[16] = {0}; // Session key, initialized to zeros

// Function: _send_response
// param_1 is a single byte (response code)
// param_2 is an integer, interpreted as a pointer to data for transmit (or a value)
// param_3 is an integer, interpreted as the size of data for transmit
void _send_response(unsigned char param_1, int param_2, int param_3) {
  unsigned char response_code_buffer[8]; // Buffer for the response code
  
  response_code_buffer[0] = param_1;
  transmit(1, response_code_buffer, 1, 0); // Transmit the single-byte response code
  
  if ((param_2 != 0) && (param_3 != 0)) {
    transmit(1, &param_3, sizeof(int), 0); // Transmit the size of the following data
    // Transmit the actual data pointed to by param_2 (after casting the integer to a pointer)
    transmit(1, (void*)(intptr_t)param_2, param_3, 0); 
  }
}

// Function: handle_auth
void handle_auth(void *param_1, int param_2) {
  // Check if param_2 is large enough (0xf = 15) and master key matches
  if ((param_2 > 0xf) && (memcmp(g_mkey, param_1, 0x10) == 0)) {
    g_auth = 3; // Set authentication status
    rand(); // Call random number generator (for side effects or to generate session key)
    // Send success response (0), session key, and its length (0x10 = 16 bytes)
    _send_response(0, (int)(intptr_t)g_session_key, 0x10);
    return;
  }
  // Send error response (0x10 = 16)
  _send_response(0x10, 0, 0);
}

// Function: handle_new_permit
void handle_new_permit(void *param_1, uint param_2) {
  // Check if param_2 is large enough (0x21 = 33), authenticated, and session key matches
  if (((param_2 > 0x21) && (g_auth != 0)) &&
      (memcmp(g_session_key, param_1, 0x10) == 0)) {
    // Set a byte at offset 0x19 to 0
    *((unsigned char *)param_1 + 0x19) = 0;
    
    // Call permit_new with appropriate arguments
    // arg1: pointer to data at offset 0x10
    // arg2: 4-byte value at offset 0x1a
    // arg3: 4-byte value at offset 0x1e
    int permit_result = permit_new((unsigned char *)param_1 + 0x10,
                                   *(uint32_t *)((unsigned char *)param_1 + 0x1a),
                                   *(uint32_t *)((unsigned char *)param_1 + 0x1e));
    if (permit_result != 0) {
      // If permit_new succeeds (returns non-zero), send success response
      _send_response(0, permit_result, 0x1a);
      g_auth--; // Decrement authentication status
      return;
    }
  }
  // Send error response
  _send_response(0x10, 0, 0);
}

// Function: handle_new_permit_ring
void handle_new_permit_ring(void *param_1, uint param_2) {
  // Check if param_2 is large enough (0x95 = 149)
  if (param_2 > 0x95) {
    unsigned char response_buffer[0x86]; // Buffer for response data (134 bytes)
    int permit_count = 0; // Counter for permits in the ring
    
    memset(response_buffer, 0, sizeof(response_buffer)); // Zero out the response buffer

    // Get the ring size from param_1 at offset 0x10
    int ring_size = *(int *)((unsigned char *)param_1 + 0x10);

    // Check authentication, session key, and ring size limit
    if (((g_auth != 0) && (memcmp(g_session_key, param_1, 0x10) == 0)) &&
        (ring_size < 6)) { // Ring size must be less than 6
      int i = 0;
      while (i <= ring_size) {
        // Loop condition: continue if i < ring_size, or if i == ring_size AND param_2 is NOT 0x96
        if (i == ring_size && param_2 == 0x96) {
          break; // Exit loop if it's the last iteration and param_2 matches 0x96
        }

        uint32_t permit_offset = i * 0x1a; // Calculate offset for current permit data

        // Test the permit data
        // arg1: pointer to permit data at 0x14 + permit_offset
        // arg2: 4-byte value at offset 0x2a from param_1 + permit_offset
        // arg3: pointer to data at offset 0x1c from param_1 + permit_offset
        if (permit_test((unsigned char *)param_1 + 0x14 + permit_offset,
                        *(uint32_t *)((unsigned char *)param_1 + permit_offset + 0x2a),
                        (void *)((unsigned char *)param_1 + permit_offset + 0x1c)) != 0) {
          _send_response(0x10, 0, 0); // Send error if permit_test fails
          return;
        }
        
        // Copy permit data to the response buffer
        memcpy(response_buffer + i * 0x1a, (unsigned char *)param_1 + 0x14 + permit_offset, 0x1a);
        permit_count++; // Increment permit counter
        i++;
      }
      // Send success response with permit_count and response_buffer
      _send_response(0, (int)(intptr_t)response_buffer, 0x86);
      g_auth--; // Decrement authentication status
      return;
    }
  }
  _send_response(0x10, 0, 0); // Send error response
}

// Function: handle_refactor_ring
void handle_refactor_ring(void *param_1, uint param_2) {
  // Check if param_2 is large enough (0x95 = 149), authenticated, and session key matches
  if (((param_2 > 0x95) && (g_auth != 0)) &&
      (memcmp(g_session_key, param_1, 0x10) == 0)) {
    // Call pring_refactor with data at offset 0x10
    int refactor_result = pring_refactor((unsigned char *)param_1 + 0x10);
    if (refactor_result != 0) {
      _send_response(0, refactor_result, 0x86); // Send success response with result
      destroy_permit_ring(refactor_result); // Destroy the permit ring
      g_auth--; // Decrement authentication status
      return;
    }
  }
  _send_response(0x10, 0, 0); // Send error response
}

// Function: handle_test_permit
void handle_test_permit(void *param_1, uint param_2) {
  // Check if param_2 is large enough (0x31 = 49), authenticated, and session key matches
  if (((param_2 > 0x31) && (g_auth != 0)) &&
      (memcmp(g_session_key, param_1, 0x10) == 0)) {
    // Call permit_test
    // arg1: pointer to data at offset 0x10
    // arg2: 4-byte value at offset 0x2a
    // arg3: pointer to data at offset 0x2e
    if (permit_test((unsigned char *)param_1 + 0x10,
                    *(uint32_t *)((unsigned char *)param_1 + 0x2a),
                    (void *)((unsigned char *)param_1 + 0x2e)) == 0) {
      _send_response(0, 0, 0); // Send success response if test passes (returns 0)
      g_auth--; // Decrement authentication status
      return;
    }
  }
  _send_response(0x10, 0, 0); // Send error response
}

// Function: handle_test_permit_ring
void handle_test_permit_ring(void *param_1, uint param_2) {
  // Check if param_2 is large enough (0xb9 = 185), authenticated, and session key matches
  if (((param_2 > 0xb9) && (g_auth != 0)) &&
      (memcmp(g_session_key, param_1, 0x10) == 0)) {
    // Call pring_test with three pointers
    if (pring_test((unsigned char *)param_1 + 0x10,
                   (unsigned char *)param_1 + 0x96,
                   (unsigned char *)param_1 + 0xaa) == 0) {
      _send_response(0, 0, 0); // Send success response if test passes (returns 0)
      g_auth--; // Decrement authentication status
      return;
    }
  }
  _send_response(0x10, 0, 0); // Send error response
}

// Function: handle_command
void handle_command(unsigned char *param_1, int param_2) {
  if (param_2 < 1) {
    _send_response(0x10, 0, 0); // Invalid command length
  } else {
    // Use the first byte of param_1 as the command code
    // Pass the rest of the buffer (param_1 + 1) and reduced length (param_2 - 1)
    switch (*param_1) {
    case 0:
      handle_auth(param_1 + 1, param_2 - 1);
      break;
    case 1:
      handle_new_permit(param_1 + 1, param_2 - 1);
      break;
    case 2:
      handle_new_permit_ring(param_1 + 1, param_2 - 1);
      break;
    case 3:
      handle_refactor_ring(param_1 + 1, param_2 - 1);
      break;
    case 4:
      handle_test_permit(param_1 + 1, param_2 - 1);
      break;
    case 5:
      handle_test_permit_ring(param_1 + 1, param_2 - 1);
      break;
    default:
      _send_response(0x10, 0, 0); // Unknown command
    }
  }
}