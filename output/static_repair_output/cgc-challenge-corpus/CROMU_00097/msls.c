#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

// Decompiler-derived types
typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned char byte;
typedef unsigned char undefined;
typedef unsigned short undefined2;
typedef unsigned int undefined4;

// Generic function pointer type for 'code'.
// Functions in the array have varying signatures, requiring explicit casts at call sites.
typedef void (*code)();

// --- Inferred Structures ---

// MSLS_MSG structure based on usage across various functions
// Total size is 0x18 (24 bytes). Using packed attribute for byte-level offset accuracy.
typedef struct __attribute__((packed)) {
    ushort length;     // 0x00
    ushort field_2;    // 0x02
    byte   msg_type;   // 0x04
    byte   padding_05; // 0x05 (likely padding or another byte field)
    undefined4 field_8; // 0x08 (checksum in send_msg, compared in receive_msg)
    undefined4 field_C; // 0x0C
    ushort data_len;   // 0x10
    undefined2 padding_12; // 0x12 (padding to align data_ptr)
    void *data_ptr;    // 0x14
} MSLS_MSG;

// MSLS_CONTEXT structure (param_1 in many functions)
// Inferred from msls_cleanup, msls_receive_msg, msls_send_msg
typedef struct {
    // ... other fields potentially before 0x04 ...
    undefined4 padding_00; // Example padding if there are fields before sls_functions
    code **sls_functions; // 0x04 (Pointer to array of function pointers)
    undefined4 contexts[0x20]; // 0x08 (array of 32 contexts, 0x20 * 4 = 128 bytes)
    // ... other fields up to 0x88 ...
    void *field_88; // 0x88
} MslsContext;

// Structures for message types 3, 4, 5 data_ptr
// Based on msls_send_msg and msls_destroy_msg
typedef struct __attribute__((packed)) {
    undefined4 field_0;
    undefined4 field_4;
    void *ptr_at_C;     // 0x0C (from *(int *)(iVar1 + 0xc))
    ushort len_at_10;   // 0x10 (from *(short *)(iVar1 + 10))
} MsgType3Data; // Minimum size 0x12 (18 bytes)

typedef struct __attribute__((packed)) {
    byte field_0;
    ushort len_at_1;   // 0x01 (from *(short *)(iVar1 + 1))
    void *ptr_at_3;    // 0x03 (from *(int *)(iVar1 + 3))
} MsgType4Data; // Minimum size 0x07 (7 bytes, assuming 4-byte pointer)

// ErrorData structure from msls_send_error_msg
typedef struct __attribute__((packed)) {
    ushort code;
    undefined sub_code;
    undefined padding; // Padding to make it 4 bytes
} ErrorData; // Total 4 bytes

// --- Global Variables ---
#define RANDOM_POOL_SIZE 256 // Example size for the random pool
static uint random_pool[RANDOM_POOL_SIZE]; // Example static array for random numbers
static uint *DAT_4347c800_ptr = random_pool; // Pointer to the start of the random pool
static uint *pRandom = random_pool; // Current pointer in the random pool

// --- Function Prototypes ---
uint get_random(void);
code **msls_get_sls_functions(void);
ushort *msls_receive_msg(int param_1);
undefined4 msls_send_msg(int param_1);
void msls_cleanup(int param_1);
bool msls_version_check(short param_1);
void msls_destroy_msg(void *param_1);
void msls_encrypt(int param_1, uint param_2, int param_3);
void msls_decrypt(int param_1, uint param_2, int param_3);
void msls_send_error_msg(ushort param_1, undefined param_2);
uint msls_generate_checksum(int param_1);
uint msls_generate_checksum_outgoing(int param_1);

// Stub implementations for missing external functions
void debug_print(const char *format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, "DEBUG: ");
    vfprintf(stderr, format, args);
    va_end(args);
}

void receive_fixed(void *buf, size_t len) {
    if (buf && len > 0) {
        // In a real scenario, this would read from a socket or file
        // For compilation, we just do nothing or fill with dummy data
        // memset(buf, 0xAA, len); // Example dummy data
    }
    // debug_print("receive_fixed: %zu bytes\n", len); // Uncomment for more verbose debug
}

void transmit_fixed(const void *buf, size_t len) {
    // In a real scenario, this would write to a socket or file
    // For compilation, we just do nothing
    // debug_print("transmit_fixed: %zu bytes\n", len); // Uncomment for more verbose debug
}

void destroy_context(undefined4 ctx) {
    debug_print("destroy_context called for %x\n", ctx);
}

// Function pointers in msls_get_sls_functions need to match these signatures
void msls_handle_heartbeat(void) { debug_print("msls_handle_heartbeat called\n"); }
void msls_handle_handshake(void) { debug_print("msls_handle_handshake called\n"); }
void msls_get_connection(void) { debug_print("msls_get_connection called\n"); }
void msls_set_cookie(void) { debug_print("msls_set_cookie called\n"); }

// Returns int based on msls_receive_msg usage
int msls_lookup_context(ushort param_1) {
    debug_print("msls_lookup_context called with %hu\n", param_1);
    return 1; // Simulate success
}

// Returns int based on msls_receive_msg usage
int msls_handle_application(ushort *param_1) {
    debug_print("msls_handle_application called with message %p\n", (void*)param_1);
    return 0x12345678; // Simulate a checksum result
}

void msls_handle_changespec(void) { debug_print("msls_handle_changespec called\n"); }
void msls_handle_error(void) { debug_print("msls_handle_error called\n"); }

// --- Provided Functions ---

// Function: get_random
uint get_random(void) {
  uint val1 = *pRandom;
  uint val2 = *(pRandom + 1);
  uint *boundary_check_ptr = pRandom + 4;

  pRandom += 2;
  // The constant 0x4347cfff is interpreted as the upper bound of the random_pool.
  // If pRandom + 4 (boundary_check_ptr) exceeds this, reset pRandom.
  if (DAT_4347c800_ptr + RANDOM_POOL_SIZE < boundary_check_ptr) {
    pRandom = DAT_4347c800_ptr;
  }
  return val2 ^ val1;
}

// Function: msls_get_sls_functions
code ** msls_get_sls_functions(void) {
  // Allocate an array of 0x3c (60) function pointers
  code **ppcVar1 = (code **)calloc(0x3c, sizeof(code *));
  if (!ppcVar1) {
      debug_print("Failed to allocate memory for SLS functions array.\n");
      return NULL;
  }
  ppcVar1[0] = (code)msls_receive_msg;
  ppcVar1[1] = (code)msls_send_msg;
  ppcVar1[2] = (code)msls_cleanup;
  ppcVar1[3] = (code)msls_version_check;
  ppcVar1[4] = (code)msls_send_error_msg;
  ppcVar1[5] = (code)msls_generate_checksum;
  ppcVar1[6] = (code)msls_handle_heartbeat;
  ppcVar1[7] = (code)msls_destroy_msg;
  ppcVar1[8] = (code)msls_handle_handshake;
  ppcVar1[9] = (code)msls_get_connection;
  ppcVar1[10] = (code)msls_set_cookie;
  ppcVar1[0xb] = (code)msls_lookup_context;
  ppcVar1[0xc] = (code)msls_handle_application;
  ppcVar1[0xd] = (code)msls_handle_changespec;
  ppcVar1[0xe] = (code)msls_handle_error;
  return ppcVar1;
}

// Function: msls_receive_msg
ushort * msls_receive_msg(int param_1) {
  MslsContext *ctx = (MslsContext *)(uintptr_t)param_1; // Cast param_1 to MslsContext*
  code **functions = ctx->sls_functions;

  // Allocate one MSLS_MSG struct (24 bytes). Original calloc(0x18, sizeof(ushort)) was 48 bytes.
  // Using exact struct size for now.
  MSLS_MSG *msg = (MSLS_MSG *)calloc(1, sizeof(MSLS_MSG));
  if (!msg) {
      debug_print("Failed to allocate memory for message.\n");
      return NULL;
  }

  debug_print("Receive MSG\n");
  receive_fixed(&(msg->length), sizeof(msg->length));
  debug_print("msg length: %d\n", msg->length);

  if (msg->length < 0xf || msg->length > 0x800) {
    ((void (*)(ushort, undefined))functions[4])(0xe0, 0xa1); // Call msls_send_error_msg
    msls_destroy_msg(msg);
    return NULL;
  }

  receive_fixed(&(msg->field_2), sizeof(msg->field_2));
  receive_fixed(&(msg->msg_type), sizeof(msg->msg_type));
  receive_fixed(&(msg->field_8), sizeof(msg->field_8));
  receive_fixed(&(msg->field_C), sizeof(msg->field_C));
  receive_fixed(&(msg->data_len), sizeof(msg->data_len));

  ushort expected_data_len = msg->length - 0xf;

  if (msg->data_len == expected_data_len) {
    if (msg->data_len != 0) {
      void *data_buffer = calloc(msg->data_len, 1); // Allocate msg->data_len bytes
      if (!data_buffer) {
          debug_print("Failed to allocate data buffer.\n");
          ((void (*)(ushort, undefined))functions[4])(0xe0, 0xa1); // Error handling
          msls_destroy_msg(msg);
          return NULL;
      }
      msg->data_ptr = data_buffer;
      receive_fixed(msg->data_ptr, msg->data_len);
    }

    // Call functions[3] which is msls_version_check
    int check_result = ((int (*)(short))functions[3])(msg->field_2);
    if (check_result == 0) {
      ((void (*)(ushort, undefined))functions[4])(0xe1, 0xa1); // Call msls_send_error_msg
    } else {
      // Call functions[5] which is msls_generate_checksum
      uint checksum_result = ((uint (*)(int))functions[5])((int)(uintptr_t)msg);
      if (checksum_result == msg->field_8) {
        return (ushort *)msg; // Return the message pointer
      }
      ((void (*)(ushort, undefined))functions[4])(0xe2, 0xa1); // Call msls_send_error_msg
    }
  } else {
    debug_print("Length: %d expected: %d received: %d\n", msg->length, expected_data_len, msg->data_len);
    ((void (*)(ushort, undefined))functions[4])(0xe0, 0xa1); // Call msls_send_error_msg
  }

  msls_destroy_msg(msg);
  return NULL;
}

// Function: msls_send_msg
undefined4 msls_send_msg(int param_1_raw) {
  MSLS_MSG *msg = (MSLS_MSG *)(uintptr_t)param_1_raw;

  if (msg == NULL) {
    return 0xffffffff;
  }

  msg->field_8 = msls_generate_checksum_outgoing(param_1_raw);

  debug_print("Send MSG\n");
  transmit_fixed(&(msg->length), sizeof(msg->length));
  transmit_fixed(&(msg->field_2), sizeof(msg->field_2));
  transmit_fixed(&(msg->msg_type), sizeof(msg->msg_type));
  transmit_fixed(&(msg->field_8), sizeof(msg->field_8));
  transmit_fixed(&(msg->field_C), sizeof(msg->field_C));
  transmit_fixed(&(msg->data_len), sizeof(msg->data_len));

  if (msg->data_len != 0) {
    void *data_ptr = msg->data_ptr;
    if (data_ptr == NULL) {
        debug_print("Data pointer is NULL for msg type %hhu with data_len %hu\n", msg->msg_type, msg->data_len);
        return 0;
    }
    switch (msg->msg_type) {
      case 2:
        transmit_fixed(data_ptr, msg->data_len);
        break;
      case 3: {
        MsgType3Data *type3_data = (MsgType3Data *)data_ptr;
        transmit_fixed(type3_data, 0xc); // Transmit initial 12 bytes of MsgType3Data
        if (type3_data->len_at_10 != 0 && type3_data->ptr_at_C != NULL) {
          transmit_fixed(type3_data->ptr_at_C, type3_data->len_at_10);
        }
        break;
      }
      case 4: {
        MsgType4Data *type4_data = (MsgType4Data *)data_ptr;
        transmit_fixed(type4_data, 1); // Transmit 1 byte (field_0)
        transmit_fixed((byte *)type4_data + 1, 2); // Transmit 2 bytes (len_at_1)
        if (type4_data->len_at_1 != 0 && type4_data->ptr_at_3 != NULL) {
          transmit_fixed(type4_data->ptr_at_3, type4_data->len_at_1);
        }
        break;
      }
      case 5: { // Same logic as case 4
        MsgType4Data *type5_data = (MsgType4Data *)data_ptr;
        transmit_fixed(type5_data, 1);
        transmit_fixed((byte *)type5_data + 1, 2);
        if (type5_data->len_at_1 != 0 && type5_data->ptr_at_3 != NULL) {
          transmit_fixed(type5_data->ptr_at_3, type5_data->len_at_1);
        }
        break;
      }
    }
  }
  return 0;
}

// Function: msls_cleanup
void msls_cleanup(int param_1) {
  MslsContext *ctx = (MslsContext *)(uintptr_t)param_1;

  debug_print("Exiting\n");
  MSLS_MSG *msg = (MSLS_MSG *)calloc(1, sizeof(MSLS_MSG));
  if (!msg) {
      debug_print("Failed to allocate memory for cleanup message.\n");
      return;
  }
  msg->msg_type = 6; // Set message type
  msg->field_2 = 0xff01;
  msg->length = 0xf;
  msls_send_msg((int)(uintptr_t)msg);
  free(msg);

  for (int i = 0; i < 0x20; i++) { // Loop 32 times
    destroy_context(ctx->contexts[i]);
  }
  free(ctx->field_88);
  free(ctx->sls_functions); // Free the array of function pointers
  return;
}

// Function: msls_version_check
bool msls_version_check(short param_1) {
  return param_1 == -0xff;
}

// Function: msls_destroy_msg
void msls_destroy_msg(void *param_1) {
  if (param_1 == NULL) {
    return;
  }

  MSLS_MSG *msg = (MSLS_MSG *)param_1;
  void *data_ptr = msg->data_ptr;

  if (data_ptr != NULL) { // Only proceed if data_ptr is not NULL
    switch (msg->msg_type) {
      case 2:
        free(data_ptr);
        break;
      case 3: {
        MsgType3Data *type3_data = (MsgType3Data *)data_ptr;
        free(type3_data->ptr_at_C);
        free(type3_data);
        break;
      }
      case 4: {
        MsgType4Data *type4_data = (MsgType4Data *)data_ptr;
        // Check if ptr_at_3 is valid before freeing
        if (type4_data->len_at_1 != 0 && type4_data->ptr_at_3 != NULL) {
          free(type4_data->ptr_at_3);
        }
        free(type4_data); // Free the MsgType4Data itself
        break;
      }
      case 5: { // Similar to case 4, but the original code freed data_ptr after freeing ptr_at_3.
        MsgType4Data *type5_data = (MsgType4Data *)data_ptr;
        free(type5_data->ptr_at_3);
        free(type5_data);
        break;
      }
    }
  }
  free(msg);
}

// Function: msls_encrypt
void msls_encrypt(int param_1, uint param_2, int param_3) {
  // param_1 is `byte *` to data to encrypt
  // param_2 is length
  // param_3 is `MslsCryptoContext *` or similar
  byte *data_to_encrypt = (byte *)(uintptr_t)param_1;
  int crypto_ctx_ptr = param_3; // Pointer to crypto context

  int key_idx = 0;
  for (uint i = 0; i < param_2; i++) {
    data_to_encrypt[i] = data_to_encrypt[i] ^
                         *(byte *)(*(uintptr_t *)(crypto_ctx_ptr + 0x14) + key_idx * 4) ^
                         *(byte *)(*(uintptr_t *)(crypto_ctx_ptr + 0x20) + key_idx * 4);
    key_idx++;
    if (0x7f < key_idx) {
      key_idx = 0;
    }
  }
  return;
}

// Function: msls_decrypt
void msls_decrypt(int param_1, uint param_2, int param_3) {
  // param_1 is `byte *` to data to decrypt
  // param_2 is length
  // param_3 is `MslsCryptoContext *` or similar
  byte *data_to_decrypt = (byte *)(uintptr_t)param_1;
  int crypto_ctx_ptr = param_3; // Pointer to crypto context

  int key_idx = 0;
  for (uint i = 0; i < param_2; i++) {
    data_to_decrypt[i] = data_to_decrypt[i] ^
                         *(byte *)(*(uintptr_t *)(crypto_ctx_ptr + 0x18) + key_idx * 4) ^
                         *(byte *)(*(uintptr_t *)(crypto_ctx_ptr + 0x1c) + key_idx * 4);
    key_idx++;
    if (0x7f < key_idx) {
      key_idx = 0;
    }
  }
  return;
}

// Function: msls_send_error_msg
void msls_send_error_msg(ushort error_code, undefined sub_code) {
  debug_print("Error: %x (Sub: %x)\n", error_code, sub_code);

  MSLS_MSG *msg = (MSLS_MSG *)calloc(1, sizeof(MSLS_MSG));
  if (!msg) {
      debug_print("Failed to allocate memory for error message.\n");
      return;
  }

  msg->msg_type = 2; // Error message type

  ErrorData *error_data = (ErrorData *)calloc(1, sizeof(ErrorData));
  if (!error_data) {
      debug_print("Failed to allocate memory for error data.\n");
      free(msg);
      return;
  }

  error_data->code = error_code;
  error_data->sub_code = sub_code;

  msg->data_ptr = error_data;
  msg->data_len = 3; // As per original code's `psVar1[8] = 3;`
  msg->length = msg->data_len + 0xf;

  msls_send_msg((int)(uintptr_t)msg);
  msls_destroy_msg(msg);
  return;
}

// Function: msls_generate_checksum
uint msls_generate_checksum(int param_1) {
  MSLS_MSG *msg = (MSLS_MSG *)(uintptr_t)param_1;

  uint checksum = 0;
  if (msg == NULL || msg->data_ptr == NULL) {
      return 0; // Handle null pointer case
  }

  for (int i = 0; i < msg->data_len; i++) { // Loop for data_len bytes
    checksum = ((byte *)msg->data_ptr)[i] ^ (checksum << 8 | checksum >> 0x18);
  }
  return checksum;
}

// Function: msls_generate_checksum_outgoing
uint msls_generate_checksum_outgoing(int param_1) {
  MSLS_MSG *msg = (MSLS_MSG *)(uintptr_t)param_1;
  uint checksum = 0;

  if (msg == NULL) {
      return 0;
  }

  void *data_ptr = msg->data_ptr;

  switch (msg->msg_type) {
  case 1:
  case 2:
  case 6:
    if (data_ptr != NULL) {
      for (int i = 0; i < msg->data_len; i++) {
        checksum = ((byte *)data_ptr)[i] ^ (checksum << 8 | checksum >> 0x18);
      }
    }
    break;
  case 3: {
    MsgType3Data *type3_data = (MsgType3Data *)data_ptr;
    if (type3_data != NULL) {
      for (int i = 0; i < 0xc; i++) { // Checksum first 12 bytes of MsgType3Data
        checksum = ((byte *)type3_data)[i] ^ (checksum << 8 | checksum >> 0x18);
      }
      if (type3_data->len_at_10 != 0 && type3_data->ptr_at_C != NULL) {
        for (int i = 0; i < type3_data->len_at_10; i++) {
          checksum = ((byte *)type3_data->ptr_at_C)[i] ^ (checksum << 8 | checksum >> 0x18);
        }
      }
    }
    break;
  }
  case 4: {
    MsgType4Data *type4_data = (MsgType4Data *)data_ptr;
    if (type4_data != NULL) {
      for (int i = 0; i < 3; i++) { // Checksum first 3 bytes (field_0 and len_at_1)
        checksum = ((byte *)type4_data)[i] ^ (checksum << 8 | checksum >> 0x18);
      }
      if (type4_data->len_at_1 != 0 && type4_data->ptr_at_3 != NULL) {
        for (int i = 0; i < type4_data->len_at_1; i++) {
          checksum = ((byte *)type4_data->ptr_at_3)[i] ^ (checksum << 8 | checksum >> 0x18);
        }
      }
    }
    break;
  }
  case 5: { // Same logic as case 4
    MsgType4Data *type5_data = (MsgType4Data *)data_ptr;
    if (type5_data != NULL) {
      for (int i = 0; i < 3; i++) {
        checksum = ((byte *)type5_data)[i] ^ (checksum << 8 | checksum >> 0x18);
      }
      if (type5_data->len_at_1 != 0 && type5_data->ptr_at_3 != NULL) {
        for (int i = 0; i < type5_data->len_at_1; i++) {
          checksum = ((byte *)type5_data->ptr_at_3)[i] ^ (checksum << 8 | checksum >> 0x18);
        }
      }
    }
    break;
  }
  }
  return checksum;
}