#include <stddef.h> // For size_t
#include <stdlib.h> // For strtol
#include <string.h> // For strlen

// Forward declaration of transmit, assuming its signature based on usage.
// The snippet ignores its return value.
extern int transmit(void *handle, const void *buf, size_t count, void *status);

// Function: output_number_printf
// handle: A pointer/handle for the transmit function.
// num: The unsigned integer to be printed.
// base: The base for number conversion (e.g., 10 for decimal, 16 for hexadecimal).
// width: A custom width parameter, whose behavior is specific to the original decompiled code.
//        It seems to control padding for the most significant digit.
// flags: Bitmask for formatting options:
//        - Bit 0 (1): If set, use '0' for padding; otherwise, use ' '.
//        - Bit 1 (2): If set, use uppercase letters for hexadecimal digits (A-F);
//                     otherwise, use lowercase (a-f).
int output_number_printf(void *handle, unsigned int num, unsigned int base, int width, unsigned int flags) {
  char digit_char;
  char transmit_status_buffer[8]; // Generic buffer for transmit's last argument

  int digits_printed_recursively = 0;
  if (base <= num) {
    digits_printed_recursively = output_number_printf(handle, num / base, base, width - 1, flags);
    num %= base;
  }

  // Padding logic: This code's specific behavior for padding.
  // Padding is applied before the current digit, but only if it's the most significant digit (first call in recursion chain).
  if ((digits_printed_recursively == 0) && (width > 0)) {
    while (--width > 0) { // The loop runs (original width - 1) times for the MSB
      char pad_char = ((flags & 1) == 0) ? ' ' : '0'; // Space or zero padding
      transmit(handle, &pad_char, 1, transmit_status_buffer);
    }
  }

  // Convert the current digit (0 to base-1) to its character representation.
  digit_char = (char)num;
  if ((flags & 2) == 0) { // Lowercase hex (or decimal)
    if (num < 10) {
      digit_char += '0';
    } else {
      digit_char += 'W'; // 'W' (0x57) + 10 = 'a' (0x61) for hex digits A-F
    }
  } else { // Uppercase hex
    if (num < 10) {
      digit_char += '0';
    } else {
      digit_char += '7'; // '7' (0x37) + 10 = 'A' (0x41) for hex digits A-F
    }
  }
  transmit(handle, &digit_char, 1, transmit_status_buffer);

  return digits_printed_recursively + 1; // Return count of digits printed in this branch + 1 for current digit
}

// Function: eprintf
// handle: A pointer/handle for the transmit function.
// format_str: The format string, similar to printf but uses '#' for specifiers.
// arg_ptr_base: A base pointer to an array of custom argument structures.
//               Each argument structure is 0x20 (32) bytes long.
// num_args_available: The total number of arguments available in the array.
unsigned int eprintf(void *handle, char *format_str, int arg_ptr_base, unsigned int num_args_available) {
  char current_char;
  char *strtol_end_ptr;
  char transmit_status_buffer[8]; // Generic buffer for transmit's last argument

  unsigned int total_chars_transmitted = 0; // This variable is always 0 in the original snippet, not used for actual count.
  unsigned int flags = 0;                   // Flags for output_number_printf (zero-padding, uppercase hex).
  long width_specifier = 0;                 // Width for output_number_printf.
  unsigned int arg_idx = 0;                 // Index for accessing arguments from the arg_ptr_base array.

  // Main loop to iterate through the format string
  while (*format_str != '\0') {
    current_char = *format_str++;

    // Print literal characters until a '#' is encountered.
    if (current_char != '#') {
      transmit(handle, &current_char, 1, transmit_status_buffer);
      continue; // Continue with the next character in the format string.
    }

    // A '#' has been found, indicating the start of a format specifier.
    // Reset flags and width for the new specifier.
    flags = 0;
    width_specifier = 0;

    // Parse '0' flags (for zero-padding).
    while (*format_str == '0') {
      flags |= 1; // Set zero-padding flag.
      format_str++; // Consume the '0'.
    }

    // Parse the width specifier (a sequence of digits).
    // `strtol` is used to parse the number from the current `format_str` position.
    if (*format_str >= '1' && *format_str <= '9') {
      width_specifier = strtol(format_str, &strtol_end_ptr, 10);
      format_str = strtol_end_ptr; // Update `format_str` to point after the parsed number.
    }

    // Get the actual type specifier character.
    current_char = *format_str;
    if (current_char == '\0') {
      // The format string ended unexpectedly after '#' and optional width/flags.
      // The outer while loop condition handles this termination naturally.
      break;
    }
    format_str++; // Consume the type specifier character.

    if (current_char == '#') {
      // If the specifier itself is '#', print it literally.
      transmit(handle, &current_char, 1, transmit_status_buffer);
      // Flags and width_specifier are already reset at the beginning of this specifier block
      // for the next iteration, so no special action is needed here to clear them.
    } else {
      // Check if there are enough arguments available.
      if (num_args_available <= arg_idx) {
        return 0xFFFFFFFF; // Error: Not enough arguments.
      }

      // Calculate the pointer to the current argument's structure.
      // Each argument structure is 0x20 bytes long.
      int current_arg_struct_ptr = arg_ptr_base + arg_idx * 0x20;
      // The argument type is stored at offset 0x14 within the structure.
      int arg_type = *(int *)(current_arg_struct_ptr + 0x14);
      // The argument value is typically stored at offset 0x18.
      arg_idx++; // Increment the argument index for the next specifier.

      // Process the format specifier.
      switch (current_char) {
        case 'X':
          flags |= 2; // Set uppercase hex flag.
          // Fallthrough to 'x' to handle hex printing.
        case 'x': {
          unsigned int val = (arg_type == 0) ? 0 : *(unsigned int *)(current_arg_struct_ptr + 0x18);
          output_number_printf(handle, val, 16, width_specifier, flags);
          break;
        }
        case 'c': {
          char val = (arg_type == 0) ? 0 : (char)*(unsigned int *)(current_arg_struct_ptr + 0x18);
          transmit(handle, &val, 1, transmit_status_buffer);
          break;
        }
        case 'd': {
          int val = (arg_type == 0) ? 0 : *(int *)(current_arg_struct_ptr + 0x18);
          if (val < 0) {
            char minus_char = '-';
            transmit(handle, &minus_char, 1, transmit_status_buffer);
            val = -val; // Make value positive for output_number_printf.
          }
          output_number_printf(handle, (unsigned int)val, 10, width_specifier, flags);
          break;
        }
        case 's': {
          char *s_val = (arg_type == 0) ? "" : *(char **)(current_arg_struct_ptr + 0x18);
          size_t s_len = strlen(s_val);
          for (unsigned int i = 0; i < s_len; ++i) {
            transmit(handle, &s_val[i], 1, transmit_status_buffer);
          }
          break;
        }
        case 'u': {
          unsigned int val = (arg_type == 0) ? 0 : *(unsigned int *)(current_arg_struct_ptr + 0x18);
          output_number_printf(handle, val, 10, width_specifier, flags);
          break;
        }
        default: { // Unrecognized specifier, print it literally.
          transmit(handle, &current_char, 1, transmit_status_buffer);
          break;
        }
      }
    }
  }

  return total_chars_transmitted; // As per original snippet, this always returns 0.
}