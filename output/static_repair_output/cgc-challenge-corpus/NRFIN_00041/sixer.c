#include <stddef.h> // For size_t
#include <stdint.h> // For uintptr_t, uint8_t, etc.
#include <string.h> // For strlen
#include <stdio.h>  // For main function example (printf)

// Structure to hold the sixer context, inferred from array accesses
// In the original code:
// *param_1 refers to bit_offset
// param_1[1] refers to current_pos (char*)
// param_1[2] refers to start_pos (char*)
typedef struct {
    unsigned int bit_offset;
    char *current_pos;
    char *start_pos;
    // The original `undefined local_18[20]` implies space for 5 unsigned int values (20 bytes / 4 bytes/uint).
    // The first three are used above. These two are padding to match the original size.
    unsigned int _padding[2];
} SixerContext;

// Forward declarations for functions used before their definition
unsigned int unarmor_ASCII_char(unsigned char param_1);
unsigned int get_byte_mask(int param_1);
unsigned int get_bits_from_sixer(SixerContext *ctx, unsigned int num_bits_to_get);

// Function: get_byte_mask
unsigned int get_byte_mask(int param_1) {
  unsigned int mask_val;
  switch(param_1) {
  case 0: mask_val = 0x3f; break;
  case 1: mask_val = 0x1f; break;
  case 2: mask_val = 0xf; break;
  case 3: mask_val = 7; break;
  case 4: mask_val = 3; break;
  case 5: mask_val = 1; break;
  default: mask_val = 0;
  }
  return mask_val;
}

// Function: init_sixer
void init_sixer(SixerContext *ctx, char *msg_buffer) {
  ctx->bit_offset = 0;
  ctx->start_pos = msg_buffer;
  ctx->current_pos = msg_buffer;
}

// Function: sixer_strlen
int sixer_strlen(SixerContext *ctx) {
  return (int)(strlen(ctx->current_pos) * 6 - ctx->bit_offset);
}

// Function: get_msg_type
void get_msg_type(char *msg_buffer) {
  SixerContext local_context;
  init_sixer(&local_context, msg_buffer);
  get_bits_from_sixer(&local_context, 3);
}

// Function: get_bits_from_sixer
unsigned int get_bits_from_sixer(SixerContext *ctx, unsigned int num_bits_to_get) {
  unsigned int result_bits = 0;
  unsigned char current_char_val;
  unsigned int mask_val;
  unsigned char bits_this_iter;

  while (0 < (int)num_bits_to_get) {
    current_char_val = unarmor_ASCII_char(*(ctx->current_pos));

    if ((int)num_bits_to_get < 6) {
      bits_this_iter = (6 - ctx->bit_offset < num_bits_to_get) ?
                       (6 - ctx->bit_offset) : (unsigned char)num_bits_to_get;

      mask_val = get_byte_mask(ctx->bit_offset);

      result_bits = ((current_char_val & mask_val) >> (6 - (ctx->bit_offset + bits_this_iter))) |
                    (result_bits << bits_this_iter);

      if (bits_this_iter + ctx->bit_offset == 6) {
        ctx->current_pos++;
        ctx->bit_offset = 0;
      } else {
        ctx->bit_offset += bits_this_iter;
      }
      num_bits_to_get -= bits_this_iter;
    } else {
      ctx->current_pos++;
      if (ctx->bit_offset == 0) {
        result_bits = current_char_val + result_bits * 0x40;
        num_bits_to_get -= 6;
      } else {
        mask_val = get_byte_mask(ctx->bit_offset);
        result_bits = (mask_val & current_char_val) | (result_bits << (6U - ctx->bit_offset));
        num_bits_to_get = (num_bits_to_get + ctx->bit_offset) - 6;
        ctx->bit_offset = 0;
      }
    }
  }
  return result_bits;
}

// Function: sixer_bits_twos_to_sint
unsigned int sixer_bits_twos_to_sint(unsigned int value, unsigned int mask_for_sign_bit) {
  if ((value & mask_for_sign_bit) == 0) { // Check if sign bit is 0 (positive)
    return value;
  } else { // Negative
    return -( (mask_for_sign_bit - 1) & (unsigned int)-(int)value );
  }
}

// Function: sixer_bits_to_ASCII_str_char
unsigned int sixer_bits_to_ASCII_str_char(unsigned char param_1) {
  if (param_1 < 0x40) {
    return (param_1 < 0x20) ? (param_1 + 0x40) : (unsigned int)param_1;
  } else {
    return 0xffffffff;
  }
}

// Function: unarmor_ASCII_char
unsigned int unarmor_ASCII_char(unsigned char param_1) {
  if ((param_1 < 0x78) && ((param_1 < 0x58 || (0x5f < param_1)))) {
    unsigned char temp_val = param_1 - 0x30;
    if (0x27 < temp_val) {
      temp_val = param_1 - 0x38;
    }
    return (unsigned int)temp_val;
  } else {
    return 0xffffffff;
  }
}

// Main function for compilation and basic demonstration
int main() {
    char test_msg[] = "PQRXYZabcdefghijklmnop"; // Example message
    SixerContext ctx;
    unsigned int bits_read;
    unsigned int signed_val;

    printf("--- Initializing Sixer Context ---\n");
    init_sixer(&ctx, test_msg);
    printf("Initial bit_offset: %u\n", ctx.bit_offset);
    printf("Initial current_pos: %s\n", ctx.current_pos);

    printf("\n--- Testing sixer_strlen ---\n");
    int len = sixer_strlen(&ctx);
    printf("sixer_strlen of '%s': %d\n", test_msg, len);

    printf("\n--- Testing get_byte_mask ---\n");
    printf("get_byte_mask(0): 0x%x\n", get_byte_mask(0)); // Expected: 0x3f
    printf("get_byte_mask(3): 0x%x\n", get_byte_mask(3)); // Expected: 0x7

    printf("\n--- Testing unarmor_ASCII_char ---\n");
    printf("unarmor_ASCII_char('P'): 0x%x\n", unarmor_ASCII_char('P')); // 'P' is 0x50, 0x50-0x38 = 0x18 (24)
    printf("unarmor_ASCII_char('0'): 0x%x\n", unarmor_ASCII_char('0')); // '0' is 0x30, 0x30-0x30 = 0x0 (0)
    printf("unarmor_ASCII_char('Z'): 0x%x\n", unarmor_ASCII_char('Z')); // 'Z' is 0x5A, 0x5A-0x38 = 0x22 (34)
    printf("unarmor_ASCII_char('a'): 0x%x\n", unarmor_ASCII_char('a')); // 'a' is 0x61, 0x61-0x38 = 0x29 (41)

    printf("\n--- Testing get_bits_from_sixer ---\n");
    // Re-initialize context to read from the start
    init_sixer(&ctx, test_msg);
    printf("Reading 3 bits from '%s' (P = 24):\n", ctx.current_pos);
    bits_read = get_bits_from_sixer(&ctx, 3);
    printf("Bits read: %u, current_pos: %s, bit_offset: %u\n", bits_read, ctx.current_pos, ctx.bit_offset); // Expect 24 (0b11000) -> 0b110 -> 6

    printf("Reading 6 bits from current_pos '%s' (Q = 25):\n", ctx.current_pos);
    bits_read = get_bits_from_sixer(&ctx, 6);
    printf("Bits read: %u, current_pos: %s, bit_offset: %u\n", bits_read, ctx.current_pos, ctx.bit_offset); // Expect 25 (0b11001)

    printf("\n--- Testing sixer_bits_twos_to_sint ---\n");
    // Example: 3-bit two's complement. Sign bit is the 3rd bit (mask 0b100 = 4)
    signed_val = sixer_bits_twos_to_sint(6, 4); // 0b110 -> -2
    printf("sixer_bits_twos_to_sint(6, 4): %u (expected -2)\n", signed_val); // Will print large unsigned int for -2
    printf("sixer_bits_twos_to_sint(3, 4): %u (expected 3)\n", six_bits_twos_to_sint(3, 4)); // 0b011 -> 3

    printf("\n--- Testing sixer_bits_to_ASCII_str_char ---\n");
    printf("sixer_bits_to_ASCII_str_char(0x10): 0x%x (expected 0x50 'P')\n", sixer_bits_to_ASCII_str_char(0x10));
    printf("sixer_bits_to_ASCII_str_char(0x20): 0x%x (expected 0x20 ' ') \n", sixer_bits_to_ASCII_str_char(0x20));
    printf("sixer_bits_to_ASCII_str_char(0x40): 0x%x (expected 0xffffffff) \n", sixer_bits_to_ASCII_str_char(0x40));

    printf("\n--- Testing get_msg_type (reads 3 bits from start of message) ---\n");
    get_msg_type(test_msg); // Will use a new SixerContext internally and read 3 bits
    printf("get_msg_type called with '%s'. (Output not directly visible, but functions run)\n", test_msg);

    return 0;
}