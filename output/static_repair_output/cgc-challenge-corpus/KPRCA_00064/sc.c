#include <stdlib.h> // For malloc, free
#include <string.h> // For memset, memcpy, memmove, memcmp
#include <stdint.h> // For fixed-width integer types
#include <stddef.h> // For size_t

// Define a struct for bit stream state
typedef struct {
    uint8_t *buffer;
    uint32_t byte_offset;
    uint32_t bit_offset; // 0-7
} bit_state_t;

// Define a struct for sc_context
// Based on offsets: 0x5f (config_data length), 0x60 (data_buffer), 0x64 (data_length), 0x68 (compare_func)
#define SC_CONFIG_DATA_SIZE 0x5f
#define BWT_BLOCK_SIZE 512 // 0x200

typedef struct sc_context {
    uint8_t config_data[SC_CONFIG_DATA_SIZE];
    uint8_t padding_1; // Padding to align data_buffer (0x60)
    void *data_buffer; // offset 0x60 (96 bytes)
    uint32_t data_length; // offset 0x64 (100 bytes)
    // Function pointer for comparison, signature deduced from _sc_compare
    int (*compare_func)(const char *, const char *, const char *, uint32_t); // offset 0x68 (104 bytes)
} sc_context_t;

// Function prototypes (needed due to mutual dependencies or forward declarations)
int _gcd(int a, int b);
void _rot_left(uint8_t *buffer, uint32_t length, int shift_amount);
int _find_char(const char *buffer, char target_char, uint32_t length);
void _sort(char **arr, int count, sc_context_t *ctx);
void *sc_bwt(sc_context_t *ctx, int mode, uint32_t *output_len_ptr);
void *sc_mtf(sc_context_t *ctx, int mode, uint32_t *output_len_ptr);


// Function: bit_new
bit_state_t *bit_new(uint8_t *buffer_start) {
    bit_state_t *state = (bit_state_t *)malloc(sizeof(bit_state_t));
    if (state == NULL) {
        return NULL;
    }
    state->buffer = buffer_start;
    state->byte_offset = 0;
    state->bit_offset = 0;
    return state;
}

// Function: bit_read
uint8_t bit_read(bit_state_t *state, uint32_t num_bits) {
    uint8_t result = 0;
    for (uint32_t i = 0; i < num_bits; ++i) {
        if (state->bit_offset == 8) {
            state->byte_offset++;
            state->bit_offset = 0;
        }
        // Extract the bit from current byte and shift it into result
        result |= ((state->buffer[state->byte_offset] >> (7 - state->bit_offset)) & 1) << (num_bits - i - 1);
        state->bit_offset++;
    }
    return result;
}

// Function: bit_write
void bit_write(bit_state_t *state, uint8_t value, uint32_t num_bits) {
    for (uint32_t i = 0; i < num_bits; ++i) {
        if (state->bit_offset == 8) {
            state->byte_offset++;
            state->bit_offset = 0;
        }
        // Extract the bit from value and set it in the current byte
        state->buffer[state->byte_offset] |= ((value >> (7 - i)) & 1) << (7 - state->bit_offset);
        state->bit_offset++;
    }
    return;
}

// Function: _find_char
// Returns the integer representation of the pointer to the found character, or 0 if not found.
int _find_char(const char *buffer, char target_char, uint32_t length) {
    for (uint32_t i = 0; i < length; ++i) {
        if (target_char == buffer[i]) {
            return (int)(uintptr_t)(buffer + i);
        }
    }
    return 0; // Not found
}

// Function: _sc_compare
// Compares two strings s1 and s2 using a character map.
// Returns a negative value if s1 < s2, zero if s1 == s2, a positive value if s1 > s2.
int _sc_compare(const char *char_map, const char *s1, const char *s2, uint32_t max_len) {
    uint32_t i = 0;
    while (i < max_len) {
        if (s1[i] != s2[i]) {
            break;
        }
        i++;
    }

    if (i == max_len) { // Strings are equal up to max_len
        return 0;
    }

    // Compare characters based on their position in the char_map
    int s1_char_pos = _find_char(char_map, s1[i], max_len);
    int s2_char_pos = _find_char(char_map, s2[i], max_len);

    if (s1_char_pos != 0 && s2_char_pos != 0) { // Both characters found in map
        return s1_char_pos - s2_char_pos;
    }
    // Fallback to direct byte comparison if not found in map or if one is not found
    return (int)(uint8_t)s1[i] - (int)(uint8_t)s2[i];
}

// Function: sc_new
void *sc_new(const void *param_1) {
    sc_context_t *ctx = (sc_context_t *)malloc(sizeof(sc_context_t));
    if (ctx == NULL) {
        return NULL;
    }
    memset(ctx, 0, sizeof(sc_context_t));
    memcpy(ctx->config_data, param_1, SC_CONFIG_DATA_SIZE);
    ctx->compare_func = _sc_compare;
    return ctx;
}

// Function: sc_scompress
// Compresses data using BWT and MTF.
// Returns 0 on success, -1 on failure.
int sc_scompress(sc_context_t *ctx, void **compressed_data_ptr, uint32_t *compressed_data_len_out) {
    if (!ctx || !compressed_data_ptr || !compressed_data_len_out) {
        return -1;
    }

    void *original_data_buffer = ctx->data_buffer;
    uint32_t original_data_length = ctx->data_length;

    // BWT compression stage
    uint32_t bwt_output_len;
    void *bwt_output = sc_bwt(ctx, 0, &bwt_output_len); // mode 0 for compress
    if (!bwt_output) {
        return -1;
    }

    ctx->data_buffer = bwt_output; // Use BWT output as input for MTF
    ctx->data_length = bwt_output_len;

    // MTF compression stage
    uint32_t mtf_output_len;
    void *mtf_output = sc_mtf(ctx, 0, &mtf_output_len); // mode 0 for compress
    if (!mtf_output) {
        free(bwt_output);
        ctx->data_buffer = original_data_buffer; // Restore original state
        ctx->data_length = original_data_length;
        return -1;
    }

    *compressed_data_ptr = mtf_output;
    *compressed_data_len_out = mtf_output_len;

    free(bwt_output); // Free intermediate BWT buffer
    ctx->data_buffer = original_data_buffer; // Restore original state
    ctx->data_length = original_data_length;

    return 0; // Success
}

// Function: sc_sdecompress
// Decompresses data using MTF and BWT.
// Returns 0 on success, -1 on failure.
int sc_sdecompress(sc_context_t *ctx, void **decompressed_data_ptr, uint32_t *decompressed_data_len_out) {
    if (!ctx || !decompressed_data_ptr || !decompressed_data_len_out) {
        return -1;
    }

    void *original_data_buffer = ctx->data_buffer;
    uint32_t original_data_length = ctx->data_length;

    // MTF decompression stage
    uint32_t mtf_output_len;
    void *mtf_output = sc_mtf(ctx, 1, &mtf_output_len); // mode 1 for decompress
    if (!mtf_output) {
        return -1;
    }

    ctx->data_buffer = mtf_output; // Use MTF output as input for BWT
    ctx->data_length = mtf_output_len;

    // BWT decompression stage
    uint32_t bwt_output_len;
    void *bwt_output = sc_bwt(ctx, 1, &bwt_output_len); // mode 1 for decompress
    if (!bwt_output) {
        free(mtf_output);
        ctx->data_buffer = original_data_buffer; // Restore original state
        ctx->data_length = original_data_length;
        return -1;
    }

    *decompressed_data_ptr = bwt_output;
    *decompressed_data_len_out = bwt_output_len;

    free(mtf_output); // Free intermediate MTF buffer
    ctx->data_buffer = original_data_buffer; // Restore original state
    ctx->data_length = original_data_length;

    return 0; // Success
}

// Function: sc_set_data
uint32_t sc_set_data(sc_context_t *ctx, const void *data, size_t length) {
    if (ctx == NULL) {
        return 0xFFFFFFFF; // Error
    }

    if (ctx->data_buffer != NULL) {
        free(ctx->data_buffer);
    }
    ctx->data_buffer = malloc(length);
    if (ctx->data_buffer == NULL) {
        ctx->data_length = 0;
        return 0xFFFFFFFF; // Error
    }
    memcpy(ctx->data_buffer, data, length);
    ctx->data_length = length;
    return 0; // Success
}

// Function: _gcd
int _gcd(int a, int b) {
    if (b != 0) {
        a = _gcd(b, a % b);
    }
    return a;
}

// Function: _rot_left
void _rot_left(uint8_t *buffer, uint32_t length, int shift_amount) {
    if (length == 0 || shift_amount == 0) {
        return;
    }
    shift_amount %= length; // Normalize shift amount

    // Use Juggling Algorithm for rotation
    int num_sets = _gcd(shift_amount, length);
    for (int i = 0; i < num_sets; ++i) {
        uint8_t temp_char = buffer[i];
        uint32_t j = i;
        while (1) {
            uint32_t k = (j + shift_amount);
            if (k >= length) { // Handle wrap-around
                k -= length;
            }
            if (k == i) { // Cycle complete
                break;
            }
            buffer[j] = buffer[k];
            j = k;
        }
        buffer[j] = temp_char;
    }
    return;
}

// Function: _merge
// Merges two sorted subarrays into a temporary array, then copies back to source.
void _merge(char **src_arr, char **temp_arr, int left, int mid, int right, sc_context_t *ctx) {
    int i = left;     // Index for the first subarray
    int j = mid;      // Index for the second subarray
    int k = left;     // Index for the temporary array

    while (i < mid && j < right) {
        if (ctx->compare_func(ctx->config_data, src_arr[i], src_arr[j], BWT_BLOCK_SIZE) < 0) {
            temp_arr[k++] = src_arr[i++];
        } else {
            temp_arr[k++] = src_arr[j++];
        }
    }

    // Copy remaining elements of the first subarray, if any
    while (i < mid) {
        temp_arr[k++] = src_arr[i++];
    }

    // Copy remaining elements of the second subarray, if any
    while (j < right) {
        temp_arr[k++] = src_arr[j++];
    }

    // Copy merged elements back to the original array
    memcpy(src_arr + left, temp_arr + left, (right - left) * sizeof(char *));
    return;
}

// Function: _msort
// Recursive merge sort implementation.
void _msort(char **src_arr, char **temp_arr, int left, int right, sc_context_t *ctx) {
    if (1 < right - left) { // If more than one element
        int mid = left + (right - left) / 2;
        _msort(src_arr, temp_arr, left, mid, ctx);
        _msort(src_arr, temp_arr, mid, right, ctx);
        _merge(src_arr, temp_arr, left, mid, right, ctx);
    }
    return;
}

// Function: _sort
// Wrapper for merge sort, allocates temporary buffer.
void _sort(char **arr, int count, sc_context_t *ctx) {
    if (count <= 1) {
        return;
    }
    char **temp_arr = (char **)malloc(count * sizeof(char *));
    if (temp_arr == NULL) {
        // Handle allocation error
        return;
    }
    _msort(arr, temp_arr, 0, count, ctx);
    free(temp_arr);
    return;
}

// Function: sc_bwt
// Performs Burrows-Wheeler Transform (BWT) or inverse BWT.
// mode 0: compress, mode 1: decompress.
void *sc_bwt(sc_context_t *ctx, int mode, uint32_t *output_len_ptr) {
    // If data_length is too large, it returns NULL.
    // The original code used 0x1001 (4097) as a limit.
    // This implies it only works for relatively small inputs or blocks.
    if (ctx->data_length >= 0x1001) {
        return NULL;
    }

    uint8_t block_buffer[BWT_BLOCK_SIZE]; // Temporary buffer for current block
    char **suffix_pointers = NULL;        // Array of pointers to cyclic shifts/rows
    uint8_t *output_buffer = NULL;        // The final output buffer

    uint32_t current_offset = 0;        // Offset in the input data
    uint32_t block_size;                // Size of the current block
    uint32_t current_block_idx = 0;     // Index of the current block being processed

    // Calculate number of blocks and total output length
    uint32_t num_blocks = (ctx->data_length + BWT_BLOCK_SIZE - 1) / BWT_BLOCK_SIZE;
    if (mode == 0) { // Compression
        *output_len_ptr = num_blocks * sizeof(uint16_t) + ctx->data_length;
    } else { // Decompression
        // For decompression, the output_len_ptr will accumulate the actual decompressed data length.
        *output_len_ptr = 0;
    }

    // Allocate memory for suffix pointers (array of char pointers)
    suffix_pointers = (char **)malloc(BWT_BLOCK_SIZE * sizeof(char *));
    if (suffix_pointers == NULL) {
        return NULL;
    }

    // Allocate memory for each cyclic shift/row buffer
    for (uint32_t i = 0; i < BWT_BLOCK_SIZE; ++i) {
        suffix_pointers[i] = (char *)malloc(BWT_BLOCK_SIZE);
        if (suffix_pointers[i] == NULL) {
            // Clean up previously allocated pointers
            for (uint32_t j = 0; j < i; ++j) free(suffix_pointers[j]);
            free(suffix_pointers);
            return NULL;
        }
    }
    
    if (mode == 0) { // Compression - Allocate the final output buffer
        output_buffer = (uint8_t *)malloc(*output_len_ptr);
        if (output_buffer == NULL) {
            for (uint32_t i = 0; i < BWT_BLOCK_SIZE; ++i) free(suffix_pointers[i]);
            free(suffix_pointers);
            return NULL;
        }
        memset(output_buffer, 0, *output_len_ptr);
    } // For decompression, output_buffer will be allocated later once total size is known

    if (mode == 0) { // BWT Compression
        for (current_offset = 0; current_offset < ctx->data_length; current_offset += block_size) {
            block_size = ctx->data_length - current_offset;
            if (block_size > BWT_BLOCK_SIZE) {
                block_size = BWT_BLOCK_SIZE;
            }

            // Copy current block to temporary buffer
            memcpy(block_buffer, (uint8_t *)ctx->data_buffer + current_offset, block_size);

            // Generate all cyclic shifts for the current block
            for (uint32_t i = 0; i < block_size; ++i) {
                memcpy(suffix_pointers[i], block_buffer, block_size);
                _rot_left((uint8_t *)suffix_pointers[i], block_size, i);
            }
            
            // Sort the cyclic shifts lexicographically
            _sort(suffix_pointers, block_size, ctx);

            // Find the primary index (index of the original block in the sorted list)
            uint16_t primary_index = 0;
            while (primary_index < block_size && memcmp(suffix_pointers[primary_index], block_buffer, block_size) != 0) {
                primary_index++;
            }
            
            // Store primary index (2 bytes per block)
            *(uint16_t *)(output_buffer + current_block_idx * sizeof(uint16_t)) = primary_index;

            // Store the last characters of the sorted cyclic shifts (BWT output)
            uint32_t data_start_offset = num_blocks * sizeof(uint16_t);
            for (uint32_t i = 0; i < block_size; ++i) {
                output_buffer[data_start_offset + current_offset + i] = ((uint8_t *)suffix_pointers[i])[block_size - 1];
            }
            current_block_idx++;
        }
    } else { // BWT Decompression
        uint32_t decompressed_data_total_len = 0;
        uint32_t input_read_offset = 0;
        
        // First pass to determine total decompressed length for allocation
        for (uint32_t block_counter = 0; block_counter < num_blocks; ++block_counter) {
            input_read_offset += sizeof(uint16_t); // Advance past primary index
            block_size = ctx->data_length - input_read_offset;
            if (block_size > BWT_BLOCK_SIZE) {
                block_size = BWT_BLOCK_SIZE;
            }
            decompressed_data_total_len += block_size;
            input_read_offset += block_size;
        }
        *output_len_ptr = decompressed_data_total_len; // Set total length
        output_buffer = (uint8_t *)malloc(decompressed_data_total_len);
        if (output_buffer == NULL) {
            for (uint32_t i = 0; i < BWT_BLOCK_SIZE; ++i) free(suffix_pointers[i]);
            free(suffix_pointers);
            return NULL;
        }
        memset(output_buffer, 0, decompressed_data_total_len);

        input_read_offset = 0; // Reset for second pass (actual decompression)
        uint32_t output_write_offset = 0;

        for (current_block_idx = 0; current_block_idx < num_blocks; ++current_block_idx) {
            uint16_t primary_index = *(uint16_t *)((uint8_t *)ctx->data_buffer + input_read_offset);
            input_read_offset += sizeof(uint16_t);

            block_size = ctx->data_length - input_read_offset;
            if (block_size > BWT_BLOCK_SIZE) {
                block_size = BWT_BLOCK_SIZE;
            }

            // Copy BWT output (last column) for current block
            memcpy(block_buffer, (uint8_t *)ctx->data_buffer + input_read_offset, block_size);
            input_read_offset += block_size;

            // Initialize suffix_pointers for reconstruction
            for (uint32_t i = 0; i < BWT_BLOCK_SIZE; ++i) {
                memset(suffix_pointers[i], 0, BWT_BLOCK_SIZE);
            }

            // Reconstruct the original block by repeatedly sorting and prepending
            for (int i_int = block_size - 1; i_int >= 0; --i_int) {
                for (uint32_t j = 0; j < block_size; ++j) {
                    ((uint8_t *)suffix_pointers[j])[i_int] = block_buffer[j];
                }
                _sort(suffix_pointers, block_size, ctx);
            }
            
            // The original block is at primary_index in the sorted list
            memcpy(output_buffer + output_write_offset, suffix_pointers[primary_index], block_size);
            output_write_offset += block_size;
        }
    }

    // Free allocated memory for suffix pointers
    for (uint32_t i = 0; i < BWT_BLOCK_SIZE; ++i) {
        free(suffix_pointers[i]);
    }
    free(suffix_pointers);

    return output_buffer;
}

// Function: sc_mtf
// Performs Move-To-Front (MTF) encoding or decoding.
// mode 0: compress, mode 1: decompress.
void *sc_mtf(sc_context_t *ctx, int mode, uint32_t *output_len_ptr) {
    uint8_t char_list[256]; // The MTF list (char_map)
    bit_state_t *bit_stream_state = NULL; // State for bit-level I/O
    void *mtf_output_data = NULL; // The final output buffer (compressed/decompressed)

    // Initialize char_list with default values as per original snippet logic
    for (uint32_t i = 0; i < 0x20; ++i) { // 0-31
        char_list[i] = (uint8_t)i;
    }
    memcpy(char_list + 0x20, ctx->config_data, SC_CONFIG_DATA_SIZE); // 32 to (32+0x5f-1) = 126
    for (uint32_t i = 0x7F; i < 0x100; ++i) { // 127-255
        char_list[i] = (uint8_t)i;
    }

    if (mode == 0) { // MTF Compression
        uint8_t *temp_mtf_indices = (uint8_t *)malloc(ctx->data_length);
        if (temp_mtf_indices == NULL) {
            return NULL;
        }
        
        // MTF encode
        for (uint32_t i = 0; i < ctx->data_length; ++i) {
            uint8_t current_char = ((uint8_t *)ctx->data_buffer)[i];
            uint32_t j;
            for (j = 0; j < 0x100; ++j) {
                if (current_char == char_list[j]) {
                    temp_mtf_indices[i] = (uint8_t)j;
                    break;
                }
            }
            // Move the found character to the front of the list
            if (j > 0) {
                memmove(char_list + 1, char_list, j);
                char_list[0] = current_char;
            }
        }

        // Allocate buffer for compressed bitstream.
        // First uint32_t stores the original data length.
        // Then the bitstream follows. A generous allocation for the bitstream.
        uint32_t stream_buffer_size = sizeof(uint32_t) + (ctx->data_length + 1) * sizeof(uint16_t);
        mtf_output_data = malloc(stream_buffer_size);
        if (mtf_output_data == NULL) {
            free(temp_mtf_indices);
            return NULL;
        }
        memset(mtf_output_data, 0, stream_buffer_size); // Initialize to zeros

        // Store original data length at the beginning of the output buffer
        ((uint32_t *)mtf_output_data)[0] = ctx->data_length;

        // Initialize bit stream writer, starting after the length prefix
        bit_stream_state = bit_new((uint8_t *)mtf_output_data + sizeof(uint32_t));
        if (bit_stream_state == NULL) {
            free(mtf_output_data);
            free(temp_mtf_indices);
            return NULL;
        }

        for (uint32_t i = 0; i < ctx->data_length; ++i) {
            if (temp_mtf_indices[i] < 0x10) { // If index is 0-15 (4 bits)
                bit_write(bit_stream_state, 0x80, 1); // Prefix with '1' bit
                bit_write(bit_stream_state, (temp_mtf_indices[i] & 0xF), 4); // Write 4 bits
            } else { // If index is 16-255 (8 bits)
                bit_write(bit_stream_state, 0, 1); // Prefix with '0' bit
                bit_write(bit_stream_state, temp_mtf_indices[i], 8); // Write 8 bits
            }
        }
        
        // Calculate the actual size of the compressed data including the length prefix
        // The byte_offset is the number of full bytes written.
        // If bit_offset is not 0, there's a partial byte, so add 1.
        *output_len_ptr = sizeof(uint32_t) + bit_stream_state->byte_offset + (bit_stream_state->bit_offset != 0);

        free(bit_stream_state);
        free(temp_mtf_indices);

        return mtf_output_data;

    } else { // MTF Decompression
        uint32_t data_len = ((uint32_t *)ctx->data_buffer)[0]; // Read original data length from input

        if (data_len >= 0x1001) { // Apply original magic number limit for decompression
            return NULL;
        }

        uint8_t mtf_output_indices[2048]; // Temporary buffer for MTF indices (max 2048)
        if (data_len > sizeof(mtf_output_indices)) {
             // If data_len exceeds static buffer, this path is not handled by original code.
             return NULL;
        }

        mtf_output_data = (uint8_t *)malloc(data_len);
        if (mtf_output_data == NULL) {
            return NULL;
        }
        
        // Initialize bit stream reader, starting after the length prefix
        bit_stream_state = bit_new((uint8_t *)ctx->data_buffer + sizeof(uint32_t));
        if (bit_stream_state == NULL) {
            free(mtf_output_data);
            return NULL;
        }

        // Read MTF encoded indices from bitstream
        for (uint32_t i = 0; i < data_len; ++i) {
            uint8_t is_short_code = bit_read(bit_stream_state, 1);
            if (is_short_code == 0) { // 8-bit index
                mtf_output_indices[i] = bit_read(bit_stream_state, 8);
            } else { // 4-bit index
                mtf_output_indices[i] = bit_read(bit_stream_state, 4);
            }
        }

        // MTF decode
        for (uint32_t i = 0; i < data_len; ++i) {
            uint8_t index = mtf_output_indices[i];
            uint8_t decoded_char = char_list[index];
            ((uint8_t *)mtf_output_data)[i] = decoded_char;

            // Move the decoded character to the front of the list
            if (index > 0) {
                memmove(char_list + 1, char_list, index);
                char_list[0] = decoded_char;
            }
        }

        *output_len_ptr = data_len; // Set the decompressed data length

        free(bit_stream_state);

        return mtf_output_data;
    }
}