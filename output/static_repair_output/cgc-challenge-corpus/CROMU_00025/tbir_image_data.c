#include <stdio.h>   // For printf
#include <stdlib.h>  // For malloc, free
#include <string.h>  // For memset
#include <stdint.h>  // For uint32_t, uint16_t, uint8_t

// Type definitions based on typical disassembler output and usage
typedef uint32_t uint; // Used for uVar1, local_20, etc.
typedef uint32_t undefined4;
typedef uint16_t undefined2;
typedef uint8_t  undefined; // Used for character data

// Dummy struct to represent the context pointer `param_1`
// Offsets derived from usage (assuming int is 4 bytes):
// 0x00: data_ptr (base address for reading, *param_1 in tbir_read_nbits)
// 0x04: total_bits (param_1[1], total size in bits, e.g., dword_count * 32)
// 0x08: current_byte_idx (param_1[2], current byte index in data_ptr)
// 0x0C: current_bit_offset (param_1[3], current bit offset within the current byte)
// 0x10: width (param_1[4])
// 0x14: height (param_1[5])
// 0x18: pixel_type (param_1[6])
// 0x1C: orientation (param_1[7])
// 0x20: pixel_data (param_1[8])
// 0x24: pixel_data_size (param_1[9])
// 0x28: checksum (param_1[10])
// 0x2C: flags (param_1[11])
typedef struct TbirContext {
    uint8_t *data_ptr;        // 0x00: Base address of data stream
    int total_bits;           // 0x04: Total bits available in the data stream
    int current_byte_idx;     // 0x08: Current byte index in data_ptr
    int current_bit_offset;   // 0x0C: Current bit offset within the current byte (0-7)
    int width;                // 0x10
    int height;               // 0x14
    int pixel_type;           // 0x18
    int orientation;          // 0x1C
    uint8_t *pixel_data;      // 0x20: Pointer to allocated pixel data
    int pixel_data_size;      // 0x24: Size of allocated pixel data
    uint32_t checksum;        // 0x28
    uint32_t flags;           // 0x2C
} TbirContext;

// Helper function for 2^n, equivalent to expi(2, n)
static unsigned int power_of_2(int n) {
    if (n < 0 || n >= 32) return 0; // Guard against invalid shifts
    return 1U << n;
}

// Mock functions for allocate/deallocate to match original semantics
// allocate returns 1 on success, 0 on failure.
static int allocate(size_t size, int param_2_unused, uint8_t **out_ptr) {
    (void)param_2_unused; // Suppress unused parameter warning
    *out_ptr = (uint8_t *)malloc(size);
    return (*out_ptr != NULL); // Return 1 on success, 0 on failure
}

// deallocate simply frees the memory.
static void deallocate(uint8_t *ptr, size_t size_unused) {
    (void)size_unused; // Suppress unused parameter warning
    free(ptr);
}

// Forward declarations
static undefined4 tbir_read_check(TbirContext *ctx, int num_bits);
static undefined4 tbir_read_nbits(TbirContext *ctx, int num_bits, uint32_t *out_val);
static undefined4 tbir_read_width(TbirContext *ctx);
static undefined4 tbir_read_height(TbirContext *ctx);
static undefined4 tbir_read_pixeltype(TbirContext *ctx);
static undefined4 tbir_read_loadd(TbirContext *ctx);
static int tbir_read_typef(TbirContext *ctx);
static undefined4 tbir_read_header(TbirContext *ctx);
static undefined4 tbir_read_chksum(TbirContext *ctx);
static undefined4 tbir_read_flags(TbirContext *ctx);
static undefined4 tbir_read_magic(TbirContext *ctx);

// Character lookup tables (derived from tbir_read_pixels initialization)
// The original code copied parts of a string into local variables. This is a direct representation.
const char char_map_type7[] = " !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"; // 95 chars, max index 94 (0x5E)
const char char_map_type6[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"; // 62 chars, max index 61 (0x3D)
const char char_map_type1[] = {0x20, 0x2e}; // 2 chars: ' ', '.'
const char char_map_type2[] = {0x20, 0x23, 0x2a, 0x40}; // 4 chars: ' ', '#', '*', '@'


// Function: tbir_read_chksum
undefined4 tbir_read_chksum(TbirContext *ctx) {
  uint32_t chksum_val = 0;
  if (ctx == NULL) {
    return 0;
  }
  if (tbir_read_nbits(ctx, 0x20, &chksum_val) == 0) { // Read 32 bits
    return 0;
  }
  if (((ctx->flags & 0x10000) == 0) || (ctx->checksum == chksum_val)) {
    ctx->checksum = chksum_val;
    ctx->flags |= 0x100000; // Set checksum present flag
    return 1;
  }
  return 0;
}

// Function: tbir_read_pixels
int tbir_read_pixels(TbirContext *ctx) {
  uint32_t pixel_val = 0;
  
  if (ctx == NULL) {
    return 0;
  }
  
  // Check required flags
  if (!((ctx->flags & 1U) && (ctx->flags & 0x10U) && (ctx->flags & 0x100U) && (ctx->flags & 0x1000U))) {
    return 0;
  }
  
  // Original `param_1[3] == 0` check, which means `ctx->current_bit_offset == 0` (byte aligned).
  // If not byte-aligned, fail.
  if (ctx->current_bit_offset != 0) {
    return 0;
  }
  
  ctx->flags |= 0x10000; // Set pixel set flag
  
  int total_pixel_bits = ctx->pixel_type * ctx->width * ctx->height;
  // Calculate padding to next 32-bit boundary (dword)
  int padding_bits = (total_pixel_bits + 0x1fU & 0xffffffe0) - total_pixel_bits;
  
  int total_data_bits_padded = padding_bits + total_pixel_bits;
  // If total_data_bits_padded becomes negative due to overflow or specific calculation, adjust
  if (total_data_bits_padded < 0) {
    total_data_bits_padded = (total_data_bits_padded + 0x1f) & 0xffffffe0; // Round up to nearest 32 if negative
  }
  int dword_count = total_data_bits_padded >> 5; // Divide by 32
  
  // Check if buffer size is sufficient
  // ctx->total_bits is param_1[1], ctx->current_byte_idx is param_1[2]
  // dword_count * 4 is bytes for the pixel data section
  if (ctx->total_bits < dword_count * 4 + ctx->current_byte_idx) {
    printf("Dword count too high: %d %d %d\n", dword_count * 4, ctx->current_byte_idx, ctx->total_bits);
    return 0;
  }
  
  uint32_t calculated_checksum = 0;
  // ctx->data_ptr is *param_1, ctx->current_byte_idx is param_1[2]
  uint8_t *checksum_start_ptr = ctx->data_ptr + ctx->current_byte_idx;
  
  for (int i = 0; i < dword_count; ++i) {
    calculated_checksum ^= *(uint32_t *)(checksum_start_ptr + i * 4);
  }
  
  if ((ctx->flags & 0x100000U) == 0) { // If checksum not already present
    ctx->checksum = calculated_checksum;
  } else if (calculated_checksum != ctx->checksum) {
    return 0; // Checksum mismatch
  }
  
  ctx->pixel_data_size = ctx->height * ctx->width;
  
  // Allocate memory for pixels (+1 for null terminator)
  if (allocate(ctx->pixel_data_size + 1, 0, &ctx->pixel_data) == 0) {
    return 0; // Allocation failed
  }
  
  memset(ctx->pixel_data, 0x20, ctx->pixel_data_size); // Fill with spaces
  ctx->pixel_data[ctx->pixel_data_size] = 0; // Null terminate
  
  int row, col;
  int current_pixel_offset;
  
  for (int i = 0; i < ctx->pixel_data_size; ++i) {
    if (tbir_read_nbits(ctx, ctx->pixel_type, &pixel_val) == 0) {
      deallocate(ctx->pixel_data, ctx->pixel_data_size + 1);
      ctx->pixel_data = NULL;
      ctx->pixel_data_size = 0;
      return 0;
    }
    
    // Calculate row and column based on orientation
    switch (ctx->orientation) {
      case 0: row = i / ctx->width; col = i % ctx->width; break;
      case 1: row = i / ctx->width; col = (ctx->width - 1) - (i % ctx->width); break;
      case 2: row = (ctx->height - 1) - (i / ctx->width); col = i % ctx->width; break;
      case 3: row = (ctx->height - 1) - (i / ctx->width); col = (ctx->width - 1) - (i % ctx->width); break;
      case 4: row = i % ctx->height; col = i / ctx->height; break;
      case 5: row = i % ctx->height; col = (ctx->width - 1) - (i / ctx->height); break;
      case 6: row = (ctx->height - 1) - (i % ctx->height); col = i / ctx->height; break;
      case 7: row = (ctx->height - 1) - (i % ctx->height); col = (ctx->width - 1) - (i / ctx->height); break;
      default:
        printf("[ERROR] Invalid orientation type: %d\n", ctx->orientation);
        deallocate(ctx->pixel_data, ctx->pixel_data_size + 1);
        ctx->pixel_data = NULL;
        ctx->pixel_data_size = 0;
        return 0;
    }
    
    current_pixel_offset = col + ctx->width * row;
    
    // Check bounds for current_pixel_offset
    if (current_pixel_offset < 0 || current_pixel_offset >= ctx->pixel_data_size) {
        printf("[ERROR] Pixel offset %d out of bounds (max %d) for pixel type %d\n", current_pixel_offset, ctx->pixel_data_size - 1, ctx->pixel_type);
        deallocate(ctx->pixel_data, ctx->pixel_data_size + 1);
        ctx->pixel_data = NULL;
        ctx->pixel_data_size = 0;
        return 0;
    }

    // Determine character based on pixel type
    switch (ctx->pixel_type) {
        case 7:
            if (pixel_val >= sizeof(char_map_type7)) {
                printf("[ERROR] Pixel value %u out of bounds for pixel type 7 (max %zu)\n", pixel_val, sizeof(char_map_type7) - 1);
                goto pixel_error_cleanup;
            }
            ctx->pixel_data[current_pixel_offset] = char_map_type7[pixel_val];
            break;
        case 6:
            if (pixel_val >= sizeof(char_map_type6)) {
                printf("[ERROR] Pixel value %u out of bounds for pixel type 6 (max %zu)\n", pixel_val, sizeof(char_map_type6) - 1);
                goto pixel_error_cleanup;
            }
            ctx->pixel_data[current_pixel_offset] = char_map_type6[pixel_val];
            break;
        case 1:
            if (pixel_val >= sizeof(char_map_type1)) {
                printf("[ERROR] Pixel value %u out of bounds for pixel type 1 (max %zu)\n", pixel_val, sizeof(char_map_type1) - 1);
                goto pixel_error_cleanup;
            }
            ctx->pixel_data[current_pixel_offset] = char_map_type1[pixel_val];
            break;
        case 2:
            if (pixel_val >= sizeof(char_map_type2)) {
                printf("[ERROR] Pixel value %u out of bounds for pixel type 2 (max %zu)\n", pixel_val, sizeof(char_map_type2) - 1);
                goto pixel_error_cleanup;
            }
            ctx->pixel_data[current_pixel_offset] = char_map_type2[pixel_val];
            break;
        default: // This case should theoretically not be hit if tbir_read_pixeltype checks are correct
            printf("[ERROR] Invalid pixel type %d\n", ctx->pixel_type);
            goto pixel_error_cleanup;
    }
  }
  
  // Read remaining padding bits
  if (tbir_read_nbits(ctx, padding_bits, &pixel_val) == 0) { // pixel_val is reused for dummy read
    goto pixel_error_cleanup;
  }
  
  return 1; // Success

pixel_error_cleanup:
  deallocate(ctx->pixel_data, ctx->pixel_data_size + 1);
  ctx->pixel_data = NULL;
  ctx->pixel_data_size = 0;
  return 0;
}

// Function: tbir_read_flags
undefined4 tbir_read_flags(TbirContext *ctx) {
  if (tbir_read_pixeltype(ctx) == 0) {
    return 0;
  }
  if (tbir_read_loadd(ctx) == 0) {
    return 0;
  }
  ctx->flags |= 0x10; // Set flags field present flag
  return 1;
}

// Function: tbir_display_img
undefined4 tbir_display_img(TbirContext *ctx) {
  if (ctx == NULL || tbir_read_magic(ctx) == 0) {
    return 0;
  }
  
  int field_type;
  while ((field_type = tbir_read_typef(ctx)) != 0) {
    switch (field_type) {
      case 0xaaee: // End of image marker
        // Check if all required fields are present
        if (((ctx->flags & 1) != 0) && ((ctx->flags & 0x10) != 0) &&
            ((ctx->flags & 0x10000) != 0) && ((ctx->flags & 0x100000) != 0)) {
          for (int i = 0; i < ctx->pixel_data_size; ++i) {
            if ((i % ctx->width == 0) && (i != 0)) {
              printf("\n");
            }
            printf("%c", ctx->pixel_data[i]);
          }
          printf("\n");
          return 1; // Success
        }
        printf("[ERROR] Missing a required tag\n");
        return 0;
      
      case 0xaadd: // Checksum field
        if ((ctx->flags & 0x100000) != 0) {
          printf("[ERROR] Only one checksum field\n");
          return 0;
        }
        if (tbir_read_chksum(ctx) == 0) {
          return 0;
        }
        break;
      
      case 0xaacc: // Pixels field
        if ((ctx->flags & 0x10000) != 0) {
          printf("[ERROR] Only one pixel set\n");
          return 0;
        }
        if (tbir_read_pixels(ctx) == 0) {
          return 0;
        }
        break;
      
      case 0xaaaa: // Header field
        if ((ctx->flags & 1) != 0) {
          printf("[ERROR] Only one header\n");
          return 0;
        }
        if (tbir_read_header(ctx) == 0) {
          return 0;
        }
        break;
      
      case 0xaabb: // Flags field
        if ((ctx->flags & 0x10) != 0) {
          printf("[ERROR] Only one flags field\n");
          return 0;
        }
        if (tbir_read_flags(ctx) == 0) {
          return 0;
        }
        break;
      
      default:
        printf("Invalid field type: 0x%x\n", field_type);
        return 0;
    }
  }
  return 0; // End of stream without 0xaaee or other error
}

// Function: tbir_read_width
undefined4 tbir_read_width(TbirContext *ctx) {
  uint32_t width_val = 0;
  if (ctx == NULL) {
    return 0;
  }
  if (tbir_read_nbits(ctx, 8, &width_val) == 0) {
    return 0;
  }
  if (width_val < 0x81) { // Max width is 128
    ctx->width = width_val;
    return 1;
  }
  return 0;
}

// Function: tbir_read_height
undefined4 tbir_read_height(TbirContext *ctx) {
  if (ctx == NULL) {
    return 0;
  }
  // param_1 + 0x14 corresponds to &ctx->height
  if (tbir_read_nbits(ctx, 8, (uint32_t*)&ctx->height) == 0) {
    return 0;
  }
  if (ctx->height < 0x81) { // Max height is 128
    return 1;
  }
  return 0;
}

// Function: tbir_read_header
undefined4 tbir_read_header(TbirContext *ctx) {
  if (ctx == NULL) {
    return 0;
  }
  if (tbir_read_width(ctx) == 0) {
    return 0;
  }
  if (tbir_read_height(ctx) == 0) {
    return 0;
  }
  ctx->flags |= 1; // Set header present flag
  return 1;
}

// Function: tbir_read_loadd
undefined4 tbir_read_loadd(TbirContext *ctx) {
  uint32_t orientation_val = 0;
  if (ctx == NULL) {
      return 0;
  }
  if (tbir_read_nbits(ctx, 4, &orientation_val) == 0) {
    return 0;
  }
  if (orientation_val < 9) { // 0-8 are valid orientations
    ctx->orientation = orientation_val; // param_1 + 0x1c corresponds to &ctx->orientation
    ctx->flags |= 0x1000; // Set orientation flag
    return 1;
  }
  return 0;
}

// Function: tbir_read_pixeltype
undefined4 tbir_read_pixeltype(TbirContext *ctx) {
  uint32_t pixel_type_val = 0;
  if (ctx == NULL) {
      return 0;
  }
  if (tbir_read_nbits(ctx, 4, &pixel_type_val) == 0) {
    return 0;
  }
  // pixel_type_val must be < 8.
  // Then, it must be (pixel_type_val < 3 OR pixel_type_val > 5) AND (pixel_type_val != 0).
  // This means allowed values are 1, 2, 6, 7.
  if (pixel_type_val < 8 &&
      ((pixel_type_val < 3 || pixel_type_val > 5) && (pixel_type_val != 0))) {
    ctx->pixel_type = pixel_type_val; // param_1 + 0x18 corresponds to &ctx->pixel_type
    ctx->flags |= 0x100; // Set pixel type flag
    return 1;
  }
  return 0;
}

// Function: tbir_read_typef
int tbir_read_typef(TbirContext *ctx) {
  uint32_t type_val = 0;
  if (tbir_read_nbits(ctx, 0x10, &type_val) == 0) { // Read 16 bits
    printf("da bits failed\n");
    return 0;
  }
  
  // Check if type_val is one of the allowed magic field types
  if (type_val == 0xaaee || type_val == 0xaadd ||
      type_val == 0xaacc || type_val == 0xaaaa || type_val == 0xaabb) {
    return type_val;
  }
  
  printf("wrong one\n");
  return 0;
}

// Function: tbir_read_check
undefined4 tbir_read_check(TbirContext *ctx, int num_bits) {
  if (ctx == NULL || ctx->data_ptr == NULL) {
    return 0;
  }
  if (num_bits < 1 || num_bits >= 0x21) { // num_bits must be between 1 and 32
    return 0;
  }
  // Check if there are enough bits remaining in the stream.
  // num_bits <= ctx->total_bits - (ctx->current_bit_offset + ctx->current_byte_idx * 8)
  if (num_bits <= (ctx->total_bits - (ctx->current_byte_idx * 8 + ctx->current_bit_offset))) {
    return 1;
  }
  return 0;
}

// Function: tbir_read_nbits
undefined4 tbir_read_nbits(TbirContext *ctx, int num_bits, uint32_t *out_val) {
  if (ctx == NULL) {
    return 0;
  }
  if (tbir_read_check(ctx, num_bits) == 0) {
    return 0;
  }
  
  uint32_t result = 0;
  int remaining_bits_in_byte = 8 - ctx->current_bit_offset;
  
  // If num_bits can be read entirely from the current byte
  if (num_bits < remaining_bits_in_byte) { // Original was `param_2 < 8 - iVar3`
    uint8_t current_byte = ctx->data_ptr[ctx->current_byte_idx];
    
    // (current_byte >> ((8 - ctx->current_bit_offset) - num_bits)) & (power_of_2(num_bits) - 1U)
    *out_val = (current_byte >> (remaining_bits_in_byte - num_bits)) & (power_of_2(num_bits) - 1U);
    ctx->current_bit_offset += num_bits;
    return 1;
  } else {
    // Read bits across byte boundaries, or exactly filling the current byte
    for (int i = 0; i < num_bits; ++i) {
      result <<= 1;
      // Read one bit from the current byte at the current bit offset
      result |= (ctx->data_ptr[ctx->current_byte_idx] >> (7U - ctx->current_bit_offset)) & 1U;
      
      ctx->current_bit_offset++;
      if (ctx->current_bit_offset >= 8) { // Move to next byte if current byte is exhausted
        ctx->current_bit_offset = 0;
        ctx->current_byte_idx++;
      }
    }
    *out_val = result;
    return 1;
  }
}

// Function: tbir_read_magic
undefined4 tbir_read_magic(TbirContext *ctx) {
  uint32_t magic_val = 0;
  if (tbir_read_nbits(ctx, 0x20, &magic_val) == 0) { // Read 32 bits
    return 0;
  }
  // -0x4f3b208a (signed int) is 0xbe4c16f2 (unsigned int)
  if (magic_val == 0xbe4c16f2) {
    return 1;
  }
  return 0;
}