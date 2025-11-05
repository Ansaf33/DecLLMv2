#include <stdio.h>   // For printf
#include <stdlib.h>  // For free, calloc, atoi, exit
#include <string.h>  // For memset, strncmp, strlen, strncpy, memcpy
#include <stdint.h>  // For uintptr_t

// Type definitions for common decompiler output types
typedef unsigned char byte;
typedef unsigned int uint;
// `undefined4` is often used for 4-byte values. In this context, it seems to imply `unsigned int`.
// For pointer storage, on 64-bit systems, `unsigned int` is too small.
// The original code uses `undefined4 *param_1` which means `param_1[0x12]` is an `undefined4`.
// When `param_1[0x12]` stores a `void*` from `calloc`, it implies `sizeof(undefined4) == sizeof(void*)`.
// This is true on 32-bit systems. To handle 64-bit gracefully while maintaining the original type logic,
// we'll use `uintptr_t` for casts to/from pointers when dealing with `unsigned int` fields that hold addresses.
// This indicates a potential architecture dependency from the original decompiled code.

// External function declarations - assuming their signatures based on usage
// These functions are not provided in the snippet, so we must declare them.
// Their exact return types and argument types are inferred.
extern unsigned int receive_fixed(void *buffer, unsigned int size);
extern unsigned int receive_until(void *buffer, char delimiter, unsigned int max_size);
extern void VerifyPointerOrTerminate(void *ptr, const char *msg); // Assumed to exit on failure
extern void *LookupNode(const char *name); // Assumed to return a pointer to a Node or NULL
extern void ServePage(void *data, unsigned int length);
extern void PrintTree(const char *name);
extern int InsertNodeInTree(void *node); // Assumed to return int (0 for success, non-zero for error)
extern int DeleteNode(const char *name); // Assumed to return int (0 for success, non-zero for error)
extern void InteractWithPage(void *data, unsigned int length, void *additional_data);

// Function: DecodeData
// dest: pointer to output buffer
// dest_len: total number of decoded bytes to produce (excluding the first byte handled before the loop)
// src: pointer to input buffer
// src_len: total number of encoded bytes (unused in the provided snippet's logic, but present in the call signature)
int DecodeData(byte *dest, int dest_len, byte *src, uint src_len) {
  int i;
  
  // First decoded byte: takes the upper 7 bits of the first source byte
  *dest = *src >> 1;
  dest++;

  // Loop for subsequent decoded bytes
  // `dest_len` here refers to the number of bytes to be written *in the loop*.
  // The call site passes `local_14` (total decoded bytes) as `dest_len`.
  // If `local_14` is the total, and one byte is handled above, then `local_14 - 1` bytes are left for the loop.
  // The original loop condition `local_10 <= param_2` implies `param_2` iterations.
  // If `param_2` is `local_14`, then `local_14` iterations. Total `local_14 + 1` bytes written.
  // This matches `calloc(local_14 + 1, 1)` in ReceiveCommand.
  for (i = 1; i <= dest_len; i++) {
    int k = i % 8;
    
    // Decode 7-bit value from current and next source bytes
    *dest = ((src[1] >> (k + 1)) | (*src << (7 - k))) & 0x7f;
    
    // Advance source pointer based on specific logic
    if (k != 7) {
      src++;
    }
    dest++;
  }
  return 0;
}

// Function: DestroyCommand
// cmd_ptr: pointer to the command structure
void DestroyCommand(unsigned int *cmd_ptr) {
  // cmd_ptr[0x12] is assumed to store a data pointer
  if (cmd_ptr[0x12] != 0) {
    free((void *)(uintptr_t)cmd_ptr[0x12]); // Convert stored unsigned int to pointer
    cmd_ptr[0x12] = 0;
  }
  cmd_ptr[0] = 0; // Clear command type
  // Clear the filename/name buffer (0x40 bytes starting from cmd_ptr + 1)
  memset(cmd_ptr + 1, 0, 0x40);
  cmd_ptr[0x11] = 0; // Clear data length
}

// Function: ReceiveCommand
// command: pointer to the command structure to populate
// direction_flag: pointer to an unsigned int to store the direction (1 for ACS+0.1, 0 for ACS-0.1)
int ReceiveCommand(unsigned int *command, unsigned int *direction_flag) {
  char temp_buf[64]; // Consolidated buffer for various receive operations
  unsigned int received_len;
  
  // If command already contains data, destroy it first
  if (command[0x12] != 0) {
    DestroyCommand(command);
  }

  // Receive initial command prefix (e.g., "ACS+0.1")
  received_len = receive_fixed(temp_buf, 7);
  if (received_len != 7) {
    return -1; // Indicate error with 0xffffffff
  }

  // Check command direction
  if (strncmp(temp_buf, "ACS+0.1", 7) == 0) {
    *direction_flag = 1;
  } else if (strncmp(temp_buf, "ACS-0.1", 7) == 0) {
    *direction_flag = 0;
  } else {
    return -2; // Indicate error with 0xfffffffe
  }

  // Receive command type string (e.g., "[REQUEST")
  received_len = receive_until(temp_buf, ':', 0x40);
  if (received_len == 0) {
    return -1; // Indicate error
  } else if (temp_buf[0] == '[') { // Command type must start with '['
    // Determine command type from the string
    // Using strlen for string literal length for clarity, compiler optimizes this to a constant.
    if (strncmp(temp_buf, "[REQUEST", strlen("[REQUEST")) == 0) {
      command[0] = 1;
    } else if (strncmp(temp_buf, "[QUERY", strlen("[QUERY")) == 0) {
      command[0] = 2;
    } else if (strncmp(temp_buf, "[SEND", strlen("[SEND")) == 0) {
      command[0] = 3;
    } else if (strncmp(temp_buf, "[REMOVE", strlen("[REMOVE")) == 0) {
      command[0] = 4;
    } else if (strncmp(temp_buf, "[VISUALIZE", strlen("[VISUALIZE")) == 0) {
      command[0] = 5;
    } else if (strncmp(temp_buf, "[INTERACT", strlen("[INTERACT")) == 0) {
      command[0] = 6;
    } else {
      return -2; // Unknown command type
    }

    // Receive file name
    received_len = receive_until(temp_buf, ':', 0x40);
    if (received_len >= 0x41) { // If received length exceeds buffer size (0x40) + 1 for null terminator
      return -1; // Error: filename too long
    }
    if (received_len != 0) {
      strncpy((char *)(command + 1), temp_buf, received_len);
      ((char *)(command + 1))[received_len] = '\0'; // Ensure null termination
    } else {
      ((char *)(command + 1))[0] = '\0'; // Empty filename
    }

    // Receive data length (as a string, then converted to int)
    received_len = receive_until(temp_buf, ':', 0x40);
    if (received_len < 7) { // This condition is unusual for a number, but follows original logic
      if (received_len != 0) {
        command[0x11] = atoi(temp_buf); // Convert string to integer
      } else {
        command[0x11] = 0; // Default length if string is empty
      }

      // Handle data based on the parsed length
      if ((int)command[0x11] < 1) { // If length is 0 or negative
        if (command[0x11] == (unsigned int)-1) { // Special value -1 indicates encoded data
          unsigned int encoded_data_len;
          received_len = receive_fixed(&encoded_data_len, 4); // Receive actual encoded data length
          if (received_len != 4) {
            return -1; // Error receiving encoded data length
          }

          int decoded_data_len = (int)(encoded_data_len * 8) / 7; // Calculate decoded length
          
          void *decoded_data_ptr = calloc(decoded_data_len + 1, 1); // Allocate buffer for decoded data
          VerifyPointerOrTerminate(decoded_data_ptr, "command->data during parsing");
          command[0x12] = (unsigned int)(uintptr_t)decoded_data_ptr; // Store pointer in command struct

          void *encoded_data_buffer = calloc(encoded_data_len + 1, 1); // Allocate buffer for encoded data
          VerifyPointerOrTerminate(encoded_data_buffer, "encoded_data during parsing");
          
          received_len = receive_fixed(encoded_data_buffer, encoded_data_len); // Receive encoded data
          if (received_len != encoded_data_len) {
            free(encoded_data_buffer);
            return -1; // Error receiving encoded data
          }
          
          // Decode the data
          if (DecodeData((byte*)decoded_data_ptr, decoded_data_len, (byte*)encoded_data_buffer, encoded_data_len) != 0) {
            free(encoded_data_buffer);
            return -1; // Error during decoding
          }

          // Adjust stored length if the decoded data is null-terminated
          if (((char *)decoded_data_ptr)[decoded_data_len - 1] == '\0') {
            command[0x11] = decoded_data_len - 1;
          } else {
            command[0x11] = decoded_data_len;
          }
          free(encoded_data_buffer); // Free temporary encoded data buffer

        } // If command[0x11] is 0, no data is expected, command[0x12] remains NULL (0).
      } else { // Positive data length: receive raw data
        void *data_ptr = calloc(command[0x11] + 1, 1); // Allocate buffer for raw data
        VerifyPointerOrTerminate(data_ptr, "command->data during parsing");
        command[0x12] = (unsigned int)(uintptr_t)data_ptr; // Store pointer
        
        received_len = receive_fixed(data_ptr, command[0x11]); // Receive raw data
        if (command[0x11] != received_len) {
          return -1; // Error receiving raw data
        }
      }

      // Receive final ']'
      char end_char;
      if (receive_fixed(&end_char, 1) != 1) {
        return -1; // Error receiving closing bracket
      }
      if (end_char == ']') {
        return 0; // Success
      } else {
        return -2; // Invalid closing bracket
      }
    } else {
      return -1; // Error: data length string too long
    }
  } else {
    return -2; // Error: command type not starting with '['
  }
}

// Function: HandleCommand
// command: pointer to the command structure
void HandleCommand(unsigned int *command) {
  void *new_node;    // For case 3 (SEND), represents a new node for the tree
  void *node_result; // For LookupNode result
  
  switch(command[0]) { // Switch on command type
  default:
    printf("Unsupported Command\n");
    printf("Command type: %x\n", command[0]); // Fixed format specifier
    printf("File name: %s\n", (char *)(command + 1)); // Fixed format specifier and added cast
    printf("Data length: %d\n", command[0x11]); // Fixed format specifier
    if (command[0x12] != 0) {
      printf("Data: %s\n", (char *)(uintptr_t)command[0x12]); // Fixed format specifier and added cast
    }
    break;
  case 1: // REQUEST command
    node_result = LookupNode((char *)(command + 1)); // Lookup node by filename
    if (node_result == NULL) { // Assuming 0 is NULL
      printf("Page not found: %s\n", (char *)(command + 1));
    } else {
      // Serve the page using data pointer and length from the looked-up node
      // Assuming node_result + 0x4c is data pointer, node_result + 0x48 is data length
      ServePage(*(void **)((char *)node_result + 0x4c), *(unsigned int *)((char *)node_result + 0x48));
    }
    break;
  case 2: // QUERY command
    PrintTree((char *)(command + 1)); // Print tree rooted at filename
    break;
  case 3: // SEND command
    new_node = calloc(0x50, 1); // Allocate memory for a new node (0x50 bytes)
    VerifyPointerOrTerminate(new_node, "new_node during SEND");
    
    // Copy filename/name part from command to new_node
    memcpy(new_node, (char *)(command + 1), 0x40);
    
    // Set data pointer and length fields in the new node
    *(void **)((char *)new_node + 0x4c) = (void *)(uintptr_t)command[0x12];
    *(unsigned int *)((char *)new_node + 0x48) = command[0x11];
    
    if (InsertNodeInTree(new_node) == 0) { // Attempt to insert the new node into the tree
      command[0x12] = 0; // On success, ownership of data transferred to the tree node
      command[0x11] = 0;
      printf("SUCCESS: Page uploaded to server\n");
    } else {
      // On failure, new_node is freed. The data it pointed to (command[0x12]) is still owned by command.
      free(new_node);
      printf("ERROR: Unable to upload page\n");
    }
    break;
  case 4: // REMOVE command
    if (DeleteNode((char *)(command + 1)) == 0) { // Attempt to delete node by filename
      printf("SUCCESS: Page deleted\n");
    } else {
      printf("ERROR: Unable to delete page\n");
    }
    break;
  case 5: // VISUALIZE command
    ServePage((void *)(uintptr_t)command[0x12], command[0x11]); // Serve page using command's data
    break;
  case 6: // INTERACT command
    node_result = LookupNode((char *)(command + 1)); // Lookup node by filename
    if (node_result == NULL) {
      printf("Page not found: %s\n", (char *)(command + 1));
    } else {
      // Interact with page using node's data and command's additional data
      InteractWithPage(*(void **)((char *)node_result + 0x4c), *(unsigned int *)((char *)node_result + 0x48), (void *)(uintptr_t)command[0x12]);
    }
    break;
  }
  return;
}