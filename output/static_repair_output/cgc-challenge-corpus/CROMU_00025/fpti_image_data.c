#include <stdio.h>    // For printf
#include <stdlib.h>   // For malloc, free
#include <string.h>   // For memset
#include <stddef.h>   // For size_t
#include <stdint.h>   // For uintptr_t

// Replace decompiler-specific types with standard C types
typedef int int32_t;
typedef unsigned int uint32_t;
typedef char byte;

// FPTI_Context structure based on usage in the provided code snippet's
// memory access patterns (e.g., param_1[0], param_1[1], *(char*)(param_1 + 0x10)).
// This layout ensures that array-like access via `int*` (as suggested by decompiler)
// and explicit offset access `*(char*)(param_1 + 0x10)` map correctly to struct members.
typedef struct {
    uintptr_t data_ptr_val; // Stores the char* data buffer address (corresponds to *param_1)
    int data_len;           // Total length of data buffer in bytes (corresponds to param_1[1])
    int byte_idx;           // Current byte offset in data buffer (corresponds to param_1[2])
    int bit_idx;            // Current bit offset within the current byte (0-7) (corresponds to param_1[3])
    // The previous 4 members (4 * sizeof(int)) total 16 bytes if sizeof(int) is 4.
    // This makes pixel_type start at offset 16 (0x10).
    byte pixel_type;        // Type of pixel char to use ('.', '*', '#', '+') (corresponds to *(char*)(param_1 + 0x10))
    byte _pad[3];           // Padding to align the struct if needed, or to match original size
} FPTI_Context;

// Helper function for power of 2, used by expi.
// Original code implicitly assumes base=2.
// It's used for small powers, so `1 << power` is safe.
static inline int expi(int base, int power) {
    (void)base; // Base is always 2 in the original code
    return 1 << power;
}

// Function: allocate (wrapper for malloc)
// Returns 0 on success, 1 on failure, matching original logic.
static inline int allocate(size_t num_elements, size_t element_size, void **out_ptr) {
    if (element_size == 0) element_size = 1; // Handle case where element_size is 0
    *out_ptr = malloc(num_elements * element_size);
    return (*out_ptr != NULL) ? 0 : 1; // 0 for success, 1 for failure
}

// Function prototypes
int fpti_read_check(FPTI_Context *ctx, int num_bits);
int fpti_read_nbits(FPTI_Context *ctx, int num_bits, uint32_t *out_value);

// Function: fpti_add_pixel
int32_t fpti_add_pixel(FPTI_Context *ctx, int x, int y, byte *image_buffer, int image_width, int image_height, int axis_type) {
    if (ctx == NULL || image_buffer == NULL) {
        return 0;
    }

    byte pixel_char = 0;
    if (ctx->pixel_type == '\0') {
        pixel_char = '.'; // 0x2e
    } else if (ctx->pixel_type == '\x01') {
        pixel_char = '*'; // 0x2a
    } else if (ctx->pixel_type == '\x02') {
        pixel_char = '#'; // 0x23
    } else if (ctx->pixel_type == '\x03') {
        pixel_char = '+'; // 0x2b
    }

    int pixel_idx;
    switch (axis_type) {
        case 1:
            pixel_idx = x + (-y) * image_width;
            break;
        case 2:
            pixel_idx = (-y) * image_width + x + image_width + (-1);
            break;
        case 3:
            pixel_idx = x + ((image_height - 1) - y) * image_width;
            break;
        case 4:
            pixel_idx = ((image_height - 1) - y) * image_width + x + image_width + (-1);
            break;
        case 7:
            pixel_idx = x + (-y) * image_width + image_width / 2 + (image_height / 2) * image_width;
            break;
        default:
            return 0; // Invalid param_7 value
    }

    if (image_width * image_height < pixel_idx || pixel_idx < 0) { // Added pixel_idx < 0 check
        printf("[ERROR] Pixel beyond image: %d > %d\n", pixel_idx, image_width * image_height);
        return 0;
    } else {
        image_buffer[pixel_idx] = pixel_char;
        return 1;
    }
}

// Function: fpti_display_img
int32_t fpti_display_img(FPTI_Context *ctx) {
    if (ctx == NULL || !fpti_read_magic(ctx)) {
        return 0;
    }

    int image_width = fpti_read_xaxis(ctx);
    if (image_width == 0) return 0;

    int image_height = fpti_read_yaxis(ctx);
    if (image_height == 0) return 0;

    int32_t pixel_type_val;
    if (!fpti_read_ptype(ctx, &pixel_type_val)) return 0;

    int axis_type = fpti_read_axist(ctx);
    if (axis_type == 0) {
        printf("[ERROR] Invalid FPTI Axis Type\n");
        return 0;
    }

    // Combined check for 0xf, 0xf and other parts of the original conditional
    if (!fpti_read_check(ctx, 0xf) || !fpti_read_nbits(ctx, 0xf, (uint32_t*)&(ctx->data_len))) { // The original passes &local_38 which is int. Using ctx->data_len as a dummy for now.
        return 0;
    }

    int min_x = 0, min_y = 0, max_x = 0, max_y = 0;
    switch (axis_type) {
        default: // Should be caught earlier by axis_type == 0 check
            printf("[ERROR] Invalid FPTI Axis Type\n");
            return 0;
        case 1:
            min_x = 0;
            min_y = 1 - image_height;
            max_x = image_width - 1;
            max_y = 0;
            break;
        case 2:
            min_x = 1 - image_width;
            min_y = 1 - image_height;
            max_x = 0;
            max_y = 0;
            break;
        case 3:
            min_x = 0;
            min_y = 0;
            max_x = image_width - 1;
            max_y = image_height - 1;
            break;
        case 4:
            min_x = 1 - image_width;
            min_y = 0;
            max_x = 0;
            max_y = image_height - 1;
            break;
        case 7:
            min_x = -(image_width / 2);
            max_y = image_height / 2;
            max_x = image_width / 2 - (image_width + 1) % 2;
            min_y = (image_height + 1) % 2 - image_height / 2;
            break;
    }

    size_t image_size = (size_t)image_width * image_height;
    byte *image_buffer = NULL;
    if (allocate(image_size + 1, 1, (void **)&image_buffer) != 0) { // allocate returns 0 on success
        return 0; // Allocation failed
    }

    memset(image_buffer, 0x20, image_size);
    image_buffer[image_size] = '\0'; // Null-terminate

    int pixel_x, pixel_y;
    while (fpti_read_pixel(ctx, &pixel_x, &pixel_y)) { // Loop while fpti_read_pixel is successful (returns 1)
        if (pixel_x < min_x || max_x < pixel_x) {
            printf("X out of bounds: %d\n", pixel_x);
            free(image_buffer);
            return 0;
        }
        // Original had &&, changed to || for correct "Y out of bounds" logic
        if (pixel_y < min_y || max_y < pixel_y) {
            printf("Y out of bounds: %d\n", pixel_y);
            free(image_buffer);
            return 0;
        }
        if (!fpti_add_pixel(ctx, pixel_x, pixel_y, image_buffer, image_width, image_height, axis_type)) {
            free(image_buffer);
            return 0; // Error adding pixel
        }
    }

    // If fpti_read_pixel returns 0, it means end of pixels or an error during read.
    // The original code displays the image in this case.
    for (int i = 0; i < image_size; ++i) {
        if (i % image_width == 0 && i != 0) {
            printf("\n");
        }
        printf("%c", image_buffer[i]);
    }
    printf("\n");

    free(image_buffer);
    return 1;
}

// Function: fpti_read_pixel
int32_t fpti_read_pixel(FPTI_Context *ctx, int *x_out, int *y_out) {
    if (ctx == NULL || x_out == NULL || y_out == NULL) {
        return 0;
    }

    uint32_t sign_x_bit, x_val, sign_y_bit, y_val;
    if (!fpti_read_check(ctx, 0xe) ||
        !fpti_read_nbits(ctx, 1, &sign_x_bit) ||
        !fpti_read_nbits(ctx, 6, &x_val) ||
        !fpti_read_nbits(ctx, 1, &sign_y_bit) ||
        !fpti_read_nbits(ctx, 6, &y_val)) {
        return 0;
    }

    *x_out = (sign_x_bit == 1) ? -(int)x_val : (int)x_val;
    *y_out = (sign_y_bit == 1) ? -(int)y_val : (int)y_val;
    return 1;
}

// Function: fpti_read_axist
int32_t fpti_read_axist(FPTI_Context *ctx) {
    uint32_t axis_type = 0;
    if (ctx != NULL && fpti_read_check(ctx, 3) && fpti_read_nbits(ctx, 3, &axis_type)) {
        return (int)axis_type;
    }
    return 0;
}

// Function: fpti_read_ptype
int32_t fpti_read_ptype(FPTI_Context *ctx, int32_t *pixel_type_out) {
    if (ctx == NULL) {
        return 0;
    }

    uint32_t pixel_type_val = 0;
    if (fpti_read_check(ctx, 2) && fpti_read_nbits(ctx, 2, &pixel_type_val)) {
        if (pixel_type_out != NULL) {
            *pixel_type_out = (int)pixel_type_val;
        }
        ctx->pixel_type = (char)pixel_type_val;
        return 1;
    }
    return 0;
}

// Function: fpti_read_xaxis
int32_t fpti_read_xaxis(FPTI_Context *ctx) {
    uint32_t xaxis_val = 0;
    if (ctx != NULL && fpti_read_check(ctx, 6) && fpti_read_nbits(ctx, 6, &xaxis_val)) {
        return (int)xaxis_val;
    }
    return 0;
}

// Function: fpti_read_yaxis
int32_t fpti_read_yaxis(FPTI_Context *ctx) {
    uint32_t yaxis_val = 0;
    if (ctx != NULL && fpti_read_check(ctx, 6) && fpti_read_nbits(ctx, 6, &yaxis_val)) {
        return (int)yaxis_val;
    }
    return 0;
}

// Function: fpti_read_magic
int32_t fpti_read_magic(FPTI_Context *ctx) {
    uint32_t magic_val = 0;
    if (!ctx || !fpti_read_check(ctx, 0x20) || !fpti_read_nbits(ctx, 0x20, &magic_val)) {
        return 0;
    }
    return (magic_val == 0x24c7ee85) ? 1 : 0;
}

// Function: fpti_read_check
int32_t fpti_read_check(FPTI_Context *ctx, int num_bits) {
    if (ctx == NULL || (char*)ctx->data_ptr_val == NULL) {
        return 0;
    }
    if (num_bits < 0 || num_bits > 32) { // Max bits for uint32_t
        return 0;
    }
    // Check if enough bits are available in the buffer
    return (ctx->data_len * 8 >= num_bits + ctx->byte_idx * 8 + ctx->bit_idx) ? 1 : 0;
}

// Function: fpti_read_nbits
int32_t fpti_read_nbits(FPTI_Context *ctx, int num_bits, uint32_t *out_value) {
    if (out_value == NULL || num_bits < 0 || num_bits > 32 || !fpti_read_check(ctx, num_bits)) {
        return 0;
    }

    *out_value = 0; // Initialize result
    byte *data_buffer = (byte*)ctx->data_ptr_val;

    // If bits can be read from the current byte without crossing a byte boundary
    if (num_bits <= (8 - ctx->bit_idx)) {
        int shift_amount = (8 - ctx->bit_idx - num_bits);
        uint32_t mask = (1U << num_bits) - 1;
        *out_value = mask & (((uint32_t)data_buffer[ctx->byte_idx]) >> shift_amount);

        ctx->bit_idx = (ctx->bit_idx + num_bits) % 8;
        if (ctx->bit_idx == 0) {
            ctx->byte_idx++;
        }
    } else { // Read bit by bit
        for (int i = 0; i < num_bits; ++i) {
            *out_value = (*out_value << 1) |
                         (((uint32_t)data_buffer[ctx->byte_idx] >> (7 - ctx->bit_idx)) & 1U);
            ctx->bit_idx = (ctx->bit_idx + 1) % 8;
            if (ctx->bit_idx == 0) {
                ctx->byte_idx++;
            }
        }
    }
    return 1;
}

// --- Main function for testing ---
int main() {
    // Example data stream (simplified for demonstration)
    // - Magic: 0x24C7EE85 (32 bits)
    // - X-axis: 10 (6 bits)
    // - Y-axis: 5 (6 bits)
    // - PType: 1 (01) (2 bits)
    // - AType: 1 (001) (3 bits)
    // - 3 dummy check bits (1 each)
    // - Pixel 1: X=3, Y=2 (sign_x=0, x_val=3, sign_y=0, y_val=2) (1+6+1+6 = 14 bits)
    // - 1 dummy check bit (1)
    // - Pixel 2: X=7, Y=4 (sign_x=0, x_val=7, sign_y=0, y_val=4) (1+6+1+6 = 14 bits)
    // Total bits: 32 + 6 + 6 + 2 + 3 + 3 + 14 + 1 + 14 = 81 bits = 11 bytes (88 bits)

    byte dummy_data[] = {
        0x24, 0xC7, 0xEE, 0x85, // Magic number
        // Next bits: 10(X=10) | 01(Y=5) | 01(PType=1) | 001(AType=1) | 1(check) | 1(check) | 1(check)
        // 00101000 01010100 10011111 (24 bits)
        0x28, 0x54, 0x9F, // 00101000 01010100 10011111 (X=10, Y=5, PType=1, AType=1, checks)
        // Next bits: 0(sx) 000011(x=3) | 0(sy) 000010(y=2) | 1(check) | 0(sx) 000111(x=7) | 0(sy) 000100(y=4)
        // 00000110 00001010 00011100 00100xxx (29 bits)
        0x06, 0x0A, 0x1C, 0x20 // Last byte has 5 bits for 00100, remaining 3 bits are 0.
    };
    size_t dummy_data_len = sizeof(dummy_data);

    FPTI_Context ctx = {
        .data_ptr_val = (uintptr_t)dummy_data,
        .data_len = (int)dummy_data_len,
        .byte_idx = 0,
        .bit_idx = 0,
        .pixel_type = 0 // Will be set by fpti_read_ptype
    };

    printf("Attempting to display image...\n");
    if (fpti_display_img(&ctx)) {
        printf("Image displayed successfully.\n");
    } else {
        printf("Failed to display image.\n");
    }

    return 0;
}