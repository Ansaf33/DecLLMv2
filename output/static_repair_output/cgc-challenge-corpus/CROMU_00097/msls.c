#include <stdint.h>  // For uint32_t, uint16_t, uint8_t
#include <stdlib.h>  // For calloc, free
#include <stdio.h>   // For printf, vprintf
#include <stdbool.h> // For bool
#include <stdarg.h>  // For va_list in debug_print
#include <string.h>  // For memset (in dummy receive_fixed if uncommented)

// Type definitions based on the snippet's context
typedef uint32_t uint;
typedef uint16_t ushort;
typedef uint8_t byte;
typedef uint8_t undefined;
typedef uint16_t undefined2;
typedef uint32_t undefined4;

// Forward declarations for functions used in function pointers
// The actual arguments for 'code' functions vary, so we define a generic function pointer type.
// Specific casts will be used when calling them from the array.
typedef void (*code)(void);

// Declare all functions from the snippet for proper linking and type checking
uint get_random(void);
code ** msls_get_sls_functions(void);
ushort * msls_receive_msg(int param_1);
undefined4 msls_send_msg(int param_1);
void msls_cleanup(int param_1);
bool msls_version_check(short param_1);
void msls_destroy_msg(void *param_1);
void msls_encrypt(int param_1, uint param_2, int param_3);
void msls_decrypt(int param_1, uint param_2, int param_3);
void msls_send_error_msg(ushort param_1, undefined param_2);
uint msls_generate_checksum(int param_1);
uint msls_generate_checksum_outgoing(int param_1);

// Placeholder for external/global variables and functions
// For get_random: Simulate a random number buffer
#define RANDOM_BUFFER_SIZE 0x200 // 0x800 bytes / 4 bytes per uint = 0x200 uints
static uint random_data_buffer[RANDOM_BUFFER_SIZE] = {
    // Initialize with some dummy data for demonstration
    0x12345678, 0xABCDEF01, 0xDEADBEEF, 0xCAFEBABE, 0x11223344, 0x55667788,
    // Fill the rest with zeros or more dummy data
};
static uint *pRandom = random_data_buffer;
// Simulate the end address `0x4347cfff` and start address `DAT_4347c800`
static const uint *RANDOM_BUFFER_END_PTR = &random_data_buffer[RANDOM_BUFFER_SIZE - 1];
#define DAT_4347c800 random_data_buffer

// Dummy implementations for functions not provided in the snippet
void debug_print(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

void receive_fixed(void *buf, size_t len) {
    if (buf && len > 0) {
        // In a real system, this would read from a network socket or similar.
        // For compilation, we can optionally fill with dummy data.
        // memset(buf, 0, len); // Example: fill with zeros
    }
}

void transmit_fixed(const void *buf, size_t len) {
    (void)buf; // Suppress unused parameter warning
    (void)len; // Suppress unused parameter warning
    // In a real system, this would write to a network socket or similar.
}

void destroy_context(uint32_t param_1) {
    (void)param_1; // Suppress unused parameter warning
    // Dummy implementation for context destruction.
}

// Dummy function prototypes for msls_get_sls_functions array
void msls_handle_heartbeat(void) { debug_print("msls_handle_heartbeat called\n"); }
void msls_handle_handshake(void) { debug_print("msls_handle_handshake called\n"); }
void msls_get_connection(void) { debug_print("msls_get_connection called\n"); }
void msls_set_cookie(void) { debug_print("msls_set_cookie called\n"); }
void msls_lookup_context(void) { debug_print("msls_lookup_context called\n"); }
void msls_handle_application(void) { debug_print("msls_handle_application called\n"); }
void msls_handle_changespec(void) { debug_print("msls_handle_changespec called\n"); }
void msls_handle_error(void) { debug_print("msls_handle_error called\n"); }


// Structure definitions based on usage and offsets in the provided snippet.
// These are educated guesses to make pointer arithmetic type-safe and readable.

// Message structure (msls_receive_msg, msls_send_msg, msls_destroy_msg)
// The `calloc(0x18, ...)` in `msls_receive_msg` suggests a total size of 24 bytes.
// Assuming 32-bit pointers (4 bytes) for `void *payload`.
typedef struct msls_message {
    ushort msg_len;       // 0x00 (2 bytes)
    ushort msg_type;      // 0x02 (2 bytes)
    undefined msg_subtype; // 0x04 (1 byte)
    uint8_t _pad1[3];     // Padding to align next 4-byte field (0x04 + 1 + 3 = 0x08)
    undefined4 checksum;  // 0x08 (4 bytes)
    undefined4 cookie;    // 0x0C (4 bytes)
    ushort payload_len;   // 0x10 (2 bytes)
    uint8_t _pad2[2];     // Padding to align next 4-byte pointer (0x10 + 2 + 2 = 0x14)
    void *payload;        // 0x14 (4 bytes assumed for 32-bit system)
} msls_message_t;

// Context structure (msls_cleanup, msls_receive_msg, msls_encrypt, msls_decrypt)
// Offsets are derived from `param_1 + X` or `param_3 + X` accesses.
typedef struct msls_context {
    uint32_t _dummy_field_0;    // 0x00-0x03 (placeholder for unknown field)
    void *sls_functions_table;  // 0x04-0x07 (pointer to array of function pointers)
    uint32_t contexts[0x20];    // 0x08-0x87 (array of 32 uint32_t's for contexts)
    void *_some_ptr_88;         // 0x88-0x8B (placeholder for unknown pointer)
    // Encryption/decryption keys, offsets are relative to `param_3` in encrypt/decrypt functions
    // These are placed here assuming `param_3` is a pointer to `msls_context_t`.
    // The actual layout might be different if keys are in a sub-struct or separate.
    // We'll use direct pointers to simulate the original access patterns.
    // These specific offsets are relative to the start of the context.
    uint32_t _dummy_field_0x8C[2]; // Padding to reach 0x14 relative to context base
    uint32_t *encrypt_key_part1; // 0x14 relative to context base
    uint32_t *decrypt_key_part1; // 0x18 relative to context base
    uint32_t *decrypt_key_part2; // 0x1C relative to context base
    uint32_t *encrypt_key_part2; // 0x20 relative to context base
    // ... potentially many other fields to fill the context struct.
} msls_context_t;

// For msls_send_error_msg payload
typedef struct error_payload_fixed {
    ushort code;      // 2 bytes
    undefined subcode; // 1 byte
} error_payload_fixed_t; // Total 3 bytes, matching `psVar1[8] = 3;` for payload length


// Function: get_random
uint get_random(void) {
  uint val1 = *pRandom;
  uint val2 = *(pRandom + 1);
  
  pRandom += 2; // Advance pRandom by 2 uints
  
  // Check if pRandom, after advancing, would make the original pRandom + 4 go past the buffer end.
  // The original `puVar3 = pRandom + 4;` refers to the `pRandom` *before* the increment.
  // This is equivalent to `(current pRandom) + 2` being past the end.
  if (pRandom + 2 > RANDOM_BUFFER_END_PTR) {
    pRandom = DAT_4347c800; // Reset to the beginning of the buffer
  }
  return val2 ^ val1;
}

// Function: msls_get_sls_functions
code ** msls_get_sls_functions(void) {
  // Allocate memory for 0x3c (60) function pointers
  code **function_pointers = (code **)calloc(0x3c, sizeof(code *));
  if (function_pointers == NULL) {
      debug_print("msls_get_sls_functions: calloc failed\n");
      return NULL;
  }
  
  function_pointers[0] = (code)msls_receive_msg;
  function_pointers[1] = (code)msls_send_msg;
  function_pointers[2] = (code)msls_cleanup;
  function_pointers[3] = (code)msls_version_check;
  function_pointers[4] = (code)msls_send_error_msg;
  function_pointers[5] = (code)msls_generate_checksum;
  function_pointers[6] = (code)msls_handle_heartbeat;
  function_pointers[7] = (code)msls_destroy_msg;
  function_pointers[8] = (code)msls_handle_handshake;
  function_pointers[9] = (code)msls_get_connection;
  function_pointers[0xb] = (code)msls_lookup_context;
  function_pointers[0xc] = (code)msls_handle_application;
  function_pointers[0xd] = (code)msls_handle_changespec;
  function_pointers[0xe] = (code)msls_handle_error;
  // Note: 0xA (10) is `msls_set_cookie` which is missing from the original list, but 0xB is `msls_lookup_context`.
  // Assuming 0xA is intended to be `msls_set_cookie` based on the sequence.
  function_pointers[10] = (code)msls_set_cookie; 

  return function_pointers;
}

// Function: msls_receive_msg
ushort * msls_receive_msg(int param_1_context_ptr) {
  msls_message_t *msg = (msls_message_t *)calloc(1, sizeof(msls_message_t));
  if (msg == NULL) {
      debug_print("msls_receive_msg: calloc failed for message\n");
      return NULL;
  }
  
  debug_print("Receive MSG\n");
  
  receive_fixed(&msg->msg_len, sizeof(msg->msg_len));
  debug_print("msg length: %d\n", msg->msg_len);
  
  if ((msg->msg_len < 0xf) || (0x800 < msg->msg_len)) {
    msls_context_t *ctx = (msls_context_t *)param_1_context_ptr;
    code **sls_funcs = (code **)ctx->sls_functions_table;
    // Call error handler: msls_send_error_msg, which is at index 4 (0x10 / sizeof(code*))
    ((void (*)(ushort, undefined))sls_funcs[4])(0xe0, 0xa1);
  } else {
    receive_fixed(&msg->msg_type, sizeof(msg->msg_type));
    receive_fixed(&msg->msg_subtype, sizeof(msg->msg_subtype));
    receive_fixed(&msg->checksum, sizeof(msg->checksum));
    receive_fixed(&msg->cookie, sizeof(msg->cookie));
    receive_fixed(&msg->payload_len, sizeof(msg->payload_len));
    
    if ((uint)msg->payload_len == msg->msg_len - 0xf) {
      if (msg->payload_len != 0) {
        msg->payload = calloc(msg->payload_len, 1); // Allocate payload in bytes
        if (msg->payload == NULL) {
            debug_print("msls_receive_msg: payload calloc failed\n");
            msls_destroy_msg(msg);
            return NULL;
        }
        receive_fixed(msg->payload, msg->payload_len);
      }
      
      msls_context_t *ctx = (msls_context_t *)param_1_context_ptr;
      code **sls_funcs = (code **)ctx->sls_functions_table;
      
      // Call msls_version_check, which is at index 3 (0xc / sizeof(code*))
      int check_result = ((int (*)(ushort))sls_funcs[3])(msg->msg_type);
      if (check_result == 0) {
        ((void (*)(ushort, undefined))sls_funcs[4])(0xe1, 0xa1);
      } else {
        // Call msls_generate_checksum, which is at index 5 (0x14 / sizeof(code*))
        int calculated_checksum = ((int (*)(msls_message_t*))sls_funcs[5])(msg);
        if (calculated_checksum == (int)msg->checksum) {
          return (ushort *)msg; // Return the message structure
        }
        ((void (*)(ushort, undefined))sls_funcs[4])(0xe2, 0xa1);
      }
    } else {
      debug_print("Length: %d expected: %d received: %d\n", msg->msg_len, msg->msg_len - 0xf, msg->payload_len);
      msls_context_t *ctx = (msls_context_t *)param_1_context_ptr;
      code **sls_funcs = (code **)ctx->sls_functions_table;
      ((void (*)(ushort, undefined))sls_funcs[4])(0xe0, 0xa1);
    }
  }
  msls_destroy_msg(msg);
  return NULL; // Return NULL on error or if message is destroyed
}

// Function: msls_send_msg
undefined4 msls_send_msg(int param_1_msg_ptr) {
  msls_message_t *msg = (msls_message_t *)param_1_msg_ptr;
  
  if (msg == NULL) {
    return 0xffffffff;
  }
  
  msg->checksum = msls_generate_checksum_outgoing(param_1_msg_ptr);
  
  debug_print("Send MSG\n");
  
  transmit_fixed(&msg->msg_len, sizeof(msg->msg_len));
  transmit_fixed(&msg->msg_type, sizeof(msg->msg_type));
  transmit_fixed(&msg->msg_subtype, sizeof(msg->msg_subtype));
  transmit_fixed(&msg->checksum, sizeof(msg->checksum));
  transmit_fixed(&msg->cookie, sizeof(msg->cookie));
  transmit_fixed(&msg->payload_len, sizeof(msg->payload_len));
  
  if (msg->payload_len != 0) {
    switch(msg->msg_subtype) {
      case 2:
        transmit_fixed(msg->payload, msg->payload_len);
        break;
      case 3: {
        char *payload_inner_ptr = (char *)msg->payload;
        if (payload_inner_ptr != NULL) {
          transmit_fixed(payload_inner_ptr, 0xc); // First 12 bytes
          ushort inner_len = *(ushort *)(payload_inner_ptr + 10);
          void *inner_data_ptr = *(void **)(payload_inner_ptr + 0xc);
          if (inner_len != 0 && inner_data_ptr != NULL) {
            transmit_fixed(inner_data_ptr, inner_len);
          }
        }
        break;
      }
      case 4: {
        char *payload_inner_ptr = (char *)msg->payload;
        transmit_fixed(payload_inner_ptr, 1); // First byte
        transmit_fixed(payload_inner_ptr + 1, 2); // Next 2 bytes
        ushort inner_len = *(ushort *)(payload_inner_ptr + 1);
        void *inner_data_ptr = *(void **)(payload_inner_ptr + 3);
        if (inner_len != 0 && inner_data_ptr != NULL) {
          transmit_fixed(inner_data_ptr, inner_len);
        }
        break;
      }
      case 5: {
        char *payload_inner_ptr = (char *)msg->payload;
        if (payload_inner_ptr != NULL) {
          transmit_fixed(payload_inner_ptr, 1);
          transmit_fixed(payload_inner_ptr + 1, 2);
          ushort inner_len = *(ushort *)(payload_inner_ptr + 1);
          void *inner_data_ptr = *(void **)(payload_inner_ptr + 3);
          if (inner_len != 0 && inner_data_ptr != NULL) {
            transmit_fixed(inner_data_ptr, inner_len);
          }
        }
        break;
      }
    }
  }
  return 0;
}

// Function: msls_cleanup
void msls_cleanup(int param_1_context_ptr) {
  debug_print("Exiting\n");
  
  msls_message_t *msg_exit = (msls_message_t *)calloc(1, sizeof(msls_message_t));
  if (msg_exit == NULL) {
      debug_print("msls_cleanup: calloc failed for exit message\n");
      // Continue cleanup as much as possible, don't return early.
  } else {
    msg_exit->msg_subtype = 6;
    msg_exit->msg_type = 0xff01;
    msg_exit->msg_len = 0xf;
    msls_send_msg((int)msg_exit);
    free(msg_exit); // Free the message struct (payload not dynamically allocated here)
  }
  
  msls_context_t *ctx = (msls_context_t *)param_1_context_ptr;
  for (int i = 0; i < 0x20; i++) {
    destroy_context(ctx->contexts[i]);
  }
  
  free(ctx->_some_ptr_88);
  free(ctx->sls_functions_table);
}

// Function: msls_version_check
bool msls_version_check(short param_1) {
  // `0xff01` as a short is -255 in signed representation.
  // The original code uses `short param_1` and compares to `-0xff`.
  // `msls_cleanup` sets `msg_type = 0xff01;`
  // This comparison is likely checking for the unsigned short value `0xff01`.
  return param_1 == (short)0xff01;
}

// Function: msls_destroy_msg
void msls_destroy_msg(void *param_1_msg_ptr) {
  msls_message_t *msg = (msls_message_t *)param_1_msg_ptr;
  
  if (msg != NULL) {
    switch(msg->msg_subtype) {
      case 2:
        free(msg->payload);
        break;
      case 3: {
        char *payload_inner_ptr = (char *)msg->payload;
        if (payload_inner_ptr != NULL) {
          free(*(void **)(payload_inner_ptr + 0xc));
          free(payload_inner_ptr);
        }
        break;
      }
      case 4: {
        char *payload_inner_ptr = (char *)msg->payload;
        if (payload_inner_ptr != NULL) { // Added null check for safety
          ushort inner_len = *(ushort *)(payload_inner_ptr + 1);
          void *inner_data_ptr = *(void **)(payload_inner_ptr + 3);
          if (inner_len != 0 && inner_data_ptr != NULL) {
            free(inner_data_ptr);
          }
          // The original code does not free `payload_inner_ptr` itself for case 4.
        }
        break;
      }
      case 5: {
        char *payload_inner_ptr = (char *)msg->payload;
        if (payload_inner_ptr != NULL) {
          free(*(void **)(payload_inner_ptr + 3));
          free(payload_inner_ptr);
        }
        break;
      }
    }
    free(msg);
  }
}

// Function: msls_encrypt
void msls_encrypt(int param_1_data_ptr, uint param_2_len, int param_3_context_ptr) {
  msls_context_t *ctx = (msls_context_t *)param_3_context_ptr;
  uint8_t *data = (uint8_t *)param_1_data_ptr;
  int key_idx = 0;
  
  for (uint i = 0; i < param_2_len; i++) {
    data[i] =
         data[i] ^
         (byte)ctx->encrypt_key_part1[key_idx] ^
         (byte)ctx->encrypt_key_part2[key_idx];
    key_idx++;
    if (0x7f < key_idx) { // 127
      key_idx = 0;
    }
  }
}

// Function: msls_decrypt
void msls_decrypt(int param_1_data_ptr, uint param_2_len, int param_3_context_ptr) {
  msls_context_t *ctx = (msls_context_t *)param_3_context_ptr;
  uint8_t *data = (uint8_t *)param_1_data_ptr;
  int key_idx = 0;
  
  for (uint i = 0; i < param_2_len; i++) {
    data[i] =
         data[i] ^
         (byte)ctx->decrypt_key_part1[key_idx] ^
         (byte)ctx->decrypt_key_part2[key_idx];
    key_idx++;
    if (0x7f < key_idx) { // 127
      key_idx = 0;
    }
  }
}

// Function: msls_send_error_msg
void msls_send_error_msg(ushort param_1_error_code, undefined param_2_error_subcode) {
  debug_print("Error: %x\n", param_1_error_code);
  
  msls_message_t *error_msg = (msls_message_t *)calloc(1, sizeof(msls_message_t));
  if (error_msg == NULL) {
      debug_print("msls_send_error_msg: calloc failed for error message\n");
      return;
  }
  
  error_msg->msg_subtype = 2; // Message type for error payload
  
  error_payload_fixed_t *err_payload = (error_payload_fixed_t *)calloc(1, sizeof(error_payload_fixed_t));
  if (err_payload == NULL) {
      debug_print("msls_send_error_msg: calloc failed for error payload\n");
      free(error_msg);
      return;
  }
  
  err_payload->code = param_1_error_code;
  err_payload->subcode = param_2_error_subcode;
  
  error_msg->payload = err_payload;
  error_msg->payload_len = sizeof(error_payload_fixed_t); // This is 3 bytes
  error_msg->msg_len = error_msg->payload_len + 0xf; // Total length includes header and payload
  
  msls_send_msg((int)error_msg);
  msls_destroy_msg(error_msg);
}

// Function: msls_generate_checksum
uint msls_generate_checksum(int param_1_msg_ptr) {
  msls_message_t *msg = (msls_message_t *)param_1_msg_ptr;
  uint checksum_val = 0;
  
  uint8_t *payload_bytes = (uint8_t *)msg->payload;
  for (int i = 0; i < msg->payload_len; i++) {
    // 8-bit left rotate for a 32-bit uint
    checksum_val = payload_bytes[i] ^ (checksum_val << 8 | checksum_val >> 0x18);
  }
  return checksum_val;
}

// Function: msls_generate_checksum_outgoing
uint msls_generate_checksum_outgoing(int param_1_msg_ptr) {
  msls_message_t *msg = (msls_message_t *)param_1_msg_ptr;
  uint checksum_val = 0;
  
  switch(msg->msg_subtype) {
    case 1:
    case 2:
    case 6: {
      uint8_t *payload_bytes = (uint8_t *)msg->payload;
      for (int i = 0; i < msg->payload_len; i++) {
        checksum_val = payload_bytes[i] ^ (checksum_val << 8 | checksum_val >> 0x18);
      }
      break;
    }
    case 3: {
      char *payload_inner_ptr = (char *)msg->payload;
      if (payload_inner_ptr != NULL) {
        for (int i = 0; i < 0xc; i++) { // First 12 bytes of inner struct
          checksum_val = payload_inner_ptr[i] ^ (checksum_val << 8 | checksum_val >> 0x18);
        }
        ushort inner_payload_len = *(ushort *)(payload_inner_ptr + 10);
        uint8_t *inner_payload_data = *(uint8_t **)(payload_inner_ptr + 0xc);
        for (int i = 0; i < inner_payload_len; i++) {
          checksum_val = inner_payload_data[i] ^ (checksum_val << 8 | checksum_val >> 0x18);
        }
      }
      break;
    }
    case 4: {
      char *payload_inner_ptr = (char *)msg->payload;
      if (payload_inner_ptr != NULL) {
        for (int i = 0; i < 3; i++) { // First 3 bytes of inner struct
          checksum_val = payload_inner_ptr[i] ^ (checksum_val << 8 | checksum_val >> 0x18);
        }
        ushort inner_payload_len = *(ushort *)(payload_inner_ptr + 1);
        uint8_t *inner_payload_data = *(uint8_t **)(payload_inner_ptr + 3);
        for (int i = 0; i < inner_payload_len; i++) {
          checksum_val = inner_payload_data[i] ^ (checksum_val << 8 | checksum_val >> 0x18);
        }
      }
      break;
    }
    case 5: {
      char *payload_inner_ptr = (char *)msg->payload;
      if (payload_inner_ptr != NULL) {
        for (int i = 0; i < 3; i++) { // First 3 bytes of inner struct
          checksum_val = payload_inner_ptr[i] ^ (checksum_val << 8 | checksum_val >> 0x18);
        }
        ushort inner_payload_len = *(ushort *)(payload_inner_ptr + 1);
        uint8_t *inner_payload_data = *(uint8_t **)(payload_inner_ptr + 3);
        for (int i = 0; i < inner_payload_len; i++) {
          checksum_val = inner_payload_data[i] ^ (checksum_val << 8 | checksum_val >> 0x18);
        }
      }
      break;
    }
  }
  return checksum_val;
}

// Minimal main function to make the code compilable and demonstrate basic usage
int main() {
    debug_print("Starting program.\n");

    // Example usage of get_random
    uint r1 = get_random();
    uint r2 = get_random();
    debug_print("Random numbers: %u, %u\n", r1, r2);

    // Example usage of msls_get_sls_functions
    code **sls_functions_table = msls_get_sls_functions();
    if (sls_functions_table != NULL) {
        debug_print("SLS functions table created successfully.\n");
        // Demonstrate calling a dummy function from the table (e.g., index 6 for heartbeat)
        ((void (*)(void))sls_functions_table[6])(); 
        free(sls_functions_table);
    }

    // Example usage of msls_send_error_msg
    msls_send_error_msg(0x123, 0xAB);

    debug_print("Program finished.\n");
    return 0;
}