#include <stdlib.h> // For calloc, free, exit, size_t
#include <string.h> // For memcpy
#include <stdint.h> // For uint8_t, uint32_t, uintptr_t
#include <stdio.h>  // For fprintf in _terminate and mock I/O
#include <stddef.h> // For size_t

// Define standard return codes
#define SUCCESS 0
#define ERROR -1

// Forward declaration for DestroyTransportMessage
// This is needed because ReceiveTransportMessage calls DestroyTransportMessage.
typedef struct TransportMessage TransportMessage;
void DestroyTransportMessage(TransportMessage **msg_ptr);


// --- Mock Functions and Error Handling ---

// Mock _terminate function (replaces the decompiler's _terminate)
static void _terminate(void) {
    fprintf(stderr, "Critical error terminated program.\n");
    exit(EXIT_FAILURE);
}

// Mock receive/transmit functions
// In a real system, these would interact with network sockets, files, etc.
// They return 0 on success, non-zero on error.
// The actual number of bytes transferred is written to `bytes_transferred_out`.
// These mocks simulate successful transfer of `count` bytes.
static int receive(void *buffer, size_t count, size_t *bytes_transferred_out, int flags) {
    (void)flags; // Unused parameter
    if (count == 0) {
        *bytes_transferred_out = 0;
        return SUCCESS;
    }
    
    // Simulate reading some data (e.g., from a dummy buffer or stdin)
    // Fill with arbitrary data for testing.
    for (size_t i = 0; i < count; ++i) {
        ((uint8_t*)buffer)[i] = (uint8_t)(i % 256); // Fill with dummy data
    }
    *bytes_transferred_out = count;
    return SUCCESS;
}

static int transmit(const void *buffer, size_t count, size_t *bytes_transferred_out, int flags) {
    (void)flags; // Unused parameter
    (void)buffer; // Unused parameter
    if (count == 0) {
        *bytes_transferred_out = 0;
        return SUCCESS;
    }
    
    // Simulate writing data
    // printf("Transmitting %zu bytes...\n", count); // Uncomment for verbose mock
    *bytes_transferred_out = count;
    return SUCCESS;
}


// --- Core I/O Functions ---

// Function: ReceiveBytes
// Reads `total_bytes_to_receive` bytes into `buffer`.
// Returns SUCCESS (0) on success, ERROR (-1) on failure.
int ReceiveBytes(void *buffer, size_t total_bytes_to_receive) {
    if (buffer == NULL && total_bytes_to_receive > 0) {
        return ERROR;
    }
    if (total_bytes_to_receive == 0) {
        return SUCCESS; // Nothing to receive
    }

    size_t bytes_received_total = 0;
    while (bytes_received_total < total_bytes_to_receive) {
        size_t bytes_to_request = total_bytes_to_receive - bytes_received_total;
        size_t bytes_received_this_call = 0; // Output parameter for receive()

        int io_result = receive((uint8_t*)buffer + bytes_received_total,
                                bytes_to_request,
                                &bytes_received_this_call,
                                0); // Flag 0 for receive

        if (io_result != SUCCESS) { // Error from receive()
            return ERROR;
        }
        
        if (bytes_received_this_call == 0) {
            // No bytes received, but no error reported. Original code treated as critical.
            return ERROR;
        }
        bytes_received_total += bytes_received_this_call;
    }
    return SUCCESS;
}

// Function: TransmitBytes
// Writes `total_bytes_to_transmit` bytes from `buffer`.
// Returns SUCCESS (0) on success, ERROR (-1) on failure.
int TransmitBytes(const void *buffer, size_t total_bytes_to_transmit) {
    if (buffer == NULL && total_bytes_to_transmit > 0) {
        return ERROR;
    }
    if (total_bytes_to_transmit == 0) {
        return SUCCESS; // Nothing to transmit
    }

    size_t bytes_transmitted_total = 0;
    while (bytes_transmitted_total < total_bytes_to_transmit) {
        size_t bytes_to_request = total_bytes_to_transmit - bytes_transmitted_total;
        size_t bytes_transmitted_this_call = 0; // Output parameter for transmit()

        int io_result = transmit((const uint8_t*)buffer + bytes_transmitted_total,
                                 bytes_to_request,
                                 &bytes_transmitted_this_call,
                                 1); // Flag 1 for transmit

        if (io_result != SUCCESS) { // Error from transmit()
            return ERROR;
        }
        
        if (bytes_transmitted_this_call == 0) {
            // No bytes transmitted, but no error reported. Original code treated as critical.
            return ERROR;
        }
        bytes_transmitted_total += bytes_transmitted_this_call;
    }
    return SUCCESS;
}


// --- TransportMessage Structure and Helpers ---

// Represents the "TransportMessage" structure inferred from the code.
// Designed to be 16 bytes, matching the original calloc(0x10, ...) behavior,
// implying a 32-bit context where pointers are 4 bytes.
struct TransportMessage {
    uint8_t type;
    uint8_t _padding[3]; // Padding to ensure 4-byte alignment for 'size'
    uint32_t size;       // Total size of data_buffer
    uint32_t data_buffer_ptr_val; // Value of the pointer to dynamically allocated data
    uint32_t read_offset; // Current read position within data_buffer
};
// This structure is 1+3+4+4+4 = 16 bytes.

// Helper to get the actual data pointer from the struct (read-only)
static inline const void* get_data_ptr_const(const TransportMessage *msg) {
    return (const void*)(uintptr_t)msg->data_buffer_ptr_val;
}

// Helper to get the actual data pointer from the struct (read-write)
static inline void* get_data_ptr(TransportMessage *msg) {
    return (void*)(uintptr_t)msg->data_buffer_ptr_val;
}

// Helper to set the actual data pointer in the struct
static inline void set_data_ptr(TransportMessage *msg, void* ptr) {
    msg->data_buffer_ptr_val = (uint32_t)(uintptr_t)ptr;
}


// --- TransportMessage Functions ---

// Function: ResetTransportMessage
// Resets the read_offset of a TransportMessage to 0.
TransportMessage * ResetTransportMessage(TransportMessage *msg) {
    if (msg != NULL) {
        msg->read_offset = 0; // Reset read position
    }
    return msg; // Return the modified message or NULL if input was NULL
}

// Function: ReceiveTransportMessage
// Receives a TransportMessage from the underlying transport.
// msg_out: Pointer to a TransportMessage* variable to store the new message.
// Returns SUCCESS (0) on success, ERROR (-1) on failure.
int ReceiveTransportMessage(TransportMessage **msg_out) {
    if (msg_out == NULL) {
        return ERROR;
    }

    TransportMessage *msg = (TransportMessage *)calloc(1, sizeof(TransportMessage));
    if (msg == NULL) {
        *msg_out = NULL;
        return ERROR;
    }

    // Read the type byte (1 byte)
    if (ReceiveBytes(&msg->type, 1) != SUCCESS) {
        DestroyTransportMessage(&msg); // Free msg struct
        *msg_out = NULL;
        return ERROR;
    }

    if (msg->type == 0x0) { // Assuming type 0 is a valid message type
        // Read the size (4 bytes)
        if (ReceiveBytes(&msg->size, sizeof(msg->size)) != SUCCESS) {
            DestroyTransportMessage(&msg);
            *msg_out = NULL;
            return ERROR;
        }

        // Check data size limit (0x1001 = 4097, so max data size is 4096)
        if (msg->size < 0x1001) { // Max size is 4096
            void *data_buffer = NULL;
            if (msg->size > 0) { // Only allocate if size is greater than 0
                data_buffer = calloc(1, msg->size);
                if (data_buffer == NULL) {
                    DestroyTransportMessage(&msg);
                    *msg_out = NULL;
                    return ERROR;
                }
            }
            set_data_ptr(msg, data_buffer); // Store the pointer value (NULL if size is 0)

            // Read the actual data (if size > 0)
            if (msg->size > 0 && ReceiveBytes(get_data_ptr(msg), msg->size) != SUCCESS) {
                DestroyTransportMessage(&msg);
                *msg_out = NULL;
                return ERROR;
            }

            *msg_out = msg; // Assign the newly created message
            return SUCCESS;
        }
    }
    
    // If type is not 0, or size limit exceeded, or other error
    DestroyTransportMessage(&msg); // Free allocated memory for msg and data_buffer
    *msg_out = NULL;
    return ERROR;
}

// Function: DestroyTransportMessage
// Frees the memory associated with a TransportMessage and sets the pointer to NULL.
void DestroyTransportMessage(TransportMessage **msg_ptr) {
    if (msg_ptr == NULL) {
        return; // Handle NULL gracefully
    }

    TransportMessage *msg = *msg_ptr;
    if (msg != NULL) {
        // Free the data_buffer if it was allocated
        void *data_buffer = get_data_ptr(msg);
        if (data_buffer != NULL) {
            free(data_buffer);
            set_data_ptr(msg, NULL); // Clear the pointer in the struct
        }

        // Free the TransportMessage structure itself
        free(msg);
        *msg_ptr = NULL; // Clear the caller's pointer
    }
    return;
}

// Function: SendTransportMessage
// Sends a TransportMessage.
// msg: Pointer to the TransportMessage to send.
void SendTransportMessage(const TransportMessage *msg) {
    if (msg == NULL) {
        return; // Or handle error appropriately
    }

    // Transmit the type byte (1 byte)
    // In a robust system, these checks would be essential.
    if (TransmitBytes(&msg->type, 1) != SUCCESS) {
        // Handle error, e.g., log, return an error code, or _terminate()
        return;
    }
    
    // Transmit the size (4 bytes)
    if (TransmitBytes(&msg->size, sizeof(msg->size)) != SUCCESS) {
        // Handle error
        return;
    }
    
    // Transmit the actual data_buffer (if size > 0)
    if (msg->size > 0) {
        if (TransmitBytes(get_data_ptr_const(msg), msg->size) != SUCCESS) {
            // Handle error
            return;
        }
    }
    return;
}

// Function: ReadFromTransportMessage
// Reads `count` bytes from a TransportMessage into `dest_buffer`.
// msg: Pointer to the TransportMessage.
// dest_buffer: Destination buffer to copy data into.
// count: Number of bytes to read.
// Returns SUCCESS (0) on success, ERROR (-1) on failure.
int ReadFromTransportMessage(TransportMessage *msg, void *dest_buffer, size_t count) {
    if (msg == NULL || dest_buffer == NULL) {
        return ERROR;
    }

    void *data_buffer = get_data_ptr(msg);
    if (data_buffer == NULL && count > 0) { // No data buffer allocated, but trying to read
        return ERROR;
    }
    if (count == 0) { // Nothing to read
        return SUCCESS;
    }

    // Check count limit and bounds
    // Original check `(int)param_3 < 0x1001` means count < 4097.
    // Original check `msg->read_offset + count <= msg->size`.
    if (count < 0x1001 && (msg->read_offset + count <= msg->size)) {
        // Perform the copy
        memcpy(dest_buffer,
               (uint8_t*)data_buffer + msg->read_offset, // Source: data_buffer + current_offset
               count);
        
        // Update the read offset
        msg->read_offset += count;
        return SUCCESS;
    } else {
        return ERROR; // Read count out of bounds or too large
    }
}