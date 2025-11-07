#include <stdint.h> // For uint8_t, uint16_t, uint32_t
#include <string.h> // For memcpy
#include <stddef.h> // For size_t

// Type definitions to match the snippet's implicit types
typedef uint8_t byte;
typedef uint16_t ushort;
typedef uint32_t uint;

// Dummy declarations for external functions to allow compilation.
// In a real system, these would be provided by a library.
extern int transmit(int channel, void *buffer, size_t length, void *status_info);
extern int receive(int channel, void *buffer, size_t length, void *status_info);

// Function: link_send
void link_send(byte param_1, uint param_2, void *param_3) {
    // payload_len is the length of the data to be sent, capped by buffer size
    uint16_t payload_len = param_2;
    uint8_t header_bytes[2]; // Replaces local_165 and local_164
    uint8_t payload_buffer[339]; // Replaces auStack_163
    uint8_t transmit_status_info[8]; // Replaces local_10

    // Cap the payload length to the maximum buffer size (0x153 = 339)
    if (payload_len > 0x153) {
        payload_len = 0x153;
    }

    // Compose the two-byte header
    // header_bytes[0]: Bits 0-1 from param_1, Bit 2 is a fixed flag (0x04), Bits 3-7 from lower 5 bits of payload_len
    header_bytes[0] = (param_1 & 0x03) | 0x04 | ((payload_len & 0x1F) << 3);
    // header_bytes[1]: Bits 0-3 from upper 4 bits of payload_len
    header_bytes[1] = (payload_len >> 5) & 0x0F;

    // Copy the payload data into the internal buffer
    memcpy(payload_buffer, param_3, payload_len);

    // Transmit the header followed by the payload
    // The transmit function expects the header_bytes array to be the start of the data.
    // The total length transmitted is payload_len + 2 (for the two header bytes).
    transmit(1, header_bytes, payload_len + 2, transmit_status_info);
}

// Function: link_recv
uint32_t link_recv(uint *param_1, uint *param_2, void *param_3) {
    int receive_status;
    uint16_t header_val; // Replaces local_12, receives the two header bytes
    uint32_t receive_info[2]; // Replaces local_10, receive_info[0] holds actual received length

    // Loop until a valid header is received or an error occurs
    while ((receive_status = receive(0, &header_val, 2, receive_info)) == 0 && (receive_info[0] >= 2)) {
        // Reconstruct the 9-bit payload length from the header_val
        // (header_val >> 3) shifts the value to align the length bits, then & 0x1FF masks for 9 bits
        uint32_t payload_len = (header_val >> 3) & 0x1FF;

        // Check if the reconstructed payload length is within the valid range (max 0x153 = 339)
        if (payload_len < 0x154) { // 0x154 is 340, so payload_len must be <= 339
            // Receive the actual payload data into param_3 buffer
            receive_status = receive(0, param_3, payload_len, receive_info);

            // Check if payload reception was successful and the actual length matches expected
            if ((receive_status == 0) && (payload_len == receive_info[0])) {
                // Extract the 2-bit control field from header_val and store in param_1
                *param_1 = (uint32_t)(header_val & 0x03);
                // Store the received payload length in param_2
                *param_2 = payload_len;
                return 0; // Success
            } else {
                return 0xFFFFFFFF; // Error during payload reception or length mismatch
            }
        }
        // If payload_len is too large, the loop continues to try and receive the next header.
        // (The current header is considered invalid and discarded by not processing its payload.)
    }
    return 0xFFFFFFFF; // Error in initial header reception or insufficient header bytes
}