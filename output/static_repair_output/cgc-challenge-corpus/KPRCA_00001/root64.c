#include <stdint.h> // Required for uint8_t, uint32_t

// Function: root64_decode_char
int root64_decode_char(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'A' && c <= 'Z') {
        return c - 'A' + 10;
    }
    if (c >= 'a' && c <= 'z') {
        return c - 'a' + 36;
    }
    if (c == ':') {
        return 62;
    }
    if (c == ';') {
        return 63;
    }
    return -1;
}

// Function: root64_decode
// out_buf: destination buffer (char*)
// in_str: source string (const char*)
int root64_decode(char *out_buf, const char *in_str) {
    int out_idx = 0;          // Current write position in out_buf
    uint32_t current_byte_accum = 0; // Accumulated bits for the current output byte
    int bits_remaining_in_byte = 8; // Number of bits left to fill the current output byte

    while (*in_str != '\0') {
        int decoded_6bit_val = root64_decode_char(*in_str);
        if (decoded_6bit_val == -1) { // Stop decoding on invalid characters or padding ('=')
            break; 
        }

        int bits_from_val_to_process = 6; // Initial bits from decoded_6bit_val

        if (bits_remaining_in_byte < 7) { // If current output byte is almost full (1-6 bits remaining)
            int bits_needed_to_fill = bits_remaining_in_byte;
            
            // Fill the remaining bits of the current output byte from the MSBs of decoded_6bit_val
            out_buf[out_idx] = (uint8_t)current_byte_accum | (uint8_t)(decoded_6bit_val >> (6 - bits_needed_to_fill));
            
            out_idx++; // Move to the next output byte
            
            // Update bits_from_val_to_process: these bits have been used from decoded_6bit_val
            bits_from_val_to_process -= bits_needed_to_fill;
            
            // Mask decoded_6bit_val to keep only the unused LSBs
            if (bits_from_val_to_process > 0) {
                decoded_6bit_val &= ((1 << bits_from_val_to_process) - 1);
            } else {
                decoded_6bit_val = 0; // All bits used
            }
            
            // Reset for the next output byte
            current_byte_accum = 0;
            bits_remaining_in_byte = 8;
        }
        
        // Add the remaining bits from decoded_6bit_val to current_byte_accum
        current_byte_accum |= decoded_6bit_val << (bits_remaining_in_byte - bits_from_val_to_process);
        bits_remaining_in_byte -= bits_from_val_to_process; // Update bits remaining in current_byte_accum

        in_str++;
    }
    return out_idx;
}

// Function: root64_encode
// out_buf: destination buffer (char*)
// in_buf: source buffer (const uint8_t*)
uint32_t root64_encode(char *out_buf, const uint8_t *in_buf) {
    uint32_t out_idx = 0; // Current write position in output buffer
    uint32_t pending_bits = 0; // Holds leftover bits from previous byte(s) to form the next 6-bit chunk
    int bits_in_pending = 0; // Number of bits currently in pending_bits (0, 2, or 4)
    
    const char *encoding_table = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz:;";

    while (*in_buf != 0) {
        uint8_t current_byte = *in_buf;
        
        switch (bits_in_pending) {
            case 0: // No pending bits from previous byte
                out_buf[out_idx++] = encoding_table[current_byte >> 2]; // Take 6 MSBs of current_byte
                pending_bits = (current_byte & 0x03) << 4; // Store 2 LSBs of current_byte, shifted left
                bits_in_pending = 2;
                break;
            case 2: // 2 bits pending from previous byte
                out_buf[out_idx++] = encoding_table[pending_bits | (current_byte >> 4)]; // Combine 2 pending with 4 MSBs of current_byte
                pending_bits = (current_byte & 0x0F) << 2; // Store 4 LSBs of current_byte, shifted left
                bits_in_pending = 4;
                break;
            case 4: // 4 bits pending from previous byte
                out_buf[out_idx++] = encoding_table[pending_bits | (current_byte >> 6)]; // Combine 4 pending with 2 MSBs of current_byte
                pending_bits = (current_byte & 0x3F); // Store 6 LSBs of current_byte (these are the next full 6-bit chunk)
                bits_in_pending = 6; // This state means `pending_bits` is a full 6-bit chunk
                break;
        }

        // If a full 6-bit chunk was formed and stored in `pending_bits` (case 4 leads to bits_in_pending = 6)
        if (bits_in_pending == 6) {
            out_buf[out_idx++] = encoding_table[pending_bits]; // Write the full 6-bit chunk
            pending_bits = 0; // Reset pending bits
            bits_in_pending = 0; // Reset state
        }
        in_buf++;
    }

    // After loop, handle any remaining pending bits
    if (bits_in_pending > 0) { // If there are leftover bits that form a partial 6-bit chunk
        out_buf[out_idx++] = encoding_table[pending_bits]; // Write it
    }

    // Pad with '=' characters until total length is a multiple of 4
    while ((out_idx % 4) != 0) {
        out_buf[out_idx++] = '=';
    }
    out_buf[out_idx] = '\0'; // Null terminate the output string

    return out_idx;
}