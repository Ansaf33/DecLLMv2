#include <string.h>   // For strchr, memcpy, memset
#include <unistd.h>   // For ssize_t, size_t
#include <sys/socket.h> // For recv

// Function: readline
// Reads a line from a socket file descriptor `fd` into `buf`, up to `max_len` bytes.
// Updates `total_bytes_read` with the cumulative number of bytes copied into `buf`.
// A line is terminated by a newline character ('\n'). The newline itself is not copied into `buf`.
//
// Parameters:
//   fd: The socket file descriptor to read from.
//   buf: A pointer to the destination buffer where the line will be stored.
//   max_len: The maximum number of bytes to read into `buf` (excluding null terminator, if any).
//   total_bytes_read: A pointer to a size_t variable that tracks the total bytes
//                     copied to `buf` across multiple calls or chunks.
//
// Returns:
//   0 on success (a newline was found, or EOF was reached and all available data was read).
//   -1 on error (e.g., socket error, or line starts with a newline character).
ssize_t readline(int fd, void *buf, size_t max_len, size_t *total_bytes_read) {
  char recv_buf[1025]; // Buffer for data received from socket (1024 bytes + 1 for null termination)
  char *newline_ptr = NULL; // Pointer to the newline character in recv_buf
  ssize_t bytes_received_from_socket; // Return value from recv (actual bytes or error)
  size_t bytes_to_copy_this_chunk; // Number of bytes to copy from recv_buf to destination buffer

  *total_bytes_read = 0; // Initialize total bytes read for this readline operation

  // Loop until a newline character is found or an error/EOF occurs.
  do {
    // Check if the maximum allowed length has been reached.
    if (max_len <= *total_bytes_read) {
      return 0; // Successfully read up to max_len
    }

    // Receive data from the socket. The original code used 0x400 (1024) bytes.
    bytes_received_from_socket = recv(fd, recv_buf, 1024, 0);

    if (bytes_received_from_socket < 0) {
      return -1; // Error during receive operation
    }
    if (bytes_received_from_socket == 0) {
      // EOF (End of File/Stream). Return 0 indicating success, but no more data available.
      return 0;
    }

    // Null-terminate the received data for safe use with strchr.
    // This ensures strchr does not read beyond the actually received bytes.
    recv_buf[bytes_received_from_socket] = '\0';
    
    // Find the newline character within the received chunk.
    newline_ptr = strchr(recv_buf, '\n');

    // If this is the very first chunk and the first character is a newline,
    // the original code considers this an error.
    if ((*total_bytes_read == 0) && (newline_ptr == recv_buf)) {
      return -1; // Error: line starts with a newline (empty line received at start)
    }

    // Determine how many bytes from the current chunk should be copied to the destination buffer.
    if (newline_ptr != NULL) {
      // Newline found: copy bytes from the start of recv_buf up to (but not including) the newline.
      bytes_to_copy_this_chunk = (size_t)(newline_ptr - recv_buf);
    } else {
      // No newline found in this chunk: copy all bytes received in this chunk.
      bytes_to_copy_this_chunk = (size_t)bytes_received_from_socket;
    }

    // Ensure that copying this chunk does not exceed the total allowed length (`max_len`).
    if (max_len < bytes_to_copy_this_chunk + *total_bytes_read) {
      bytes_to_copy_this_chunk = max_len - *total_bytes_read;
    }

    // Copy data to the destination buffer if there are bytes to copy.
    if (bytes_to_copy_this_chunk > 0) {
      memcpy(buf, recv_buf, bytes_to_copy_this_chunk);
      buf = (char *)buf + bytes_to_copy_this_chunk; // Advance destination pointer
      *total_bytes_read += bytes_to_copy_this_chunk; // Update total bytes read
    }

    // Clear the receive buffer. While not strictly necessary if `recv` always overwrites,
    // it's good practice for security and to prevent stale data issues.
    memset(recv_buf, 0, sizeof(recv_buf));

  } while (newline_ptr == NULL); // Continue looping as long as no newline has been found

  return 0; // Successfully read a line (newline character was found)
}