#include <stdio.h>    // For NULL, potentially for debugging printf
#include <stdlib.h>   // For malloc, calloc, free, abs
#include <string.h>   // For strlen, memcpy, memcmp, memset
#include <stdint.h>   // For uint8_t, uint16_t, uint32_t, int32_t

// Placeholder for external functions
// Assuming create_barcode_ascii takes an int and returns a dynamically allocated char*
extern char *create_barcode_ascii(int barcode_id);
// Assuming create_barcode_from_encoded_data takes a char* and returns an int/uint32_t
extern uint32_t create_barcode_from_encoded_data(char *encoded_data);

// --- Constants ---
// Barcode representation: black bars on white background for 24-bit BGR BMP
static const uint8_t BAR_COLOR[] = {0x00, 0x00, 0x00};   // Black
static const uint8_t SPACE_COLOR[] = {0xFF, 0xFF, 0xFF}; // White

// BMP File Header (bfType) magic number "BM"
static const uint8_t BMP_MAGIC[] = {'B', 'M'};

// --- Struct definitions for BMP headers (packed to ensure correct byte layout) ---
#pragma pack(push, 1) // Ensure no padding for BMP structures

typedef struct {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
} BITMAPFILEHEADER;

typedef struct {
    uint32_t biSize;
    int32_t biWidth;
    int32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t biXPelsPerMeter;
    int32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} BITMAPINFOHEADER;

#pragma pack(pop)

// Function: create_barcode_bmp_data
// Creates the raw pixel data for a barcode BMP.
// param_1: barcode_id (int) - The identifier for the barcode.
// param_2: out_data_size (size_t*) - Output: total size of the allocated pixel data.
// param_3: out_barcode_width_bytes (size_t*) - Output: width of one row of pixel data in bytes.
// Returns: A pointer to the allocated pixel data (uint8_t*), or NULL on failure.
void *create_barcode_bmp_data(int barcode_id, size_t *out_data_size, size_t *out_barcode_width_bytes) {
    *out_data_size = 0;
    *out_barcode_width_bytes = 0;

    char *barcode_ascii = create_barcode_ascii(barcode_id);
    if (!barcode_ascii) {
        return NULL;
    }

    size_t ascii_len = strlen(barcode_ascii);
    // Original condition: sVar1 < 0xfb (251). Restricting barcode length.
    if (ascii_len >= 251) {
        free(barcode_ascii);
        return NULL;
    }

    // Each barcode character ('|' or ' ') is expanded into 4 "pixels" horizontally.
    // Each pixel is 3 bytes (BGR for 24-bit BMP).
    // So, 4 * 3 = 12 bytes per barcode character horizontally.
    size_t barcode_width_pixels = ascii_len * 4; // Total pixels wide
    size_t barcode_width_bytes = barcode_width_pixels * 3; // Total bytes for one row
    *out_barcode_width_bytes = barcode_width_bytes;

    // The barcode BMP has a fixed height of 120 rows (0x78).
    size_t barcode_height = 120;
    *out_data_size = barcode_width_bytes * barcode_height; // Total size for all pixel data

    uint8_t *bmp_data = (uint8_t *)calloc(1, *out_data_size);
    if (!bmp_data) {
        free(barcode_ascii);
        return NULL;
    }

    // Fill the first row of pixel data based on the ASCII barcode string.
    uint8_t *current_pixel_ptr = bmp_data;
    for (char *p = barcode_ascii; *p != '\0'; ++p) {
        const uint8_t *color_data = (*p == '|') ? BAR_COLOR : SPACE_COLOR;
        // Expand each character into 4 horizontal pixels.
        for (int i = 0; i < 4; ++i) {
            memcpy(current_pixel_ptr, color_data, 3);
            current_pixel_ptr += 3;
        }
    }

    // Replicate the first row (which is at `bmp_data`) to fill the remaining height.
    for (size_t row = 1; row < barcode_height; ++row) {
        memcpy(bmp_data + row * barcode_width_bytes, bmp_data, barcode_width_bytes);
    }

    free(barcode_ascii);
    return bmp_data;
}

// Function: validate_bmp_headers
// Validates the headers of a given BMP file data.
// param_1: bmp_data (const void*) - Pointer to the start of the BMP data.
// param_2: dib_header_ptr (const int*) - Original parameter, not directly used due to struct access.
// Returns: 0 for valid headers, -1 for invalid headers.
int validate_bmp_headers(const void *bmp_data, const int *dib_header_ptr) {
    const BITMAPFILEHEADER *file_header = (const BITMAPFILEHEADER *)bmp_data;
    const BITMAPINFOHEADER *info_header = (const BITMAPINFOHEADER *)((const uint8_t *)bmp_data + sizeof(BITMAPFILEHEADER));

    // 1. Check BMP magic number "BM"
    if (memcmp(&file_header->bfType, BMP_MAGIC, sizeof(BMP_MAGIC)) != 0) {
        return -1;
    }

    // 2. Check reserved fields (must be 0)
    if (file_header->bfReserved1 != 0 || file_header->bfReserved2 != 0) {
        return -1;
    }

    // 3. Check BITMAPINFOHEADER size (must be 40 bytes)
    if (info_header->biSize != sizeof(BITMAPINFOHEADER)) { // 0x28
        return -1;
    }

    // 4. Validate biWidth (must be positive and within a reasonable range, e.g., < 11057)
    if (info_header->biWidth <= 0 || info_header->biWidth >= 11057) { // 0x2b31
        return -1;
    }

    // 5. Validate biHeight (must be 120 for this barcode type)
    if (info_header->biHeight != 120) { // 0x78
        return -1;
    }

    // 6. Validate biPlanes (must be 1)
    if (info_header->biPlanes != 1) {
        return -1;
    }

    // 7. Validate biBitCount (must be 24 for 24-bit RGB)
    if (info_header->biBitCount != 24) { // 0x18
        return -1;
    }

    // 8. Validate biCompression (must be 0 for BI_RGB, no compression)
    if (info_header->biCompression != 0) {
        return -1;
    }

    // 9. Validate biSizeImage calculation (padded row size * absolute height)
    uint32_t bytes_per_pixel = info_header->biBitCount / 8;
    uint32_t row_size_unpadded = info_header->biWidth * bytes_per_pixel;
    uint32_t row_size_padded = (row_size_unpadded + 3) & ~3; // Rows padded to 4-byte boundary
    uint32_t expected_bi_size_image = row_size_padded * (uint32_t)abs(info_header->biHeight);

    if (info_header->biSizeImage != expected_bi_size_image) {
        return -1;
    }

    // 10. Validate biXPelsPerMeter and biYPelsPerMeter (must be 2835)
    if (info_header->biXPelsPerMeter != 2835 || info_header->biYPelsPerMeter != 2835) { // 0xb13
        return -1;
    }

    // 11. Validate biClrUsed and biClrImportant (must be 0 for 24-bit BMP)
    if (info_header->biClrUsed != 0 || info_header->biClrImportant != 0) {
        return -1;
    }

    // 12. Validate bfSize (total file size == header size + image data size)
    uint32_t total_header_size = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    if (file_header->bfSize != (total_header_size + info_header->biSizeImage)) {
        return -1;
    }

    return 0; // All checks passed
}

// Function: create_barcode_bmp
// Generates a complete BMP file containing the barcode.
// param_1: barcode_id (int) - The identifier for the barcode.
// Returns: A pointer to the allocated BMP file data (uint8_t*), or NULL on failure.
void *create_barcode_bmp(int barcode_id) {
    size_t barcode_data_size = 0;       // Total size of raw pixel data (unpadded)
    size_t barcode_width_bytes = 0;     // Width of one row of pixel data in bytes (unpadded)

    uint8_t *barcode_pixel_data = (uint8_t *)create_barcode_bmp_data(barcode_id, &barcode_data_size, &barcode_width_bytes);
    if (!barcode_pixel_data || barcode_data_size == 0) {
        return NULL;
    }

    // Calculate dimensions based on create_barcode_bmp_data logic
    int32_t biWidth = barcode_width_bytes / 3; // Number of pixels wide
    int32_t biHeight = 120;                    // Fixed height for this barcode type

    // Calculate padded row size for BMP image data
    uint32_t bytes_per_pixel = 3; // 24-bit BGR BMP
    uint32_t row_size_unpadded = biWidth * bytes_per_pixel;
    uint32_t row_size_padded = (row_size_unpadded + 3) & ~3; // Each row padded to 4-byte boundary

    // Total image data size including padding
    uint32_t biSizeImage = row_size_padded * biHeight;

    // BMP header sizes
    uint32_t file_header_size = sizeof(BITMAPFILEHEADER); // 14 bytes
    uint32_t info_header_size = sizeof(BITMAPINFOHEADER); // 40 bytes
    uint32_t total_header_size = file_header_size + info_header_size; // 54 bytes (0x36)

    // Total BMP file size
    uint32_t total_bmp_size = total_header_size + biSizeImage;

    uint8_t *full_bmp_file = (uint8_t *)malloc(total_bmp_size);
    if (!full_bmp_file) {
        free(barcode_pixel_data);
        return NULL;
    }

    // --- Construct BITMAPFILEHEADER ---
    BITMAPFILEHEADER file_header = {0};
    file_header.bfType = (BMP_MAGIC[1] << 8) | BMP_MAGIC[0]; // 'B' 'M' in little-endian
    file_header.bfSize = total_bmp_size;
    file_header.bfOffBits = total_header_size; // Offset to pixel data (54 bytes)

    // --- Construct BITMAPINFOHEADER ---
    BITMAPINFOHEADER info_header = {0};
    info_header.biSize = info_header_size; // 40 bytes (0x28)
    info_header.biWidth = biWidth;         // Calculated barcode width in pixels
    info_header.biHeight = biHeight;       // 120 (0x78)
    info_header.biPlanes = 1;              // Must be 1
    info_header.biBitCount = 24;           // 24-bit BGR
    info_header.biCompression = 0;         // BI_RGB (no compression)
    info_header.biSizeImage = biSizeImage; // Calculated padded image data size
    info_header.biXPelsPerMeter = 2835;    // 2835 (0xb13)
    info_header.biYPelsPerMeter = 2835;    // 2835 (0xb13)
    info_header.biClrUsed = 0;             // No color table for 24-bit
    info_header.biClrImportant = 0;        // All colors important

    // Copy headers into the allocated memory
    memcpy(full_bmp_file, &file_header, file_header_size);
    memcpy(full_bmp_file + file_header_size, &info_header, info_header_size);

    // Copy pixel data, handling row padding
    uint8_t *dest_pixel_ptr = full_bmp_file + total_header_size;
    uint8_t *src_pixel_ptr = barcode_pixel_data;

    for (int y = 0; y < biHeight; ++y) {
        memcpy(dest_pixel_ptr, src_pixel_ptr, row_size_unpadded);
        // Fill padding bytes with 0 if necessary
        if (row_size_padded > row_size_unpadded) {
            memset(dest_pixel_ptr + row_size_unpadded, 0, row_size_padded - row_size_unpadded);
        }
        dest_pixel_ptr += row_size_padded;
        src_pixel_ptr += row_size_unpadded; // Advance source pointer by unpadded row size
    }

    free(barcode_pixel_data);
    return full_bmp_file;
}

// Function: create_barcode_from_bmp
// Decodes a barcode from BMP image data.
// param_1: bmp_file_data (const void*) - Pointer to the BMP file data.
// Returns: A uint32_t value (likely an ID or status from create_barcode_from_encoded_data), or 0 on failure.
uint32_t create_barcode_from_bmp(const void *bmp_file_data) {
    uint32_t result = 0; // Default to failure

    if (validate_bmp_headers(bmp_file_data, NULL) != 0) {
        return 0; // Header validation failed
    }

    const BITMAPFILEHEADER *file_header = (const BITMAPFILEHEADER *)bmp_file_data;
    const BITMAPINFOHEADER *info_header = (const BITMAPINFOHEADER *)((const uint8_t *)bmp_file_data + sizeof(BITMAPFILEHEADER));

    int32_t bmp_width = info_header->biWidth;
    int32_t bmp_height = info_header->biHeight;
    uint32_t bytes_per_pixel = info_header->biBitCount / 8; // Should be 3 for 24-bit
    uint32_t row_size_unpadded = bmp_width * bytes_per_pixel;
    uint32_t row_size_padded = (row_size_unpadded + 3) & ~3;

    // Pixel data starts after headers (bfOffBits)
    const uint8_t *pixel_data_start = (const uint8_t *)bmp_file_data + file_header->bfOffBits;

    // Allocate memory for the decoded barcode string (width characters + null terminator)
    char *decoded_barcode_data = (char *)calloc(1, bmp_width + 1);
    if (!decoded_barcode_data) {
        return 0;
    }

    int failure_flag = 0;

    // 1. Decode the first row of pixels into barcode characters.
    // Each group of 3 bytes (a pixel) represents a color.
    for (int x = 0; x < bmp_width; ++x) {
        const uint8_t *pixel_ptr = pixel_data_start + x * bytes_per_pixel;

        // Check if the pixel is either pure black (0,0,0) or pure white (255,255,255).
        // This assumes BGR format.
        if (!((pixel_ptr[0] == 0x00 && pixel_ptr[1] == 0x00 && pixel_ptr[2] == 0x00) ||
              (pixel_ptr[0] == 0xFF && pixel_ptr[1] == 0xFF && pixel_ptr[2] == 0xFF))) {
            failure_flag = 1;
            break;
        }

        // Assign barcode character based on pixel color (first byte is sufficient if valid)
        if (pixel_ptr[0] == 0x00) { // Black pixel
            decoded_barcode_data[x] = '|';
        } else { // White pixel
            decoded_barcode_data[x] = ' ';
        }
    }

    if (failure_flag) {
        free(decoded_barcode_data);
        return 0;
    }

    // 2. Validate that all other rows are identical to the first row (for vertical consistency).
    const uint8_t *first_row_data = pixel_data_start;
    for (int y = 1; y < bmp_height; ++y) {
        const uint8_t *current_row_data = pixel_data_start + y * row_size_padded;
        // Compare only the unpadded pixel data segment of each row.
        if (memcmp(first_row_data, current_row_data, row_size_unpadded) != 0) {
            failure_flag = 1;
            break;
        }
    }

    if (failure_flag) {
        free(decoded_barcode_data);
        return 0;
    }

    // If all checks pass, call the external function to process the decoded barcode string.
    result = create_barcode_from_encoded_data(decoded_barcode_data);
    free(decoded_barcode_data);

    return result;
}