#include <stdio.h>    // For printf (for dummy functions)
#include <stdlib.h>   // For exit, rand
#include <string.h>   // For memset, memcpy, memcmp
#include <stdint.h>   // For uint8_t, uint32_t
#include <sys/socket.h> // For send, recv (assuming recv_all is a wrapper)
#include <unistd.h>   // For close (not used but good practice for sockets)

// Global socket descriptor (assuming it's initialized elsewhere in a real program)
int g_client_socket_fd = -1; 

// --- Dummy / Placeholder Functions (replace with actual implementations) ---

// Placeholder for _terminate
void _terminate(int code) {
    fprintf(stderr, "Program terminated with code: 0x%x\n", code);
    exit(EXIT_FAILURE);
}

// Placeholder for recv_all
// In a real scenario, this would loop until all bytes are received or an error occurs.
ssize_t recv_all(void *buf, size_t len) {
    // Dummy implementation:
    // If a real socket is available, attempt to receive.
    // Otherwise, fill with dummy data or specific data for testing do_nonce.
    if (g_client_socket_fd != -1) {
        ssize_t bytes_received = 0;
        while (bytes_received < len) {
            ssize_t current_recv = recv(g_client_socket_fd, (uint8_t*)buf + bytes_received, len - bytes_received, 0);
            if (current_recv <= 0) {
                return current_recv; // Error or connection closed
            }
            bytes_received += current_recv;
        }
        return bytes_received;
    } else {
        // Dummy behavior for testing without a real socket
        // For do_nonce, return the expected challenge nonce for a successful comparison
        if (len == 8) {
            uint32_t* p = (uint32_t*)buf;
            p[0] = 0x34333231; // "1234"
            p[1] = 0x37363500; // "567\0"
        } else {
            memset(buf, 0xAA, len); // Fill with some arbitrary data
        }
        return len;
    }
}

// Dummy functions for do_build
char init_electric_model(uint32_t param) { return (char)(rand() % 2); }
char add_breaker_to_load_center(uint32_t param_1, uint8_t *param_2) { return (char)(rand() % 2); }
char add_outlet_to_breaker(uint32_t param_1, uint32_t param_2, uint8_t *param_3) { return (char)(rand() % 2); }
char add_n_way_splitter_to_receptacle(uint32_t param_1, uint32_t param_2, uint8_t *param_3) { return (char)(rand() % 2); }
char add_light_string_to_receptacle(uint32_t param_1, uint32_t param_2, uint8_t *param_3) { return (char)(rand() % 2); }

// Dummy functions for do_examine
uint32_t is_electrical_panel_overloaded(void) { return (uint32_t)(rand() % 2); }
uint32_t is_breaker_overloaded(uint32_t param) { return (uint32_t)(rand() % 2); }
uint32_t is_outlet_overloaded(uint32_t param) { return (uint32_t)(rand() % 2); }
uint32_t is_splitter_overloaded(uint32_t param) { return (uint32_t)(rand() % 2); }
uint32_t is_light_string_overloaded(uint32_t param) { return (uint32_t)(rand() % 2); }
uint32_t is_receptacle_overloaded(uint32_t param) { return (uint32_t)(rand() % 2); }

// --- Fixed Functions ---

// Function: do_send_results
// Assuming param_1 (from original snippet) is the buffer pointer.
// The size is inferred as 40 bytes (0x28) from call sites (local_44[40], local_40[10]*4).
// The socket descriptor `g_client_socket_fd` is assumed to be global.
void do_send_results(void *buffer_ptr) {
  // The original `send` call was `send(param_1,(void *)0x25,in_stack_ffffffec,0x1321e);`
  // This is highly problematic. We interpret `param_1` as the buffer,
  // `0x25` (37 bytes) as a likely misinterpreted size, and `0x1321e` as flags.
  // Using 40 bytes as the data size (consistent with `local_44` and `local_40`) and flags = 0.
  send(g_client_socket_fd, buffer_ptr, 40, 0);
}

// Function: recv_uint32
uint32_t recv_uint32(void) {
  uint32_t value = 0; // Initialize directly
  if (recv_all(&value, sizeof(value)) != sizeof(value)) {
    _terminate(0xffffffac); // -84
  }
  return value;
}

// Function: recv_uint8
uint8_t recv_uint8(void) {
  uint8_t value = 0; // Initialize directly
  if (recv_all(&value, sizeof(value)) != sizeof(value)) {
    _terminate(0xffffffac); // -84
  }
  return value;
}

// Function: do_build
int do_build(void) {
  uint8_t result_buffer[40];
  memset(result_buffer, 0, sizeof(result_buffer));

  int result = 0;
  uint32_t command = recv_uint32();
  uint32_t arg1 = 0;
  uint32_t arg2 = 0;

  switch(command) {
  case 0x418:
    arg1 = recv_uint32();
    result = init_electric_model(arg1);
    break;
  case 0x419:
    arg1 = recv_uint32();
    result = add_breaker_to_load_center(arg1, result_buffer);
    break;
  case 0x41a:
    arg1 = recv_uint32();
    arg2 = recv_uint32();
    result = add_outlet_to_breaker(arg1, arg2, result_buffer);
    break;
  case 0x41b:
    arg1 = recv_uint32();
    arg2 = recv_uint32();
    result = add_n_way_splitter_to_receptacle(arg1, arg2, result_buffer);
    break;
  case 0x41c:
    arg1 = recv_uint32();
    arg2 = recv_uint32();
    result = add_light_string_to_receptacle(arg1, arg2, result_buffer);
    break;
  default:
    result = -0x51; // -81
  }

  if ((result >= 0) && (command != 0x418)) {
    do_send_results(result_buffer);
  }
  return result;
}

// Function: do_examine
uint32_t do_examine(void) {
  uint32_t result_buffer[10];
  memset(result_buffer, 0, sizeof(result_buffer)); // Equivalent to original loop logic

  uint32_t result = 0;
  uint32_t command = recv_uint32();
  uint32_t arg1 = 0;

  switch(command) {
  case 0x801:
    result = is_electrical_panel_overloaded();
    result &= 0xff;
    break;
  case 0x802:
    arg1 = recv_uint32();
    result = is_breaker_overloaded(arg1);
    result &= 0xff;
    break;
  case 0x803:
    arg1 = recv_uint32();
    result = is_outlet_overloaded(arg1);
    result &= 0xff;
    break;
  case 0x804:
    arg1 = recv_uint32();
    result = is_splitter_overloaded(arg1);
    result &= 0xff;
    break;
  case 0x805:
    arg1 = recv_uint32();
    result = is_light_string_overloaded(arg1);
    result &= 0xff;
    break;
  case 0x806:
    arg1 = recv_uint32();
    result = is_receptacle_overloaded(arg1);
    result &= 0xff;
    break;
  default:
    result = 0xffffffaf; // -81
  }

  if ((int)result >= 0) {
    result_buffer[0] = result;
    do_send_results(result_buffer);
  }
  return result;
}

// Function: do_nonce
uint32_t do_nonce(void) {
  // The original code uses complex stack pointer arithmetic, which is typically decompiler artifact.
  // This re-implementation assumes the intent was to send an 8-byte nonce,
  // then receive an 8-byte nonce and compare them.

  // Challenge nonce (8 bytes)
  uint32_t challenge_nonce_val1 = 0x34333231; // "1234" (little-endian)
  uint32_t challenge_nonce_val2 = 0x37363500; // "567\0" (little-endian)
  uint8_t challenge_nonce[8];
  memcpy(challenge_nonce, &challenge_nonce_val1, sizeof(uint32_t));
  memcpy(challenge_nonce + sizeof(uint32_t), &challenge_nonce_val2, sizeof(uint32_t));

  // The original `rand()` call and check were present but the random value was not used in the nonce.
  // Keeping the check as per original logic.
  int rand_val = rand();
  if (rand_val < 0) {
    _terminate(0xffffffad); // -83
  }

  // Send the challenge nonce
  send(g_client_socket_fd, challenge_nonce, sizeof(challenge_nonce), 0);

  // Received nonce (8 bytes)
  uint8_t received_nonce[8];
  if (recv_all(received_nonce, sizeof(received_nonce)) != sizeof(received_nonce)) {
    _terminate(0xffffffac); // -84
  }

  // Compare the received nonce with the sent challenge nonce.
  // The original comparison `(char)local_18 == (char)local_20` only compared the first byte.
  // A full 8-byte comparison is performed here for correctness.
  if (memcmp(challenge_nonce, received_nonce, sizeof(challenge_nonce)) == 0) {
    return 0; // Success
  } else {
    return 0xffffffae; // -82 (Failure)
  }
}