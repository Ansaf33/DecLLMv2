#include <stddef.h> // For size_t
#include <string.h> // For memcpy, strlen (used in mock receive)
#include <stdio.h>  // For a minimal main (optional, could be removed if main is truly empty)

// Mock receive function declaration.
// In a real application, this would typically be a system call like `read` or `recv`.
// The original snippet implies a custom `receive` function with the following behavior:
// - `fd`: An integer file/socket descriptor.
// - `buf`: A pointer to the buffer to read into.
// - `count`: The number of bytes to attempt to read.
// - `bytes_read_output`: A pointer to an integer where the actual number of bytes read is stored.
// - Return value: 0 on success, non-zero on error.
int receive(int fd, char *buf, size_t count, int *bytes_read_output);

// Function: read_until
// Reads bytes from `fd` into `buffer` until `delimiter` is found, a null character is found,
// `max_len` bytes have been read, or an error/EOF occurs.
// The delimiter character itself is read into the buffer and then overwritten with a null terminator.
//
// Parameters:
//   fd: The file or socket descriptor to read from.
//   buffer: A pointer to the character array where data will be stored.
//   max_len: The maximum number of bytes to read into the buffer (including the delimiter).
//   delimiter: The character to stop reading at.
//
// Returns:
//   The number of bytes read (including the overwritten delimiter/null character) on success.
//   -1 on error or EOF.
//   0 if max_len is 0.
int read_until(int fd, char *buffer, size_t max_len, char delimiter) {
  if (max_len == 0) {
    return 0;
  }

  size_t i = 0; // Tracks bytes read and current buffer index
  int bytes_received_this_call;

  while (i < max_len) {
    int ret = receive(fd, buffer + i, 1, &bytes_received_this_call);

    if (ret != 0 || bytes_received_this_call == 0) {
      // Error or EOF. As per original logic, return -1 immediately.
      return -1;
    }

    // Check if the character just read is the delimiter or a null character
    if (buffer[i] == delimiter || buffer[i] == '\0') {
      i++; // Increment `i` to include the delimiter/null in the returned count
      buffer[i - 1] = '\0'; // Overwrite the delimiter/null with a null terminator
      return (int)i;
    }
    i++; // Move to the next position in the buffer
  }

  // If the loop finishes, it means `max_len` bytes have been read without finding the delimiter.
  // Null terminate the last character read to ensure buffer is a valid string.
  buffer[max_len - 1] = '\0';
  return (int)max_len;
}

// Function: read_n
// Reads exactly `count` bytes from `fd` into `buffer`.
// Stops early if an error or EOF occurs.
//
// Parameters:
//   fd: The file or socket descriptor to read from.
//   buffer: A pointer to the character array where data will be stored.
//   count: The number of bytes to attempt to read.
//
// Returns:
//   The total number of bytes successfully read.
//   This value may be less than `count` if an error or EOF occurred.
int read_n(int fd, char *buffer, size_t count) {
  size_t i;
  int bytes_received_this_call;
  size_t total_bytes_read = 0;

  for (i = 0; i < count; i++) {
    int ret = receive(fd, buffer + i, 1, &bytes_received_this_call);

    if (ret != 0 || bytes_received_this_call == 0) {
      // Error or EOF. Stop reading.
      break;
    }
    total_bytes_read++; // One byte successfully read
  }
  return (int)total_bytes_read;
}

// Minimal main function for compilation.
// The actual implementation of 'receive' would link against a library or be provided elsewhere.
int main() {
    // This main function is purely for compilation and does not provide
    // a meaningful execution without a proper 'receive' implementation
    // or further calls to read_until/read_n with a configured 'receive' mock.
    // Example:
    // char buf[128];
    // int fd = 0; // Placeholder for a file descriptor
    // int bytes_read = read_n(fd, buf, sizeof(buf));
    // if (bytes_read > 0) {
    //     // Process data
    // }
    return 0;
}

// Mock receive function definition for compilation purposes.
// This provides a basic simulation of reading data.
// In a real application, this would be an actual system call (e.g., `read` from <unistd.h>,
// or `recv` from <sys/socket.h>).
static const char *mock_data_source = "Hello, World!\nThis is a test string for compilation.";
static size_t mock_data_offset = 0; // Tracks position in mock_data_source

int receive(int fd, char *buf, size_t count, int *bytes_read_output) {
    (void)fd; // Suppress unused parameter warning for 'fd'

    if (buf == NULL || bytes_read_output == NULL || count == 0) {
        *bytes_read_output = 0;
        return -1; // Indicate invalid arguments
    }

    // Check if end of mock data source has been reached
    if (mock_data_offset >= strlen(mock_data_source)) {
        *bytes_read_output = 0;
        return 0; // Indicate success but 0 bytes read (EOF)
    }

    // Calculate how many bytes can be copied
    size_t remaining_mock_data = strlen(mock_data_source) - mock_data_offset;
    size_t bytes_to_copy = (count < remaining_mock_data) ? count : remaining_mock_data;

    // Copy data from mock source to the buffer
    memcpy(buf, mock_data_source + mock_data_offset, bytes_to_copy);
    mock_data_offset += bytes_to_copy; // Advance the offset
    *bytes_read_output = (int)bytes_to_copy; // Store actual bytes copied

    return 0; // Indicate success
}