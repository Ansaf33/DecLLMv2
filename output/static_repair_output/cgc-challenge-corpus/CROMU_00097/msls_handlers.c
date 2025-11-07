#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h> // For va_list in debug_print
#include <stddef.h> // For NULL

// --- Stub Functions and Global Placeholders ---

// A basic debug_print function similar to printf
void debug_print(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}

// Placeholder for _GLOBAL_OFFSET_TABLE_
// In a real scenario, this would be provided by the linker or defined in a specific module.
// For standalone compilation, we'll make a simple placeholder.
void *_GLOBAL_OFFSET_TABLE_[] = {NULL};

// Stub for msls_send_msg
void msls_send_msg(uint8_t *msg_ptr) {
    // Placeholder for actual message sending logic
    debug_print("msls_send_msg called with pointer %p\n", (void*)msg_ptr);
    // In a real system, this might send the buffer and then it's managed externally,
    // or it might be freed here if it's a "fire and forget" message.
    // For this example, msls_destroy_msg is called explicitly after msls_send_msg.
}

// Stub for msls_destroy_msg
void msls_destroy_msg(uint8_t *msg_ptr) {
    // Placeholder for actual message destruction/freeing logic
    if (msg_ptr) {
        // Based on the structure, heartbeat_data_ptr (puVar2) is nested in msg_buffer (psVar1).
        // allocated_data_buffer (pvVar3) is nested in heartbeat_data_ptr.
        uint8_t *heartbeat_data_ptr = *(uint8_t **)(msg_ptr + 10 * sizeof(int16_t));
        if (heartbeat_data_ptr) {
            void *allocated_data_buffer = *(void **)(heartbeat_data_ptr + 3);
            if (allocated_data_buffer) {
                free(allocated_data_buffer);
            }
            free(heartbeat_data_ptr);
        }
        free(msg_ptr);
    }
    debug_print("msls_destroy_msg called with pointer %p\n", (void*)msg_ptr);
}

// Stub for msls_send_server_hello
void **msls_send_server_hello(void *param_2) {
    debug_print("msls_send_server_hello called with %p\n", param_2);
    // Return a placeholder, e.g., _GLOBAL_OFFSET_TABLE_
    return (void **)_GLOBAL_OFFSET_TABLE_;
}

// Define a type for the function pointer used in msls_handle_error
typedef int (*handler_func)(void *, uint32_t);

// --- Fixed and Refactored Functions ---

// Function: msls_handle_heartbeat
void msls_handle_heartbeat(void *param_1) {
  uint8_t *msg_buffer; // Renamed psVar1 to msg_buffer for clarity
  uint8_t *heartbeat_data_ptr; // Renamed puVar2 to heartbeat_data_ptr
  void *src_data_ptr; // Renamed __src to src_data_ptr
  void *allocated_data_buffer = NULL; // Renamed pvVar3 to allocated_data_buffer, initialized to NULL
  uint16_t data_length;

  debug_print("Handling Heartbeat\n");

  // Allocate 48 bytes (0x18 * sizeof(int16_t)) for the main message buffer
  msg_buffer = (uint8_t *)calloc(1, 0x18 * sizeof(int16_t));
  if (!msg_buffer) {
      debug_print("Error: calloc for msg_buffer failed\n");
      return;
  }

  // Allocate 7 bytes for the heartbeat data structure
  heartbeat_data_ptr = (uint8_t *)calloc(1, 7 * sizeof(uint8_t));
  if (!heartbeat_data_ptr) {
      debug_print("Error: calloc for heartbeat_data_ptr failed\n");
      free(msg_buffer);
      return;
  }

  // Set specific bytes/words in msg_buffer
  msg_buffer[2] = 5; // *(uint8_t *)(msg_buffer + 2) = 5;
  *(int16_t *)(msg_buffer + sizeof(int16_t)) = (int16_t)-0xff; // msg_buffer[1] (as short) = -0xff;
  *(uint32_t *)(msg_buffer + 6 * sizeof(int16_t)) = *(uint32_t *)((uint8_t *)param_1 + 0xc);

  // Set the first byte of heartbeat_data_ptr
  heartbeat_data_ptr[0] = 0x42;

  // Calculate source pointer and data length
  // *(uint32_t *)(param_1 + 0x14) is likely a base address, +3 is an offset
  uint32_t base_addr_from_param = *(uint32_t *)((uint8_t *)param_1 + 0x14);
  src_data_ptr = (uint8_t *)(uintptr_t)(base_addr_from_param + 3); // Use uintptr_t for pointer arithmetic on integers
  data_length = *(uint16_t *)((uint8_t *)(uintptr_t)base_addr_from_param + 1);

  // Store data_length in heartbeat_data_ptr (at offset 1)
  *(uint16_t *)(heartbeat_data_ptr + 1) = data_length;

  if (data_length != 0) {
    allocated_data_buffer = calloc(1, data_length);
    if (!allocated_data_buffer) {
        debug_print("Error: calloc for allocated_data_buffer failed\n");
        // Free previously allocated memory before returning
        free(heartbeat_data_ptr);
        free(msg_buffer);
        return;
    }
    *(void **)(heartbeat_data_ptr + 3) = allocated_data_buffer; // Store pointer at byte offset 3
    debug_print("Reading heartbeats from %p to %p\n", src_data_ptr,
                (uint8_t *)src_data_ptr + data_length);
    memcpy(*(void **)(heartbeat_data_ptr + 3), src_data_ptr, data_length);
  }

  // Set fields in msg_buffer related to heartbeat_data_ptr
  *(int16_t *)(msg_buffer + 8 * sizeof(int16_t)) = data_length + 3;
  *(uint8_t **)(msg_buffer + 10 * sizeof(int16_t)) = heartbeat_data_ptr;
  *(int16_t *)msg_buffer = *(int16_t *)(msg_buffer + 8 * sizeof(int16_t)) + 0xf;

  debug_print("Sending HB response. Length: %d\n", data_length);
  msls_send_msg(msg_buffer);
  msls_destroy_msg(msg_buffer); // This should free msg_buffer and its nested allocations
  return;
}

// Function: msls_handle_changespec
void ** msls_handle_changespec(uint32_t param_1, void *param_2, void *param_3) {
  void **result_ptr = (void **)_GLOBAL_OFFSET_TABLE_; // Initialize with placeholder

  // Check if the pointer at param_3 + 0x14 is not NULL and param_2 is not NULL
  uint16_t **ptr_to_u16_ptr = (uint16_t **)((uint8_t *)param_3 + 0x14);
  
  // Ensure both the pointer to the pointer and the pointer itself are not NULL
  if (ptr_to_u16_ptr != NULL && *ptr_to_u16_ptr != NULL && param_2 != NULL) {
    // Dereference twice to get the uint16_t value
    uint16_t value_from_param3 = **ptr_to_u16_ptr;
    *(uint16_t *)((uint8_t *)param_2 + 0x10) = value_from_param3;
    result_ptr = msls_send_server_hello(param_2);
  }
  return result_ptr;
}

// Function: msls_handle_error
void msls_handle_error(void *param_1, void *param_2) {
  uint8_t error_type;
  uint16_t *error_data_ptr;
  int handler_result;

  // Extract function pointer from param_1's structure (likely a vtable)
  // *(uint32_t *)(param_1 + 4) gets a pointer to the vtable
  // + 0x2c gets the offset within the vtable for the specific function pointer
  uint32_t vtable_base = *(uint32_t *)((uint8_t *)param_1 + 4);
  handler_func func_ptr = (handler_func)*(uint32_t *)((uint8_t *)(uintptr_t)vtable_base + 0x2c);

  // Call the function pointer, passing param_1 and a value from param_2
  handler_result = func_ptr(param_1, *(uint32_t *)((uint8_t *)param_2 + 0xc));

  // Get the error data pointer from param_2
  error_data_ptr = *(uint16_t **)((uint8_t *)param_2 + 0x14);

  if (error_data_ptr != NULL) {
    error_type = *(uint8_t *)((uint8_t *)error_data_ptr + 1); // Get the byte at offset 1 from error_data_ptr
    uint16_t error_code = *error_data_ptr; // Get the short at offset 0

    if (error_type == 0xa2) {
      debug_print("Received Fatal Error 0x%x. Shutting down server\n", error_code);
      ((uint8_t *)param_1)[1] = 1; // Set a flag in param_1's structure
    } else if (error_type < 0xa3) {
        if (error_type == 0xa0) {
          debug_print("Received Client Warning 0x%x\n", error_code);
          return;
        }
        if (error_type == 0xa1) {
          if (handler_result == 0) {
            return;
          }
          debug_print("Received Critical Error 0x%x. Shutting down connection.\n", error_code);
          // Assuming handler_result is a pointer to a structure
          ((uint8_t *)(uintptr_t)handler_result)[0xc] = 0; // Set a flag in the structure pointed to by handler_result
          return;
        }
    }
    // Fall through for unknown error codes or if error_type >= 0xa3
    debug_print("Unknown Error Code 0x%x 0x%x\n", error_code, error_type);
  }
  return;
}