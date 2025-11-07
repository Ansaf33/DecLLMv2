#include <stdlib.h> // For malloc, free
#include <string.h> // For strlen, memset, memcpy
#include <stddef.h> // For size_t, NULL
#include <stdint.h> // For uint32_t, intptr_t
#include <ctype.h>  // For isalnum, isdigit, isalpha, isspace

// Dummy implementations for custom memory functions.
// In a real scenario, these would link to a specific library.
// The `flags` and `line_info` parameters are ignored for standard malloc/free behavior.
// The original `allocate` returns 0 on success, non-zero on failure.
int allocate(size_t size, int flags, void **out_ptr, uint32_t line_info) {
  (void)flags;     // Suppress unused parameter warning
  (void)line_info; // Suppress unused parameter warning
  *out_ptr = malloc(size);
  return (*out_ptr == NULL); // Return 0 on success, non-zero on failure
}

// The original `deallocate` takes a size argument which is ignored by `free`.
void deallocate(void *ptr, size_t size) {
  (void)size; // Suppress unused parameter warning
  free(ptr);
}

// Function: freeString
void freeString(int *param_1) {
  if (param_1 != NULL) {
    // *param_1 holds the address of the actual string data
    if (*param_1 != 0) {
      // param_1[1] holds the size of the allocated string data buffer
      deallocate((void *)(intptr_t)*param_1, (size_t)param_1[1]);
    }
    // Deallocate the structure itself (the int array).
    // Original used 0xc (12 bytes), implying 3 ints.
    deallocate(param_1, 3 * sizeof(int));
  }
}

// Function: skipUrl
int skipUrl(int *param_1) {
  if (param_1 == NULL) {
    return -1;
  }

  char *str_data = (char *)(intptr_t)*param_1; // Base address of the string data
  int length = param_1[1];                  // Total length of the string
  int current_idx = param_1[2];             // Current parsing index

  for (int i = current_idx; i < length; ++i) {
    char c = str_data[i];
    // Check if char is not alphanumeric, and not '.', '/', or ':'
    if (!isalnum((unsigned char)c) && c != '.' && c != '/' && c != ':') {
      param_1[2] = i; // Update the current index in the structure
      return i;       // Return the found index
    }
  }
  return -1; // Character not found
}

// Function: getIndex
// param_1 is an address value, so using intptr_t for robustness on 64-bit systems.
uint32_t getIndex(intptr_t param_1_addr, uint32_t *param_2) {
  if (param_1_addr == 0) {
    return 0xFFFFFFFF;
  }
  // Reinterpret param_1_addr as a pointer to an int array (the string structure)
  int *str_struct = (int *)param_1_addr;
  *param_2 = (uint32_t)str_struct[2]; // Update param_2 with the current index
  return (uint32_t)str_struct[2];
}

// Function: skipInt
int skipInt(int *param_1) {
  if (param_1 == NULL) {
    return -1;
  }

  char *str_data = (char *)(intptr_t)*param_1;
  int length = param_1[1];
  int current_idx = param_1[2];

  for (int i = current_idx; i < length; ++i) {
    char c = str_data[i];
    // Check if char is not '+' or '-' and not a digit
    if (c != '+' && c != '-' && !isdigit((unsigned char)c)) {
      param_1[2] = i;
      return i;
    }
  }
  return -1;
}

// Function: skipFloat
int skipFloat(int *param_1) {
  if (param_1 == NULL) {
    return -1;
  }

  char *str_data = (char *)(intptr_t)*param_1;
  int length = param_1[1];
  int current_idx = param_1[2];

  for (int i = current_idx; i < length; ++i) {
    char c = str_data[i];
    // Check if char is not '.', '+', or '-' and not a digit
    if (c != '.' && c != '+' && c != '-' && !isdigit((unsigned char)c)) {
      param_1[2] = i;
      return i;
    }
  }
  return -1;
}

// Function: copyData
void *copyData(int *param_1, int param_2, int param_3) {
  // Consolidated all error checks into a single condition
  if (param_1 == NULL || param_3 < 0 || param_2 < 0 || param_3 <= param_2 ||
      param_1[1] < param_3 || param_1[1] < param_2) {
    return NULL;
  }

  size_t data_len = param_3 - param_2;
  void *new_data = NULL;

  // Use allocate wrapper for consistency, returns non-zero on failure
  if (allocate(data_len + 1, 0, &new_data, 0x1af14) != 0) {
    return NULL;
  }

  char *source_data = (char *)(intptr_t)*param_1;

  memset(new_data, 0, data_len + 1);
  memcpy(new_data, source_data + param_2, data_len);

  return new_data;
}

// Function: skipAlpha
int skipAlpha(int *param_1) {
  if (param_1 == NULL) {
    return -1;
  }

  char *str_data = (char *)(intptr_t)*param_1;
  int length = param_1[1];
  int current_idx = param_1[2];

  for (int i = current_idx; i < length; ++i) {
    char c = str_data[i];
    if (!isalpha((unsigned char)c)) {
      param_1[2] = i;
      return i;
    }
  }
  return -1;
}

// Function: skipToNonAlphaNumSpace
int skipToNonAlphaNumSpace(int *param_1) {
  if (param_1 == NULL) {
    return -1;
  }

  char *str_data = (char *)(intptr_t)*param_1;
  int length = param_1[1];
  int current_idx = param_1[2];

  for (int i = current_idx; i < length; ++i) {
    char c = str_data[i];
    // Check if char is not alphanumeric and not a space
    if (!isalnum((unsigned char)c) && c != ' ') {
      param_1[2] = i;
      return i;
    }
  }
  return -1;
}

// Function: skipToNonAlphaNum
int skipToNonAlphaNum(int *param_1) {
  if (param_1 == NULL) {
    return -1;
  }

  char *str_data = (char *)(intptr_t)*param_1;
  int length = param_1[1];
  int current_idx = param_1[2];

  for (int i = current_idx; i < length; ++i) {
    char c = str_data[i];
    if (!isalnum((unsigned char)c)) {
      param_1[2] = i;
      return i;
    }
  }
  return -1;
}

// Function: incChar
// param_1 is an address value, so using intptr_t.
uint32_t incChar(intptr_t param_1_addr) {
  if (param_1_addr == 0) {
    return 0xFFFFFFFF;
  }

  int *str_struct = (int *)param_1_addr; // [data_ptr, length, current_idx]

  // Check if (current_idx + 1) < length
  if (str_struct[2] + 1 < str_struct[1]) {
    str_struct[2]++; // Increment current_idx
    return (uint32_t)str_struct[2];
  }
  return 0xFFFFFFFF;
}

// Function: skipLength
// param_1 is an address value, so using intptr_t.
uint32_t skipLength(intptr_t param_1_addr, int param_2) {
  if (param_1_addr == 0) {
    return 0xFFFFFFFF;
  }

  int *str_struct = (int *)param_1_addr; // [data_ptr, length, current_idx]

  // Check if (current_idx + param_2) < length
  if (str_struct[2] + param_2 < str_struct[1]) {
    str_struct[2] += param_2; // Increment current_idx by param_2
    return (uint32_t)str_struct[2];
  }
  return 0xFFFFFFFF;
}

// Function: atChar
uint32_t atChar(int *param_1, char param_2) {
  if (param_1 == NULL) {
    return 0;
  }
  // Access the character directly and compare
  return (uint32_t)(param_2 == ((char *)(intptr_t)*param_1)[param_1[2]]);
}

// Function: skipTo
int skipTo(int *param_1, char param_2) {
  if (param_1 == NULL) {
    return -1;
  }

  char *str_data = (char *)(intptr_t)*param_1;
  int length = param_1[1];
  int current_idx = param_1[2];

  for (int i = current_idx; i < length; ++i) {
    if (param_2 == str_data[i]) {
      param_1[2] = i;
      return i;
    }
  }
  return -1;
}

// Function: initString
// The original return type `void **` for `local_14` was likely a decompiler artifact.
// The variable `local_14` is used as an `int*` array (e.g., `local_14[1] = ...`).
// Thus, the function returns a pointer to the `int` array structure.
int *initString(char *param_1) {
  if (param_1 == NULL) {
    return NULL;
  }

  size_t string_len = strlen(param_1);
  size_t total_data_size = string_len + 1; // For null terminator

  // Allocate the structure itself: [char_data_ptr, length, current_idx] (3 ints)
  int *string_struct = NULL;
  // Use allocate wrapper, returns non-zero on failure
  if (allocate(3 * sizeof(int), 0, (void **)&string_struct, 0x1b302) != 0) {
    return NULL;
  }

  // Allocate the actual string data buffer
  char *data_buffer = NULL;
  // Use allocate wrapper, returns non-zero on failure
  if (allocate(total_data_size, 0, (void **)&data_buffer, 0x1b302) != 0) {
    deallocate(string_struct, 3 * sizeof(int)); // Clean up previously allocated struct
    return NULL;
  }

  // Initialize data buffer
  memset(data_buffer, 0, total_data_size);
  memcpy(data_buffer, param_1, string_len);

  // Store pointers/values in the structure
  string_struct[0] = (int)(intptr_t)data_buffer; // Store address of data
  string_struct[1] = (int)total_data_size;       // Store total allocated size (including null)
  string_struct[2] = 0;                          // Initial index

  return string_struct;
}

// Function: skipWhiteSpace
int skipWhiteSpace(int *param_1) {
  if (param_1 == NULL) {
    return -1;
  }

  char *str_data = (char *)(intptr_t)*param_1;
  int length = param_1[1];
  int current_idx = param_1[2];

  if (current_idx >= length) { // No characters to skip
    return -1;
  }

  int i;
  for (i = current_idx; i < length; ++i) {
    char c = str_data[i];
    // Using isspace for more robust handling of whitespace characters
    // The original checks for '\n', '\t', ' ', '\r'. isspace covers these.
    if (!isspace((unsigned char)c)) {
      break; // Found a non-whitespace character
    }
  }

  param_1[2] = i; // Update the current index
  return i;       // Return the new index
}