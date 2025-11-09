#include <stdio.h> // Required for printf in the main function
#include <string.h> // Required for strlen in the mock_input_buffer (though not directly used in the provided snippet)

// Mock receive function for compilation purposes.
// In a real system, this would be provided by the OS or a library.
// This mock simulates reading from a static buffer.
static const char *mock_input_buffer = "hello world\nthis is a test input";
static int mock_buffer_pos = 0;

// receive function signature: int receive(int fd, byte *buf, int len, int *bytes_received)
// Using unsigned char for byte for standard C compatibility.
int receive(int fd, unsigned char *buf, int len, int *bytes_received) {
    if (fd != 0) { // Simulating only stdin (fd 0)
        *bytes_received = 0;
        return -1; // Invalid file descriptor
    }

    if (len <= 0) {
        *bytes_received = 0;
        return -1; // Invalid length
    }

    if (mock_input_buffer[mock_buffer_pos] == '\0') {
        *bytes_received = 0;
        return 1; // End of mock input, return non-zero to indicate no more data
    }

    *buf = (unsigned char)mock_input_buffer[mock_buffer_pos];
    mock_buffer_pos++;
    *bytes_received = 1;
    return 0; // Success
}

// Function: readUntil
// Using unsigned int for uint and unsigned char for byte for standard C compatibility.
unsigned int readUntil(unsigned char *param_1, unsigned int param_2, char param_3) {
  unsigned int i; // Loop counter, used for the return value
  unsigned char current_byte; // Stores the byte received
  int receive_status; // Stores the return value of the receive function
  int bytes_received_count; // Stores the number of bytes actually received

  for (i = 0; i < param_2; ++i) {
    receive_status = receive(0, &current_byte, 1, &bytes_received_count);
    
    // Check for errors from receive or if not exactly one byte was received
    if (receive_status != 0 || bytes_received_count != 1) {
      return 0xffffffff; // Return error code
    }
    
    // Check if the received byte matches the delimiter character
    if (current_byte == (unsigned char)param_3) {
      break; // Delimiter found, exit loop
    }
    
    // Store the received byte and advance the buffer pointer
    *param_1++ = current_byte;
  }
  
  // Null-terminate the buffer
  *param_1 = 0;
  return i; // Return the number of bytes read before the delimiter or buffer full
}

// Main function for compilation and testing
int main() {
    unsigned char buffer[100];
    unsigned int bytes_read;

    // Test 1: Read until 'o'
    printf("Test 1: Reading until 'o'\n");
    mock_buffer_pos = 0; // Reset mock input buffer position
    bytes_read = readUntil(buffer, sizeof(buffer) - 1, 'o');
    if (bytes_read == 0xffffffff) {
        printf("Error reading!\n");
    } else {
        printf("Read %u bytes: '%s'\n", bytes_read, buffer); // Expected: "hell"
    }

    // Test 2: Read until ' '
    printf("\nTest 2: Reading until ' '\n");
    mock_buffer_pos = 0; // Reset mock input buffer position
    bytes_read = readUntil(buffer, sizeof(buffer) - 1, ' ');
    if (bytes_read == 0xffffffff) {
        printf("Error reading!\n");
    } else {
        printf("Read %u bytes: '%s'\n", bytes_read, buffer); // Expected: "hello"
    }

    // Test 3: Buffer too small (max 3 bytes for content)
    printf("\nTest 3: Buffer too small (max 3 bytes), until 'o'\n");
    mock_buffer_pos = 0; // Reset mock input buffer position
    bytes_read = readUntil(buffer, 3, 'o'); // Max 3 chars + null terminator
    if (bytes_read == 0xffffffff) {
        printf("Error reading!\n");
    } else {
        printf("Read %u bytes: '%s'\n", bytes_read, buffer); // Expected: "hel" (stopped by param_2)
    }

    // Test 4: Delimiter 'z' not found within limit (10 bytes)
    printf("\nTest 4: Delimiter 'z' not found within limit (10 bytes)\n");
    mock_buffer_pos = 0; // Reset mock input buffer position
    bytes_read = readUntil(buffer, 10, 'z');
    if (bytes_read == 0xffffffff) {
        printf("Error reading!\n");
    } else {
        printf("Read %u bytes: '%s'\n", bytes_read, buffer); // Expected: "hello worl"
    }
    
    // Test 5: Read until newline
    printf("\nTest 5: Reading until '\\n'\n");
    mock_buffer_pos = 0; // Reset mock input buffer position
    bytes_read = readUntil(buffer, sizeof(buffer) - 1, '\n');
    if (bytes_read == 0xffffffff) {
        printf("Error reading!\n");
    } else {
        printf("Read %u bytes: '%s'\n", bytes_read, buffer); // Expected: "hello world"
    }

    return 0;
}