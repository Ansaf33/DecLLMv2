#include <stdio.h>    // For printf
#include <stdlib.h>   // For free, calloc, atoi
#include <string.h>   // For memset, strncpy, strncmp, strlen, memcpy
#include <stdint.h>   // For uint8_t, uint32_t, uintptr_t

// Type aliases for clarity and compatibility
typedef uint32_t undefined4;
typedef uint8_t byte;

// Forward declarations for external functions
// These function signatures are inferred and might need adjustment based on actual implementation.
// For simplicity, assuming common types like void*, size_t, char*, uint32_t.
// receive_fixed and receive_until return count of bytes read.
uint32_t receive_fixed(void *buf, size_t len);
// receive_until reads until delimiter or max_len bytes, returns bytes read (excluding delimiter).
uint32_t receive_until(void *buf, char delimiter, size_t max_len);
void VerifyPointerOrTerminate(void *ptr, const char *msg);
int LookupNode(char *name);
void PrintTree(char *root_name);
int InsertNodeInTree(void *node);
int DeleteNode(char *name);
void ServePage(void *data, uint32_t len);
void InteractWithPage(void *data, uint32_t len, void *input);


// Function: DecodeData
// Fixed signature to match the call site in ReceiveCommand and added bounds checks.
// The internal logic preserves the original (potentially unusual) bit-shifting and pointer advancement.
uint32_t DecodeData(byte *decoded_buf, int total_decoded_len, byte *encoded_buf, int encoded_len) {
    if (total_decoded_len <= 0) {
        return 0; // Nothing to decode
    }
    if (encoded_len <= 0) {
        return -1; // No encoded data to read from
    }

    // Decode the first byte
    decoded_buf[0] = encoded_buf[0] >> 1;

    byte *current_encoded_ptr = encoded_buf; // Pointer to current byte in encoded_buf
                                              // It will be advanced based on the original logic.

    // Loop for the remaining total_decoded_len - 1 bytes
    // 'i' represents the 0-indexed position in decoded_buf being written (from 1 to total_decoded_len - 1)
    for (int i = 1; i < total_decoded_len; ++i) {
        // bit_offset_in_byte is derived from 'i', simulating the original 'local_10 % 8' behavior
        // where local_10 started from 1 for the second decoded byte.
        int bit_offset_in_byte = i % 8;

        // Check if accessing current_encoded_ptr[1] would go out of bounds of encoded_buf
        // (current_encoded_ptr - encoded_buf) gives the current byte offset from the start of encoded_buf.
        if ((current_encoded_ptr - encoded_buf) + 1 >= encoded_len) {
            // Not enough encoded data to read current_encoded_ptr[1]
            return -1; // Indicate error due to insufficient encoded data
        }

        // Apply the original bit-shifting logic
        // The casts to (uint32_t) ensure proper promotion before shifting, replicating original disassembly intent
        decoded_buf[i] = (byte)(((uint32_t)current_encoded_ptr[1] >> (bit_offset_in_byte + 1)) |
                               ((uint32_t)current_encoded_ptr[0] << (7 - bit_offset_in_byte))) & 0x7f;

        // Advance current_encoded_ptr based on original logic
        if (bit_offset_in_byte != 7) {
            current_encoded_ptr++;
        }
    }
    return 0; // Success
}

// Function: DestroyCommand
void DestroyCommand(undefined4 *param_1) {
  if (param_1[0x12] != 0) {
    free((void *)(uintptr_t)param_1[0x12]); // Safely cast uint32_t to void*
    param_1[0x12] = 0;
  }
  *param_1 = 0;
  memset(param_1 + 1,0,0x40); // Clears 0x40 bytes (64 bytes) starting from param_1[1]
  param_1[0x11] = 0;
}

// Function: ReceiveCommand
undefined4 ReceiveCommand(undefined4 *param_1,undefined4 *param_2) {
  uint32_t bytes_received;
  char buffer[65]; // Increased size to 65 for 0x40 (64) max_len + null terminator
  uint32_t decoded_data_len;
  void *decoded_data_ptr;
  void *encoded_data_ptr;
  uint32_t encoded_data_len;
  
  if (param_1[0x12] != 0) {
    DestroyCommand(param_1);
  }

  // Receive initial command header (e.g., "ACS+0.1" or "ACS-0.1")
  bytes_received = receive_fixed(buffer, 7);
  if (bytes_received != 7) {
    return 0xffffffff; // Error: did not receive expected 7 bytes
  }
  buffer[7] = '\0'; // Null-terminate for strncmp safety

  if (strncmp(buffer, "ACS+0.1", 7) == 0) {
    *param_2 = 1; // Set flag for "+"
  } else if (strncmp(buffer, "ACS-0.1", 7) == 0) {
    *param_2 = 0; // Set flag for "-"
  } else {
    return 0xfffffffe; // Error: unknown header
  }

  // Receive command type (e.g., "[REQUEST")
  // Fix: The original code used a single char `local_5c` here, losing most of the data.
  // Assuming it should read into `buffer` for subsequent comparisons.
  bytes_received = receive_until(buffer, ':', 64); // Max 64 bytes + null terminator
  if (bytes_received == 0 || bytes_received == 64) { // 64 means delimiter not found within max_len
    return 0xffffffff; // Error: no command type received, delimiter immediately present, or buffer full
  }
  buffer[bytes_received] = '\0'; // Null-terminate the received string

  if (buffer[0] != '[') { // Check the first character of the received string
    return 0xfffffffe; // Error: command type does not start with '['
  }

  // Determine command type from string (after '[').
  // The original strncmp uses `acStack_5b` which was not populated correctly.
  // Adjust comparison to start after '['.
  char *cmd_str = buffer + 1; // Point past '['
  size_t cmd_len = bytes_received - 1; // Length of string after '['

  if (cmd_len == strlen("REQUEST") && strncmp(cmd_str, "REQUEST", cmd_len) == 0) {
    *param_1 = 1;
  } else if (cmd_len == strlen("QUERY") && strncmp(cmd_str, "QUERY", cmd_len) == 0) {
    *param_1 = 2;
  } else if (cmd_len == strlen("SEND") && strncmp(cmd_str, "SEND", cmd_len) == 0) {
    *param_1 = 3;
  } else if (cmd_len == strlen("REMOVE") && strncmp(cmd_str, "REMOVE", cmd_len) == 0) {
    *param_1 = 4;
  } else if (cmd_len == strlen("VISUALIZE") && strncmp(cmd_str, "VISUALIZE", cmd_len) == 0) {
    *param_1 = 5;
  } else if (cmd_len == strlen("INTERACT") && strncmp(cmd_str, "INTERACT", cmd_len) == 0) {
    *param_1 = 6;
  } else {
    return 0xfffffffe; // Error: unknown command type
  }

  // Receive filename/identifier
  bytes_received = receive_until(buffer, ':', 64);
  if (bytes_received == 64) { // 64 means delimiter not found within max_len
    return 0xffffffff; // Error: filename too long or no delimiter
  }
  
  if (bytes_received != 0) {
    strncpy((char *)(param_1 + 1), buffer, bytes_received);
    ((char *)(param_1 + 1))[bytes_received] = '\0'; // Null-terminate filename
  } else {
    ((char *)(param_1 + 1))[0] = '\0'; // Ensure filename is empty string if nothing received
  }

  // Receive data length (as string, then convert to int)
  bytes_received = receive_until(buffer, ':', 64);
  // Length string too long for typical int (e.g. "1234567" is 7 chars) or buffer full (64 chars)
  if (bytes_received >= 7 || bytes_received == 64) { 
    return 0xffffffff; // Error: length string too long or no delimiter
  }
  buffer[bytes_received] = '\0';
  param_1[0x11] = (uint32_t)atoi(buffer); // Convert string to integer

  decoded_data_len = param_1[0x11]; // Use a clearer variable name

  if ((int32_t)decoded_data_len < 1) { // Check if data length is less than 1 (could be 0 or -1)
    if (decoded_data_len == (uint32_t)-1) { // Special case for -1 indicating encoded data
      // Receive actual encoded data length
      bytes_received = receive_fixed(&encoded_data_len, 4); 
      if (bytes_received != 4) {
        return 0xffffffff; // Error: did not receive encoded data length
      }

      // Calculate the maximum possible decoded length based on encoded_data_len
      decoded_data_len = (encoded_data_len * 8) / 7;
      // Allocate one extra byte for potential null termination or safety, as per original `calloc`
      decoded_data_ptr = calloc(decoded_data_len + 1, 1);
      param_1[0x12] = (undefined4)(uintptr_t)decoded_data_ptr; // Store pointer in command structure
      VerifyPointerOrTerminate(decoded_data_ptr, "command->data during parsing");

      // Allocate buffer for raw encoded data
      encoded_data_ptr = calloc(encoded_data_len + 1, 1);
      VerifyPointerOrTerminate(encoded_data_ptr, "encoded_data during parsing");

      // Receive the encoded data
      bytes_received = receive_fixed(encoded_data_ptr, encoded_data_len);
      if (bytes_received != encoded_data_len) {
        free(encoded_data_ptr);
        return 0xffffffff; // Error: did not receive all encoded data
      }

      // Decode the data
      if (DecodeData((byte *)decoded_data_ptr, decoded_data_len, (byte *)encoded_data_ptr, encoded_data_len) != 0) {
        free(encoded_data_ptr);
        return 0xffffffff; // Error during decoding
      }

      // Check if the last byte of decoded data is null, adjust length if so
      if (*((char *)decoded_data_ptr + decoded_data_len - 1) == '\0') {
        param_1[0x11] = decoded_data_len - 1;
      } else {
        param_1[0x11] = decoded_data_len;
      }
      free(encoded_data_ptr); // Free the temporary encoded data buffer
    }
  } else { // decoded_data_len >= 1
    // Allocate buffer for direct data
    decoded_data_ptr = calloc(decoded_data_len + 1, 1);
    param_1[0x12] = (undefined4)(uintptr_t)decoded_data_ptr;
    VerifyPointerOrTerminate(decoded_data_ptr, "command->data during parsing");
    
    // Receive the direct data
    bytes_received = receive_fixed(decoded_data_ptr, decoded_data_len);
    if (bytes_received != decoded_data_len) {
      return 0xffffffff; // Error: did not receive all expected data
    }
  }

  // Receive final closing bracket ']'
  bytes_received = receive_fixed(buffer, 1);
  if (bytes_received != 1) {
    return 0xffffffff; // Error: did not receive closing bracket
  }

  if (buffer[0] == ']') {
    return 0; // Success
  } else {
    return 0xfffffffe; // Error: unexpected character instead of ']'
  }
}

// Function: HandleCommand
void HandleCommand(undefined4 *param_1) {
  void *new_node_ptr; 
  int lookup_result; 
  
  switch(*param_1) {
  default:
    printf("Unsupported Command\n");
    printf("Command type: %x\n",*param_1); 
    printf("File name: %s\n",(char *)(param_1 + 1)); 
    printf("Data length: %u\n",param_1[0x11]); // Use %u for uint32_t
    if (param_1[0x12] != 0) {
      printf("Data: %s\n",(char *)(uintptr_t)param_1[0x12]); 
    }
    break;
  case 1: // REQUEST
    lookup_result = LookupNode((char *)(param_1 + 1));
    if (lookup_result == 0) {
      printf("Page not found: %s\n",(char *)(param_1 + 1));
    } else {
      // Assuming LookupNode returns a pointer (or an integer that can be cast to a pointer)
      // and that the offsets 0x4c and 0x48 are for data_ptr and data_len within that node struct.
      ServePage((void *)(uintptr_t)(*(undefined4 *)((char *)(uintptr_t)lookup_result + 0x4c)), // data_ptr
                *(undefined4 *)((char *)(uintptr_t)lookup_result + 0x48)); // data_len
    }
    break;
  case 2: // QUERY
    PrintTree((char *)(param_1 + 1));
    break;
  case 3: // SEND
    new_node_ptr = calloc(0x50,1); // Allocate 80 bytes for the new node
    VerifyPointerOrTerminate(new_node_ptr,"new_node during SEND"); // Corrected variable

    memcpy(new_node_ptr, (char *)(param_1 + 1), 0x40); // Copy 64 bytes for filename

    // Set data_ptr and data_len within the new node structure
    *(undefined4 *)((char *)new_node_ptr + 0x4c) = param_1[0x12]; // Set data_ptr
    *(undefined4 *)((char *)new_node_ptr + 0x48) = param_1[0x11]; // Set data_len

    lookup_result = InsertNodeInTree(new_node_ptr);
    if (lookup_result == 0) {
      // On success, ownership of data is transferred to the new node,
      // so clear command's data fields to prevent double freeing in DestroyCommand.
      param_1[0x12] = 0;
      param_1[0x11] = 0;
      printf("SUCCESS: Page uploaded to server\n");
    } else {
      free(new_node_ptr); // On error, free the newly allocated node
      printf("ERROR: Unable to upload page\n");
    }
    break;
  case 4: // REMOVE
    lookup_result = DeleteNode((char *)(param_1 + 1));
    if (lookup_result == 0) {
      printf("SUCCESS: Page deleted\n");
    } else {
      printf("ERROR: Unable to delete page\n");
    }
    break;
  case 5: // VISUALIZE
    ServePage((void *)(uintptr_t)param_1[0x12], param_1[0x11]);
    break;
  case 6: // INTERACT
    lookup_result = LookupNode((char *)(param_1 + 1));
    if (lookup_result == 0) {
      printf("Page not found: %s\n",(char *)(param_1 + 1));
    } else {
      InteractWithPage((void *)(uintptr_t)(*(undefined4 *)((char *)(uintptr_t)lookup_result + 0x4c)), // data_ptr
                       *(undefined4 *)((char *)(uintptr_t)lookup_result + 0x48), // data_len
                       (void *)(uintptr_t)param_1[0x12]); // command's data as input
    }
  }
}