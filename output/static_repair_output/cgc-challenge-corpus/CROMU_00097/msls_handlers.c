#include <stdlib.h> // For calloc, free
#include <string.h> // For memcpy
#include <stdio.h>  // For debug_print (assuming it's printf-like)
#include <stdint.h> // For fixed-width integers, uintptr_t, intptr_t
#include <stdarg.h> // For va_list in debug_print if it's custom

// --- Placeholder function declarations ---
// Assuming debug_print is a printf-like function for simplicity.
// In a real scenario, this would be provided by a library or defined elsewhere.
void debug_print(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}

// Placeholder for external functions
void msls_send_msg(void *msg);
void msls_destroy_msg(void *msg);
void **msls_send_server_hello(void *param_2);

// Function: msls_handle_heartbeat
void msls_handle_heartbeat(void *param_1) {
  debug_print("Handling Heartbeat\n");

  // Allocate 24 bytes for the main message structure.
  // The decompiler inferred 'short *' for psVar1, but byte-level offsets are common.
  // We use uint8_t* as the base for byte-level access, and short* for short-aligned access.
  short *msg_base = (short *)calloc(0x18, 1); // Allocate 24 bytes, initialized to zero
  if (msg_base == NULL) {
    return; // Handle allocation error
  }
  uint8_t *msg_bytes = (uint8_t *)msg_base;

  // Allocate 7 bytes for the heartbeat data structure.
  uint8_t *hb_data = (uint8_t *)calloc(7, 1); // Allocate 7 bytes, initialized to zero
  if (hb_data == NULL) {
    free(msg_base);
    return; // Handle allocation error
  }

  // Populate msg_base fields using specific byte offsets and types
  msg_bytes[4] = 5; // *(undefined *)(psVar1 + 2) = 5; (byte at offset 4, assuming psVar1 is short* base)
  msg_base[1] = (short)-0xff; // psVar1[1] = -0xff; (short at offset 2)
  *(uint32_t *)(msg_bytes + 12) = *(uint32_t *)((uint8_t *)param_1 + 0xc); // *(undefined4 *)(psVar1 + 6) = *(undefined4 *)(param_1 + 0xc);

  // Populate hb_data fields
  hb_data[0] = 0x42; // *puVar2 = 0x42;

  // Get base address for data from param_1 and calculate __src
  uint32_t param1_base_addr = *(uint32_t *)((uint8_t *)param_1 + 0x14);
  void *__src = (void *)(uintptr_t)(param1_base_addr + 3);

  // Set the length field in hb_data
  *(uint16_t *)(hb_data + 1) = *(uint16_t *)(uintptr_t)(param1_base_addr + 1);

  uint16_t data_length = *(uint16_t *)(hb_data + 1);
  if (data_length != 0) {
    void *buffer_for_data = calloc(data_length, 1); // Allocate 'data_length' bytes
    if (buffer_for_data == NULL) {
      free(msg_base);
      free(hb_data);
      return; // Handle allocation error
    }
    *(void **)(hb_data + 3) = buffer_for_data; // Store pointer to allocated buffer

    debug_print("Reading heartbeats from %p to %p\n", __src,
                (void *)((uintptr_t)__src + data_length));
    memcpy(*(void **)(hb_data + 3), __src, data_length);
  }

  // Finalize msg_base fields
  *(int16_t *)(msg_bytes + 16) = *(int16_t *)(hb_data + 1) + 3; // psVar1[8] = *(short *)(puVar2 + 1) + 3;
  *(void **)(msg_bytes + 20) = hb_data; // *(undefined **)(psVar1 + 10) = puVar2;
  *msg_base = *(int16_t *)(msg_bytes + 16) + 0xf; // *psVar1 = psVar1[8] + 0xf;

  debug_print("Sending HB response. Length: %d\n", data_length);
  msls_send_msg(msg_base);
  // Assuming msls_destroy_msg handles deep freeing (msg_base, hb_data, buffer_for_data)
  msls_destroy_msg(msg_base);
  return;
}

// Function: msls_handle_changespec
void **msls_handle_changespec(uint32_t param_1_unused, void *param_2, void *param_3) {
  void **ppuVar1 = NULL; // Initialize with NULL as a default/error return value

  // Check if the pointer to the pointer to uint16_t is valid, and param_2 is not NULL
  uint16_t **ptr_to_ptr_to_uint16 = (uint16_t **)((uint8_t *)param_3 + 0x14);
  if ((ptr_to_ptr_to_uint16 != NULL) && (*ptr_to_ptr_to_uint16 != NULL) && (param_2 != NULL)) {
    // Dereference twice to get the uint16_t value and assign it
    *(uint16_t *)((uint8_t *)param_2 + 0x10) = **ptr_to_ptr_to_uint16;
    ppuVar1 = msls_send_server_hello(param_2);
  }
  return ppuVar1;
}

// Function: msls_handle_error
void msls_handle_error(void *param_1, void *param_2) {
  // Extract function pointer from param_1 structure
  uint32_t func_table_base = *(uint32_t *)((uint8_t *)param_1 + 4);
  // Assuming the callback returns intptr_t (a pointer or integer result) and takes void*, uint32_t
  typedef intptr_t (*CallbackFunc)(void *, uint32_t);
  CallbackFunc callback = *(CallbackFunc *)(uintptr_t)(func_table_base + 0x2c);

  // Call the function pointer
  intptr_t iVar3 = callback(param_1, *(uint32_t *)((uint8_t *)param_2 + 0xc));

  // Extract pointer to error data from param_2
  uint16_t *error_data_ptr = *(uint16_t **)((uint8_t *)param_2 + 0x14);

  if (error_data_ptr != NULL) {
    // Assuming puVar2 is uint16_t*, then puVar2 + 1 means 2 bytes offset (sizeof(uint16_t))
    // So, *(byte *)(puVar2 + 1) is a byte at (uint8_t*)error_data_ptr + 2
    uint8_t error_type = *(uint8_t *)((uint8_t *)error_data_ptr + 2);
    uint16_t error_code = *error_data_ptr; // Value at puVar2 (byte offset 0)

    if (error_type == 0xa2) {
      debug_print("Received Fatal Error 0x%x. Shutting down server\n", error_code);
      ((uint8_t *)param_1)[1] = 1; // *(undefined *)(param_1 + 1) = 1;
    } else if (error_type < 0xa3) { // Covers 0xa0, 0xa1
      if (error_type == 0xa0) {
        debug_print("Received Client Warning 0x%x\n", error_code);
      } else if (error_type == 0xa1) {
        if (iVar3 != 0) {
          debug_print("Received Critical Error 0x%x. Shutting down connection.\n", error_code);
          ((uint8_t *)iVar3)[0xc] = 0; // *(undefined *)(iVar3 + 0xc) = 0;
        }
      }
    } else { // error_type >= 0xa3 (unknown)
      debug_print("Unknown Error Code 0x%x 0x%x\n", error_code, error_type);
    }
  }
  return;
}