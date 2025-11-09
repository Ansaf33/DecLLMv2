#include <stdlib.h> // For realloc, free
#include <stdint.h> // For fixed-width integers
#include <stddef.h> // For size_t

// External function declarations (assuming they are defined elsewhere)
// These functions are placeholders for actual I/O and vault operations.
extern int read_bytes(void* buf, size_t count);
extern int write_bytes(const void* buf, size_t count);
extern void init_vault(void);
extern void store_in_vault(uint32_t idx, void* data, size_t size);
extern uint16_t swap16(uint16_t val);
extern uint32_t swap32(uint32_t val);

// Type definition for message handler functions
typedef uint32_t (*msg_handler_func)(void*, uint32_t);

// Global array of message handlers
// The size '3' is inferred from handle_msg's `param_1 < 3` check.
msg_handler_func handlers[3];

// Function: is_supported
uint32_t is_supported(int msg_type) {
  return (msg_type == 0 || msg_type == 1 || msg_type == 2);
}

// Function: consume_bytes
uint32_t consume_bytes(size_t bytes_to_consume) {
  uint8_t buffer[512]; // Using fixed-size buffer for chunks
  while (bytes_to_consume > 0) {
    size_t chunk_size = (bytes_to_consume < sizeof(buffer)) ? bytes_to_consume : sizeof(buffer);
    if (read_bytes(buffer, chunk_size) == 0) { // read_bytes returns 0 on error/EOF
      return 0; // Error or EOF
    }
    bytes_to_consume -= chunk_size;
  }
  return 1; // Success
}

// Function: handle_msg
uint32_t handle_msg(uint16_t msg_type, void* data, uint32_t size) {
  if (msg_type < (sizeof(handlers) / sizeof(handlers[0]))) {
    if (handlers[msg_type] == NULL) { // Check if handler is registered
      return 0;
    } else {
      return handlers[msg_type](data, size);
    }
  }
  return 0; // Unsupported message type
}

// Function: handle_msg_ping
uint32_t handle_msg_ping(void* data, size_t size) {
  if (size < 0x8000) { // Size fits in 16 bits
    // Header format: 2 bytes type (0), 2 bytes size
    struct __attribute__((packed)) {
      uint16_t type;
      uint16_t size;
    } header;
    header.type = swap16(0);
    header.size = swap16((uint16_t)size);
    write_bytes(&header, sizeof(header));
  } else { // Size requires 32 bits
    // Header format: 2 bytes type (0), 4 bytes size (MSB set)
    struct __attribute__((packed)) {
      uint16_t type;
      uint32_t size;
    } header;
    header.type = swap16(0);
    header.size = swap32((uint32_t)size | 0x80000000); // Set MSB for 32-bit size
    write_bytes(&header, sizeof(header));
  }
  write_bytes(data, size); // Write the actual payload
  return 1;
}

// Function: main
int main(void) {
  uint16_t msg_type;
  uint32_t msg_size;
  void* msg_buffer = NULL; // Buffer for message payload

  init_vault();
  // Initialize handlers. Example:
  handlers[0] = &handle_msg_ping;
  handlers[1] = NULL; // Example: no handler for type 1
  handlers[2] = NULL; // Example: no handler for type 2

  // Store the address of the handlers array in the vault.
  // The size 0xc (12 bytes) implies 3 pointers of 4 bytes each,
  // which is typical for 32-bit systems.
  store_in_vault(0, handlers, sizeof(handlers));

  while (1) { // Main message processing loop
    // Read the initial 4-byte header prefix
    union {
      uint8_t bytes[4];
      struct __attribute__((packed)) {
        uint16_t type;
        uint16_t size_prefix;
      } fields;
    } header_prefix;

    if (read_bytes(&header_prefix, sizeof(header_prefix)) == 0) {
      // If read fails (EOF or error), exit loop
      break;
    }

    msg_type = swap16(header_prefix.fields.type);
    uint32_t size_val = swap16(header_prefix.fields.size_prefix);
    msg_size = size_val & 0xffff; // Initial 16 bits of size

    if ((size_val & 0x8000) != 0) { // Check if MSB is set, indicating a 32-bit size
      uint16_t size_upper_half_net;
      if (read_bytes(&size_upper_half_net, sizeof(size_upper_half_net)) == 0) {
        // If read fails, exit loop
        break;
      }
      // Combine the lower 16 bits (size_val) with the newly read upper 16 bits
      uint32_t full_size_net = ((uint32_t)size_upper_half_net << 16) | size_val;
      msg_size = swap32(full_size_net) & 0x7fffffff; // Clear the MSB as it's a flag
    }

    int proceed_with_payload = 1; // Flag to determine if payload should be processed

    if (!is_supported(msg_type)) {
      proceed_with_payload = 0; // Message type not supported
    } else {
      void* realloc_ptr = realloc(msg_buffer, msg_size);
      if (realloc_ptr == NULL) {
        proceed_with_payload = 0; // realloc failed
      } else {
        msg_buffer = realloc_ptr; // Update buffer pointer on success
      }
    }

    if (!proceed_with_payload) {
      // If message type is not supported or realloc failed,
      // consume the payload bytes to maintain stream synchronization.
      if (consume_bytes(msg_size) == 0) {
        // If consuming bytes fails, something is wrong, terminate.
        break;
      }
      continue; // Go to the next message
    }

    // If we reach here, msg_type is supported and msg_buffer is allocated.
    if (read_bytes(msg_buffer, msg_size) == 0) {
      // If reading the message payload fails, exit loop
      break;
    }

    handle_msg(msg_type, msg_buffer, msg_size);
    // No explicit error handling for handle_msg result; loop continues.
  } // End of while(1) loop

  free(msg_buffer); // Free any allocated buffer
  return 0; // Main always returns 0
}