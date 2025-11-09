#include <stdint.h>  // For uint32_t, uint8_t, uintptr_t
#include <string.h>  // For memcpy, memset
#include <stdbool.h> // For bool
#include <stddef.h>  // For size_t

// Dummy declarations for external functions.
// These signatures are inferred from their usage in the provided snippet.
uint32_t send_bytes(uint32_t fd, void *buf, uint32_t len, uint32_t flags);
int recv_bytes(uint32_t fd, void *buf, uint32_t len);
int allocate(uint32_t size, uint32_t count, void **out_ptr);
uint32_t static_switch(uint32_t param_1);

// Global variables (based on decompiler output and usage).
// rx_buf_cursor points within rx_buf or to &func_ptr as a sentinel.
uint8_t *rx_buf_cursor;
// rx_buf size is inferred from `0x7d04cU - (int)rx_buf_cursor < 0x1d` condition.
// This suggests the buffer spans up to address 0x7d04C, making its size 0x7d04C bytes.
uint8_t rx_buf[0x7d04C];
// func_ptr is used as a uint32_t value and its address is used as a sentinel.
uint32_t func_ptr;
// buf_scratch is a pointer to a buffer allocated by `allocate`.
uint8_t *buf_scratch;
// DAT_0007d038 is a global 32-bit integer variable.
uint32_t DAT_0007d038;
// DAT_0007d034 is a global function pointer.
uint32_t (*DAT_0007d034)(int, uint32_t *);

// Struct to represent the incoming packet data.
// Field names are descriptive based on common packet structures and usage patterns.
// The main function's local variables (local_30, local_2c, local_28, local_24, local_20)
// correspond to the first few fields of this struct when `packet_recv` writes to its beginning.
typedef struct {
    uint32_t type;              // Offset 0x0, e.g., command type
    uint32_t arg1;              // Offset 0x4, e.g., a general argument
    uint32_t op_code;           // Offset 0x8, e.g., operation code for dispatch
    uint32_t flag_c;            // Offset 0xc, e.g., a flag or mode setting
    uint32_t flag_10;           // Offset 0x10, e.g., another flag or mode setting
    uint32_t arg2_ptr_or_val;   // Offset 0x14, can be a value or a pointer
    uint32_t arg3_ptr_or_val;   // Offset 0x18, can be a value or a pointer
} PacketData; // Total size 0x1c (7 * 4 bytes)

// Function: access_check
// Checks if `param_1` and `param_1 + 3` are both entirely outside the range [`param_2`, `param_3`].
// Returns 0 if true (both outside), 1 otherwise.
uint32_t access_check(uint32_t param_1, uint32_t param_2, uint32_t param_3) {
  bool is_param1_outside = (param_1 < param_2) || (param_3 < param_1);
  bool is_param1_plus3_outside = (param_1 + 3 < param_2) || (param_3 < param_1 + 3);

  return (is_param1_outside && is_param1_plus3_outside) ? 0 : 1;
}

// Function: packet_send
// Sends a packet of data. `param_1` is expected to be a pointer to the data.
uint32_t packet_send(void *param_1) {
  return send_bytes(1, param_1, 0x1c, 0x1104d);
}

// Function: packet_recv
// Receives a packet into `param_1`. Manages the `rx_buf` and `rx_buf_cursor`.
int packet_recv(void *param_1) {
  // Check if the remaining buffer space in `rx_buf` is less than 0x1d bytes.
  if ((uintptr_t)(rx_buf + sizeof(rx_buf)) - (uintptr_t)rx_buf_cursor < 0x1d) {
    // If `rx_buf_cursor` points to the address of `func_ptr`, it's a special sentinel state
    // indicating the buffer needs to be refilled entirely.
    if (rx_buf_cursor == (uint8_t *)&func_ptr) {
      int result = recv_bytes(0, rx_buf, 0x1c);
      if (result != 0) {
        return result;
      }
      rx_buf_cursor = rx_buf; // Reset cursor to the beginning of the buffer.
    }
    // Copy 0x1c bytes from `rx_buf_cursor` into `param_1`.
    memcpy(param_1, rx_buf_cursor, 0x1c);
    rx_buf_cursor += 0x1c; // Advance cursor by the size of the packet.
  }
  return 0; // Success
}

// Function: do_normal
// Processes a normal request based on the packet data.
// `param_1` is a pointer to the `PacketData` struct.
// `param_2` is a pointer to a `uint32_t` array for response data.
uint32_t do_normal(int param_1, uint32_t *param_2) {
  PacketData *packet = (PacketData *)param_1;

  if (packet->flag_c == 0) {
    if ((packet->flag_10 == 1) &&
        (access_check(packet->arg3_ptr_or_val, 0x4347c000, 0x4347d000) == 1)) {
      param_2[0] = 2; // Set error status in response.
      return 0;
    }
    // Assign a value to the 6th element of the response array (index 5).
    param_2[5] = (packet->flag_10 == 0) ? packet->arg3_ptr_or_val : *(uint32_t *)(uintptr_t)packet->arg3_ptr_or_val;
  } else {
    if (packet->flag_10 != 0) {
      param_2[0] = 2; // Set error status.
      return 0;
    }
    if ((access_check(packet->arg2_ptr_or_val, 0x8048000, 0x8146000) == 0) &&
        (access_check(packet->arg2_ptr_or_val, 0x8146000, 0x816b000) == 0)) {
      param_2[0] = 2; // Set error status.
      return 0;
    }
    // Dereference `arg2_ptr_or_val` as a pointer and assign `arg3_ptr_or_val` to it.
    *(uint32_t *)(uintptr_t)packet->arg2_ptr_or_val = packet->arg3_ptr_or_val;
  }
  param_2[0] = 1; // Set success status.
  return 0;
}

// Function: do_scratch
// Processes a scratchpad memory request based on the packet data.
// `param_1` is a pointer to the `PacketData` struct.
// `param_2` is a pointer to a `uint32_t` array for response data.
uint32_t do_scratch(int param_1, uint32_t *param_2) {
  PacketData *packet = (PacketData *)param_1;
  uint32_t val_c = 0;
  uint32_t val_10 = 0;

  if ((packet->flag_c != 0) || (packet->flag_10 != 0)) {
    if ((packet->flag_c == 0) && (packet->flag_10 == 1)) {
      val_c = packet->arg3_ptr_or_val;
    } else if ((packet->flag_c == 1) && (packet->flag_10 == 0)) {
      val_c = packet->arg2_ptr_or_val;
    } else {
      if (packet->flag_c != 1) {
        param_2[0] = 2; // Set error status.
        return 0;
      }
      if (packet->flag_10 != 1) {
        param_2[0] = 2; // Set error status.
        return 0;
      }
      val_c = packet->arg3_ptr_or_val;
      val_10 = packet->arg2_ptr_or_val;
    }
  }

  // Check access for `val_c` within the `buf_scratch` region.
  if ((val_c != 0) &&
      (access_check(val_c, (uint32_t)(uintptr_t)buf_scratch, (uint32_t)(uintptr_t)(buf_scratch + 0x1000)) == 0)) {
    param_2[0] = 2;
    return 0;
  }
  // Check access for `val_10` within the `buf_scratch` region.
  if ((val_10 != 0) &&
      (access_check(val_10, (uint32_t)(uintptr_t)buf_scratch, (uint32_t)(uintptr_t)(buf_scratch + 0x1000)) == 0)) {
    param_2[0] = 2;
    return 0;
  }

  uint32_t uVar_val;
  if (packet->flag_c == 0) {
    uVar_val = (packet->flag_10 == 0) ? packet->arg3_ptr_or_val : *(uint32_t *)(uintptr_t)packet->arg3_ptr_or_val;
    param_2[5] = uVar_val;
  } else {
    uVar_val = (packet->flag_10 == 0) ? packet->arg3_ptr_or_val : *(uint32_t *)(uintptr_t)packet->arg3_ptr_or_val;
    *(uint32_t *)(uintptr_t)packet->arg2_ptr_or_val = uVar_val;
  }
  param_2[0] = 1; // Set success status.
  return 0;
}

// Function: do_static
// Processes a static memory access request.
// `param_1` is a pointer to the `PacketData` struct.
// `param_2` is a pointer to a `uint32_t` array for response data.
uint32_t do_static(int param_1, uint32_t *param_2) {
  PacketData *packet = (PacketData *)param_1;

  if (packet->flag_c == 0) {
    if (packet->arg3_ptr_or_val < 0xfffd) {
      param_2[5] = static_switch(packet->arg3_ptr_or_val);
      param_2[0] = 1; // Set success status.
    } else {
      param_2[0] = 2; // Set error status.
    }
  } else {
    param_2[0] = 2; // Set error status.
  }
  return 0;
}

// Function: bad_request
// Handles an invalid or disallowed request.
// `param_1` is unused.
// `param_2` is a pointer to a `uint32_t` array for response data.
uint32_t bad_request(uint32_t param_1, uint32_t *param_2) {
  (void)param_1; // Suppress unused parameter warning.
  param_2[0] = 2; // Set error status.
  return 0;
}

// Function: init
// Initializes global variables and allocates scratch memory.
// The original `EVP_PKEY_CTX *ctx` parameter was unused and has been removed.
int init(void) {
  rx_buf_cursor = (uint8_t *)&func_ptr; // Initialize cursor to a sentinel address.
  return allocate(0x1000, 1, (void **)&buf_scratch); // Allocate 0x1000 bytes for buf_scratch.
}

// Function: main
// Main loop for packet processing.
int main(void) {
  int init_status;
  PacketData received_packet;       // Buffer for incoming packet data.
  uint32_t response_packet_data[7]; // Buffer for outgoing response data (0x1c bytes = 7 * uint32_t).

  init_status = init(); // Initialize the system.
  if (init_status == 0) {
    while (true) {
      func_ptr = 0xdeadbeef;    // Set sentinel value.
      DAT_0007d038 = 0xaaaaaaaa; // Set another global value.

      init_status = packet_recv(&received_packet); // Receive a packet.
      // Break loop if packet reception fails or if the packet type is 3.
      if (init_status != 0 || received_packet.type == 3) {
        break;
      }

      // Initialize response buffer and copy `arg1` from received packet.
      memset(response_packet_data, 0, sizeof(response_packet_data));
      response_packet_data[1] = received_packet.arg1;

      // Determine if dispatching the request is allowed based on flags.
      bool dispatch_allowed_flags =
          ((received_packet.type == 0) &&
           ((received_packet.flag_c == 0 || received_packet.flag_c == 1))) &&
          ((received_packet.flag_10 == 0 || received_packet.flag_10 == 1));

      if (dispatch_allowed_flags) {
        // Dispatch the request based on `op_code` using a switch statement (no goto).
        switch (received_packet.op_code) {
          case 0:
            DAT_0007d034 = do_normal;
            break;
          case 1:
            DAT_0007d034 = do_scratch;
            break;
          case 2:
            DAT_0007d034 = do_static;
            break;
          default:
            DAT_0007d034 = bad_request; // Default to bad_request for unknown op_codes.
            break;
        }

        // Call the dispatched function.
        init_status = (*DAT_0007d034)((int)&received_packet, response_packet_data);
        if (init_status != 0) {
          return init_status; // Exit if the dispatched function returns an error.
        }
      } else {
        response_packet_data[0] = 2; // Set error status for disallowed dispatch.
      }

      // Send the response packet.
      int send_result = packet_send(response_packet_data);
      if (send_result != 0) {
        return send_result; // Exit if sending fails.
      }
      init_status = 0; // Reset status for the next iteration.
    }
  }
  return init_status; // Return final status from init or loop exit.
}