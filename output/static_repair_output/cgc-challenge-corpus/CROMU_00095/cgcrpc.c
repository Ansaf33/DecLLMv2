#include <stdio.h>    // For potential debugging, not strictly required by the snippet
#include <stdlib.h>   // For calloc, free, rand, srand
#include <string.h>   // For strcpy, strcmp, memcpy
#include <stdint.h>   // For uint32_t, uint16_t, uint8_t, int16_t
#include <time.h>     // For time (to seed rand)

// --- Type definitions for Ghidra's 'undefined' types ---
typedef uint8_t  undefined;
typedef uint16_t undefined2;
typedef uint32_t undefined4; // Assuming 32-bit system where pointers are 4 bytes.

// --- Global data structures and constants ---

// Define the structure for an endpoint entry
// Total size 0x48 (72 bytes)
#define MAX_ENDPOINTS 10
#define ENDPOINT_ENTRY_SIZE 0x48
#define ENDPOINT_NAME_OFFSET 0x00
#define ENDPOINT_PORT_OFFSET 0x40
#define ENDPOINT_HANDLER_OFFSET 0x44

// Function pointer type for endpoint handlers
typedef void (*EndpointHandlerFunc)(int16_t param_1, void* param_2, uint16_t param_3, void** param_4, uint16_t* param_5);

// Using a raw char array to match the original code's direct memory access patterns
// where `endpoints` refers to this buffer directly.
static char g_endpoints_buffer[MAX_ENDPOINTS * ENDPOINT_ENTRY_SIZE];

// Define the structure for a connection
#define MAX_CONNECTIONS 10
struct Connection {
    uint32_t id;         // 0x00
    void* endpoint_data; // 0x04, pointer to an EndpointEntry within g_endpoints_buffer
};
static struct Connection g_connections[MAX_CONNECTIONS];

// --- Placeholder for NETSTUFF_Handler ---
void NETSTUFF_Handler(int16_t arg1, void* arg2, uint16_t arg3, void** arg4, uint16_t* arg5) {
    // Placeholder implementation
    // For now, just set some dummy values to avoid crashes.
    if (arg4) *arg4 = NULL; // No data to return
    if (arg5) *arg5 = 0;    // No data length
}

// --- Helper function: random_in_range ---
uint32_t random_in_range(uint32_t min, uint32_t max) {
    // Seed the random number generator once
    static int initialized = 0;
    if (!initialized) {
        srand(time(NULL));
        initialized = 1;
    }
    if (min > max) { // Swap if min is greater than max
        uint32_t temp = min;
        min = max;
        max = temp;
    }
    // Generate a random number between min and max (inclusive)
    return min + (rand() % (max - min + 1));
}

// Function: InitializeCGCRPC
void InitializeCGCRPC(void) {
  // Access g_endpoints_buffer directly as a raw memory area
  // strcpy to offset 0 (start of first endpoint entry's name)
  strcpy(&g_endpoints_buffer[0], "NETSTUFF");
  // Access offset 0x64 (100) as a uint16_t
  *(uint16_t*)(&g_endpoints_buffer[0x64]) = 0xdc;
  // Access offset 0x68 (104) as an EndpointHandlerFunc pointer
  *(EndpointHandlerFunc*)(&g_endpoints_buffer[0x68]) = NETSTUFF_Handler;
}

// Function: AddConnection
uint32_t AddConnection(void* endpoint_data_ptr) {
  for (int i = 0; i < MAX_CONNECTIONS; ++i) {
    if (g_connections[i].id == 0) { // Found an empty slot
      g_connections[i].id = random_in_range(1, 0xffffffff);
      g_connections[i].endpoint_data = endpoint_data_ptr;
      return g_connections[i].id;
    }
  }
  return 0; // No available connection slots
}

// Function: BindEndpoint
uint32_t BindEndpoint(char *param_1, int16_t param_2) {
  for (int i = 0; i < MAX_ENDPOINTS; ++i) {
    char* current_endpoint_base = &g_endpoints_buffer[i * ENDPOINT_ENTRY_SIZE];
    // Check if endpoint name is not empty and matches param_1
    if (current_endpoint_base[ENDPOINT_NAME_OFFSET] != '\0' &&
        strcmp(&current_endpoint_base[ENDPOINT_NAME_OFFSET], param_1) == 0 &&
        *(int16_t*)(&current_endpoint_base[ENDPOINT_PORT_OFFSET]) == param_2) {
      // Found a matching endpoint, add a connection to it
      return AddConnection(current_endpoint_base);
    }
  }
  return 0; // No matching endpoint found
}

// Function: LookupBindID
void* LookupBindID(uint32_t param_1) {
  if (param_1 != 0) {
    for (int i = 0; i < MAX_CONNECTIONS; ++i) {
      if (param_1 == g_connections[i].id) {
        return g_connections[i].endpoint_data;
      }
    }
  }
  return NULL; // No matching bind ID found
}

// Function: HandleCGCRPCMessage
// Renamed param_3 to out_response_ptr and param_4 to out_response_len_ptr for clarity
uint32_t HandleCGCRPCMessage(uint16_t *param_1, uint32_t param_2, undefined4 *out_response_ptr, int *out_response_len_ptr) {
  if (!param_1) {
    return 0xffffffff;
  }
  if (param_2 < 2) {
    return 2; // Message too short to contain message type
  }

  uint16_t message_type = *param_1;

  if (message_type == 0xa0) { // BindEndpoint message
    if (param_2 < 4) { // Need at least 2 bytes for type + 2 bytes for string_len
      return 2;
    }
    uint16_t string_len = param_1[1];
    // Total expected size: 2 (type) + 2 (string_len) + string_len + 2 (port)
    if (param_2 != (4 + string_len + 2) || string_len == 0) {
      return 2; // Mismatched length or zero length string
    }

    // Allocate buffer for the string + null terminator
    char* endpoint_name_buf = (char*)calloc(string_len + 1, sizeof(char));
    if (!endpoint_name_buf) return 0xffffffff; // Allocation failed

    // Copy string data (param_1 + 2 points to start of string)
    memcpy(endpoint_name_buf, param_1 + 2, string_len);
    // Null terminator is already handled by calloc

    // Get the port value (int16_t after the string data)
    int16_t port = *(int16_t*)((char*)(param_1 + 2) + string_len);

    uint32_t bind_id = BindEndpoint(endpoint_name_buf, port);
    free(endpoint_name_buf); // Free temporary buffer

    if (bind_id == 0) {
      return 8; // Binding failed
    }

    // Prepare response buffer (7 bytes total)
    char *response_buf_bytes = (char*)calloc(7, sizeof(char));
    if (!response_buf_bytes) return 0xffffffff; // Allocation failed

    *out_response_ptr = (undefined4)(uintptr_t)response_buf_bytes; // Return pointer to response buffer
    *(uint16_t*)response_buf_bytes = 10;         // Response type (2 bytes at offset 0)
    response_buf_bytes[2] = 0;                   // Some byte field (1 byte at offset 2)
    *(uint32_t*)(response_buf_bytes + 3) = bind_id; // Bind ID (4 bytes at offset 3)
    *out_response_len_ptr = 7;                   // Total response size in bytes

  } else if (message_type == 0xa1) { // CallEndpoint message
    if (param_2 < 10) { // Need 2 (type) + 4 (bind_id) + 2 (func_id) + 2 (data_len)
      return 2;
    }
    uint32_t bind_id_val = *(uint32_t*)(param_1 + 1); // Bind ID (4 bytes from param_1[1] and param_1[2])
    int16_t function_id = param_1[3];
    uint16_t data_len = param_1[4];

    // Pointer to the start of the data payload
    void *data_payload_ptr = (void*)(param_1 + 5);

    // Total expected size: 2 (type) + 4 (bind_id) + 2 (func_id) + 2 (data_len) + data_len
    if (param_2 != (10 + data_len)) {
      return 2; // Mismatched data length
    }

    void* endpoint_data_ptr = LookupBindID(bind_id_val);
    if (!endpoint_data_ptr) {
      return 8; // Invalid bind ID
    }

    void* handler_return_data = NULL;
    uint16_t handler_return_len = 0;

    // Get the handler function pointer from the endpoint data
    EndpointHandlerFunc handler = *(EndpointHandlerFunc*)((char*)endpoint_data_ptr + ENDPOINT_HANDLER_OFFSET);

    // Call the handler
    handler(function_id, data_payload_ptr, data_len, &handler_return_data, &handler_return_len);

    // Limit return length
    if (handler_return_len > 0x800) {
      handler_return_len = 0x800;
    }

    // Prepare response buffer (8 bytes header + handler_return_len data)
    char *response_buf_bytes = (char*)calloc(8 + handler_return_len, sizeof(char));
    if (!response_buf_bytes) {
        free(handler_return_data); // Clean up if allocation for response fails
        return 0xffffffff;
    }

    *out_response_ptr = (undefined4)(uintptr_t)response_buf_bytes; // Return pointer to response buffer
    *(uint16_t*)response_buf_bytes = 0x1a;         // Response type (2 bytes at offset 0)
    *(uint32_t*)(response_buf_bytes + 2) = bind_id_val; // Bind ID (4 bytes at offset 2)
    *(uint16_t*)(response_buf_bytes + 6) = handler_return_len; // Data length (2 bytes at offset 6)

    // Copy handler return data if any
    if (handler_return_data && handler_return_len > 0) {
      memcpy(response_buf_bytes + 8, handler_return_data, handler_return_len);
    }
    free(handler_return_data); // Free handler's allocated data

    *out_response_len_ptr = 8 + handler_return_len; // Total response size in bytes

  } else {
    return 4; // Unknown message type
  }

  return 0; // Success
}