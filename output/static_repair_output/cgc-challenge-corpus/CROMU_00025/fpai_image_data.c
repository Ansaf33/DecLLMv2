#include <stdio.h>    // For printf, perror
#include <stdlib.h>   // For malloc, free
#include <string.h>   // For memset
#include <stdint.h>   // For uint8_t, uint16_t, uint32_t, intptr_t
#include <stddef.h>   // For size_t

// Define the ImageContext structure based on memory access patterns
// This structure represents the state and metadata of the image being processed.
typedef struct {
    intptr_t data_base_ptr;       // Represents the base address of the image data buffer (uint8_t* cast to intptr_t)
    int      total_data_size_bytes; // Total allocated size for data_base_ptr in bytes
    int      current_byte_index;  // Current byte offset from data_base_ptr
    int      current_bit_index;   // Current bit offset within the byte at current_byte_index (0-7)
    
    // These fields are accessed directly via offsets (e.g., param_1 + 0x10 in original code)
    // which implies they are placed after the initial 4 int-sized fields (16 bytes) in the struct.
    uint32_t xaxis;               // 0x10 offset (width)
    uint32_t yaxis;               // 0x14 offset (height)
    uint32_t axist;               // 0x18 offset (axis type/orientation)
    uint16_t cksum;               // 0x1c offset (checksum)
} ImageContext;

// Function: fpai_add_pixel
// Adds a pixel to the display buffer based on image axis configuration.
// ctx: Pointer to the image context.
// pixel_buffer: The character array representing the display output.
// x, y: Coordinates of the pixel.
// pixel_value: The ASCII character value to place.
// Returns 1 on success, 0 on failure (e.g., out of bounds).
int fpai_add_pixel(ImageContext* ctx, char* pixel_buffer, int x, int y, char pixel_value) {
    if (!ctx || !pixel_buffer) {
        return 0;
    }

    int offset;
    int width = (int)ctx->xaxis;
    int height = (int)ctx->yaxis;

    switch (ctx->axist) {
        default:
            return 0; // Invalid axis type
        case 1: // Y-axis inverted
            offset = x + -y * width;
            break;
        case 2: // X-axis inverted, Y-axis inverted
            offset = x + width - 1 + -y * width;
            break;
        case 3: // X-axis normal, Y-axis inverted (original comment was wrong, it's Y inverted)
            offset = x + (height - 1 - y) * width;
            break;
        case 4: // X-axis inverted, Y-axis normal
            offset = x + width - 1 + (height - 1 - y) * width;
            break;
        case 7: // Centered
            offset = x + -y * width + width / 2 + width * (height / 2);
            break;
    }

    // Check if the calculated offset is within the bounds of the pixel buffer
    if (offset < 0 || (size_t)offset >= (size_t)(height * width)) {
        return 0; // Out of bounds
    } else {
        pixel_buffer[offset] = pixel_value;
        return 1;
    }
}

// Function: fpai_read_check
// Checks if there are enough bits remaining in the image data stream to read.
// ctx: Pointer to the image context.
// bits_to_read: The number of bits required for the next read operation.
// Returns 1 if enough bits are available, 0 otherwise.
int fpai_read_check(ImageContext* ctx, int bits_to_read) {
    if (!ctx || !ctx->data_base_ptr) {
        return 0;
    }

    // Calculate total bits available and bits already read
    size_t total_bits_available = (size_t)ctx->total_data_size_bytes * 8;
    size_t bits_already_read = (size_t)ctx->current_byte_index * 8 + ctx->current_bit_index;
    
    // Check if the remaining bits are sufficient
    if (total_bits_available - bits_already_read < (size_t)bits_to_read) {
        return 0;
    }
    return 1;
}

// Function: fpai_read_nbits
// Reads a specified number of bits from the image data stream.
// ctx: Pointer to the image context.
// num_bits: The number of bits to read (0-32).
// value_out: Pointer to a uint32_t where the read bits will be stored.
// Returns 1 on success, 0 on failure (e.g., invalid num_bits, NULL pointers).
int fpai_read_nbits(ImageContext* ctx, int num_bits, uint32_t* value_out) {
    // num_bits must be positive and fit within uint32_t
    if (num_bits < 0 || num_bits > 32) {
        return 0;
    }
    if (!ctx || !value_out || !ctx->data_base_ptr) {
        return 0;
    }

    uint32_t current_value = 0;
    // Cast data_base_ptr to uint8_t* to access individual bytes
    uint8_t* data = (uint8_t*)ctx->data_base_ptr;

    for (int i = 0; i < num_bits; i++) {
        // Read the current bit, assuming MSB-first within each byte (7-bit_index)
        uint8_t bit = (data[ctx->current_byte_index] >> (7 - ctx->current_bit_index)) & 1U;
        current_value = (current_value << 1) | bit; // Build the value MSB first

        // Advance bit/byte index
        ctx->current_bit_index++;
        if (ctx->current_bit_index == 8) { // Moved to the next byte
            ctx->current_bit_index = 0;
            ctx->current_byte_index++;
        }
    }
    *value_out = current_value;
    return 1;
}

// Function: fpai_read_magic
// Reads and validates the 32-bit magic number from the image data.
// ctx: Pointer to the image context.
// Returns 1 on success (magic number matches), 0 on failure.
int fpai_read_magic(ImageContext* ctx) {
    if (!ctx) return 0;

    uint32_t magic_val = 0;
    // Check for 32 bits and read them
    if (!fpai_read_check(ctx, 0x20) || !fpai_read_nbits(ctx, 0x20, &magic_val)) {
        return 0;
    }

    // Check if the magic value matches the expected pattern (DE B6 D9 55)
    if (((magic_val >> 24) & 0xFF) == 0xDE &&
        ((magic_val >> 16) & 0xFF) == 0xB6 &&
        ((magic_val >> 8) & 0xFF) == 0xD9 &&
        (magic_val & 0xFF) == 0x55) {
        return 1;
    }
    return 0;
}

// Function: fpai_read_xaxis
// Reads the 6-bit X-axis (width) value from the image data.
// ctx: Pointer to the image context.
// Returns 1 on success, 0 on failure.
int fpai_read_xaxis(ImageContext* ctx) {
    if (!ctx) return 0;

    uint32_t val = 0;
    // Check for 6 bits and read them
    if (!fpai_read_check(ctx, 6) || !fpai_read_nbits(ctx, 6, &val)) {
        return 0;
    }
    ctx->xaxis = val; // Store the read value in the context
    return 1;
}

// Function: fpai_read_yaxis
// Reads the 6-bit Y-axis (height) value from the image data.
// ctx: Pointer to the image context.
// Returns 1 on success, 0 on failure.
int fpai_read_yaxis(ImageContext* ctx) {
    if (!ctx) return 0;

    uint32_t val = 0;
    // Check for 6 bits and read them
    if (!fpai_read_check(ctx, 6) || !fpai_read_nbits(ctx, 6, &val)) {
        return 0;
    }
    ctx->yaxis = val; // Store the read value in the context
    return 1;
}

// Function: fpai_read_axist
// Reads the 3-bit axis type value from the image data and validates it.
// ctx: Pointer to the image context.
// Returns 1 on success, 0 on failure (e.g., invalid axis type).
int fpai_read_axist(ImageContext* ctx) {
    if (!ctx) return 0;

    uint32_t val = 0;
    // Check for 3 bits and read them
    if (!fpai_read_check(ctx, 3) || !fpai_read_nbits(ctx, 3, &val)) {
        return 0;
    }

    // Axis types 0, 5, and 6 are considered invalid by the original logic
    if (val == 0 || val == 5 || val == 6) {
        return 0;
    }
    ctx->axist = val; // Store the valid axis type
    return 1;
}

// Function: fpai_read_cksum
// Reads the 16-bit checksum value from the image data.
// ctx: Pointer to the image context.
// Returns 1 on success, 0 on failure.
int fpai_read_cksum(ImageContext* ctx) {
    if (!ctx) return 0;

    uint32_t val = 0;
    // Check for 16 bits and read them
    if (!fpai_read_check(ctx, 0x10) || !fpai_read_nbits(ctx, 0x10, &val)) {
        return 0;
    }
    ctx->cksum = (uint16_t)val; // Store the read checksum
    return 1;
}

// Function: fpai_calc_cksum
// Calculates the checksum of the remaining image data and compares it to the stored checksum.
// ctx: Pointer to the image context.
// Returns 1 if checksums match, 0 on failure (e.g., checksum mismatch, data alignment issues).
int fpai_calc_cksum(ImageContext* ctx) {
    if (!ctx || !ctx->data_base_ptr) {
        return 0;
    }

    // Checksum calculation expects to start on a byte boundary
    if (ctx->current_bit_index != 0) {
        return 0;
    }

    // Remaining data must be an even number of bytes for 16-bit reads
    if ((ctx->total_data_size_bytes - ctx->current_byte_index) % 2 != 0) {
        return 0;
    }

    uint16_t calculated_checksum = 0;
    uint8_t* data_ptr = (uint8_t*)ctx->data_base_ptr;

    // Iterate over remaining data, reading 16 bits (2 bytes) at a time
    for (int i = 0; i < (ctx->total_data_size_bytes - ctx->current_byte_index) / 2; i++) {
        // Reads 2 bytes as a uint16_t. Assumes native endianness, matching original behavior.
        calculated_checksum += *(uint16_t*)(data_ptr + ctx->current_byte_index + i * 2);
    }

    if (calculated_checksum == ctx->cksum) {
        return 1;
    } else {
        printf("[ERROR] Checksum failed. Expected: %hu, Got: %hu\n", ctx->cksum, calculated_checksum);
        return 0;
    }
}

// Function: fpai_read_pixel
// Reads pixel data (X-sign, X-value, Y-sign, Y-value, character index) from the image data stream.
// ctx: Pointer to the image context.
// x_out, y_out: Pointers to integers to store the calculated X and Y coordinates.
// pixel_val_out: Pointer to an integer to store the ASCII character value for the pixel.
// Returns 1 on success, 0 on failure.
int fpai_read_pixel(ImageContext* ctx, int* x_out, int* y_out, int* pixel_val_out) {
    if (!ctx || !x_out || !y_out || !pixel_val_out) {
        return 0;
    }

    uint32_t x_sign, x_val, y_sign, y_val, char_idx;

    // Read X-sign bit (1 bit)
    if (!fpai_read_check(ctx, 1) || !fpai_read_nbits(ctx, 1, &x_sign)) return 0;
    // Read X-value (6 bits)
    if (!fpai_read_check(ctx, 6) || !fpai_read_nbits(ctx, 6, &x_val)) return 0;
    // Read Y-sign bit (1 bit)
    if (!fpai_read_check(ctx, 1) || !fpai_read_nbits(ctx, 1, &y_sign)) return 0;
    // Read Y-value (6 bits)
    if (!fpai_read_check(ctx, 6) || !fpai_read_nbits(ctx, 6, &y_val)) return 0;
    // Read character index (7 bits)
    if (!fpai_read_check(ctx, 7) || !fpai_read_nbits(ctx, 7, &char_idx)) return 0;

    // Character index must be less than 95 (0x5F) as per original logic (0x20 to 0x7E ASCII range)
    if (char_idx >= 0x5f) {
        return 0;
    }

    // Apply sign to coordinates
    *x_out = (x_sign != 0) ? -(int)x_val : (int)x_val;
    *y_out = (y_sign != 0) ? -(int)y_val : (int)y_val;
    // Convert character index to ASCII value (0x20 is ASCII space)
    *pixel_val_out = (int)(char_idx + 0x20);
    return 1;
}

// Function: fpai_display_img
// Main function to read, validate, and display an image from the provided context.
// ctx: Pointer to the image context containing the image data and metadata.
// Returns 1 on successful display, 0 on any failure.
int fpai_display_img(ImageContext* ctx) {
    if (!ctx) {
        return 0;
    }

    // Read and validate all image header information
    if (!(fpai_read_magic(ctx) &&
          fpai_read_xaxis(ctx) &&
          fpai_read_yaxis(ctx) &&
          fpai_read_axist(ctx) &&
          fpai_read_cksum(ctx))) {
        return 0; // Return on any header read/validation failure
    }

    // Calculate and verify checksum
    if (!fpai_calc_cksum(ctx)) {
        // Error message already printed by fpai_calc_cksum
        return 0;
    }

    int x_min = 0, y_min = 0, x_max = 0, y_max = 0;
    int width = (int)ctx->xaxis;
    int height = (int)ctx->yaxis;

    // Determine coordinate bounds based on axis type
    switch (ctx->axist) {
        default: // This case should ideally not be reached if fpai_read_axist succeeded
            return 0;
        case 1: // Y-axis inverted
            x_min = 0;
            y_min = 1 - height;
            x_max = width - 1;
            y_max = 0;
            break;
        case 2: // X-axis inverted, Y-axis inverted
            x_min = 1 - width;
            y_min = 1 - height;
            x_max = 0;
            y_max = 0;
            break;
        case 3: // X-axis normal, Y-axis inverted
            x_min = 0;
            y_min = 0;
            x_max = width - 1;
            y_max = height - 1;
            break;
        case 4: // X-axis inverted, Y-axis normal
            x_min = 1 - width;
            y_min = 0;
            x_max = 0;
            y_max = height - 1;
            break;
        case 7: // Centered
            x_min = -(width / 2);
            y_max = height / 2;
            x_max = width / 2 - (width + 1) % 2;
            y_min = (height + 1) % 2 - height / 2;
            break;
    }

    size_t total_pixels = (size_t)width * height;
    // Allocate buffer for displaying pixels (+1 for null terminator for safety, though not strictly used by printf %c)
    char* pixel_buffer = (char*)malloc(total_pixels + 1);
    if (!pixel_buffer) {
        perror("[ERROR] Failed to allocate pixel buffer");
        return 0;
    }

    memset(pixel_buffer, ' ', total_pixels); // Initialize buffer with spaces
    pixel_buffer[total_pixels] = '\0';       // Null-terminate the buffer

    int x_coord, y_coord, pixel_char_val;
    // Loop to read and place pixels until fpai_read_pixel indicates no more pixels or an error
    while (fpai_read_pixel(ctx, &x_coord, &y_coord, &pixel_char_val)) {
        // Validate pixel coordinates against calculated bounds
        if (x_coord < x_min || x_coord > x_max) {
            printf("[ERROR] X coordinate out of bounds: %d (expected %d-%d)\n", x_coord, x_min, x_max);
            free(pixel_buffer);
            return 0;
        }
        if (y_coord < y_min || y_coord > y_max) {
            printf("[ERROR] Y coordinate out of bounds: %d (expected %d-%d)\n", y_coord, y_min, y_max);
            free(pixel_buffer);
            return 0;
        }

        // Add the pixel to the buffer
        if (!fpai_add_pixel(ctx, pixel_buffer, x_coord, y_coord, (char)pixel_char_val)) {
            printf("[ERROR] Pixel placement failed\n");
            free(pixel_buffer);
            return 0;
        }
    }
    // The loop naturally terminates when fpai_read_pixel returns 0 (either EOF or an error during read).

    // Display the image content
    for (size_t i = 0; i < total_pixels; i++) {
        // Print a newline at the end of each row, except before the first character of the first row
        if ((i % width == 0) && (i != 0)) {
            printf("\n");
        }
        printf("%c", pixel_buffer[i]);
    }
    printf("\n"); // Ensure a newline after the last row

    free(pixel_buffer); // Free the allocated buffer
    return 1;
}