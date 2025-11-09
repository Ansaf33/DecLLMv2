#include <stdio.h>   // For printf
#include <stdlib.h>  // For malloc, realloc, free, strdup
#include <string.h>  // For strlen, strncpy, strdup
#include <ctype.h>   // For isspace, isalnum

// --- Helper function for dynamic array management ---
// This function appends a token to the dynamic array of tokens.
// It resizes the array if necessary.
// Parameters:
//   list_ptr: Pointer to the char** (the token list)
//   count_ptr: Pointer to the current number of tokens
//   capacity_ptr: Pointer to the current capacity of the token list
//   token: The string token to add (can be NULL for the list terminator)
// Returns 0 on success, -1 on failure (e.g., realloc failure).
static int tok_list_append(char*** list_ptr, int* count_ptr, int* capacity_ptr, char* token) {
    // If current count equals capacity, we need to resize
    if (*count_ptr == *capacity_ptr) {
        // Determine new capacity: start with 3, then double
        int new_capacity = (*capacity_ptr == 0) ? 3 : *capacity_ptr * 2;
        char** new_list = realloc(*list_ptr, new_capacity * sizeof(char*));
        if (new_list == NULL) {
            return -1; // Reallocation failed
        }
        *list_ptr = new_list;
        *capacity_ptr = new_capacity;
    }
    // Add the token and increment count
    (*list_ptr)[*count_ptr] = token;
    (*count_ptr)++;
    return 0; // Success
}

// Function: ok_for_sym
// Determines if a character is valid for a symbol (not '(', ')', or whitespace).
// Returns 1 if valid, 0 otherwise.
int ok_for_sym(char c) {
  // Use unsigned char with ctype functions to avoid issues with negative char values
  return (c != '(' && c != ')' && !isspace((unsigned char)c));
}

// Function: tokenize
// Tokenizes an input string into an array of dynamically allocated string tokens.
// The returned array is NULL-terminated.
// Returns char** on success, where each element is a dynamically allocated string.
// Returns NULL on any error (e.g., memory allocation failure).
char** tokenize(const char* input_string) {
    size_t len = strlen(input_string);
    size_t current_pos = 0; // Current position in the input string
    
    char** tokens = NULL;      // Pointer to the array of token strings
    int token_count = 0;     // Current number of tokens in the array
    int token_capacity = 0;  // Current allocated capacity of the array

    // Add the initial "START" token
    char* start_token_str = strdup("START"); // Duplicate "START" to manage its memory
    if (start_token_str == NULL) {
        return NULL; // strdup failed
    }
    if (tok_list_append(&tokens, &token_count, &token_capacity, start_token_str) < 0) {
        free(start_token_str); // Free the token string itself if append failed
        return NULL; // Realloc failed
    }

    // Process the input string character by character
    while (current_pos < len) {
        // 1. Skip leading whitespace characters
        while (current_pos < len && isspace((unsigned char)input_string[current_pos])) {
            current_pos++;
        }
        if (current_pos == len) { // If end of string reached after skipping spaces
            break; // Exit the main loop
        }

        size_t token_start_pos = current_pos; // Mark the beginning of the current token

        // 2. Handle single-character tokens: '(' or ')'
        if (input_string[current_pos] == '(' || input_string[current_pos] == ')') {
            char* current_token_str = (char*)malloc(2); // Allocate space for char + null terminator
            if (current_token_str == NULL) {
                // Cleanup on malloc failure
                for (int i = 0; i < token_count; ++i) {
                    free(tokens[i]);
                }
                free(tokens);
                return NULL;
            }
            current_token_str[0] = input_string[current_pos];
            current_token_str[1] = '\0'; // Null-terminate the token string
            
            if (tok_list_append(&tokens, &token_count, &token_capacity, current_token_str) < 0) {
                free(current_token_str); // Free the token string itself
                // Cleanup on append (realloc) failure
                for (int i = 0; i < token_count; ++i) {
                    free(tokens[i]);
                }
                free(tokens);
                return NULL;
            }
            current_pos++; // Move past the processed character
        } 
        // 3. Handle multi-character tokens (alphanumeric sequences)
        else {
            // If the character is not alphanumeric, it's an unexpected character
            // (since spaces and parentheses are handled, this implies an error)
            if (!isalnum((unsigned char)input_string[current_pos])) {
                // Cleanup: free all previously allocated tokens and the token list
                for (int i = 0; i < token_count; ++i) {
                    free(tokens[i]);
                }
                free(tokens);
                return NULL;
            }

            // Advance current_pos until a non-alphanumeric character or end of string is found
            while (current_pos < len && isalnum((unsigned char)input_string[current_pos])) {
                current_pos++;
            }

            // This check ensures a valid token was found (current_pos must have advanced)
            if (current_pos <= token_start_pos) {
                for (int i = 0; i < token_count; ++i) {
                    free(tokens[i]);
                }
                free(tokens);
                return NULL;
            }

            // Allocate memory for the token substring
            size_t token_len = current_pos - token_start_pos;
            char* current_token_str = (char*)malloc(token_len + 1); // +1 for null terminator
            if (current_token_str == NULL) {
                // Cleanup on malloc failure
                for (int i = 0; i < token_count; ++i) {
                    free(tokens[i]);
                }
                free(tokens);
                return NULL;
            }
            // Copy the token characters and null-terminate
            strncpy(current_token_str, input_string + token_start_pos, token_len);
            current_token_str[token_len] = '\0'; 

            if (tok_list_append(&tokens, &token_count, &token_capacity, current_token_str) < 0) {
                free(current_token_str); // Free the token string itself
                // Cleanup on append (realloc) failure
                for (int i = 0; i < token_count; ++i) {
                    free(tokens[i]);
                }
                free(tokens);
                return NULL;
            }
        }
    }

    // Add a NULL pointer at the end of the tokens array to mark its end.
    // This allows easy iteration (e.g., `for (i=0; tokens[i] != NULL; ++i)`).
    if (tok_list_append(&tokens, &token_count, &token_capacity, NULL) < 0) {
        // This case would mean realloc failed for the NULL terminator slot.
        // Free all previously successful tokens and the array.
        for (int i = 0; i < token_count; ++i) {
            free(tokens[i]);
        }
        free(tokens);
        return NULL;
    }

    return tokens; // Return the successfully created token list
}

// Main function for demonstration and testing
int main() {
    // Test cases
    const char* test_strings[] = {
        " ( def main ( print \"hello\" ) ) ",
        "  (add 10 20) ",
        "  (  func  (x y) (+ x y) )  ",
        "  just_a_symbol  ",
        "()",
        " (a(b)c) ",
        "", // Empty string
        " (123 + 456) ", // Will fail on '+' if not alphanumeric
        NULL // Sentinel for end of test cases
    };

    for (int j = 0; test_strings[j] != NULL; ++j) {
        const char* input = test_strings[j];
        printf("Tokenizing: \"%s\"\n", input);
        char** tokens = tokenize(input);

        if (tokens == NULL) {
            printf("  Error: Tokenization failed.\n");
        } else {
            printf("  Tokens:\n");
            for (int i = 0; tokens[i] != NULL; ++i) {
                printf("    [%d]: \"%s\"\n", i, tokens[i]);
            }
            // Free allocated memory for tokens
            // Remember to free each token string first, then the array itself
            for (int i = 0; tokens[i] != NULL; ++i) {
                free(tokens[i]);
            }
            free(tokens); // Free the array of char* pointers
        }
        printf("\n");
    }

    // Test ok_for_sym
    printf("Testing ok_for_sym:\n");
    printf("  'a': %d\n", ok_for_sym('a'));
    printf("  '(': %d\n", ok_for_sym('('));
    printf("  ')': %d\n", ok_for_sym(')'));
    printf("  ' ': %d\n", ok_for_sym(' '));
    printf("  '\\t': %d\n", ok_for_sym('\t'));
    printf("  'Z': %d\n", ok_for_sym('Z'));
    printf("  '9': %d\n", ok_for_sym('9'));

    return 0;
}