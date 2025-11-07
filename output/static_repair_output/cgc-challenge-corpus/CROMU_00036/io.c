#include <stdio.h>   // For printf
#include <string.h>  // For string manipulation in main (e.g., null termination)

// --- Mock transmit and receive functions ---
// These are placeholder functions to allow the provided code to compile and run.
// In a real application, these would be provided by a system library (e.g., socket APIs, device drivers).

// Simulates sending data. Returns 0 on success, non-zero on error.
// The actual number of bytes sent is written to 'bytes_transferred'.
int transmit(int fd, const void *buf, unsigned int count, int *bytes_transferred) {
    if (fd < 0) { // Example: invalid file descriptor
        *bytes_transferred = 0;
        return -1; // Error
    }
    // For this mock, assume all bytes are sent successfully.
    *bytes_transferred = (int)count;
    return 0; // Success
}

// Simulates receiving data. Returns 0 on success, non-zero on error.
// The actual number of bytes received is written to 'bytes_transferred'.
int receive(int fd, void *buf, unsigned int count, int *bytes_transferred) {
    if (fd < 0) { // Example: invalid file descriptor
        *bytes_transferred = 0;
        return -1; // Error
    }
    // For this mock, assume all bytes are received successfully.
    // In a real scenario, data would be copied into 'buf'.
    *bytes_transferred = (int)count;
    return 0; // Success
}

// --- Original functions, refactored ---

// Function: SendAll
// Sends 'size' bytes from 'buffer' using 'transmit'.
// Returns 0 on complete success, -1 on any error or partial send/error from transmit.
int SendAll(void *buffer, unsigned int size) {
  unsigned int total_sent = 0;
  int bytes_sent; // Stores the number of bytes actually sent by 'transmit'

  while(total_sent < size) {
    // Attempt to transmit the remaining bytes
    int ret = transmit(1, (char *)buffer + total_sent, size - total_sent, &bytes_sent);

    // Check for transmit errors or if no bytes were sent (which typically indicates an issue)
    if (ret != 0 || bytes_sent == 0) {
      return -1; // Return error
    }
    total_sent += bytes_sent; // Accumulate the total number of bytes successfully sent
  }
  return 0; // All bytes sent successfully
}

// Function: SendNull
// Sends 'count' null bytes using 'transmit'.
// Returns 0 on complete success, -1 on any error or if a null byte couldn't be sent.
int SendNull(unsigned int count) {
  char null_byte = 0; // The single null byte to transmit repeatedly
  unsigned int total_sent = 0;
  int bytes_sent; // Stores the number of bytes actually sent by 'transmit'

  while(total_sent < count) {
    // Attempt to transmit one null byte
    int ret = transmit(1, &null_byte, 1, &bytes_sent);

    // Check for transmit errors or if exactly one byte wasn't sent
    if (ret != 0 || bytes_sent != 1) {
      return -1; // Return error
    }
    total_sent++; // Increment count of null bytes sent
  }
  return 0; // All null bytes sent successfully
}

// Function: ReceiveAll
// Receives 'size' bytes into 'buffer' using 'receive'.
// Returns 0 on complete success, -1 on any error or partial receive/error from receive.
int ReceiveAll(void *buffer, unsigned int size) {
  unsigned int total_received = 0;
  int bytes_received; // Stores the number of bytes actually received by 'receive'

  while(total_received < size) {
    // Attempt to receive the remaining bytes
    int ret = receive(0, (char *)buffer + total_received, size - total_received, &bytes_received);

    // Check for receive errors or if no bytes were received (which typically indicates an issue/EOF)
    if (ret != 0 || bytes_received == 0) {
      return -1; // Return error
    }
    total_received += bytes_received; // Accumulate the total number of bytes successfully received
  }
  return 0; // All bytes received successfully
}

// --- Main function for demonstration and compilation ---
int main() {
    char send_buffer[] = "Hello, World!";
    // Ensure recv_buffer is large enough and null-terminated for printf
    char recv_buffer[sizeof(send_buffer) + 5] = {0}; // +5 for safety, and ensure null termination
    unsigned int send_len = sizeof(send_buffer) - 1; // Exclude null terminator for data length

    printf("--- Demonstrating SendAll ---\n");
    printf("Attempting to SendAll \"%s\" (%u bytes)...\n", send_buffer, send_len);
    if (SendAll(send_buffer, send_len) == 0) {
        printf("SendAll successful.\n");
    } else {
        printf("SendAll failed.\n");
    }

    printf("\n--- Demonstrating ReceiveAll ---\n");
    printf("Attempting to ReceiveAll %u bytes into buffer...\n", send_len);
    // In a real scenario, data received would match what was sent.
    // Here, mock receive just indicates success, actual content might be garbage.
    if (ReceiveAll(recv_buffer, send_len) == 0) {
        recv_buffer[send_len] = '\0'; // Ensure null termination for printing
        printf("ReceiveAll successful. Mock received data (might not be actual content): \"%s\"\n", recv_buffer);
    } else {
        printf("ReceiveAll failed.\n");
    }

    printf("\n--- Demonstrating SendNull ---\n");
    unsigned int null_count = 5;
    printf("Attempting to SendNull %u times...\n", null_count);
    if (SendNull(null_count) == 0) {
        printf("SendNull successful.\n");
    } else {
        printf("SendNull failed.\n");
    }

    return 0;
}