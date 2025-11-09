#include <stdio.h>    // For sprintf
#include <stdlib.h>   // For calloc, free
#include <string.h>   // For memset, memcpy
#include <stdint.h>   // For uint8_t, uint16_t, uint32_t

// --- External declarations (replace with actual definitions if available) ---
// Assuming debug_print takes standard format specifiers.
// Original uses '$d', changed to '%u' or '%d' as appropriate for integers.
void debug_print(const char *format, ...);

// These types are inferred from usage in the snippet.
// `data_ptr` is used as `undefined4 *` in msls_encrypt usage, so `uint32_t *` or `void *` for flexibility.
void msls_encrypt(void *data_ptr, uint16_t data_len, uint32_t param_2);
void msls_decrypt(void *data_ptr, uint16_t data_len, uint32_t param_2);
// `msg_ptr` is used as `undefined2 *` in msls_send_msg/destroy_msg usage, so `uint16_t *` or `void *` for flexibility.
void msls_send_msg(void *msg_ptr);
void msls_destroy_msg(void *msg_ptr);

// Global variables (mocked or actual, based on context)
// 0x40 is 64, 0x80 is 128.
uint8_t APPLICATION_MSG_BOARD[64][128];
int numMessages = 0; // Global counter for messages

// --- Function prototypes for internal functions ---
uint32_t post_new_message(const void *message_data);
uint32_t delete_message(uint8_t message_idx);
void clear_message_board(void);

// Function: clear_message_board
void clear_message_board(void) {
  memset(APPLICATION_MSG_BOARD, 0, sizeof(APPLICATION_MSG_BOARD));
  numMessages = 0;
}

// Function: delete_message
uint32_t delete_message(uint8_t message_idx) {
  // Ensure message_idx is within valid bounds and there are messages to delete
  if (message_idx < 64 && message_idx < numMessages) {
    // Shift messages up to fill the gap created by the deletion
    for (uint32_t i = message_idx; i < (uint32_t)(numMessages - 1); ++i) {
      memcpy(APPLICATION_MSG_BOARD[i], APPLICATION_MSG_BOARD[i + 1], 128);
    }
    numMessages--;
    // Clear the last message slot (which now contains a duplicate of the previous last message)
    memset(APPLICATION_MSG_BOARD[numMessages], 0, 128);
    return 1; // Success
  }
  return 0xffffffff; // Failure (invalid index)
}

// Function: post_new_message
uint32_t post_new_message(const void *message_data) {
  if (numMessages < 64) { // 64 is MAX_MESSAGES
    memcpy(APPLICATION_MSG_BOARD[numMessages], message_data, 128); // 128 is MSG_SIZE
    numMessages++;
    return 1; // Success
  }
  return 0xffffffff; // Failure (board is full)
}

// Function: msls_handle_application
void msls_handle_application(uint32_t param_1_unused, uint32_t param_2, const uint8_t *param_3) {
  // Extract fields from param_3 directly, reducing intermediate variables
  uint16_t app_msg_len_field = *(const uint16_t *)(param_3 + 0x10);
  debug_print("Handling Application Message (%u)\n", app_msg_len_field);

  if (2 < app_msg_len_field) {
    const uint8_t *app_msg_payload = *(const uint8_t **)(param_3 + 0x14);
    uint8_t msg_type = *app_msg_payload;
    uint16_t msg_len = *(const uint16_t *)(app_msg_payload + 1);

    debug_print("Type: %u Len: %u\n", msg_type, msg_len);

    // Variables for response message, declared here to reduce re-declaration
    uint8_t *out_msg_header = NULL;
    uint8_t *out_msg_body_header = NULL;
    char *out_msg_data = NULL;
    uint32_t return_val; // For post_new_message, delete_message return values

    switch(msg_type) {
    case 0xa0: // APP: List Board
      debug_print("APP: List Board\n");
      out_msg_header = (uint8_t *)calloc(1, 0x18);
      out_msg_body_header = (uint8_t *)calloc(1, 7);
      out_msg_data = (char *)calloc(1, 0x80);
      if (!out_msg_header) { free(out_msg_body_header); free(out_msg_data); return; }
      if (!out_msg_body_header) { free(out_msg_header); free(out_msg_data); return; }
      if (!out_msg_data) { free(out_msg_header); free(out_msg_body_header); return; }

      // Populate out_msg_header
      *(uint16_t *)(out_msg_header + 0) = 0x92;
      *(uint16_t *)(out_msg_header + 2) = 0xff01;
      *(uint8_t *)(out_msg_header + 4) = 4;
      *(uint32_t *)(out_msg_header + 12) = *(const uint32_t *)(param_3 + 0xc);
      *(uint16_t *)(out_msg_header + 16) = 0x83;
      *(uint8_t **)(out_msg_header + 20) = out_msg_body_header;

      // Populate out_msg_body_header
      *(uint8_t *)(out_msg_body_header + 0) = 0xaa;
      *(uint16_t *)(out_msg_body_header + 1) = 0x80;
      *(char **)(out_msg_body_header + 3) = out_msg_data;

      sprintf(out_msg_data, "%d of %d slots filled\n", numMessages, 64);
      msls_encrypt((uint32_t *)(out_msg_body_header + 3), *(uint16_t *)(out_msg_body_header + 1), param_2);
      msls_send_msg(out_msg_header);
      msls_destroy_msg(out_msg_header);
      break;

    case 0xa1: // APP: Post New Message
      if (0x7f < msg_len) {
        msls_decrypt(app_msg_payload + 3, msg_len, param_2);
        return_val = post_new_message(app_msg_payload + 3);

        debug_print("Posting to slot %u\n", numMessages);
        out_msg_header = (uint8_t *)calloc(1, 0x18);
        out_msg_body_header = (uint8_t *)calloc(1, 7);
        out_msg_data = (char *)calloc(1, 0x80);
        if (!out_msg_header) { free(out_msg_body_header); free(out_msg_data); return; }
        if (!out_msg_body_header) { free(out_msg_header); free(out_msg_data); return; }
        if (!out_msg_data) { free(out_msg_header); free(out_msg_body_header); return; }

        // Populate out_msg_header
        *(uint16_t *)(out_msg_header + 0) = 0x92;
        *(uint16_t *)(out_msg_header + 2) = 0xff01;
        *(uint8_t *)(out_msg_header + 4) = 4;
        *(uint32_t *)(out_msg_header + 12) = *(const uint32_t *)(param_3 + 0xc);
        *(uint16_t *)(out_msg_header + 16) = 0x83;
        *(uint8_t **)(out_msg_header + 20) = out_msg_body_header;

        // Populate out_msg_body_header
        *(uint8_t *)(out_msg_body_header + 0) = 0xaa;
        *(uint16_t *)(out_msg_body_header + 1) = 0x80;
        *(char **)(out_msg_body_header + 3) = out_msg_data;

        sprintf(out_msg_data, "POST returned: %u\n", return_val);
        msls_encrypt((uint32_t *)(out_msg_body_header + 3), *(uint16_t *)(out_msg_body_header + 1), param_2);
        msls_send_msg(out_msg_header);
        msls_destroy_msg(out_msg_header);
      }
      break;

    case 0xa2: // APP: Delete Message
      if (msg_len != 0) {
        msls_decrypt(app_msg_payload + 3, msg_len, param_2);
        uint8_t delete_idx = app_msg_payload[3];
        return_val = delete_message(delete_idx);

        debug_print("Deleting message %u\n", delete_idx);
        out_msg_header = (uint8_t *)calloc(1, 0x18);
        out_msg_body_header = (uint8_t *)calloc(1, 7);
        out_msg_data = (char *)calloc(1, 0x80);
        if (!out_msg_header) { free(out_msg_body_header); free(out_msg_data); return; }
        if (!out_msg_body_header) { free(out_msg_header); free(out_msg_data); return; }
        if (!out_msg_data) { free(out_msg_header); free(out_msg_body_header); return; }

        // Populate out_msg_header
        *(uint16_t *)(out_msg_header + 0) = 0x92;
        *(uint16_t *)(out_msg_header + 2) = 0xff01;
        *(uint8_t *)(out_msg_header + 4) = 4;
        *(uint32_t *)(out_msg_header + 12) = *(const uint32_t *)(param_3 + 0xc);
        *(uint16_t *)(out_msg_header + 16) = 0x83;
        *(uint8_t **)(out_msg_header + 20) = out_msg_body_header;

        // Populate out_msg_body_header
        *(uint8_t *)(out_msg_body_header + 0) = 0xaa;
        *(uint16_t *)(out_msg_body_header + 1) = 0x80;
        *(char **)(out_msg_body_header + 3) = out_msg_data;

        sprintf(out_msg_data, "DELETE returned: %u\n", return_val);
        msls_encrypt((uint32_t *)(out_msg_body_header + 3), *(uint16_t *)(out_msg_body_header + 1), param_2);
        msls_send_msg(out_msg_header);
        msls_destroy_msg(out_msg_header);
      }
      break;

    case 0xa3: // APP: Clear Board
      clear_message_board();
      out_msg_header = (uint8_t *)calloc(1, 0x18);
      out_msg_body_header = (uint8_t *)calloc(1, 7);
      out_msg_data = (char *)calloc(1, 0x80);
      if (!out_msg_header) { free(out_msg_body_header); free(out_msg_data); return; }
      if (!out_msg_body_header) { free(out_msg_header); free(out_msg_data); return; }
      if (!out_msg_data) { free(out_msg_header); free(out_msg_body_header); return; }

      // Populate out_msg_header
      *(uint16_t *)(out_msg_header + 0) = 0x92;
      *(uint16_t *)(out_msg_header + 2) = 0xff01;
      *(uint8_t *)(out_msg_header + 4) = 4;
      *(uint32_t *)(out_msg_header + 12) = *(const uint32_t *)(param_3 + 0xc);
      *(uint16_t *)(out_msg_header + 16) = 0x83;
      *(uint8_t **)(out_msg_header + 20) = out_msg_body_header;

      // Populate out_msg_body_header
      *(uint8_t *)(out_msg_body_header + 0) = 0xaa;
      *(uint16_t *)(out_msg_body_header + 1) = 0x80;
      *(char **)(out_msg_body_header + 3) = out_msg_data;

      sprintf(out_msg_data, "Cleared Board\n");
      msls_encrypt((uint32_t *)(out_msg_body_header + 3), *(uint16_t *)(out_msg_body_header + 1), param_2);
      msls_send_msg(out_msg_header);
      msls_destroy_msg(out_msg_header);
      break;

    case 0xa4: // APP: Read Slot
      if (msg_len != 0) {
        msls_decrypt(app_msg_payload + 3, msg_len, param_2);
        uint8_t read_idx = app_msg_payload[3];
        debug_print("Reading slot %u\n", read_idx);

        out_msg_header = (uint8_t *)calloc(1, 0x18);
        out_msg_body_header = (uint8_t *)calloc(1, 7);
        out_msg_data = (char *)calloc(1, 0x80);
        if (!out_msg_header) { free(out_msg_body_header); free(out_msg_data); return; }
        if (!out_msg_body_header) { free(out_msg_header); free(out_msg_data); return; }
        if (!out_msg_data) { free(out_msg_header); free(out_msg_body_header); return; }

        // Populate out_msg_header
        *(uint16_t *)(out_msg_header + 0) = 0x92;
        *(uint16_t *)(out_msg_header + 2) = 0xff01;
        *(uint8_t *)(out_msg_header + 4) = 4;
        *(uint32_t *)(out_msg_header + 12) = *(const uint32_t *)(param_3 + 0xc);
        *(uint16_t *)(out_msg_header + 16) = 0x83;
        *(uint8_t **)(out_msg_header + 20) = out_msg_body_header;

        // Populate out_msg_body_header
        *(uint8_t *)(out_msg_body_header + 0) = 0xaa;
        *(uint16_t *)(out_msg_body_header + 1) = 0x80;
        *(char **)(out_msg_body_header + 3) = out_msg_data;

        if (read_idx < 64) { // 64 is MAX_MESSAGES
          memcpy(out_msg_data, APPLICATION_MSG_BOARD[read_idx], 128);
        } else {
          sprintf(out_msg_data, "INVALID MESSAGE");
        }
        msls_encrypt((uint32_t *)(out_msg_body_header + 3), *(uint16_t *)(out_msg_body_header + 1), param_2);
        msls_send_msg(out_msg_header);
        msls_destroy_msg(out_msg_header);
      }
      break;
    }
  }
  return;
}