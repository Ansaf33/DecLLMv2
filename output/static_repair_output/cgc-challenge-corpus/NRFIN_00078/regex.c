#include <stdbool.h> // For bool
#include <string.h>  // For memset, strlen
#include <stdlib.h>  // For calloc, free
#include <stdint.h>  // For uint8_t, uint32_t
#include <stdio.h>   // For main example, can be removed if not needed

// Define custom types from the snippet
typedef uint8_t byte;

// Structure for a regex element, inferred from offsets
// 0x00 - 0x1F: char_set (32 bytes)
// 0x20: is_star (4 bytes)
// 0x24: is_caret (4 bytes)
// 0x28: is_dollar (4 bytes)
typedef struct regex_element {
    byte char_set[0x20]; // 32 bytes for bitset (0-255 characters)
    uint32_t is_star;   // 0x20: 1 if quantifier '*', 0 otherwise
    uint32_t is_caret;  // 0x24: 1 if anchor '^', 0 otherwise
    uint32_t is_dollar; // 0x28: 1 if anchor '$', 0 otherwise
} regex_element_t;

// Function: init_char_set
// Initializes the character set bitmask to all zeros.
void init_char_set(regex_element_t *element) {
  memset(element->char_set, 0, sizeof(element->char_set));
}

// Function: set_bit
// Sets a specific bit in the character set bitmask.
void set_bit(byte *char_set_ptr, byte char_code) {
  char_set_ptr[char_code >> 3] |= (byte)(1 << (char_code & 7));
}

// Function: get_bit
// Checks if a specific bit is set in the character set bitmask.
bool get_bit(const byte *char_set_ptr, byte char_code) {
  return (char_set_ptr[char_code >> 3] >> (char_code & 7) & 1U) != 0;
}

// Function: invert_char_set
// Inverts all bits in the character set bitmask.
void invert_char_set(regex_element_t *element) {
  for (uint32_t i = 0; i < sizeof(element->char_set); ++i) {
    element->char_set[i] = ~element->char_set[i];
  }
}

// Function: parse_regex_element
// Parses a single regex element from the input string and populates a regex_element_t structure.
// Returns 0 on success, -1 on error.
int parse_regex_element(const char *regex_str, regex_element_t *element, char **next_regex_char) {
  const char *current_pos = regex_str;

  init_char_set(element);
  element->is_dollar = 0;
  element->is_caret = 0;
  element->is_star = 0;

  if (*current_pos == '^') {
    element->is_caret = 1;
    current_pos++;
  }

  if (*current_pos == '.') {
    invert_char_set(element);
    current_pos++;
  } else if (*current_pos == '[') {
    bool invert_set_flag = false;
    current_pos++; // Move past '['
    if (*current_pos == '^') {
      invert_set_flag = true;
      current_pos++; // Move past '^'
    }

    while (*current_pos != '\0' && *current_pos != ']') {
      set_bit(element->char_set, (byte)*current_pos);
      current_pos++;
    }

    if (invert_set_flag) {
      invert_char_set(element);
    }

    if (*current_pos != ']') {
      return -1; // Error: Missing ']'
    }
    current_pos++; // Move past ']'
  } else {
    if (*current_pos == '\\') {
      current_pos++;
    }
    if (*current_pos == '\0') {
      return -1; // Error: Unexpected end of string after escape
    }
    set_bit(element->char_set, (byte)*current_pos);
    current_pos++;
  }

  if (*current_pos == '*') {
    element->is_star = 1;
    current_pos++;
  }
  if (*current_pos == '$') {
    element->is_dollar = 1;
    current_pos++;
  }

  *next_regex_char = (char *)current_pos;
  return 0;
}

// Function: greedy_match_one
// Attempts to greedily match one regex element against the text.
// Returns 1 on success, 0 on failure.
int greedy_match_one(const char *text_pos, const regex_element_t *element, bool is_start_of_text, char **next_match_pos) {
  *next_match_pos = (char *)text_pos; // Initialize output pointer

  // Handle '^' (start of string anchor)
  if (element->is_caret && !is_start_of_text) {
    return 0; // Match fails if '^' is present and it's not the start of the text
  }

  if (element->is_star == 0) { // Not a '*' quantifier (match exactly one character)
    if (*text_pos == '\0' || !get_bit(element->char_set, (byte)*text_pos)) {
      return 0; // No match for a single character
    }
    *next_match_pos = (char *)text_pos + 1; // Advance past the matched character
  } else { // '*' quantifier (match zero or more characters greedily)
    while (*(*next_match_pos) != '\0' && get_bit(element->char_set, (byte)*(*next_match_pos))) {
      (*next_match_pos)++; // Consume as many matching characters as possible
    }
  }

  // Handle '$' (end of string anchor)
  // strlen(*next_match_pos) == 0 means *next_match_pos points to the null terminator
  if (element->is_dollar && strlen(*next_match_pos) != 0) {
    return 0; // Match fails if '$' is present and it's not the end of the text
  }

  return 1; // Match successful
}

// Function: do_regex_match
// Recursively attempts to match a sequence of regex elements against the text.
// Returns 0 on full match, -1 on failure.
int do_regex_match(const char *text_pos, regex_element_t **elements, bool is_start_of_text, char **final_match_pos) {
  regex_element_t **current_element_ptr = elements;

  if (*current_element_ptr == NULL) { // No more regex elements to match
    *final_match_pos = (char *)text_pos; // The rest of the text is matched (empty string)
    return 0; // Success
  }

  char *matched_char_pos = (char *)text_pos; // Current position in text after matching one element

  int match_result = greedy_match_one(text_pos, *current_element_ptr, is_start_of_text, &matched_char_pos);

  bool needs_common_block = false;

  if (match_result == 0) {
    needs_common_block = true; // Current element didn't match
  } else { // Current element matched successfully
    // Backtracking loop for '*' quantifier
    while (true) {
      // Condition to determine if we should stop backtracking and proceed to the common "skip remaining star elements" block.
      // This logic corresponds to the original LAB_00012c24 goto target.
      bool condition_for_common_block = false;
      if (matched_char_pos <= text_pos) { // Pointer comparison: current match position is at or before original start
          // If it's not a star, and we didn't advance (or went backward), it's a failure for this branch.
          // If it's a star, and we backtracked past the original start, it's also a failure for this branch.
          if ((*current_element_ptr)->is_star == 0 || matched_char_pos < text_pos) {
              condition_for_common_block = true;
          }
      }

      if (condition_for_common_block) {
        needs_common_block = true;
        break; // Exit backtracking loop
      }

      // Recursively try to match the rest of the regex from the current text position
      int recursive_result = do_regex_match(matched_char_pos, current_element_ptr + 1, false, final_match_pos);
      if (recursive_result == 0) {
        return 0; // Found a full match, propagate success
      }

      // Backtrack: if the recursive match failed, and current element is a star,
      // try matching fewer characters with '*' by moving matched_char_pos backward.
      if ((*current_element_ptr)->is_star == 0) {
          // If it's not a star, and recursive match failed, there's no backtracking for this element.
          needs_common_block = true; // Fall through to common block as no more options for this element
          break;
      }

      matched_char_pos--; // Backtrack one character for a '*' element
      if (matched_char_pos < text_pos) { // If backtracked beyond the original starting point for this element
          needs_common_block = true; // No more valid backtracking possible
          break;
      }
    }
  }

  // Common block (Logic from original LAB_00012c24)
  if (needs_common_block) {
    // Skip any remaining '*' elements that can match zero characters (i.e., not anchored to '$')
    while (*current_element_ptr != NULL &&
           (*current_element_ptr)->is_star != 0 &&
           (*current_element_ptr)->is_dollar == 0) {
      current_element_ptr++;
    }

    if (*current_element_ptr == NULL) { // All elements matched (possibly zero-width for some * elements)
      *final_match_pos = matched_char_pos; // Set final match position
      return 0; // Success
    } else {
      return -1; // Failure: remaining elements couldn't be matched
    }
  }
  return 0; // This path is reached if the while loop broke because `recursive_result == 0`
}

// Function: regex_match
// Main function to match a regex pattern against a text string.
// Returns 0 on success (match found), -1 on failure.
int regex_match(const char *regex_str, const char *text_to_search, char **match_start_pos, char **match_end_pos) {
  int result = -1; // Default to failure
  uint32_t num_elements = 0;
  const char *current_regex_char_ptr = regex_str;
  regex_element_t **elements_array_ptr = NULL;

  // First pass: count regex elements to determine allocation size
  while (*current_regex_char_ptr != '\0') {
    regex_element_t temp_element; // Temporary element for parse_regex_element signature
    const char *prev_char_ptr = current_regex_char_ptr;
    if (parse_regex_element(current_regex_char_ptr, &temp_element, (char **)&current_regex_char_ptr) != 0) {
      return -1; // Error parsing regex pattern
    }
    if (current_regex_char_ptr == prev_char_ptr) { // If no characters were consumed, it's an infinite loop or bad regex
        return -1;
    }
    num_elements++;
  }

  // Allocate memory for an array of regex_element_t pointers (plus a NULL terminator)
  elements_array_ptr = (regex_element_t **)calloc(num_elements + 1, sizeof(regex_element_t *));
  if (elements_array_ptr == NULL) {
    return -1;
  }

  bool allocation_and_parse_successful = true;
  current_regex_char_ptr = regex_str; // Reset regex string pointer for second pass
  for (uint32_t i = 0; i < num_elements; i++) {
    elements_array_ptr[i] = (regex_element_t *)calloc(1, sizeof(regex_element_t)); // Allocate memory for each element
    if (elements_array_ptr[i] == NULL) {
      allocation_and_parse_successful = false;
      break;
    }
    // Second pass: parse regex elements and fill allocated memory
    if (parse_regex_element(current_regex_char_ptr, elements_array_ptr[i], (char **)&current_regex_char_ptr) != 0) {
      allocation_and_parse_successful = false;
      break;
    }
  }
  elements_array_ptr[num_elements] = NULL; // Null terminate the array of pointers

  if (allocation_and_parse_successful) {
    // Main matching loop: try to match the regex at every possible starting position in the text
    const char *current_text_pos = text_to_search;
    bool match_found_in_text = false;
    while (*current_text_pos != '\0' && !match_found_in_text) {
      *match_start_pos = (char *)current_text_pos; // Tentative match start
      // is_start_of_text is true only if current_text_pos is the actual beginning of the text_to_search
      if (do_regex_match(current_text_pos, elements_array_ptr, current_text_pos == text_to_search, match_end_pos) == 0) {
        result = 0; // Found a match
        match_found_in_text = true;
      }
      if (!match_found_in_text) { // Only advance if no match found at current_text_pos
          current_text_pos++;
      }
    }

    // After the loop, if no match was found, try matching at the very end of the string (for '$' only patterns)
    if (!match_found_in_text) {
        *match_start_pos = (char *)current_text_pos; // Points to the null terminator
        if (do_regex_match(current_text_pos, elements_array_ptr, current_text_pos == text_to_search, match_end_pos) == 0) {
            result = 0;
        }
    }
  }

  // Cleanup: Free all allocated memory
  if (elements_array_ptr != NULL) {
    // Iterate up to num_elements because elements_array_ptr[num_elements] is NULL, not allocated.
    for (uint32_t i = 0; i < num_elements; i++) {
      free(elements_array_ptr[i]);
    }
    free(elements_array_ptr);
  }
  return result;
}

// Minimal main function for compilation
int main() {
    // Example usage:
    // char *regex_pattern = "^a*b$";
    // char *text_to_search = "aaab";
    // char *match_start = NULL;
    // char *match_end = NULL;
    // int res = regex_match(regex_pattern, text_to_search, &match_start, &match_end);
    // if (res == 0) {
    //     printf("Regex '%s' matched text '%s'\n", regex_pattern, text_to_search);
    //     printf("Match found from index %td to %td\n", match_start - text_to_search, match_end - text_to_search);
    //     printf("Matched substring: '%.*s'\n", (int)(match_end - match_start), match_start);
    // } else {
    //     printf("Regex '%s' did not match text '%s'\n", regex_pattern, text_to_search);
    // }

    // char *regex_pattern2 = "a*";
    // char *text_to_search2 = "aaabbb";
    // match_start = NULL;
    // match_end = NULL;
    // res = regex_match(regex_pattern2, text_to_search2, &match_start, &match_end);
    // if (res == 0) {
    //     printf("Regex '%s' matched text '%s'\n", regex_pattern2, text_to_search2);
    //     printf("Match found from index %td to %td\n", match_start - text_to_search2, match_end - text_to_search2);
    //     printf("Matched substring: '%.*s'\n", (int)(match_end - match_start), match_start);
    // } else {
    //     printf("Regex '%s' did not match text '%s'\n", regex_pattern2, text_to_search2);
    // }

    return 0;
}