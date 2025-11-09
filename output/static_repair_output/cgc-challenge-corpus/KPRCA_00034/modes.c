#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h> // For uint8_t, uint32_t, uint64_t, size_t

// Forward declarations for external functions and structures
// Assuming a maximum block size of 16 bytes based on context buffers.
// The self-test implies a block size of 12 bytes for its specific test case.
#define BLOCK_SIZE_MAX 16

// Structure for the cipher algorithm context (e.g., AES context)
typedef struct {
    uint32_t block_bits; // e.g., 128 for 16 bytes, so 128 >> 3 = 16
    // Add other fields for the cipher algorithm context if known
} codes_ctx_t;

// Structure for the overall cipher mode context
// This structure is designed to match the memory layout implied by
// how the original code accesses `param_1` in `modes_init` and `_do_encode/_do_decode`.
typedef struct {
    codes_ctx_t *codes_context; // This is `*param_1` in original code
    uint32_t mode;              // This is `param_1[1]` in original code (offset by sizeof(void*))
    uint32_t iv_data[BLOCK_SIZE_MAX / sizeof(uint32_t)]; // This is `param_1[2]` onwards, 16 bytes = 4 uint32_t
} cipher_mode_ctx_t;

// Function prototypes (guessed signatures for external functions)
// These are dummy implementations to make the code compilable.
// Replace with actual cipher implementations if available.
int codes_encode(codes_ctx_t *ctx, uint8_t *data);
int codes_decode(codes_ctx_t *ctx, uint8_t *data);
int codes_init(codes_ctx_t *ctx, int p2, int p3); // p2, p3 are likely algorithm parameters

// test_0 definition for modes_self_test
static const uint8_t test_0[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09
}; // 10 bytes

// Helper for fdprintf (replaces decompiler's fdprintf)
#define fdprintf(fd, fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)

// Function: _do_encode
uint32_t _do_encode(cipher_mode_ctx_t *ctx, uint8_t *data_block) {
    size_t block_size = ctx->codes_context->block_bits >> 3;
    if (block_size == 0 || block_size > BLOCK_SIZE_MAX) {
        // Handle invalid block size, or use a default/return error
        return 1; // Indicate error
    }

    if (ctx->mode == 1) { // Mode 1 (e.g., CTR mode)
        uint8_t temp_keystream[BLOCK_SIZE_MAX + 1]; // +1 for 1-indexing in original code
        uint64_t counter_val = ((uint64_t)ctx->iv_data[1] << 32) | ctx->iv_data[0];

        // Generate keystream block from counter
        for (size_t i = block_size; i != 0; --i) {
            temp_keystream[i] = (uint8_t)counter_val; // Extract lowest byte
            counter_val >>= 8;                         // Shift right by 8 bits
        }

        if (codes_encode(ctx->codes_context, temp_keystream + 1) != 0) {
            return 1;
        }

        // XOR data with keystream
        for (size_t i = 0; i < block_size; ++i) {
            data_block[i] ^= temp_keystream[i + 1];
        }

        // Decrement 64-bit counter (ctx->iv_data[1]:ctx->iv_data[0])
        if (ctx->iv_data[0] == 0) {
            ctx->iv_data[1]--;
        }
        ctx->iv_data[0]--;

    } else if (ctx->mode == 2) { // Mode 2 (e.g., CBC mode without IV, or similar feedback)
        // XOR data with current IV (stored in ctx->iv_data)
        for (size_t i = 0; i < block_size; ++i) {
            data_block[i] ^= ((uint8_t *)ctx->iv_data)[i];
        }
        if (codes_encode(ctx->codes_context, data_block) != 0) {
            return 1;
        }
        // Update IV with encrypted block (ciphertext stealing/feedback)
        for (size_t i = 0; i < block_size; ++i) {
            ((uint8_t *)ctx->iv_data)[i] = data_block[i]; // Store encrypted block as next IV
        }
    } else if (ctx->mode == 3) { // Mode 3 (e.g., CFB mode)
        uint8_t original_data_block[BLOCK_SIZE_MAX];
        memcpy(original_data_block, data_block, block_size); // Save original data block

        // XOR data with current IV (stored in ctx->iv_data)
        for (size_t i = 0; i < block_size; ++i) {
            data_block[i] ^= ((uint8_t *)ctx->iv_data)[i];
        }
        if (codes_encode(ctx->codes_context, data_block) != 0) {
            return 1;
        }
        // Update IV with original data block (input to cipher)
        memcpy(ctx->iv_data, original_data_block, block_size);
        // XOR updated IV with encrypted block
        for (size_t i = 0; i < block_size; ++i) {
            ((uint8_t *)ctx->iv_data)[i] ^= data_block[i];
        }
    }
    return 0;
}

// Function: _do_decode
uint32_t _do_decode(cipher_mode_ctx_t *ctx, uint8_t *data_block) {
    size_t block_size = ctx->codes_context->block_bits >> 3;
    if (block_size == 0 || block_size > BLOCK_SIZE_MAX) {
        return 1; // Indicate error
    }

    if (ctx->mode == 1) { // Mode 1 (e.g., CTR mode)
        // For CTR, encode and decode are the same operation
        return _do_encode(ctx, data_block);
    } else if (ctx->mode == 2) { // Mode 2 (e.g., CBC mode)
        uint8_t ciphertext_block_copy[BLOCK_SIZE_MAX];
        memcpy(ciphertext_block_copy, data_block, block_size); // Save ciphertext block

        if (codes_decode(ctx->codes_context, data_block) != 0) {
            return 1;
        }

        // XOR decrypted data with current IV
        for (size_t i = 0; i < block_size; ++i) {
            data_block[i] ^= ((uint8_t *)ctx->iv_data)[i];
        }
        // Update IV with saved ciphertext block
        memcpy(ctx->iv_data, ciphertext_block_copy, block_size);
    } else if (ctx->mode == 3) { // Mode 3 (e.g., CFB mode)
        uint8_t ciphertext_block_copy[BLOCK_SIZE_MAX];
        memcpy(ciphertext_block_copy, data_block, block_size); // Save ciphertext block

        if (codes_decode(ctx->codes_context, data_block) != 0) {
            return 1;
        }

        // XOR decrypted data with current IV
        for (size_t i = 0; i < block_size; ++i) {
            data_block[i] ^= ((uint8_t *)ctx->iv_data)[i];
        }
        // Update IV with saved ciphertext block (input to cipher)
        memcpy(ctx->iv_data, ciphertext_block_copy, block_size);
        // XOR updated IV with decrypted data block
        for (size_t i = 0; i < block_size; ++i) {
            ((uint8_t *)ctx->iv_data)[i] ^= data_block[i];
        }
    }
    return 0;
}

// Function: modes_init
bool modes_init(cipher_mode_ctx_t *ctx, uint32_t mode_val, codes_ctx_t *codes_context) {
    if (mode_val < 4) { // Assuming modes 0, 1, 2, 3 are valid
        ctx->mode = mode_val;
        ctx->codes_context = codes_context;
        memset(ctx->iv_data, 0, BLOCK_SIZE_MAX); // Clear IV data
        return true;
    }
    return false;
}

// Function: modes_encode
uint32_t modes_encode(cipher_mode_ctx_t *ctx, const uint8_t *input_data, size_t input_len, uint8_t **output_ptr, size_t *output_len_ptr) {
    if (input_len == 0) {
        *output_len_ptr = 0;
        *output_ptr = NULL;
        return 0;
    }

    size_t block_size = ctx->codes_context->block_bits >> 3;
    if (block_size == 0 || block_size > BLOCK_SIZE_MAX) {
        return 1; // Indicate error for invalid block size
    }

    // Calculate `padded_data_len`: input_len rounded up to the nearest multiple of block_size.
    // This is the length of data blocks that will contain actual data and/or padding.
    size_t padded_data_len = ((input_len + block_size - 1) / block_size) * block_size;

    // Allocate memory for padded data + potential extra padding block.
    // PKCS#7 padding scheme: if data is already block-aligned, an entire extra block
    // of padding is added. So, allocate for `padded_data_len` plus an additional `block_size`.
    uint8_t *output_buffer = (uint8_t *)malloc(padded_data_len + block_size);
    if (output_buffer == NULL) {
        return 1; // Memory allocation failed
    }

    size_t current_input_offset = 0;
    bool padding_applied_in_loop = false;
    size_t final_output_len = 0;

    // Process blocks containing actual data and/or padding
    for (size_t current_output_offset = 0; current_output_offset < padded_data_len; current_output_offset += block_size) {
        size_t bytes_to_copy = block_size;
        if (input_len < block_size) {
            bytes_to_copy = input_len; // Last block with partial data
        }

        memcpy(output_buffer + current_output_offset, input_data + current_input_offset, bytes_to_copy);
        current_input_offset += bytes_to_copy; // Advance input offset
        input_len -= bytes_to_copy;             // Decrement remaining input length

        if (bytes_to_copy < block_size) {
            padding_applied_in_loop = true;
            size_t padding_bytes = block_size - bytes_to_copy;
            // PKCS#7 padding: fill with value equal to padding_bytes
            memset(output_buffer + current_output_offset + bytes_to_copy, (uint8_t)padding_bytes, padding_bytes);
        }

        if (_do_encode(ctx, output_buffer + current_output_offset) != 0) {
            free(output_buffer);
            return 1; // Encoding failed
        }
        final_output_len += block_size; // Each processed block adds to the final length
    }

    // If no padding was applied in the loop (i.e., input_len was originally a multiple of block_size)
    // then an extra block of padding is added.
    if (!padding_applied_in_loop) {
        // This block is written to `output_buffer + padded_data_len`.
        // The value of padding bytes is `block_size`.
        memset(output_buffer + padded_data_len, (uint8_t)block_size, block_size);
        if (_do_encode(ctx, output_buffer + padded_data_len) != 0) {
            free(output_buffer);
            return 1; // Encoding failed for extra padding block
        }
        final_output_len += block_size; // Add the extra block to final length
    }

    *output_ptr = output_buffer;
    *output_len_ptr = final_output_len;
    return 0;
}

// Function: modes_decode
uint32_t modes_decode(cipher_mode_ctx_t *ctx, const uint8_t *input_data, size_t input_len, uint8_t **output_ptr, size_t *output_len_ptr) {
    if (input_len == 0) {
        *output_len_ptr = 0;
        *output_ptr = NULL;
        return 0;
    }

    size_t block_size = ctx->codes_context->block_bits >> 3;
    if (block_size == 0 || block_size > BLOCK_SIZE_MAX) {
        return 1; // Indicate error for invalid block size
    }

    // Input length must be a multiple of block size for block cipher modes
    if (input_len % block_size != 0) {
        return 1; // Invalid input length
    }

    uint8_t *output_buffer = (uint8_t *)malloc(input_len);
    if (output_buffer == NULL) {
        return 1; // Memory allocation failed
    }
    memcpy(output_buffer, input_data, input_len); // Copy input to mutable buffer for decryption

    // Decrypt all blocks
    for (size_t current_offset = 0; current_offset < input_len; current_offset += block_size) {
        if (_do_decode(ctx, output_buffer + current_offset) != 0) {
            free(output_buffer);
            return 1; // Decryption failed
        }
    }

    // PKCS#7 padding check and removal
    uint8_t padding_val = output_buffer[input_len - 1]; // Last byte is padding value

    // Check for valid padding value: must be > 0 and <= block_size
    if (padding_val == 0 || padding_val > block_size) {
        free(output_buffer);
        return 1; // Invalid padding value
    }

    // Check if all padding bytes have the same value
    for (size_t i = 1; i <= padding_val; ++i) {
        if (output_buffer[input_len - i] != padding_val) {
            free(output_buffer);
            return 1; // Mismatched padding bytes
        }
    }

    *output_ptr = output_buffer;
    *output_len_ptr = input_len - padding_val; // Actual data length after removing padding
    return 0;
}

// Function: modes_self_test
uint32_t modes_self_test(void) {
    codes_ctx_t codes_context_obj;      // Corresponds to local_18 in original decompiler output
    cipher_mode_ctx_t mode_context_obj; // Corresponds to local_30 in original decompiler output
    uint8_t *encoded_data = NULL;
    uint8_t *decoded_data = NULL;
    size_t encoded_len = 0;
    size_t decoded_len = 0;

    // Initialize the codes context. The self-test expects a block size that results in 12 bytes
    // for 10 bytes of input (i.e., a 12-byte block size).
    codes_context_obj.block_bits = 12 * 8; // Set block size to 12 bytes (96 bits) for the test

    if (codes_init(&codes_context_obj, 0, 0) != 0) {
        fdprintf(2, "Codes initialization FAILED!\n");
        return 1;
    }

    bool test_failed = false;
    for (uint32_t mode_id = 0; mode_id < 4; ++mode_id) {
        // --- Test Encoding ---
        // Initialize mode context for encoding
        if (!modes_init(&mode_context_obj, mode_id, &codes_context_obj)) {
            fdprintf(2, "Modes initialization FAILED for mode %u (before encode)!\n", mode_id);
            test_failed = true;
            break;
        }
        if (modes_encode(&mode_context_obj, test_0, sizeof(test_0), &encoded_data, &encoded_len) != 0) {
            fdprintf(2, "Modes encode FAILED for mode %u!\n", mode_id);
            test_failed = true;
            break;
        }

        // Check encoded length: 10 bytes input with 12-byte block size.
        // Needs 2 bytes of padding (value 2). Total 12 bytes.
        if (encoded_len != 12) {
            fdprintf(2, "Modes encode output length mismatch for mode %u! Expected 12, got %zu\n", mode_id, encoded_len);
            test_failed = true;
            break;
        }

        // --- Test Decoding ---
        // Re-initialize mode context for decoding, as _do_encode modifies IV
        if (!modes_init(&mode_context_obj, mode_id, &codes_context_obj)) {
            fdprintf(2, "Modes initialization FAILED for mode %u (before decode)!\n", mode_id);
            test_failed = true;
            break;
        }
        if (modes_decode(&mode_context_obj, encoded_data, encoded_len, &decoded_data, &decoded_len) != 0) {
            fdprintf(2, "Modes decode FAILED for mode %u!\n", mode_id);
            test_failed = true;
            break;
        }

        free(encoded_data); // Free encoded data buffer
        encoded_data = NULL; // Reset pointer to prevent double free or use-after-free

        // Check decoded length: should be original input length (10 bytes)
        if (decoded_len != sizeof(test_0)) {
            fdprintf(2, "Modes decode output length mismatch for mode %u! Expected %zu, got %zu\n", mode_id, sizeof(test_0), decoded_len);
            test_failed = true;
            break;
        }
        // Check decoded data content
        if (memcmp(decoded_data, test_0, sizeof(test_0)) != 0) {
            fdprintf(2, "Modes decode data mismatch for mode %u!\n", mode_id);
            test_failed = true;
            break;
        }

        free(decoded_data); // Free decoded data buffer
        decoded_data = NULL; // Reset pointer
    }

    // Cleanup any remaining allocated memory if a test failed prematurely
    if (encoded_data) {
        free(encoded_data);
    }
    if (decoded_data) {
        free(decoded_data);
    }

    if (test_failed) {
        fdprintf(2, "Modes self-test FAILED!\n");
        return 1;
    }
    return 0; // All tests passed
}


// --- Dummy Implementations for External Functions ---
// Replace these with actual cipher library calls if available.

// Dummy implementation for codes_encode: simple XOR with a fixed key
int codes_encode(codes_ctx_t *ctx, uint8_t *data) {
    uint8_t dummy_key = 0x5A; // Arbitrary key for dummy encryption
    size_t block_size = ctx->block_bits >> 3;
    for (size_t i = 0; i < block_size; ++i) {
        data[i] ^= dummy_key;
    }
    return 0; // Success
}

// Dummy implementation for codes_decode: simple XOR with the same fixed key
int codes_decode(codes_ctx_t *ctx, uint8_t *data) {
    uint8_t dummy_key = 0x5A; // Same key for dummy decryption
    size_t block_size = ctx->block_bits >> 3;
    for (size_t i = 0; i < block_size; ++i) {
        data[i] ^= dummy_key;
    }
    return 0; // Success
}

// Dummy implementation for codes_init: just sets a default block size
int codes_init(codes_ctx_t *ctx, int p2, int p3) {
    if (ctx) {
        // Default to BLOCK_SIZE_MAX if not explicitly set by the caller (like self-test)
        ctx->block_bits = BLOCK_SIZE_MAX * 8;
    }
    return 0; // Success
}

// Main function to run the self-test
int main() {
    return modes_self_test();
}