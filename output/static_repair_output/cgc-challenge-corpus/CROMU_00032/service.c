#include <stdio.h>   // For printf
#include <stdlib.h>  // For malloc, free
#include <string.h>  // For memset
#include <stddef.h>  // For size_t
#include <stdint.h>  // For uint32_t

// --- Mock/External function declarations ---
// Assume 'receive' is a function that reads data from a source (e.g., socket, file descriptor).
// Signature: int receive(int fd, void *buf, size_t len, int *bytes_received_ptr);
// Returns 0 on success, non-zero on error.
// Fills bytes_received_ptr with the actual number of bytes read in that call.
int receive(int fd, void *buf, size_t len, int *bytes_received_ptr) {
    // This is a placeholder/mock implementation for compilation.
    // In a real system, this would typically be a `recv` call or similar.
    if (buf == NULL || bytes_received_ptr == NULL || len == 0) {
        *bytes_received_ptr = 0;
        return -1; // Error
    }
    // Simulate receiving all requested bytes for successful path.
    *bytes_received_ptr = (int)len;
    return 0; // Success
}

// Placeholder for initStream.
// Assumed signature: int initStream(void *data, size_t len);
// Returns a stream handle (int) or 0 on failure.
int initStream(void *data, size_t len) {
    if (data == NULL || len == 0) {
        return 0; // Failure
    }
    return 123; // Example handle
}

// Placeholder for renderCVF.
// Assumed signature: void renderCVF(int handle);
void renderCVF(int handle) {
    // Simulate rendering operation
}

// Placeholder for freeStream.
// Assumed signature: void freeStream(int *handle_ptr);
// The original code passed `&local_18`, implying it expects a pointer to the handle.
void freeStream(int *handle_ptr) {
    if (handle_ptr != NULL) {
        *handle_ptr = 0; // Clear the handle value
    }
}

// Function: receiveIt
// buffer: Pointer to the buffer where data will be stored.
// total_bytes_to_receive: The total number of bytes to attempt to receive.
int receiveIt(void *buffer, size_t total_bytes_to_receive) {
    int bytes_received_current_call;
    size_t bytes_accumulated = 0; // Accumulates total bytes received

    if (buffer == NULL) {
        return 0;
    }

    // Loop until all requested bytes are received or an error occurs.
    // `total_bytes_to_receive` will store the *remaining* bytes to receive in each iteration.
    while (total_bytes_to_receive > 0) {
        // Call the 'receive' function.
        // The buffer offset is `bytes_accumulated`.
        int ret = receive(0, (char*)buffer + bytes_accumulated, total_bytes_to_receive, &bytes_received_current_call);
        if (ret != 0) {
            printf("[ERROR] Failed to receive\n");
            return 0;
        }
        if (bytes_received_current_call <= 0) {
            printf("[ERROR] Receive returned non-positive bytes. Connection might be closed prematurely.\n");
            return 0;
        }
        bytes_accumulated += bytes_received_current_call;
        total_bytes_to_receive -= bytes_received_current_call; // Update remaining bytes
    }
    return (int)bytes_accumulated; // Return total bytes successfully received
}

// Function: readImageData
int readImageData(void) {
    uint32_t image_size = 0; // Use uint32_t for explicit 4-byte size for image length
    void *image_buffer = NULL;
    int stream_handle = 0;

    printf("----------------Stream Me Your Video----------------\n");

    // First, receive the 4-byte image size into the 'image_size' variable.
    size_t bytes_received = receiveIt(&image_size, sizeof(uint32_t));
    if (bytes_received != sizeof(uint32_t)) {
        printf("[ERROR] Failed to receive image size.\n");
        return 0;
    }

    // Validate image size constraints
    if (image_size >= 4097) { // Max size is 4096 bytes
        printf("[ERROR] Image must be smaller than 4096 bytes\n");
        return 0;
    }

    if (image_size < 8) { // Min size is 8 bytes
        printf("[ERROR] Image must be greater than 8 bytes\n");
        return 0;
    }

    // Allocate memory for the image data
    image_buffer = malloc(image_size);
    if (image_buffer == NULL) {
        printf("[ERROR] Failed to allocate memory for image data.\n");
        return 0;
    }

    // Initialize the allocated buffer to zeros
    memset(image_buffer, 0, image_size);

    // Receive the actual image data into the allocated buffer
    bytes_received = receiveIt(image_buffer, image_size);
    if (bytes_received != image_size) {
        printf("[ERROR] Failed to receive full image data (expected %u, got %zu).\n", image_size, bytes_received);
        free(image_buffer);
        return 0;
    }

    // Initialize the stream with the received image data
    stream_handle = initStream(image_buffer, image_size);

    // Free the image buffer, assuming initStream copies the data or takes ownership
    free(image_buffer);

    return stream_handle;
}

// Function: main
int main(void) {
    int stream_handle = readImageData();
    if (stream_handle != 0) {
        renderCVF(stream_handle);
        freeStream(&stream_handle); // Pass pointer to handle as per original code
    }
    return 0;
}