#include <unistd.h> // For size_t, ssize_t
#include <stdint.h> // For uint8_t, uint32_t (if needed, but unsigned int is used)
#include <stdio.h>  // For printf
#include <string.h> // For memset (in mock)

// Define types as specified by typical usage in similar contexts
typedef unsigned int uint;
typedef unsigned char byte;

// Mock implementation of receive function for compilation
// In a real scenario, this would be a system call (e.g., recv, read).
// Signature: int receive(int fd, byte *buf, uint len, int *bytes_received);
// Returns 0 on success, non-zero on error.
// The fd parameter is ignored in this mock.
// The len parameter is expected to be 1 for this use case.

// Static buffer to simulate input data
static const byte mock_input_buffer[] = "Hello World!\nThis is a test.\n";
static size_t mock_input_pos = 0;
static const size_t mock_input_len = sizeof(mock_input_buffer) - 1; // Exclude null terminator

int receive(int fd, byte *buf, uint len, int *bytes_received) {
    if (len != 1) {
        if (bytes_received) *bytes_received = 0;
        return -1; // Indicate error
    }

    if (mock_input_pos < mock_input_len) {
        *buf = mock_input_buffer[mock_input_pos++];
        if (bytes_received) *bytes_received = 1;
        return 0; // Success
    } else {
        // No more data
        if (bytes_received) *bytes_received = 0;
        return -1; // Indicate end of file/error
    }
}

// Function: readUntil
uint readUntil(byte *param_1, uint param_2, char param_3) {
  byte received_byte;
  int bytes_read_from_call;
  unsigned int i;

  for (i = 0; i < param_2; ++i) {
    if (receive(0, &received_byte, 1, &bytes_read_from_call) != 0) {
      return 0xffffffff;
    }
    if (bytes_read_from_call != 1) {
      return 0xffffffff;
    }
    
    if (received_byte == (byte)param_3) {
      break;
    }
    
    *param_1 = received_byte;
    param_1++;
  }
  
  *param_1 = 0;
  
  return i;
}

// Main function for testing
int main() {
    byte buffer[50];
    uint result;

    printf("--- Test Case 1: Read until 'W' ---\n");
    mock_input_pos = 0; // Reset mock input
    result = readUntil(buffer, sizeof(buffer) - 1, 'W');
    if (result == 0xffffffff) {
        printf("Error during readUntil.\n");
    } else {
        printf("Read %u bytes: '%s'\n", result, buffer);
    }

    printf("\n--- Test Case 2: Read until '\\n' ---\n");
    mock_input_pos = 0; // Reset mock input
    result = readUntil(buffer, sizeof(buffer) - 1, '\n');
    if (result == 0xffffffff) {
        printf("Error during readUntil.\n");
    } else {
        printf("Read %u bytes: '%s'\n", result, buffer);
    }

    printf("\n--- Test Case 3: Read more than available ---\n");
    mock_input_pos = 0; // Reset mock input
    result = readUntil(buffer, 5, 'X');
    if (result == 0xffffffff) {
        printf("Error during readUntil.\n");
    } else {
        printf("Read %u bytes: '%s'\n", result, buffer);
    }

    printf("\n--- Test Case 4: Terminator is first char ---\n");
    mock_input_pos = 0; // Reset mock input
    result = readUntil(buffer, sizeof(buffer) - 1, 'H');
    if (result == 0xffffffff) {
        printf("Error during readUntil.\n");
    } else {
        printf("Read %u bytes: '%s'\n", result, buffer);
    }
    
    printf("\n--- Test Case 5: Read until end of mock buffer ---\n");
    mock_input_pos = 0; // Reset mock input
    result = readUntil(buffer, sizeof(buffer) - 1, '.');
    if (result == 0xffffffff) {
        printf("Error during readUntil.\n");
    } else {
        printf("Read %u bytes: '%s'\n", result, buffer);
    }

    printf("\n--- Test Case 6: Buffer too small for content before terminator ---\n");
    mock_input_pos = 0; // Reset mock input
    result = readUntil(buffer, 5, '\n');
    if (result == 0xffffffff) {
        printf("Error during readUntil.\n");
    } else {
        printf("Read %u bytes: '%s'\n", result, buffer);
    }

    return 0;
}