#include <stdint.h> // For uint32_t, size_t
#include <stdlib.h> // For malloc, free
#include <string.h> // For memcmp, memcpy
#include <unistd.h> // For _exit (used by _terminate)
#include <stdio.h>  // For printf (in mocks) and fprintf (in _terminate)

// Define custom types from the snippet
typedef unsigned char byte;
typedef unsigned int uint;
typedef unsigned short ushort;
typedef uint32_t undefined4; // 4-byte unsigned integer
typedef unsigned char undefined; // 1-byte unsigned char

// --- Global variables and external function mocks for compilation ---

// Global arrays for results (OK, ERR)
unsigned char OK[4] = {0};
unsigned char ERR[4] = {0};

// Mock base address for global data (0x4347c000).
// In a real scenario, this would point to a specific memory region or a global array.
// For compilation, we'll make it point to a static array.
// The size (0xfff + 1 = 4096 bytes) is inferred from the loop `local_c < 0xfff`.
static const unsigned char GLOBAL_DATA_STORAGE[4096] = {
    // Dummy data for compilation. In a real system, this would be actual program data.
    // Example: fill with a pattern for demonstration
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, /* ... */
};
const unsigned char *GLOBAL_DATA_BASE = GLOBAL_DATA_STORAGE;

// Global input type strings (assuming 4-byte comparison)
const unsigned char INPUT_TYPE_PLAIN[] = "PLAI"; // Or "PLAIN\0" if 5 bytes and memcmp is 4
const unsigned char INPUT_TYPE_SERIALIZED[] = "SERI"; // Or "SERIALIZED"

// Global pointer for allocated input buffer
void *in = NULL;

// Global buffer for `recv_all_no_args` (which takes no arguments)
// It's assumed to fill this buffer and return the count.
#define GLOBAL_RECV_BUFFER_SIZE 512 // Arbitrary size, large enough for 6 bytes + data
static unsigned char g_recv_buffer_temp[GLOBAL_RECV_BUFFER_SIZE];

// Mock external functions:

// `_terminate`: Exits the program due to an unrecoverable error.
void _terminate(void) {
    fprintf(stderr, "Program terminated due to error.\n");
    _exit(1); // Use _exit for immediate termination without cleanup
}

// `recv_all_no_args`: This is a mock. In a real system, it would perform I/O.
// It's assumed to fill `g_recv_buffer_temp` and return the number of bytes read.
// This mock simulates two calls: first for header, second for data.
size_t recv_all_no_args(void) {
    static int call_count = 0;
    call_count++;

    if (call_count == 1) {
        // Simulate receiving "PLAI" and length 10
        memcpy(g_recv_buffer_temp, INPUT_TYPE_PLAIN, 4);
        ushort mock_data_len = 10; // Example data length
        memcpy(g_recv_buffer_temp + 4, &mock_data_len, 2);
        return 6; // 4 bytes type + 2 bytes length
    } else if (call_count == 2) {
        // Simulate receiving 10 bytes of actual data
        memset(g_recv_buffer_temp, 'A', 10); // Dummy data 'AAAAAAAAAA'
        return 10; // The actual data length
    }
    return 0; // No more data to simulate
}

// `send`: Mock function for `send(int sockfd, const void *buf, size_t len, int flags)`
int send(int sockfd, const void *buf, size_t len, int flags) {
    printf("MOCK SEND: Socket %d, Data (hex): '", sockfd);
    for (size_t i = 0; i < len; ++i) {
        printf("%02x", ((unsigned char*)buf)[i]);
    }
    printf("', Length: %zu, Flags: %d\n", len, flags);
    return len; // Simulate sending all bytes
}

// Mock processing functions
int process_plain_input(void *input_buf) {
    printf("MOCK: Processing plain input from %p. Content: '%.*s'\n", input_buf, 4, (char*)input_buf);
    // Simulate some processing and return status
    return 0; // Success
}

int process_serialized_input(void *input_buf) {
    printf("MOCK: Processing serialized input from %p. Content: '%.*s'\n", input_buf, 4, (char*)input_buf);
    // Simulate some processing and return status
    return 0; // Success
}

// --- Fixed Functions ---

// Function: gen_result_bufs
void gen_result_bufs(void) {
  uint local_c;
  
  for (local_c = 0; local_c < 0xfff; local_c += 2) {
    // Perform XOR operations on global buffers OK and ERR using GLOBAL_DATA_BASE
    OK[local_c & 3] ^= GLOBAL_DATA_BASE[local_c];
    ERR[(local_c + 1) & 3] ^= GLOBAL_DATA_BASE[local_c + 1];
  }
}

// Function: receive_input
undefined4 receive_input(void) {
  unsigned char input_type_bytes[4]; // Buffer to hold the 4-byte input type
  ushort data_length;                // Variable to hold the 2-byte data length
  uint bytes_read_count;             // Stores the return value of recv_all_no_args

  // First call to recv_all_no_args to get the 6-byte header (4 bytes type + 2 bytes length)
  bytes_read_count = recv_all_no_args();
  if (bytes_read_count != 6) {
    _terminate(); // Error: expected 6 bytes for the header
  }

  // Copy input type and data length from the global receive buffer
  memcpy(input_type_bytes, g_recv_buffer_temp, 4);
  memcpy(&data_length, g_recv_buffer_temp + 4, 2);

  // Compare the received input type with known types
  if (memcmp(INPUT_TYPE_PLAIN, input_type_bytes, 4) != 0) {
    if (memcmp(INPUT_TYPE_SERIALIZED, input_type_bytes, 4) != 0) {
      return 0xffffffff; // Input type not recognized
    }
  }

  // Allocate memory for the full input data:
  // 4 bytes for the type field + 2 bytes for the length field + `data_length` for the actual payload.
  in = malloc(6 + data_length);
  if (in == NULL) {
    _terminate(); // Memory allocation failed
  }

  // Store the data length at offset 4 within the 'in' buffer
  // (Assuming 'in' buffer structure is: 0-3 type, 4-5 length, 6+ data payload)
  *(ushort *)((unsigned char *)in + 4) = data_length;

  // Copy the 4-byte input type into the 'in' buffer at offset 0
  memcpy(in, input_type_bytes, 4);

  // Second call to recv_all_no_args to get the actual data payload
  bytes_read_count = recv_all_no_args();
  if (bytes_read_count != data_length) {
    _terminate(); // Error: expected `data_length` bytes for the payload
  }

  // Copy the actual data payload from the global receive buffer into 'in' at offset 6
  memcpy((unsigned char *)in + 6, g_recv_buffer_temp, data_length);

  return 0; // Successfully received and structured input
}

// Function: main
undefined4 main(void) {
  int result_code;
  int status_value = 0x1092; // Initial status value

  // Mock socket descriptor (e.g., 1 for stdout, or a specific socket FD)
  int g_sockfd = 1;

  gen_result_bufs();

  while (1) {
    result_code = receive_input();

    if (result_code >= 0) { // If receive_input was successful (0 or positive)
      // Send the current status_value (4 bytes) to the mock socket
      send(g_sockfd, &status_value, sizeof(status_value), 0);

      // Determine input type by checking the first 4 bytes of 'in' and process
      if (memcmp(INPUT_TYPE_PLAIN, in, 4) == 0) {
        result_code = process_plain_input(in);
      } else {
        result_code = process_serialized_input(in);
      }
    }

    if (result_code != 0) {
      break; // Exit loop if processing failed (result_code is negative or non-zero error)
    }

    // Send a success status code (0x1708C as a 4-byte integer)
    int success_status_code = 0x1708C;
    send(g_sockfd, &success_status_code, sizeof(success_status_code), 0);

    free(in); // Free the allocated input buffer
    in = NULL; // Clear the global pointer after freeing
    status_value++; // Increment status counter for the next iteration
  }

  // If the loop exited due to an error, send an error status code (0x17090 as a 4-byte integer)
  int error_status_code = 0x17090;
  send(g_sockfd, &error_status_code, sizeof(error_status_code), 0);

  return 0; // Program exits after handling the error
}