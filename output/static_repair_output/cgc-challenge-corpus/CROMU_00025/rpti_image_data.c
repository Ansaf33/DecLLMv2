#include <stdio.h>
#include <stdlib.h> // For malloc, free
#include <string.h> // For memset, memcpy
#include <stdint.h> // For uint32_t

// Define the RPTI_FILE_HANDLE structure
typedef struct RPTI_FILE_HANDLE_STRUCT {
    char *data;         // Pointer to the raw RPTI file data in memory
    size_t size;        // Total size of the data in bytes
    size_t byte_offset; // Current byte position in the data
    int bit_offset;     // Current bit position within the current byte (0-7)
} RPTI_FILE_HANDLE;

// Function prototypes
int rpti_add_pixel(char *image_buffer, int x, int y, int axis_type, int width, int height);
int rpti_display_img(RPTI_FILE_HANDLE *handle);
int rpti_read_bits(RPTI_FILE_HANDLE *handle, int num_bits, unsigned int *out_value);
int rpti_inc_index(RPTI_FILE_HANDLE *handle, int num_bits);
int rpti_read_check(RPTI_FILE_HANDLE *handle, int num_bits);
int rpti_read_magic(RPTI_FILE_HANDLE *handle);
int rpti_read_xaxis(RPTI_FILE_HANDLE *handle);
int rpti_read_yaxis(RPTI_FILE_HANDLE *handle);
int rpti_read_initx(RPTI_FILE_HANDLE *handle, int *out_initx);
int rpti_read_inity(RPTI_FILE_HANDLE *handle, int *out_inity);
int rpti_read_axist(RPTI_FILE_HANDLE *handle);
int rpti_read_pixel(RPTI_FILE_HANDLE *handle, unsigned int *out_pixel_data);


// Function: rpti_add_pixel
int rpti_add_pixel(char *image_buffer, int x, int y, int axis_type, int width, int height) {
    if (image_buffer == NULL) {
        return 0;
    }

    int pixel_index;
    switch (axis_type) {
        default:
            return 0; // Invalid axis_type
        case 1:
            pixel_index = x - y * width;
            break;
        case 2:
            pixel_index = -y * width + x + width - 1;
            break;
        case 3:
            pixel_index = x + ((height - 1) - y) * width;
            break;
        case 4:
            pixel_index = ((height - 1) - y) * width + x + width - 1;
            break;
        case 7:
            pixel_index = x - y * width + width / 2 + (height / 2) * width;
            break;
    }

    if (pixel_index < 0 || pixel_index >= width * height) {
        printf("[ERROR] Pixel beyond image border\n");
        return 0;
    } else {
        image_buffer[pixel_index] = '.'; // Use '.' instead of 0x2e for clarity
        return 1;
    }
}

// Function: rpti_display_img
int rpti_display_img(RPTI_FILE_HANDLE *handle) {
    if (handle == NULL) {
        return 0;
    }

    int width = 0, height = 0, current_x = 0, current_y = 0, axis_type = 0;
    char *image_buffer = NULL;
    size_t image_size = 0;

    // Read header information
    if (!rpti_read_magic(handle)) {
        printf("RPTI magic fail\n");
        return 0;
    }
    width = rpti_read_xaxis(handle);
    if (width == 0) {
        printf("rpti xlen fail\n");
        return 0;
    }
    height = rpti_read_yaxis(handle);
    if (height == 0) {
        printf("rpti ylen fail\n");
        return 0;
    }
    if (!rpti_read_initx(handle, &current_x)) {
        printf("rpti initx fail\n");
        return 0;
    }
    if (!rpti_read_inity(handle, &current_y)) {
        printf("rpti inity fail\n");
        return 0;
    }
    axis_type = rpti_read_axist(handle);
    if (axis_type == 0) {
        printf("axis type fail\n");
        return 0;
    }
    if (!rpti_inc_index(handle, 3)) { // Skip 3 padding bits as indicated by original
        return 0;
    }

    // Determine image bounds based on axis type
    int min_x, max_x, min_y, max_y;
    switch (axis_type) {
        default:
            printf("type switch fail\n");
            return 0;
        case 1:
            min_x = 0;
            max_x = width - 1;
            min_y = 1 - height;
            max_y = 0;
            break;
        case 2:
            min_x = 1 - width;
            max_x = 0;
            min_y = 1 - height;
            max_y = 0;
            break;
        case 3:
            min_x = 0;
            max_x = width - 1;
            min_y = 0;
            max_y = height - 1;
            break;
        case 4:
            min_x = 1 - width;
            max_x = 0;
            min_y = 0;
            max_y = height - 1;
            break;
        case 7:
            min_x = -(width / 2);
            max_x = width / 2 - (width + 1) % 2;
            min_y = (height + 1) % 2 - height / 2;
            max_y = height / 2;
            break;
    }

    image_size = (size_t)width * height;
    image_buffer = (char *)malloc(image_size + 1); // +1 for null terminator
    if (image_buffer == NULL) {
        printf("Memory allocation failed\n");
        return 0;
    }
    memset(image_buffer, ' ', image_size); // Fill with spaces
    image_buffer[image_size] = '\0'; // Null-terminate the string

    // Read pixels and draw them
    unsigned int pixel_data;
    while (rpti_read_pixel(handle, &pixel_data)) {
        int dx_raw = (pixel_data >> 7);
        int dy_raw = (pixel_data & 0x7f);

        // Apply sign extension for 7-bit signed values
        if ((dx_raw & 0x40) != 0) { // If 7th bit (0-indexed) is set, it's negative
            dx_raw = -(dx_raw & 0x3f); // Mask out the sign bit, then negate
        }
        if ((dy_raw & 0x40) != 0) { // If 7th bit (0-indexed) is set, it's negative
            dy_raw = -(dy_raw & 0x3f); // Mask out the sign bit, then negate
        }

        int new_x = current_x + dx_raw;
        int new_y = current_y + dy_raw;

        if ((new_x < min_x) || (max_x < new_x)) {
            printf("X out of bounds (current_x: %d, dx_raw: %d, new_x: %d, min_x: %d, max_x: %d)\n", current_x, dx_raw, new_x, min_x, max_x);
            free(image_buffer);
            return 0;
        }
        if ((new_y < min_y) || (max_y < new_y)) {
            printf("Y out of bounds (current_y: %d, dy_raw: %d, new_y: %d, min_y: %d, max_y: %d)\n", current_y, dy_raw, new_y, min_y, max_y);
            free(image_buffer);
            return 0;
        }

        if (!rpti_add_pixel(image_buffer, new_x, new_y, axis_type, width, height)) {
            free(image_buffer);
            printf("add pixel fail\n");
            return 0;
        }
        current_x = new_x;
        current_y = new_y;
    }

    // Print the image
    for (size_t i = 0; i < image_size; ++i) {
        if ((i % width == 0) && (i != 0)) {
            printf("\n");
        }
        printf("%c", image_buffer[i]);
    }
    printf("\n");

    free(image_buffer);
    return 1;
}

// Function: rpti_read_bits
int rpti_read_bits(RPTI_FILE_HANDLE *handle, int num_bits, unsigned int *out_value) {
    if (out_value == NULL || handle == NULL || handle->data == NULL) {
        return 0;
    }
    if (num_bits <= 0 || num_bits > 32) { // unsigned int is typically 32-bit
        return 0;
    }

    if (!rpti_read_check(handle, num_bits)) {
        return 0;
    }

    unsigned int value = 0;
    for (int i = 0; i < num_bits; ++i) {
        char current_byte = handle->data[handle->byte_offset];
        int bit = (current_byte >> (7 - handle->bit_offset)) & 1;

        value = (value << 1) | bit; // Build value MSB first

        handle->bit_offset++;
        if (handle->bit_offset == 8) {
            handle->bit_offset = 0;
            handle->byte_offset++;
        }
    }
    *out_value = value;
    return 1;
}

// Function: rpti_inc_index
int rpti_inc_index(RPTI_FILE_HANDLE *handle, int num_bits) {
    if (handle == NULL || handle->data == NULL) {
        return 0;
    }
    if (num_bits < 0) {
        return 0;
    }

    if (!rpti_read_check(handle, num_bits)) {
        return 0;
    }

    int total_bits_to_advance = handle->bit_offset + num_bits;
    handle->byte_offset += total_bits_to_advance / 8;
    handle->bit_offset = total_bits_to_advance % 8;

    return 1;
}

// Function: rpti_read_check
int rpti_read_check(RPTI_FILE_HANDLE *handle, int num_bits) {
    if (handle == NULL || handle->data == NULL) {
        return 0;
    }
    if (num_bits < 0) {
        return 0;
    }

    // Calculate total bits remaining in the buffer
    size_t bits_remaining = (handle->size * 8) - (handle->byte_offset * 8 + handle->bit_offset);
    if (bits_remaining < (size_t)num_bits) {
        return 0;
    }
    return 1;
}

// Function: rpti_read_magic
int rpti_read_magic(RPTI_FILE_HANDLE *handle) {
    if (handle == NULL || handle->data == NULL) {
        return 0;
    }
    // Original code implies magic must be read from the very beginning.
    if (handle->byte_offset != 0 || handle->bit_offset != 0) {
        return 0;
    }

    uint32_t magic_val;
    if (!rpti_read_check(handle, 32)) { // Magic is 4 bytes = 32 bits
        return 0;
    }

    // Directly copy 4 bytes for magic, assuming host endianness matches file format.
    // Original code used memcpy, so replicating that behavior.
    memcpy(&magic_val, handle->data + handle->byte_offset, sizeof(uint32_t));

    // The magic value -0x3caef62d is 0xC35109D3.
    // Assuming little-endian system, the bytes in memory for 0xC35109D3U would be D3 09 51 C3.
    // If the file stores it as D3 09 51 C3, then `memcpy` will yield 0xC35109D3U.
    if (magic_val == 0xC35109D3U) {
        if (!rpti_inc_index(handle, 32)) { // Advance 32 bits (4 bytes)
            return 0;
        }
        return 1;
    } else {
        return 0;
    }
}

// Function: rpti_read_xaxis
int rpti_read_xaxis(RPTI_FILE_HANDLE *handle) {
    unsigned int x_axis_len = 0;
    if (!rpti_read_bits(handle, 6, &x_axis_len)) {
        return 0;
    }
    return (int)x_axis_len;
}

// Function: rpti_read_yaxis
int rpti_read_yaxis(RPTI_FILE_HANDLE *handle) {
    unsigned int y_axis_len = 0;
    if (!rpti_read_bits(handle, 6, &y_axis_len)) {
        return 0;
    }
    return (int)y_axis_len;
}

// Function: rpti_read_initx
int rpti_read_initx(RPTI_FILE_HANDLE *handle, int *out_initx) {
    if (out_initx == NULL) {
        return 0;
    }

    unsigned int sign_bit;
    if (!rpti_read_bits(handle, 1, &sign_bit)) {
        return 0;
    }

    unsigned int value;
    if (!rpti_read_bits(handle, 6, &value)) {
        return 0;
    }

    *out_initx = (int)value;
    if (sign_bit == 1) {
        *out_initx = -(*out_initx);
    }
    return 1;
}

// Function: rpti_read_inity
int rpti_read_inity(RPTI_FILE_HANDLE *handle, int *out_inity) {
    if (out_inity == NULL) {
        return 0;
    }

    unsigned int sign_bit;
    if (!rpti_read_bits(handle, 1, &sign_bit)) {
        return 0;
    }

    unsigned int value;
    if (!rpti_read_bits(handle, 6, &value)) {
        return 0;
    }

    *out_inity = (int)value;
    if (sign_bit == 1) {
        *out_inity = -(*out_inity);
    }
    return 1;
}

// Function: rpti_read_axist
int rpti_read_axist(RPTI_FILE_HANDLE *handle) {
    unsigned int axis_type = 0;
    if (!rpti_read_bits(handle, 3, &axis_type)) {
        return 0;
    }
    // Original check: (4 < local_10[0]) && (local_10[0] < 7) -> values 5, 6 are invalid
    if ((axis_type > 4) && (axis_type < 7)) {
        return 0; // These values are considered invalid
    }
    return (int)axis_type;
}

// Function: rpti_read_pixel
int rpti_read_pixel(RPTI_FILE_HANDLE *handle, unsigned int *out_pixel_data) {
    if (out_pixel_data == NULL) {
        return 0;
    }

    // Check if enough bits are available for a pixel (7 for dx, 7 for dy = 14 bits)
    if (!rpti_read_check(handle, 14)) {
        return 0; // No more pixels to read
    }

    unsigned int dx_bits;
    if (!rpti_read_bits(handle, 7, &dx_bits)) {
        return 0;
    }

    unsigned int dy_bits;
    if (!rpti_read_bits(handle, 7, &dy_bits)) {
        return 0;
    }

    *out_pixel_data = (dy_bits | (dx_bits << 7));
    return 1;
}

// Main function to demonstrate usage
int main() {
    // Example RPTI data
    // Magic: 0xC35109D3 (stored as D3 09 51 C3 in little-endian)
    // Header values (MSB-first bitstream):
    // X-len: 10 (0b001010)
    // Y-len: 10 (0b001010)
    // InitX_sign: 0, InitX_val: 5 (0b000101) -> InitX = 5
    // InitY_sign: 0, InitY_val: 5 (0b000101) -> InitY = 5
    // Axis Type: 3 (0b011)
    // Padding: 0 (0b000)
    // Combined 32-bit header stream: 00101000 10000010 10000101 01100000 -> 0x28, 0x82, 0x85, 0x60

    // Pixel data (14 bits per pixel, MSB-first bitstream for each pixel)
    // Initial current_x=5, current_y=5
    // P1: (dx=0, dy=0) -> (5,5) : 0b0000000_0000000 -> 0x0000
    // P2: (dx=1, dy=0) -> (6,5) : 0b0000001_0000000 -> 0x0080
    // P3: (dx=0, dy=1) -> (6,6) : 0b0000000_0000001 -> 0x0001
    // P4: (dx=-1, dy=0) -> (5,6) : 0b1000001_0000000 -> 0x4080 (7th bit set for negative, value 1)
    // P5: (dx=0, dy=-1) -> (5,5) : 0b0000000_1000001 -> 0x0041 (7th bit set for negative, value 1)

    unsigned char dummy_rpti_data[] = {
        // Magic: 0xC35109D3 (Little-endian byte order for memcpy)
        0xD3, 0x09, 0x51, 0xC3,
        // Header bytes (MSB-first bitstream as derived above)
        0x28, 0x82, 0x85, 0x60,
        // Pixel Data (14 bits per pixel, padded to bytes)
        // P1: (dx=0, dy=0) -> 0x0000. Bitstream: 00000000 00000000 (14 bits + 2 unused)
        0x00, 0x00,
        // P2: (dx=1, dy=0) -> 0x0080. Bitstream: 00000010 00000000 (14 bits + 2 unused)
        0x02, 0x00,
        // P3: (dx=0, dy=1) -> 0x0001. Bitstream: 00000000 00000001 (14 bits + 2 unused)
        0x00, 0x01,
        // P4: (dx=-1, dy=0) -> 0x4080. Bitstream: 10000010 00000000 (14 bits + 2 unused)
        0x82, 0x00,
        // P5: (dx=0, dy=-1) -> 0x0041. Bitstream: 00000001 00000010 (14 bits + 2 unused)
        0x01, 0x02,
    };

    RPTI_FILE_HANDLE rpti_handle = {
        .data = (char *)dummy_rpti_data,
        .size = sizeof(dummy_rpti_data),
        .byte_offset = 0,
        .bit_offset = 0
    };

    printf("Displaying RPTI image:\n");
    if (rpti_display_img(&rpti_handle)) {
        printf("RPTI image displayed successfully.\n");
    } else {
        printf("Failed to display RPTI image.\n");
    }

    return 0;
}