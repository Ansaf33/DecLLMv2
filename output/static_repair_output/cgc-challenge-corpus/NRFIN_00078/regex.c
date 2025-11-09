#include <stdbool.h> // For bool
#include <stdint.h>  // For uint8_t, uint32_t
#include <string.h>  // For memset, strlen
#include <stdlib.h>  // For calloc, free
#include <stdio.h>   // For main function example

// Define custom types based on their usage
typedef uint8_t byte;
typedef uint8_t undefined;
typedef uint32_t undefined4; // Assuming 4-byte undefined value

// Struct for regex element, based on offsets observed in original code
typedef struct regex_element {
    uint8_t char_set[32]; // Represents 256 bits for ASCII characters (0-255)
    int is_star;          // Offset 0x20 (32 bytes) - True if quantifier is '*'
    int is_caret;         // Offset 0x24 - True if anchor is '^' (start of string)
    int is_dollar;        // Offset 0x28 - True if anchor is '$' (end of string)
} regex_element; // Total size: 32 + 4 + 4 + 4 = 44 bytes (0x2C)

// Function: init_char_set
// Initializes the character set bitmask to all zeros.
void init_char_set(regex_element *element) {
  memset(element->char_set, 0, sizeof(element->char_set));
}

// Function: set_bit
// Sets the bit corresponding to char_code in the element's character set.
void set_bit(regex_element *element, byte char_code) {
  element->char_set[char_code >> 3] |= (1 << (char_code & 7));
}

// Function: get_bit
// Checks if the bit corresponding to char_code is set in the element's character set.
bool get_bit(regex_element *element, byte char_code) {
  return ((element->char_set[char_code >> 3] >> (char_code & 7)) & 1) != 0;
}

// Function: invert_char_set
// Inverts all bits in the element's character set.
void invert_char_set(regex_element *element) {
  for (unsigned int i = 0; i < sizeof(element->char_set); ++i) {
    element->char_set[i] = ~element->char_set[i];
  }
}

// Function: parse_regex_element
// Parses a single regex element from the regex string.
// regex_str: The current position in the regex pattern string.
// element: Pointer to the regex_element structure to populate.
// next_regex_char: Output parameter, pointer to the character after the parsed element.
// Returns 0 on success, 0xffffffff on error.
undefined4 parse_regex_element(const char *regex_str, regex_element *element, const char **next_regex_char) {
  const char *current_char = regex_str;

  init_char_set(element);
  element->is_dollar = 0;
  element->is_caret = 0;
  element->is_star = 0;

  if (*current_char == '^') {
    element->is_caret = 1;
    current_char++;
  }

  if (*current_char == '.') {
    invert_char_set(element);
    current_char++;
  } else if (*current_char == '[') {
    bool invert_set = false;
    current_char++; // Move past '['
    if (*current_char == '^') {
      invert_set = true;
      current_char++; // Move past '^'
    }
    
    // Loop until end of string or ']'
    while (*current_char != '\0' && *current_char != ']') {
      set_bit(element, *current_char);
      current_char++;
    }

    if (invert_set) {
      invert_char_set(element);
    }
    
    if (*current_char != ']') {
      return 0xffffffff; // Error: missing ']'
    }
    current_char++; // Move past ']'
  } else {
    if (*current_char == '\\') {
      current_char++;
    }
    if (*current_char == '\0') {
      return 0xffffffff; // Error: escaped char missing
    }
    set_bit(element, *current_char);
    current_char++;
  }

  if (*current_char == '*') {
    element->is_star = 1;
    current_char++;
  }
  if (*current_char == '$') {
    element->is_dollar = 1;
    current_char++;
  }

  *next_regex_char = current_char;
  return 0;
}

// Function: greedy_match_one
// Tries to match a single regex element (or a sequence for '*') greedily.
// text: Current position in the text to match against.
// element: The regex_element to match.
// is_start_of_string: True if 'text' points to the very beginning of the original search string.
// next_text_pos: Output parameter, pointer to the position in 'text' after a successful match.
// Returns 1 on match, 0 on no match.
undefined4 greedy_match_one(const char *text, const regex_element *element, bool is_start_of_string, const char **next_text_pos) {
  *next_text_pos = text;

  // Handle '^' anchor: if element is '^' and not at string start, no match.
  if (element->is_caret && !is_start_of_string) {
    return 0;
  }

  if (element->is_star == 0) { // Not a '*' quantifier (match one character)
    if (*text == '\0' || !get_bit(element, *text)) {
      return 0; // No match for this single character
    }
    *next_text_pos = text + 1; // Matched one character
  } else { // '*' quantifier (match zero or more characters greedily)
    while (*(*next_text_pos) != '\0' && get_bit(element, *(*next_text_pos))) {
      *next_text_pos = *(*next_text_pos) + 1;
    }
  }

  // Handle '$' anchor: if element is '$' and not at end of string, no match.
  if (element->is_dollar && *(*next_text_pos) != '\0') {
    return 0;
  }
  
  return 1; // Match found for this element
}

// Function: do_regex_match
// Recursively attempts to match the sequence of regex elements against the text.
// text_pos: Current position in the text to match.
// elements: Pointer to the current regex_element pointer in the array of elements.
// is_start_of_string: True if text_pos is the very beginning of the original search string.
// match_end_pos: Output parameter, pointer to the end of the full match in the text.
// Returns 0 on successful full match, 0xffffffff on no match.
undefined4 do_regex_match(const char *text_pos, regex_element **elements, bool is_start_of_string, const char **match_end_pos) {
  // Base case: if no more regex elements, we've matched successfully.
  if (*elements == NULL) {
    *match_end_pos = text_pos;
    return 0; // Success
  }

  const regex_element *current_element = *elements;
  const char *current_match_end = text_pos;
  undefined4 result = 0xffffffff; // Assume failure

  // If the current element has a '*' quantifier, try greedy matching with backtracking.
  if (current_element->is_star) {
    // Greedy approach: match as many characters as possible first.
    // current_match_end will point to the character AFTER the longest possible match.
    greedy_match_one(text_pos, current_element, is_start_of_string, &current_match_end);

    // Backtrack from the longest match down to zero characters.
    // Try matching the rest of the regex from each backtrack point.
    // Loop condition: `current_match_end >= text_pos` allows matching zero characters.
    while (current_match_end >= text_pos) {
      // Recursively try to match the rest of the regex (elements + 1)
      // from the current backtrack point (current_match_end).
      // is_start_of_string is false for subsequent elements in a match.
      result = do_regex_match(current_match_end, elements + 1, false, match_end_pos);
      if (result == 0) { // If the rest of the regex matches, we found a full match.
        return 0; // Success
      }
      // If not, backtrack: try matching one fewer character with the current '*' element.
      current_match_end--;
    }
    return 0xffffffff; // No match found even after backtracking.
  } else {
    // Not a '*' quantifier: try to match exactly one character (or zero for anchors if applicable).
    if (greedy_match_one(text_pos, current_element, is_start_of_string, &current_match_end) == 0) {
      return 0xffffffff; // Current element didn't match.
    }
    // If it matched, try to match the rest of the regex from the new position.
    result = do_regex_match(current_match_end, elements + 1, false, match_end_pos);
    return result;
  }
}

// Function: regex_match
// Main function to match a regex pattern against a text string.
// regex_pattern: The regex pattern string.
// text_to_search: The text string to search within.
// match_start_ptr: Output parameter, pointer to the start of the first full match.
// match_end_ptr: Output parameter, pointer to the end of the first full match.
// Returns 0 on successful match, 0xffffffff on no match or error.
undefined4 regex_match(const char *regex_pattern, const char *text_to_search, const char **match_start_ptr, const char **match_end_ptr) {
  unsigned int num_elements = 0;
  const char *current_regex_pos = regex_pattern;
  regex_element temp_element; // Temporary element for the first pass to count elements

  // First pass: parse regex elements to count them.
  // This helps in allocating the correct size for the elements_array.
  while (*current_regex_pos != '\0') {
    if (parse_regex_element(current_regex_pos, &temp_element, &current_regex_pos) != 0) {
      return 0xffffffff; // Error in regex parsing
    }
    num_elements++;
  }

  // Allocate memory for an array of regex_element pointers.
  // Add 1 for the NULL terminator, which signals the end of the regex pattern to do_regex_match.
  regex_element **elements_array = (regex_element **)calloc(num_elements + 1, sizeof(regex_element *));
  if (elements_array == NULL) {
    return 0xffffffff;
  }

  // Second pass: parse regex elements again and populate the allocated array.
  current_regex_pos = regex_pattern;
  for (unsigned int i = 0; i < num_elements; ++i) {
    elements_array[i] = (regex_element *)calloc(1, sizeof(regex_element));
    if (elements_array[i] == NULL) {
      // Clean up previously allocated elements on error.
      for (unsigned int j = 0; j < i; ++j) {
        free(elements_array[j]);
      }
      free(elements_array);
      return 0xffffffff;
    }
    // Parse the element and advance the regex string pointer.
    if (parse_regex_element(current_regex_pos, elements_array[i], &current_regex_pos) != 0) {
      // This case should ideally not happen if the first pass was successful,
      // but included for robustness.
      for (unsigned int j = 0; j <= i; ++j) {
        free(elements_array[j]);
      }
      free(elements_array);
      return 0xffffffff;
    }
  }
  elements_array[num_elements] = NULL; // Null-terminate the array.

  undefined4 result = 0xffffffff; // Assume no match initially.
  const char *current_text_search_pos = text_to_search;

  // Iterate through the text to find the first match.
  // The loop continues until the end of the text string.
  while (*current_text_search_pos != '\0') {
    *match_start_ptr = current_text_search_pos; // Potential start of match
    // Attempt to match the entire regex from the current text position.
    // (current_text_search_pos == text_to_search) checks if we are at the very start of the text.
    if (do_regex_match(current_text_search_pos, elements_array, (current_text_search_pos == text_to_search), match_end_ptr) == 0) {
      result = 0; // Match found.
      break;      // Exit loop after first match.
    }
    current_text_search_pos++; // Move to the next character in the text.
  }

  // After the loop, check if a match can occur at the very end of the string (empty match possible for 'a*$', for example).
  if (result != 0) { 
      *match_start_ptr = current_text_search_pos; // This will be the '\0' terminator position.
      if (do_regex_match(current_text_search_pos, elements_array, (current_text_search_pos == text_to_search), match_end_ptr) == 0) {
          result = 0; // Match found at the end.
      }
  }

  // Clean up all allocated memory.
  for (unsigned int i = 0; i < num_elements; ++i) {
    free(elements_array[i]);
  }
  free(elements_array);

  return result;
}

// Main function for testing regex_match functionality.
int main() {
    const char *regex_pattern1 = "^a.c*d$";
    const char *text1 = "abcccd";
    const char *match_start1 = NULL;
    const char *match_end1 = NULL;

    printf("Regex: \"%s\", Text: \"%s\"\n", regex_pattern1, text1);
    if (regex_match(regex_pattern1, text1, &match_start1, &match_end1) == 0) {
        printf("Match found! Start: %td, End: %td\n", match_start1 - text1, match_end1 - text1);
        printf("Matched substring: '%.*s'\n", (int)(match_end1 - match_start1), match_start1);
    } else {
        printf("No match.\n");
    }
    printf("\n");

    const char *regex_pattern2 = "b[aeiou]*z";
    const char *text2 = "xyzbaoeuizabc";
    const char *match_start2 = NULL;
    const char *match_end2 = NULL;

    printf("Regex: \"%s\", Text: \"%s\"\n", regex_pattern2, text2);
    if (regex_match(regex_pattern2, text2, &match_start2, &match_end2) == 0) {
        printf("Match found! Start: %td, End: %td\n", match_start2 - text2, match_end2 - text2);
        printf("Matched substring: '%.*s'\n", (int)(match_end2 - match_start2), match_start2);
    } else {
        printf("No match.\n");
    }
    printf("\n");

    const char *regex_pattern3 = "a*b";
    const char *text3 = "aaaaab";
    const char *match_start3 = NULL;
    const char *match_end3 = NULL;

    printf("Regex: \"%s\", Text: \"%s\"\n", regex_pattern3, text3);
    if (regex_match(regex_pattern3, text3, &match_start3, &match_end3) == 0) {
        printf("Match found! Start: %td, End: %td\n", match_start3 - text3, match_end3 - text3);
        printf("Matched substring: '%.*s'\n", (int)(match_end3 - match_start3), match_start3);
    } else {
        printf("No match.\n");
    }
    printf("\n");

    const char *regex_pattern4 = "a*b";
    const char *text4 = "b";
    const char *match_start4 = NULL;
    const char *match_end4 = NULL;

    printf("Regex: \"%s\", Text: \"%s\"\n", regex_pattern4, text4);
    if (regex_match(regex_pattern4, text4, &match_start4, &match_end4) == 0) {
        printf("Match found! Start: %td, End: %td\n", match_start4 - text4, match_end4 - text4);
        printf("Matched substring: '%.*s'\n", (int)(match_end4 - match_start4), match_start4);
    } else {
        printf("No match.\n");
    }
    printf("\n");

    const char *regex_pattern5 = "^a";
    const char *text5 = "ba";
    const char *match_start5 = NULL;
    const char *match_end5 = NULL;

    printf("Regex: \"%s\", Text: \"%s\"\n", regex_pattern5, text5);
    if (regex_match(regex_pattern5, text5, &match_start5, &match_end5) == 0) {
        printf("Match found! Start: %td, End: %td\n", match_start5 - text5, match_end5 - text5);
        printf("Matched substring: '%.*s'\n", (int)(match_end5 - match_start5), match_start5);
    } else {
        printf("No match.\n");
    }
    printf("\n");

    const char *regex_pattern6 = "a$";
    const char *text6 = "baa";
    const char *match_start6 = NULL;
    const char *match_end6 = NULL;

    printf("Regex: \"%s\", Text: \"%s\"\n", regex_pattern6, text6);
    if (regex_match(regex_pattern6, text6, &match_start6, &match_end6) == 0) {
        printf("Match found! Start: %td, End: %td\n", match_start6 - text6, match_end6 - text6);
        printf("Matched substring: '%.*s'\n", (int)(match_end6 - match_start6), match_start6);
    } else {
        printf("No match.\n");
    }
    printf("\n");

    const char *regex_pattern7 = "a.c";
    const char *text7 = "axc";
    const char *match_start7 = NULL;
    const char *match_end7 = NULL;

    printf("Regex: \"%s\", Text: \"%s\"\n", regex_pattern7, text7);
    if (regex_match(regex_pattern7, text7, &match_start7, &match_end7) == 0) {
        printf("Match found! Start: %td, End: %td\n", match_start7 - text7, match_end7 - text7);
        printf("Matched substring: '%.*s'\n", (int)(match_end7 - match_start7), match_start7);
    } else {
        printf("No match.\n");
    }
    printf("\n");

    const char *regex_pattern8 = "a[\\^]c"; // Match a^c
    const char *text8 = "aa^c";
    const char *match_start8 = NULL;
    const char *match_end8 = NULL;

    printf("Regex: \"%s\", Text: \"%s\"\n", regex_pattern8, text8);
    if (regex_match(regex_pattern8, text8, &match_start8, &match_end8) == 0) {
        printf("Match found! Start: %td, End: %td\n", match_start8 - text8, match_end8 - text8);
        printf("Matched substring: '%.*s'\n", (int)(match_end8 - match_start8), match_start8);
    } else {
        printf("No match.\n");
    }
    printf("\n");

    const char *regex_pattern9 = "a[^b]c"; // Match a then anything but b then c
    const char *text9 = "axc";
    const char *match_start9 = NULL;
    const char *match_end9 = NULL;

    printf("Regex: \"%s\", Text: \"%s\"\n", regex_pattern9, text9);
    if (regex_match(regex_pattern9, text9, &match_start9, &match_end9) == 0) {
        printf("Match found! Start: %td, End: %td\n", match_start9 - text9, match_end9 - text9);
        printf("Matched substring: '%.*s'\n", (int)(match_end9 - match_start9), match_start9);
    } else {
        printf("No match.\n");
    }
    printf("\n");

    const char *regex_pattern10 = "a[^b]c"; // Match a then anything but b then c
    const char *text10 = "abc";
    const char *match_start10 = NULL;
    const char *match_end10 = NULL;

    printf("Regex: \"%s\", Text: \"%s\"\n", regex_pattern10, text10);
    if (regex_match(regex_pattern10, text10, &match_start10, &match_end10) == 0) {
        printf("Match found! Start: %td, End: %td\n", match_start10 - text10, match_end10 - text10);
        printf("Matched substring: '%.*s'\n", (int)(match_end10 - match_start10), match_start10);
    } else {
        printf("No match.\n");
    }
    printf("\n");

    const char *regex_pattern11 = "a*"; // Matches empty string
    const char *text11 = "xyz";
    const char *match_start11 = NULL;
    const char *match_end11 = NULL;

    printf("Regex: \"%s\", Text: \"%s\"\n", regex_pattern11, text11);
    if (regex_match(regex_pattern11, text11, &match_start11, &match_end11) == 0) {
        printf("Match found! Start: %td, End: %td\n", match_start11 - text11, match_end11 - text11);
        printf("Matched substring: '%.*s'\n", (int)(match_end11 - match_start11), match_start11);
    } else {
        printf("No match.\n");
    }
    printf("\n");

    const char *regex_pattern12 = "a*b$"; 
    const char *text12 = "aaab";
    const char *match_start12 = NULL;
    const char *match_end12 = NULL;

    printf("Regex: \"%s\", Text: \"%s\"\n", regex_pattern12, text12);
    if (regex_match(regex_pattern12, text12, &match_start12, &match_end12) == 0) {
        printf("Match found! Start: %td, End: %td\n", match_start12 - text12, match_end12 - text12);
        printf("Matched substring: '%.*s'\n", (int)(match_end12 - match_start12), match_start12);
    } else {
        printf("No match.\n");
    }
    printf("\n");

    const char *regex_pattern13 = "a*b$"; 
    const char *text13 = "aaabc";
    const char *match_start13 = NULL;
    const char *match_end13 = NULL;

    printf("Regex: \"%s\", Text: \"%s\"\n", regex_pattern13, text13);
    if (regex_match(regex_pattern13, text13, &match_start13, &match_end13) == 0) {
        printf("Match found! Start: %td, End: %td\n", match_start13 - text13, match_end13 - text13);
        printf("Matched substring: '%.*s'\n", (int)(match_end13 - match_start13), match_start13);
    } else {
        printf("No match.\n");
    }
    printf("\n");

    return 0;
}