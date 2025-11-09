#include <stdlib.h> // For malloc, free
#include <string.h> // For memcpy, memcmp
#include <unistd.h> // For ssize_t, read, STDIN_FILENO

// Placeholder for DAT_00015374. In a real scenario, this would be an external constant
// or defined based on the specific application's data segment.
// It's used in memcmp with a size of 3 bytes.
static const char DAT_00015374[3] = {'M', 'A', 'G'}; // Example magic bytes

// A simple implementation of recvall that attempts to read all requested bytes.
// In a real application, error handling (e.g., EINTR) might be more robust.
ssize_t recvall(int fd, void *buf, size_t count) {
    size_t total_received = 0;
    char *ptr = (char *)buf;
    while (total_received < count) {
        ssize_t bytes_read = read(fd, ptr + total_received, count - total_received);
        if (bytes_read == -1) {
            return -1; // Error
        }
        if (bytes_read == 0) { // EOF before all bytes received
            return total_received; // Return number of bytes actually received
        }
        total_received += bytes_read;
    }
    return total_received;
}

// Function: open_sfile
void * open_sfile(void) {
  char initial_header[24]; // Buffer for initial receive
  void *file_data_buffer = NULL; // Pointer to allocated memory

  // 1. Receive the initial 24-byte header
  ssize_t bytes_received = recvall(STDIN_FILENO, initial_header, sizeof(initial_header));
  if (bytes_received != sizeof(initial_header)) { // Must receive exactly 24 bytes
      return NULL;
  }

  // 2. Validate the header
  // Check 1: Magic bytes comparison
  if (memcmp(initial_header, DAT_00015374, 3) != 0) {
      return NULL;
  }

  // Check 2: Fourth byte (index 3) must be 0 or 1
  if (!(initial_header[3] == '\0' || initial_header[3] == '\x01')) {
      return NULL;
  }

  // Check 3: Byte 19 must be 0 AND the integer at offset 20 must not be 0
  // The integer at offset 20 (initial_header + 20) determines the size of the following data.
  // Storing this value once reduces repeated memory access.
  int data_content_size = *(int *)(initial_header + 20);
  if (initial_header[19] != '\0' || data_content_size <= 0) { // data_content_size cannot be 0 or negative
      return NULL;
  }

  // 3. Allocate memory for the full file content + header + null terminator
  // Total size = initial_header (24 bytes) + data_content_size + null_terminator (1 byte)
  file_data_buffer = malloc(sizeof(initial_header) + data_content_size + 1);
  if (file_data_buffer == NULL) {
      return NULL; // Allocation failed
  }

  // 4. Copy the initial 24-byte header into the newly allocated buffer
  memcpy(file_data_buffer, initial_header, sizeof(initial_header));

  // 5. Receive the remaining data content into the allocated buffer, right after the header
  bytes_received = recvall(STDIN_FILENO, (char *)file_data_buffer + sizeof(initial_header), data_content_size);
  if (bytes_received != data_content_size) { // Must receive exactly data_content_size bytes
      free(file_data_buffer); // Free on partial read or error
      return NULL;
  }

  // 6. Null-terminate the entire content
  // The null byte goes at the end of the data content.
  // Offset = start + sizeof(initial_header) + data_content_size
  *(char *)((char *)file_data_buffer + sizeof(initial_header) + data_content_size) = '\0';

  return file_data_buffer;
}

// Function: close_sfile
void close_sfile(void **param_1) {
  if (*param_1 != NULL) { // Replaced (void *)0x0 with NULL
    free(*param_1);
  }
  *param_1 = NULL; // Replaced (void *)0x0 with NULL
  return;
}