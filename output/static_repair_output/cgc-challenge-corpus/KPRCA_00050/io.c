#include <stddef.h>  // For size_t
#include <string.h>  // For strlen

// External functions and global variables assumed for compilation
// These functions are placeholders and would need actual implementations
// or definitions from a specific library (e.g., networking, custom I/O).
// The signature is inferred from the usage in the original snippet.
extern int receive(int fd, void *buf, size_t len, size_t *bytes_read);
extern int transmit(int fd, const void *buf, size_t len, size_t *bytes_written);

// Global variables for XOR and accumulation, initialized to 0.
// Using unsigned char to explicitly handle byte-level operations.
static unsigned char ikey = 0;
static unsigned char okey = 0;

// Function: read_bytes
int read_bytes(char *buf, int len) {
  size_t bytes_received;

  while (len > 0) {
    // Attempt to receive bytes into the buffer
    if (receive(0, buf, len, &bytes_received) != 0 || bytes_received == 0) {
      return 0; // Error or EOF
    }

    // Process received bytes (XOR and update ikey)
    for (size_t i = 0; i < bytes_received; ++i) {
      unsigned char *current_byte_ptr = (unsigned char *)(buf + i);
      *current_byte_ptr = ikey ^ *current_byte_ptr;
      ikey += *current_byte_ptr; // ikey is updated with the decrypted byte
    }

    // Update remaining length and buffer pointer
    len -= bytes_received;
    buf += bytes_received;
  }
  return 1; // Successfully read all requested bytes
}

// Function: read_until
int read_until(char *buf, size_t max_len, char delimiter) {
  size_t i;

  for (i = 0; i < max_len; ++i) {
    // Read one byte into buf[i]
    if (read_bytes(buf + i, 1) == 0) {
      return 0; // Error or EOF
    }
    // Check if the current byte is the delimiter
    if (delimiter == buf[i]) {
      break;
    }
  }

  // Null-terminate the string
  if (i < max_len) {
    buf[i] = '\0'; // Delimiter found, null-terminate at its position
  } else {
    buf[max_len - 1] = '\0'; // Delimiter not found, buffer full, null-terminate at the end
  }
  return 1; // Successfully read until delimiter or max_len
}

// Function: write_bytes
void write_bytes(char *buf, size_t len) {
  char *current_buf_ptr = buf;
  size_t bytes_to_send = len;

  // Process bytes for transmission (XOR and update okey)
  for (size_t i = 0; i < len; ++i) {
    char original_byte = buf[i];          // Store original byte value
    buf[i] = okey ^ buf[i];               // XOR in place
    okey += original_byte;                // Update okey with the original byte
  }

  // Transmit bytes
  size_t bytes_sent;
  while (bytes_to_send > 0) {
    // Attempt to transmit bytes from the buffer
    if (transmit(1, current_buf_ptr, bytes_to_send, &bytes_sent) != 0 || bytes_sent == 0) {
      break; // Error or nothing sent, stop transmitting
    }
    // Update remaining length and buffer pointer
    bytes_to_send -= bytes_sent;
    current_buf_ptr += bytes_sent;
  }
  // No return value as it's a void function
}

// Function: write_string
void write_string(char *s) {
  // Calculate string length and call write_bytes
  write_bytes(s, strlen(s));
}