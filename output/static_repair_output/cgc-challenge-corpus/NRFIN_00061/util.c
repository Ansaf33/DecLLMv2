#include <stdio.h>   // For fprintf, NULL
#include <stdlib.h>  // For malloc, free, exit, atoi, size_t
#include <string.h>  // For strlen, strtok_r, memcpy, memset, strcmp

// Define the structure for attributes
struct Attribute {
    char *key;
    char *value;
    struct Attribute *next;
};

// Global variable for the head of the attribute linked list
struct Attribute *attributes = NULL;

// Global stubs for external functions
void _terminate(void) {
    fprintf(stderr, "Program terminated due to an error.\n");
    exit(EXIT_FAILURE);
}

// transmit_all is assumed to take the buffer and its size.
// The original call `transmit_all()` without arguments is fixed here to pass `buffer` and `response_len`.
int transmit_all(const char* buffer, size_t size) {
    // This is a stub implementation. In a real system, this would send data over a network, etc.
    // For compilation and basic functionality, we just print to stderr.
    fprintf(stderr, "TRANSMIT: %.*s (size: %zu)\n", (int)size, buffer, size);
    return 0; // 0 for success, non-zero for error
}

// Global string literals (inferred from usage in original code)
const char *DELIMITER_EQUALS = "=";
const char *DELIMITER_COMMA = ",";

// Function: clearAttributes
void clearAttributes(void) {
  struct Attribute *current = attributes;
  while (current != NULL) {
    struct Attribute *next_attr = current->next;
    free(current->key);
    free(current->value);
    free(current);
    current = next_attr;
  }
  attributes = NULL;
}

// Function: initializeAttributes
void initializeAttributes(char *param_1) {
  clearAttributes();

  char *saveptr1;
  // First, tokenize the input string by commas to get individual key=value pairs
  char *pair_token = strtok_r(param_1, (char *)DELIMITER_COMMA, &saveptr1);

  while (pair_token != NULL) {
    char *saveptr2;
    // For each pair_token, tokenize by equals to separate key and value
    char *key_token = strtok_r(pair_token, (char *)DELIMITER_EQUALS, &saveptr2);
    char *value_token = strtok_r(NULL, (char *)DELIMITER_EQUALS, &saveptr2);

    if (key_token == NULL || value_token == NULL) {
      // Handle malformed pairs (e.g., "key" or "key=")
      fprintf(stderr, "Warning: Malformed attribute pair encountered: '%s'. Skipping.\n", pair_token);
      // Move to the next pair
      pair_token = strtok_r(NULL, (char *)DELIMITER_COMMA, &saveptr1);
      continue;
    }

    struct Attribute *new_attr = (struct Attribute *)malloc(sizeof(struct Attribute));
    if (new_attr == NULL) {
      _terminate(); // Out of memory
    }
    memset(new_attr, 0, sizeof(struct Attribute)); // Initialize allocated memory to zero

    // Allocate and copy key string
    size_t key_len = strlen(key_token);
    new_attr->key = (char *)malloc(key_len + 1);
    if (new_attr->key == NULL) {
      free(new_attr); // Clean up partially allocated struct
      _terminate();
    }
    memcpy(new_attr->key, key_token, key_len + 1); // Copy key including null terminator

    // Allocate and copy value string
    size_t value_len = strlen(value_token);
    new_attr->value = (char *)malloc(value_len + 1);
    if (new_attr->value == NULL) {
      free(new_attr->key); // Clean up key
      free(new_attr);      // Clean up struct
      _terminate();
    }
    memcpy(new_attr->value, value_token, value_len + 1); // Copy value including null terminator

    // Add the new attribute to the beginning of the linked list
    new_attr->next = attributes;
    attributes = new_attr;

    // Get the next key=value pair
    pair_token = strtok_r(NULL, (char *)DELIMITER_COMMA, &saveptr1);
  }
}

// Function: sendErrorResponse
void sendErrorResponse(char *param_1) {
  size_t param_len = strlen(param_1);
  // Allocate buffer for "Response=%s?" and null terminator.
  // "Response=" is 9 characters, "?" is 1 character, null terminator is 1 character.
  // Total 11 characters + param_len. The original `strlen(param_1) + 10` is sufficient.
  char *buffer = (char *)malloc(param_len + 11);
  if (buffer == NULL) {
    _terminate(); // Out of memory
  }
  memset(buffer, 0, param_len + 11); // Initialize buffer to zeros

  // Format the error message into the buffer
  // The original format string `"!X=!X?"` is ambiguous; `"%s=%s?"` is a common interpretation.
  // Given the arguments ("Response", param_1), this seems to be the intent.
  sprintf(buffer, "Response=%s?", param_1);

  size_t response_len = strlen(buffer);
  int transmit_status = transmit_all(buffer, response_len);
  if (transmit_status != 0) {
    _terminate(); // Transmission failed
  }
  free(buffer);
}

// Function: getStringAttribute
void getStringAttribute(char **param_1, char *param_2) {
  struct Attribute *current = attributes;
  while (current != NULL) {
    // Search for an exact match of the key.
    // The original `strncmp` logic with `MAX(len1, len2)` was problematic; `strcmp` is correct for exact match.
    if (strcmp(current->key, param_2) == 0) {
      *param_1 = current->value; // Found, return the value
      return;
    }
    current = current->next;
  }
  *param_1 = NULL; // Key not found
}

// Function: getIntegerAttribute
void getIntegerAttribute(int *param_1, char *param_2) { // param_2 type corrected to char*
  char *value_str = NULL;
  getStringAttribute(&value_str, param_2);
  if (value_str == NULL) {
    *param_1 = 0; // Attribute not found, return default integer value 0
  } else {
    *param_1 = atoi(value_str); // Convert found string value to integer
  }
}