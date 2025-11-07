#include <stdint.h> // For uint32_t, uint16_t, etc.
#include <stdlib.h> // For malloc, realloc, free, NULL
#include <unistd.h> // For read, write, STDIN_FILENO, STDOUT_FILENO
#include <string.h> // For memcpy (used in store_in_vault mock)

// --- Helper Functions and Globals ---

// Generic byte swapping functions
static uint16_t swap16(uint16_t val) {
    return (val << 8) | (val >> 8);
}

static uint32_t swap32(uint32_t val) {
    return (val << 24) | ((val << 8) & 0x00FF0000) | ((val >> 8) & 0x0000FF00) | (val >> 24);
}

// Mock read/write_bytes.
// These functions assume reading from STDIN_FILENO and writing to STDOUT_FILENO.
// They return 1 on full success (all bytes read/written), 0 on error or EOF.
static int read_bytes(void *buf, size_t count) {
    size_t bytes_read = 0;
    while (bytes_read < count) {
        ssize_t r = read(STDIN_FILENO, (char*)buf + bytes_read, count - bytes_read);
        if (r <= 0) { // EOF or error
            return 0;
        }
        bytes_read += r;
    }
    return 1; // Success
}

static int write_bytes(const void *buf, size_t count) {
    size_t bytes_written = 0;
    while (bytes_written < count) {
        ssize_t w = write(STDOUT_FILENO, (const char*)buf + bytes_written, count - bytes_written);
        if (w <= 0) { // Error
            return 0;
        }
        bytes_written += w;
    }
    return 1; // Success
}

// Function pointer type for message handlers
typedef uint32_t (*msg_handler_func)(uint32_t, uint32_t);

// Global array of function pointers
// Size 3 based on `param_1 < 3` check in `handle_msg` and `store_in_vault` size 0xc (3 * 4 bytes for 32-bit pointers)
msg_handler_func handlers[3];

// External functions declarations (mocked implementations provided for completeness)
void init_vault(void) {
    // Placeholder for vault initialization
}

void store_in_vault(int index, void* data, size_t size) {
    // Placeholder for storing data in vault.
    // Assumed to store the 'handlers' array based on `main`'s call.
    if (index == 0 && size == sizeof(handlers)) {
        memcpy(handlers, data, size);
    }
}

// --- Original Functions (refactored) ---

// Function: is_supported
uint32_t is_supported(int32_t param_1) {
    return (param_1 == 0 || param_1 == 1 || param_1 == 2);
}

// Function: consume_bytes
uint32_t consume_bytes(uint32_t param_1) {
    uint8_t buffer[512]; // Buffer to read bytes into
    uint32_t bytes_to_read;

    while (param_1 > 0) {
        bytes_to_read = (param_1 < sizeof(buffer)) ? param_1 : sizeof(buffer);
        if (read_bytes(buffer, bytes_to_read) == 0) {
            return 0; // Error or EOF
        }
        param_1 -= bytes_to_read;
    }
    return 1; // Success
}

// Function: handle_msg
uint32_t handle_msg(uint16_t param_1, uint32_t param_2, uint32_t param_3) {
    if (param_1 < 3) {
        if (handlers[param_1] == NULL) {
            return 0;
        } else {
            return handlers[param_1](param_2, param_3);
        }
    }
    return 0;
}

// Function: handle_msg_ping
uint32_t handle_msg_ping(uint32_t param_1, uint32_t param_2) {
    uint16_t msg_type = swap16(0);
    uint32_t ret_val = 1; // Assume success

    if (param_2 < 0x8000) {
        uint16_t msg_len_short = swap16(param_2 & 0xffff);
        
        // Structure for a 4-byte header (type + short length)
        struct __attribute__((packed)) {
            uint16_t type;
            uint16_t length;
        } header_short = { .type = msg_type, .length = msg_len_short };

        if (write_bytes(&header_short, sizeof(header_short)) == 0) {
            ret_val = 0;
        }
    } else {
        uint32_t msg_len_long = swap32(param_2 | 0x80000000); // Set MSB for long length
        
        if (write_bytes(&msg_type, sizeof(msg_type)) == 0) { // Write 2-byte type
            ret_val = 0;
        } else if (write_bytes(&msg_len_long, sizeof(msg_len_long)) == 0) { // Write 4-byte length
            ret_val = 0;
        }
    }

    if (ret_val == 1) { // Only write payload if header was sent successfully
        if (write_bytes((const void*)param_1, param_2) == 0) { // param_1 is treated as a pointer to data
            ret_val = 0;
        }
    }
    return ret_val;
}

// Function: main
uint32_t main(void) {
    void *message_buffer = NULL; // Buffer for message body
    uint16_t msg_type;           // Message type
    uint32_t msg_len;            // Message length

    init_vault();
    // Initialize handlers array. 0xc is 12 bytes, suggesting 3 * sizeof(void*) on a 32-bit system.
    store_in_vault(0, handlers, sizeof(handlers));

    int overall_success = 1; // Flag to control the main processing loop

    while (overall_success) {
        uint16_t initial_header_type_val;
        uint16_t initial_header_len_val;

        // Read the first 4 bytes of the message header (type and initial length part)
        if (read_bytes(&initial_header_type_val, sizeof(initial_header_type_val)) == 0) {
            overall_success = 0; // EOF or error
            break;
        }
        if (read_bytes(&initial_header_len_val, sizeof(initial_header_len_val)) == 0) {
            overall_success = 0; // EOF or error
            break;
        }

        msg_type = swap16(initial_header_type_val);
        uint32_t uVar2_raw = swap16(initial_header_len_val);
        msg_len = uVar2_raw & 0xffff; // Initial 16-bit length (low 16 bits)

        // Check for extended length flag (MSB of the initial length part)
        if ((uVar2_raw & 0x8000) != 0) {
            uint16_t extended_len_part; // Read the next 2 bytes for full 32-bit length
            if (read_bytes(&extended_len_part, sizeof(extended_len_part)) == 0) {
                overall_success = 0; // EOF or error
                break;
            }
            // Reconstruct the 32-bit raw length, then byte-swap and clear MSB flag
            uint32_t full_raw_len = ((uint32_t)initial_header_len_val << 16) | extended_len_part;
            msg_len = swap32(full_raw_len);
            msg_len &= 0x7fffffff; // Clear the MSB flag
        }

        int should_process_message_body_into_buffer = 1; // Flag for current message's body processing

        if (is_supported(msg_type) == 0) {
            should_process_message_body_into_buffer = 0; // Not supported, just consume bytes
        } else {
            void *new_buffer = realloc(message_buffer, msg_len);
            if (new_buffer == NULL) {
                should_process_message_body_into_buffer = 0; // realloc failed, just consume bytes
                overall_success = 0; // Stop processing further messages after this one
            }
            message_buffer = new_buffer; // Update buffer pointer
        }

        if (should_process_message_body_into_buffer) {
            // Read message body into the allocated buffer
            if (read_bytes(message_buffer, msg_len) == 0) {
                overall_success = 0; // EOF or error during body read
                break;
            }
            handle_msg(msg_type, (uint32_t)message_buffer, msg_len);
        } else {
            // Message not supported or realloc failed, consume bytes without storing
            if (consume_bytes(msg_len) == 0) {
                overall_success = 0; // Error during consume_bytes
                break;
            }
        }
    } // End of while (overall_success)

    free(message_buffer); // Free any allocated memory
    return 0;
}