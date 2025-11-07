#include <stdio.h>   // For printf
#include <stdlib.h>  // For malloc, free
#include <string.h>  // For memset
#include <stddef.h>  // For size_t, NULL

// --- Mock/Forward Declarations for external functions ---
// These are placeholders to make the code compilable.
// In a real application, these would be provided by other modules or libraries.

// Assuming receive(fd, buffer, length, bytes_received_ptr)
// Returns 0 on success, non-zero on error.
// Fills bytes_received_ptr with the number of bytes actually received.
int receive(int fd, void* buffer, int length, int* bytes_received) {
    if (length <= 0) {
        *bytes_received = 0;
        return 0;
    }
    // Simulate receiving all requested bytes for simplicity.
    // In a real scenario, this would interact with a socket or file descriptor
    // and might receive fewer bytes than requested.
    *bytes_received = length;
    return 0; // Success
}

// Assuming initStream takes a buffer and its size, returns a stream ID (int) on success, 0 on failure.
int initStream(void* buffer, size_t size) {
    if (buffer != NULL && size > 0) {
        // Dummy stream ID for successful initialization
        return 1;
    }
    return 0; // Failure
}

// Assuming renderCVF takes an int stream ID.
void renderCVF(int stream_id) {
    printf("Rendering stream with ID: %d\n", stream_id);
}

// Assuming freeStream takes a pointer to an int stream ID.
void freeStream(int* stream_id_ptr) {
    printf("Freeing stream with ID: %d\n", *stream_id_ptr);
    *stream_id_ptr = 0; // Invalidate the ID after freeing
}

// --- Original functions, refactored ---

// Function: receiveIt
int receiveIt(void* buffer, int length) {
  int total_bytes_received = 0;
  int bytes_received_this_call;

  // If buffer is NULL or length is non-positive, nothing to receive.
  if (buffer == NULL || length <= 0) {
    return 0;
  }

  // Loop until all 'length' bytes are received or an error occurs.
  int remaining_length = length;
  while (remaining_length > 0) {
    // Call the external receive function.
    // The buffer for the current receive call is the base buffer plus the total bytes already received.
    int status = receive(0, (char*)buffer + total_bytes_received, remaining_length, &bytes_received_this_call);
    
    if (status != 0) {
      printf("[ERROR] Failed to receive\n");
      return 0; // Return 0 on external receive error
    }
    
    // If receive returns 0 bytes but there's still data expected, it means progress stalled.
    // This could indicate a broken pipe or non-blocking socket behavior without an error code.
    if (bytes_received_this_call == 0) {
        printf("[ERROR] Receive made no progress, expected %d more bytes\n", remaining_length);
        return 0; // Indicate failure
    }

    total_bytes_received += bytes_received_this_call;
    remaining_length -= bytes_received_this_call; // Reduce remaining length
  }
  return total_bytes_received;
}

// Function: readImageData
int readImageData(void) {
  int image_size_val; // Use int to store the 4-byte image size
  void *image_buffer = NULL; // Initialize pointer to NULL

  printf("----------------Stream Me Your Video----------------\n");

  // Attempt to receive the 4-byte image size value.
  int bytes_read_for_size = receiveIt(&image_size_val, sizeof(int));
  if (bytes_read_for_size != sizeof(int)) {
    printf("[ERROR] Failed to read image size (expected %zu bytes, got %d)\n", sizeof(int), bytes_read_for_size);
    return 0; // Indicate failure
  }

  // Validate image size constraints.
  // "Image must be greater than 8 bytes"
  if (image_size_val < 8) {
    printf("[ERROR] Image must be greater than 8 bytes (got %d)\n", image_size_val);
    return 0; // Indicate failure
  }
  // "Image must be smaller than 4096 bytes" (0x1000)
  // This means max allowed size is 4095.
  if (image_size_val >= 4096) {
    printf("[ERROR] Image must be smaller than 4096 bytes (got %d)\n", image_size_val);
    return 0; // Indicate failure
  }

  // Allocate memory for the image data.
  // Cast image_size_val to size_t for malloc, as malloc expects size_t.
  image_buffer = malloc((size_t)image_size_val);
  if (image_buffer == NULL) {
    printf("[ERROR] Failed to allocate memory for image data\n");
    return 0; // Indicate failure
  }

  // Initialize allocated memory to zero.
  memset(image_buffer, 0, (size_t)image_size_val);

  // Attempt to receive the actual image data.
  int bytes_read_for_data = receiveIt(image_buffer, image_size_val);
  if (bytes_read_for_data != image_size_val) {
    printf("[ERROR] Failed to read all image data (expected %d bytes, got %d)\n", image_size_val, bytes_read_for_data);
    free(image_buffer); // Clean up allocated memory
    return 0; // Indicate failure
  }

  // Initialize the stream with the received image data.
  // The buffer is freed afterwards, assuming initStream makes its own copy or processes it.
  int stream_id = initStream(image_buffer, (size_t)image_size_val);
  free(image_buffer); // Clean up allocated memory

  return stream_id; // Return the stream ID (0 on initStream failure)
}

// Function: main
int main(void) {
  int stream_id = readImageData();
  if (stream_id != 0) { // If readImageData returned a valid stream ID
    renderCVF(stream_id);
    freeStream(&stream_id); // Pass address of stream_id to free it
  }
  return 0; // Standard successful exit code
}