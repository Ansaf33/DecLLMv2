#include <stdio.h>   // For printf in main
#include <ctype.h>   // For islower, isdigit
#include <string.h>  // For memset, strncpy
#include <stdlib.h>  // For strtol

// Define undefined4 as unsigned int, as commonly seen in decompiled code.
// Each token will occupy two undefined4 slots: [type, value].
typedef unsigned int undefined4;

// Function: parse_variable
// Parses a variable name (starts with '_' or lowercase, followed by lowercase).
// Stores token type 1 and the variable name (packed into undefined4) in param_2.
// Returns the length of the parsed variable name, or -1 on error.
int parse_variable(char *param_1, undefined4 *param_2) {
  char *end_var_name = param_1;

  if (*param_1 == '_') {
    end_var_name++;
  } else {
    while (islower((int)*end_var_name)) {
      end_var_name++;
    }
  }

  int length = end_var_name - param_1;

  // Variable names must be between 1 and 4 characters long (inclusive).
  if (length == 0 || length > 4) {
    return -1;
  } else {
    param_2[0] = 1; // Token type: Variable

    // Store the variable name in the value part of the token (param_2[1]).
    // This packs up to 4 characters into an unsigned int.
    memset(&param_2[1], 0, sizeof(undefined4));
    strncpy((char *)&param_2[1], param_1, length);
    return length;
  }
}

// Function: parse_constant
// Parses an integer constant using strtol.
// Stores token type 0 and the integer value in param_2.
// Returns the length of the parsed constant string, or -1 on error.
int parse_constant(char *param_1, undefined4 *param_2) {
  char *endptr;
  long val = strtol(param_1, &endptr, 10);
  int length = endptr - param_1;

  // If no digits were parsed, it's an error.
  if (length == 0) {
    return -1;
  }
  
  param_2[0] = 0; // Token type: Constant
  param_2[1] = (undefined4)val; // Store the integer value
  return length;
}

// Function: tokenize
// Tokenizes an input string into a sequence of tokens.
// Each token consists of a type and a value (stored as two undefined4s).
// Returns the number of tokens parsed, or -1 on error.
int tokenize(char *param_1, undefined4 *param_2) {
  undefined4 *token_out_ptr = param_2;
  char current_char;
  int len;

  while (1) { // Main tokenization loop
    current_char = *param_1;

    // Check for end of string
    if (current_char == '\0') {
      return (int)(token_out_ptr - param_2) / 2; // Return number of tokens
    }

    // Attempt to parse a variable
    if (current_char == '_' || islower((int)current_char)) {
      len = parse_variable(param_1, token_out_ptr);
      if (len < 0) {
        return len; // Error during variable parsing
      }
      token_out_ptr += 2; // Advance token output pointer (type + value)
      param_1 += len;     // Advance input string pointer by parsed length
      continue;           // Continue to the next character in the input string
    }

    // Attempt to parse a constant
    if (isdigit((int)current_char)) {
      len = parse_constant(param_1, token_out_ptr);
      if (len < 0) {
        return len; // Error during constant parsing
      }
      token_out_ptr += 2; // Advance token output pointer
      param_1 += len;     // Advance input string pointer
      continue;           // Continue to the next character in the input string
    }

    // Handle single-character tokens or whitespace
    int token_type = -1; // Default: no token generated (e.g., space) or error

    if (current_char == ' ') {
      // Skip space, no token generated. param_1 will advance by 1 below.
    } else if (current_char == '~') {
      token_type = 7;
    } else {
      switch (current_char) {
        case '$': token_type = 9; break;
        case '&': token_type = 8; break;
        case '(': token_type = 10; break;
        case ')': token_type = 11; break;
        case '*': token_type = 5; break;
        case '+': token_type = 3; break;
        case '-': token_type = 4; break;
        case '/': token_type = 6; break;
        case '=': token_type = 2; break;
        default:
          return -1; // Unknown character, return error
      }
    }

    // If a single-character token was identified, store it
    if (token_type != -1) {
      token_out_ptr[0] = token_type;
      token_out_ptr[1] = 0; // Single-char tokens usually don't have an associated value
      token_out_ptr += 2;   // Advance token output pointer
    }
    
    param_1++; // Advance input string pointer for single characters (including space)
  }
}

// Main function for demonstration and testing
int main() {
    char input_string1[] = "var1 = 123 + _abc / $ ( ) ~ &";
    char input_string2[] = "invalid_variable = 5"; // _variable name too long
    char input_string3[] = "var = badchar"; // Invalid character
    char input_string4[] = "test"; // Valid variable
    char input_string5[] = "12345"; // Valid constant
    char input_string6[] = " _a + 5"; // Leading space, valid variable

    // Allocate space for tokens (e.g., 50 tokens, each 2 undefined4s)
    undefined4 tokens[100]; // Max 50 tokens

    printf("Tokenizing: \"%s\"\n", input_string1);
    int num_tokens = tokenize(input_string1, tokens);
    if (num_tokens < 0) {
        printf("Error tokenizing string 1: %d\n", num_tokens);
    } else {
        printf("Tokens found: %d\n", num_tokens);
        for (int i = 0; i < num_tokens; ++i) {
            int type = tokens[i * 2];
            undefined4 value = tokens[i * 2 + 1];
            printf("  Token %d: Type = %d, Value = ", i, type);
            if (type == 1) { // Variable
                // Print the packed string. This relies on endianness.
                char var_name[5];
                memcpy(var_name, &value, 4);
                var_name[4] = '\0';
                printf("'%s'\n", var_name);
            } else if (type == 0) { // Constant
                printf("%u\n", value);
            } else { // Operator/Symbol
                printf("%u\n", value); // Value is usually 0 for these
            }
        }
    }
    printf("\n");

    printf("Tokenizing: \"%s\"\n", input_string2);
    num_tokens = tokenize(input_string2, tokens);
    if (num_tokens < 0) {
        printf("Error tokenizing string 2: %d (Expected -1 for variable length)\n", num_tokens);
    } else {
        printf("Tokens found: %d\n", num_tokens);
    }
    printf("\n");

    printf("Tokenizing: \"%s\"\n", input_string3);
    num_tokens = tokenize(input_string3, tokens);
    if (num_tokens < 0) {
        printf("Error tokenizing string 3: %d (Expected -1 for invalid char)\n", num_tokens);
    } else {
        printf("Tokens found: %d\n", num_tokens);
    }
    printf("\n");
    
    printf("Tokenizing: \"%s\"\n", input_string4);
    num_tokens = tokenize(input_string4, tokens);
    if (num_tokens < 0) {
        printf("Error tokenizing string 4: %d\n", num_tokens);
    } else {
        printf("Tokens found: %d\n", num_tokens);
        for (int i = 0; i < num_tokens; ++i) {
            int type = tokens[i * 2];
            undefined4 value = tokens[i * 2 + 1];
            printf("  Token %d: Type = %d, Value = ", i, type);
            if (type == 1) { // Variable
                char var_name[5];
                memcpy(var_name, &value, 4);
                var_name[4] = '\0';
                printf("'%s'\n", var_name);
            } else {
                printf("%u\n", value);
            }
        }
    }
    printf("\n");

    printf("Tokenizing: \"%s\"\n", input_string5);
    num_tokens = tokenize(input_string5, tokens);
    if (num_tokens < 0) {
        printf("Error tokenizing string 5: %d\n", num_tokens);
    } else {
        printf("Tokens found: %d\n", num_tokens);
        for (int i = 0; i < num_tokens; ++i) {
            int type = tokens[i * 2];
            undefined4 value = tokens[i * 2 + 1];
            printf("  Token %d: Type = %d, Value = ", i, type);
            if (type == 0) { // Constant
                printf("%u\n", value);
            } else {
                printf("%u\n", value);
            }
        }
    }
    printf("\n");

    printf("Tokenizing: \"%s\"\n", input_string6);
    num_tokens = tokenize(input_string6, tokens);
    if (num_tokens < 0) {
        printf("Error tokenizing string 6: %d\n", num_tokens);
    } else {
        printf("Tokens found: %d\n", num_tokens);
        for (int i = 0; i < num_tokens; ++i) {
            int type = tokens[i * 2];
            undefined4 value = tokens[i * 2 + 1];
            printf("  Token %d: Type = %d, Value = ", i, type);
            if (type == 1) { // Variable
                char var_name[5];
                memcpy(var_name, &value, 4);
                var_name[4] = '\0';
                printf("'%s'\n", var_name);
            } else if (type == 0) { // Constant
                printf("%u\n", value);
            } else { // Operator/Symbol
                printf("%u\n", value);
            }
        }
    }
    printf("\n");


    return 0;
}