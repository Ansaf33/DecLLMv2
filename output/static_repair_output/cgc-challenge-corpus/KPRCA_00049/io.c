#include <stddef.h> // For size_t
#include <string.h> // For strlen
#include <stdint.h> // For standard integer types, if needed (e.g., uint32_t for undefined4)

// Mock functions for compilation. In a real application, these would be
// system calls (e.g., send/recv for sockets, write/read for file descriptors).
// We assume 0 for success and non-zero for error, and that 'len' bytes are
// processed in a single call, updating 'processed_len'.

// transmit: Sends data from a buffer.
// fd: file descriptor or socket.
// buf: pointer to the data to send.
// len: number of bytes to attempt to send.
// sent_len: pointer to a size_t to store the actual number of bytes sent.
// Returns 0 on success, -1 on error.
int transmit(int fd, const char *buf, size_t len, size_t *sent_len) {
    if (buf == NULL || sent_len == NULL) {
        return -1; // Invalid arguments
    }
    // Simulate successful transmission of all bytes requested
    *sent_len = len;
    // In a real scenario, this would involve actual I/O and could send fewer bytes than 'len'.
    // For this mock, we assume full success.
    return 0;
}

// receive: Receives data into a buffer.
// fd: file descriptor or socket.
// buf: pointer to the buffer to store received data.
// len: maximum number of bytes to attempt to receive.
// recv_len: pointer to a size_t to store the actual number of bytes received.
// Returns 0 on success, -1 on error.
int receive(int fd, char *buf, size_t len, size_t *recv_len) {
    if (buf == NULL || recv_len == NULL) {
        return -1; // Invalid arguments
    }
    // Simulate successful reception of all bytes requested
    *recv_len = len;
    // In a real scenario, this would involve actual I/O and could receive fewer bytes than 'len'.
    // For this mock, we assume full success.
    return 0;
}

// Function: send_n_bytes
int send_n_bytes(int fd, const char *buffer, size_t total_bytes_to_send) {
  size_t bytes_sent_this_call;
  size_t bytes_remaining = total_bytes_to_send;
  const char *current_buffer_ptr = buffer; // Use const char* as we are reading from it

  if (total_bytes_to_send == 0 || buffer == NULL) {
    return -1; // Error: nothing to send or invalid buffer
  }

  while (bytes_remaining > 0) {
    if (transmit(fd, current_buffer_ptr, bytes_remaining, &bytes_sent_this_call) != 0) {
      return -1; // Transmit error
    }
    if (bytes_sent_this_call == 0) {
      break; // No bytes sent, connection closed or error
    }
    bytes_remaining -= bytes_sent_this_call;
    current_buffer_ptr += bytes_sent_this_call;
  }
  return (int)(total_bytes_to_send - bytes_remaining); // Return actual bytes sent
}

// Function: read_n_bytes
int read_n_bytes(int fd, char *buffer, size_t total_bytes_to_read) {
  size_t bytes_received_this_call;
  size_t bytes_remaining = total_bytes_to_read;
  char *current_buffer_ptr = buffer;

  if (total_bytes_to_read == 0 || buffer == NULL) {
    return -1; // Error: nothing to read or invalid buffer
  }

  while (bytes_remaining > 0) {
    if (receive(fd, current_buffer_ptr, bytes_remaining, &bytes_received_this_call) != 0) {
      return -1; // Receive error
    }
    if (bytes_received_this_call == 0) {
      break; // No bytes received, connection closed or error
    }
    bytes_remaining -= bytes_received_this_call;
    current_buffer_ptr += bytes_received_this_call;
  }
  return (int)(total_bytes_to_read - bytes_remaining); // Return actual bytes received
}

// Function: transmit_string
int transmit_string(int fd, const char *str) {
  size_t len;
  int sent_bytes;
  
  if (str == NULL) {
      return -1; // Invalid string pointer
  }

  len = strlen(str);
  if (len == 0) {
    return 0; // Successfully "sent" an empty string
  }
  
  // send_n_bytes expects a buffer to read from. The const char* is compatible.
  sent_bytes = send_n_bytes(fd, str, len); 
  
  if (sent_bytes == (int)len) {
    return 0; // Success
  } else {
    return -1; // Failure
  }
}

// Function: read_until
int read_until(int fd, size_t max_buffer_size, char delimiter, char *buffer) {
  size_t total_bytes_read = 0;
  size_t bytes_received_this_call;
  int ret_val;

  if (buffer == NULL || max_buffer_size == 0) {
    return -1; // Invalid buffer or size
  }

  while (total_bytes_read < max_buffer_size) {
    // Attempt to receive one byte into the current position in the buffer
    ret_val = receive(fd, buffer + total_bytes_read, 1, &bytes_received_this_call);
    
    if (ret_val != 0 || bytes_received_this_call == 0) {
      // Error during receive, or connection closed/EOF before delimiter or max_buffer_size
      return -1;
    }

    // Check if the received byte is the delimiter
    if (buffer[total_bytes_read] == delimiter) {
      buffer[total_bytes_read] = '\0'; // Null-terminate at the delimiter's position
      return (int)(total_bytes_read + 1); // Return total bytes read, including the (now null-ed) delimiter
    }
    
    total_bytes_read++; // Move to the next position
  }

  // If the loop completes, max_buffer_size bytes have been read without finding the delimiter.
  // The original code returned -1 in this scenario.
  return -1; 
}