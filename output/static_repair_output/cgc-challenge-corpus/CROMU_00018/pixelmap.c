#include <stdint.h> // For uint8_t, uint16_t, uint32_t, int32_t
#include <stdlib.h> // For malloc, size_t
#include <string.h> // For memcpy

// Define packed structs for BMP header to reduce intermediate variables
// and ensure correct byte alignment for writing to file.
// __attribute__((__packed__)) is a GCC extension commonly used on Linux.
typedef struct __attribute__((__packed__)) {
    uint16_t bfType;        // BM (0x4D42)
    uint32_t bfSize;        // File size in bytes
    uint16_t bfReserved1;   // 0
    uint16_t bfReserved2;   // 0
    uint32_t bfOffBits;     // Offset to pixel data (54 bytes)
} BMPFileHeader;

typedef struct __attribute__((__packed__)) {
    uint32_t biSize;            // Info header size (40 bytes)
    int32_t  biWidth;           // Width in pixels
    int32_t  biHeight;          // Height in pixels (positive for bottom-up DIB)
    uint16_t biPlanes;          // 1
    uint16_t biBitCount;        // 24 bits for RGB
    uint32_t biCompression;     // 0 (BI_RGB, no compression)
    uint32_t biSizeImage;       // Image size in bytes (can be 0 for BI_RGB)
    int32_t  biXPelsPerMeter;   // X pixels per meter
    int32_t  biYPelsPerMeter;   // Y pixels per meter
    uint32_t biClrUsed;         // Number of colors in palette
    uint32_t biClrImportant;    // Number of important colors
} BMPInfoHeader;

// Function: pm_write_u32
// Writes a 32-bit unsigned integer to a buffer at the current offset
// and increments the offset. Returns the number of bytes written.
uint32_t pm_write_u32(void* buffer, size_t* offset, uint32_t value) {
  *(uint32_t*)((uint8_t*)buffer + *offset) = value;
  *offset += 4;
  return 4;
}

// Function: pm_write_u16
// Writes a 16-bit unsigned integer to a buffer at the current offset
// and increments the offset. Returns the number of bytes written.
uint32_t pm_write_u16(void* buffer, size_t* offset, uint16_t value) {
  *(uint16_t*)((uint8_t*)buffer + *offset) = value;
  *offset += 2;
  return 2;
}

// Function: pm_write_u8
// Writes an 8-bit unsigned integer to a buffer at the current offset
// and increments the offset. Returns the number of bytes written.
uint32_t pm_write_u8(void* buffer, size_t* offset, uint8_t value) {
  *(uint8_t*)((uint8_t*)buffer + *offset) = value;
  *offset += 1;
  return 1;
}

// Function: pixelmap_write_file
// Writes pixel data to a BMP file format in a dynamically allocated buffer.
// out_buffer_ptr: Pointer to a void* where the allocated buffer will be stored.
// out_buffer_size_ptr: Pointer to a size_t where the size of the allocated buffer will be stored.
// pixel_data: Pointer to the raw pixel data (RGB format, 24-bit).
// width: Width of the image in pixels.
// height: Height of the image in pixels.
int pixelmap_write_file(void **out_buffer_ptr, size_t *out_buffer_size_ptr, const uint8_t *pixel_data, uint32_t width, uint32_t height) {
  size_t current_offset;
  void *buffer_start;

  // Error checks
  if ((width == 0) || (height == 0) || (pixel_data == NULL)) {
    return -1;
  }

  // Calculate image sizes
  uint32_t row_size_bytes_unpadded = width * 3;
  // BMP rows must be padded to a multiple of 4 bytes
  uint32_t row_size_bytes_padded = (row_size_bytes_unpadded + 3) & ~3;
  
  uint32_t image_size_bytes = row_size_bytes_padded * height;
  uint32_t total_file_size = image_size_bytes + sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);

  // Allocate memory
  buffer_start = malloc(total_file_size);
  if (buffer_start == NULL) {
      return -1; // Memory allocation failed
  }
  *out_buffer_ptr = buffer_start;
  *out_buffer_size_ptr = total_file_size;

  // Initialize offset for writing
  current_offset = 0;

  // Populate and write BMP File Header
  BMPFileHeader file_header = {
      .bfType = 0x4D42, // 'BM'
      .bfSize = total_file_size,
      .bfReserved1 = 0,
      .bfReserved2 = 0,
      .bfOffBits = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) // 54 bytes
  };
  memcpy((uint8_t*)buffer_start + current_offset, &file_header, sizeof(BMPFileHeader));
  current_offset += sizeof(BMPFileHeader);

  // Populate and write BMP Info Header
  BMPInfoHeader info_header = {
      .biSize = sizeof(BMPInfoHeader), // 40 bytes
      .biWidth = (int32_t)width,
      .biHeight = (int32_t)height, // Positive for bottom-up DIB
      .biPlanes = 1,
      .biBitCount = 24, // 24-bit RGB
      .biCompression = 0, // BI_RGB
      .biSizeImage = image_size_bytes, // Can be 0 for BI_RGB
      .biXPelsPerMeter = 0x0B13, // 2835 pixels/meter (72 DPI)
      .biYPelsPerMeter = 0x0B13, // 2835 pixels/meter (72 DPI)
      .biClrUsed = 0,
      .biClrImportant = 0
  };
  memcpy((uint8_t*)buffer_start + current_offset, &info_header, sizeof(BMPInfoHeader));
  current_offset += sizeof(BMPInfoHeader);

  // Write pixel data (BMP stores pixels bottom-up, BGR format)
  // The input pixel_data is assumed to be top-down, RGB.
  // We iterate from the last row of the input image to the first.
  for (uint32_t y = height; y > 0; --y) {
    size_t row_bytes_written_this_row = 0;
    // Calculate the start of the current row in the *input* pixel_data
    // Input is top-down, so row (y-1) corresponds to the (height-y)th row from top.
    // Since BMP is bottom-up, we write row (y-1) from input to the current row in BMP.
    const uint8_t *input_row_start = pixel_data + (y - 1) * width * 3;

    for (uint32_t x = 0; x < width; ++x) {
      // Get R, G, B components for the current pixel from input (RGB)
      const uint8_t *pixel_ptr = input_row_start + x * 3;
      uint8_t r = pixel_ptr[0];
      uint8_t g = pixel_ptr[1];
      uint8_t b = pixel_ptr[2];

      // Write B, G, R to the BMP buffer
      pm_write_u8(buffer_start, &current_offset, b);
      pm_write_u8(buffer_start, &current_offset, g);
      pm_write_u8(buffer_start, &current_offset, r);
      row_bytes_written_this_row += 3;
    }

    // Add row padding
    for (; (row_bytes_written_this_row % 4) != 0; ++row_bytes_written_this_row) {
      pm_write_u8(buffer_start, &current_offset, 0); // Write padding byte (0)
    }
  }

  return (int)current_offset;
}

// Function: pixelmap_get_size
// Calculates the total size in bytes required for a BMP file with given dimensions.
// width: Width of the image in pixels.
// height: Height of the image in pixels.
int pixelmap_get_size(uint32_t width, uint32_t height) {
  // Calculate row size padded to 4 bytes
  uint32_t row_size_bytes_unpadded = width * 3;
  uint32_t row_size_bytes_padded = (row_size_bytes_unpadded + 3) & ~3;
  
  // Calculate total image data size
  uint32_t image_data_size = row_size_bytes_padded * height;

  // Total file size includes image data and header size (54 bytes)
  return (int)(image_data_size + sizeof(BMPFileHeader) + sizeof(BMPInfoHeader));
}