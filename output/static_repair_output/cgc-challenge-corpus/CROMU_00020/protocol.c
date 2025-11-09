#include <stdint.h> // For uint32_t, uint8_t
#include <stdlib.h> // For malloc, free, NULL
#include <stddef.h> // For size_t

// --- External / assumed functions ---
// These functions are placeholders and their exact behavior is inferred
// from the decompiled code's usage patterns. In a real system, these
// would be provided by a specific library or OS.

// Critical error handler. Assumed to halt execution.
void _terminate(int error_code);

// Internal resource allocation/deallocation.
// Returns 0 on success, non-zero on failure.
// These seem to manage some system-level resource, not the frame memory itself.
int allocate(void);
int deallocate(void);

// Communication functions.
// `buffer`: pointer to data to send/receive.
// `len`: expected length of data in bytes.
// `actual_len`: pointer to store the actual number of bytes sent/received.
// Returns 0 on success, non-zero on failure.
int receive(void* buffer, size_t len, size_t* actual_len);
int transmit(const void* buffer, size_t len, size_t* actual_len);

// --- Frame Structure Definition ---
// Based on the usage in the decompiled code, a frame consists of:
// - A 32-bit type identifier.
// - A 32-bit size for the data payload (in bytes).
// - A pointer to a dynamically allocated data buffer.
typedef struct {
    uint32_t type;
    uint32_t size;
    uint8_t *data; // Pointer to the data payload (NULL if size is 0)
} frame_t;

// Maximum allowed size for the data payload, based on the `0x10000` check.
#define MAX_FRAME_DATA_SIZE 0x10000

// --- Function Implementations ---

// Allocates a new frame_t structure and its data buffer if `data_size` is non-zero.
// `type`: The frame type identifier.
// `data_size`: The size of the data payload in bytes.
// Returns a pointer to the newly allocated frame_t on success, terminates on failure.
frame_t *allocate_frame(uint32_t type, uint32_t data_size) {
    // Internal resource allocation (as seen in original code)
    if (allocate() != 0) {
        _terminate(0xffffffff);
    }

    frame_t *frame = (frame_t *)malloc(sizeof(frame_t));
    if (frame == NULL) {
        _terminate(0xffffffff); // Memory allocation failure for frame header
    }

    frame->type = type;
    frame->size = data_size;
    frame->data = NULL; // Initialize data pointer to NULL

    if (data_size != 0) {
        if (data_size > MAX_FRAME_DATA_SIZE) {
            free(frame); // Clean up header before terminating
            _terminate(0xffffffff);
        }
        // Another internal resource allocation (as seen in original code)
        if (allocate() != 0) {
            free(frame); // Clean up header before terminating
            _terminate(0xffffffff);
        }
        frame->data = (uint8_t *)malloc(data_size);
        if (frame->data == NULL) {
            // Data buffer allocation failed, clean up previous allocations.
            // Note: The `allocate()` call above succeeded, but its corresponding
            // `deallocate()` is not called here, mirroring the original decompiled logic
            // which implies `_terminate` handles global cleanup or `allocate` is reference counted.
            free(frame);
            _terminate(0xffffffff); // Memory allocation failure for data buffer
        }
    }
    return frame;
}

// Frees the memory associated with a frame, including its data payload.
// Also calls internal `deallocate` functions.
// `frame`: Pointer to the frame_t structure to free.
void free_frame(frame_t *frame) {
    if (frame == NULL) {
        return; // Nothing to free
    }

    if (frame->size != 0) { // Only free data and call deallocate if data was present
        if (frame->data != NULL) {
            free(frame->data); // Free the data payload
        }
        // Internal resource deallocation (as seen in original code)
        if (deallocate() != 0) {
            _terminate(0xffffffff);
        }
    }
    free(frame); // Free the frame_t structure itself
    // Another internal resource deallocation (as seen in original code)
    if (deallocate() != 0) {
        _terminate(0xffffffff);
    }
}

// Receives a frame from the communication channel.
// Returns a pointer to a newly allocated and populated frame_t on success,
// terminates on failure.
frame_t *receive_frame(void) {
    uint32_t header_buffer[2]; // Buffer for type and size
    size_t actual_received_len;

    // Receive the 8-byte header (type and size)
    if (receive(header_buffer, sizeof(header_buffer), &actual_received_len) != 0) {
        _terminate(0xffffffff);
    }
    if (actual_received_len != sizeof(header_buffer)) {
        _terminate(0xffffffff); // Did not receive expected header size
    }

    uint32_t type = header_buffer[0];
    uint32_t data_size = header_buffer[1];

    // Allocate the frame structure and its data buffer
    frame_t *frame = allocate_frame(type, data_size);
    // `allocate_frame` handles its own termination on failure.

    if (data_size != 0) {
        if (frame->data == NULL) {
            // This should ideally not happen if allocate_frame is robust,
            // but as a safety check, if data_size > 0 and data is NULL.
            free_frame(frame); // Clean up partially allocated frame
            _terminate(0xffffffff);
        }
        // Receive the data payload into the allocated buffer
        if (receive(frame->data, data_size, &actual_received_len) != 0) {
            free_frame(frame); // Clean up before terminating
            _terminate(0xffffffff);
        }
        if (actual_received_len != data_size) {
            free_frame(frame); // Clean up before terminating
            _terminate(0xffffffff); // Did not receive expected data_size bytes
        }
    }
    return frame;
}

// Sends a frame over the communication channel.
// `frame`: Pointer to the frame_t structure to send.
void send_frame(frame_t *frame) {
    size_t actual_sent_len;

    // Consistency check: if size is non-zero, data pointer must not be NULL.
    // This matches the original decompiled logic `if ((size != 0) && (data_ptr == 0))`.
    if (frame->size != 0 && frame->data == NULL) {
        _terminate(0xffffffff);
    }

    uint32_t header[2] = {frame->type, frame->size};

    // Transmit the 8-byte header (type and size)
    if (transmit(header, sizeof(header), &actual_sent_len) != 0) {
        _terminate(0xffffffff);
    }
    if (actual_sent_len != sizeof(header)) {
        _terminate(0xffffffff); // Did not send expected header size
    }

    if (frame->size != 0) {
        // Transmit the data payload
        if (transmit(frame->data, frame->size, &actual_sent_len) != 0) {
            _terminate(0xffffffff);
        }
        if (actual_sent_len != frame->size) {
            _terminate(0xffffffff); // Did not send expected data_size bytes
        }
    }
}

// Receives a frame and verifies its type.
// If the type does not match, the frame is freed, and the program terminates.
// Returns a pointer to the allocated frame_t on success.
frame_t *expect_frame(uint32_t expected_type) {
    frame_t *frame = receive_frame();
    if (expected_type != frame->type) {
        free_frame(frame); // Clean up the received frame before terminating
        _terminate(0xffffffff);
    }
    return frame;
}

// Creates and sends an empty frame (data_size = 0).
// `type`: The frame type identifier.
void send_empty_frame(uint32_t type) {
    // Create a temporary frame on the stack.
    // An empty frame has a size of 0 and no data payload.
    frame_t temp_frame = { .type = type, .size = 0, .data = NULL };
    send_frame(&temp_frame); // Pass the address of the stack-allocated frame
}

// Expects an empty frame of a specific type.
// Verifies the type and ensures the frame has no data payload.
// Frees the received frame after verification.
// `expected_type`: The expected frame type identifier.
void expect_empty_frame(uint32_t expected_type) {
    frame_t *frame = expect_frame(expected_type);
    // An empty frame must have a size of 0
    if (frame->size != 0) {
        free_frame(frame); // Clean up before terminating
        _terminate(0xffffffff);
    }
    free_frame(frame); // Frame is valid and empty, free it.
}