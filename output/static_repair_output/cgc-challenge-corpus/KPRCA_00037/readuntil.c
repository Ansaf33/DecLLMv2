#include <unistd.h> // For read, ssize_t, STDIN_FILENO
#include <stddef.h> // For size_t
#include <stdio.h>  // For printf, fprintf, NULL, perror
#include <string.h> // For strlen
#include <stdlib.h> // For EXIT_SUCCESS, EXIT_FAILURE

// Function: read_until
// Reads from a file descriptor `fd` into `buf` until `delimiter` is found, `max_len` bytes are read,
// or an error/EOF occurs.
// The delimiter character is read into the buffer, then overwritten with a null terminator.
// Returns the total number of bytes read (including the delimiter if found), or -1 on error/EOF.
int read_until(int fd, char *buf, size_t max_len, char delimiter) {
  size_t i;
  ssize_t bytes_read_count;
  char *current_pos = buf; 
  
  if (max_len == 0 || buf == NULL) {
      if (buf != NULL && max_len > 0) buf[0] = '\0'; // Ensure null termination if possible
      return 0; // No space to read or invalid buffer
  }

  for (i = 0; i < max_len; ++i) {
    bytes_read_count = read(fd, current_pos, 1);
    
    if (bytes_read_count <= 0) { // Error or EOF
      // Original code returns -1 immediately on read error or EOF.
      return -1; 
    }
    
    // Character just read is now at *current_pos.
    // Store it before `current_pos` is advanced, to match original logic.
    char char_read = *current_pos; 
    current_pos++; // Advance `current_pos` for the next iteration or final calculation.
    
    if (delimiter == char_read) { 
      break; // Delimiter found, exit loop.
    }
  }

  // After loop: `current_pos` points to `buf + total_bytes_read`.
  // If any bytes were read (i.e., `current_pos` moved past the original `buf` start).
  if (buf < current_pos) {
    current_pos[-1] = '\0'; // Null-terminate *before* the current position (overwrites delimiter or last char).
  }
  
  return (int)(current_pos - buf); // Return total bytes read (including delimiter if found).
}

// Function: read_n
// Reads exactly `len` bytes from a file descriptor `fd` into `buf`.
// Stops early if an error or EOF occurs.
// Does NOT null-terminate the buffer.
// Returns the actual number of bytes read.
int read_n(int fd, char *buf, size_t len) {
  size_t i;
  ssize_t bytes_read_count;
  char *current_pos = buf; 

  if (len == 0 || buf == NULL) return 0;

  for (i = 0; i < len; ++i) {
    bytes_read_count = read(fd, current_pos, 1);
    
    if (bytes_read_count <= 0) { // Error or EOF
      break; // Stop reading and return bytes read so far.
    }
    current_pos++; // Advance for the next iteration.
  }
  
  return (int)(current_pos - buf); // Return total bytes successfully read.
}

// Main function for demonstration and compilation
int main() {
    char buffer[256];
    int bytes_read;

    printf("--- Testing read_until ---\n");
    printf("Enter a line (max %zu chars, will read until newline): ", sizeof(buffer) - 1);
    fflush(stdout); // Ensure prompt is displayed

    // Read from standard input (file descriptor 0) until newline or buffer full
    bytes_read = read_until(STDIN_FILENO, buffer, sizeof(buffer) - 1, '\n');
    if (bytes_read == -1) {
        perror("read_until error");
        // Clear stdin buffer on error to prevent issues with next read
        while (read(STDIN_FILENO, buffer, 1) > 0 && buffer[0] != '\n');
        return EXIT_FAILURE;
    } else {
        printf("read_until returned %d bytes. String: \"%s\" (length %zu)\n", bytes_read, buffer, strlen(buffer));
        // Note: read_until returns the count of bytes processed *including* the delimiter,
        // but the string itself is null-terminated *before* the delimiter.
        // So, `bytes_read` might be greater than `strlen(buffer)`.
    }

    printf("\n--- Testing read_n ---\n");
    printf("Enter some characters (will read exactly 5): ");
    fflush(stdout);

    // Read exactly 5 bytes from standard input
    bytes_read = read_n(STDIN_FILENO, buffer, 5);
    if (bytes_read < 0) { // read_n doesn't return -1, but checking defensively
        perror("read_n error"); 
        return EXIT_FAILURE;
    } else {
        buffer[bytes_read] = '\0'; // Manually null-terminate for printing
        printf("read_n returned %d bytes. String: \"%s\" (length %zu)\n", bytes_read, buffer, strlen(buffer));
    }

    return EXIT_SUCCESS;
}