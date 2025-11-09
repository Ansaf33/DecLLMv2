#include <ctype.h>   // For isspace
#include <stdbool.h> // For bool type
#include <stddef.h>  // For NULL

char * crackaddr(char *input_str, char *output_buffer) {
  char *write_ptr = output_buffer;
  char *buffer_end_reserve_marker = output_buffer + 0x3fb; // 1019 bytes offset from start

  if (input_str == NULL) {
    return NULL;
  }

  // Skip leading whitespace
  while (*input_str != '\0' && isspace((unsigned char)*input_str)) {
    input_str++;
  }

  bool flag_A_found = false;
  bool flag_C_found = false;
  bool flag_D_toggled = false;
  int counter_B_nesting = 0;
  int counter_b_output = 0;
  int counter_a_output_limit = 0;

  char *read_ptr = input_str;
  char *next_read_ptr_candidate = input_str + 1;

  while (*read_ptr != '\0') {
    bool buffer_full_or_reserved = (buffer_end_reserve_marker <= write_ptr);

    if (!buffer_full_or_reserved && counter_a_output_limit > 0) {
      *write_ptr = *read_ptr;
      write_ptr++;
    }

    if (*read_ptr == 'A') {
      counter_a_output_limit = 0;
      if (!flag_A_found) {
        flag_A_found = true;
        buffer_end_reserve_marker--;
      }
    }
    if (*read_ptr == 'a') {
      counter_a_output_limit++;
      if (counter_a_output_limit < 2) {
        *write_ptr = 'a';
        write_ptr++;
      }
      if (flag_A_found) {
        flag_A_found = false;
        buffer_end_reserve_marker++;
      }
    }
    if (*read_ptr == 'B') {
      counter_B_nesting++;
      if (!buffer_full_or_reserved) {
        counter_b_output++;
        buffer_end_reserve_marker--;
        counter_a_output_limit++;
      }
    }
    if (*read_ptr == 'b' && counter_B_nesting > 0) {
      counter_B_nesting--;
      counter_a_output_limit--;
      if (!buffer_full_or_reserved) {
        counter_b_output--;
        buffer_end_reserve_marker++;
      }
    } else {
      if (*read_ptr == 'b' && !buffer_full_or_reserved && counter_a_output_limit > 0) {
        write_ptr--;
      }
      if (*read_ptr == 'C' && !flag_C_found) {
        flag_C_found = true;
      }
      if (*read_ptr == 'c' && flag_C_found) {
        flag_C_found = false;
        buffer_end_reserve_marker++;
      }
      if (*read_ptr == 'D') {
        flag_D_toggled = !flag_D_toggled;
      }

      if (*read_ptr == 'F' && !buffer_full_or_reserved) {
        if (*next_read_ptr_candidate != '\0') {
          *write_ptr = *next_read_ptr_candidate;
          next_read_ptr_candidate = read_ptr + 2;
          write_ptr++;
        }
      } else {
        if (*read_ptr == 'G' && *next_read_ptr_candidate != '\0') {
          next_read_ptr_candidate = read_ptr + 2;
        }
      }
    }
    read_ptr = next_read_ptr_candidate;
    next_read_ptr_candidate = read_ptr + 1;
  }

  if (flag_A_found) {
    *write_ptr = 'a';
    write_ptr++;
  }
  while (counter_b_output > 0 && (write_ptr < buffer_end_reserve_marker - 2)) {
    *write_ptr = 'b';
    counter_b_output--;
    write_ptr++;
  }
  if (flag_C_found) {
    *write_ptr = 'c';
    write_ptr++;
  }
  if (flag_D_toggled) {
    *write_ptr = 'D';
    write_ptr++;
  }
  *write_ptr = '\0';

  return output_buffer;
}