#include <stdlib.h> // For calloc, free, strdup
#include <string.h> // For memset
#include <stdbool.h> // For bool
#include <stdint.h> // For uintptr_t, uint8_t

// Type definitions to clarify original 'undefined' types
// 'undefined' is likely a single byte, 'char' is appropriate.
// 'undefined *' is a generic pointer, 'void *' is appropriate.
// 'undefined4' can be an integer or a pointer depending on context.
// For return values 0/1, 'int' is used. For pointers, 'void *' or 'uintptr_t' for storage.

// Forward declarations
void *init_rop(char param_1, int param_2);
int simplify(char **param_1, void *param_2);
int parse_class(void **param_1, const char *param_2);
int regexp_parse(void **param_1, const char *param_2, int param_3);
void _recursive_free(void *param_1_ptr);
int regexp_free(void **param_1);
int check_prev_state(void *state_ptr, uintptr_t node_id);
void add_state(void *state_ptr, uintptr_t node_id);
int _regexp_match(void *state_ptr, void *node_ptr, int char_val);


// Structure for RegexpMatchState, inferred from regexp_match offsets
typedef struct RegexpMatchState {
    void *regex_root_node;          // Corresponds to param_1[4] in regexp_init (at offset 0x4 * sizeof(void*))
                                    // For this struct, assuming it's the first field, offset 0.
                                    // Let's re-evaluate the offsets if `regexp_obj_ptr` is directly cast.
                                    // `regexp_match` uses `*(void **)(param_1 + 0x4)` for `regex_root_node`.
                                    // `*(void **)(param_1 + 0x8)` for `current_states`.
                                    // `*(unsigned int *)(param_1 + 0xc)` for `current_states_count`.
                                    // `*(size_t *)(param_1 + 0x10)` for `max_states_capacity`.
                                    // `*(void **)(param_1 + 0x14)` for `prev_check_states`.
                                    // `*(unsigned int *)(param_1 + 0x18)` for `prev_check_states_count`.
                                    // This means `regexp_obj_ptr` points to a block of memory, and these are offsets from its base.
                                    // The `regexp_t` structure (an array of char* or void*) defined implicitly in regexp_init
                                    // is different from this runtime state structure.
                                    // It's safer to use direct pointer arithmetic on `void *regexp_obj_ptr`.
} RegexpMatchState;


// Function: init_rop
void *init_rop(char param_1, int param_2) {
  void *puVar1 = calloc(1, param_2 + 0x10);
  if (puVar1 != NULL) {
    *(char *)puVar1 = param_1;
  }
  return puVar1;
}

// Function: simplify
int simplify(char **param_1, void *param_2) {
  char *pcVar1 = *param_1;
  int total_count = 0;
  void *temp_node_ptr;

  if (*pcVar1 == '\x05') { // Node type 5 (sub-expression)
    total_count = simplify((char **)(pcVar1 + 0xc), pcVar1);
    if (total_count == 0) {
      return 0;
    }
  } else if (*pcVar1 == '\x06') { // Node type 6 (OR)
    total_count = simplify((char **)(pcVar1 + 0xc), pcVar1);
    if (total_count == 0) {
      return 0;
    }
    int iVar2 = simplify((char **)(pcVar1 + 0x10), pcVar1);
    if (iVar2 == 0) {
      return 0;
    }
    total_count += iVar2;
  }

  // Offset 4 is usually for the 'next' node in a sequence
  char **ptr_to_next_node = (char **)(pcVar1 + 4);
  if (*ptr_to_next_node != NULL) {
    int iVar2 = simplify(ptr_to_next_node, param_2);
    if (iVar2 == 0) {
      return 0;
    }
    total_count += iVar2;
  }

  // Handle quantifiers and optional nodes based on pcVar1[1]
  if (pcVar1[1] == '\x01') { // '+' quantifier
    temp_node_ptr = init_rop(6, 8); // Create an OR node (type 6)
    if (temp_node_ptr == NULL) {
      return 0;
    }
    *(char **)((char *)temp_node_ptr + 0xc) = pcVar1; // Left child is current node
    *(void **)((char *)temp_node_ptr + 0x10) = *(void **)(pcVar1 + 4); // Right child is current 'next'
    *(void **)(pcVar1 + 4) = temp_node_ptr; // Current 'next' now points to the new OR node
  } else if (pcVar1[1] == '\x03') { // '?' quantifier
    temp_node_ptr = init_rop(6, 8); // Create an OR node (type 6)
    if (temp_node_ptr == NULL) {
      return 0;
    }
    *(char **)((char *)temp_node_ptr + 0xc) = pcVar1; // Left child is current node
    *(void **)((char *)temp_node_ptr + 0x10) = *(void **)(pcVar1 + 4); // Right child is current 'next'
    *param_1 = (char *)temp_node_ptr; // param_1 (pointer to current node) now points to new OR node
  } else if (pcVar1[1] == '\x02') { // '*' quantifier
    temp_node_ptr = init_rop(6, 8); // First OR node
    if (temp_node_ptr == NULL) {
      return 0;
    }
    *(char **)((char *)temp_node_ptr + 0xc) = pcVar1;
    *(void **)((char *)temp_node_ptr + 0x10) = *(void **)(pcVar1 + 4);
    *param_1 = (char *)temp_node_ptr;

    temp_node_ptr = init_rop(6, 8); // Second OR node
    if (temp_node_ptr == NULL) {
      return 0;
    }
    *(char **)((char *)temp_node_ptr + 0xc) = pcVar1;
    *(void **)((char *)temp_node_ptr + 0x10) = *(void **)(pcVar1 + 4);
    *(void **)(pcVar1 + 4) = temp_node_ptr;
  }

  pcVar1[1] = '\0'; // Reset flag/quantifier byte
  *(void **)(pcVar1 + 8) = param_2; // Store 'parent' or 'previous' link
  return total_count + 1;
}

// Function: parse_class
int parse_class(void **param_1, const char *param_2) {
  bool bVar1 = false;
  bool bVar2 = false;
  unsigned int local_1c = 0xffffffff; // Stores previous character in a range
  void *iVar3 = init_rop(4, 0x20); // Create a node of type 4 (char class) with 0x20 bytes for bitmap
  int local_10 = 0; // Loop counter and return value

  if (iVar3 == NULL) {
    return 0;
  }

  for (local_10 = 0; param_2[local_10] != ']'; local_10++) {
    unsigned int local_20 = (unsigned char)param_2[local_10]; // Current character

    if (local_20 == 0) { // Unexpected null terminator
      return 0;
    }

    if ((local_10 == 0) && (local_20 == '^')) { // Negated character class
      bVar2 = true;
    } else {
      if (local_20 == '-') { // Range operator
        if (local_1c != 0xffffffff) { // If a character precedes the '-'
          bVar1 = true;
        }
      } else if (local_20 == '\\') { // Escape character
        if (param_2[local_10 + 1] == '\0') {
          return 0;
        }
        local_10++; // Skip escape char
        local_20 = (unsigned char)param_2[local_10];
      }

      if (bVar1) { // If currently processing a range (e.g., 'a-z')
        if ((int)local_20 < (int)local_1c) { // Range is inverted (e.g., 'z-a')
          // Set a flag in the node (byte at offset 0x11)
          *((uint8_t *)iVar3 + 0x11) |= 0x20;
        } else {
          // Fill bitmap for characters in the range
          for (unsigned int local_24 = local_1c; (int)local_24 <= (int)local_20; local_24++) {
            unsigned int uVar4 = local_24;
            // The original logic `if ((int)local_24 < 0) { uVar4 = local_24 + 7; }` is suspicious
            // for unsigned char values (0-255). It will not be true.
            // Assuming it's for byte-to-bit-index mapping.
            *((uint8_t *)iVar3 + 0xc + (uVar4 >> 3)) |= (1 << (local_24 & 7));
          }
        }
      }
      // Set the bit for the current character
      unsigned int uVar4 = local_20;
      // Same suspicious logic for `if ((int)local_20 < 0)`.
      *((uint8_t *)iVar3 + 0xc + (uVar4 >> 3)) |= (1 << (local_20 & 7));
      local_1c = local_20; // Store current char as previous for next range check
      bVar1 = false;
    }
  }

  if (bVar2) { // If character class was negated (e.g., '[^abc]')
    for (int i = 0; i < 0x20; i++) { // Invert all bits in the bitmap
      *((uint8_t *)iVar3 + i + 0xc) = ~*((uint8_t *)iVar3 + i + 0xc);
    }
  }
  *param_1 = iVar3; // Store the created character class node
  return local_10 + 1; // Return length consumed + 1 for ']'
}

// Function: regexp_parse
int regexp_parse(void **param_1, const char *param_2, int param_3) {
  void *current_node_head = NULL;
  void *current_node_tail = NULL;
  int index = 0;

  while (true) {
    char current_char = param_2[index];

    if (param_3 == current_char) { // Reached terminator character (e.g., ')')
      *param_1 = current_node_head;
      return index + 1;
    }

    if (current_char == '\0') { // Unexpected null terminator
      return 0;
    }

    void *new_node = NULL;
    int sub_parse_len = 0;
    bool handled_special_case = false; // For cases like '|' that modify param_1 and continue

    switch (current_char) {
      case '|': // OR operator
        if (current_node_tail == NULL) { // Cannot start with OR
          return 0;
        }
        new_node = init_rop(6, 8); // Create an OR node
        if (new_node == NULL) { return 0; }
        *param_1 = new_node; // The parent's child pointer now points to this OR node
        *(void **)((char *)new_node + 0xc) = current_node_head; // Left child is the expression parsed so far
        param_1 = (void **)((char *)new_node + 0x10); // Update param_1 to point to the right child slot of the OR node
        current_node_tail = NULL; // Reset for parsing the right-hand side of OR
        current_node_head = NULL;
        handled_special_case = true; // This path handles its own loop progression
        break;
      case '$': // End of line anchor
        new_node = init_rop(1, 0);
        break;
      case '(': // Start of sub-expression
        new_node = init_rop(5, 0); // Create a node of type 5 (sub-expression)
        if (new_node == NULL) { return 0; }
        sub_parse_len = regexp_parse((void **)((char *)new_node + 0xc), param_2 + index + 1, ')'); // Recursive call for content
        if (sub_parse_len == 0) { return 0; }
        index += sub_parse_len; // Advance index by length of sub-expression
        break;
      case '*': // Zero or more quantifier
        if (current_node_tail == NULL || ((char *)current_node_tail)[1] != '\0') { return 0; } // Must follow a node, and not already quantified
        ((char *)current_node_tail)[1] = 2; // Set quantifier flag on previous node
        break;
      case '+': // One or more quantifier
        if (current_node_tail == NULL || ((char *)current_node_tail)[1] != '\0') { return 0; }
        ((char *)current_node_tail)[1] = 1;
        break;
      case '.': // Any character
        new_node = init_rop(2, 0);
        break;
      case '?': // Zero or one quantifier
        if (current_node_tail == NULL || ((char *)current_node_tail)[1] != '\0') { return 0; }
        ((char *)current_node_tail)[1] = 3;
        break;
      case '[': // Character class
        sub_parse_len = parse_class(&new_node, param_2 + index + 1);
        if (sub_parse_len == 0) { return 0; }
        index += sub_parse_len;
        break;
      case '\\': // Escape character
        new_node = init_rop(3, 0); // Create literal char node
        if (new_node == NULL) { return 0; }
        if (param_2[index + 1] == '\0') { return 0; } // Must be followed by a character
        index++; // Consume '\'
        *(char *)((char *)new_node + 0xc) = param_2[index]; // Store escaped character
        break;
      case '^': // Start of line anchor
        new_node = init_rop(0, 0);
        break;
      default: // Literal character
        new_node = init_rop(3, 0); // Create literal char node
        if (new_node == NULL) { return 0; }
        *(char *)((char *)new_node + 0xc) = current_char; // Store character
        break;
    }

    if (new_node != NULL) { // If a new node was created in this iteration
      if (current_node_head == NULL) {
        current_node_head = new_node;
        current_node_tail = new_node;
      } else {
        *(void **)((char *)current_node_tail + 4) = new_node; // Link previous tail to new node
        current_node_tail = new_node; // New node becomes the new tail
      }
    }

    if (!handled_special_case) { // Increment index only if not handled by a special case
      index++;
    }
  }
}

// Function: regexp_init
// param_1 is expected to be an array of `char *` pointers, acting as a struct.
// param_1[0] = regexp_str
// param_1[1] = parsed_tree_root
// param_1[4] = simplified_tree_root
int regexp_init(char **param_1, char *param_2) {
  // Clear 0x1c bytes (7 pointers on 32-bit, or 3.5 pointers on 64-bit if char** is array of 8-byte pointers)
  // Assuming 0x1c bytes means 7 4-byte pointers, so it's likely a 32-bit context.
  // For Linux compilable C code, size_t is usually 8 bytes on 64-bit.
  // Let's assume `param_1` points to an array of `void *` for flexibility.
  // `char **param_1` implies `sizeof(char*)`. `0x1c` means 7 `char*` on 32-bit.
  // `char *param_1[7]` would be `7 * sizeof(char*)`.
  // So `memset(param_1, 0, 0x1c);` would zero out `param_1[0]` through `param_1[6]`.
  memset(param_1, 0, 0x1c);
  param_1[0] = strdup(param_2); // Store original regex string

  if (param_1[0] == NULL) {
    regexp_free((void **)param_1);
    return 0;
  }

  // Parse the regex string, store root of parsed tree in param_1[1]
  int parse_result = regexp_parse((void **)(param_1 + 1), param_1[0], 0);
  if (parse_result == 0) {
    regexp_free((void **)param_1);
    return 0;
  }

  // Simplify the parsed tree, store root of simplified tree in param_1[4]
  void *simplified_tree_root = simplify((char **)(param_1 + 1), NULL); // param_2 for simplify is 0 (NULL)
  param_1[4] = (char *)simplified_tree_root;

  if (param_1[4] != NULL) {
    return 1;
  }

  regexp_free((void **)param_1);
  return 0;
}

// Function: _recursive_free
void _recursive_free(void *param_1_ptr) {
  char *param_1 = (char *)param_1_ptr;
  if (param_1 != NULL && param_1[1] != '\x04') { // Check if node is not NULL and not already freed/visited
    param_1[1] = '\x04'; // Mark node as visited/freed

    char node_type = param_1[0];

    if (node_type == '\x05') { // Sub-expression node
      _recursive_free(*(void **)(param_1 + 0xc)); // Free child node
    } else if (node_type == '\x06') { // OR node
      _recursive_free(*(void **)(param_1 + 0xc)); // Free left child
      _recursive_free(*(void **)(param_1 + 0x10)); // Free right child
    }
    _recursive_free(*(void **)(param_1 + 4)); // Free next node in sequence
    free(param_1);
  }
}

// Function: regexp_free
int regexp_free(void **param_1) {
  free(*param_1); // Free the regex string (param_1[0])
  _recursive_free(param_1[1]); // Free the parsed tree (param_1[1])
  // If param_1[4] (simplified_tree) is distinct and not reachable from param_1[1], it should be freed separately.
  // However, the original code only frees param_1[1]. Assuming simplified tree nodes are part of or replace
  // nodes in the original tree such that _recursive_free(param_1[1]) covers all allocated nodes.
  return 1;
}

// Function: check_prev_state
// param_1 is a pointer to the RegexpMatchState struct.
// param_2 is a node pointer (stored as an integer ID).
int check_prev_state(void *state_ptr, uintptr_t node_id) {
  unsigned int current_index = 0;
  unsigned int *current_size_ptr = (unsigned int *)((char *)state_ptr + 0x18); // Offset for count
  uintptr_t **array_ptr_field = (uintptr_t **)((char *)state_ptr + 0x14); // Offset for array pointer

  while (current_index < *current_size_ptr) {
    if (node_id == (*array_ptr_field)[current_index]) {
      return 0; // State already exists
    }
    current_index++;
  }

  // State not found, add it
  (*array_ptr_field)[*current_size_ptr] = node_id;
  (*current_size_ptr)++;
  return 1;
}

// Function: add_state
// param_1 is a pointer to the RegexpMatchState struct.
// param_2 is a node pointer (stored as an integer ID).
void add_state(void *state_ptr, uintptr_t node_id) {
  unsigned int current_index = 0;
  unsigned int *current_size_ptr = (unsigned int *)((char *)state_ptr + 0xc); // Offset for count
  uintptr_t **array_ptr_field = (uintptr_t **)((char *)state_ptr + 0x8); // Offset for array pointer

  while (current_index < *current_size_ptr) {
    if (node_id == (*array_ptr_field)[current_index]) {
      return; // State already exists
    }
    current_index++;
  }

  // State not found, add it
  (*array_ptr_field)[*current_size_ptr] = node_id;
  (*current_size_ptr)++;
  return;
}

// Function: _regexp_match
// param_1 is a pointer to the RegexpMatchState struct.
// param_2 is a pointer to the current regex node.
// param_3 is the character value to match (or special flags 0x100 for start, 0x101 for end).
int _regexp_match(void *state_ptr, void *node_ptr, int char_val) {
  bool is_not_start_of_string = (char_val != 0x100);
  bool is_not_end_of_string = (char_val != 0x101);

  char *node = (char *)node_ptr;
  unsigned int check_result = 0;

  switch (node[0]) { // Node type
    case 0: // '^' (start of line)
      if (is_not_start_of_string) { return 0; }
      check_result = check_prev_state(state_ptr, (uintptr_t)node_ptr);
      break;
    case 1: // '$' (end of line)
      if (is_not_end_of_string) { return 0; }
      check_result = check_prev_state(state_ptr, (uintptr_t)node_ptr);
      break;
    case 2: // '.' (any character)
      if (!is_not_start_of_string) { // If it's start of string char_val (0x100)
        add_state(state_ptr, (uintptr_t)node_ptr);
        return 0;
      }
      if (!is_not_end_of_string) { return 0; } // If it's end of string char_val (0x101)
      goto common_tail_processing; // Fall through to common sequence handling
    case 3: // Literal character
      if (!is_not_start_of_string) {
        add_state(state_ptr, (uintptr_t)node_ptr);
        return 0;
      }
      if (!is_not_end_of_string) { return 0; }
      if (char_val != node[0xc]) { return 0; } // Compare char_val with stored literal character
      goto common_tail_processing;
    case 4: // Character class '['
      if (!is_not_start_of_string) {
        add_state(state_ptr, (uintptr_t)node_ptr);
        return 0;
      }
      if (!is_not_end_of_string) { return 0; }
      unsigned int char_val_u = (unsigned int)char_val;
      // The original code has `if (param_3 < 0) { iVar2 = param_3 + 7; }`
      // For char values (0-255), this branch is not taken.
      // Assuming char_val is positive for actual characters, or special flags.
      // Bit check for character in class bitmap.
      check_result = ((uint8_t)node[(char_val_u >> 3) + 0xc] >> (char_val_u & 7)) & 1;
      break;
    case 5: // Parenthesized group (sub-expression)
      return _regexp_match(state_ptr, *(void **)(node + 0xc), char_val); // Recurse on child
    case 6: // '|' (OR operator)
      // Try left child
      if (_regexp_match(state_ptr, *(void **)(node + 0xc), char_val) != 0) {
        return 1;
      }
      // If left child fails, try right child
      return _regexp_match(state_ptr, *(void **)(node + 0x10), char_val);
    default:
      goto common_tail_processing;
  }

  if (check_result == 0) { return 0; } // If a check (like check_prev_state or char class) failed

common_tail_processing: // Handles advancing to the next node in a sequence
  // This loop finds the effective "next" node in the sequence, considering parent links
  // if the current node doesn't have a direct "next" (offset 4 is NULL).
  while (node_ptr != NULL && *(void **)((char *)node_ptr + 4) == NULL) {
    node_ptr = *(void **)((char *)node_ptr + 8); // Move to previous node in sequence (parent)
  }

  if (node_ptr != NULL) {
    // Add the "next" node in the sequence to the current states
    add_state(state_ptr, (uintptr_t)*(void **)((char *)node_ptr + 4));
  }
  return 0; // No match yet, but state added for next step
}

// Function: regexp_match
// param_1 is a pointer to the RegexpMatchState struct.
// param_2 is the input string to match.
int regexp_match(void *regexp_obj_ptr, const char *text) {
  int result = 0; // Default to no match
  void *current_states_array = NULL;
  void *next_states_array = NULL;
  void *prev_check_states_array = NULL;

  // Cast regexp_obj_ptr to its internal structure for easier access
  // Using direct pointer arithmetic as no explicit struct for regexp_obj_ptr was provided.
  // Offsets are based on original code's access patterns.
  // 0x4: regex_root_node (void*)
  // 0x8: current_states (void*)
  // 0xc: current_states_count (unsigned int)
  // 0x10: max_states_capacity (size_t)
  // 0x14: prev_check_states (void*)
  // 0x18: prev_check_states_count (unsigned int)

  // Initialize prev_check_states to NULL and get capacity
  *(void **)((char *)regexp_obj_ptr + 0x14) = NULL;
  size_t capacity = *(size_t *)((char *)regexp_obj_ptr + 0x10);

  bool allocations_successful = false;

  // Allocate state arrays
  current_states_array = calloc(capacity, sizeof(uintptr_t)); // Store node pointers (uintptr_t)
  if (current_states_array != NULL) {
    next_states_array = calloc(capacity, sizeof(uintptr_t));
    if (next_states_array != NULL) {
      prev_check_states_array = calloc(capacity, sizeof(uintptr_t));
      *(void **)((char *)regexp_obj_ptr + 0x14) = prev_check_states_array; // Store pointer to prev_check_states
      if (prev_check_states_array != NULL) {
        allocations_successful = true;
      }
    }
  }

  if (allocations_successful) {
    result = 1; // Assume match initially, will be set to 0 if no match found

    // Initialize current state
    *(void **)((char *)regexp_obj_ptr + 0x8) = current_states_array;
    *(unsigned int *)((char *)regexp_obj_ptr + 0xc) = 0;
    *(unsigned int *)((char *)regexp_obj_ptr + 0x18) = 0;

    // Initial match check for start of string (0x100)
    int iVar3 = _regexp_match(regexp_obj_ptr, *(void **)((char *)regexp_obj_ptr + 0x4), 0x100);
    if (iVar3 != 0) { // If a match is found at the very beginning
      // result remains 1
    } else {
      bool matched_during_text_loop = false;
      int text_index = 0;

      // Iterate through each character of the input text
      while (text[text_index] != '\0' && !matched_during_text_loop) {
        // Add the regex root node as a possible starting point for current character processing
        add_state(regexp_obj_ptr, (uintptr_t)*(void **)((char *)regexp_obj_ptr + 0x4));

        // Swap current and next state arrays
        void *temp_ptr_swap = *(void **)((char *)regexp_obj_ptr + 0x8);
        unsigned int current_count = *(unsigned int *)((char *)regexp_obj_ptr + 0xc);

        *(void **)((char *)regexp_obj_ptr + 0x8) = (temp_ptr_swap == current_states_array) ? next_states_array : current_states_array;
        *(unsigned int *)((char *)regexp_obj_ptr + 0xc) = 0;
        *(unsigned int *)((char *)regexp_obj_ptr + 0x18) = 0; // Reset prev_check_states_count

        // Process each state from the previous iteration with the current character
        for (unsigned int i = 0; i < current_count; i++) {
          uintptr_t node_id = ((uintptr_t*)temp_ptr_swap)[i];
          iVar3 = _regexp_match(regexp_obj_ptr, (void*)node_id, (int)text[text_index]);
          if (iVar3 != 0) {
            matched_during_text_loop = true; // Match found
            break; // Exit inner loop
          }
        }
        text_index++;
      }

      if (matched_during_text_loop) {
        // result remains 1
      } else {
        // After processing all characters, check for end-of-string match (0x101)
        add_state(regexp_obj_ptr, (uintptr_t)*(void **)((char *)regexp_obj_ptr + 0x4));

        void *temp_ptr_swap = *(void **)((char *)regexp_obj_ptr + 0x8);
        unsigned int current_count = *(unsigned int *)((char *)regexp_obj_ptr + 0xc);

        *(void **)((char *)regexp_obj_ptr + 0x8) = (temp_ptr_swap == current_states_array) ? next_states_array : current_states_array;
        *(unsigned int *)((char *)regexp_obj_ptr + 0xc) = 0;
        *(unsigned int *)((char *)regexp_obj_ptr + 0x18) = 0;

        for (unsigned int i = 0; i < current_count; i++) {
          uintptr_t node_id = ((uintptr_t*)temp_ptr_swap)[i];
          iVar3 = _regexp_match(regexp_obj_ptr, (void*)node_id, 0x101);
          if (iVar3 != 0) {
            matched_during_text_loop = true; // Match found at end of string
            break;
          }
        }

        if (!matched_during_text_loop) {
          result = 0; // No match found after all checks
        }
      }
    }
  }

  // Cleanup allocated memory
  free(current_states_array);
  free(next_states_array);
  free(*(void **)((char *)regexp_obj_ptr + 0x14)); // Free prev_check_states_array
  *(void **)((char *)regexp_obj_ptr + 0x14) = NULL;
  *(void **)((char *)regexp_obj_ptr + 0x8) = NULL; // Clear current_states pointer
  return result;
}