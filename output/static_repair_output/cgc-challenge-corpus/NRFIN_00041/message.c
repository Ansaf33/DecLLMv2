#include <stdio.h>   // For snprintf
#include <stdint.h>  // For uint32_t (used in sixer_context_t)
#include <string.h>  // Not explicitly used in the provided snippet but good for char* handling

// Custom type definitions based on common decompilation output
typedef unsigned char byte;
typedef unsigned int undefined4; // Assuming 4 bytes, unsigned
typedef unsigned int uint;       // Assuming unsigned int

// Dummy sixer context structure
// This structure needs to hold enough information to simulate the bitstream.
// For the provided code, it needs a 'data' source and a 'bit_offset'.
typedef struct {
    uint32_t data_source; // The undefined4 parameter passed to init_sixer
    int bit_offset;       // Current bit position
    int total_bits;       // Total length of the message in bits, for sixer_strlen
} sixer_context_t;

// Dummy declarations and implementations for external functions and global variables
// These are minimal implementations to allow the provided code to compile.
// In a real system, these would have full logic.

// get_msg_type: Returns a byte representing the message type.
// The original code checks if it matches *(byte *)(param_2 + 0x11).
// For compilation, it can return any byte.
byte get_msg_type(undefined4 param_1) {
    (void)param_1; // Suppress unused parameter warning
    // In a real scenario, this would parse `param_1` to determine message type.
    // For compilation, let's return a value that allows `to_english` to proceed.
    // E.g., return 1, 4, or 5 to hit parse_msg_type_X.
    return 1; // Example: return 1 to test parse_msg_type_1
}

// init_sixer: Initializes the sixer context with message data.
// `param_2` is usually a pointer to the raw message data.
void init_sixer(void* sixer_ctx_ptr, undefined4 param_2) {
    sixer_context_t* ctx = (sixer_context_t*)sixer_ctx_ptr;
    ctx->data_source = param_2; // Store the data source
    ctx->bit_offset = 0;       // Start at the beginning of the bitstream
    // For `sixer_strlen`, we need a total length. This is a heuristic for compilation.
    // The original code checks for specific lengths (0x90, 0x8a, 0x180).
    // Let's make `total_bits` dependent on `param_2` in a simple way for the dummy.
    // This is a weak assumption, but allows `sixer_strlen` to return different values.
    if (param_2 == 0x90) ctx->total_bits = 0x90; // For type 1
    else if (param_2 == 0x8a) ctx->total_bits = 0x8a; // For type 4
    else if (param_2 == 0x180) ctx->total_bits = 0x180; // For type 5
    else ctx->total_bits = 0x90; // Default
}

// sixer_strlen: Returns the total number of bits in the sixer message.
int sixer_strlen(void* sixer_ctx_ptr) {
    sixer_context_t* ctx = (sixer_context_t*)sixer_ctx_ptr;
    return ctx->total_bits; // Return the total bits stored during init
}

// get_bits_from_sixer: Extracts `num_bits` from the bitstream.
// This is a simplified implementation. Real bitstream parsing is more complex.
int get_bits_from_sixer(void* sixer_ctx_ptr, int num_bits) {
    sixer_context_t* ctx = (sixer_context_t*)sixer_ctx_ptr;
    if (ctx->bit_offset + num_bits > ctx->total_bits) {
        // Attempt to read beyond buffer, return an error value or handle.
        // The original code checks for < 0, so returning -1 is a good fit.
        return -1;
    }
    // Simulate reading bits from the data_source.
    // This is a very basic simulation and assumes data_source can be directly bit-shifted.
    // In reality, it would read from a byte array.
    int value = (ctx->data_source >> ctx->bit_offset) & ((1 << num_bits) - 1);
    ctx->bit_offset += num_bits;
    return value;
}

// sixer_bits_twos_to_sint: Converts a two's complement value to a signed integer.
int sixer_bits_twos_to_sint(int value, int sign_bit_mask) {
    if ((value & sign_bit_mask) != 0) { // Check if the highest bit (sign bit) is set
        // Perform two's complement conversion by subtracting the full range value
        return value - (sign_bit_mask << 1);
    }
    return value;
}

// sixer_bits_to_ASCII_str_char: Converts a 6-bit value to an ASCII character.
char sixer_bits_to_ASCII_str_char(int value) {
    // This mapping is specific to the six-bit character set used in AIS messages.
    // Common mappings include 0-25 for A-Z, 0x30-0x39 for 0-9, etc.
    // The original code checks for 0x40 ('@') as a sentinel, implying a specific charset.
    // For compilation, we'll return a simple mapping.
    if (value >= 0x20 && value <= 0x5F) { // Range for printable ASCII including '@'
        return (char)value;
    }
    return '@'; // Default for unknown or sentinel values
}

// Dummy global variables, likely pointers to string arrays or string literals.
// Defined as arrays of const char* for direct access.
const char* STATUS[] = {
    "Status 0", "Status 1", "Status 2", "Status 3",
    "Status 4", "Status 5", "Status 6", "Status 7", "Status 8"
};

const char* MANEUEVER[] = {
    "Maneuver 0", "Maneuver 1", "Maneuver 2", "Maneuver 3"
};

const char* EPFD[] = {
    "EPFD 0", "EPFD 1", "EPFD 2", "EPFD 3",
    "EPFD 4", "EPFD 5", "EPFD 6", "EPFD 7", "EPFD 8"
};

// DAT_000141de is used with "~c," and a buffer size of 5. It's likely a short string, e.g., "unk".
const char DAT_000141de[] = "unk"; // "unk" + null terminator is 4 bytes. `snprintf` with 5 bytes fits "unk,"

// Function declarations (forward declarations)
undefined4 parse_msg_type_1(char *param_1, undefined4 param_2);
undefined4 parse_msg_type_4(char *param_1, undefined4 param_2);
undefined4 parse_msg_type_5(char *param_1, undefined4 param_2);

// Function: to_english
undefined4 to_english(undefined4 param_1, void *param_2) { // Changed int to void* for pointer safety
  // Check the byte at offset 0x10 of the structure pointed to by param_2
  if (*(char *)((char *)param_2 + 0x10) != '\x03') { // Cast param_2 to char* for byte arithmetic
    return 0xffffffcd;
  }

  // Get message type and compare with byte at offset 0x11
  byte msg_type = get_msg_type(*(undefined4 *)((char *)param_2 + 0x14));
  if (msg_type != *(byte *)((char *)param_2 + 0x11)) {
    return 0xffffffcd;
  }

  // Dispatch based on message type
  if (msg_type == 5) {
    return parse_msg_type_5(param_1, *(undefined4 *)((char *)param_2 + 0x14));
  } else if (msg_type == 1) {
    return parse_msg_type_1(param_1, *(undefined4 *)((char *)param_2 + 0x14));
  } else if (msg_type == 4) {
    return parse_msg_type_4(param_1, *(undefined4 *)((char *)param_2 + 0x14));
  }
  // No matching type
  return 0xffffffcd;
}

// Function: parse_msg_type_1
undefined4 parse_msg_type_1(char *param_1, undefined4 param_2) {
  sixer_context_t sixer_ctx;
  char *current_pos = param_1;

  init_sixer(&sixer_ctx, param_2);

  if (sixer_strlen(&sixer_ctx) != 0x90) {
    return 0xffffffcd;
  }

  int bits_val = get_bits_from_sixer(&sixer_ctx, 3);
  if (bits_val != 1) {
    return 0xffffffcd;
  }

  current_pos += snprintf(current_pos, 8, "~c,", "type 1");

  bits_val = get_bits_from_sixer(&sixer_ctx, 0x1e);
  if (bits_val < 0) {
    return 0xffffffcd;
  }
  current_pos += snprintf(current_pos, 0xb, "~n,", bits_val);

  bits_val = get_bits_from_sixer(&sixer_ctx, 4);
  if ((bits_val < 0) || (8 < bits_val)) {
    return 0xffffffcd;
  }
  current_pos += snprintf(current_pos, 0x1c, "~c,", STATUS[bits_val]);

  bits_val = get_bits_from_sixer(&sixer_ctx, 8);
  if (0x80 < bits_val) {
    bits_val = sixer_bits_twos_to_sint(bits_val, 0x80);
  }
  if (bits_val == 0) {
    current_pos += snprintf(current_pos, 0xd, "~c,", "not turning");
  } else if (bits_val < 0) {
    current_pos += snprintf(current_pos, 0x12, "~c ~n,", "turning left", -bits_val);
  } else {
    current_pos += snprintf(current_pos, 0x13, "~c ~n,", "turning right", bits_val);
  }

  bits_val = get_bits_from_sixer(&sixer_ctx, 10);
  if (bits_val == 0x3ff) {
    current_pos += snprintf(current_pos, 0xb, "~c,", "speed unk");
  } else if (bits_val < 0x3fd) {
    current_pos += snprintf(current_pos, 8, "~nkts,", bits_val / 10);
  } else {
    current_pos += snprintf(current_pos, 9, "~c,", ">102kts");
  }

  get_bits_from_sixer(&sixer_ctx, 1); // Value not used

  bits_val = get_bits_from_sixer(&sixer_ctx, 0x1b);
  if (0x535020 < bits_val) {
    bits_val = sixer_bits_twos_to_sint(bits_val, 0x800000);
  }
  if (bits_val == 0x535020) {
    current_pos += snprintf(current_pos, 9, "~c,", "lat unk");
  } else if (bits_val == 0) {
    current_pos += snprintf(current_pos, 3, "~n,", 0);
  } else if (bits_val < 1) {
    current_pos += snprintf(current_pos, 10, "~nS,", -bits_val);
  } else {
    current_pos += snprintf(current_pos, 10, "~nN,", bits_val);
  }

  bits_val = get_bits_from_sixer(&sixer_ctx, 0x1c);
  if (0xa5b5e0 < bits_val) {
    bits_val = sixer_bits_twos_to_sint(bits_val, 0x1000000);
  }
  if (bits_val == 0xa5b5e0) {
    current_pos += snprintf(current_pos, 9, "~c,", "lon unk");
  } else if (bits_val == 0) {
    current_pos += snprintf(current_pos, 3, "~n,", 0);
  } else if (bits_val < 1) {
    current_pos += snprintf(current_pos, 0xb, "~nW,", -bits_val);
  } else {
    current_pos += snprintf(current_pos, 0xb, "~nE,", bits_val);
  }

  bits_val = get_bits_from_sixer(&sixer_ctx, 0xc);
  if (bits_val == 0xe10) {
    current_pos += snprintf(current_pos, 0xc, "~c,", "course unk");
  } else {
    if (0xe06 < bits_val) {
      return 0xffffffcd;
    }
    current_pos += snprintf(current_pos, 8, "c:~n,", bits_val / 10);
  }

  bits_val = get_bits_from_sixer(&sixer_ctx, 9);
  if (bits_val == 0x1ff) {
    current_pos += snprintf(current_pos, 0xd, "~c,", "heading unk");
  } else {
    if (0x167 < bits_val) {
      return 0xffffffcd;
    }
    current_pos += snprintf(current_pos, 10, "h:~n TN,", bits_val);
  }

  get_bits_from_sixer(&sixer_ctx, 6); // Value not used

  bits_val = get_bits_from_sixer(&sixer_ctx, 2);
  if (bits_val == 3) {
    return 0xffffffcd;
  }
  snprintf(current_pos, 0x18, "~c.", MANEUEVER[bits_val]);
  return 0;
}

// Function: parse_msg_type_4
undefined4 parse_msg_type_4(char *param_1, undefined4 param_2) {
  sixer_context_t sixer_ctx;
  char *current_pos = param_1;

  init_sixer(&sixer_ctx, param_2);

  if (sixer_strlen(&sixer_ctx) != 0x8a) {
    return 0xffffffcd;
  }

  int bits_val = get_bits_from_sixer(&sixer_ctx, 3);
  if (bits_val != 4) {
    return 0xffffffcd;
  }

  current_pos += snprintf(current_pos, 8, "~c,", "type 4");

  bits_val = get_bits_from_sixer(&sixer_ctx, 0x1e);
  if (bits_val < 0) {
    return 0xffffffcd;
  }
  current_pos += snprintf(current_pos, 0xb, "~n,", bits_val);

  bits_val = get_bits_from_sixer(&sixer_ctx, 0xe);
  if (bits_val == 0) {
    current_pos += snprintf(current_pos, 5, "~c,", DAT_000141de);
  } else {
    current_pos += snprintf(current_pos, 6, "~n,", bits_val);
  }

  bits_val = get_bits_from_sixer(&sixer_ctx, 4);
  if (bits_val >= 0xd) {
    return 0xffffffcd;
  }
  if (bits_val == 0) {
    current_pos += snprintf(current_pos, 5, "~c,", DAT_000141de);
  } else {
    current_pos += snprintf(current_pos, 4, "~n,", bits_val);
  }

  bits_val = get_bits_from_sixer(&sixer_ctx, 5);
  if (bits_val >= 0x20) {
    return 0xffffffcd;
  }
  if (bits_val == 0) {
    current_pos += snprintf(current_pos, 5, "~c,", DAT_000141de);
  } else {
    current_pos += snprintf(current_pos, 4, "~n,", bits_val);
  }

  bits_val = get_bits_from_sixer(&sixer_ctx, 5);
  if (bits_val >= 0x19) {
    return 0xffffffcd;
  }
  if (bits_val == 0x18) {
    current_pos += snprintf(current_pos, 5, "~c,", DAT_000141de);
  } else {
    current_pos += snprintf(current_pos, 4, "~n,", bits_val);
  }

  bits_val = get_bits_from_sixer(&sixer_ctx, 6);
  if (bits_val >= 0x3d) {
    return 0xffffffcd;
  }
  if (bits_val == 0x3c) {
    current_pos += snprintf(current_pos, 5, "~c,", DAT_000141de);
  } else {
    current_pos += snprintf(current_pos, 4, "~n,", bits_val);
  }

  bits_val = get_bits_from_sixer(&sixer_ctx, 6);
  if (bits_val >= 0x3d) {
    return 0xffffffcd;
  }
  if (bits_val == 0x3c) {
    current_pos += snprintf(current_pos, 5, "~c,", DAT_000141de);
  } else {
    current_pos += snprintf(current_pos, 4, "~n,", bits_val);
  }

  get_bits_from_sixer(&sixer_ctx, 1); // Value not used

  bits_val = get_bits_from_sixer(&sixer_ctx, 0x1b);
  if (0x535020 < bits_val) {
    bits_val = sixer_bits_twos_to_sint(bits_val, 0x800000);
  }
  if (bits_val == 0x535020) {
    current_pos += snprintf(current_pos, 9, "~c,", "lat unk");
  } else if (bits_val == 0) {
    current_pos += snprintf(current_pos, 3, "~n,", 0);
  } else if (bits_val < 1) {
    current_pos += snprintf(current_pos, 10, "~nS,", -bits_val);
  } else {
    current_pos += snprintf(current_pos, 10, "~nN,", bits_val);
  }

  bits_val = get_bits_from_sixer(&sixer_ctx, 0x1c);
  if (0xa5b5e0 < bits_val) {
    bits_val = sixer_bits_twos_to_sint(bits_val, 0x1000000);
  }
  if (bits_val == 0xa5b5e0) {
    current_pos += snprintf(current_pos, 9, "~c,", "lon unk");
  } else if (bits_val == 0) {
    current_pos += snprintf(current_pos, 3, "~n,", 0);
  } else if (bits_val < 1) {
    current_pos += snprintf(current_pos, 0xb, "~nW,", -bits_val);
  } else {
    current_pos += snprintf(current_pos, 0xb, "~nE,", bits_val);
  }

  bits_val = get_bits_from_sixer(&sixer_ctx, 4);
  if (bits_val >= 9) {
    return 0xffffffcd;
  }
  snprintf(current_pos, 0x1e, "~c.", EPFD[bits_val]);
  return 0;
}

// Function: parse_msg_type_5
undefined4 parse_msg_type_5(char *param_1, undefined4 param_2) {
  sixer_context_t sixer_ctx;
  char *current_pos = param_1;
  int is_padding_active = 0; // Renamed from local_18

  init_sixer(&sixer_ctx, param_2);

  if (sixer_strlen(&sixer_ctx) != 0x180) {
    return 0xffffffcd;
  }

  uint bits_val_u = get_bits_from_sixer(&sixer_ctx, 3);
  if (bits_val_u != 5) {
    return 0xffffffcd;
  }

  current_pos += snprintf(current_pos, 8, "~c,", "type 5");

  bits_val_u = get_bits_from_sixer(&sixer_ctx, 0x1e);
  if ((int)bits_val_u < 0) {
    return 0xffffffcd;
  }
  current_pos += snprintf(current_pos, 0xb, "~n,", bits_val_u);

  bits_val_u = get_bits_from_sixer(&sixer_ctx, 0x1e);
  if ((int)bits_val_u < 0) {
    return 0xffffffcd;
  }
  current_pos += snprintf(current_pos, 0xb, "~n,", bits_val_u);

  is_padding_active = 0;
  for (int i = 0; i < 7; ++i) {
    int char_bits = get_bits_from_sixer(&sixer_ctx, 6);
    if (char_bits < 0) {
      return 0xffffffcd;
    }
    char c = sixer_bits_to_ASCII_str_char(char_bits & 0xff);
    if ((c == '@') || (is_padding_active == 1)) {
      is_padding_active = 1;
    } else {
      *current_pos++ = c;
    }
  }
  *current_pos++ = ',';

  is_padding_active = 0;
  for (int i = 0; i < 0x14; ++i) {
    int char_bits = get_bits_from_sixer(&sixer_ctx, 6);
    if (char_bits < 0) {
      return 0xffffffcd;
    }
    char c = sixer_bits_to_ASCII_str_char(char_bits & 0xff);
    if ((c == '@') || (is_padding_active == 1)) {
      is_padding_active = 1;
    } else {
      *current_pos++ = c;
    }
  }
  *current_pos++ = ',';

  bits_val_u = get_bits_from_sixer(&sixer_ctx, 4);
  if ((int)bits_val_u >= 9) {
    return 0xffffffcd;
  }
  current_pos += snprintf(current_pos, 0x1e, "~c,", EPFD[bits_val_u]);

  bits_val_u = get_bits_from_sixer(&sixer_ctx, 4);
  if ((int)bits_val_u >= 0xd) {
    return 0xffffffcd;
  }
  if (bits_val_u == 0) {
    current_pos += snprintf(current_pos, 5, "~c,", DAT_000141de);
  } else {
    current_pos += snprintf(current_pos, 4, "~n,", bits_val_u);
  }

  bits_val_u = get_bits_from_sixer(&sixer_ctx, 5);
  if ((int)bits_val_u >= 0x20) {
    return 0xffffffcd;
  }
  if (bits_val_u == 0) {
    current_pos += snprintf(current_pos, 5, "~c,", DAT_000141de);
  } else {
    current_pos += snprintf(current_pos, 4, "~n,", bits_val_u);
  }

  bits_val_u = get_bits_from_sixer(&sixer_ctx, 5);
  if ((int)bits_val_u >= 0x19) {
    return 0xffffffcd;
  }
  if (bits_val_u == 0x18) {
    current_pos += snprintf(current_pos, 5, "~c,", DAT_000141de);
  } else {
    current_pos += snprintf(current_pos, 4, "~n,", bits_val_u);
  }

  bits_val_u = get_bits_from_sixer(&sixer_ctx, 6);
  if ((int)bits_val_u >= 0x3d) {
    return 0xffffffcd;
  }
  if (bits_val_u == 0x3c) {
    current_pos += snprintf(current_pos, 5, "~c,", DAT_000141de);
  } else {
    current_pos += snprintf(current_pos, 4, "~n,", bits_val_u);
  }

  bits_val_u = get_bits_from_sixer(&sixer_ctx, 8);
  current_pos += snprintf(current_pos, 5, "~n,", bits_val_u);

  is_padding_active = 0;
  for (int i = 0; i < 0x14; ++i) {
    int char_bits = get_bits_from_sixer(&sixer_ctx, 6);
    if (char_bits < 0) {
      return 0xffffffcd;
    }
    char c = sixer_bits_to_ASCII_str_char(char_bits & 0xff);
    if ((c == '@') || (is_padding_active == 1)) {
      is_padding_active = 1;
    } else {
      *current_pos++ = c;
    }
  }
  *current_pos = '.'; // Final character, no increment
  return 0;
}