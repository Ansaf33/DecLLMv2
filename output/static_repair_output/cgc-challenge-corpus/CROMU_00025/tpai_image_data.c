#include <stdbool.h> // For bool, true, false
#include <stdint.h>  // For uint32_t, intptr_t
#include <stdio.h>   // For printf, fprintf, stderr
#include <stdlib.h>  // For NULL
#include <string.h>  // For memset
#include <stddef.h>  // For size_t

// Define the structure for TPAI_Context based on memory accesses
// This structure is inferred from the offset-based accesses in the original code.
typedef struct {
    char *data_buffer_ptr;      // Offset 0x00: Base address of the data buffer (e.g., file content)
    uint32_t total_data_len;    // Offset 0x04: Total length of the data buffer in bytes
    uint32_t current_byte_offset; // Offset 0x08: Current byte offset in data_buffer
    uint32_t bit_offset_or_checksum_flag; // Offset 0x0C: Used as current bit offset (0-7) in bit reading functions,
                                          // and as a flag (0 for enabled) in checksum calculation.

    uint32_t width;             // Offset 0x10: Image width
    uint32_t height;            // Offset 0x14: Image height
    uint32_t load_direction;    // Offset 0x18: Pixel loading direction (0-7)
    // ... potentially more fields if the context is larger
} TPAI_Context;

// Magic number for TPAI format, converted from signed -0x34a6f0cf to unsigned.
#define TPAI_MAGIC 0xCB590F31U

// Character map for pixel display.
// The original code used `local_80 < 0x60` (96) as a bound.
// The inferred string " !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"
// has 95 characters (ASCII 32 to 126).
// To allow access up to index 95 (0x5F), we pad the array to 96 characters.
static const char pixel_char_map[96] =
    " !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~?";

// External function declarations (assuming these are provided elsewhere)
// These are placeholders for custom memory allocation/deallocation.
extern bool allocate(size_t size, int flags, void **buffer);
extern void deallocate(void *buffer, size_t size);


// Function: tpai_read_check
// Checks if there are enough bits remaining in the buffer to read.
bool tpai_read_check(TPAI_Context *ctx, int num_bits_to_read) {
    if (ctx == NULL || ctx->data_buffer_ptr == NULL) {
        return false;
    }
    // num_bits_to_read must be positive and not exceed 32 bits (0x20)
    // The original check `param_2 < 0x21` means `param_2 <= 0x20` (32).
    if (num_bits_to_read <= 0 || num_bits_to_read > 32) {
        return false;
    }

    // Calculate total bits available from current position
    // available_bits = (total_bytes - current_byte_offset) * 8 - current_bit_offset
    uint32_t available_bits =
        (ctx->total_data_len - ctx->current_byte_offset) * 8 - ctx->bit_offset_or_checksum_flag;

    return available_bits >= (uint32_t)num_bits_to_read;
}

// Function: tpai_read_nbits
// Reads a specified number of bits from the context's data buffer.
bool tpai_read_nbits(TPAI_Context *ctx, int num_bits_to_read, unsigned int *output_value_ptr) {
    if (output_value_ptr == NULL) {
        return false;
    }

    if (!tpai_read_check(ctx, num_bits_to_read)) {
        return false;
    }

    unsigned int read_value = 0;
    char *current_data_byte_ptr = ctx->data_buffer_ptr + ctx->current_byte_offset;

    for (int i = 0; i < num_bits_to_read; i++) {
        // Read the current bit from the current byte
        unsigned char current_byte = *(unsigned char *)current_data_byte_ptr;
        unsigned char bit = (current_byte >> (7U - ctx->bit_offset_or_checksum_flag)) & 1;

        read_value = (read_value << 1) | bit; // Shift left and OR the new bit

        ctx->bit_offset_or_checksum_flag = (ctx->bit_offset_or_checksum_flag + 1) % 8; // Increment bit offset, wrap around
        if (ctx->bit_offset_or_checksum_flag == 0) { // If bit offset wraps to 0, advance to next byte
            ctx->current_byte_offset++;
            current_data_byte_ptr++; // Update pointer to the next byte
        }
    }

    *output_value_ptr = read_value;
    return true;
}

// Function: tpai_read_magic
// Reads and validates the magic number from the context.
bool tpai_read_magic(TPAI_Context *ctx) {
    if (ctx == NULL) {
        return false;
    }

    unsigned int magic_value = 0;
    if (!tpai_read_nbits(ctx, 32, &magic_value)) { // Read 32 bits for magic number
        return false;
    }
    return magic_value == TPAI_MAGIC;
}

// Function: tpai_read_width
// Reads the image width from the context.
bool tpai_read_width(TPAI_Context *ctx) {
    if (ctx == NULL) {
        return false;
    }

    unsigned int width_val = 0;
    if (!tpai_read_nbits(ctx, 6, &width_val)) { // Read 6 bits for width
        ctx->width = 0; // Set to 0 on failure
        return false;
    }
    ctx->width = width_val;
    return true;
}

// Function: tpai_read_height
// Reads the image height from the context.
bool tpai_read_height(TPAI_Context *ctx) {
    if (ctx == NULL) {
        return false;
    }

    unsigned int height_val = 0;
    if (!tpai_read_nbits(ctx, 6, &height_val)) { // Read 6 bits for height
        ctx->height = 0; // Set to 0 on failure
        return false;
    }
    ctx->height = height_val;
    return true;
}

// Function: tpai_read_loadd
// Reads the pixel load direction and stores it in the context.
bool tpai_read_loadd(TPAI_Context *ctx, unsigned int *load_direction_out) {
    if (ctx == NULL || load_direction_out == NULL) {
        return false;
    }

    unsigned int read_value = 0;
    if (!tpai_read_nbits(ctx, 3, &read_value)) { // Read 3 bits for load direction
        return false;
    }

    *load_direction_out = read_value;
    ctx->load_direction = read_value; // Update context directly
    return true;
}

// Function: tpai_skip_rsrvd
// Skips the reserved section, expecting it to be zero.
bool tpai_skip_rsrvd(TPAI_Context *ctx) {
    if (ctx == NULL) {
        return false;
    }

    unsigned int reserved_value = 0;
    if (!tpai_read_nbits(ctx, 0x11, &reserved_value)) { // Read 17 bits for reserved value
        return false;
    }
    // Assuming the reserved value must be 0 for success
    return reserved_value == 0;
}

// Function: tpai_calc_checksum
// Calculates and validates the checksum of a data section.
bool tpai_calc_checksum(TPAI_Context *ctx) {
    if (ctx == NULL) {
        return false;
    }
    // Assuming bit_offset_or_checksum_flag is 0 when checksum is enabled
    if (ctx->bit_offset_or_checksum_flag != 0) {
        return false; // Checksum is disabled or invalid flag
    }

    // Check if the data section length for checksum is even
    if (((ctx->total_data_len - ctx->current_byte_offset) & 1U) != 0) {
        return false; // Data length must be even for 2-byte checksums
    }

    // Number of 2-byte words to checksum, excluding the last word (which is the stored checksum)
    int num_words_to_checksum = (int)((ctx->total_data_len - ctx->current_byte_offset) / 2) - 1;

    if (num_words_to_checksum < 0) { // Not enough data for at least one word + checksum word
        return false;
    }

    unsigned short calculated_checksum = 0;
    char *data_base = ctx->data_buffer_ptr + ctx->current_byte_offset;

    for (int i = 0; i < num_words_to_checksum; i++) {
        calculated_checksum ^= *(unsigned short *)(data_base + (size_t)i * 2);
    }

    // The last 2-byte word is the stored checksum
    unsigned short stored_checksum = *(unsigned short *)(data_base + (size_t)num_words_to_checksum * 2);

    return calculated_checksum == stored_checksum;
}

// Function: tpai_read_pixel
// Reads a single pixel value and converts it to its corresponding character.
bool tpai_read_pixel(TPAI_Context *ctx, int *pixel_char_val_out) {
    if (ctx == NULL || pixel_char_val_out == NULL) {
        return false;
    }

    unsigned int pixel_index_val = 0; // The value read from the stream, determines char from map
    if (!tpai_read_nbits(ctx, 7, &pixel_index_val)) { // Read 7 bits for pixel value index
        return false;
    }

    // Check if pixel_index_val is within valid bounds for the character map
    // `pixel_char_map` has 96 characters (indices 0-95).
    if (pixel_index_val < sizeof(pixel_char_map)) {
        *pixel_char_val_out = (int)pixel_char_map[pixel_index_val];
        return true;
    }
    return false; // Pixel value out of bounds for character map
}

// Function: tpai_display_image
// Reads image data from the context and prints it to stdout.
bool tpai_display_image(TPAI_Context *ctx) {
    void *image_buffer = NULL;
    size_t image_size = 0;
    unsigned int pixel_char_val = 0;
    unsigned int current_pixel_index = 0;
    unsigned int row = 0;
    unsigned int col = 0;
    unsigned int buffer_offset = 0;

    if (ctx == NULL) {
        return false;
    }

    // Chain function calls using logical AND for early exit on failure
    if (!tpai_read_magic(ctx) ||
        !tpai_read_width(ctx) ||
        !tpai_read_height(ctx) ||
        !tpai_read_loadd(ctx, &ctx->load_direction) || // tpai_read_loadd updates ctx->load_direction
        !tpai_skip_rsrvd(ctx) ||
        !tpai_calc_checksum(ctx)) {
        return false;
    }

    // Calculate image_size based on width and height from ctx
    image_size = (size_t)ctx->width * ctx->height;

    // Allocate memory for the image buffer (+1 for null terminator)
    if (!allocate(image_size + 1, 0, &image_buffer)) {
        return false;
    }

    memset(image_buffer, ' ', image_size); // Initialize buffer with spaces
    ((char *)image_buffer)[image_size] = '\0'; // Null-terminate the buffer

    // Read and place pixels into the buffer
    while (true) {
        if (!tpai_read_pixel(ctx, (int *)&pixel_char_val)) {
            break; // Stop if pixel reading fails (e.g., end of stream, invalid pixel)
        }

        // Determine row and col based on load_direction
        switch (ctx->load_direction) {
            case 0: // Top-Left to Bottom-Right
                row = current_pixel_index / ctx->width;
                col = current_pixel_index % ctx->width;
                break;
            case 1: // Top-Right to Bottom-Left
                row = current_pixel_index / ctx->width;
                col = (ctx->width - (current_pixel_index % ctx->width)) - 1;
                break;
            case 2: // Bottom-Left to Top-Right
                row = (ctx->height - (current_pixel_index / ctx->width)) - 1;
                col = current_pixel_index % ctx->width;
                break;
            case 3: // Bottom-Right to Top-Left
                row = (ctx->height - (current_pixel_index / ctx->width)) - 1;
                col = (ctx->width - (current_pixel_index % ctx->width)) - 1;
                break;
            case 4: // Top-Left to Bottom-Right (Transposed: width and height swapped for coordinates)
                row = current_pixel_index % ctx->height; // Transposed row based on height
                col = current_pixel_index / ctx->height; // Transposed col based on height
                break;
            case 5: // Top-Right to Bottom-Left (Transposed)
                row = current_pixel_index % ctx->height;
                col = (ctx->width - (current_pixel_index / ctx->height)) - 1;
                break;
            case 6: // Bottom-Left to Top-Right (Transposed)
                row = (ctx->height - (current_pixel_index % ctx->height)) - 1;
                col = current_pixel_index / ctx->height;
                break;
            case 7: // Bottom-Right to Top-Left (Transposed)
                row = (ctx->height - (current_pixel_index % ctx->height)) - 1;
                col = (ctx->width - (current_pixel_index / ctx->height)) - 1;
                break;
            default:
                fprintf(stderr, "[ERROR] Unknown load direction: %u\n", ctx->load_direction);
                deallocate(image_buffer, image_size + 1);
                return false;
        }

        // Calculate the linear offset in the 1D image buffer
        buffer_offset = col + ctx->width * row;
        if (buffer_offset >= image_size) {
            fprintf(stderr, "[ERROR] Pixel out of bounds in image buffer: offset %u, size %zu\n", buffer_offset, image_size);
            deallocate(image_buffer, image_size + 1);
            return false;
        }
        ((char *)image_buffer)[buffer_offset] = (char)pixel_char_val;
        current_pixel_index++;
    }

    // Print the image to stdout
    for (buffer_offset = 0; buffer_offset < image_size; buffer_offset++) {
        if ((buffer_offset % ctx->width == 0) && (buffer_offset != 0)) {
            printf("\n"); // Newline at the start of each row (except the very first pixel)
        }
        printf("%c", ((char *)image_buffer)[buffer_offset]);
    }
    printf("\n"); // Final newline after the image

    deallocate(image_buffer, image_size + 1);
    return true;
}