#include <stdio.h>   // For printf (in main and mock receive)
#include <stdlib.h>  // For exit (not used but common)
#include <string.h>  // For memcpy (in mock receive)

// Assuming 'uint' means unsigned int
typedef unsigned int uint;

// Global variables referenced in the snippet
char rxbuf[0x80]; // 128 bytes buffer
int rxidx = 0;    // Current read index in rxbuf
int rxcnt = 0;    // Number of valid bytes in rxbuf

// Mock receive function
// This function simulates receiving data into rxbuf.
// In a real application, this would be a system call like `read` or `recv`.
// param_1: file descriptor (unused in this mock)
// param_2: buffer to fill (rxbuf in this context)
// param_3: maximum length to read (0x80 in this context)
// param_4: an int array where param_4[0] will store the actual bytes received.
int receive(int fd, char *buffer, int max_len, int *info_array) {
    static int call_count = 0;
    call_count++;

    // Simulate different data reception scenarios
    if (call_count == 1) {
        const char *data_chunk = "Hello World\nThis is a test message.\n";
        int len = strlen(data_chunk);
        if (len > max_len) len = max_len;
        memcpy(buffer, data_chunk, len);
        info_array[0] = len;
        printf("[MOCK] receive: Sent %d bytes (first call)\n", len);
        return 0; // Success
    } else if (call_count == 2) {
        const char *data_chunk = "Another chunk of data for reading.\n";
        int len = strlen(data_chunk);
        if (len > max_len) len = max_len;
        memcpy(buffer, data_chunk, len);
        info_array[0] = len;
        printf("[MOCK] receive: Sent %d bytes (second call)\n", len);
        return 0; // Success
    } else {
        info_array[0] = 0; // No more data
        printf("[MOCK] receive: No more data.\n");
        return 0; // Success, but 0 bytes received
    }
}

// Function: next_byte
// Reads the next byte from the receive buffer or attempts to fill it.
// Returns the byte as an unsigned int, or 0xFFFFFFFF on error/end of stream.
uint next_byte(void) {
  // If there are buffered bytes available
  if (rxidx < rxcnt) {
    return (uint)rxbuf[rxidx++]; // Return current byte and advance index
  }
  // No buffered bytes, try to receive more data
  else {
    int receive_info[2]; // Array to store receive metadata; receive_info[0] holds bytes count
    // Call the external receive function
    int status = receive(0, rxbuf, sizeof(rxbuf), receive_info);

    // Check if receive was successful and new bytes were actually received
    if ((status == 0) && (receive_info[0] != 0)) {
      rxcnt = receive_info[0]; // Update total count of bytes in buffer
      rxidx = 1;               // Set index to 1, as rxbuf[0] is about to be returned
      return (uint)rxbuf[0];   // Return the first byte of the newly received data
    } else {
      // Error during receive or no data available
      return 0xffffffff;
    }
  }
}

// Function: read_until
// Reads bytes into a buffer until a specific terminator character is found,
// the buffer is full, or an error occurs.
// param_1: Unused parameter (originally 'undefined4')
// param_2: Pointer to the buffer to fill
// param_3: Maximum number of characters to read (including the terminator if found,
//          but the terminator itself will be replaced by a null byte).
// param_4: The terminator character
// Returns the number of bytes read (including the position of the null-terminator),
// or -1 on error.
int read_until(int param_1, char *param_2, uint param_3, char param_4) {
  char *current_pos = param_2;
  uint bytes_read = 0; // Counter for bytes successfully written to buffer

  while (1) {
    // Check if the buffer's capacity (param_3) has been reached.
    // If bytes_read equals param_3, it means param_3 characters have already been written.
    // The original logic dictates overwriting the last character with null.
    if (bytes_read == param_3) {
      if (param_3 > 0) { // Ensure param_3 is positive before accessing current_pos[-1]
        current_pos[-1] = '\0'; // Overwrite the last written character with null
      }
      return (int)(current_pos - param_2); // Return total length (param_3)
    }

    int byte_val = next_byte(); // Read the next byte
    if (byte_val == -1) {
      return -1; // Error or end of stream
    }
    *current_pos = (char)byte_val; // Store the byte in the buffer

    // Check if the current byte is the terminator
    if (param_4 == *current_pos) {
      current_pos++;         // Advance pointer past the terminator's position
      current_pos[-1] = '\0'; // Overwrite the terminator character with null
      return (int)(current_pos - param_2); // Return length including terminator's original position
    }
    current_pos++;
    bytes_read++;
  }
}

// Function: read_n
// Reads a specified number of bytes into a buffer.
// param_1: Unused parameter (originally 'undefined4')
// param_2: Pointer to the buffer to fill
// param_3: The exact number of bytes to read
// Returns the number of bytes read (param_3 if successful), or -1 on error.
int read_n(int param_1, char *param_2, uint param_3) {
  char *current_pos = param_2;
  uint bytes_read = 0; // Counter for bytes successfully written to buffer

  while (bytes_read < param_3) { // Loop until 'param_3' bytes are read
    int byte_val = next_byte(); // Read the next byte
    if (byte_val == -1) {
      return -1; // Error or end of stream
    }
    *current_pos = (char)byte_val; // Store the byte
    current_pos++;
    bytes_read++;
  }
  return (int)(current_pos - param_2); // Return total bytes read (which will be param_3)
}

// Main function for testing the above functions
int main() {
    printf("--- Testing next_byte ---\n");
    printf("Reading 20 bytes or until end of stream:\n");
    for (int i = 0; i < 20; ++i) {
        uint byte = next_byte();
        if (byte == 0xFFFFFFFF) {
            printf("  next_byte: End of stream or error.\n");
            break;
        }
        printf("  next_byte: '%c' (0x%02x)\n", (char)byte, byte);
    }
    printf("-------------------------\n");

    printf("\n--- Testing read_until ---\n");
    char buffer_until[50]; // Buffer to hold up to 49 chars + null terminator
    rxidx = 0; // Reset global state for new test
    rxcnt = 0;
    printf("Attempting to read until '\\n' or buffer full (size 50):\n");
    int bytes_read_until = read_until(0, buffer_until, sizeof(buffer_until), '\n');
    if (bytes_read_until == -1) {
        printf("  read_until: Error occurred.\n");
    } else {
        printf("  read_until: Read %d bytes. Buffer content: \"%s\"\n", bytes_read_until, buffer_until);
    }

    rxidx = 0; // Reset global state for new test
    rxcnt = 0;
    printf("Attempting to read until 'X' or buffer full (size 50):\n");
    bytes_read_until = read_until(0, buffer_until, sizeof(buffer_until), 'X');
    if (bytes_read_until == -1) {
        printf("  read_until: Error occurred.\n");
    } else {
        printf("  read_until: Read %d bytes. Buffer content: \"%s\"\n", bytes_read_until, buffer_until);
    }
    printf("-------------------------\n");

    printf("\n--- Testing read_n ---\n");
    char buffer_n[20]; // Buffer to hold 20 characters
    rxidx = 0; // Reset global state for new test
    rxcnt = 0;
    printf("Attempting to read 10 bytes:\n");
    int bytes_read_n = read_n(0, buffer_n, 10);
    if (bytes_read_n == -1) {
        printf("  read_n: Error occurred.\n");
    } else {
        buffer_n[bytes_read_n] = '\0'; // Manually null-terminate for printing
        printf("  read_n: Read %d bytes. Buffer content: \"%s\"\n", bytes_read_n, buffer_n);
    }

    rxidx = 0; // Reset global state for new test
    rxcnt = 0;
    printf("Attempting to read 15 bytes:\n");
    bytes_read_n = read_n(0, buffer_n, 15);
    if (bytes_read_n == -1) {
        printf("  read_n: Error occurred.\n");
    } else {
        buffer_n[bytes_read_n] = '\0'; // Manually null-terminate for printing
        printf("  read_n: Read %d bytes. Buffer content: \"%s\"\n", bytes_read_n, buffer_n);
    }
    printf("-------------------------\n");

    return 0;
}