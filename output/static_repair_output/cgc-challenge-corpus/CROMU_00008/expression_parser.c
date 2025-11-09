#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h> // For uint32_t, uint16_t, uint8_t

// --- Global variables and Type definitions ---

typedef uint32_t undefined4;
typedef uint16_t undefined2;
typedef uint8_t byte;

// Global variables - assuming their types and purpose based on usage
int g_parseStackSize;
byte *g_memoryGlobal; // Assuming it's a byte array for general memory, acts as parser stack

// Structure for parse table entries
struct ParseTableEntry {
    char *symbol_str;
    int symbol_id;
};
struct ParseTableEntry *g_parseTable; // Pointer to an array of ParseTableEntry

// --- External functions (declarations) ---
// These functions are called but not defined in the snippet.
// Their signatures are inferred from their usage in the provided code.
unsigned int db_get_record_count(void);
void *db_search_index(unsigned int record_idx); // Returns pointer to record data for a given index
int date_compare(void *record_data, uint32_t date_val, uint16_t date_len); // Compares date in record with supplied date
int parse_date(const char *input_str, uint32_t *out_date_val, uint16_t *out_date_len); // Parses date string
void print_record_helper(void *record_data); // Prints a database record

// --- Helper Functions ---

// Function: peek_parser_stack
uint32_t *peek_parser_stack(uint32_t *param_1) {
    if (g_parseStackSize == 0) {
        uint32_t default_stack_item[18] = {2}; // Default for empty stack: Type 2
        memcpy(param_1, default_stack_item, sizeof(default_stack_item));
    } else {
        // Calculate pointer to the top of the stack (g_parseStackSize - 1) * 0x48
        uint32_t *stack_top_ptr = (uint32_t *)(g_memoryGlobal + (g_parseStackSize - 1) * 0x48);
        memcpy(param_1, stack_top_ptr, 18 * sizeof(uint32_t));
    }
    return param_1;
}

// Function: pop_parser_stack
uint32_t *pop_parser_stack(uint32_t *param_1) {
    if (g_parseStackSize == 0) {
        uint32_t default_stack_item[18] = {2}; // Default for empty stack: Type 2
        memcpy(param_1, default_stack_item, sizeof(default_stack_item));
    } else {
        g_parseStackSize--; // Decrement stack size
        // Calculate pointer to the new top of the stack (g_parseStackSize) * 0x48
        uint32_t *stack_top_ptr = (uint32_t *)(g_memoryGlobal + (unsigned int)g_parseStackSize * 0x48);
        memcpy(param_1, stack_top_ptr, 18 * sizeof(uint32_t));
    }
    return param_1;
}

// Function: push_parser_stack
void push_parser_stack(const uint32_t *data_to_push) {
    // Calculate pointer to the next available slot on the stack
    uint32_t *dest_ptr = (uint32_t *)(g_memoryGlobal + (unsigned int)g_parseStackSize * 0x48);
    memcpy(dest_ptr, data_to_push, 18 * sizeof(uint32_t));
    g_parseStackSize++; // Increment stack size
}

// Function: strbeg
// Checks if s1 begins with s2. Returns 0 if true, 1 if false.
uint32_t strbeg(const char *s1, const char *s2) {
    size_t len1 = strlen(s1);
    size_t len2 = strlen(s2);

    if (len2 == 0) {
        return 0; // An empty string is a prefix of any string.
    }
    if (len1 < len2) {
        return 1; // s1 cannot begin with s2 if s1 is shorter.
    }
    
    // Compare up to the length of s2
    if (strncmp(s1, s2, len2) == 0) {
        return 0; // s1 begins with s2.
    }
    return 1; // s1 does not begin with s2.
}

// Function: parse_get_symbol
// Parses the input string to identify the next symbol.
// Returns the symbol ID and updates consumed_len with the length of the parsed token.
uint32_t parse_get_symbol(const char *input_str, int *consumed_len) {
    int current_offset = 0;

    // Skip leading whitespace
    while (isspace((unsigned char)input_str[current_offset])) {
        current_offset++;
    }

    if (input_str[current_offset] == '\0') {
        *consumed_len = current_offset;
        return 0xd; // End-of-string symbol ID
    }

    int table_idx = 0;
    // Iterate through g_parseTable to find a matching symbol
    while (g_parseTable[table_idx].symbol_id != 0xc) { // 0xc is assumed to be a sentinel for the end of the table or 'unknown'
        if (strbeg(input_str + current_offset, g_parseTable[table_idx].symbol_str) == 0) {
            // Match found
            *consumed_len = current_offset + strlen(g_parseTable[table_idx].symbol_str);
            return g_parseTable[table_idx].symbol_id;
        }
        table_idx++;
    }

    // No match found in the parse table
    *consumed_len = current_offset;
    return 0xc; // Unknown symbol ID
}

// Function: is_symbol_close_paran
bool is_symbol_close_paran(int symbol_id) {
  return symbol_id == 1;
}

// Function: is_symbol_open_paran
bool is_symbol_open_paran(int symbol_id) {
  return symbol_id == 0;
}

// Function: is_symbol_result_operator (AND, OR)
bool is_symbol_result_operator(int symbol_id) {
  return (unsigned int)(symbol_id - 6) < 2; // Checks if symbol_id is 6 or 7
}

// Function: is_symbol_db_operator (==, !=, <, >)
bool is_symbol_db_operator(int symbol_id) {
  return (unsigned int)(symbol_id - 2) < 4; // Checks if symbol_id is 2, 3, 4, or 5
}

// Function: is_symbol_element (NAME, ADDRESS, EMAIL, BIRTHDATE)
bool is_symbol_element(int symbol_id) {
  return (unsigned int)(symbol_id - 8) < 4; // Checks if symbol_id is 8, 9, 10, or 11
}

// Function: is_symbol_birthdate
bool is_symbol_birthdate(int symbol_id) {
  return symbol_id == 0xb; // Checks if symbol_id is 11 (BIRTHDATE)
}

// Function: do_date_search
// Performs a search on database records based on a date and an operator.
// Pushes a result list onto the parser stack. Returns 0 on success, 1 on error.
int do_date_search(unsigned int operator_type, uint32_t date_val, uint16_t date_len) {
    byte result_count = 0;
    unsigned int record_idx = 0;
    unsigned int total_records = db_get_record_count();
    
    byte result_indices[64]; // Stores indices of matching records

    while (record_idx < total_records) {
        void *record_data = db_search_index(record_idx); 
        if (!record_data) {
            return 1; // Error getting record data
        }
        int compare_result = date_compare(record_data, date_val, date_len);

        bool match = false;
        if (operator_type == 5) { // Greater than (>)
            match = (compare_result == 1);
        } else if (operator_type == 4) { // Less than (<)
            match = (compare_result == -1);
        } else if (operator_type == 2) { // Equal (==)
            match = (compare_result == 0);
        } else if (operator_type == 3) { // Not equal (!=)
            match = (compare_result != 0);
        } else {
            return 1; // Error: Invalid operator type
        }

        if (match) {
            if (result_count < sizeof(result_indices)) {
                result_indices[result_count++] = record_idx;
            }
        }
        record_idx++;
    }

    // Prepare the 18-element stack item to push results
    uint32_t stack_item[18];
    stack_item[0] = 1; // Item Type: ResultList
    stack_item[1] = result_count; // Number of results
    for (int k = 0; k < result_count; ++k) {
        stack_item[k + 2] = result_indices[k];
    }
    // Fill remaining elements with zeros
    for (int k = result_count + 2; k < 18; ++k) {
        stack_item[k] = 0;
    }
    
    push_parser_stack(stack_item);
    return 0; // Success
}

// Function: do_string_search
// Performs a search on database records based on a string field and an operator.
// Pushes a result list onto the parser stack. Returns 0 on success, 1 on error.
int do_string_search(unsigned int element_type, unsigned int operator_type, const char *search_string) {
    byte result_count = 0;
    unsigned int record_idx = 0;
    unsigned int total_records = db_get_record_count();
    
    byte result_indices[64];

    while (record_idx < total_records) {
        void *record_data = db_search_index(record_idx);
        if (!record_data) {
            return 1; // Error getting record data
        }

        char *field_to_compare = NULL;
        // Determine which string field to compare based on element_type
        if (element_type == 10) { // Example: "email" field offset
            field_to_compare = (char *)record_data + 0x106;
        } else if (element_type == 8) { // Example: "name" field offset
            field_to_compare = (char *)record_data + 0x85;
        } else if (element_type == 9) { // Example: "address" field offset
            field_to_compare = (char *)record_data + 4;
        } else {
            return 1; // Error: Invalid element type
        }

        if (!field_to_compare) { // Should not happen if element_type is valid
            return 1;
        }

        int compare_result = strcmp(field_to_compare, search_string);

        bool match = false;
        if (operator_type == 5) { // Greater than (>)
            match = (compare_result == 1);
        } else if (operator_type == 4) { // Less than (<)
            match = (compare_result == -1);
        } else if (operator_type == 2) { // Equal (==)
            match = (compare_result == 0);
        } else if (operator_type == 3) { // Not equal (!=)
            match = (compare_result != 0);
        } else {
            return 1; // Error: Invalid operator type
        }

        if (match) {
            if (result_count < sizeof(result_indices)) {
                result_indices[result_count++] = record_idx;
            }
        }
        record_idx++;
    }

    // Prepare the 18-element stack item to push results
    uint32_t stack_item[18];
    stack_item[0] = 1; // Item Type: ResultList
    stack_item[1] = result_count; // Number of results
    for (int k = 0; k < result_count; ++k) {
        stack_item[k + 2] = result_indices[k];
    }
    // Fill remaining elements with zeros
    for (int k = result_count + 2; k < 18; ++k) {
        stack_item[k] = 0;
    }
    
    push_parser_stack(stack_item);
    return 0; // Success
}

// Function: do_result_search
// Combines two result lists (operands) using a logical operator (AND/OR).
// Pushes the new combined result list onto the parser stack. Returns 0 on success, 1 on error.
int do_result_search(int operator_type, const uint32_t *operand1_data, const uint32_t *operand2_data) {
    // operandX_data is an 18-element stack item: [0]=type, [1]=count, [2...]=indices
    byte operand1_count = operand1_data[1];
    byte operand2_count = operand2_data[1];

    uint32_t result_item_type = 1; // New result list will also be type 1
    byte result_count = 0;
    byte result_indices[64]; // Buffer for new result indices

    if (operator_type == 6) { // AND operation (intersection of results)
        for (int i = 0; i < operand1_count; ++i) {
            uint32_t val1 = operand1_data[i + 2]; // Actual index value from operand1
            for (int j = 0; j < operand2_count; ++j) {
                uint32_t val2 = operand2_data[j + 2]; // Actual index value from operand2
                if (val1 == val2) {
                    if (result_count < sizeof(result_indices)) {
                        result_indices[result_count++] = val1;
                    }
                    break; // Found a match for val1, move to next val1
                }
            }
        }
    } else if (operator_type == 7) { // OR operation (union of results)
        // First, add all unique elements from operand1_data
        for (int i = 0; i < operand1_count; ++i) {
            uint32_t val = operand1_data[i + 2];
            bool found = false;
            for (int k = 0; k < result_count; ++k) {
                if (result_indices[k] == val) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                if (result_count < sizeof(result_indices)) {
                    result_indices[result_count++] = val;
                }
            }
        }
        // Then, add unique elements from operand2_data that are not already in result_indices
        for (int i = 0; i < operand2_count; ++i) {
            uint32_t val = operand2_data[i + 2];
            bool found = false;
            for (int k = 0; k < result_count; ++k) {
                if (result_indices[k] == val) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                if (result_count < sizeof(result_indices)) {
                    result_indices[result_count++] = val;
                }
            }
        }
    } else {
        return 1; // Error: Invalid operator type
    }

    // Prepare the 18-element stack item to push results
    uint32_t stack_item[18];
    stack_item[0] = result_item_type; // Type 1: ResultList
    stack_item[1] = result_count; // Number of results
    for (int k = 0; k < result_count; ++k) {
        stack_item[k + 2] = result_indices[k];
    }
    // Fill remaining elements with zeros
    for (int k = result_count + 2; k < 18; ++k) {
        stack_item[k] = 0;
    }

    push_parser_stack(stack_item);
    return 0; // Success
}

// Function: do_reduce_stack
// Attempts to reduce the parser stack by combining [ResultList] [ResultOperator] [ResultList] patterns.
// Returns 0 on success (or no more reductions), 1 on error during reduction.
int do_reduce_stack(void) {
    while (true) {
        // Check for enough items on stack: [Operand1] [Operator] [Operand2]
        if (g_parseStackSize < 3) {
            return 0; // No more reductions possible with this pattern
        }

        // Peek at the top three items without popping
        uint32_t operand2_data[18];
        peek_parser_stack(operand2_data); // Top item (right operand)

        uint32_t operator_data_peek[18];
        pop_parser_stack(operator_data_peek); // Pop to get 2nd item
        peek_parser_stack(operator_data_peek); // Peek 2nd item (operator)
        push_parser_stack(operator_data_peek); // Push back to restore stack
        
        uint32_t operand1_data_peek[18];
        pop_parser_stack(operand1_data_peek); // Pop 2nd item
        pop_parser_stack(operand1_data_peek); // Pop 3rd item
        peek_parser_stack(operand1_data_peek); // Peek 3rd item (left operand)
        push_parser_stack(operand1_data_peek); // Push back 3rd item
        push_parser_stack(operator_data_peek); // Push back 2nd item

        // Check if the structure matches [ResultList] [ResultOperator] [ResultList]
        if (operand2_data[0] != 1 || // Not a result list type
            !is_symbol_result_operator(operator_data_peek[1]) || // Not a result operator
            operand1_data_peek[0] != 1) { // Not a result list type
            return 0; // Cannot reduce further with this pattern
        }

        // If conditions met, pop the three items and perform reduction
        pop_parser_stack(operand2_data); // Pop right operand
        uint32_t operator_item[18];
        pop_parser_stack(operator_item); // Pop operator
        uint32_t operator_type = operator_item[1]; // Get the actual operator value
        uint32_t operand1_item[18];
        pop_parser_stack(operand1_item); // Pop left operand

        // Perform the search operation for results
        if (do_result_search(operator_type, operand1_item, operand2_data) != 0) {
            return 1; // Error during result search
        }
        // The result of do_result_search is pushed onto the stack, so the loop continues
        // to check for further reductions.
    }
}

// Function: parse_search_expression
// Main function to parse a search expression string and execute the search.
// Returns 0 on success, 1 on error.
int parse_search_expression(char *param_1) {
    int parsing_done = 0; // Flag: 1 if parsing is done (end of expression or error)
    char *current_input_ptr = param_1; // Current position in the input string
    g_parseStackSize = 0; // Initialize parser stack

    while (parsing_done == 0) {
        int consumed_len = 0;
        uint32_t symbol_id = parse_get_symbol(current_input_ptr, &consumed_len);
        current_input_ptr += consumed_len; // Advance input string pointer

        // --- Handling different symbol types ---

        // 1. Database operator (e.g., ==, <, >)
        if (is_symbol_db_operator(symbol_id)) {
            // Expected stack state: [Element]
            // Pop the element from the stack
            uint32_t popped_item[18];
            if (g_parseStackSize == 0 || pop_parser_stack(popped_item) == NULL) {
                return 1; // Error: Stack underflow or pop failed
            }
            uint32_t element_type = popped_item[0]; // Type of element (should be 0)
            uint32_t element_value = popped_item[1]; // Value of element (e.g., 8 for name, 9 for address)

            // Validate popped item: Must be an element type [0, element_value]
            if (element_type != 0 || !is_symbol_element(element_value)) {
                return 1; // Error: Expected an element on stack
            }

            // If the element is a birthdate (0xb)
            if (is_symbol_birthdate(element_value)) {
                uint32_t date_val;
                uint16_t date_len;
                int date_parsed_len = parse_date(current_input_ptr, &date_val, &date_len);
                if (date_parsed_len == 0) {
                    return 1; // Error parsing date
                }
                current_input_ptr += date_parsed_len; // Advance input string pointer

                // Perform date search (symbol_id is the operator type)
                if (do_date_search(symbol_id, date_val, date_len) != 0) {
                    return 1; // Error during date search
                }
            } else { // Element is a string type (name, address, email)
                // Skip whitespace
                while (isspace((unsigned char)*current_input_ptr)) {
                    current_input_ptr++;
                }
                // Expect a quoted string literal
                if (*current_input_ptr != '\"') {
                    return 1; // Error: Expected quoted string
                }
                current_input_ptr++; // Skip opening quote

                // Extract the string literal
                char string_literal[128]; // Max 127 chars + null terminator
                unsigned int string_len = 0;
                while (*current_input_ptr != '\0' && *current_input_ptr != '\"' && string_len < sizeof(string_literal) - 1) {
                    string_literal[string_len++] = *current_input_ptr;
                    current_input_ptr++;
                }
                string_literal[string_len] = '\0'; // Null-terminate

                if (*current_input_ptr == '\0') { // Reached end of string without closing quote
                    return 1; // Error: Unclosed string literal
                }
                current_input_ptr++; // Skip closing quote

                // Perform string search (element_value is element type, symbol_id is operator type)
                if (do_string_search(element_value, symbol_id, string_literal) != 0) {
                    return 1; // Error during string search
                }
            }
        }
        // 2. Open parenthesis '('
        else if (is_symbol_open_paran(symbol_id)) {
            // An open parenthesis can be pushed if the stack is empty, or if the top is an operator.
            // If the top is a result list, it's an error.
            if (g_parseStackSize > 0) {
                uint32_t peeked_item[18];
                peek_parser_stack(peeked_item);
                if (peeked_item[0] == 1) { // If top is a ResultList, error
                     return 1;
                }
            }

            // Push the open parenthesis symbol onto the stack
            uint32_t open_paran_item[18] = {0, symbol_id}; // Type 0 (Operator/Element/Paren), Value: symbol_id (0 for open paren)
            push_parser_stack(open_paran_item);
        }
        // 3. Close parenthesis ')'
        else if (is_symbol_close_paran(symbol_id)) {
            // Expected stack state: [ResultList] [OpenParen]
            // Pop the result list
            uint32_t result_list_item[18];
            if (g_parseStackSize < 1 || pop_parser_stack(result_list_item) == NULL) {
                return 1; // Error: Stack underflow
            }
            if (result_list_item[0] != 1) { // Expected a result list
                return 1; // Error
            }
            
            // Pop the open parenthesis
            uint32_t open_paran_item[18];
            if (g_parseStackSize < 1 || pop_parser_stack(open_paran_item) == NULL) {
                return 1; // Error: Stack underflow
            }
            // Expected an open parenthesis [type 0, value 0]
            if (open_paran_item[0] != 0 || open_paran_item[1] != 0) { 
                return 1; // Error: Expected an open parenthesis
            }
            
            // Push the result list back onto the stack (effectively removing the parentheses)
            push_parser_stack(result_list_item);
        }
        // 4. Result operator (AND, OR)
        else if (is_symbol_result_operator(symbol_id)) {
            // Expected stack state: [ResultList]
            // Peek at the top item
            uint32_t peeked_item[18];
            peek_parser_stack(peeked_item);
            // If not a result list, error
            if (peeked_item[0] != 1) {
                return 1; // Error: Expected a result list before an operator
            }

            // Push the operator onto the stack
            uint32_t operator_item[18] = {0, symbol_id}; // Type 0 (Operator/Element/Paren), Value: symbol_id (6 or 7)
            push_parser_stack(operator_item);
        }
        // 5. Element (e.g., NAME, ADDRESS, EMAIL, BIRTHDATE)
        else if (is_symbol_element(symbol_id)) {
            // Expected stack state: [empty] or [OpenParen] or [ResultOperator]
            // If the top is a result list, it's an error.
            if (g_parseStackSize > 0) {
                uint32_t peeked_item[18];
                peek_parser_stack(peeked_item);
                if (peeked_item[0] == 1) { // Error if previous is a result list
                     return 1;
                }
            }

            // Push the element onto the stack
            uint32_t element_item[18] = {0, symbol_id}; // Type 0 (Operator/Element/Paren), Value: symbol_id (8,9,10,11)
            push_parser_stack(element_item);
        }
        // 6. End of input (0xd) or unknown symbol (0xc)
        else {
            if (symbol_id == 0xd) { // End of input
                parsing_done = 1; // Set flag to terminate loop
            } else if (symbol_id == 0xc) { // Unknown symbol
                return 1; // Error
            } else {
                return 1; // Unexpected symbol
            }
        }

        // After processing a symbol, attempt to reduce the stack
        if (do_reduce_stack() != 0) {
            return 1; // Error during stack reduction
        }
    } // End of while loop

    // After loop, if parsing successfully finished (parsing_done == 1)
    // Expected stack state: [ResultList] (and stack size should be 1)
    if (g_parseStackSize != 1) {
        return 1; // Error: Stack not in final expected state
    }

    uint32_t final_result_item[18];
    if (pop_parser_stack(final_result_item) == NULL) {
        return 1; // Error popping final result
    }
    uint32_t final_item_type = final_result_item[0]; // Should be 1 (ResultList)
    uint32_t final_result_count = final_result_item[1]; // Number of results

    if (final_item_type != 1) { // Expected a result list
        return 1; // Error
    }

    // Print search results
    printf("Search results, %u items found:\n", final_result_count);
    for (unsigned int i = 0; i < final_result_count; ++i) {
        // The result indices are stored from element 2 onwards in the stack item
        void *record_data = db_search_index(final_result_item[i + 2]);
        if (record_data) {
            print_record_helper(record_data);
        }
    }
    return 0; // Success
}