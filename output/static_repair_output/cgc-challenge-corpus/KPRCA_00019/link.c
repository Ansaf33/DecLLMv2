#include <stdint.h> // For uint8_t, uint16_t, uint32_t
#include <string.h> // For memcpy

// Assuming external functions transmit and receive have these signatures.
// The return value is typically 0 for success, non-zero for error.
// The last parameter (uint32_t *status_or_len) is assumed to be an output parameter
// that indicates the actual number of bytes processed or a status code.
extern int transmit(int channel, const void *buffer, size_t length, uint32_t *status_or_len);
extern int receive(int channel, void *buffer, size_t max_length, uint32_t *status_or_len);

// Function: link_send
void link_send(uint8_t param_1, uint32_t param_2, void *param_3) {
    // Max data length is 0x153 (339 bytes)
    const uint32_t MAX_DATA_LEN = 0x153;
    // Buffer for 2-byte header + max data payload
    uint8_t tx_buffer[2 + MAX_DATA_LEN]; 

    // Cap the data length to the maximum allowed
    if (param_2 > MAX_DATA_LEN) {
        param_2 = MAX_DATA_LEN;
    }

    // The length value (param_2) is 9 bits (max 0x1FF, which is 511).
    // It's packed into two header bytes along with param_1 and a fixed bit.
    uint16_t len_val = (uint16_t)param_2;

    // Construct the first header byte (tx_buffer[0]):
    // Bits 0-1: param_1 (2 bits)
    // Bit 2: Fixed to 1 (from original '4')
    // Bits 3-7: Lower 5 bits of len_val, shifted by 3
    tx_buffer[0] = (uint8_t)( ((len_val & 0x1F) << 3) | (param_1 & 0x03) | 0x04 );

    // Construct the second header byte (tx_buffer[1]):
    // Bits 0-3: Upper 4 bits of len_val (since len_val is 9 bits total)
    tx_buffer[1] = (uint8_t)( len_val >> 5 );

    // Copy the data payload into the buffer, starting after the 2-byte header
    memcpy(tx_buffer + 2, param_3, param_2);

    // Array to hold status or actual transmitted length from the transmit function
    uint32_t transmit_result[2]; 
    transmit(1, tx_buffer, param_2 + 2, transmit_result);
}

// Function: link_recv
uint32_t link_recv(uint32_t *param_1, uint32_t *param_2, uint8_t *param_3) {
    uint16_t header_val; // To receive the 2-byte header
    // Array to hold status or actual received length from the receive function
    uint32_t receive_result[2]; 

    // Loop to receive the 2-byte header.
    // Continue if receive is successful (returns 0) AND at least 2 bytes (header) were received.
    while (receive(0, &header_val, sizeof(header_val), receive_result) == 0 && receive_result[0] >= sizeof(header_val)) {
        // Extract the 9-bit length from the received header
        uint16_t extracted_len = (header_val >> 3) & 0x1ff;

        // Check if the extracted length is within the valid range (0 to 0x153, i.e., < 0x154)
        if (extracted_len < 0x154) { 
            // Attempt to receive the data payload
            if (receive(0, param_3, extracted_len, receive_result) == 0 && receive_result[0] == extracted_len) {
                // Data reception successful
                *param_1 = (uint32_t)((uint8_t)header_val & 0x03); // Extract 2-bit type/channel
                *param_2 = (uint32_t)extracted_len;              // Output the extracted data length
                return 0; // Success
            } else {
                // Data reception failed (e.g., error from receive, or actual length mismatch)
                return 0xFFFFFFFF; // Failure
            }
        } else {
            // Extracted length is out of bounds
            return 0xFFFFFFFF; // Failure
        }
    }
    // Header reception failed or insufficient bytes received for header
    return 0xFFFFFFFF; // Failure
}