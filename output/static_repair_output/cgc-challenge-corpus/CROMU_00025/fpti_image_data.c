#include <stdio.h>   // For printf
#include <stdlib.h>  // For malloc, free
#include <string.h>  // For memset
#include <stddef.h>  // For size_t
#include <stdint.h>  // For uint8_t, uint32_t, etc. (using unsigned int for simplicity as per original)

// Type redefinitions based on common decompiler output
typedef unsigned int uint;
typedef int undefined4; // Assuming it's a 4-byte integer, signed for return values
typedef char undefined; // Assuming it's a 1-byte char

// Structure to hold the bitstream context for reading data
// This corresponds to the `int *param_1` in fpti_read_check and fpti_read_nbits
typedef struct {
    char *data;         // The buffer containing the binary data (original *param_1)
    int size;           // Total size of the buffer in bytes (original param_1[1])
    int byte_offset;    // Current byte index in the buffer (original param_1[2])
    int bit_offset;     // Current bit index within the current byte (0-7) (original param_1[3])
} FPTI_BitStream;

// Structure to hold the overall image context
// This corresponds to the `int param_1` in fpti_add_pixel and fpti_display_img
typedef struct {
    FPTI_BitStream *stream; // Pointer to the bitstream reader
    char pixel_type;        // Pixel type, stored at offset 0x10 in the original code's struct
                            // (e.g., *(char *)(param_1 + 0x10))
} FPTI_Image;


// Forward declarations
int fpti_read_check(FPTI_BitStream *stream, int num_bits_needed);
int fpti_read_nbits(FPTI_BitStream *stream, int num_bits, uint *value);
int fpti_read_magic(FPTI_BitStream *stream);
int fpti_read_xaxis(FPTI_BitStream *stream);
int fpti_read_yaxis(FPTI_BitStream *stream);
int fpti_read_ptype(FPTI_Image *img, int *pixel_type_val);
int fpti_read_axist(FPTI_BitStream *stream);
int fpti_read_pixel(FPTI_BitStream *stream, int *x_coord, int *y_coord);
int fpti_add_pixel(FPTI_Image *img, int x, int y, char *image_buffer, int width, int height, int axis_type);
int fpti_display_img(FPTI_Image *img);


// Function: fpti_add_pixel
int fpti_add_pixel(FPTI_Image *img, int x, int y, char *image_buffer, int width, int height, int axis_type) {
  char pixel_char = 0;
  int pixel_index;
  
  if (img == NULL || image_buffer == NULL) {
    return 0;
  }

  // Calculate pixel_index based on axis_type
  switch (axis_type) {
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
    default:
      return 0; // Invalid axis_type
  }

  // Determine character to draw based on pixel_type
  switch (img->pixel_type) {
    case 0:
      pixel_char = '.';
      break;
    case 1:
      pixel_char = '*';
      break;
    case 2:
      pixel_char = '#';
      break;
    case 3:
      pixel_char = '+';
      break;
    default: // Default case if pixel_type is unknown
      pixel_char = '?';
      break;
  }
  
  if (width * height <= pixel_index || pixel_index < 0) { // Check for out of bounds access
    printf("[ERROR] Pixel beyond image: %d (max: %d)\n", pixel_index, width * height - 1);
    return 0;
  } else {
    image_buffer[pixel_index] = pixel_char;
    return 1;
  }
}

// Function: fpti_display_img
int fpti_display_img(FPTI_Image *img) {
  int width = 0;
  int height = 0;
  int pixel_type_val = 0;
  int axis_type = 0;
  char *image_buffer = NULL;
  size_t total_pixels = 0;
  
  int min_x = 0, min_y = 0;
  int max_x = 0, max_y = 0;

  if (img == NULL || img->stream == NULL) {
      return 0;
  }

  // Read header information
  // The fpti_read_nbits for 0xf (15 bits) seems to be a dummy read into min_x,
  // as min_x is then overwritten.
  if (!(fpti_read_magic(img->stream) &&
        (width = fpti_read_xaxis(img->stream)) && width != 0 &&
        (height = fpti_read_yaxis(img->stream)) && height != 0 &&
        fpti_read_ptype(img, &pixel_type_val) &&
        (axis_type = fpti_read_axist(img->stream)) && axis_type != 0 &&
        fpti_read_check(img->stream, 0xf) && // Check for 15 bits
        fpti_read_nbits(img->stream, 0xf, (uint*)&min_x))) { // Dummy read of 15 bits
      printf("[ERROR] Failed to read image header.\n");
      return 0;
  }

  // Determine axis boundaries based on axis_type
  switch(axis_type) {
    case 1: // Y-axis inverted
      min_x = 0;
      min_y = 1 - height;
      max_x = width - 1;
      max_y = 0;
      break;
    case 2: // X and Y axis inverted
      min_x = 1 - width;
      min_y = 1 - height;
      max_x = 0;
      max_y = 0;
      break;
    case 3: // X-axis inverted
      min_x = 0;
      min_y = 0;
      max_x = width - 1;
      max_y = height - 1;
      break;
    case 4: // X-axis inverted
      min_x = 1 - width;
      min_y = 0;
      max_x = 0;
      max_y = height - 1;
      break;
    case 7: // Centered
      min_x = -(width / 2);
      max_y = height / 2;
      max_x = width / 2 - (width + 1) % 2;
      min_y = (height + 1) % 2 - height / 2;
      break;
    default:
      printf("[ERROR] Invalid FPTI Axis Type: %d\n", axis_type);
      return 0;
  }

  total_pixels = (size_t)width * height;
  image_buffer = (char *)malloc(total_pixels + 1); // +1 for null terminator
  if (image_buffer == NULL) {
      printf("[ERROR] Failed to allocate memory for image buffer.\n");
      return 0;
  }
  
  memset(image_buffer, ' ', total_pixels); // Initialize with spaces
  image_buffer[total_pixels] = '\0'; // Null-terminate the buffer

  int read_x, read_y;
  while (fpti_read_pixel(img->stream, &read_x, &read_y)) {
    if ((read_x < min_x) || (max_x < read_x)) {
      printf("[ERROR] X coordinate out of bounds: %d (min: %d, max: %d)\n", read_x, min_x, max_x);
      free(image_buffer);
      return 0;
    }
    // Corrected Y-axis bounds check: read_y should be within [min_y, max_y]
    if ((read_y < min_y) || (max_y < read_y)) {
      printf("[ERROR] Y coordinate out of bounds: %d (min: %d, max: %d)\n", read_y, min_y, max_y);
      free(image_buffer);
      return 0;
    }
    if (!fpti_add_pixel(img, read_x, read_y, image_buffer, width, height, axis_type)) {
        printf("[ERROR] Failed to add pixel at (%d, %d).\n", read_x, read_y);
        free(image_buffer);
        return 0;
    }
  }

  // Display the image
  for (size_t i = 0; i < total_pixels; ++i) {
    if ((i % width == 0) && (i != 0)) {
      printf("\n");
    }
    printf("%c", image_buffer[i]);
  }
  printf("\n");
  
  free(image_buffer);
  return 1;
}

// Function: fpti_read_pixel
int fpti_read_pixel(FPTI_BitStream *stream, int *x_coord, int *y_coord) {
  uint sign_x, val_x, sign_y, val_y;
  
  if (stream == NULL || x_coord == NULL || y_coord == NULL) {
    return 0;
  }

  // Read sign and value for X coordinate and Y coordinate
  // Total 14 bits: 1-bit sign_x, 6-bit val_x, 1-bit sign_y, 6-bit val_y
  if (!(fpti_read_check(stream, 14) &&
        fpti_read_nbits(stream, 1, &sign_x) &&
        fpti_read_nbits(stream, 6, &val_x) &&
        fpti_read_nbits(stream, 1, &sign_y) &&
        fpti_read_nbits(stream, 6, &val_y))) {
    return 0;
  }

  *x_coord = (sign_x == 1) ? -(int)val_x : (int)val_x;
  *y_coord = (sign_y == 1) ? -(int)val_y : (int)val_y;
  
  return 1;
}

// Function: fpti_read_axist
int fpti_read_axist(FPTI_BitStream *stream) {
  uint axis_type = 0;
  if (!(stream != NULL &&
        fpti_read_check(stream, 3) &&
        fpti_read_nbits(stream, 3, &axis_type))) {
    return 0;
  }
  return (int)axis_type;
}

// Function: fpti_read_ptype
int fpti_read_ptype(FPTI_Image *img, int *pixel_type_val) {
  uint type_read = 0;
  if (img == NULL || img->stream == NULL) {
    return 0;
  }
  
  if (!(fpti_read_check(img->stream, 2) &&
        fpti_read_nbits(img->stream, 2, &type_read))) {
    return 0;
  }
  
  *pixel_type_val = (int)type_read;
  img->pixel_type = (char)type_read; // Store in the image context
  
  return 1;
}

// Function: fpti_read_xaxis
int fpti_read_xaxis(FPTI_BitStream *stream) {
  uint axis_val = 0;
  if (!(stream != NULL &&
        fpti_read_check(stream, 6) &&
        fpti_read_nbits(stream, 6, &axis_val))) {
    return 0;
  }
  return (int)axis_val;
}

// Function: fpti_read_yaxis
int fpti_read_yaxis(FPTI_BitStream *stream) {
  uint axis_val = 0;
  if (!(stream != NULL &&
        fpti_read_check(stream, 6) &&
        fpti_read_nbits(stream, 6, &axis_val))) {
    return 0;
  }
  return (int)axis_val;
}

// Function: fpti_read_magic
int fpti_read_magic(FPTI_BitStream *stream) {
  uint magic_val = 0;
  if (!(stream != NULL &&
        fpti_read_check(stream, 0x20) && // Check for 32 bits
        fpti_read_nbits(stream, 0x20, &magic_val))) {
    return 0;
  }
  return (magic_val == 0x24c7ee85);
}

// Function: fpti_read_check
int fpti_read_check(FPTI_BitStream *stream, int num_bits_needed) {
  if (stream == NULL || stream->data == NULL) {
    return 0;
  }
  if (num_bits_needed <= 0 || num_bits_needed > 32) { // Max bits to read at once is 32 for 'uint'
    return 0;
  }

  // Calculate total bits remaining from current position
  // (stream->size - stream->byte_offset - 1) gives full remaining bytes after current
  // * 8 converts to bits.
  // (8 - stream->bit_offset) gives bits remaining in the current byte.
  int bits_remaining = (stream->size - stream->byte_offset - 1) * 8; 
  if (stream->byte_offset < stream->size) { // Ensure we are within bounds before calculating bits in current byte
      bits_remaining += (8 - stream->bit_offset); 
  }

  return bits_remaining >= num_bits_needed;
}

// Function: fpti_read_nbits
int fpti_read_nbits(FPTI_BitStream *stream, int num_bits, uint *value) {
  if (value == NULL || num_bits <= 0 || num_bits > 32) { // Max bits for uint
    return 0;
  }
  if (!fpti_read_check(stream, num_bits)) {
    return 0;
  }

  *value = 0; // Initialize result
  
  // Read bits one by one to correctly handle byte boundaries
  for (int i = 0; i < num_bits; ++i) {
    *value <<= 1; // Shift existing value to make room for new bit
    
    // Get the current bit from the current byte
    unsigned char current_byte = (unsigned char)stream->data[stream->byte_offset];
    int bit_val = (current_byte >> (7 - stream->bit_offset)) & 1U;
    
    *value |= bit_val; // Set the new bit

    // Advance bitstream position
    stream->bit_offset = (stream->bit_offset + 1) % 8;
    if (stream->bit_offset == 0) {
      stream->byte_offset++;
    }
  }
  
  return 1;
}

// Dummy main function for compilation
int main() {
    // This main function creates a small, bit-packed FPTI image data structure
    // and attempts to display it using the provided functions.
    // The data represents a 4x4 image with pixel type 0 ('.') and axis type 1 (Y-inverted).
    // It contains 4 pixels at (0,0), (1,1), (2,0), (3,1).

    // Bitstream data layout:
    // Magic (32 bits) = 0x24C7EE85
    // X-axis (6 bits) = 4 (0b000100)
    // Y-axis (6 bits) = 4 (0b000100)
    // Ptype (2 bits) = 0 (0b00)
    // Axist (3 bits) = 1 (0b001)
    // Check (15 bits) = 0 (padding/reserved)
    // Nbits (15 bits) = 0 (padding/reserved)
    // Pixel 1 (14 bits): X=0, Y=0 (0b0_000000_0_000000)
    // Pixel 2 (14 bits): X=1, Y=1 (0b0_000001_0_000001)
    // Pixel 3 (14 bits): X=2, Y=0 (0b0_000010_0_000000)
    // Pixel 4 (14 bits): X=3, Y=1 (0b0_000011_0_000001)

    // Total bits: 32 + 6 + 6 + 2 + 3 + 15 + 15 + (4 * 14) = 135 bits
    // Total bytes: ceil(135 / 8) = 17 bytes

    unsigned char raw_data[17];
    memset(raw_data, 0, sizeof(raw_data));

    // Helper to set bits into the raw_data buffer
    // This simulates how the data would be written to a file.
    int current_byte_idx = 0;
    int current_bit_in_byte = 0;

    void set_bits_in_buffer(unsigned int value, int num_bits) {
        for (int i = num_bits - 1; i >= 0; --i) { // Iterate from MSB to LSB of the value
            if ((value >> i) & 1) { // If the current bit in 'value' is 1
                raw_data[current_byte_idx] |= (1 << (7 - current_bit_in_byte)); // Set the corresponding bit in the buffer
            }
            current_bit_in_byte++;
            if (current_bit_in_byte == 8) { // Move to next byte if current byte is full
                current_bit_in_byte = 0;
                current_byte_idx++;
            }
        }
    }

    // Populate the raw_data buffer with the image header and pixel data
    set_bits_in_buffer(0x24c7ee85, 32); // Magic number
    set_bits_in_buffer(4, 6);           // Width = 4
    set_bits_in_buffer(4, 6);           // Height = 4
    set_bits_in_buffer(0, 2);           // Ptype = 0
    set_bits_in_buffer(1, 3);           // Axist = 1
    set_bits_in_buffer(0, 15);          // Check/Nbits dummy 1
    set_bits_in_buffer(0, 15);          // Check/Nbits dummy 2

    // Pixel data (14 bits each: 1-bit x_sign, 6-bit x_val, 1-bit y_sign, 6-bit y_val)
    set_bits_in_buffer(0b00000000000000, 14); // Pixel (0,0)
    set_bits_in_buffer(0b00000010000001, 14); // Pixel (1,1)
    set_bits_in_buffer(0b00000100000000, 14); // Pixel (2,0)
    set_bits_in_buffer(0b00000110000001, 14); // Pixel (3,1)

    // Initialize FPTI_BitStream and FPTI_Image structures
    FPTI_BitStream stream = {
        .data = (char*)raw_data,
        .size = sizeof(raw_data),
        .byte_offset = 0,
        .bit_offset = 0
    };

    FPTI_Image img = {
        .stream = &stream,
        .pixel_type = 0 // Initial value, will be updated by fpti_read_ptype
    };

    printf("Attempting to display image...\n");
    if (fpti_display_img(&img)) {
        printf("Image displayed successfully.\n");
    } else {
        printf("Failed to display image.\n");
    }

    return 0;
}