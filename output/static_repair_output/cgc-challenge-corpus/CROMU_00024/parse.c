#include <stdlib.h> // For calloc, free
#include <stddef.h> // For size_t
#include <stdio.h>  // For printf, NULL

// Function: parse_command
// Parses a string into tokens based on a separator character.
// Modifies the input string (null-terminates tokens).
// param_1: The string to parse. This string will be modified.
// param_2: The separator character.
// param_3: A pointer to a char*. On success, it will point to a newly allocated array of char*
//          where each element points to the start of a token within param_1.
// Returns: The number of tokens found, or (size_t)-1 if an error occurred (e.g., empty string or allocation failure).
size_t parse_command(char *param_1, char param_2, char **param_3) {
  size_t token_count;
  char *current_char_ptr;
  size_t i; // Loop counter for populating array

  // Handle empty input string case as per original logic
  if (*param_1 == '\0') {
    return (size_t)-1;
  }

  // First pass: count tokens
  // The original logic counts tokens by initializing to 1 and incrementing for each separator found
  // *after* the initial character. This implies that a string like "a,b,c" yields 3 tokens,
  // and "a," yields 2 tokens ("a", "").
  // It also implies that ",a,b" will yield 2 tokens (missing the initial empty token).
  // This behavior is preserved from the original snippet.
  token_count = 1;
  current_char_ptr = param_1;
  while (*current_char_ptr != '\0') {
    current_char_ptr++; // Advance to the next character
    // Check if the character we just advanced to is a separator and not the end of the string
    if (*current_char_ptr != '\0' && param_2 == *current_char_ptr) {
      token_count++;
    }
  }

  // Allocate memory for the array of char pointers
  // Each element in the array will be a 'char *'
  *param_3 = (char *)calloc(token_count, sizeof(char *));
  if (*param_3 == NULL) { // Check if memory allocation failed
    return (size_t)-1; // Indicate error
  }

  // Second pass: populate the array with pointers to token starts and null-terminate tokens
  current_char_ptr = param_1; // Reset iterator to the beginning of the string
  for (i = 0; i < token_count; i++) {
    // Store the start of the current token in the allocated array
    // (*param_3) is a char*, but we treat it as a char** to index it correctly.
    ((char **)*param_3)[i] = current_char_ptr;

    // Find the end of the current token: either a separator or the null terminator
    while (*current_char_ptr != '\0' && *current_char_ptr != param_2) {
      current_char_ptr++;
    }

    // If a separator was found (i.e., not the end of the string)
    if (*current_char_ptr != '\0') {
      *current_char_ptr = '\0'; // Null-terminate the current token
      current_char_ptr++;       // Move past the null terminator to the start of the next token
    }
  }

  return token_count;
}

// Main function for testing the parse_command function
int main() {
    // Test cases - note that param_1 strings are modified in place
    // so copies are used for each test.
    char test_str1[] = "hello,world,this,is,a,test";
    char test_str2[] = "one_token";
    char test_str3[] = ",leading,separator"; // Expected 2 tokens by current logic: {"leading", "separator"}
                                             // The initial empty token before ',' is not counted.
    char test_str4[] = "empty";
    char test_str5[] = ""; // Empty string
    char test_str6[] = "a,,b"; // Expected 3 tokens: {"a", "", "b"}
    char test_str7[] = "trailing,"; // Expected 2 tokens: {"trailing", ""}

    char **tokens = NULL;
    size_t count;
    size_t i; // Loop counter for printing tokens

    printf("--- Test Case 1: Standard string ---\n");
    printf("Input: \"%s\", Separator: ','\n", test_str1);
    count = parse_command(test_str1, ',', &tokens);
    if (count == (size_t)-1) {
        printf("Error parsing command or empty input.\n");
    } else {
        printf("Parsed %zu tokens:\n", count);
        for (i = 0; i < count; i++) {
            printf("  Token %zu: \"%s\"\n", i, tokens[i]);
        }
        free(tokens); // Free the allocated array of char pointers
        tokens = NULL;
    }
    printf("\n");

    printf("--- Test Case 2: Single token, different separator ---\n");
    printf("Input: \"%s\", Separator: '_'\n", test_str2);
    count = parse_command(test_str2, '_', &tokens);
    if (count == (size_t)-1) {
        printf("Error parsing command or empty input.\n");
    } else {
        printf("Parsed %zu tokens:\n", count);
        for (i = 0; i < count; i++) {
            printf("  Token %zu: \"%s\"\n", i, tokens[i]);
        }
        free(tokens);
        tokens = NULL;
    }
    printf("\n");

    printf("--- Test Case 3: Leading separator (behavior preserved from original) ---\n");
    printf("Input: \"%s\", Separator: ','\n", test_str3);
    count = parse_command(test_str3, ',', &tokens);
    if (count == (size_t)-1) {
        printf("Error parsing command or empty input.\n");
    } else {
        printf("Parsed %zu tokens:\n", count);
        for (i = 0; i < count; i++) {
            printf("  Token %zu: \"%s\"\n", i, tokens[i]);
        }
        free(tokens);
        tokens = NULL;
    }
    printf("\n");

    printf("--- Test Case 4: No separator ---\n");
    printf("Input: \"%s\", Separator: ','\n", test_str4);
    count = parse_command(test_str4, ',', &tokens);
    if (count == (size_t)-1) {
        printf("Error parsing command or empty input.\n");
    } else {
        printf("Parsed %zu tokens:\n", count);
        for (i = 0; i < count; i++) {
            printf("  Token %zu: \"%s\"\n", i, tokens[i]);
        }
        free(tokens);
        tokens = NULL;
    }
    printf("\n");

    printf("--- Test Case 5: Empty input string ---\n");
    printf("Input: \"%s\", Separator: ','\n", test_str5);
    count = parse_command(test_str5, ',', &tokens);
    if (count == (size_t)-1) {
        printf("Correctly returned -1 for empty input.\n");
    } else {
        printf("Unexpectedly parsed %zu tokens for empty input.\n", count);
        free(tokens);
        tokens = NULL;
    }
    printf("\n");

    printf("--- Test Case 6: Empty middle token ---\n");
    printf("Input: \"%s\", Separator: ','\n", test_str6);
    count = parse_command(test_str6, ',', &tokens);
    if (count == (size_t)-1) {
        printf("Error parsing command or empty input.\n");
    } else {
        printf("Parsed %zu tokens:\n", count);
        for (i = 0; i < count; i++) {
            printf("  Token %zu: \"%s\"\n", i, tokens[i]);
        }
        free(tokens);
        tokens = NULL;
    }
    printf("\n");

    printf("--- Test Case 7: Trailing separator ---\n");
    printf("Input: \"%s\", Separator: ','\n", test_str7);
    count = parse_command(test_str7, ',', &tokens);
    if (count == (size_t)-1) {
        printf("Error parsing command or empty input.\n");
    } else {
        printf("Parsed %zu tokens:\n", count);
        for (i = 0; i < count; i++) {
            printf("  Token %zu: \"%s\"\n", i, tokens[i]);
        }
        free(tokens);
        tokens = NULL;
    }
    printf("\n");

    return 0;
}