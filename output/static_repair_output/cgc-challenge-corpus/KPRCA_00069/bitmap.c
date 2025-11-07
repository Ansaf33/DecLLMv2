#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

// --- Dummy external functions (replace with actual implementations if available) ---
// Assumed to return a dynamically allocated string, which should be freed by the caller.
char *create_barcode_ascii(int barcode_id) {
    // Example implementation:
    // For barcode_id = 1, return "|| | | ||";
    // For barcode_id = 2, return "| | || | |";
    // For other, return NULL;
    if (barcode_id == 1) {
        return strdup("|| | | ||");
    } else if (barcode_id == 2) {
        return strdup("| | || | |");
    }
    return NULL; // Return NULL on failure
}

// Assumed to process the decoded barcode string and return an integer status.
// 0 for success, non-zero for failure.
int create_barcode_from_encoded_data(char *encoded_data) {
    // Example implementation: just print and return success
    if (encoded_data) {
        printf("Decoded barcode data: %s\n", encoded_data);
        return 0; // Success
    }
    return -1; // Failure
}

// --- Static data definitions from decompiled code ---
// DAT_00017049: BMP file signature "BM"
const char BMP_SIGNATURE[2] = {'B', 'M'};

// DAT_00017041: 3 bytes for '|' (black pixel in BGR format)
const uint8_t BARCODE_BLACK_PIXEL[3] = {0x00, 0x00, 0x00};

// DAT_00017045: 3 bytes for other (space, white pixel in BGR format)
const uint8_t BARCODE_WHITE_PIXEL[3] = {0xFF, 0xFF, 0xFF};

// --- BMP Header Structures ---
#pragma pack(push, 1) // Ensure no padding between struct members
typedef struct {
    uint16_t bfType;      // Signature "BM"
    uint32_t bfSize;      // File size in bytes
    uint16_t bfReserved1; // Reserved, must be 0
    uint16_t bfReserved2; // Reserved, must be 0
    uint32_t bfOffBits;   // Offset to start of image data
} BITMAPFILEHEADER;

typedef struct {
    uint32_t biSize;          // Size of this header (40 bytes)
    int32_t  biWidth;         // Image width in pixels
    int32_t  biHeight;        // Image height in pixels
    uint16_t biPlanes;        // Number of color planes (must be 1)
    uint16_t biBitCount;      // Bits per pixel (e.g., 24)
    uint32_t biCompression;   // Compression method (0 = BI_RGB)
    uint32_t biSizeImage;     // Size of raw image data (including padding)
    int32_t  biXPelsPerMeter; // Horizontal resolution
    int32_t  biYPelsPerMeter; // Vertical resolution
    uint32_t biClrUsed;       // Number of colors in palette
    uint32_t biClrImportant;  // Number of important colors
} BITMAPINFOHEADER;
#pragma pack(pop)

// Function: create_barcode_bmp_data
// Generates the raw pixel data for a barcode BMP image.
// barcode_id: Identifier for the barcode content.
// out_bmp_data_size: Output parameter for the total size of the generated pixel data.
// out_barcode_line_length_bytes: Output parameter for the byte length of a single barcode line.
// Returns a pointer to the allocated raw pixel data, or NULL on failure.
void *create_barcode_bmp_data(int barcode_id, size_t *out_bmp_data_size, size_t *out_barcode_line_length_bytes) {
    *out_bmp_data_size = 0;
    *out_barcode_line_length_bytes = 0;

    char *barcode_ascii_data = create_barcode_ascii(barcode_id);
    if (!barcode_ascii_data) {
        return NULL;
    }

    size_t ascii_len = strlen(barcode_ascii_data);

    // The original code had a check `sVar1 < 0xfb` (251).
    // If the ASCII string length is 251 or more, it's considered too long.
    if (ascii_len >= 0xfb) {
        free(barcode_ascii_data);
        return NULL;
    }

    // A barcode character expands to 4 pixels. Each pixel is 3 bytes (24-bit BGR).
    // So, one line of barcode data (in bytes) = ascii_len * 4 pixels * 3 bytes/pixel.
    *out_barcode_line_length_bytes = ascii_len * 12; // 0xc = 12
    const int IMAGE_HEIGHT_PIXELS = 0x78; // 120 pixels

    // Total raw BMP data size = barcode_line_length_bytes * image_height.
    *out_bmp_data_size = *out_barcode_line_length_bytes * IMAGE_HEIGHT_PIXELS; // 0x5a0 = 1440 = 12 * 120

    uint8_t *bmp_data = (uint8_t *)calloc(1, *out_bmp_data_size);
    if (!bmp_data) {
        free(barcode_ascii_data);
        return NULL;
    }

    uint8_t *current_write_ptr = bmp_data;
    const uint8_t *pixel_pattern;

    // Generate the first barcode line's pixel data
    for (char *c = barcode_ascii_data; *c != '\0'; ++c) {
        pixel_pattern = (*c == '|') ? BARCODE_BLACK_PIXEL : BARCODE_WHITE_PIXEL;

        // Each character expands to 4 pixels
        for (int i = 0; i < 4; ++i) {
            memcpy(current_write_ptr, pixel_pattern, 3); // 3 bytes per pixel (BGR)
            current_write_ptr += 3;
        }
    }

    // Duplicate the first barcode line to fill the remaining 119 lines (total 120)
    for (int i = 1; i < IMAGE_HEIGHT_PIXELS; ++i) {
        memcpy(bmp_data + (i * (*out_barcode_line_length_bytes)), bmp_data, *out_barcode_line_length_bytes);
    }

    free(barcode_ascii_data);
    return bmp_data;
}

// Function: validate_bmp_headers
// Validates the headers of a given BMP file data.
// bmp_file_data: Pointer to the start of the BMP file in memory.
// Returns 0 on success, -1 on failure.
int validate_bmp_headers(const void *bmp_file_data) {
    if (!bmp_file_data) {
        return -1;
    }

    const BITMAPFILEHEADER *bfh = (const BITMAPFILEHEADER *)bmp_file_data;
    const BITMAPINFOHEADER *bih = (const BITMAPINFOHEADER *)((const char *)bmp_file_data + sizeof(BITMAPFILEHEADER));

    // --- Validate BITMAPFILEHEADER ---
    if (memcmp(&bfh->bfType, BMP_SIGNATURE, 2) != 0) return -1; // "BM" signature
    if (bfh->bfReserved1 != 0 || bfh->bfReserved2 != 0) return -1;
    if (bfh->bfOffBits != sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)) return -1; // Expected offset to pixel data (0x36)

    // --- Validate BITMAPINFOHEADER ---
    if (bih->biSize != sizeof(BITMAPINFOHEADER)) return -1; // Expected DIB header size (0x28)
    if (bih->biWidth <= 0 || bih->biWidth >= 0x2b31) return -1; // Width check (< 11057)
    if (bih->biHeight != 0x78) return -1; // Height check (== 120)
    if (bih->biPlanes != 1) return -1;
    if (bih->biBitCount != 0x18) return -1; // 24 bits per pixel
    if (bih->biCompression != 0) return -1; // BI_RGB (no compression)

    // Calculate expected biSizeImage, considering 4-byte scanline padding
    uint32_t bytes_per_pixel = bih->biBitCount / 8;
    uint32_t scanline_width_bytes_padded = (bih->biWidth * bytes_per_pixel + 3) & ~3;
    uint32_t expected_biSizeImage = scanline_width_bytes_padded * bih->biHeight;

    if (bih->biSizeImage != expected_biSizeImage) return -1;

    // Check total file size consistency
    if (bfh->bfSize != sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bih->biSizeImage) return -1;

    // Specific resolution checks (0xb13 = 2835)
    if (bih->biXPelsPerMeter != 0xb13 || bih->biYPelsPerMeter != 0xb13) return -1;

    // Color palette checks (should be 0 for 24-bit BMP)
    if (bih->biClrUsed != 0 || bih->biClrImportant != 0) return -1;

    return 0; // Success
}

// Function: create_barcode_bmp
// Creates a complete BMP file in memory for a given barcode ID.
// barcode_id: Identifier for the barcode content.
// Returns a pointer to the allocated BMP file data, or NULL on failure.
void *create_barcode_bmp(int barcode_id) {
    size_t barcode_bmp_data_size = 0;
    size_t barcode_line_length_bytes = 0;

    // Get the raw pixel data for the barcode image
    uint8_t *raw_bmp_data = (uint8_t *)create_barcode_bmp_data(barcode_id, &barcode_bmp_data_size, &barcode_line_length_bytes);

    if (!raw_bmp_data || barcode_bmp_data_size == 0) {
        return NULL;
    }

    // Calculate total BMP file size
    const size_t TOTAL_HEADER_SIZE = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER); // 0x36
    size_t total_bmp_file_size = TOTAL_HEADER_SIZE + barcode_bmp_data_size;

    uint8_t *bmp_file = (uint8_t *)malloc(total_bmp_file_size);
    if (!bmp_file) {
        free(raw_bmp_data);
        return NULL;
    }

    BITMAPFILEHEADER *bfh = (BITMAPFILEHEADER *)bmp_file;
    BITMAPINFOHEADER *bih = (BITMAPINFOHEADER *)(bmp_file + sizeof(BITMAPFILEHEADER));

    // --- Fill BITMAPFILEHEADER ---
    memcpy(&bfh->bfType, BMP_SIGNATURE, 2);
    bfh->bfSize = (uint32_t)total_bmp_file_size;
    bfh->bfReserved1 = 0;
    bfh->bfReserved2 = 0;
    bfh->bfOffBits = (uint32_t)TOTAL_HEADER_SIZE;

    // --- Fill BITMAPINFOHEADER ---
    bih->biSize = sizeof(BITMAPINFOHEADER); // 0x28
    bih->biWidth = (int32_t)(barcode_line_length_bytes / 3); // Width in pixels
    bih->biHeight = 0x78; // Height in pixels (120)
    bih->biPlanes = 1;
    bih->biBitCount = 0x18; // 24 bits per pixel
    bih->biCompression = 0; // BI_RGB (no compression)
    bih->biSizeImage = (uint32_t)barcode_bmp_data_size; // Raw image data size
    bih->biXPelsPerMeter = 0xb13; // 2835
    bih->biYPelsPerMeter = 0xb13; // 2835
    bih->biClrUsed = 0;
    bih->biClrImportant = 0;

    // Copy the raw pixel data after the headers
    memcpy(bmp_file + bfh->bfOffBits, raw_bmp_data, barcode_bmp_data_size);

    free(raw_bmp_data);
    return bmp_file;
}

// Function: create_barcode_from_bmp
// Decodes a barcode from BMP file data.
// bmp_file_data: Pointer to the start of the BMP file in memory.
// Returns 0 on success (barcode processed), or non-zero on failure.
int create_barcode_from_bmp(const void *bmp_file_data) {
    if (validate_bmp_headers(bmp_file_data) != 0) {
        return 0; // Validation failed, return 0 as per original (meaning no barcode created)
    }

    const BITMAPINFOHEADER *bih = (const BITMAPINFOHEADER *)((const char *)bmp_file_data + sizeof(BITMAPFILEHEADER));
    const uint8_t *image_data_start = (const uint8_t *)bmp_file_data + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    uint32_t bytes_per_pixel = bih->biBitCount / 8; // Should be 3 for 24-bit
    uint32_t scanline_width_pixels = bih->biWidth;
    uint32_t scanline_width_bytes_padded = (scanline_width_pixels * bytes_per_pixel + 3) & ~3; // Padded to 4-byte boundary

    // Each character in the original ASCII barcode expanded to 4 pixels.
    // So, the length of the original ASCII barcode is `scanline_width_pixels / 4`.
    size_t barcode_ascii_len = scanline_width_pixels / 4;
    char *decoded_barcode = (char *)calloc(barcode_ascii_len + 1, sizeof(char)); // +1 for null terminator
    if (!decoded_barcode) {
        return 0;
    }

    bool decode_success = true;
    const uint32_t char_block_bytes = 4 * bytes_per_pixel; // Size of a 4-pixel barcode character block (12 bytes)

    // Iterate through the first scanline to decode the barcode
    for (size_t block_idx = 0; block_idx < barcode_ascii_len; ++block_idx) {
        const uint8_t *current_block_start = image_data_start + (block_idx * char_block_bytes);
        uint8_t first_byte_of_block = current_block_start[0];

        // Verify that all bytes within this 12-byte block are identical
        for (uint32_t i = 0; i < char_block_bytes; ++i) {
            if (current_block_start[i] != first_byte_of_block) {
                decode_success = false;
                break;
            }
        }
        if (!decode_success) break;

        // Determine if the block represents a black '|' or white ' ' character
        if (first_byte_of_block == BARCODE_BLACK_PIXEL[0]) {
            decoded_barcode[block_idx] = '|';
        } else if (first_byte_of_block == BARCODE_WHITE_PIXEL[0]) {
            decoded_barcode[block_idx] = ' ';
        } else {
            // Block is neither pure black nor pure white
            decode_success = false;
            break;
        }
    }

    if (!decode_success) {
        free(decoded_barcode);
        return 0;
    }

    // Verify that all image rows are identical to the first row (as expected for this barcode type)
    for (int row = 1; row < bih->biHeight; ++row) {
        const uint8_t *current_row_data = image_data_start + (row * scanline_width_bytes_padded);
        if (memcmp(current_row_data, image_data_start, scanline_width_bytes_padded) != 0) {
            decode_success = false;
            break;
        }
    }

    if (!decode_success) {
        free(decoded_barcode);
        return 0;
    }

    // Call the external function to process the decoded barcode data
    int result = create_barcode_from_encoded_data(decoded_barcode);
    free(decoded_barcode);
    return result;
}