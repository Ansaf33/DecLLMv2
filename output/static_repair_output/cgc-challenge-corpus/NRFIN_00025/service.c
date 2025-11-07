#include <stdint.h> // For uint32_t, uint8_t, int8_t
#include <string.h> // For memset, memcpy, memcmp
#include <stddef.h> // For size_t

// --- External Function Declarations and Global Variables ---

// External function declarations (assuming these exist and link)
// These are placeholders for illustration. In a real scenario,
// their actual signatures would be in a header file.
int receive_all(int fd, void *buf, size_t count, size_t *bytes_received);
int transmit_all(int fd, const void *buf, size_t count, size_t *bytes_sent);

// Global context structure to represent the state accessed via 'iVar2' in the original code.
// The exact sizes and types of arrays are speculative based on offsets and usage.
// Dummy fields are added to ensure correct offsets for the active members.
typedef struct {
    char dummy_0[0x33bf];       // Fill up to offset 0x33bf
    int defrag_counter;         // At offset 0x33bf (original 'field_33bf')
    char dummy_1[0x33c7 - (0x33bf + sizeof(int))]; // Fill to offset 0x33c7
    uint32_t rx_processing_index; // At offset 0x33c7 (original 'field_33c7', corresponds to main's 'rx_index')
    char dummy_2[0x33df - (0x33c7 + sizeof(uint32_t))]; // Fill to offset 0x33df
    uint32_t ct_fill_index;     // At offset 0x33df (original 'field_33df', corresponds to main's 'ct_index')
    char dummy_3[0x351f - (0x33df + sizeof(uint32_t))]; // Fill to offset 0x351f
    uint8_t processed_ct_buffer[256]; // At offset 0x351f (original 'field_351f', corresponds to main's 'ct')
    char dummy_4[0x361f - (0x351f + 256)]; // Fill to offset 0x361f
    uint8_t defrag_input_buffer[256]; // At offset 0x361f (original 'field_361f', likely a buffer for defrag to read from)
} GlobalContext;

// Global instance of the context structure
GlobalContext g_ctx;

// Other global variables
uint8_t rx_buf[256]; // Buffer for received data
size_t rx_bytes;     // Number of bytes received
size_t tx_bytes;     // Number of bytes transmitted

// Constant packet data (using dummy data for compilation)
const uint8_t PKT_EXIT[0x2a] = {0x00};
const uint8_t PKT_EXIT_ACK[0x2a] = {0x01};
const uint8_t PKT_INVALID_SZ[0x100] = {0x02};
const uint8_t PKT_CONNTERM[0x100] = {0x03};
const uint8_t PKT_CONNTERM_ACK[0x100] = {0x04};
const uint8_t PKT_CONNTERM_ERR[0x100] = {0x05};
const uint8_t PKT_OTP_ACK[0x10] = {0x06};

uint8_t otp[16];          // One-time pad buffer, size 0x10
uint8_t *g_DAT_ba2bb000;  // Pointer used to access 'otp'

// Forward declarations for functions
int defrag(uint32_t param_1);
void exercise_stack(void);

// Dummy implementations for external functions to make the code compilable.
// In a real application, these would link to actual system/library functions.
int receive_all(int fd, void *buf, size_t count, size_t *bytes_received) {
    (void)fd; // Unused parameter
    (void)buf; // Unused parameter
    // Simulate receiving 'count' bytes.
    if (bytes_received) {
        *bytes_received = count;
    }
    return 0; // Simulate success
}

int transmit_all(int fd, const void *buf, size_t count, size_t *bytes_sent) {
    (void)fd;   // Unused parameter
    (void)buf;  // Unused parameter
    // Simulate transmitting 'count' bytes.
    if (bytes_sent) {
        *bytes_sent = count;
    }
    return 0; // Simulate success
}

// --- Function Implementations ---

// Function: defrag
int defrag(uint32_t param_1) {
    // 'ctx' points to the global context structure, replacing 'iVar2'.
    GlobalContext *ctx = &g_ctx;

    int return_code = 0;
    uint32_t loop_counter = 0;
    uint8_t current_byte;

    // Increment a counter in the global context
    ctx->defrag_counter++;

    // Main loop for defragmentation logic
    while (loop_counter < param_1) {
        // Check for index overflow
        if (ctx->rx_processing_index > 0xff) {
            return_code = -0xf;
            break; // Exit loop
        }

        // Read byte from input buffer and increment index
        current_byte = ctx->defrag_input_buffer[ctx->rx_processing_index];
        ctx->rx_processing_index++;

        // Process byte based on its sign bit (most significant bit)
        if ((int8_t)current_byte < 0) { // If MSB is set
            uint32_t recursive_param = current_byte & 0x7f; // Use lower 7 bits as parameter
            return_code = defrag(recursive_param); // Recursive call
            if (return_code != 0) {
                break; // Exit loop if recursive call failed
            }
        } else { // If MSB is not set
            // Store byte in output buffer and increment index
            ctx->processed_ct_buffer[ctx->ct_fill_index] = current_byte;
            ctx->ct_fill_index++;
        }
        loop_counter++;
    }

    // Decrement the counter in the global context
    ctx->defrag_counter--;
    return return_code;
}

// Function: exercise_stack
void exercise_stack(void) {
    // The original code performs a large stack allocation and memset.
    // This is translated to a large local array which is then cleared.
    // WARNING: This allocates 8MB on the stack. On systems with small default stack sizes,
    // this function may cause a stack overflow. Adjust stack limits if necessary.
    uint8_t large_stack_buffer[0x7ff000]; // 8MB - 4 bytes
    memset(large_stack_buffer, 0, sizeof(large_stack_buffer));
    return;
}

// Function: main
int main(void) {
    int operation_result;
    uint32_t loop_idx;             // Loop counter (original 'local_20')
    uint32_t expected_rx_size = 0x100; // Expected receive size (original 'local_1c')
    uint32_t current_state = 0;        // State machine variable (original 'local_18')

    exercise_stack();
    g_DAT_ba2bb000 = otp; // Initialize global pointer to OTP buffer

    // Main program loop (outer communication state machine)
    while (1) {
        // Inner loop 1: Packet reception and initial processing
        while (1) {
            // Inner loop 2: Specific packet handling
            while (1) {
                // Inner loop 3: Receive packet
                while (1) {
                    rx_bytes = 0;
                    memset(&rx_buf, 0, sizeof(rx_buf));
                    operation_result = receive_all(0, &rx_buf, sizeof(rx_buf), &rx_bytes);
                    if (operation_result != 0) {
                        return 0xfffffff9; // Error return
                    }

                    // Check for PKT_EXIT
                    if (rx_bytes == 0x2a && memcmp(&rx_buf, PKT_EXIT, 0x2a) == 0) {
                        operation_result = transmit_all(1, PKT_EXIT_ACK, 0x2a, NULL);
                        if (operation_result != 0) {
                            return 0xfffffff8; // Error return
                        }
                        return 0; // Successful exit
                    }

                    // Check if received size matches expected
                    if (expected_rx_size == rx_bytes) break; // Exit Inner loop 3

                    // If size mismatch, send error and continue receiving
                    operation_result = transmit_all(1, PKT_INVALID_SZ, sizeof(PKT_INVALID_SZ), NULL);
                    if (operation_result != 0) {
                        return 0xfffffff8; // Error return
                    }
                } // End Inner loop 3

                // State 2 handling
                if (current_state == 2) {
                    if (memcmp(&rx_buf, PKT_CONNTERM, sizeof(PKT_CONNTERM)) == 0) {
                        current_state = 1;
                        expected_rx_size = 0x10;
                    } else if (rx_buf[0] == 0xFF) { // Check first byte for -1 (0xFF)
                        transmit_all(1, &rx_buf, 0xff, NULL); // Transmit 0xff bytes of the received buffer
                    } else {
                        // Prepare global context for defrag operation
                        g_ctx.rx_processing_index = 0;
                        g_ctx.ct_fill_index = 0;
                        memset(g_ctx.processed_ct_buffer, 0, sizeof(g_ctx.processed_ct_buffer));
                        memcpy(g_ctx.defrag_input_buffer, rx_buf, sizeof(rx_buf));

                        // Call defrag
                        defrag(0x100);

                        // Post-defrag processing (XOR with OTP)
                        for (loop_idx = 0; loop_idx < g_ctx.ct_fill_index; loop_idx++) {
                            g_ctx.processed_ct_buffer[loop_idx] ^= g_DAT_ba2bb000[loop_idx & 0xf];
                        }
                        // Transmit processed data
                        operation_result = transmit_all(1, g_ctx.processed_ct_buffer, g_ctx.ct_fill_index, &tx_bytes);
                        if (operation_result != 0) {
                            return 0xfffffff8; // Error return
                        }
                    }
                    break; // Exit Inner loop 2 after handling state 2
                }
                break; // Exit Inner loop 2 if current_state != 2
            } // End Inner loop 2

            // State validation (current_state > 2 check)
            if (current_state > 2) {
                return 0xfffffff3; // Error return
            }

            // State 0 handling
            if (current_state == 0) {
                if (memcmp(&rx_buf, PKT_CONNTERM, sizeof(PKT_CONNTERM)) == 0) {
                    operation_result = transmit_all(1, PKT_CONNTERM_ACK, sizeof(PKT_CONNTERM_ACK), NULL);
                    if (operation_result != 0) {
                        return 0xfffffff8; // Error return
                    }
                    current_state = 1;
                    expected_rx_size = 0x10;
                } else {
                    operation_result = transmit_all(1, PKT_CONNTERM_ERR, sizeof(PKT_CONNTERM_ERR), NULL);
                    if (operation_result != 0) {
                        return 0xfffffff8; // Error return
                    }
                }
                break; // Exit Inner loop 1 after handling state 0
            }
            break; // Exit Inner loop 1 if current_state != 0
        } // End Inner loop 1

        // State 1 handling (after inner loops)
        if (current_state != 1) {
            return 0xfffffff3; // Error return
        }
        memcpy(g_DAT_ba2bb000, &rx_buf, 0x10); // Copy 0x10 bytes (OTP data)
        current_state = 2;
        expected_rx_size = 0x100;
        operation_result = transmit_all(1, PKT_OTP_ACK, 0x10, NULL);
        if (operation_result != 0) {
            return 0xfffffff8; // Error return
        }
    } // End main loop
    // This point should conceptually not be reached due to explicit returns within the loops.
    // Added for completeness, though unreachable in this logic flow.
    return 0;
}