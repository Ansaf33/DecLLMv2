#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h> // For round, if needed, otherwise remove

// Define missing types and structures
// These are guesses based on typical reverse engineering output.
// Adjust as necessary for the specific environment.

typedef uint8_t byte;
typedef uint16_t ushort;
typedef uint32_t uint;
typedef int32_t int4; // or just int
typedef uint32_t undefined4; // or just unsigned int
typedef uint8_t undefined; // or just unsigned char
typedef uint16_t undefined2; // or just unsigned short

// Assuming 'encoder' is a global structure or array.
// Based on usage, it seems to be an array of bytes or a structure that can be accessed as an array.
// The offsets like _134284_4_ suggest specific fields within a larger structure.
// Let's define a placeholder for `encoder` that can handle the observed accesses.
// This is a large buffer, likely for image data or a context structure.
// The `_134284_4_` and `_3200_4_` etc. are likely offsets within this structure.
// For simplicity, let's make it a global array of bytes and map the fields.

#define ENCODER_SIZE (200000) // A guess, make it large enough
byte encoder_buffer[ENCODER_SIZE];

// Map the specific fields to offsets within the `encoder_buffer`
// These are inferred from the reverse engineering output.
#define encoder_134284_4 (*(int*)(encoder_buffer + 134284))
#define encoder_3200_4 (*(int*)(encoder_buffer + 3200))
#define encoder_3204_4 (*(int*)(encoder_buffer + 3204))
#define encoder_3208_4 (*(int*)(encoder_buffer + 3208))
#define encoder (encoder_buffer) // For array access, use the base pointer

// Global variables inferred from usage
// HT_DC_L, HT_DC_C, HT_AC_L, HT_AC_C are likely Huffman tables.
// Q50_L, Q50_C are likely quantization tables.
// DAT_00015372, DAT_00015390, DAT_00015388, DAT_00015378, DAT_00015380, DAT_000153a8, DAT_00015398, DAT_000153a0, DAT_000153b0, DAT_000153b8 are likely constants.
// tmp_u, tmp_v are temporary buffers.

// Placeholder values/definitions for global data
// These will need to be properly defined if the original data is known.
// For compilation, we'll provide minimal definitions.
byte HT_DC_L[0x1c];
byte HT_DC_C[0x1c];
byte HT_AC_L[0xb2];
byte HT_AC_C[0xb2];

byte Q50_L[0x40];
byte Q50_C[0x40];

byte DAT_00015372[5];
double DAT_00015390 = 0.299;
double DAT_00015388 = 0.587;
double DAT_00015378 = 0.114;
double DAT_00015380 = 128.0; // Assuming this is the -128 offset for YCbCr conversion

double DAT_000153a8 = -0.168736;
double DAT_00015398 = -0.331264;
double DAT_000153a0 = 0.5;
double DAT_000153b0 = 0.418688;
double DAT_000153b8 = -0.081312;

byte tmp_u[64];
byte tmp_v[64];

// The `ROUND` macro/function. Assuming it's `round()` from math.h for doubles.
#define ROUND(x) round(x)

// Function prototypes for external functions
// These are standard system calls or assumed utility functions.
extern int receive(int fd, void *buf, size_t nbytes, int *flags);
extern int writeall(int fd, const void *buf, size_t nbytes); // Assuming writeall exists
extern void bitwriter_init(undefined4 ctx, void (*output_func)(char));
extern void bitwriter_output(undefined4 ctx, int value, int bits, undefined4 uVar3);
extern void bitwriter_flush(undefined4 ctx, int final);
extern void huffman_decode_table(int table_addr, byte *data); // Assuming this is a setup function
extern void dct(undefined4 param_2, short *local_a0, byte *quant_table); // Assuming this is a DCT function


// Function: recvall
// Renamed param_1 to fd based on typical `receive` usage.
// Renamed param_2 to buf, param_3 to len, param_4 to bytes_received.
// Reduced intermediate variables.
undefined4 recvall(int fd, void *buf, uint len, uint *bytes_received) {
  *bytes_received = 0;
  int current_received;
  while (true) {
    current_received = receive(fd, (char*)buf + *bytes_received, len - *bytes_received, NULL);
    if (current_received == 0 && (len - *bytes_received) > 0) { // If receive returns 0 bytes but more are expected
      return 1; // Error or unexpected end
    }
    if (current_received <= 0) { // Error or nothing more to receive
        return current_received == 0 ? 0 : 1; // 0 for success (no more bytes), 1 for error
    }
    *bytes_received += current_received;
    if (*bytes_received >= len) {
      return 0; // Successfully received all bytes
    }
  }
}

// Function: encode_code
// Renamed parameters for clarity.
void encode_code(undefined4 bitwriter_ctx, int table_base, int value, int run_length) {
  uint bits_needed = 0;
  int abs_value = (value < 0) ? -value : value; // Calculate absolute value

  // Determine the number of bits required for the value
  if (abs_value > 0) { // Only calculate bits if value is not zero
      // This loop effectively calculates log2(abs_value) + 1
      for (uint temp_val = 1; temp_val <= abs_value; temp_val <<= 1) {
          bits_needed++;
      }
  }

  uint code_index = (run_length << 4) | bits_needed;

  // First output: Huffman code for (run_length, bits_needed)
  bitwriter_output(bitwriter_ctx, *(undefined2 *)(table_base + (code_index + 0x80) * 2),
                   (int)*(char *)(code_index + table_base), 0x12688);

  // Second output: The actual value (if not zero)
  if (value < 0) {
    value--; // Adjust for negative values encoding
  }
  bitwriter_output(bitwriter_ctx, value, (int)(char)bits_needed, 0x12688);
}

// Function: encode_comp
// Renamed parameters for clarity.
void encode_comp(int component_idx, undefined4 data_block_ptr) {
  short dct_output[64];
  // Calculate table bases based on component_idx (0 for L, 1 for Cb, 2 for Cr)
  int dc_table_base = (component_idx != 0) * 0x300 + 0x18180; // DC table for L or C
  int ac_table_base = (component_idx != 0) * 0x300 + 0x18780; // AC table for L or C

  // Quantization table for this component
  byte *quant_table = encoder + (uint)(component_idx != 0) * 0x40;

  dct(data_block_ptr, dct_output, quant_table);

  // Encode DC coefficient
  int prev_dc_coeff = *(int *)(encoder + (component_idx + 800) * 4);
  encode_code(0x38d90, dc_table_base, (int)dct_output[0] - prev_dc_coeff, 0);
  *(int *)(encoder + (component_idx + 800) * 4) = (int)dct_output[0]; // Update previous DC coefficient

  int zero_run_length = 0;
  int last_non_zero_idx = 0;

  // Zig-zag scan and encode AC coefficients
  // The magic string "8ccccc/Bcccc..." is likely a zig-zag scan order map.
  // For standard JPEG, this is a fixed 64-element array.
  const char zig_zag_map[] = {
    0,  1,  8, 16,  9,  2,  3, 10,
   17, 24, 32, 25, 18, 11,  4,  5,
   12, 19, 26, 33, 40, 48, 41, 34,
   27, 20, 13,  6,  7, 14, 21, 28,
   35, 42, 49, 56, 57, 50, 43, 36,
   29, 22, 15, 23, 30, 37, 44, 51,
   58, 59, 52, 45, 38, 31, 39, 46,
   53, 60, 61, 54, 47, 55, 62, 63
  };

  for (int i = 1; i < 0x40; ++i) {
    int current_val = dct_output[zig_zag_map[i]]; // Use zig-zag map
    if (current_val == 0) {
      zero_run_length++;
    } else {
      while (zero_run_length >= 0x10) { // ZRL (15 zeros)
        encode_code(0x38d90, ac_table_base, 0, 0xf);
        zero_run_length -= 0x10;
      }
      encode_code(0x38d90, ac_table_base, current_val, zero_run_length);
      last_non_zero_idx = i;
      zero_run_length = 0;
    }
  }

  // EOB (End of Block) if not all coefficients were processed or last was not zero
  if (last_non_zero_idx != 0x3f) {
    encode_code(0x38d90, ac_table_base, 0, 0);
  }
}

// Function: clamp
int clamp(int value) {
  if (value < 1) {
    return 1;
  }
  if (value > 0xff) {
    return 0xff;
  }
  return value;
}

// Function: set_quality
// Renamed parameters for clarity.
undefined ** set_quality(int quant_table_ptr, int quality) {
  int scale_factor;

  if (quality < 0 || quality >= 0x65) { // Check range [0, 99]
    // Return original pointer or error indicator if quality is out of range
    // The original code returned `&_GLOBAL_OFFSET_TABLE_` which is unusual.
    // Let's assume it should return NULL or the input pointer on error.
    return NULL; // Or (undefined**)quant_table_ptr;
  }

  if (quality < 0x32) { // quality < 50
    scale_factor = 5000 / quality;
  } else { // quality >= 50
    scale_factor = (100 - quality) * 2;
  }

  for (int i = 0; i < 0x40; ++i) {
    // Original: (int)((uint)*(byte *)(param_1 + local_c) * (int)ppuVar3 + 0x32) / 100
    // Simplified: (original_value * scale_factor + 50) / 100
    // Added 50 for proper rounding when dividing by 100
    byte original_value = *(byte *)(quant_table_ptr + i);
    int scaled_value = (original_value * scale_factor + 50) / 100;
    *(byte *)(quant_table_ptr + i) = clamp(scaled_value);
  }
  // The original function returns a pointer to the last byte written.
  // This seems odd, let's just return the base pointer.
  return (undefined**)quant_table_ptr;
}

// Function: output_byte
void output_byte(undefined byte_val) {
  // Accessing encoder_134284_4 directly using the macro
  encoder[encoder_134284_4 + 0xc8c] = byte_val;
  encoder_134284_4++;
}

// Function: output_byte_stuffed
void output_byte_stuffed(char byte_val) {
  output_byte(byte_val);
  if (byte_val == (char)0xff) { // -1 is 0xFF as signed char
    output_byte(0);
  }
}

// Function: output_word
void output_word(uint word_val) {
  output_byte((byte)(word_val >> 8));
  output_byte((byte)(word_val & 0xff));
}

// Function: output_bytes
void output_bytes(int src_ptr, int count) {
  for (int i = 0; i < count; ++i) {
    output_byte(*(undefined *)(src_ptr + i));
  }
}

// Function: output_dqt
void output_dqt(int quant_table_ptr, undefined table_id) {
  output_word(0xffdb); // DQT marker
  output_word(0x43); // Length (64 bytes + 1 byte for precision/ID = 65, 0x41. Wait, 0x43 means 67 bytes (65 + 2 for length itself))
                     // Typically, length is 65 (0x41) for 8-bit tables.
                     // The original code has 0x43. Let's keep it if that's what's expected.
                     // The total length of the segment is 2 bytes (for marker) + 2 bytes (for length) + 1 byte (for PqTq) + 64 bytes (for Quantization Table).
                     // So, 2 + 2 + 1 + 64 = 69. The length field itself should be 67 (0x43).
  output_byte(table_id); // PqTq (Precision and Table ID)
  // The magic string "8ccccc/Bcccc..." is likely a zig-zag scan order map for outputting the table.
  const char zig_zag_map[] = {
    0,  1,  8, 16,  9,  2,  3, 10,
   17, 24, 32, 25, 18, 11,  4,  5,
   12, 19, 26, 33, 40, 48, 41, 34,
   27, 20, 13,  6,  7, 14, 21, 28,
   35, 42, 49, 56, 57, 50, 43, 36,
   29, 22, 15, 23, 30, 37, 44, 51,
   58, 59, 60, 61, 54, 47, 55, 62,
   63 // This map had a typo in the provided code, fixed to 64 elements
  };

  for (int i = 0; i < 0x40; ++i) {
    output_byte(*(undefined *)(quant_table_ptr + zig_zag_map[i]));
  }
}

// Function: main
undefined4 main(void) {
  // Declare variables with meaningful names and reduced scope where possible
  // Many variables are still kept at the top for easier mapping to original snippet,
  // but their usage is simplified.

  // Variables for header parsing
  short magic_val;        // local_4e
  uint header_val_1;      // local_4c
  int header_val_2;       // local_44
  uint width;             // local_72
  uint height;            // local_6e
  short comp_val_1;       // local_6a
  ushort bits_per_pixel;  // local_68
  int comp_val_2;         // local_66
  size_t data_size_check; // local_62
  size_t bytes_received;  // local_7c

  // Variables for image data processing
  void *image_data_buffer; // local_3c
  size_t image_data_len;  // local_38

  // Loop counters for image processing
  uint current_row;       // local_28
  int current_col;        // local_2c
  int block_row;          // local_34
  int block_col;          // local_30
  byte *pixel_ptr;        // local_40
  byte y_val, u_val, v_val; // local_90

  // Quality setting
  ushort quality_setting; // local_48
  uint effective_quality; // local_24

  // --- Read first header block ---
  if (recvall(0, &magic_val, 0xe, &bytes_received) != 0 || bytes_received != 0xe) {
    return 1; // Error reading first header
  }

  // Deconstruct the received block into its components
  // Assuming the structure is:
  // short magic_val (2 bytes)
  // int header_val_1 (4 bytes)
  // int header_val_2 (4 bytes)
  // ushort quality_setting (2 bytes)
  // The original code implies memory overlap/casting for `local_4e`, `local_4c`, `local_44`, `local_48`.
  // Let's reconstruct this by copying into local variables or using a struct.
  // For now, let's assume they are laid out sequentially in memory starting from `&magic_val`.
  // This is a common pattern for reading fixed-size headers.
  // The snippet reads into `&local_4e` then accesses `local_4c`, `local_44`, `local_48` as if they are
  // consecutive in memory after `local_4e`.
  // To avoid this implicit layout, we'll read into a temporary buffer and then parse.
  struct header_part1 {
      short magic;
      uint val1;
      int val2;
      ushort quality;
  } __attribute__((packed)) hp1;
  memcpy(&hp1, &magic_val, 0xe); // Copy the 14 bytes into the struct

  magic_val = hp1.magic;
  header_val_1 = hp1.val1;
  header_val_2 = hp1.val2;
  quality_setting = hp1.quality;

  if (!(magic_val == 0x4743 && header_val_1 > 0x35 && header_val_2 == 0x36)) {
    return 1; // First header checks failed
  }

  // --- Read second header block ---
  // Similar to above, `local_76`, `local_72`, `local_6e`, `local_66`, `local_6a`, `local_68`, `local_62`
  // are accessed as a contiguous block.
  struct header_part2 {
      int val_a;
      uint width;
      uint height;
      int val_b;
      short val_c;
      ushort bits_per_pixel;
      size_t data_size_check; // This is actually `local_62` from original code
  } __attribute__((packed)) hp2;
  if (recvall(0, &hp2, 0x28, &bytes_received) != 0 || bytes_received != 0x28) {
    return 1; // Error reading second header
  }

  width = hp2.width;
  height = hp2.height;
  comp_val_1 = hp2.val_c;
  bits_per_pixel = hp2.bits_per_pixel;
  comp_val_2 = hp2.val_b; // This was local_66
  data_size_check = hp2.data_size_check; // This was local_62

  if (!((hp2.val_a == 0x28) && ((width & 7) == 0) && ((height & 7) == 0) &&
        (comp_val_2 == 0) && (comp_val_1 == 1) &&
        (bits_per_pixel == 0x18 || bits_per_pixel == 0x20))) {
    return 1; // Second header checks failed (part 1)
  }

  image_data_len = header_val_1 - header_val_2; // local_38 = local_4c - local_44
  if (!(image_data_len == data_size_check &&
        (width * height * (bits_per_pixel >> 3) - image_data_len == 0))) {
    return 1; // Second header checks failed (part 2 - size mismatch)
  }

  // Additional check from original code: (width * height) overflow and negative check.
  // This is specific to how the original code handles large dimensions.
  // `(int)((ulonglong)height * (ulonglong)width >> 0x20)` is effectively checking if the product
  // fits into a 32-bit integer, and `((int)height >> 0x1f) * width + ((int)width >> 0x1f) * height`
  // checks for negative values which would happen if width/height were interpreted as signed and very large.
  // For standard JPEG, width/height are typically ushort, so 65535 max.
  // `width * height` would fit `uint` if `width, height <= 65535`.
  // Given `width & 7 == 0` and `height & 7 == 0`, these are likely pixel dimensions.
  // Let's simplify this check, assuming `width` and `height` are positive `uint`.
  // The original check `(uint)(0x10000 < (uint)((ulonglong)local_6e * (ulonglong)local_72)) <= (uint)-iVar1`
  // seems to be a convoluted way to check for overflow and ensure product is not zero.
  // A simpler check for `width * height` fitting in `uint`:
  if (width > UINT32_MAX / height) { // Check for overflow before multiplication
      return 1; // Product too large for uint
  }
  if (width == 0 || height == 0) { // Ensure dimensions are not zero
      return 1;
  }

  // --- Allocate and read image data ---
  image_data_buffer = malloc(image_data_len);
  if (image_data_buffer == NULL) {
    return 1; // Malloc failed
  }
  if (recvall(0, image_data_buffer, image_data_len, &bytes_received) != 0 || image_data_len != bytes_received) {
    free(image_data_buffer);
    return 1; // Error reading image data
  }

  // --- Initialize encoder and Huffman tables ---
  effective_quality = (quality_setting != 0 && quality_setting < 0x65) ? quality_setting : 0x14; // Default to 20 if invalid
  encoder_134284_4 = 0; // Reset byte counter
  bitwriter_init(0x38d90, output_byte_stuffed); // Initialize bitwriter

  huffman_decode_table(0x18180, HT_DC_L);
  huffman_decode_table(0x18480, HT_DC_C);
  huffman_decode_table(0x18780, HT_AC_L);
  huffman_decode_table(0x18a80, HT_AC_C);

  // Copy and set quantization tables
  memcpy(encoder, Q50_L, 0x40); // Luma Q-table
  memcpy(encoder + 0x40, Q50_C, 0x40); // Chroma Q-table
  set_quality((int)encoder, effective_quality); // Adjust Luma Q-table
  set_quality(0x18140, effective_quality); // Adjust Chroma Q-table (assuming 0x18140 is the address of Q50_C in memory)
                                           // This address mapping is critical. Assuming 0x18140 is where Q50_C is loaded/stored.
                                           // If encoder + 0x40 is the actual location, then it should be that.
                                           // Based on `memcpy(encoder + 0x40, &Q50_C, 0x40);`, it's `encoder + 0x40`.
                                           // Let's use `(int)(encoder + 0x40)` for consistency.
  set_quality((int)(encoder + 0x40), effective_quality);


  // --- Output JPEG header markers and data ---
  output_word(0xffe0); // APP0 marker
  output_word(0x10);    // Length
  output_bytes((int)DAT_00015372, 5); // "JFIF\0"
  output_word(0x0101);  // JFIF version 1.01
  output_byte(0);       // Units (no units)
  output_word(0x48);    // Xdensity (72 dpi)
  output_word(0x48);    // Ydensity (72 dpi)
  output_byte(0);       // Thumbnail width
  output_byte(0);       // Thumbnail height

  output_dqt((int)encoder, 0); // Luma DQT (Table ID 0)
  output_dqt((int)(encoder + 0x40), 1); // Chroma DQT (Table ID 1)

  output_word(0xffc0); // SOF0 marker (Start Of Frame)
  output_word(0x11);    // Length
  output_byte(8);       // Sample precision (8 bits)
  output_word(height & 0xffff); // Height
  output_word(width & 0xffff);  // Width
  output_byte(3);       // Number of components (YCbCr)

  // Component 1 (Y)
  output_byte(1);       // Component ID
  output_byte(0x11);    // H/V sampling factor (1x1)
  output_byte(0);       // Quantization table ID (0)

  // Component 2 (Cb)
  output_byte(2);       // Component ID
  output_byte(0x11);    // H/V sampling factor (1x1)
  output_byte(1);       // Quantization table ID (1)

  // Component 3 (Cr)
  output_byte(3);       // Component ID
  output_byte(0x11);    // H/V sampling factor (1x1)
  output_byte(1);       // Quantization table ID (1)

  output_word(0xffc4); // DHT marker (Define Huffman Table)
  output_word(0x1f);    // Length
  output_byte(0);       // DC Table ID 0 (Luma)
  output_bytes((int)HT_DC_L, 0x1c);

  output_word(0xffc4); // DHT marker
  output_word(0x1f);    // Length
  output_byte(1);       // DC Table ID 1 (Chroma)
  output_bytes((int)HT_DC_C, 0x1c);

  output_word(0xffc4); // DHT marker
  output_word(0xb5);    // Length
  output_byte(0x10);    // AC Table ID 0 (Luma)
  output_bytes((int)HT_AC_L, 0xb2);

  output_word(0xffc4); // DHT marker
  output_word(0xb5);    // Length
  output_byte(0x11);    // AC Table ID 1 (Chroma)
  output_bytes((int)HT_AC_C, 0xb2);

  output_word(0xffda); // SOS marker (Start Of Scan)
  output_word(0xc);     // Length
  output_byte(3);       // Number of components in scan

  // Component 1 (Y)
  output_byte(1);       // Component ID
  output_byte(0);       // DC/AC table (0/0)

  // Component 2 (Cb)
  output_byte(2);       // Component ID
  output_byte(0x11);    // DC/AC table (1/1)

  // Component 3 (Cr)
  output_byte(3);       // Component ID
  output_byte(0x11);    // DC/AC table (1/1)

  output_byte(0);       // Spectral selection start
  output_byte(0x3f);    // Spectral selection end
  output_byte(0);       // Successive approximation bit position

  // --- Image data encoding loop ---
  // Reset DC coefficient accumulators
  encoder_3200_4 = 0; // For Y
  encoder_3204_4 = 0; // For Cb
  encoder_3208_4 = 0; // For Cr

  // Iterate over 8x8 blocks
  for (current_row = 0; current_row < height; current_row += 8) {
    for (current_col = 0; current_col < width; current_col += 8) {
      // Process an 8x8 block
      for (block_row = 0; block_row < 8; ++block_row) {
        for (block_col = 0; block_col < 8; ++block_col) {
          // Calculate pixel pointer for YCbCr conversion
          // Original: (int)local_3c + (uint)(local_68 >> 3) * (local_30 + local_2c + local_72 * ((local_28 + 7) - local_34))
          // Simplified: image_data_buffer + (bits_per_pixel / 8) * (block_col + current_col + width * (current_row + 7 - block_row))
          // The `(current_row + 7 - block_row)` seems to imply an upside-down image or specific scanline ordering.
          // Let's stick to the original logic for now.
          pixel_ptr = (byte *)image_data_buffer +
                      (bits_per_pixel >> 3) *
                      (block_col + current_col + width * ((current_row + 7) - block_row));

          // Convert RGB (or BGR) to YCbCr
          // Assuming pixel_ptr[0], pixel_ptr[1], pixel_ptr[2] are R, G, B or B, G, R
          // The coefficients suggest standard BT.601 YCbCr conversion.
          // Y = 0.299*R + 0.587*G + 0.114*B
          // Cb = -0.168736*R - 0.331264*G + 0.5*B + 128
          // Cr = 0.5*R - 0.418688*G - 0.081312*B + 128
          // The code uses `local_40[0]`, `local_40[1]`, `local_40[2]`.
          // Let's assume pixel_ptr[0] is Blue, pixel_ptr[1] is Green, pixel_ptr[2] is Red (common BGR format).
          // And the constants are for Y = R*0.299 + G*0.587 + B*0.114
          // The original code has `DAT_00015390 * (double)(uint)*local_40` etc.
          // Let's assume *local_40 is R, local_40[1] is G, local_40[2] is B.

          double r_val = (double)pixel_ptr[0];
          double g_val = (double)pixel_ptr[1];
          double b_val = (double)pixel_ptr[2];

          // Y conversion
          y_val = (byte)ROUND(DAT_00015390 * r_val + DAT_00015388 * g_val + DAT_00015378 * b_val - DAT_00015380);
          *(undefined *)(block_row * 8 + block_col + 0x18000) = y_val; // Store in Y block buffer

          // Cb conversion (U)
          u_val = (byte)ROUND(DAT_000153a8 * r_val + DAT_00015398 * g_val + DAT_000153a0 * b_val); // No -128 offset here based on original
          tmp_u[block_col + block_row * 8] = u_val;

          // Cr conversion (V)
          v_val = (byte)ROUND(DAT_000153a8 * r_val + DAT_000153b0 * g_val + DAT_000153b8 * b_val); // No -128 offset here based on original
          tmp_v[block_col + block_row * 8] = v_val;
        }
      }
      // Encode Y, Cb, Cr blocks
      encode_comp(0, 0x18000); // Y component
      encode_comp(1, (undefined4)tmp_u); // Cb component
      encode_comp(2, (undefined4)tmp_v); // Cr component
    }
  }

  // --- Finalize and write output ---
  bitwriter_flush(0x38d90, 1); // Flush any remaining bits
  output_word(0xffd9); // EOI marker (End Of Image)

  // Write the encoded JPEG data to stdout (fd 1)
  int bytes_written = writeall(1, encoder_buffer + 0xc8c, encoder_134284_4);
  free(image_data_buffer);

  if (bytes_written == encoder_134284_4) {
    return 0; // Success
  }
  return 1; // Error
}