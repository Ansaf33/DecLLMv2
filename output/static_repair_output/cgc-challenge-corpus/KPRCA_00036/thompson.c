#include <stdio.h>    // For fprintf, stdout, stderr
#include <stdlib.h>   // For malloc, free, calloc
#include <string.h>   // For strlen, strcpy, memcpy, strchr
#include <ctype.h>    // For isprint
#include <stdbool.h>  // For bool type

// Function: normalize_infix
// Takes a pointer to an unsigned char* (regex string) and modifies it in place.
// The string might be reallocated if character sets are expanded.
// Returns 0 on success, or a negative error code.
int normalize_infix(unsigned char **param_1) {
  unsigned char *current_str_base = *param_1; // Base pointer to the current string in memory, might change
  unsigned char *current_char_ptr = current_str_base; // Iterator for the string

  unsigned char temp_expanded_bracket[204]; // Buffer for expanded character sets
  int bracket_char_count = 0; // Index for temp_expanded_bracket

  // Loop through the regex string until null terminator or error
  while (*current_char_ptr != 0) {
    // Check if character is printable
    if (isprint((int)*current_char_ptr) == 0) {
      return -4; // Error: Non-printable character
    }

    // Check for special regex operators: |*+?()
    char *special_char_match = strchr("|*+?()", (int)*current_char_ptr);
    if (special_char_match != NULL) {
      // Mark special operators by setting MSB (adding 0x80)
      *current_char_ptr += 0x80;
    } else {
      // Handle escaped characters or character sets
      if (*current_char_ptr == '\\') { // 0x5c
        if (current_char_ptr[1] == 0) {
          return -4; // Error: Incomplete escape sequence
        }
        // Check if the escaped character is a special one
        if (strchr("|*+?()[]\\", (int)current_char_ptr[1]) == NULL) {
          return -4; // Error: Invalid escaped character
        }
        // Replace '\' with a sentinel (0xdc) and advance to the escaped char
        *current_char_ptr = 0xdc; // Sentinel for escaped char
        current_char_ptr++; // Advance to the character that was escaped
      } else if (*current_char_ptr == '[') { // 0x5b
        unsigned char *bracket_start_ptr = current_char_ptr; // Mark the start of the bracket
        bool closing_bracket_found = false;
        bracket_char_count = 0; // Reset index for temp_expanded_bracket
        temp_expanded_bracket[bracket_char_count++] = 0xa8; // Sentinel for '[' (Open Character Set)

        // Loop through characters inside the bracket
        unsigned char *inner_char_ptr = current_char_ptr + 1; // Start after '['
        while (*inner_char_ptr != 0) {
          if (*inner_char_ptr == ']') { // 0x5d
            if (bracket_char_count < 2) { // Must have at least one character (0xa8 + char)
              return -4; // Error: Empty character set or only [ ]
            }
            closing_bracket_found = true;
            current_char_ptr = inner_char_ptr + 1; // Advance main iterator past ']'
            break; // Exit inner while loop
          } else {
            // Ensure there's enough space in temp_expanded_bracket for char, 0xfc, 0xa9, and null terminator
            if (bracket_char_count >= sizeof(temp_expanded_bracket) - 3) {
              return -4; // Error: Character set too long
            }
            temp_expanded_bracket[bracket_char_count++] = *inner_char_ptr;
            temp_expanded_bracket[bracket_char_count++] = 0xfc; // Concatenation sentinel (OR operator)
            inner_char_ptr++; // Advance inner iterator
          }
        }

        if (!closing_bracket_found) {
          return -4; // Error: Unclosed character set
        }

        // Adjust temp_expanded_bracket: remove last 0xfc and add 0xa9 (Close Character Set)
        if (bracket_char_count > 0) {
            bracket_char_count--; // Remove the last 0xfc
        }
        temp_expanded_bracket[bracket_char_count++] = 0xa9; // Sentinel for ']'
        temp_expanded_bracket[bracket_char_count] = 0; // Null terminate the expanded string

        // Reallocate memory for the expanded string
        size_t prefix_len = bracket_start_ptr - current_str_base;
        size_t expanded_len = strlen((char *)temp_expanded_bracket);
        unsigned char *suffix_ptr = current_char_ptr; // Points to character after ']'
        size_t suffix_len = strlen((char *)suffix_ptr);

        size_t new_total_len = prefix_len + expanded_len + suffix_len + 1;
        unsigned char *new_regex_str = (unsigned char *)malloc(new_total_len);
        if (!new_regex_str) {
          return -1; // Memory allocation error
        }

        memcpy(new_regex_str, current_str_base, prefix_len);
        memcpy(new_regex_str + prefix_len, temp_expanded_bracket, expanded_len);
        memcpy(new_regex_str + prefix_len + expanded_len, suffix_ptr, suffix_len + 1);

        free(current_str_base);
        current_str_base = new_regex_str;
        *param_1 = current_str_base; // Update the caller's pointer to the new string

        // Adjust main iterator to point to the character *before* the suffix.
        // The outer loop's `current_char_ptr++` will then correctly point to the first char of the suffix.
        current_char_ptr = current_str_base + prefix_len + expanded_len - 1;
      }
    }
    current_char_ptr++; // Move to the next character in the (potentially new) string
  }

  *param_1 = current_str_base; // Ensure the final pointer is returned
  return 0; // Success
}

// Function: is_nonconcat_char
// Checks if a character is an operator that does not imply concatenation with the preceding character.
// Returns 1 if non-concatenating, 0 otherwise.
int is_nonconcat_char(unsigned char c) {
  // These are the normalized internal representations of operators.
  // 0xfc: '|' (OR operator)
  // 0xaa: '*' (Kleene star operator)
  // 0xab: '+' (Kleene plus operator)
  // 0xbf: '?' (Question mark operator)
  // 0xa9: ')' (Closing parenthesis)
  // 0xdc: Escaped character sentinel (e.g., '\a' becomes 0xdc 'a')
  if (c == 0xfc || c == 0xaa || c == 0xab || c == 0xbf || c == 0xa9 || c == 0xdc) {
    return 1;
  }
  return 0;
}

// Function: debug_print_re
// Prints the normalized regex string in a human-readable format and its internal byte representation.
void debug_print_re(unsigned char *param_1) {
  size_t original_len = strlen((char *)param_1);
  unsigned char *printable_re = (unsigned char *)malloc(original_len + 1);
  if (!printable_re) {
      fprintf(stderr, "Memory allocation failed for debug_print_re\n");
      return;
  }

  unsigned char *dest_ptr = printable_re;
  for (unsigned char *src_ptr = param_1; *src_ptr != 0; src_ptr++) {
    if (*src_ptr < 0x81) { // If not a normalized operator (MSB not set)
      *dest_ptr = *src_ptr;
    } else { // It's a normalized operator, undo the 0x80 addition
      *dest_ptr = *src_ptr - 0x80;
    }
    dest_ptr++;
  }
  *dest_ptr = 0; // Null terminate the printable string

  fprintf(stdout, "Normalized re = %s\n", printable_re);
  fprintf(stdout, "Original re = %s\n", param_1); // Prints the internal byte representation
  free(printable_re);
}

// Function: retorpn (Regular Expression to Reverse Polish Notation)
// param_1_original_copy: Input regex string (a copy that will be modified by normalize_infix)
// max_len: Maximum length for the output RPN string (used for buffer allocation)
// param_3: Pointer to store the output RPN string
// Returns 0 on success, or a negative error code.
int retorpn(unsigned char *param_1_original_copy, unsigned int max_len, unsigned char **param_3) {
  *param_3 = NULL; // Initialize output pointer

  size_t input_len = strlen((char *)param_1_original_copy);
  if (input_len == 0) {
    return -4; // Error: Empty regex
  }

  // Check for length limits (original code checks `input_len < 0xfffffff1` for max length, and `max_len < input_len` for output buffer size)
  if (input_len >= 0xfffffff1) {
    return -0x20; // Error: Regex too long
  }
  if (max_len < input_len) {
    return -0x40; // Error: Output buffer too small
  }

  // Allocate buffers for RPN output and operator stack
  // RPN buffer: Can be up to 2 * input_len (for implicit concatenations) + 1 for null
  unsigned char *rpn_buffer = (unsigned char *)calloc(1, input_len * 2 + 1);
  // Operator stack: Can be up to input_len + 1 for null
  unsigned char *op_stack = (unsigned char *)calloc(1, input_len + 1);
  // Temporary copy of the input string, as normalize_infix might reallocate it.
  // Allocated with `max_len` as per original code's `malloc(param_2)`.
  unsigned char *current_regex_str_base = (unsigned char *)malloc(max_len);
  if (!rpn_buffer || !op_stack || !current_regex_str_base) {
    free(rpn_buffer);
    free(op_stack);
    free(current_regex_str_base);
    return -1; // Memory allocation error
  }

  strcpy((char *)current_regex_str_base, (char *)param_1_original_copy); // Copy original regex
  
  // Normalize the regex string. current_regex_str_base might be updated due to reallocations.
  int error_code = normalize_infix(&current_regex_str_base);
  if (error_code != 0) {
    free(rpn_buffer);
    free(op_stack);
    free(current_regex_str_base); // Free the possibly reallocated string.
    return error_code;
  }

  unsigned char *rpn_ptr = rpn_buffer; // Pointer for filling RPN buffer
  unsigned char *op_stack_ptr = op_stack; // Pointer for top of operator stack
  bool processing_error = false; // Flag to indicate an error during RPN conversion

  // Iterate through the normalized regex string
  for (unsigned char *char_ptr = current_regex_str_base; *char_ptr != 0 && !processing_error; char_ptr++) {
    unsigned char current_char = *char_ptr;

    if (current_char == 0xfc) { // Handle OR operator ('|')
      // Error if stack is empty (no operand before OR) or next char is null/non-concatenating
      if (rpn_ptr == rpn_buffer || char_ptr[1] == 0 || is_nonconcat_char(char_ptr[1])) {
        processing_error = true;
        break;
      }
      // Pop operators from stack to RPN output until a lower precedence operator or '('
      while (op_stack_ptr != op_stack && op_stack_ptr[-1] != 0xa8 /* '(' */ && op_stack_ptr[-1] != 0xfc /* '|' */) {
        *rpn_ptr++ = *(--op_stack_ptr);
        *op_stack_ptr = 0; // Clear stack position
      }
      *op_stack_ptr++ = current_char; // Push current OR operator
    } else if (current_char == 0xa8) { // Handle OPEN_PAREN ('(')
      // Error if next char is null or is a non-concatenating character (e.g., '()', '(*')
      if (char_ptr[1] == 0 || is_nonconcat_char(char_ptr[1])) {
        processing_error = true;
        break;
      }
      *op_stack_ptr++ = current_char; // Push open parenthesis
    } else if (current_char == 0xa9) { // Handle CLOSE_PAREN (')')
      bool found_open_paren = false;
      while (op_stack_ptr != op_stack) {
        if (op_stack_ptr[-1] == 0xa8) { // Found matching open parenthesis
          *(--op_stack_ptr) = 0; // Pop and clear
          found_open_paren = true;
          break;
        }
        *rpn_ptr++ = *(--op_stack_ptr); // Pop operator to RPN output
        *op_stack_ptr = 0; // Clear stack position
      }
      if (!found_open_paren) {
        processing_error = true; // Error: Unmatched close parenthesis
        break;
      }
    } else if (current_char == 0xaa || current_char == 0xab || current_char == 0xbf) { // Handle unary operators: *, +, ?
      // These operators require an operand before them.
      if (rpn_ptr == rpn_buffer) { // RPN buffer is empty, no operand
        processing_error = true;
        break;
      }
      // Pop operators with higher or equal precedence (other unary ops or implicit concat)
      while (op_stack_ptr != op_stack && (op_stack_ptr[-1] == 0xa6 /* implicit concat */ ||
                                          op_stack_ptr[-1] == 0xaa /* * */ ||
                                          op_stack_ptr[-1] == 0xab /* + */ ||
                                          op_stack_ptr[-1] == 0xbf /* ? */)) {
        *rpn_ptr++ = *(--op_stack_ptr);
        *op_stack_ptr = 0;
      }
      *op_stack_ptr++ = current_char; // Push current unary operator
    } else { // Handle operands (literals or escaped characters)
      *rpn_ptr++ = current_char; // Push operand directly to RPN output
    }

    // Implicit concatenation logic:
    // If the current character is not an OR operator (0xfc), and there's a next character,
    // AND the next character IS a concatenating character (i.e., not a non-concatenating one),
    // then insert an implicit concatenation operator (0xa6).
    if (current_char != 0xfc && char_ptr[1] != 0 && is_nonconcat_char(char_ptr[1]) == 0) {
      // Pop operators with higher or equal precedence than implicit concat (0xa6)
      // Implicit concat has lower precedence than unary operators (*,+,?) but higher than OR.
      while (op_stack_ptr != op_stack && (op_stack_ptr[-1] == 0xaa || op_stack_ptr[-1] == 0xab ||
                                          op_stack_ptr[-1] == 0xbf || op_stack_ptr[-1] == 0xa6)) {
        *rpn_ptr++ = *(--op_stack_ptr);
        *op_stack_ptr = 0;
      }
      *op_stack_ptr++ = 0xa6; // Push implicit concatenation operator
    }
  }

  // After processing all characters, pop any remaining operators from the stack to RPN output
  while (op_stack_ptr != op_stack && !processing_error) {
    if (op_stack_ptr[-1] == 0xa8) { // Unmatched open parenthesis
      processing_error = true;
      break;
    }
    *rpn_ptr++ = *(--op_stack_ptr);
    *op_stack_ptr = 0; // Clear stack position
  }

  // Final cleanup and return based on error flag
  if (processing_error) {
    free(rpn_buffer);
    free(op_stack);
    free(current_regex_str_base); // Free the potentially reallocated string
    return -4; // Error: Malformed regex
  } else {
    *rpn_ptr = 0; // Null terminate the RPN string
    *param_3 = rpn_buffer; // Set output pointer
    free(op_stack);
    free(current_regex_str_base); // Free the possibly reallocated string
    return 0; // Success
  }
}

// Dummy main function for compilation
int main() {
    // Example usage (uncomment to test):
    // unsigned char *regex_str = (unsigned char *)strdup("a[bc]d*");
    // if (!regex_str) {
    //     fprintf(stderr, "strdup failed\n");
    //     return 1;
    // }
    // unsigned char *rpn_output = NULL;
    // int result = retorpn(regex_str, 100, &rpn_output); // Max RPN length 100
    // if (result == 0) {
    //     debug_print_re(rpn_output);
    //     free(rpn_output);
    // } else {
    //     fprintf(stderr, "Error converting regex to RPN: %d\n", result);
    // }
    // free(regex_str); // Free the initial copy passed to retorpn

    // unsigned char *regex_str2 = (unsigned char *)strdup("a(b|c)*d");
    // if (!regex_str2) {
    //     fprintf(stderr, "strdup failed\n");
    //     return 1;
    // }
    // rpn_output = NULL;
    // result = retorpn(regex_str2, 100, &rpn_output);
    // if (result == 0) {
    //     debug_print_re(rpn_output);
    //     free(rpn_output);
    // } else {
    //     fprintf(stderr, "Error converting regex to RPN: %d\n", result);
    // }
    // free(regex_str2);

    return 0;
}