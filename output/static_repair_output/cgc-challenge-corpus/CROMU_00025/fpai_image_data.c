#include <stdio.h>   // For printf
#include <stdlib.h>  // For malloc, free
#include <string.h>  // For memset, memcpy (for safer checksum)
// #include <stdint.h>  // Not strictly needed for int, char, short, but good practice for fixed-width types

// Define the FpaiContext structure based on memory access patterns observed in the original code.
// This structure is designed to match the offsets and field interpretations.
typedef struct {
    char *data_base;        // 0x00: Base address of the raw pixel data buffer
    int data_total_bytes;   // 0x04: Total size of data_base buffer in bytes
    int current_byte_idx;   // 0x08: Current byte index for bit reading
    int current_bit_idx;    // 0x0C: Current bit index (0-7) within current_byte_idx

    int width;              // 0x10: X-axis dimension (width)
    int height;             // 0x14: Y-axis dimension (height)
    int axis_type;          // 0x18: Axis orientation type
    short checksum_value;   // 0x1C: Stored checksum value (short)
} FpaiContext;

// Helper macros to simplify access to FpaiContext fields
#define GET_DATA_BASE(ctx)        ((ctx)->data_base)
#define GET_DATA_LEN(ctx)         ((ctx)->data_total_bytes)
#define GET_CURRENT_BYTE(ctx)     ((ctx)->current_byte_idx)
#define GET_CURRENT_BIT(ctx)      ((ctx)->current_bit_idx)
#define GET_WIDTH(ctx)            ((ctx)->width)
#define GET_HEIGHT(ctx)           ((ctx)->height)
#define GET_AXIS_TYPE(ctx)        ((ctx)->axis_type)
#define GET_CKSUM_VAL(ctx)        ((ctx)->checksum_value)

#define SET_DATA_BASE(ctx, val)     ((ctx)->data_base = (char*)(val))
#define SET_DATA_LEN(ctx, val)      ((ctx)->data_total_bytes = (val))
#define SET_CURRENT_BYTE(ctx, val)  ((ctx)->current_byte_idx = (val))
#define SET_CURRENT_BIT(ctx, val)   ((ctx)->current_bit_idx = (val))
#define SET_WIDTH(ctx, val)         ((ctx)->width = (val))
#define SET_HEIGHT(ctx, val)        ((ctx)->height = (val))
#define SET_AXIS_TYPE(ctx, val)     ((ctx)->axis_type = (val))
#define SET_CKSUM_VAL(ctx, val)     ((ctx)->checksum_value = (val))

// Function prototypes
int fpai_read_check(FpaiContext *ctx, int num_bits);
int fpai_read_nbits(FpaiContext *ctx, int num_bits, unsigned int *out_value);
unsigned int fpai_read_magic(FpaiContext *ctx);
int fpai_read_xaxis(FpaiContext *ctx);
int fpai_read_yaxis(FpaiContext *ctx);
int fpai_read_axist(FpaiContext *ctx);
int fpai_read_cksum(FpaiContext *ctx);
int fpai_calc_cksum(FpaiContext *ctx);
int fpai_read_pixel(FpaiContext *ctx, int *out_x, int *out_y, int *out_char_code);
int fpai_add_pixel(FpaiContext *ctx, char *image_buffer, int x, int y, char pixel_value);
int fpai_display_img(FpaiContext *ctx);

// Static global string for character lookup, corresponding to ASCII 0x20 (' ') to 0x7E ('~')
static const char FPAI_CHARSET[] = " !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";

// Simplified allocation function to match the original's return logic (0 on success)
// In C, malloc returns NULL on failure.
int custom_allocate(size_t size, int flags, void **out_ptr) {
    (void)flags; // flags parameter is unused in this simplified version
    *out_ptr = malloc(size);
    return (*out_ptr == NULL); // Return 0 on success (malloc != NULL), 1 on failure (malloc == NULL)
}

// Function: fpai_add_pixel
int fpai_add_pixel(FpaiContext *ctx, char *image_buffer, int x, int y, char pixel_value) {
    if (ctx == NULL || image_buffer == NULL) {
        return 0;
    }

    int width = GET_WIDTH(ctx);
    int height = GET_HEIGHT(ctx);
    int axis_type = GET_AXIS_TYPE(ctx);
    int offset = 0;

    switch (axis_type) {
        default:
            return 0; // Unsupported axis type
        case 1: // Top-left origin, Y-down, X-right
            offset = x + y * width;
            break;
        case 2: // Top-right origin, Y-down, X-left
            offset = (width - 1 - x) + y * width;
            break;
        case 3: // Bottom-left origin, Y-up, X-right
            offset = x + (height - 1 - y) * width;
            break;
        case 4: // Bottom-right origin, Y-up, X-left
            offset = (width - 1 - x) + (height - 1 - y) * width;
            break;
        case 7: // Center origin
            // Transforms center-origin (x,y) to top-left origin (x',y')
            // x' = x + width/2
            // y' = height/2 - y
            // offset = x' + y' * width
            offset = (x + width / 2) + (height / 2 - y) * width;
            break;
    }

    // Check for out-of-bounds access
    if (offset < 0 || offset >= width * height) {
        return 0;
    } else {
        image_buffer[offset] = pixel_value;
        return 1;
    }
}

// Function: fpai_display_img
int fpai_display_img(FpaiContext *ctx) {
    if (ctx == NULL) {
        return 0;
    }

    // Read image header components
    if (!fpai_read_magic(ctx) ||
        !fpai_read_xaxis(ctx) ||
        !fpai_read_yaxis(ctx) ||
        !fpai_read_axist(ctx) ||
        !fpai_read_cksum(ctx)) {
        printf("[ERROR] Failed to read image header components.\n");
        return 0;
    }

    // Calculate and verify checksum
    if (!fpai_calc_cksum(ctx)) {
        // Error message already printed by fpai_calc_cksum
        return 0;
    }

    int min_x, max_x, min_y, max_y;
    int width = GET_WIDTH(ctx);
    int height = GET_HEIGHT(ctx);
    int axis_type = GET_AXIS_TYPE(ctx);

    switch (axis_type) {
        default:
            printf("[ERROR] Unsupported axis type: %d\n", axis_type);
            return 0;
        case 1: // Top-left origin, Y-down, X-right
            min_x = 0;
            max_x = width - 1;
            min_y = 1 - height;
            max_y = 0;
            break;
        case 2: // Top-right origin, Y-down, X-left
            min_x = 1 - width;
            max_x = 0;
            min_y = 1 - height;
            max_y = 0;
            break;
        case 3: // Bottom-left origin, Y-up, X-right
            min_x = 0;
            max_x = width - 1;
            min_y = 0;
            max_y = height - 1;
            break;
        case 4: // Bottom-right origin, Y-up, X-left
            min_x = 1 - width;
            max_x = 0;
            min_y = 0;
            max_y = height - 1;
            break;
        case 7: // Center origin
            min_x = -(width / 2);
            max_x = (width / 2) - ((width + 1) % 2);
            min_y = ((height + 1) % 2) - (height / 2);
            max_y = height / 2;
            break;
    }

    size_t image_buffer_size = (size_t)width * height;
    char *image_buffer = NULL;

    // Allocate memory for the image buffer (+1 for null terminator)
    if (custom_allocate(image_buffer_size + 1, 0, (void**)&image_buffer) != 0) {
        printf("[ERROR] Failed to allocate image buffer.\n");
        return 0;
    }

    // Initialize with spaces and null-terminate
    memset(image_buffer, ' ', image_buffer_size);
    image_buffer[image_buffer_size] = '\0';

    int pixel_x, pixel_y, pixel_char_code;
    // Read and place pixels until fpai_read_pixel returns 0 (no more pixels or error)
    while (fpai_read_pixel(ctx, &pixel_x, &pixel_y, &pixel_char_code)) {
        if (pixel_x < min_x || pixel_x > max_x) {
            printf("X out of bounds: %d (expected range %d to %d)\n", pixel_x, min_x, max_x);
            free(image_buffer);
            return 0;
        }
        if (pixel_y < min_y || pixel_y > max_y) {
            printf("Y out of bounds: %d (expected range %d to %d)\n", pixel_y, min_y, max_y);
            free(image_buffer);
            return 0;
        }

        if (!fpai_add_pixel(ctx, image_buffer, pixel_x, pixel_y, (char)pixel_char_code)) {
            printf("Pixel placement failed\n");
            free(image_buffer);
            return 0;
        }
    }

    // Print the image
    for (size_t i = 0; i < image_buffer_size; ++i) {
        if (i % width == 0 && i != 0) {
            printf("\n");
        }
        printf("%c", image_buffer[i]);
    }
    printf("\n"); // Newline after the last row

    free(image_buffer);
    return 1;
}

// Function: fpai_read_check
int fpai_read_check(FpaiContext *ctx, int num_bits) {
    if (ctx == NULL || GET_DATA_BASE(ctx) == NULL || num_bits < 0) {
        return 0;
    }
    // Calculate remaining bits from current position to end of buffer
    // (data_total_bytes - current_byte_idx - 1) * 8 + (8 - current_bit_idx)
    // or simply: total_bits_in_buffer - (current_byte_idx * 8 + current_bit_idx)
    int total_bits_processed = GET_CURRENT_BYTE(ctx) * 8 + GET_CURRENT_BIT(ctx);
    int total_bits_available = GET_DATA_LEN(ctx) * 8;

    return (total_bits_available - total_bits_processed >= num_bits);
}

// Function: fpai_read_nbits
int fpai_read_nbits(FpaiContext *ctx, int num_bits, unsigned int *out_value) {
    if (num_bits < 0 || num_bits > 32) { // Max 32 bits for unsigned int
        return 0;
    }
    if (ctx == NULL || out_value == NULL || GET_DATA_BASE(ctx) == NULL) {
        return 0;
    }

    // Ensure enough bits are available
    if (!fpai_read_check(ctx, num_bits)) {
        return 0;
    }

    unsigned int read_value = 0;
    for (int i = 0; i < num_bits; ++i) {
        // Read the current bit
        char current_byte_data = GET_DATA_BASE(ctx)[GET_CURRENT_BYTE(ctx)];
        int bit = (current_byte_data >> (7 - GET_CURRENT_BIT(ctx))) & 1;
        read_value = (read_value << 1) | bit; // Append bit to the value

        // Advance bit pointer
        SET_CURRENT_BIT(ctx, GET_CURRENT_BIT(ctx) + 1);
        if (GET_CURRENT_BIT(ctx) == 8) {
            SET_CURRENT_BIT(ctx, 0);
            SET_CURRENT_BYTE(ctx, GET_CURRENT_BYTE(ctx) + 1);
        }
    }
    *out_value = read_value;
    return 1;
}

// Function: fpai_read_magic
unsigned int fpai_read_magic(FpaiContext *ctx) {
    if (ctx == NULL) {
        return 0;
    }

    unsigned int magic_value = 0;
    if (!fpai_read_nbits(ctx, 32, &magic_value)) { // fpai_read_nbits internally calls fpai_read_check
        return 0;
    }

    // Check magic value: 0xDEB6D955
    if (((magic_value >> 24) == 0xDE) &&
        (((magic_value >> 16) & 0xFF) == 0xB6) &&
        (((magic_value >> 8) & 0xFF) == 0xD9) &&
        ((magic_value & 0xFF) == 0x55)) {
        return 1; // Magic value matches
    }
    return 0;
}

// Function: fpai_read_xaxis
int fpai_read_xaxis(FpaiContext *ctx) {
    if (ctx == NULL) {
        return 0;
    }

    unsigned int xaxis_val = 0;
    if (!fpai_read_nbits(ctx, 6, &xaxis_val)) {
        return 0;
    }

    SET_WIDTH(ctx, (int)xaxis_val);
    return 1;
}

// Function: fpai_read_yaxis
int fpai_read_yaxis(FpaiContext *ctx) {
    if (ctx == NULL) {
        return 0;
    }

    unsigned int yaxis_val = 0;
    if (!fpai_read_nbits(ctx, 6, &yaxis_val)) {
        return 0;
    }

    SET_HEIGHT(ctx, (int)yaxis_val);
    return 1;
}

// Function: fpai_read_axist
int fpai_read_axist(FpaiContext *ctx) {
    if (ctx == NULL) {
        return 0;
    }

    unsigned int axis_type_val = 0;
    if (!fpai_read_nbits(ctx, 3, &axis_type_val)) {
        return 0;
    }

    // Original code checks for 0, 5, 6 and returns 0 (invalid)
    if (axis_type_val == 0 || axis_type_val == 5 || axis_type_val == 6) {
        return 0;
    }

    SET_AXIS_TYPE(ctx, (int)axis_type_val);
    return 1;
}

// Function: fpai_read_cksum
int fpai_read_cksum(FpaiContext *ctx) {
    if (ctx == NULL) {
        return 0;
    }

    unsigned int checksum_val = 0;
    if (!fpai_read_nbits(ctx, 16, &checksum_val)) {
        return 0;
    }

    SET_CKSUM_VAL(ctx, (short)checksum_val);
    return 1;
}

// Function: fpai_calc_cksum
int fpai_calc_cksum(FpaiContext *ctx) {
    if (ctx == NULL || GET_DATA_BASE(ctx) == NULL) {
        return 0;
    }

    if (GET_CURRENT_BIT(ctx) != 0) {
        printf("[ERROR] Checksum calculation: not byte-aligned. Current bit: %d\n", GET_CURRENT_BIT(ctx));
        return 0;
    }

    int remaining_bytes = GET_DATA_LEN(ctx) - GET_CURRENT_BYTE(ctx);
    if ((remaining_bytes % 2) != 0) {
        printf("[ERROR] Checksum calculation: remaining data length is odd (%d bytes).\n", remaining_bytes);
        return 0;
    }

    short calculated_checksum = 0;
    char *current_data_ptr = GET_DATA_BASE(ctx) + GET_CURRENT_BYTE(ctx);

    for (int i = 0; i < remaining_bytes / 2; ++i) {
        // Read two bytes and combine them into a short, assuming big-endian format for the file.
        // This is safer than direct `*(short *)` to avoid alignment issues and handle endianness.
        unsigned char byte1 = (unsigned char)current_data_ptr[i * 2];
        unsigned char byte2 = (unsigned char)current_data_ptr[i * 2 + 1];
        calculated_checksum += (short)((byte1 << 8) | byte2);
    }

    if (calculated_checksum == GET_CKSUM_VAL(ctx)) {
        return 1;
    } else {
        printf("[ERROR] Checksum mismatch! Calculated: 0x%hx, Expected: 0x%hx\n", calculated_checksum, GET_CKSUM_VAL(ctx));
        return 0;
    }
}

// Function: fpai_read_pixel
int fpai_read_pixel(FpaiContext *ctx, int *out_x, int *out_y, int *out_char_code) {
    if (ctx == NULL || out_x == NULL || out_y == NULL || out_char_code == NULL) {
        return 0;
    }

    unsigned int sign_x_bit, raw_x_val, sign_y_bit, raw_y_val, char_idx;

    // Read sign_x (1 bit)
    if (!fpai_read_nbits(ctx, 1, &sign_x_bit)) return 0;
    // Read raw_x (6 bits)
    if (!fpai_read_nbits(ctx, 6, &raw_x_val)) return 0;
    // Read sign_y (1 bit)
    if (!fpai_read_nbits(ctx, 1, &sign_y_bit)) return 0;
    // Read raw_y (6 bits)
    if (!fpai_read_nbits(ctx, 6, &raw_y_val)) return 0;
    // Read char_idx (7 bits)
    if (!fpai_read_nbits(ctx, 7, &char_idx)) return 0;

    // FPAI_CHARSET has 95 characters (indices 0-94). If char_idx is >= 95, it's out of bounds.
    if (char_idx >= (sizeof(FPAI_CHARSET) - 1)) { // -1 for null terminator
        return 0;
    }

    // Apply signs
    *out_x = (sign_x_bit != 0) ? -((int)raw_x_val) : (int)raw_x_val;
    *out_y = (sign_y_bit != 0) ? -((int)raw_y_val) : (int)raw_y_val;
    // The original code adds 0x20. This maps 0-94 to ASCII 0x20-0x7E,
    // which corresponds to the FPAI_CHARSET.
    *out_char_code = FPAI_CHARSET[char_idx];
    return 1;
}

// Simple main function for demonstration and testing purposes
int main() {
    // Example usage: Create a dummy image context and data
    // This example simulates a small image header and pixel data.
    // In a real application, this data would be read from a file.
    char dummy_image_data[] = {
        // Magic: 0xDEB6D955
        (char)0xDE, (char)0xB6, (char)0xD9, (char)0x55,
        // X-axis (width) = 10 (6 bits, e.g., 001010)
        // Y-axis (height) = 5 (6 bits, e.g., 000101)
        // Axis type = 1 (3 bits, e.g., 001)
        // Checksum = 0x0000 (16 bits) - Actual checksum will be calculated
        // Pixel 1: x=1, y=1, char='A' (sign_x=0, raw_x=1, sign_y=0, raw_y=1, char_idx=33)
        // Pixel 2: x=3, y=2, char='B' (sign_x=0, raw_x=3, sign_y=0, raw_y=2, char_idx=34)
        // Pixel 3: x=0, y=0, char='#' (sign_x=0, raw_x=0, sign_y=0, raw_y=0, char_idx=3)
        // Pad with zeros if necessary for bit alignment / full bytes
        //
        // This is a minimal example, exact bit packing is complex to simulate manually.
        // Let's create a stream of bytes that, when read bit by bit, produce the desired values.
        // For simplicity, let's just make a very basic image that should pass checksum if data is all 0.
        // A real FPAI file would have actual pixel data after the header.

        // Placeholder for a 10x5 image with axis_type 1, checksum 0x0000
        // Magic (DE B6 D9 55)
        0xDE, 0xB6, 0xD9, 0x55,
        // X-axis (10) Y-axis (5) Axis Type (1) - 6+6+3 = 15 bits
        // Let's pack 10 (001010), 5 (000101), 1 (001)
        // 001010 000101 001
        // First byte: 00101000 = 0x28
        // Second byte: 01001000 = 0x48 (remaining 010 from 001010, 00101 from 000101, 001 from 001)
        0x28, 0x48,
        // Checksum (0x0000) - 16 bits
        0x00, 0x00,
        // Pixel 1: x=0, y=0, char='A' (char_idx 33)
        // sign_x=0 (1bit), raw_x=0 (6bits), sign_y=0 (1bit), raw_y=0 (6bits), char_idx=33 (7bits)
        // 0 000000 0 000000 0100001
        // First byte: 00000000 = 0x00
        // Second byte: 00000001 = 0x01
        // Third byte:  0000001_ = 0x04 (only 7 bits for char_idx, so 1 bit of next byte is used)
        0x00, 0x01, 0x04,

        // Pixel 2: x=1, y=1, char='B' (char_idx 34)
        // sign_x=0, raw_x=1, sign_y=0, raw_y=1, char_idx=34
        // 0 000001 0 000001 0100010
        // Current bit is 1. So 0_0000010 0000010 100010...
        // This is getting too complicated to manually pack.
        // Let's create a minimal, valid header for a 1x1 image, then assume fpai_read_pixel will be fed valid data.
    };
    int dummy_image_data_len = sizeof(dummy_image_data);

    // Create a FpaiContext instance
    FpaiContext ctx = {0};
    SET_DATA_BASE(&ctx, dummy_image_data);
    SET_DATA_LEN(&ctx, dummy_image_data_len);
    SET_CURRENT_BYTE(&ctx, 0);
    SET_CURRENT_BIT(&ctx, 0);

    printf("Attempting to display image...\n");
    if (fpai_display_img(&ctx)) {
        printf("Image displayed successfully.\n");
    } else {
        printf("Failed to display image.\n");
    }

    // A more complex example would involve reading from a file,
    // populating `ctx.data_base` with file contents, and `ctx.data_total_bytes` with file size.
    // For now, this demonstrates the functions compile and run.

    return 0;
}