#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h> // For va_list, va_start, va_end
#include <unistd.h> // For STDOUT_FILENO

// Define ushort if not available (common on some systems)
#ifndef _USHORT_DEFINED
typedef unsigned short ushort;
#define _USHORT_DEFINED
#endif

// Global constants and buffer sizes
#define MAX_STATES 1000 // Assuming max 1000 states based on 4000 byte allocations / sizeof(int or ptr)

// Struct for State representation
typedef struct State {
    int id;             // Unique identifier for the state (index in g_states array)
    char type;          // Character type for transition, or 0x80 for epsilon (-128 as signed char)
    char accepting;     // 0 for non-accepting, non-0 for accepting
    struct State *next1; // Pointer to next state 1
    struct State *next2; // Pointer to next state 2
} State;

// Global variables for the regex engine and debugging
State *g_states = NULL;       // Array of State objects
int g_num_states = 0;         // Current number of states allocated

int *g_states_checked = NULL; // Array of int (state IDs) for debug_state_helper traversal
int g_state_length = 0;       // Current number of states in g_states_checked

// For match_helper: Stores remaining string length when a state was last visited to detect epsilon loops
int *g_epsilon_loop = NULL; // Array of int (remaining string length)

int *g_stack = NULL;          // Stack for evalrpn function

// Mock/Placeholder functions (as they are not provided in the snippet)
// fdprintf is assumed to be a custom printf to a file descriptor.
// For Linux compilable, we map it to fprintf(stdout, ...) or fprintf(stderr, ...)
int fdprintf(int fd, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vfprintf(fd == STDOUT_FILENO ? stdout : stderr, format, args);
    va_end(args);
    return ret;
}

// Global string literals (from DAT_0001xxxx in original snippet)
const char DAT_000152eb[] = "  "; // Indentation
const char DAT_000152ef[] = "State Id::%d"; // For epsilon state
const char DAT_000152f3[] = "Char::%c"; // For character state
const char DAT_00015310[] = "\n"; // Newline after state info
const char DAT_00015312[] = "Matched::%s\n"; // For match function

// Regex operation placeholders - these are highly simplified and would need proper NFA construction logic
// to function as a real regex engine. They are provided to allow the main functions to compile.

// Creates a new state and returns its ID (index in g_states array)
int create_state(unsigned char character) {
    if (g_states == NULL) {
        g_states = (State *)calloc(MAX_STATES, sizeof(State));
        if (!g_states) {
            perror("malloc failed for g_states");
            return 0;
        }
    }
    if (g_num_states >= MAX_STATES) {
        fprintf(stderr, "Error: Max states reached (%d).\n", MAX_STATES);
        return 0;
    }

    State *new_state = &g_states[g_num_states];
    new_state->id = g_num_states;
    new_state->type = (char)character;
    new_state->accepting = 0;
    new_state->next1 = NULL;
    new_state->next2 = NULL;
    g_num_states++;
    return new_state->id;
}

// Clears all allocated regex states and global buffers
void clear_trex() {
    if (g_states) {
        free(g_states);
        g_states = NULL;
    }
    g_num_states = 0;

    if (g_states_checked) {
        free(g_states_checked);
        g_states_checked = NULL;
    }
    g_state_length = 0;

    if (g_epsilon_loop) {
        free(g_epsilon_loop);
        g_epsilon_loop = NULL;
    }

    if (g_stack) {
        free(g_stack);
        g_stack = NULL;
    }
}

// Simplified regex operators - these are conceptual placeholders.
// A real NFA construction would be much more complex.
// For compilation, they return a valid state ID or 0 on error.
int op_union(int s1_id, int s2_id) {
    if (s1_id == 0 && g_num_states > 0) return 0; // Invalid state ID 0 if g_num_states > 0 (error from create_state)
    if (s2_id == 0 && g_num_states > 0) return 0;

    int new_start_id = create_state((char)0x80); // Epsilon start state
    if (new_start_id == 0 && g_num_states > 0) return 0;
    g_states[new_start_id].next1 = &g_states[s1_id];
    g_states[new_start_id].next2 = &g_states[s2_id];
    return new_start_id;
}

int op_concat(int s1_id, int s2_id) {
    if (s1_id == 0 && g_num_states > 0) return 0;
    if (s2_id == 0 && g_num_states > 0) return 0;

    // In a simple NFA, this would involve linking accepting states of s1 to the start of s2.
    // For this mock, we'll return s1_id, assuming s1's accepting states are somehow modified
    // to point to s2's start (not directly supported by this simple State struct).
    // This is a simplification to allow compilation.
    return s1_id; 
}

int op_star(int s_id) {
    if (s_id == 0 && g_num_states > 0) return 0;

    int new_start_id = create_state((char)0x80); // Epsilon start state
    if (new_start_id == 0 && g_num_states > 0) return 0;
    g_states[new_start_id].accepting = 1; // Can match empty string
    g_states[new_start_id].next1 = &g_states[s_id]; // Epsilon transition to s
    // Actual NFA construction would involve more complex linking for loops.
    return new_start_id;
}

int op_plus(int s_id) {
    if (s_id == 0 && g_num_states > 0) return 0;

    int new_start_id = create_state((char)0x80); // Epsilon start state
    if (new_start_id == 0 && g_num_states > 0) return 0;
    g_states[new_start_id].next1 = &g_states[s_id]; // Epsilon transition to s
    // Actual NFA construction for s+ (s followed by s*) is more involved.
    return new_start_id;
}

int op_qmark(int s_id) {
    if (s_id == 0 && g_num_states > 0) return 0;

    int new_start_id = create_state((char)0x80); // Epsilon start state
    if (new_start_id == 0 && g_num_states > 0) return 0;
    g_states[new_start_id].accepting = 1; // Can match empty string
    g_states[new_start_id].next1 = &g_states[s_id]; // Epsilon transition to s
    return new_start_id;
}


// Function: debug_state_helper
void debug_state_helper(State *state, int depth) {
    if (state == NULL) {
        return;
    }

    // Check if state already visited in this debug session
    for (int i = 0; i < g_state_length; ++i) {
        if (g_states_checked[i] == state->id) {
            return; // Already printed this state in current traversal
        }
    }

    // Mark state as visited
    if (g_states_checked == NULL) {
        g_states_checked = (int *)malloc(MAX_STATES * sizeof(int));
        if (!g_states_checked) {
            perror("malloc failed for g_states_checked");
            return;
        }
    }
    if (g_state_length < MAX_STATES) {
        g_states_checked[g_state_length++] = state->id;
    } else {
        fprintf(stderr, "Warning: g_states_checked buffer full. State ID %d not recorded.\n", state->id);
    }

    // Print indentation
    for (int i = 0; i < depth; ++i) {
        fdprintf(STDOUT_FILENO, DAT_000152eb);
    }

    // Print state information
    if (state->type == (char)0x80) { // Epsilon state (0x80 is -128 as signed char)
        fdprintf(STDOUT_FILENO, DAT_000152ef, state->id);
    } else { // Character state
        fdprintf(STDOUT_FILENO, DAT_000152f3, state->type);
    }

    if (state->accepting != '\0') {
        fdprintf(STDOUT_FILENO, "::Accepting State Id::%d", state->id);
    }
    fdprintf(STDOUT_FILENO, DAT_00015310); // Newline

    // Recursive calls
    debug_state_helper(state->next1, depth + 1);
    debug_state_helper(state->next2, depth + 1);
}

// Function: match_helper
int match_helper(State *state, char *str, int matched_len, int full_match_mode) {
    if (state == NULL) {
        return 0;
    }

    // Epsilon loop detection and prevention
    // g_epsilon_loop is initialized by the `match` function for each new attempt.
    int remaining_len = strlen(str);
    if (g_epsilon_loop[state->id] != 1000 && g_epsilon_loop[state->id] < remaining_len) {
        // If we've visited this state before with a longer or equal remaining string,
        // it means we're in an epsilon loop without consuming characters, or already explored this path.
        return 0;
    }
    // Update the remaining length for this state
    g_epsilon_loop[state->id] = remaining_len;

    int chars_to_advance = 1;
    int is_accepting = state->accepting != '\0';

    if (state->type == (char)0x80) { // Epsilon transition (0x80 is -128 as signed char)
        chars_to_advance = 0; // No character consumed

        // Check for accepting state under epsilon transition
        if (is_accepting && matched_len != 0) {
            // If not full_match_mode, or (full_match_mode AND end of string), then it's a match.
            if (!full_match_mode || (full_match_mode && *str == '\0')) {
                return matched_len; // Return current matched_len (no char consumed)
            }
        }
    } else { // Character transition
        if (*str != state->type) {
            return 0; // Character mismatch, no path
        }
        // Character matched. Check if this is an accepting state.
        if (is_accepting) {
            // If not full_match_mode, or (full_match_mode AND end of string), then it's a match.
            if (!full_match_mode || (full_match_mode && str[1] == '\0')) {
                return matched_len + 1; // Return matched_len + 1 (char consumed)
            }
        }
    }

    // Explore transitions recursively
    int result1 = match_helper(state->next2, str + chars_to_advance, matched_len + chars_to_advance, full_match_mode);
    int result2 = match_helper(state->next1, str + chars_to_advance, matched_len + chars_to_advance, full_match_mode);

    return (result1 > result2) ? result1 : result2; // Return the maximum match length
}

// Function: debug_state
void debug_state(int start_state_id) {
    if (g_states == NULL || start_state_id < 0 || start_state_id >= g_num_states) {
        fprintf(stderr, "Error: Invalid start state ID %d for debug_state. Total states: %d.\n", start_state_id, g_num_states);
        return;
    }

    // Reset visited states for this debug session
    g_state_length = 0; // Clear the list of visited states for a new traversal

    debug_state_helper(&g_states[start_state_id], 0);
}

// Function: match
void match(int start_state_id, char *input_string, int full_match_mode) {
    if (g_states == NULL || start_state_id < 0 || start_state_id >= g_num_states) {
        fprintf(stderr, "Error: Invalid start state ID %d for match. Total states: %d.\n", start_state_id, g_num_states);
        return;
    }
    if (input_string == NULL) {
        return;
    }

    size_t input_len = strlen(input_string);
    char *working_copy = (char *)malloc(input_len + 1);
    if (!working_copy) {
        perror("malloc failed for working_copy");
        return;
    }
    memcpy(working_copy, input_string, input_len + 1);

    if (g_epsilon_loop == NULL) {
        g_epsilon_loop = (int *)malloc(MAX_STATES * sizeof(int));
        if (!g_epsilon_loop) {
            perror("malloc failed for g_epsilon_loop");
            free(working_copy);
            return;
        }
    }

    char *line_start = working_copy;
    char *current_char_ptr = working_copy;
    bool end_of_input_reached = false;

    while (true) {
        // Find end of current line
        while (*current_char_ptr != '\n' && *current_char_ptr != '\0') {
            current_char_ptr++;
        }
        end_of_input_reached = (*current_char_ptr == '\0');
        *current_char_ptr = '\0'; // Null-terminate the current line for processing

        char *line_iter = line_start;
        while (*line_iter != '\0') { // Iterate through the current line for potential matches
            // Reset epsilon loop tracker for each new match attempt
            // The original code initialized with 1000.
            for (int i = 0; i < g_num_states; i++) { // Only reset for active states
                g_epsilon_loop[i] = 1000; // Value indicating "not yet visited for current string"
            }

            int match_result = match_helper(&g_states[start_state_id], line_iter, 0, full_match_mode);
            if (match_result != 0) {
                fdprintf(STDOUT_FILENO, DAT_00015312, line_start); // Print the entire line if any match found
                break; // Found a match for this line, move to next line
            }
            if (full_match_mode == 1) { // If full match mode, only try from start of line
                break;
            }
            line_iter++; // Try matching from the next character on the line
        }

        if (end_of_input_reached) {
            free(working_copy);
            return;
        }

        // Move to the start of the next line (skipping the '\n' and '\0' from previous line)
        line_start = current_char_ptr + 1;
        current_char_ptr = line_start;
    }
}

// Function: evalrpn
int evalrpn(unsigned char *rpn_expression) {
    if (g_stack == NULL) {
        g_stack = (int *)malloc(MAX_STATES * sizeof(int)); // Stack can hold MAX_STATES state IDs
        if (!g_stack) {
            perror("malloc failed for g_stack");
            return 0;
        }
    }

    int *stack_ptr = g_stack; // Points to the next available slot

    for (; *rpn_expression != 0; rpn_expression++) {
        unsigned char current_op = *rpn_expression;
        int op_result = 0;

        // Check for stack overflow before pushing
        if (stack_ptr - g_stack >= MAX_STATES) {
            fprintf(stderr, "Error: RPN stack overflow. Max stack depth %d.\n", MAX_STATES);
            clear_trex();
            return 0;
        }

        // Handle operators
        if (current_op == 0xfc) { // Union operator
            if (stack_ptr - g_stack < 2) { op_result = 0; }
            else {
                int operand2 = *(--stack_ptr);
                int operand1 = *(--stack_ptr);
                op_result = op_union(operand1, operand2);
            }
        } else if (current_op == 0xbf) { // Qmark operator
            if (stack_ptr - g_stack < 1) { op_result = 0; }
            else {
                int operand1 = *(--stack_ptr);
                op_result = op_qmark(operand1);
            }
        } else if (current_op == 0xab) { // Plus operator
            if (stack_ptr - g_stack < 1) { op_result = 0; }
            else {
                int operand1 = *(--stack_ptr);
                op_result = op_plus(operand1);
            }
        } else if (current_op == 0xa6) { // Concat operator
            if (stack_ptr - g_stack < 2) { op_result = 0; }
            else {
                int operand2 = *(--stack_ptr);
                int operand1 = *(--stack_ptr);
                op_result = op_concat(operand1, operand2);
            }
        } else if (current_op == 0xaa) { // Star operator
            if (stack_ptr - g_stack < 1) { op_result = 0; }
            else {
                int operand1 = *(--stack_ptr);
                op_result = op_star(operand1);
            }
        } else { // Literal character (create_state)
            op_result = create_state(current_op);
        }

        if (op_result == 0 && g_num_states > 0) { // If any operation failed (returned 0), and states were created
            clear_trex();
            return 0;
        }
        
        *(stack_ptr++) = op_result; // Push result onto stack
    }

    // After processing the RPN expression, the stack should contain exactly one result
    if (stack_ptr == g_stack + 1) { // One item on stack
        int final_result = g_stack[0];
        g_stack[0] = 0; // Clear the result (original code did this, though not strictly necessary)
        return final_result;
    } else { // Stack empty or multiple items left (invalid RPN or error)
        clear_trex();
        return 0;
    }
}