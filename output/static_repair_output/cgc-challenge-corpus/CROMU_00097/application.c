#include <stdio.h>   // For printf, sprintf, vsnprintf
#include <stdlib.h>  // For calloc, free
#include <string.h>  // For memset, memcpy
#include <stdint.h>  // For uint32_t, uint16_t, uint8_t
#include <stddef.h>  // For size_t
#include <stdarg.h>  // For va_list, va_start, va_end

// --- Dummy declarations for missing functions/globals ---
// These are placeholders to make the code compilable.
// In a real application, these would be defined elsewhere.

// A simple debug print function, acts like printf
void debug_print(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

// Dummy MSLS functions
void msls_encrypt(uint32_t data_ptr_val, uint16_t len, uint32_t key) {
    (void)data_ptr_val; // Suppress unused parameter warning
    (void)len;          // Suppress unused parameter warning
    (void)key;          // Suppress unused parameter warning
    // In a real implementation, this would encrypt the data at `(char*)(uintptr_t)data_ptr_val`
}

void msls_send_msg(void* msg) {
    (void)msg; // Suppress unused parameter warning
    // In a real implementation, this would send the message.
    // The message is freed by msls_destroy_msg, so no free here.
}

void msls_decrypt(uint8_t* data, uint16_t len, uint32_t key) {
    (void)data; // Suppress unused parameter warning
    (void)len;  // Suppress unused parameter warning
    (void)key;  // Suppress unused parameter warning
    // In a real implementation, this would decrypt the data.
}

// Global variables
int numMessages = 0;
char APPLICATION_MSG_BOARD[0x40 * 0x80]; // 64 messages, each 128 bytes

// --- Original functions (fixed) ---

// Function: clear_message_board
void clear_message_board(void) {
  memset(APPLICATION_MSG_BOARD, 0, sizeof(APPLICATION_MSG_BOARD));
  numMessages = 0;
}

// Function: delete_message
uint32_t delete_message(uint8_t param_1) {
  if (param_1 < 0x40) { // Max 64 messages (0 to 63)
    // Clear the message slot
    memset(APPLICATION_MSG_BOARD + (uint32_t)param_1 * 0x80, 0, 0x80);
    // Shift subsequent messages up
    for (uint32_t i = (uint32_t)param_1; i < 0x3f; i++) { // Iterate up to 62, copying from i+1 to i
      memcpy(APPLICATION_MSG_BOARD + i * 0x80, APPLICATION_MSG_BOARD + (i + 1) * 0x80, 0x80);
    }
    // Clear the last slot (which now contains a duplicate)
    memset(APPLICATION_MSG_BOARD + 0x3f * 0x80, 0, 0x80);
    numMessages--;
    return 1;
  }
  return 0xffffffff; // Error code for invalid index
}

// Function: post_new_message
uint32_t post_new_message(void *param_1) {
  if (numMessages < 0x40) { // Max 64 messages
    memcpy(APPLICATION_MSG_BOARD + numMessages * 0x80, param_1, 0x80);
    numMessages++;
    return 1;
  }
  return 0xffffffff; // Error code for board full
}

// Function: msls_destroy_msg
// This function is responsible for freeing all dynamically allocated parts of a message.
void msls_destroy_msg(void* msg_hdr_ptr) {
    if (msg_hdr_ptr == NULL) {
        return;
    }

    uint16_t* msg_hdr = (uint16_t*)msg_hdr_ptr;
    // The payload pointer is stored at offset 20 bytes (puVar4 + 10 as undefined**)
    uint8_t* msg_payload = *((uint8_t**)((uint8_t*)msg_hdr + 20));

    if (msg_payload != NULL) {
        // The text buffer pointer is stored at offset 3 bytes within the payload (puVar5 + 3 as char**)
        // Assuming 32-bit pointer storage as per original decompiled code
        char* msg_text_buffer = (char*)(uintptr_t)(*((uint32_t*)(msg_payload + 3)));
        free(msg_text_buffer);
        free(msg_payload);
    }
    free(msg_hdr);
}

// Helper function to consolidate common message response logic
// This function constructs a standard MSLS response message, formats its text payload,
// encrypts it, sends it, and then frees its resources.
void send_msls_response(void* param_3_ptr, uint32_t param_2_key, const char* format, ...) {
    // Allocate memory for message header, payload structure, and text buffer
    // Original code pattern `calloc(SIZE, uVar1)` was suspicious.
    // Assuming `calloc(1, SIZE)` for a single instance of each structure.
    uint16_t* msg_hdr = (uint16_t*)calloc(1, 0x18); // 24 bytes for header structure
    uint8_t* msg_payload = (uint8_t*)calloc(1, 7);   // 7 bytes for payload structure
    char* msg_text_buffer = (char*)calloc(1, 0x80);  // 128 bytes for text content

    // Basic error handling for memory allocation
    if (!msg_hdr || !msg_payload || !msg_text_buffer) {
        fprintf(stderr, "Error: Failed to allocate memory for MSLS response.\n");
        free(msg_hdr); // Safe to call free on NULL
        free(msg_payload);
        free(msg_text_buffer);
        return;
    }

    // Populate msg_hdr (structure at 0x18 bytes)
    // Decompiler's interpretation of undefined2* puVar4 and its offsets:
    // *(uint16_t*)((uint8_t*)msg_hdr + 0) = 0x92; // *puVar4 = 0x92
    // *(uint16_t*)((uint8_t*)msg_hdr + 2) = 0xff01; // puVar4[1] = 0xff01
    // *(uint8_t*)((uint8_t*)msg_hdr + 4) = 4; // *(undefined *)(puVar4 + 2) = 4
    // *(uint32_t*)((uint8_t*)msg_hdr + 12) = *(uint32_t*)((uint8_t*)param_3_ptr + 0xc); // *(undefined4 *)(puVar4 + 6)
    // *(uint16_t*)((uint8_t*)msg_hdr + 16) = 0x83; // puVar4[8] = 0x83
    // *(uint8_t**)((uint8_t*)msg_hdr + 20) = msg_payload; // *(undefined **)(puVar4 + 10) = puVar5

    msg_hdr[0] = 0x92;
    msg_hdr[1] = 0xff01;
    *((uint8_t*)msg_hdr + 4) = 4;
    *((uint32_t*)((uint8_t*)msg_hdr + 12)) = *(uint32_t*)((uint8_t*)param_3_ptr + 0xc);
    *((uint16_t*)((uint8_t*)msg_hdr + 16)) = 0x83;
    *((uint8_t**)((uint8_t*)msg_hdr + 20)) = msg_payload;

    // Populate msg_payload (structure at 7 bytes)
    // Decompiler's interpretation of undefined* puVar5 and its offsets:
    // *(uint8_t*)((uint8_t*)msg_payload + 0) = 0xaa; // *puVar5 = 0xaa
    // *(uint16_t*)((uint8_t*)msg_payload + 1) = 0x80; // *(undefined2 *)(puVar5 + 1) = 0x80
    // *(uint32_t*)((uint8_t*)msg_payload + 3) = (uint32_t)(uintptr_t)msg_text_buffer; // *(char **)(puVar5 + 3) = pcVar6 (assuming 32-bit pointer storage)

    msg_payload[0] = 0xaa;
    *((uint16_t*)(msg_payload + 1)) = 0x80;
    *((uint32_t*)(msg_payload + 3)) = (uint32_t)(uintptr_t)msg_text_buffer; // Store pointer as 32-bit value

    // Format the text buffer using vsnprintf for safety
    va_list args;
    va_start(args, format);
    vsnprintf(msg_text_buffer, 0x80, format, args);
    va_end(args);

    // Encrypt and send the message
    msls_encrypt(*((uint32_t*)(msg_payload + 3)), *((uint16_t*)(msg_payload + 1)), param_2_key);
    msls_send_msg(msg_hdr);

    // Destroy/free the message resources after sending
    msls_destroy_msg(msg_hdr);
}

// Function: msls_handle_application
void msls_handle_application(uint32_t param_1, uint32_t param_2, void* param_3) {
  // Use explicit type casting for pointer arithmetic to avoid compiler warnings
  // and ensure correct byte offsets.
  uint16_t app_msg_len = *(uint16_t*)((uint8_t*)param_3 + 0x10);

  debug_print("Handling Application Message (%d)\n", app_msg_len);

  if (2 < app_msg_len) {
    uint8_t *app_data_ptr = *(uint8_t**)((uint8_t*)param_3 + 0x14);
    uint16_t data_len = *(uint16_t*)(app_data_ptr + 1);
    uint8_t data_type = *app_data_ptr;

    debug_print("Type: %d Len: %d\n", (uint32_t)data_type, (uint32_t)data_len);

    switch(data_type) {
    case 0xa0: // APP: List Board
      debug_print("APP: List Board\n");
      send_msls_response(param_3, param_2, "%d of %d slots filled\n", numMessages, 0x40);
      break;

    case 0xa1: // APP: Post New Message
      if (0x7f < data_len) {
        msls_decrypt(app_data_ptr + 3, data_len, param_2);
        uint32_t post_result = post_new_message(app_data_ptr + 3);
        debug_print("Posting to slot %d\n", numMessages - 1); // Fixed missing argument
        send_msls_response(param_3, param_2, "POST returned: %d\n", post_result);
      }
      break;

    case 0xa2: // APP: Delete Message
      if (data_len != 0) {
        msls_decrypt(app_data_ptr + 3, data_len, param_2);
        uint8_t delete_idx = app_data_ptr[3]; // The message index to delete
        uint32_t delete_result = delete_message(delete_idx);
        debug_print("Deleting message %d\n", (uint32_t)delete_idx); // Fixed missing argument
        send_msls_response(param_3, param_2, "DELETE returned: %d\n", delete_result);
      }
      break;

    case 0xa3: // APP: Clear Message Board
      clear_message_board();
      send_msls_response(param_3, param_2, "Cleared Board\n"); // Original sprintf had extra unused args
      break;

    case 0xa4: // APP: Read Message
      if (data_len != 0) {
        msls_decrypt(app_data_ptr + 3, data_len, param_2);
        uint8_t read_idx = app_data_ptr[3]; // The message index to read
        debug_print("Reading slot %d\n", (uint32_t)read_idx); // Fixed missing argument

        char response_content[0x80]; // Buffer for the message content or error
        if (read_idx < 0x40) {
          memcpy(response_content, APPLICATION_MSG_BOARD + (uint32_t)read_idx * 0x80, 0x80);
        } else {
          sprintf(response_content, "INVALID MESSAGE"); // Safe use of sprintf for fixed string
        }
        send_msls_response(param_3, param_2, "%s", response_content);
      }
      break;
    }
  }
}