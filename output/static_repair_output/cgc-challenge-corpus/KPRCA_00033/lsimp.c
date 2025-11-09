#include <stdint.h> // For uint32_t, uint8_t
#include <stdlib.h> // For malloc, free
#include <string.h> // For memset, memmove
#include <stdbool.h> // For bool

// Function: compute_guard
// param_1: Base address of the data buffer (e.g., the start of the message)
// param_2: Length of the data to compute the guard over
uint32_t compute_guard(const uint8_t *data, uint32_t length) {
    uint32_t guard_val = 0;
    for (uint32_t i = 0; i < length; ++i) {
        // Original logic: local_c = local_c + *(byte *)(local_8 + param_1) ^ local_c << 10;
        // C operator precedence: `<<` > `^` > `+`. Parentheses ensure original intent.
        guard_val = guard_val + (data[i] ^ (guard_val << 10));
    }
    return guard_val;
}

// Structs for decode_data parameters to improve readability and type safety
typedef struct {
    const uint8_t *key_data;
    uint32_t key_len;
    uint32_t flags; // Assuming flags are stored as a uint32_t, with lower 4 bits relevant
} DecodeKeyInfo;

typedef struct {
    uint8_t *data_ptr;
    uint32_t data_len;
} DecodeBufferInfo;

// Function: decode_data
// param_1: Pointer to DecodeKeyInfo structure
// param_2: Pointer to DecodeBufferInfo structure
uint32_t decode_data(DecodeKeyInfo *key_info, DecodeBufferInfo *buffer_info) {
    if (buffer_info->data_len < 4) {
        return 0; // Not enough data
    }

    if ((key_info->flags & 0xF) == 7) {
        // Shift data 4 bytes to the left, effectively removing the first 4 bytes.
        // The size of the data block to move is the original length minus 4.
        memmove(buffer_info->data_ptr, buffer_info->data_ptr + 4, buffer_info->data_len - 4);
    }

    buffer_info->data_len -= 4; // Update length after processing header/shift

    // XOR data with key
    for (uint32_t i = 0; i < buffer_info->data_len; ++i) {
        buffer_info->data_ptr[i] ^= key_info->key_data[i % key_info->key_len];
    }

    if ((key_info->flags & 0xF) != 7) {
        // Null-terminate the buffer if the flag is not 7
        buffer_info->data_ptr[buffer_info->data_len] = 0;
    }

    return 1; // Success
}

// Function: parse_msg
// param_1: Pointer to the raw input message buffer (const uint8_t* for safety)
// param_2: Total length of the input message buffer
void * parse_msg(const uint8_t *param_msg_buf, uint32_t param_msg_len) {
    // Current pointers and lengths for parsing
    const uint8_t *current_ptr = param_msg_buf;
    uint32_t current_len = param_msg_len;
    const uint32_t original_msg_len = param_msg_len; // Keep original length for final guard calculation

    // The allocated 20-byte structure to hold parsed message information
    // Access is via byte offsets and casting to uint32_t* or void**
    uint8_t *parsed_msg_struct_ptr = NULL;
    bool parse_success = false;

    // Check minimum message length (at least 4 bytes for initial tag)
    if (param_msg_len > 3) {
        parsed_msg_struct_ptr = (uint8_t *)malloc(0x14); // Allocate 20 bytes for the result struct
        if (parsed_msg_struct_ptr != NULL) {
            memset(parsed_msg_struct_ptr, 0, 0x14); // Initialize with zeros
            parse_success = true; // Assume success initially
        }
    }

    if (parse_success) { // Proceed only if initial allocation was successful
        // Loop through message parts. The condition `current_len != 4` implies
        // that the loop processes all fields until only the final 4-byte guard remains.
        while (current_len > 3 && current_len != 4) {
            uint32_t tag = *(const uint32_t *)current_ptr; // Read the 4-byte tag
            current_ptr += 4; // Advance pointer past tag
            current_len -= 4; // Decrease remaining length

            // Store tag in the result structure at offset 0x4
            *(uint32_t *)(parsed_msg_struct_ptr + 4) = tag;

            if (tag == 0x5859454b /* 'KEYX' tag */) {
                if (current_len > 0) {
                    // Store a byte from the current position at offset 0x10
                    *(uint8_t *)(parsed_msg_struct_ptr + 0x10) = *current_ptr;
                    current_ptr += 1;
                    current_len -= 1;
                }
                if (current_len > 3) {
                    // Store length of key data at offset 0xC
                    *(uint32_t *)(parsed_msg_struct_ptr + 0xC) = *(const uint32_t *)current_ptr;
                    current_ptr += 4;
                    current_len -= 4;
                }
                uint32_t key_data_len = *(uint32_t *)(parsed_msg_struct_ptr + 0xC);
                // Guard check for key_data_len (max 0x80 bytes, and must fit in remaining buffer)
                if (key_data_len > 0x80 || current_len < key_data_len) {
                    parse_success = false;
                    break;
                }
                if (key_data_len != 0) {
                    void *key_data_ptr = malloc(key_data_len);
                    // Store pointer to allocated key data at offset 0x8
                    *(void **)(parsed_msg_struct_ptr + 0x8) = key_data_ptr;
                    if (key_data_ptr == NULL) { // Allocation failed
                        parse_success = false;
                        break;
                    }
                    memmove(key_data_ptr, current_ptr, key_data_len);
                    current_ptr += key_data_len;
                    current_len -= key_data_len;
                }
            } else if (tag < 0x5859454c /* 'KEYY' - used for branch condition */) {
                if (tag == 0x54584554 /* 'TEXT' tag */) {
                    if (current_len > 3) {
                        // Store length of text data at offset 0xC
                        *(uint32_t *)(parsed_msg_struct_ptr + 0xC) = *(const uint32_t *)current_ptr;
                        current_ptr += 4;
                        current_len -= 4;
                    }
                    uint32_t text_data_len = *(uint32_t *)(parsed_msg_struct_ptr + 0xC);
                    // Guard check for text_data_len (max 0x400 bytes, and must fit in remaining buffer)
                    if (text_data_len > 0x400 || current_len < text_data_len) {
                        parse_success = false;
                        break;
                    }
                    if (text_data_len != 0) {
                        void *text_data_ptr = malloc(text_data_len + 1); // +1 for null terminator
                        // Store pointer to allocated text data at offset 0x8
                        *(void **)(parsed_msg_struct_ptr + 0x8) = text_data_ptr;
                        if (text_data_ptr == NULL) { // Allocation failed
                            parse_success = false;
                            break;
                        }
                        memset(text_data_ptr, 0, text_data_len + 1);
                        memmove(text_data_ptr, current_ptr, text_data_len);
                        current_ptr += text_data_len;
                        current_len -= text_data_len;
                    }
                } else if (tag < 0x54584555 /* 'TEXT' + 1 - used for branch condition */) {
                    if (tag == 0x41544144 /* 'DATA' tag */) {
                        if (current_len > 3) {
                            // Store data_param1 at offset 0x8
                            *(uint32_t *)(parsed_msg_struct_ptr + 0x8) = *(const uint32_t *)current_ptr;
                            current_ptr += 4;
                            current_len -= 4;
                        }
                        uint32_t data_param1 = *(uint32_t *)(parsed_msg_struct_ptr + 0x8);
                        if (data_param1 < 1) { // Guard check
                            parse_success = false;
                            break;
                        }
                        if (current_len > 3) {
                            // Store data_length at offset 0x10
                            *(uint32_t *)(parsed_msg_struct_ptr + 0x10) = *(const uint32_t *)current_ptr;
                            current_ptr += 4;
                            current_len -= 4;
                        }
                        uint32_t data_data_len = *(uint32_t *)(parsed_msg_struct_ptr + 0x10);
                        // Guard check for data_data_len (max 0x200 bytes, and must fit in remaining buffer)
                        if (data_data_len > 0x200 || current_len < data_data_len) {
                            parse_success = false;
                            break;
                        }
                        if (data_data_len != 0) {
                            void *data_ptr = malloc(data_data_len + 1); // +1 for null terminator
                            // Store pointer to allocated data at offset 0xC
                            *(void **)(parsed_msg_struct_ptr + 0xC) = data_ptr;
                            if (data_ptr == NULL) { // Allocation failed
                                parse_success = false;
                                break;
                            }
                            memset(data_ptr, 0, data_data_len + 1);
                            memmove(data_ptr, current_ptr, data_data_len);
                            current_ptr += data_data_len;
                            current_len -= data_data_len;
                        }
                    } else if (tag == 0x4f4c4548 /* 'HELO' tag */) {
                        if (current_len > 0) {
                            // Store helo_param1 at offset 0x8 (as a byte)
                            *(uint8_t *)(parsed_msg_struct_ptr + 0x8) = *current_ptr;
                            current_ptr += 1;
                            current_len -= 1;
                        }
                        if (current_len > 0) {
                            // Store helo_param2 at offset 0x9 (as a byte)
                            *(uint8_t *)(parsed_msg_struct_ptr + 0x9) = *current_ptr;
                            current_ptr += 1;
                            current_len -= 1;
                        }
                        if (current_len > 3) {
                            // Store helo_param3 at offset 0xC
                            *(uint32_t *)(parsed_msg_struct_ptr + 0xC) = *(const uint32_t *)current_ptr;
                            current_ptr += 4;
                            current_len -= 4;
                        }
                    }
                }
            }
        } // End of while loop

        // Final guard check: if parse_success is still true and there are exactly 4 bytes left.
        // The loop condition `current_len != 4` ensures we exit when only the guard remains.
        if (parse_success && current_len == 4) {
            uint32_t received_guard = *(const uint32_t *)current_ptr;
            // Calculate the expected guard over the original message buffer, excluding the final 4 bytes.
            uint32_t expected_guard = compute_guard(param_msg_buf, original_msg_len - 4);
            if (received_guard != expected_guard) {
                parse_success = false;
            }
        } else if (parse_success && current_len != 0) {
            // If parse_success so far but current_len is not 0 (and not 4),
            // this indicates unparsed data or an incorrect message structure.
            parse_success = false;
        }
    }

    // Cleanup logic if parsing failed
    if (!parse_success && parsed_msg_struct_ptr != NULL) {
        // Retrieve the tag to determine which sub-buffers might need freeing.
        // This assumes the tag was successfully parsed and stored before failure.
        uint32_t current_tag = *(uint32_t *)(parsed_msg_struct_ptr + 4);

        if (current_tag == 0x5859454b /* 'KEYX' */ || current_tag == 0x54584554 /* 'TEXT' */) {
            void *data_ptr = *(void **)(parsed_msg_struct_ptr + 0x8);
            if (data_ptr != NULL) {
                free(data_ptr);
            }
        } else if (current_tag == 0x41544144 /* 'DATA' */) {
            void *data_ptr = *(void **)(parsed_msg_struct_ptr + 0xC);
            if (data_ptr != NULL) {
                free(data_ptr);
            }
        }
        free(parsed_msg_struct_ptr); // Free the main struct itself
        return NULL;
    }

    return parsed_msg_struct_ptr; // Return the parsed struct on success, or NULL on failure
}