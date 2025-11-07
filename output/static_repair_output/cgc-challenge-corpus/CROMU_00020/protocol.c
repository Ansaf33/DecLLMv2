#include <stdint.h> // For uint32_t, uint8_t
#include <stdlib.h> // For malloc, free, exit
#include <stdio.h>  // For fprintf, printf
#include <string.h> // For memcpy, memset

// --- Dummy External Functions (for compilation and simulation) ---

/**
 * @brief Terminates the program with a given status.
 * @param status The exit status.
 */
void _terminate(int status) {
    fprintf(stderr, "Error: Terminating with status %d\n", status);
    exit(status);
}

/**
 * @brief Allocates a block of memory. Terminates the program on failure.
 * @param size The number of bytes to allocate.
 * @return A pointer to the allocated memory.
 */
void* allocate(size_t size) {
    void* ptr = malloc(size);
    if (ptr == NULL) {
        fprintf(stderr, "Memory allocation failed for size %zu\n", size);
        _terminate(-1);
    }
    printf("DEBUG: Allocated %zu bytes at %p\n", size, ptr);
    return ptr;
}

/**
 * @brief Deallocates a block of memory.
 * @param ptr A pointer to the memory to deallocate.
 */
void deallocate(void* ptr) {
    if (ptr) {
        printf("DEBUG: Deallocated %p\n", ptr);
        free(ptr);
    }
}

/**
 * @brief Simulates receiving data into a buffer.
 * @param buffer Pointer to the buffer to store received data.
 * @param max_size The maximum number of bytes to receive.
 * @param actual_size Pointer to store the actual number of bytes received.
 * @return 0 on success, non-zero on failure.
 */
int receive(uint8_t* buffer, size_t max_size, size_t* actual_size) {
    if (buffer == NULL || actual_size == NULL) {
        fprintf(stderr, "Receive: Invalid arguments\n");
        return -1;
    }
    // Simulate receiving data. In a real application, this would be network I/O.
    memset(buffer, 0xDE, max_size); // Fill with dummy data
    *actual_size = max_size;
    printf("DEBUG: Simulated receive: %zu bytes into %p\n", max_size, buffer);
    return 0; // Success
}

/**
 * @brief Simulates transmitting data from a buffer.
 * @param buffer Pointer to the data to transmit.
 * @param size The number of bytes to transmit.
 * @param actual_size Pointer to store the actual number of bytes transmitted.
 * @return 0 on success, non-zero on failure.
 */
int transmit(const uint8_t* buffer, size_t size, size_t* actual_size) {
    if (buffer == NULL || actual_size == NULL) {
        fprintf(stderr, "Transmit: Invalid arguments\n");
        return -1;
    }
    // Simulate transmitting data. In a real application, this would be network I/O.
    printf("DEBUG: Simulated transmit: %zu bytes from %p\n", size, buffer);
    *actual_size = size;
    return 0; // Success
}
// --- End Dummy External Functions ---


// Define the Frame structure
typedef struct {
    uint32_t type;     // Type of the frame
    uint32_t size;     // Size of the data buffer in bytes
    uint8_t* data;     // Pointer to the allocated data buffer
} Frame;

// Function declarations for internal use
Frame* receive_frame(void);
void free_frame(Frame* frame);


/**
 * @brief Allocates a new frame structure and its data buffer if specified.
 * @param type The type of the frame.
 * @param data_size The size of the data buffer to allocate.
 * @return A pointer to the newly allocated Frame. Terminates on error.
 */
Frame* allocate_frame(uint32_t type, uint32_t data_size) {
    Frame* frame = (Frame*)allocate(sizeof(Frame));

    frame->type = type;
    frame->size = data_size;
    frame->data = NULL; // Initialize data pointer

    if (data_size != 0) {
        if (data_size > 0x10000) { // Check for maximum allowed data size
            _terminate(-1);
        }
        frame->data = (uint8_t*)allocate(data_size); // Allocate data buffer
    }
    return frame;
}

/**
 * @brief Receives a frame and verifies its type.
 * @param expected_type The expected type of the frame.
 * @return A pointer to the received Frame. Terminates on error or type mismatch.
 */
Frame* expect_frame(uint32_t expected_type) {
    Frame* frame = receive_frame();
    if (frame == NULL) { // Should not happen if receive_frame terminates on error
        _terminate(-1);
    }
    if (expected_type != frame->type) {
        free_frame(frame); // Clean up before terminating
        _terminate(-1);
    }
    return frame;
}

/**
 * @brief Receives a frame from the underlying communication channel.
 * @return A pointer to the received Frame. Terminates on error.
 */
Frame* receive_frame(void) {
    uint32_t header_buffer[2]; // To receive type and size (8 bytes total)
    size_t actual_received_bytes;

    // Receive header (type and size)
    if (receive((uint8_t*)header_buffer, sizeof(header_buffer), &actual_received_bytes) != 0) {
        _terminate(-1);
    }
    if (actual_received_bytes != sizeof(header_buffer)) {
        _terminate(-1); // Mismatch in received header size
    }

    uint32_t type = header_buffer[0];
    uint32_t size = header_buffer[1];

    Frame* frame = allocate_frame(type, size); // Allocate frame structure and data buffer
    if (frame == NULL) { // allocate_frame terminates on failure, but for safety
        _terminate(-1);
    }

    if (frame->size != 0) {
        // Receive data payload
        if (receive(frame->data, frame->size, &actual_received_bytes) != 0) {
            free_frame(frame); // Clean up before terminating
            _terminate(-1);
        }
        if (actual_received_bytes != frame->size) {
            free_frame(frame); // Clean up before terminating
            _terminate(-1); // Mismatch in received data size
        }
    }
    return frame;
}

/**
 * @brief Sends a frame through the underlying communication channel.
 * @param frame A pointer to the Frame to send. Terminates on error.
 */
void send_frame(Frame* frame) {
    if (frame == NULL) {
        _terminate(-1);
    }

    // If data exists but the data pointer is NULL, it's an error.
    if (frame->size != 0 && frame->data == NULL) {
        _terminate(-1);
    }

    uint32_t header_buffer[2];
    header_buffer[0] = frame->type;
    header_buffer[1] = frame->size;
    size_t actual_sent_bytes;

    // Transmit header
    if (transmit((uint8_t*)header_buffer, sizeof(header_buffer), &actual_sent_bytes) != 0) {
        _terminate(-1);
    }
    if (actual_sent_bytes != sizeof(header_buffer)) {
        _terminate(-1); // Mismatch in transmitted header size
    }

    if (frame->size != 0) {
        // Transmit data payload
        if (transmit(frame->data, frame->size, &actual_sent_bytes) != 0) {
            _terminate(-1);
        }
        if (actual_sent_bytes != frame->size) {
            _terminate(-1); // Mismatch in transmitted data size
        }
    }
}

/**
 * @brief Sends an empty frame of a specific type.
 * @param type The type of the empty frame. Terminates on error.
 */
void send_empty_frame(uint32_t type) {
    Frame* frame = allocate_frame(type, 0); // Create an empty frame
    if (frame == NULL) { // allocate_frame terminates on failure, but for safety
        _terminate(-1);
    }
    send_frame(frame);
    free_frame(frame); // Free the temporary frame after sending
}

/**
 * @brief Expects and receives an empty frame of a specific type.
 * @param expected_type The expected type of the empty frame. Terminates on error or if frame is not empty.
 */
void expect_empty_frame(uint32_t expected_type) {
    Frame* frame = expect_frame(expected_type);
    if (frame == NULL) { // expect_frame terminates on failure, but for safety
        _terminate(-1);
    }
    if (frame->size != 0) { // An empty frame should have size 0
        free_frame(frame); // Clean up before terminating
        _terminate(-1);
    }
    free_frame(frame);
}

/**
 * @brief Frees the memory associated with a frame, including its data buffer.
 * @param frame A pointer to the Frame to free.
 */
void free_frame(Frame* frame) {
    if (frame == NULL) return; // Defensive check

    // If data buffer was allocated and has a non-zero size, deallocate it.
    if (frame->data != NULL && frame->size != 0) {
        deallocate(frame->data);
    }
    deallocate(frame); // Deallocate the frame structure itself
}

// Main function for demonstrating the functionality
int main() {
    printf("--- Starting Frame Operations Test ---\n");

    // Test send_empty_frame and expect_empty_frame
    printf("\n--- Test Empty Frame (Type 10) ---\n");
    send_empty_frame(10);
    expect_empty_frame(10);
    printf("Empty frame test passed.\n");

    // Test allocate_frame, send_frame, receive_frame, expect_frame, free_frame with data
    printf("\n--- Test Data Frame (Type 20, Size 16 bytes) ---\n");
    uint32_t test_data[] = {0x11223344, 0x55667788, 0xAABBCCDD, 0xEEFF0011};
    size_t data_len = sizeof(test_data);
    
    Frame* sent_frame = allocate_frame(20, data_len);
    if (sent_frame->data) {
        memcpy(sent_frame->data, test_data, data_len);
    }
    send_frame(sent_frame);
    free_frame(sent_frame);
    printf("Data frame sent.\n");

    Frame* received_frame = expect_frame(20);
    if (received_frame->size != data_len) {
        fprintf(stderr, "Received frame size mismatch! Expected %zu, got %u\n", data_len, received_frame->size);
        _terminate(-1);
    }
    if (received_frame->data) {
        if (memcmp(received_frame->data, test_data, data_len) != 0) {
            fprintf(stderr, "Received frame data mismatch!\n");
            _terminate(-1);
        }
    } else {
        fprintf(stderr, "Received frame has no data when expected!\n");
        _terminate(-1);
    }
    printf("Data frame received and verified.\n");
    free_frame(received_frame);
    printf("--- Frame Operations Test Complete ---\n");

    return 0;
}