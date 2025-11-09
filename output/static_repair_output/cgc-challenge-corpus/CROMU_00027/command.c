#include <stdlib.h> // For calloc, free, atoi
#include <string.h> // For memset, strncmp, strlen, strncpy, memcpy
#include <stdio.h>  // For printf
#include <stdint.h> // For uint8_t, uint32_t, int32_t, uintptr_t

// Define custom types based on common patterns in such snippets
typedef uint8_t byte;
typedef uint32_t undefined4;

// External function declarations (assuming these exist in other compilation units)
// The specific signatures are inferred from usage in the provided code.
extern uint32_t receive_fixed(void *buffer, uint32_t length);
extern uint32_t receive_until(char *buffer, char delimiter, uint32_t max_len);
extern void VerifyPointerOrTerminate(void *ptr, const char *msg);
extern int LookupNode(const char *filename); // Assuming it takes filename string
extern void ServePage(undefined4 data_ptr, undefined4 data_len);
extern void PrintTree(const char *path); // Assuming it takes a path string
extern int InsertNodeInTree(void *new_node); // Assuming it takes a pointer to a node structure
extern int DeleteNode(const char *filename); // Assuming it takes filename string
extern void InteractWithPage(undefined4 data_ptr, undefined4 data_len, undefined4 interaction_data);


// Function: DecodeData
undefined4 DecodeData(byte *dest, int count, byte *src) {
  // The first byte is a special case, decoded from src[0]
  *dest++ = *src >> 1;

  for (int i = 1; i <= count; i++) {
    int bit_offset_in_group = i % 8;

    // This logic combines bits from the current and next source bytes.
    // The `& 0x1f` in the original shifts are often for robustness against
    // undefined behavior if shift amount exceeds bit width, though for these
    // small offsets, it's not strictly necessary for correctness on typical systems.
    uint8_t val1 = src[1] >> (bit_offset_in_group + 1);
    uint8_t val2 = *src << (7 - bit_offset_in_group);

    *dest++ = (val1 | val2) & 0x7f; // Combine and mask to 7 bits

    // Advance source pointer based on `bit_offset_in_group`.
    // It advances for bit_offset_in_group = 0, 1, ..., 6.
    // It does NOT advance for bit_offset_in_group = 7.
    if (bit_offset_in_group != 7) {
      src++;
    }
  }
  return 0;
}

// Function: DestroyCommand
void DestroyCommand(undefined4 *command_ptr) {
  // param_1[0x12] is likely a pointer to command data.
  if (command_ptr[0x12] != 0) {
    free((void *)(uintptr_t)command_ptr[0x12]);
    command_ptr[0x12] = 0;
  }
  // param_1[0] is command type.
  command_ptr[0] = 0;
  // param_1 + 1 is likely the filename field, 0x40 bytes long.
  memset(command_ptr + 1, 0, 0x40);
  // param_1[0x11] is data length.
  command_ptr[0x11] = 0;
}

// Function: ReceiveCommand
undefined4 ReceiveCommand(undefined4 *command_ptr, undefined4 *status_ptr) {
  uint32_t received_len;
  char buffer[64]; // General purpose buffer for strings (ACS, command type, filename, length)
  char single_char_buffer; // For single character reads like ']'

  if (command_ptr[0x12] != 0) {
    DestroyCommand(command_ptr);
  }

  // Receive "ACS+/-0.1" (7 bytes)
  received_len = receive_fixed(buffer, 7);
  if (received_len != 7) {
    return 0xffffffff; // Error: incomplete read
  }

  if (strncmp(buffer, "ACS+0.1", 7) == 0) {
    *status_ptr = 1;
  } else if (strncmp(buffer, "ACS-0.1", 7) == 0) {
    *status_ptr = 0;
  } else {
    return 0xfffffffe; // Error: unknown ACS status string
  }

  // Receive command type (e.g., "[REQUEST")
  // The original code had a bug using a single char for `receive_until`.
  // `buffer` is used to store the received string.
  received_len = receive_until(buffer, ':', sizeof(buffer) - 1);
  if (received_len == 0) {
    return 0xffffffff; // Error: no command type received
  }
  buffer[received_len] = '\0'; // Null-terminate the received string

  if (buffer[0] == '[') {
    char *command_keyword = buffer + 1; // Point past '['
    size_t keyword_len = received_len - 1;

    // Determine command type based on keyword
    if (strncmp(command_keyword, "REQUEST", 7) == 0 && keyword_len == 7) {
      command_ptr[0] = 1;
    } else if (strncmp(command_keyword, "QUERY", 5) == 0 && keyword_len == 5) {
      command_ptr[0] = 2;
    } else if (strncmp(command_keyword, "SEND", 4) == 0 && keyword_len == 4) {
      command_ptr[0] = 3;
    } else if (strncmp(command_keyword, "REMOVE", 6) == 0 && keyword_len == 6) {
      command_ptr[0] = 4;
    } else if (strncmp(command_keyword, "VISUALIZE", 9) == 0 && keyword_len == 9) {
      command_ptr[0] = 5;
    } else if (strncmp(command_keyword, "INTERACT", 8) == 0 && keyword_len == 8) {
      command_ptr[0] = 6;
    } else {
      return 0xfffffffe; // Error: unknown command keyword
    }

    // Receive filename
    received_len = receive_until(buffer, ':', sizeof(buffer) - 1);
    if (received_len >= sizeof(buffer)) { // Check for buffer overflow
      return 0xffffffff; // Error: filename too long
    }
    strncpy((char *)(command_ptr + 1), buffer, received_len);
    ((char *)(command_ptr + 1))[received_len] = '\0'; // Null-terminate the filename field

    // Receive data length (as string)
    received_len = receive_until(buffer, ':', sizeof(buffer) - 1);
    if (received_len >= sizeof(buffer)) {
      return 0xffffffff; // Error: data length string too long
    }
    buffer[received_len] = '\0'; // Null-terminate

    if (received_len != 0) { // If a number string was provided
      command_ptr[0x11] = atoi(buffer); // Convert string to integer
    } else { // Empty string for length, `atoi("")` returns 0.
      command_ptr[0x11] = 0;
    }

    // Handle data based on command_ptr[0x11] (data_length)
    if ((int32_t)command_ptr[0x11] < 1) {
      if ((int32_t)command_ptr[0x11] == -1) { // Special case: encoded data
        uint32_t encoded_data_len;
        received_len = receive_fixed(&encoded_data_len, 4); // Read actual encoded data length (4 bytes)
        if (received_len != 4) {
          return 0xffffffff; // Error: incomplete read of encoded data length
        }

        int32_t decoded_data_len = (encoded_data_len * 8) / 7;
        void *decoded_data_buffer = calloc(decoded_data_len + 1, 1);
        command_ptr[0x12] = (uint32_t)(uintptr_t)decoded_data_buffer; // Store pointer to decoded data
        VerifyPointerOrTerminate(decoded_data_buffer, "command->data during parsing");

        void *temp_encoded_data = calloc(encoded_data_len + 1, 1);
        VerifyPointerOrTerminate(temp_encoded_data, "encoded_data during parsing");

        received_len = receive_fixed(temp_encoded_data, encoded_data_len);
        if (received_len != encoded_data_len) {
          free(temp_encoded_data);
          return 0xffffffff; // Error: incomplete read of encoded data
        }

        // Fix: Removed extra `local_60` parameter from DecodeData call.
        if (DecodeData((byte *)decoded_data_buffer, decoded_data_len, (byte *)temp_encoded_data) != 0) {
          free(temp_encoded_data);
          return 0xffffffff; // Error: decoding failed
        }

        // Adjust actual length if the last character of decoded data is null
        if (*(char *)((byte *)decoded_data_buffer + decoded_data_len - 1) == '\0') {
          command_ptr[0x11] = decoded_data_len - 1;
        } else {
          command_ptr[0x11] = decoded_data_len;
        }
        free(temp_encoded_data); // Free temporary buffer for encoded data

      } else { // command_ptr[0x11] is 0 or other negative (not -1)
        // No data to receive for length 0 or invalid negative lengths.
        // Ensure command_ptr[0x12] is NULL if no data is expected.
        if (command_ptr[0x12] != 0) {
            free((void*)(uintptr_t)command_ptr[0x12]);
            command_ptr[0x12] = 0;
        }
      }
    } else { // command_ptr[0x11] >= 1 (direct data)
      void *data_buffer = calloc(command_ptr[0x11] + 1, 1);
      command_ptr[0x12] = (uint32_t)(uintptr_t)data_buffer; // Store pointer to data
      VerifyPointerOrTerminate(data_buffer, "command->data during parsing");

      received_len = receive_fixed(data_buffer, command_ptr[0x11]);
      if (command_ptr[0x11] != received_len) {
        return 0xffffffff; // Error: incomplete read of direct data
      }
      // Data buffer is already null-terminated by calloc at `command_ptr[0x11]` + 1.
    }

    // Receive closing ']'
    received_len = receive_fixed(&single_char_buffer, 1);
    if (received_len == 1) {
      return (single_char_buffer == ']') ? 0 : 0xfffffffe; // 0 for success, -2 for expected ']'
    } else {
      return 0xffffffff; // Error: read error for ']'
    }
  } else {
    return 0xfffffffe; // Error: command type did not start with '['
  }
}

// Function: HandleCommand
void HandleCommand(undefined4 *command_ptr) {
  // command_ptr[0] is the command type.
  switch (command_ptr[0]) {
  default:
    printf("Unsupported Command\n");
    printf("Command type: 0x%x\n", command_ptr[0]); // Fixed format specifier
    printf("File name: %s\n", (char *)(command_ptr + 1)); // Fixed format specifier
    printf("Data length: %d\n", (int32_t)command_ptr[0x11]); // Fixed format specifier
    if (command_ptr[0x12] != 0) {
      printf("Data: %s\n", (char *)(uintptr_t)command_ptr[0x12]); // Fixed format specifier, cast to char*
    }
    break;
  case 1: // REQUEST
    // command_ptr + 1 is the filename string
    int node_ptr_val = LookupNode((const char *)(command_ptr + 1));
    if (node_ptr_val == 0) {
      printf("Page not found: %s\n", (char *)(command_ptr + 1));
    } else {
      // Assuming LookupNode returns a pointer to a struct, and 0x4c/0x48 are offsets within it.
      // Cast to uintptr_t for pointer arithmetic before dereferencing.
      ServePage(*(undefined4 *)((uintptr_t)node_ptr_val + 0x4c), *(undefined4 *)((uintptr_t)node_ptr_val + 0x48));
    }
    break;
  case 2: // QUERY
    // command_ptr + 1 is the filename/path for the tree query
    PrintTree((const char *)(command_ptr + 1));
    break;
  case 3: // SEND
    void *new_node = calloc(0x50, 1); // Allocate space for a new node structure
    VerifyPointerOrTerminate(new_node, "new_node during SEND");
    memcpy(new_node, command_ptr + 1, 0x40); // Copy filename from command to new_node
    *(undefined4 *)((uintptr_t)new_node + 0x4c) = command_ptr[0x12]; // Set data pointer
    *(undefined4 *)((uintptr_t)new_node + 0x48) = command_ptr[0x11]; // Set data length
    int insert_result = InsertNodeInTree(new_node);
    if (insert_result == 0) { // Success
      // If data was successfully uploaded, the command no longer owns the data.
      command_ptr[0x12] = 0;
      command_ptr[0x11] = 0;
      printf("SUCCESS: Page uploaded to server\n");
    } else { // Error
      // If insertion failed, the new_node and its associated data (if any) need to be freed.
      // Assuming InsertNodeInTree does not free data on failure.
      free(new_node);
      printf("ERROR: Unable to upload page\n");
    }
    break;
  case 4: // REMOVE
    int delete_result = DeleteNode((const char *)(command_ptr + 1)); // Filename
    if (delete_result == 0) {
      printf("SUCCESS: Page deleted\n");
    } else {
      printf("ERROR: Unable to delete page\n");
    }
    break;
  case 5: // VISUALIZE
    // command_ptr[0x12] is data pointer, command_ptr[0x11] is data length.
    ServePage(command_ptr[0x12], command_ptr[0x11]);
    break;
  case 6: // INTERACT
    node_ptr_val = LookupNode((const char *)(command_ptr + 1)); // Filename
    if (node_ptr_val == 0) {
      printf("Page not found: %s\n", (char *)(command_ptr + 1));
    } else {
      // Assuming LookupNode returns a pointer to a struct.
      // command_ptr[0x12] is the interaction data.
      InteractWithPage(*(undefined4 *)((uintptr_t)node_ptr_val + 0x4c), *(undefined4 *)((uintptr_t)node_ptr_val + 0x48), command_ptr[0x12]);
    }
    break;
  }
}