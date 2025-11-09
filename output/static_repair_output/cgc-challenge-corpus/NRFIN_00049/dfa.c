#include <stdio.h>    // For NULL, if needed
#include <stdlib.h>   // For strtoul, rand, NULL
#include <string.h>   // For strncmp, strcmp, strrchr, memset
#include <stdbool.h>  // For bool
#include <stdint.h>   // For uintptr_t (for robust pointer casting)

// Define constants for DFA structure and limits
#define DFA_STATE_SIZE_BYTES 264 // 0x108 bytes per state
#define DFA_TRANSITIONS_OFFSET 8 // Offset of transitions array within DFAState
#define DFA_MAX_STATE_IDX 255    // Maximum valid state index (0xFF)
#define DFA_MAX_TRANSITIONS 256  // Size of transitions array (0x100 characters)
#define DFA_MAX_STATES (DFA_MAX_STATE_IDX + 1) // Total number of possible states (256)

// Bitset related constants. The bitset starts at a fixed offset from dfa_base.
#define DFA_BITSET_OFFSET 0x41c0
#define DFA_BITSET_SIZE (DFA_MAX_STATES / 8) // 32 bytes for 256 bits

// Type for a callback function when a match occurs
typedef void (*on_match_callback_t)(void);

// Structure for a single DFA state
// Assumes a 32-bit address space for function and data pointers,
// consistent with the original `undefined4` type.
typedef struct {
    unsigned int on_match_func_ptr; // Stores address of on_match_callback_t
    unsigned int on_match_data_ptr; // Stores address of user data
    unsigned char transitions[DFA_MAX_TRANSITIONS]; // Transition table
} DFAState; // Total size: 2 * sizeof(unsigned int) + DFA_MAX_TRANSITIONS = 4+4+256 = 264 bytes (0x108)

// Helper function for bitset initialization
// bitset_ptr points to the start of the bitset (unsigned char array)
void bitset_init(unsigned char* bitset_ptr, unsigned char max_bit_idx) {
    memset(bitset_ptr, 0, (max_bit_idx / 8) + 1);
}

// Helper function to set a bit in the bitset
unsigned int bitset_set_bit(unsigned char* bitset_ptr, unsigned char bit_idx) {
    bitset_ptr[bit_idx / 8] |= (1 << (bit_idx % 8));
    return 0; // Original returns 0
}

// Helper function to get a bit from the bitset
unsigned int bitset_get_bit(const unsigned char* bitset_ptr, unsigned char bit_idx) {
    return (bitset_ptr[bit_idx / 8] >> (bit_idx % 8)) & 1;
}

// Helper function for string to unsigned int conversion
// Returns 0 on success, -1 on error
int str_to_uint(const char* s, unsigned int base, unsigned int* out_val) {
    char* endptr;
    unsigned long val = strtoul(s, &endptr, base);
    if (s == endptr || *endptr != '\0') { // No conversion or extra characters
        return -1; // Error
    }
    *out_val = (unsigned int)val;
    return 0; // Success
}

// Function: dfa_init
void dfa_init(unsigned int *dfa_base) {
  // Initialize states (0 to DFA_MAX_STATE_IDX - 1, i.e., 0 to 254)
  for (unsigned int state_idx = 0; state_idx < DFA_MAX_STATE_IDX; ++state_idx) {
    // Calculate pointer to current state
    DFAState *current_state = (DFAState*)((char*)(dfa_base + 2) + state_idx * DFA_STATE_SIZE_BYTES);

    // Initialize all transitions for the current state to 0xff (undefined)
    for (unsigned int char_idx = 0; char_idx < DFA_MAX_TRANSITIONS; ++char_idx) {
      current_state->transitions[char_idx] = 0xff;
    }

    // Initialize on_match_func_ptr and on_match_data_ptr to 0
    current_state->on_match_func_ptr = 0;
    current_state->on_match_data_ptr = 0;
  }

  // Initialize the bitset at its specific offset within the DFA structure
  bitset_init((unsigned char*)((char*)dfa_base + DFA_BITSET_OFFSET), DFA_MAX_STATE_IDX);

  // Initialize global DFA properties
  dfa_base[0] = 0; // DFA not yet "done" or fully initialized
  dfa_base[1] = 0; // Current state pointer (initially NULL or address of state 0)
}

// Function: dfa_reset
void dfa_reset(unsigned int *dfa_base) {
  // Set the current state pointer to the address of the first state (state 0)
  dfa_base[1] = (unsigned int)(uintptr_t)(dfa_base + 2);
}

// Function: dfa_update_onmatch
void * dfa_update_onmatch(unsigned int *dfa_base, unsigned int func_ptr, void *data_ptr) {
  // Iterate through possible state indices (0 to DFA_MAX_STATE_IDX - 1, i.e., 0 to 254)
  for (unsigned int state_idx = 0; state_idx < DFA_MAX_STATE_IDX; ++state_idx) {
    DFAState *current_state = (DFAState*)((char*)(dfa_base + 2) + state_idx * DFA_STATE_SIZE_BYTES);

    // If the state already has an on_match_func_ptr set (not 0), update it
    if (current_state->on_match_func_ptr != 0) {
      current_state->on_match_func_ptr = func_ptr;
      current_state->on_match_data_ptr = (unsigned int)(uintptr_t)data_ptr;
    }
  }
  return data_ptr; // Original function returned param_3
}

// Function: validate_dfa
unsigned int validate_dfa(unsigned int *dfa_base, unsigned char num_states_to_validate) {
  // Iterate through the specified number of states (0 to num_states_to_validate - 1)
  for (unsigned char state_idx = 0; state_idx < num_states_to_validate; ++state_idx) {
    DFAState *current_state = (DFAState*)((char*)(dfa_base + 2) + state_idx * DFA_STATE_SIZE_BYTES);

    // Check all possible character transitions for the current state
    for (unsigned int char_val = 0; char_val < DFA_MAX_TRANSITIONS; ++char_val) {
      unsigned char next_state_idx = current_state->transitions[char_val];
      // If a transition exists (not 0xff) and points to an invalid state index
      if ((next_state_idx != 0xff) && (num_states_to_validate <= next_state_idx)) {
        return 0xffffffff; // Validation failed
      }
    }
  }
  return 0; // All transitions are valid
}

// Function: dfa_parse_desc
unsigned int dfa_parse_desc(unsigned int *dfa_base, char *description_line,
                            unsigned int func_ptr_val, unsigned int data_ptr_val) {
  int cmp_result;
  unsigned int current_state_idx;
  DFAState *current_state_obj;

  cmp_result = strncmp(description_line, "state", 5);
  if (cmp_result == 0) {
    // Determine the address of the new state
    if (dfa_base[1] == 0) {
      // If no current state is set, new state is the first (state 0)
      current_state_obj = (DFAState*)(dfa_base + 2);
    } else {
      // Otherwise, new state is the one immediately after the current state
      current_state_obj = (DFAState*)((char*)((uintptr_t)dfa_base[1]) + DFA_STATE_SIZE_BYTES);
    }
    // Update dfa_base[1] to point to the new current state
    dfa_base[1] = (unsigned int)(uintptr_t)current_state_obj;

    cmp_result = strcmp(description_line + 6, "accept");
    // If the description is "state accept" and it's not the very first state (state 0)
    if ((cmp_result == 0) && (current_state_obj != (DFAState*)(dfa_base + 2))) {
      current_state_obj->on_match_func_ptr = func_ptr_val;
      current_state_obj->on_match_data_ptr = data_ptr_val;

      // Calculate current state index and set the corresponding bit in the bitset
      current_state_idx = ((uintptr_t)current_state_obj - (uintptr_t)(dfa_base + 2)) / DFA_STATE_SIZE_BYTES;
      bitset_set_bit((unsigned char*)((char*)dfa_base + DFA_BITSET_OFFSET), (unsigned char)current_state_idx);
    }
    return 0; // Success
  } else {
    cmp_result = strcmp(description_line, "done");
    if (cmp_result == 0) {
      // Calculate the number of states that have been defined
      current_state_idx = ((uintptr_t)dfa_base[1] - (uintptr_t)(dfa_base + 2)) / DFA_STATE_SIZE_BYTES;
      // Validate the DFA with the actual number of defined states (+1 for count)
      if (validate_dfa(dfa_base, (unsigned char)(current_state_idx + 1)) == 0) {
        dfa_base[0] = 1; // Mark DFA as "done" (initialized)
        dfa_base[1] = (unsigned int)(uintptr_t)(dfa_base + 2); // Reset current state to state 0
        return 1; // Success
      } else {
        return 0xffffffff; // Validation failed
      }
    } else {
      cmp_result = strncmp(description_line, "transition ", 11);
      if (cmp_result == 0) {
        char *transition_chars_start = description_line + 11;
        char *space_ptr = strrchr(transition_chars_start, ' ');

        if (space_ptr == NULL) {
          return 0xffffffff; // Malformed transition string (missing space before next state index)
        } else {
          unsigned int next_state_idx;
          // Temporarily null-terminate the character string part
          *space_ptr = '\0';

          // Parse the next state index
          if (str_to_uint(space_ptr + 1, 10, &next_state_idx) == -1 || next_state_idx > DFA_MAX_STATE_IDX) {
            return 0xffffffff; // Invalid next state index
          } else {
            current_state_obj = (DFAState*)((uintptr_t)dfa_base[1]);
            // Update transitions for each character in the string
            for (char *c = transition_chars_start; *c != '\0'; ++c) {
              current_state_obj->transitions[(unsigned char)*c] = (unsigned char)next_state_idx;
            }
            return 0; // Success
          }
        }
      } else {
        return 0xffffffff; // Unknown command
      }
    }
  }
}

// Function: dfa_process_input
unsigned int dfa_process_input(unsigned int *dfa_base, char input_char) {
  if (dfa_base[0] == 0) { // If DFA not marked as "done" (initialized)
    return 0xffffffff;
  }

  DFAState *current_state_obj = (DFAState*)((uintptr_t)dfa_base[1]);
  unsigned char next_state_idx = current_state_obj->transitions[(unsigned char)input_char];

  if (next_state_idx == 0xff) { // No valid transition for the input character
    return 0xffffffff;
  } else {
    // Update the current state pointer to the new state
    dfa_base[1] = (unsigned int)(uintptr_t)((char*)(dfa_base + 2) + next_state_idx * DFA_STATE_SIZE_BYTES);
    current_state_obj = (DFAState*)((uintptr_t)dfa_base[1]); // Update current_state_obj to the new state

    // Check if the new state is an accept state
    unsigned int state_idx = ((uintptr_t)current_state_obj - (uintptr_t)(dfa_base + 2)) / DFA_STATE_SIZE_BYTES;
    if (bitset_get_bit((unsigned char*)((char*)dfa_base + DFA_BITSET_OFFSET), (unsigned char)state_idx) == 1) {
      // If it's an accept state and has an on_match_func_ptr, call the callback
      if (current_state_obj->on_match_func_ptr != 0) {
        ((on_match_callback_t)(uintptr_t)current_state_obj->on_match_func_ptr)();
      }
      return 1; // Matched an accept state
    } else {
      return 0; // No match
    }
  }
}

// Function: dfa_give_example
unsigned int dfa_give_example(unsigned int *dfa_base, char *buffer, unsigned int max_len) {
  unsigned int original_current_state_ptr = dfa_base[1]; // Save original current state
  dfa_base[1] = (unsigned int)(uintptr_t)(dfa_base + 2); // Start pathfinding from state 0
  unsigned int example_len = 0;
  unsigned int result = 0xffffffff; // Default to error
  bool success_path_found = false;
  bool possible_to_continue = true;

  if (dfa_base[0] != 0) { // If DFA is marked as "done" (initialized)
    // Check if any accept states exist in the DFA
    bool has_accept_states = false;
    for (unsigned char state_idx = 0; state_idx < DFA_MAX_STATES; ++state_idx) {
      if (bitset_get_bit((unsigned char*)((char*)dfa_base + DFA_BITSET_OFFSET), state_idx) == 1) {
        has_accept_states = true;
        break;
      }
    }

    if (has_accept_states) {
      // Loop to build the example string
      while (example_len < max_len - 1 && possible_to_continue) { // Leave space for null terminator
        DFAState *current_state_obj = (DFAState*)((uintptr_t)dfa_base[1]);
        unsigned int current_state_idx = ((uintptr_t)current_state_obj - (uintptr_t)(dfa_base + 2)) / DFA_STATE_SIZE_BYTES;

        // If the current state is an accept state, a path has been found
        if (bitset_get_bit((unsigned char*)((char*)dfa_base + DFA_BITSET_OFFSET), (unsigned char)current_state_idx) == 1) {
          success_path_found = true;
          break; // Exit loop, example found
        }

        // Count valid outgoing transitions from the current state
        unsigned int valid_transitions_count = 0;
        for (unsigned int char_val = 0; char_val < DFA_MAX_TRANSITIONS; ++char_val) {
          if (current_state_obj->transitions[char_val] != 0xff) {
            valid_transitions_count++;
          }
        }

        if (valid_transitions_count == 0) {
          possible_to_continue = false; // No outgoing transitions, cannot reach an accept state from here
        } else {
          // Select a random valid transition character
          unsigned int random_choice = rand() % valid_transitions_count;
          char chosen_char = '\0';
          for (unsigned int char_val = 0; char_val < DFA_MAX_TRANSITIONS; ++char_val) {
            if (current_state_obj->transitions[char_val] != 0xff) {
              if (random_choice == 0) {
                chosen_char = (char)char_val;
                break;
              }
              random_choice--;
            }
          }

          if (chosen_char == '\0') { // Should not happen if valid_transitions_count > 0
            possible_to_continue = false;
          } else {
            buffer[example_len] = chosen_char; // Add character to the example string
            // Move to the next state based on the chosen character
            dfa_base[1] = (unsigned int)(uintptr_t)((char*)(dfa_base + 2) + current_state_obj->transitions[(unsigned char)chosen_char] * DFA_STATE_SIZE_BYTES);
            example_len++;
          }
        }
      } // End of while loop

      // Null-terminate the buffer
      buffer[example_len] = '\0';

      if (success_path_found) {
        result = 0; // Successfully found an example path
      } else if (possible_to_continue) { // Loop finished because max_len was reached
        // Final check if the last state reached is an accept state
        DFAState *final_state_obj = (DFAState*)((uintptr_t)dfa_base[1]);
        unsigned int final_state_idx = ((uintptr_t)final_state_obj - (uintptr_t)(dfa_base + 2)) / DFA_STATE_SIZE_BYTES;
        if (bitset_get_bit((unsigned char*)((char*)dfa_base + DFA_BITSET_OFFSET), (unsigned char)final_state_idx) == 1) {
            result = 0; // Successfully found an example
        } else {
            result = 0xffffffff; // Did not end in an accept state
        }
      } else {
          result = 0xffffffff; // No path found or got stuck
      }
    }
  }

  dfa_base[1] = original_current_state_ptr; // Restore original current state
  return result;
}