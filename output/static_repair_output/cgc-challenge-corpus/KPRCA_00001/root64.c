#include <stdint.h> // For uint8_t
#include <stddef.h> // For size_t (though not strictly required for this specific snippet, it's good practice for buffer sizes)

// Function: root64_decode_char
int root64_decode_char(char param_1) {
    if (param_1 >= '0' && param_1 <= '9') {
        return param_1 - '0';
    }
    if (param_1 >= 'A' && param_1 <= 'Z') {
        return param_1 - 'A' + 10;
    }
    if (param_1 >= 'a' && param_1 <= 'z') {
        return param_1 - 'a' + 36;
    }
    if (param_1 == ':') {
        return 62;
    }
    if (param_1 == ';') {
        return 63;
    }
    return -1;
}

// Function: root64_decode
int root64_decode(uint8_t *output_buf, const char *input_str) {
    int output_idx = 0;             // local_8: current index in output_buf
    unsigned int current_byte_acc = 0; // local_c: accumulates bits for the current output byte
    int bits_left_in_output_byte = 8; // local_10: bits remaining to fill the current output byte
    
    // Loop until null terminator or padding character is encountered
    while (*input_str != '\0' && *input_str != '=') {
        int decoded_val = root64_decode_char(*input_str); // local_14: 6-bit value of current base64 char
        if (decoded_val == -1) {
            break; // Invalid character encountered, stop decoding
        }

        // `bits_from_char_processed` tracks how many bits from `decoded_val` are used
        // for the current accumulation step (initially all 6 bits)
        int bits_from_char_processed = 6; 

        if (bits_left_in_output_byte < 7) {
            // Case 1: The current output byte needs `bits_left_in_output_byte` (1-6) bits to be completed.
            // Take the necessary MSBs from `decoded_val` to fill the current output byte.
            output_buf[output_idx] = (uint8_t)(current_byte_acc | (decoded_val >> (6 - bits_left_in_output_byte)));
            output_idx++; // Move to the next output byte slot

            // Calculate how many bits *remain* in `decoded_val` after filling the output byte.
            bits_from_char_processed = 6 - bits_left_in_output_byte; 
            
            // Mask `decoded_val` to keep only these remaining LSBs.
            decoded_val &= (1U << bits_from_char_processed) - 1; 

            // Reset state for the next output byte.
            current_byte_acc = 0;
            bits_left_in_output_byte = 8;
        }
        
        // Add the *remaining* `decoded_val` bits (which is `bits_from_char_processed` bits)
        // to `current_byte_acc`. These bits will form the start of the next output byte.
        current_byte_acc |= (unsigned int)decoded_val << (bits_left_in_output_byte - bits_from_char_processed);
        bits_left_in_output_byte -= bits_from_char_processed;
        
        input_str++; // Move to the next input character
    }
    return output_idx;
}

// Function: root64_encode
unsigned int root64_encode(char *output_buf, const uint8_t *input_buf) {
    static const char encoding_table[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz:;";
    
    unsigned int output_idx = 0; // local_c: current index in output_buf
    unsigned int current_6bit_chunk = 0; // local_10: accumulates bits for the next 6-bit chunk
    int bits_processed_from_input_byte = 0; // local_14: bits from current input byte already processed (0, 2, or 4)
    
    while (*input_buf != '\0') {
        uint8_t current_input_byte = *input_buf;

        // Encode the first part of the current 6-bit chunk
        // This combines `current_6bit_chunk` (lower bits from previous input byte)
        // with higher bits from `current_input_byte`.
        output_buf[output_idx] = encoding_table[current_6bit_chunk | (current_input_byte >> (bits_processed_from_input_byte + 2U))];
        output_idx++;

        // Prepare `current_6bit_chunk` for the next base64 character.
        // This takes the remaining lower bits from `current_input_byte`
        // and shifts them to the correct position for the next 6-bit chunk.
        current_6bit_chunk = (unsigned int)current_input_byte << (4U - bits_processed_from_input_byte) & 0x3f;
        
        bits_processed_from_input_byte += 2; // Mark 2 more bits from the current input byte as processed
        input_buf++; // Advance to the next input byte (potentially)

        if (bits_processed_from_input_byte == 6) {
            // If 6 bits from the current input byte have been processed,
            // `current_6bit_chunk` now holds a complete 6-bit value.
            output_buf[output_idx] = encoding_table[current_6bit_chunk];
            output_idx++;

            // Reset for processing the next full input byte
            current_6bit_chunk = 0;
            bits_processed_from_input_byte = 0;
        }
    }
    
    // After the loop, handle any remaining bits in `current_6bit_chunk`
    // (This occurs if the total input bytes length is not a multiple that aligns perfectly with 6-bit chunks)
    if (bits_processed_from_input_byte > 0) {
        output_buf[output_idx++] = encoding_table[current_6bit_chunk];
    }
    
    // Add padding '=' characters to ensure the output length is a multiple of 4
    while ((output_idx % 4) != 0) {
        output_buf[output_idx++] = '=';
    }
    output_buf[output_idx] = '\0'; // Null-terminate the output string
    
    return output_idx;
}