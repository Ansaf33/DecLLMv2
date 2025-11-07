#include <stdio.h>   // For printf
#include <stdlib.h>  // For malloc, realloc, free
#include <string.h>  // For strlen, strncpy, strdup
#include <ctype.h>   // For isspace, isalnum

// Function: ok_for_sym
// Returns 1 if 'c' is a valid character for a symbol, 0 otherwise.
// A character is valid if it's not '(', not ')', and not a whitespace character.
int ok_for_sym(char c) {
  return (c != '(' && c != ')' && !isspace((unsigned char)c));
}

// Structure to hold the token list
typedef struct {
    char** tokens;
    int count;
    int capacity;
} TokenList;

// Function to initialize a TokenList
TokenList* create_token_list() {
    TokenList* list = (TokenList*)malloc(sizeof(TokenList));
    if (list == NULL) return NULL;
    
    list->capacity = 4; // Initial capacity
    list->tokens = (char**)malloc(sizeof(char*) * list->capacity);
    if (list->tokens == NULL) {
        free(list);
        return NULL;
    }
    list->count = 0;
    return list;
}

// Function to append a token to the TokenList
// Returns 0 on success, -1 on failure.
// If an error occurs, the 'token' pointer is freed.
int tok_list_append(TokenList* list, char* token) {
    if (list == NULL || token == NULL) {
        free(token); // Free the token if list or token is invalid
        return -1;
    }

    if (list->count >= list->capacity) {
        int new_capacity = list->capacity * 2;
        char** new_tokens = (char**)realloc(list->tokens, sizeof(char*) * new_capacity);
        if (new_tokens == NULL) {
            free(token); // Free the token if reallocation fails
            return -1; // Reallocation failed
        }
        list->tokens = new_tokens;
        list->capacity = new_capacity;
    }
    list->tokens[list->count++] = token;
    return 0; // Success
}

// Function to free a TokenList and its contained tokens
void free_token_list(TokenList* list) {
    if (list == NULL) return;
    for (int i = 0; i < list->count; ++i) {
        free(list->tokens[i]); // Free individual tokens
    }
    free(list->tokens); // Free the array of token pointers
    free(list);         // Free the TokenList struct itself
}

// Function: tokenize
// Tokenizes the input string into a list of dynamically allocated strings.
// Returns a pointer to a TokenList on success, or NULL on failure.
TokenList* tokenize(const char* input_string) {
    size_t input_len = strlen(input_string);
    size_t current_pos = 0;
    
    TokenList* list = create_token_list();
    if (list == NULL) {
        return NULL;
    }

    // Add "START" token
    // strdup allocates memory, so it needs to be freed later
    if (tok_list_append(list, strdup("START")) < 0) { 
        free_token_list(list);
        return NULL;
    }

    // Main tokenization loop
    while (current_pos < input_len) {
        // Skip whitespace
        while (current_pos < input_len && isspace((unsigned char)input_string[current_pos])) {
            current_pos++;
        }

        // If end of string after skipping spaces, break
        if (current_pos >= input_len) {
            break;
        }

        char* token = NULL;
        int append_status;

        // Handle parentheses as single-character tokens
        if (input_string[current_pos] == '(' || input_string[current_pos] == ')') {
            token = (char*)malloc(2);
            if (token == NULL) {
                free_token_list(list);
                return NULL;
            }
            token[0] = input_string[current_pos];
            token[1] = '\0';
            current_pos++; // Move past the parenthesis
        } else {
            // Assume it's an alphanumeric token (or error if not)
            size_t token_start_pos = current_pos;

            // Check if the current character is valid to start a symbol/alphanumeric token
            if (!isalnum((unsigned char)input_string[current_pos])) {
                // If it's not a parenthesis, space, or alphanumeric, it's an unrecognized character
                free_token_list(list);
                return NULL;
            }

            // Find the end of the alphanumeric token
            while (current_pos < input_len && isalnum((unsigned char)input_string[current_pos])) {
                current_pos++;
            }
            
            size_t token_len = current_pos - token_start_pos;
            token = (char*)malloc(token_len + 1);
            if (token == NULL) {
                free_token_list(list);
                return NULL;
            }
            strncpy(token, input_string + token_start_pos, token_len);
            token[token_len] = '\0';
        }
        
        // Append the token to the list
        append_status = tok_list_append(list, token);
        if (append_status < 0) {
            // tok_list_append already freed the 'token' on failure.
            // Now free the list itself.
            free_token_list(list);
            return NULL;
        }
    }
    
    return list; // Return the successfully created token list
}

// Main function for testing
int main() {
    const char* test_string1 = "(hello world) (123)";
    const char* test_string2 = " ( ) ";
    const char* test_string3 = "  a(b c)d ";
    const char* test_string4 = "invalid@char";
    const char* test_string5 = "";
    const char* test_string6 = "just_a_symbol";

    TokenList* tokens = NULL;

    printf("Tokenizing: \"%s\"\n", test_string1);
    tokens = tokenize(test_string1);
    if (tokens) {
        for (int i = 0; i < tokens->count; ++i) {
            printf("  Token %d: \"%s\"\n", i, tokens->tokens[i]);
        }
        free_token_list(tokens);
    } else {
        printf("  Tokenization failed.\n");
    }
    printf("\n");

    printf("Tokenizing: \"%s\"\n", test_string2);
    tokens = tokenize(test_string2);
    if (tokens) {
        for (int i = 0; i < tokens->count; ++i) {
            printf("  Token %d: \"%s\"\n", i, tokens->tokens[i]);
        }
        free_token_list(tokens);
    } else {
        printf("  Tokenization failed.\n");
    }
    printf("\n");

    printf("Tokenizing: \"%s\"\n", test_string3);
    tokens = tokenize(test_string3);
    if (tokens) {
        for (int i = 0; i < tokens->count; ++i) {
            printf("  Token %d: \"%s\"\n", i, tokens->tokens[i]);
        }
        free_token_list(tokens);
    } else {
        printf("  Tokenization failed.\n");
    }
    printf("\n");

    printf("Tokenizing: \"%s\"\n", test_string4);
    tokens = tokenize(test_string4);
    if (tokens) {
        for (int i = 0; i < tokens->count; ++i) {
            printf("  Token %d: \"%s\"\n", i, tokens->tokens[i]);
        }
        free_token_list(tokens);
    } else {
        printf("  Tokenization failed (as expected for invalid char).\n");
    }
    printf("\n");

    printf("Tokenizing: \"%s\"\n", test_string5);
    tokens = tokenize(test_string5);
    if (tokens) {
        for (int i = 0; i < tokens->count; ++i) {
            printf("  Token %d: \"%s\"\n", i, tokens->tokens[i]);
        }
        free_token_list(tokens);
    } else {
        printf("  Tokenization failed.\n"); // Empty string should still produce "START" token
    }
    printf("\n");
    
    printf("Tokenizing: \"%s\"\n", test_string6);
    tokens = tokenize(test_string6);
    if (tokens) {
        for (int i = 0; i < tokens->count; ++i) {
            printf("  Token %d: \"%s\"\n", i, tokens->tokens[i]);
        }
        free_token_list(tokens);
    } else {
        printf("  Tokenization failed.\n");
    }
    printf("\n");

    return 0;
}