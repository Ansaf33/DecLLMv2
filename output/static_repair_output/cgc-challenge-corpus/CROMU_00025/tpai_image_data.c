#include <stdio.h>    // For printf
#include <stdlib.h>   // For malloc, free
#include <string.h>   // For memset
#include <stdint.h>   // For uint16_t, uint32_t, etc.
#include <stdbool.h>  // For bool type

// Define the TpaiImageContext structure based on memory access patterns
// Offsets inferred from the original code:
// param_1 + 0x00: file_data (char*)
// param_1 + 0x04: size (int)
// param_1 + 0x08: offset (int)
// param_1 + 0x0C: bit_offset (int)
// param_1 + 0x10: width (int)
// param_1 + 0x14: height (int)
// param_1 + 0x18: load_mode (int)
typedef struct {
    char *file_data;
    int size;
    int offset;
    int bit_offset;
    int width;
    int height;
    int load_mode;
} TpaiImageContext;

// Forward declarations for functions
bool tpai_read_check(TpaiImageContext *ctx, int num_bits);
bool tpai_read_nbits(TpaiImageContext *ctx, int num_bits, unsigned int *value);
bool tpai_read_magic(TpaiImageContext *ctx);
bool tpai_read_width(TpaiImageContext *ctx);
bool tpai_read_height(TpaiImageContext *ctx);
bool tpai_read_loadd(TpaiImageContext *ctx, unsigned int *load_mode_value);
bool tpai_skip_rsrvd(TpaiImageContext *ctx);
bool tpai_calc_checksum(TpaiImageContext *ctx);
bool tpai_read_pixel(TpaiImageContext *ctx, int *pixel_value);
bool tpai_display_image(TpaiImageContext *ctx);

// Custom memory allocation/deallocation functions
// Assuming 'allocate' returns non-zero on success (and sets *out_ptr), 0 on failure.
// Assuming 'deallocate' is void.
static int allocate(size_t size, int flags, void **out_ptr) {
    (void)flags; // flags parameter is unused in this simple wrapper
    *out_ptr = malloc(size);
    return (*out_ptr != NULL);
}

static void deallocate(void *ptr, size_t size) {
    (void)size; // size parameter might be for debugging/tracking, not strictly needed for free
    free(ptr);
}

// Function: tpai_read_check
bool tpai_read_check(TpaiImageContext *ctx, int num_bits) {
    if (!ctx || !ctx->file_data) {
        return false;
    }
    if (num_bits <= 0 || num_bits > 32) { // Max 32 bits for unsigned int in this context
        return false;
    }
    // Check if there are enough bits left in the buffer
    // ctx->size is total bytes, ctx->offset is current byte offset, ctx->bit_offset is current bit in byte
    // Total bits available from current position: (ctx->size - ctx->offset) * 8 - ctx->bit_offset
    if ((size_t)ctx->offset * 8 + ctx->bit_offset + num_bits > (size_t)ctx->size * 8) {
        return false;
    }
    return true;
}

// Function: tpai_read_nbits
bool tpai_read_nbits(TpaiImageContext *ctx, int num_bits, unsigned int *value) {
    if (!tpai_read_check(ctx, num_bits) || !value) {
        return false;
    }

    *value = 0; // Initialize result
    for (int i = 0; i < num_bits; ++i) {
        // Read the current bit
        // (7 - ctx->bit_offset) gives the position of the bit from MSB (0-7)
        unsigned char current_byte = (unsigned char)ctx->file_data[ctx->offset];
        unsigned int bit = (current_byte >> (7 - ctx->bit_offset)) & 1;

        // Accumulate the bit (shift existing value left, then add new bit)
        *value = (*value << 1) | bit;

        // Advance bit_offset
        ctx->bit_offset++;
        if (ctx->bit_offset == 8) {
            ctx->bit_offset = 0;
            ctx->offset++;
        }
    }
    return true;
}

// Function: tpai_read_magic
bool tpai_read_magic(TpaiImageContext *ctx) {
    if (!ctx) {
        return false;
    }
    unsigned int magic_value = 0;
    if (!tpai_read_nbits(ctx, 32, &magic_value)) { // 0x20 is 32 bits
        return false;
    }
    // The magic value is -0x34a6f0cf, which is 0xCB590F31 as an unsigned int
    return (magic_value == 0xCB590F31);
}

// Function: tpai_read_width
bool tpai_read_width(TpaiImageContext *ctx) {
    if (!ctx) {
        return false;
    }
    unsigned int width_value = 0;
    if (!tpai_read_nbits(ctx, 6, &width_value)) {
        ctx->width = 0; // Ensure width is zero on failure
        return false;
    }
    ctx->width = (int)width_value;
    return true;
}

// Function: tpai_read_height
bool tpai_read_height(TpaiImageContext *ctx) {
    if (!ctx) {
        return false;
    }
    unsigned int height_value = 0;
    if (!tpai_read_nbits(ctx, 6, &height_value)) {
        ctx->height = 0; // Ensure height is zero on failure
        return false;
    }
    ctx->height = (int)height_value;
    return true;
}

// Function: tpai_read_loadd
bool tpai_read_loadd(TpaiImageContext *ctx, unsigned int *load_mode_value) {
    if (!ctx || !load_mode_value) {
        return false;
    }
    unsigned int mode = 0;
    if (!tpai_read_nbits(ctx, 3, &mode)) {
        return false;
    }
    *load_mode_value = mode;
    ctx->load_mode = (int)mode;
    return true;
}

// Function: tpai_skip_rsrvd
bool tpai_skip_rsrvd(TpaiImageContext *ctx) {
    if (!ctx) {
        return false;
    }
    unsigned int reserved_value = 0;
    if (!tpai_read_nbits(ctx, 17, &reserved_value)) { // 0x11 is 17 bits
        return false;
    }
    // The original code checks if reserved_value is 0.
    return (reserved_value == 0);
}

// Function: tpai_calc_checksum
bool tpai_calc_checksum(TpaiImageContext *ctx) {
    if (!ctx) {
        return false;
    }

    // Checksum is calculated only if bit_offset is 0 (byte aligned)
    // and the remaining data length is an even number of bytes (for ushort access).
    // The last ushort is the expected checksum.
    if (ctx->bit_offset != 0) {
        return false; // Not byte-aligned
    }

    int remaining_bytes = ctx->size - ctx->offset;
    if (remaining_bytes < 2 || (remaining_bytes % 2) != 0) {
        return false; // Not enough bytes for at least one 16-bit word, or odd number of bytes
    }

    int num_checksum_words = remaining_bytes / 2; // Total 16-bit words including the checksum itself

    uint16_t current_checksum = 0;
    char *data_ptr = ctx->file_data + ctx->offset;

    // Iterate over all words except the last one (which is the stored checksum)
    for (int i = 0; i < num_checksum_words - 1; ++i) {
        // Read 16-bit word. Assuming direct memory access (endianness dependent).
        // To be strictly portable, one would read two bytes and combine them.
        // For now, mirroring the decompiler's `*(ushort *)`
        uint16_t word = *(uint16_t *)(data_ptr + i * 2);
        current_checksum ^= word;
    }

    // The last word is the expected checksum
    uint16_t expected_checksum = *(uint16_t *)(data_ptr + (num_checksum_words - 1) * 2);

    ctx->offset += remaining_bytes; // Advance offset past checksum data
    ctx->bit_offset = 0;            // Ensure bit_offset is reset

    return (current_checksum == expected_checksum);
}

// Pixel lookup table
// Original string: " !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"
// This string has 95 characters. Valid indices are 0 to 94.
static const char TPAI_PIXEL_MAP[] =
    " !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
#define TPAI_PIXEL_MAP_LENGTH (sizeof(TPAI_PIXEL_MAP) - 1) // Exclude null terminator

// Function: tpai_read_pixel
bool tpai_read_pixel(TpaiImageContext *ctx, int *pixel_value) {
    if (!ctx || !pixel_value) {
        return false;
    }

    unsigned int pixel_index = 0;
    // Reads 7 bits for the pixel index
    if (!tpai_read_nbits(ctx, 7, &pixel_index)) {
        return false;
    }

    // The original code had a check `(local_80 < 0x60) && (local_80 != 0)`.
    // `0x60` (96) is the upper bound. `TPAI_PIXEL_MAP_LENGTH` is 95.
    // If pixel_index is 0, it maps to ' ' (space). This is a valid character.
    // The `local_80 != 0` check is removed as it makes ' ' an invalid pixel.
    // The upper bound is corrected to match the actual string length.
    if (pixel_index < TPAI_PIXEL_MAP_LENGTH) {
        *pixel_value = (int)TPAI_PIXEL_MAP[pixel_index];
        return true;
    }

    return false; // Pixel index out of bounds
}

// Function: tpai_display_image
bool tpai_display_image(TpaiImageContext *ctx) {
    if (!ctx) {
        return false;
    }

    // Chain success checks for header parsing
    if (!tpai_read_magic(ctx)) return false;
    if (!tpai_read_width(ctx)) return false;
    if (!tpai_read_height(ctx)) return false;

    unsigned int load_mode_val;
    if (!tpai_read_loadd(ctx, &load_mode_val)) return false;
    // ctx->load_mode is already set by tpai_read_loadd, this line is redundant but harmless:
    // ctx->load_mode = (int)load_mode_val;

    if (!tpai_skip_rsrvd(ctx)) return false;
    if (!tpai_calc_checksum(ctx)) return false;

    // Calculate image size
    size_t image_pixel_count = (size_t)ctx->width * ctx->height;
    if (image_pixel_count == 0) {
        printf("\n"); // Print newline for consistency if nothing to display
        return true; // No image data to display, but operations succeeded.
    }

    char *pixel_buffer = NULL;
    // Allocate image_pixel_count + 1 for null terminator
    if (!allocate(image_pixel_count + 1, 0, (void **)&pixel_buffer)) {
        return false;
    }
    memset(pixel_buffer, ' ', image_pixel_count); // Initialize with spaces
    pixel_buffer[image_pixel_count] = '\0';       // Null terminate

    for (size_t current_pixel_idx = 0; current_pixel_idx < image_pixel_count; ++current_pixel_idx) {
        int pixel_char_val = 0;
        if (!tpai_read_pixel(ctx, &pixel_char_val)) {
            deallocate(pixel_buffer, image_pixel_count + 1);
            return false;
        }

        unsigned int row = 0;
        unsigned int col = 0;

        // Calculate row and column based on load_mode
        switch (ctx->load_mode) {
            case 0: // Normal (row-major, top-left origin)
                row = current_pixel_idx / ctx->width;
                col = current_pixel_idx % ctx->width;
                break;
            case 1: // Flipped horizontally
                row = current_pixel_idx / ctx->width;
                col = (ctx->width - 1) - (current_pixel_idx % ctx->width);
                break;
            case 2: // Flipped vertically
                row = (ctx->height - 1) - (current_pixel_idx / ctx->width);
                col = current_pixel_idx % ctx->width;
                break;
            case 3: // Flipped horizontally and vertically
                row = (ctx->height - 1) - (current_pixel_idx / ctx->width);
                col = (ctx->width - 1) - (current_pixel_idx % ctx->width);
                break;
            case 4: // Transposed (column-major, top-left origin)
                row = current_pixel_idx % ctx->height;
                col = current_pixel_idx / ctx->height;
                break;
            case 5: // Transposed, flipped horizontally
                row = current_pixel_idx % ctx->height;
                col = (ctx->width - 1) - (current_pixel_idx / ctx->height);
                break;
            case 6: // Transposed, flipped vertically
                row = (ctx->height - 1) - (current_pixel_idx % ctx->height);
                col = current_pixel_idx / ctx->height;
                break;
            case 7: // Transposed, flipped horizontally and vertically
                row = (ctx->height - 1) - (current_pixel_idx % ctx->height);
                col = (ctx->width - 1) - (current_pixel_idx / ctx->height);
                break;
            default:
                printf("[ERROR] Unknown load mode: %d\n", ctx->load_mode);
                deallocate(pixel_buffer, image_pixel_count + 1);
                return false;
        }

        // Calculate linear index in the buffer
        size_t linear_buffer_idx = (size_t)row * ctx->width + col;

        if (linear_buffer_idx >= image_pixel_count) { // Should not happen if calculations are correct
            printf("[ERROR] Pixel out of bounds during placement\n");
            deallocate(pixel_buffer, image_pixel_count + 1);
            return false;
        }
        pixel_buffer[linear_buffer_idx] = (char)pixel_char_val;
    }

    // Print the image
    for (size_t i = 0; i < image_pixel_count; ++i) {
        // Print newline after each row, except before the very first character
        if (i % ctx->width == 0 && i != 0) {
            printf("\n");
        }
        printf("%c", pixel_buffer[i]);
    }
    printf("\n"); // Final newline after the image

    deallocate(pixel_buffer, image_pixel_count + 1);
    return true;
}

// Main function (dummy for compilation)
// This function was not part of the original snippet but is included
// to make the provided code a compilable unit.
int main() {
    // This is a placeholder main function.
    // To test, you would initialize a TpaiImageContext with actual image data
    // and then call tpai_display_image(&ctx).
    printf("Placeholder main function executed. No image data processed.\n");
    return 0;
}