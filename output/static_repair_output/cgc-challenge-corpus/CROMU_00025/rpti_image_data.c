#include <stdio.h>   // For printf, fprintf, stderr
#include <stdlib.h>  // For malloc, free
#include <string.h>  // For memset, memcpy
#include <stdint.h>  // For uint32_t, size_t

// Structure to hold RPTI context, replacing the generic `int *param_1` array
typedef struct {
    char *data_buffer;
    size_t buffer_size_bytes;
    size_t current_byte_offset;
    int current_bit_offset_in_byte; // 0-7
} RPTIContext;

// --- Helper functions (allocate/deallocate) ---
// Returns 1 on success, 0 on failure. Sets *ptr_out.
static int allocate(size_t size, int unused_param, void **ptr_out) {
    void *ptr = malloc(size);
    if (ptr == NULL) {
        if (ptr_out != NULL) {
            *ptr_out = NULL;
        }
        return 0; // Failure
    }
    if (ptr_out != NULL) {
        *ptr_out = ptr;
    }
    return 1; // Success
}

static void deallocate(void *ptr, size_t unused_size) {
    free(ptr);
}

// --- Function Prototypes ---
// Declared here to resolve circular dependencies and allow compilation.
// All `undefined4` return types are mapped to `int` (0 for failure, 1 for success).
// `uint` is mapped to `unsigned int`.
int rpti_read_check(RPTIContext *ctx, int num_bits_to_read);
int rpti_inc_index(RPTIContext *ctx, int bits_to_inc);
int rpti_read_bits(RPTIContext *ctx, int num_bits, unsigned int *value_out);
int rpti_read_magic(RPTIContext *ctx);
int rpti_read_xaxis(RPTIContext *ctx);
int rpti_read_yaxis(RPTIContext *ctx);
int rpti_read_initx(RPTIContext *ctx, int *init_x_out);
int rpti_read_inity(RPTIContext *ctx, int *init_y_out);
int rpti_read_axist(RPTIContext *ctx);
int rpti_read_pixel(RPTIContext *ctx, unsigned int *pixel_value_out);
int rpti_add_pixel(char *image_buffer, int x_coord, int y_coord, int axis_type, int image_width, int image_height);


// Function: rpti_add_pixel
int rpti_add_pixel(char *image_buffer, int x_coord, int y_coord, int axis_type, int image_width, int image_height) {
    if (image_buffer == NULL) {
        return 0;
    }

    int pixel_index;
    switch (axis_type) {
        case 1:
            pixel_index = x_coord - y_coord * image_width;
            break;
        case 2:
            pixel_index = -y_coord * image_width + x_coord + image_width - 1;
            break;
        case 3:
            pixel_index = x_coord + (image_height - 1 - y_coord) * image_width;
            break;
        case 4:
            pixel_index = (image_height - 1 - y_coord) * image_width + x_coord + image_width - 1;
            break;
        case 7:
            pixel_index = x_coord - y_coord * image_width + image_width / 2 + (image_height / 2) * image_width;
            break;
        default:
            return 0; // Invalid axis_type
    }

    if (pixel_index < 0 || (size_t)image_width * image_height <= (size_t)pixel_index) {
        fprintf(stderr, "[ERROR] Pixel beyond image border (index %d, max %zu)\n", pixel_index, (size_t)image_width * image_height - 1);
        return 0;
    } else {
        image_buffer[pixel_index] = '.'; // 0x2e is '.'
        return 1;
    }
}

// Function: rpti_display_img
int rpti_display_img(RPTIContext *ctx) {
    char *image_buffer = NULL;
    size_t image_size = 0;
    int image_width = 0;
    int image_height = 0;
    int current_x = 0;
    int current_y = 0;
    int axis_type = 0;

    // Read header information
    if (!rpti_read_magic(ctx)) {
        fprintf(stderr, "RPTI magic fail\n");
        return 0;
    }

    image_width = rpti_read_xaxis(ctx);
    if (image_width == 0) {
        fprintf(stderr, "rpti xlen fail\n");
        return 0;
    }

    image_height = rpti_read_yaxis(ctx);
    if (image_height == 0) {
        fprintf(stderr, "rpti ylen fail\n");
        return 0;
    }

    if (!rpti_read_initx(ctx, &current_x)) {
        fprintf(stderr, "rpti initx fail\n");
        return 0;
    }

    if (!rpti_read_inity(ctx, &current_y)) {
        fprintf(stderr, "rpti inity fail\n");
        return 0;
    }

    axis_type = rpti_read_axist(ctx);
    if (axis_type == 0) {
        fprintf(stderr, "axis type fail\n");
        return 0;
    }

    // This 3-bit increment seems to be moving past some reserved bits after axis_type.
    if (!rpti_inc_index(ctx, 3)) {
        // No specific error message in original, just returns 0.
        return 0;
    }

    int min_x, max_x, min_y, max_y;
    switch (axis_type) {
        case 1:
            min_x = 0;
            max_x = image_width - 1;
            min_y = 1 - image_height;
            max_y = 0;
            break;
        case 2:
            min_x = 1 - image_width;
            max_x = 0;
            min_y = 1 - image_height;
            max_y = 0;
            break;
        case 3:
            min_x = 0;
            max_x = image_width - 1;
            min_y = 0;
            max_y = image_height - 1;
            break;
        case 4:
            min_x = 1 - image_width;
            max_x = 0;
            min_y = 0;
            max_y = image_height - 1;
            break;
        case 7:
            max_x = image_width / 2 - (image_width + 1) % 2;
            min_x = -(image_width / 2);
            max_y = image_height / 2;
            min_y = (image_height + 1) % 2 - image_height / 2;
            break;
        default:
            fprintf(stderr, "type switch fail\n");
            return 0;
    }

    image_size = (size_t)image_width * image_height;
    if (!allocate(image_size + 1, 0, (void **)&image_buffer)) {
        fprintf(stderr, "Failed to allocate image buffer\n");
        return 0; // allocate failed
    }

    memset(image_buffer, ' ', image_size);
    image_buffer[image_size] = '\0'; // Null-terminate for safety when printing as string

    unsigned int read_pixel_value;
    while (rpti_read_pixel(ctx, &read_pixel_value)) {
        int delta_x = (int)(read_pixel_value >> 7);
        int delta_y = (int)(read_pixel_value & 0x7f);

        // Sign extension for 7-bit values (if the 7th bit is set, it's negative)
        if ((delta_x & 0x40) != 0) { // Check 7th bit (0-indexed)
            delta_x |= ~0x7F; // Sign extend to 32-bit (fill higher bits with 1s)
        }
        if ((delta_y & 0x40) != 0) {
            delta_y |= ~0x7F;
        }

        int new_x = current_x + delta_x;
        int new_y = current_y + delta_y;

        if (new_x < min_x || new_x > max_x) {
            fprintf(stderr, "X out of bounds (X: %d, Range: [%d, %d])\n", new_x, min_x, max_x);
            deallocate(image_buffer, image_size + 1);
            return 0;
        }
        if (new_y < min_y || new_y > max_y) {
            fprintf(stderr, "Y out of bounds (Y: %d, Range: [%d, %d])\n", new_y, min_y, max_y);
            deallocate(image_buffer, image_size + 1);
            return 0;
        }

        if (!rpti_add_pixel(image_buffer, new_x, new_y, axis_type, image_width, image_height)) {
            deallocate(image_buffer, image_size + 1);
            fprintf(stderr, "add pixel fail\n");
            return 0;
        }

        current_x = new_x;
        current_y = new_y;
    }

    // Display image
    for (size_t i = 0; i < image_size; ++i) {
        if (i % image_width == 0 && i != 0) {
            printf("\n");
        }
        printf("%c", image_buffer[i]);
    }
    printf("\n");

    deallocate(image_buffer, image_size + 1);
    return 1;
}

// Function: rpti_read_bits
int rpti_read_bits(RPTIContext *ctx, int num_bits, unsigned int *value_out) {
    if (value_out == NULL || num_bits <= 0 || num_bits > 32) {
        return 0;
    }
    if (!rpti_read_check(ctx, num_bits)) {
        return 0;
    }

    unsigned int bits_read = 0;
    char *data_buffer = ctx->data_buffer;
    size_t *byte_offset = &ctx->current_byte_offset;
    int *bit_offset = &ctx->current_bit_offset_in_byte;

    for (int i = 0; i < num_bits; ++i) {
        char current_byte_val = data_buffer[*byte_offset];
        int bit_val = (current_byte_val >> (7 - *bit_offset)) & 1; // Read MSB first

        bits_read = (bits_read << 1) | bit_val; // Accumulate bits

        (*bit_offset)++;
        if (*bit_offset == 8) {
            *bit_offset = 0;
            (*byte_offset)++;
        }
    }

    *value_out = bits_read;
    return 1;
}

// Function: rpti_inc_index
int rpti_inc_index(RPTIContext *ctx, int bits_to_inc) {
    if (ctx == NULL || ctx->data_buffer == NULL) {
        return 0;
    }
    if (!rpti_read_check(ctx, bits_to_inc)) {
        return 0;
    }

    int total_bits_offset = ctx->current_bit_offset_in_byte + bits_to_inc;
    ctx->current_byte_offset += total_bits_offset / 8;
    ctx->current_bit_offset_in_byte = total_bits_offset % 8;

    return 1;
}

// Function: rpti_read_check
int rpti_read_check(RPTIContext *ctx, int num_bits_to_read) {
    if (ctx == NULL || ctx->data_buffer == NULL) {
        return 0;
    }

    long long total_bits_available = (long long)ctx->buffer_size_bytes * 8;
    long long current_bit_position = (long long)ctx->current_byte_offset * 8 + ctx->current_bit_offset_in_byte;

    if (num_bits_to_read < 0 || total_bits_available - current_bit_position < num_bits_to_read) {
        return 0;
    }
    return 1;
}

// Function: rpti_read_magic
int rpti_read_magic(RPTIContext *ctx) {
    if (ctx == NULL || ctx->data_buffer == NULL) {
        return 0;
    }

    // The original code implies this function should be called at the start of the stream.
    if (ctx->current_byte_offset != 0 || ctx->current_bit_offset_in_byte != 0) {
        fprintf(stderr, "rpti_read_magic called at non-zero offset.\n");
        return 0;
    }

    if (!rpti_read_check(ctx, 32)) { // Magic is 4 bytes = 32 bits
        fprintf(stderr, "Not enough bits for magic value.\n");
        return 0;
    }

    uint32_t magic_val;
    // Copy 4 bytes from the start of the data buffer into magic_val
    memcpy(&magic_val, ctx->data_buffer, 4);

    // The magic value -0x3caef62d is 0xC35109D3 in hex (signed 32-bit).
    if (magic_val != 0xC35109D3) {
        fprintf(stderr, "RPTI magic value mismatch: expected 0x%X, got 0x%X\n", 0xC35109D3, magic_val);
        return 0;
    }

    if (!rpti_inc_index(ctx, 32)) {
        return 0; // Should not fail after read_check and successful memcpy
    }

    return 1;
}

// Function: rpti_read_xaxis
int rpti_read_xaxis(RPTIContext *ctx) {
    unsigned int x_axis_val = 0;
    if (!rpti_read_bits(ctx, 6, &x_axis_val)) {
        return 0;
    }
    return (int)x_axis_val;
}

// Function: rpti_read_yaxis
int rpti_read_yaxis(RPTIContext *ctx) {
    unsigned int y_axis_val = 0;
    if (!rpti_read_bits(ctx, 6, &y_axis_val)) {
        return 0;
    }
    return (int)y_axis_val;
}

// Function: rpti_read_initx
int rpti_read_initx(RPTIContext *ctx, int *init_x_out) {
    if (init_x_out == NULL) {
        return 0;
    }

    unsigned int is_negative_flag = 0;
    if (!rpti_read_bits(ctx, 1, &is_negative_flag)) {
        return 0;
    }

    unsigned int abs_x_val = 0;
    if (!rpti_read_bits(ctx, 6, &abs_x_val)) {
        return 0;
    }

    *init_x_out = (int)abs_x_val;
    if (is_negative_flag == 1) {
        *init_x_out = -(*init_x_out);
    }
    return 1;
}

// Function: rpti_read_inity
int rpti_read_inity(RPTIContext *ctx, int *init_y_out) {
    if (init_y_out == NULL) {
        return 0;
    }

    unsigned int is_negative_flag = 0;
    if (!rpti_read_bits(ctx, 1, &is_negative_flag)) {
        return 0;
    }

    unsigned int abs_y_val = 0;
    if (!rpti_read_bits(ctx, 6, &abs_y_val)) {
        return 0;
    }

    *init_y_out = (int)abs_y_val;
    if (is_negative_flag == 1) {
        *init_y_out = -(*init_y_out);
    }
    return 1;
}

// Function: rpti_read_axist
int rpti_read_axist(RPTIContext *ctx) {
    unsigned int axis_type_val = 0;
    if (!rpti_read_bits(ctx, 3, &axis_type_val)) {
        return 0;
    }

    // Original check: `(4 < local_10[0]) && (local_10[0] < 7)`
    // This means if axis_type_val is 5 or 6, it's invalid.
    if (axis_type_val > 4 && axis_type_val < 7) {
        fprintf(stderr, "Invalid axis type read: %u\n", axis_type_val);
        return 0;
    }
    return (int)axis_type_val;
}

// Function: rpti_read_pixel
int rpti_read_pixel(RPTIContext *ctx, unsigned int *pixel_value_out) {
    if (pixel_value_out == NULL) {
        return 0;
    }

    unsigned int delta_x_bits = 0;
    if (!rpti_read_bits(ctx, 7, &delta_x_bits)) {
        return 0;
    }

    unsigned int delta_y_bits = 0;
    if (!rpti_read_bits(ctx, 7, &delta_y_bits)) {
        return 0;
    }

    *pixel_value_out = (delta_x_bits << 7) | delta_y_bits;
    return 1;
}

// --- Main function for demonstration ---
int main() {
    // Example RPTI data for a 10x5 image, starting at (0,0), axis type 3 (standard cartesian)
    // and drawing a simple diagonal line.
    //
    // Binary structure of the data:
    // Magic: 0xC35109D3 (4 bytes)
    // Width: 10 (6 bits: 001010)
    // Height: 5 (6 bits: 000101)
    // InitX sign: 0 (1 bit: 0)
    // InitX value: 0 (6 bits: 000000)
    // InitY sign: 0 (1 bit: 0)
    // InitY value: 0 (6 bits: 000000)
    // Axis Type: 3 (3 bits: 011)
    // Reserved: (3 bits: 000, incremented by rpti_inc_index)
    //
    // Pixel 1: delta_x = 1 (7 bits: 0000001), delta_y = 1 (7 bits: 0000001)
    // Pixel 2: delta_x = 1 (7 bits: 0000001), delta_y = 0 (7 bits: 0000000)
    // Pixel 3: delta_x = 0 (7 bits: 0000000), delta_y = 1 (7 bits: 0000001)
    // Pixel 4: delta_x = -1 (7 bits: 1111111), delta_y = 0 (7 bits: 0000000)
    // Pixel 5: delta_x = 0 (7 bits: 0000000), delta_y = -1 (7 bits: 1111111)

    unsigned char raw_data[] = {
        0xC3, 0x51, 0x09, 0xD3, // Magic: 0xC35109D3
        // Next 4 bytes represent 6+6+7+7+3+3 = 32 bits of header data
        // Width (10): 001010
        // Height (5): 000101
        // InitX_sign (0): 0
        // InitX_val (0): 000000
        // InitY_sign (0): 0
        // InitY_val (0): 000000
        // Axis_type (3): 011
        // Reserved (0): 000
        //
        // Byte 4: 00101000 (width 10, first 2 bits of height) -> 0x28
        // Byte 5: 01000000 (last 4 bits of height, first 4 bits of InitX) -> 0x40
        // Byte 6: 00000000 (last 3 bits of InitX, first 5 bits of InitY) -> 0x00
        // Byte 7: 00110000 (last 2 bits of InitY, AxisType 3, Reserved 0) -> 0x18
        0x28, 0x40, 0x00, 0x18, // Header data
        // Pixel data (7 bits dx, 7 bits dy)
        // Pixel 1: dx=1, dy=1 -> 0000001 0000001
        0x01, 0x01,
        // Pixel 2: dx=1, dy=0 -> 0000001 0000000
        0x01, 0x00,
        // Pixel 3: dx=0, dy=1 -> 0000000 0000001
        0x00, 0x01,
        // Pixel 4: dx=-1, dy=0 -> 1111111 0000000 (7-bit signed -1 is 0x7F)
        0x7F, 0x00,
        // Pixel 5: dx=0, dy=-1 -> 0000000 1111111
        0x00, 0x7F
    };

    RPTIContext ctx = {
        .data_buffer = (char *)raw_data,
        .buffer_size_bytes = sizeof(raw_data),
        .current_byte_offset = 0,
        .current_bit_offset_in_byte = 0
    };

    printf("Attempting to display RPTI image...\n");
    if (rpti_display_img(&ctx)) {
        printf("RPTI image displayed successfully.\n");
    } else {
        printf("Failed to display RPTI image.\n");
    }

    // Example of an empty (invalid) context
    RPTIContext empty_ctx = {0};
    printf("\nAttempting to display with empty context (should fail)...\n");
    if (!rpti_display_img(&empty_ctx)) {
        printf("Empty context test: Failed as expected.\n");
    } else {
        printf("Empty context test: Unexpected success.\n");
    }

    return 0;
}