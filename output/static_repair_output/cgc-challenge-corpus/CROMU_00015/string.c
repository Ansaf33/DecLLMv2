#include <stdio.h>    // For NULL (though stdlib also has it)
#include <stdlib.h>   // For malloc, free
#include <string.h>   // For strlen, memcpy, memset
#include <ctype.h>    // For isalnum, isdigit, isalpha
#include <stdint.h>   // For intptr_t

// Mimic allocate: allocates memory and stores the pointer in *ptr_out
// Returns 0 on success, non-zero on failure.
// The `flags` and `debug_info` parameters are ignored for standard malloc.
int allocate(size_t size, int flags, void **ptr_out, int debug_info) {
    *ptr_out = malloc(size);
    return (*ptr_out == NULL); // 0 for success (allocated), 1 for failure
}

// Mimic deallocate: frees memory
// The `size` parameter is ignored for standard free.
void deallocate(void *ptr, size_t size) {
    free(ptr);
}

// The 'undefined4' type is assumed to be an unsigned 4-byte integer.
typedef unsigned int undefined4;

// Function: freeString
void freeString(int *param_1) {
  if (param_1 != NULL) {
    // param_1[0] stores the address of the string data.
    // param_1[1] stores the length/size of that string data block.
    if (*param_1 != 0) {
      deallocate((void *)(intptr_t)*param_1, (size_t)param_1[1]);
    }
    // param_1 itself points to the structure/metadata block (likely 3 ints).
    deallocate(param_1, 0xc); // 0xc is 12 bytes, likely sizeof(int)*3
  }
}

// Function: skipUrl
int skipUrl(int *param_1) {
  if (param_1 == NULL) {
    return -1;
  }
  char *data = (char *)(intptr_t)*param_1; // Actual string data
  int length = param_1[1];                 // Total length of buffer

  for (int i = param_1[2]; i < length; ++i) {
    char c = data[i];
    if (!isalnum((unsigned char)c) && c != '.' && c != '/' && c != ':') {
      param_1[2] = i; // Update current_pos in the structure
      return i;
    }
  }
  return -1;
}

// Function: getIndex
undefined4 getIndex(int param_1, undefined4 *param_2) {
  if (param_1 == 0) { // param_1 is an int, likely a pointer cast to int
    return 0xffffffff;
  }
  // param_1 (as int) is the address of the String struct (int*).
  // *(undefined4 *)(param_1 + 8) accesses param_1[2] (current_pos).
  *param_2 = ((undefined4 *)(intptr_t)param_1)[2]; // Store current_pos in param_2
  return *param_2; // Return current_pos
}

// Function: skipInt
int skipInt(int *param_1) {
  if (param_1 == NULL) {
    return -1;
  }
  char *data = (char *)(intptr_t)*param_1;
  int length = param_1[1];

  for (int i = param_1[2]; i < length; ++i) {
    char c = data[i];
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
  char *data = (char *)(intptr_t)*param_1;
  int length = param_1[1];

  for (int i = param_1[2]; i < length; ++i) {
    char c = data[i];
    if (c != '.' && c != '+' && c != '-' && !isdigit((unsigned char)c)) {
      param_1[2] = i;
      return i;
    }
  }
  return -1;
}

// Function: copyData
void * copyData(int *param_1, int param_2, int param_3) {
  if (param_1 == NULL || param_3 < 0 || param_2 < 0 || param_3 <= param_2 ||
      param_1[1] < param_3 || param_1[1] < param_2) {
    return NULL;
  }

  size_t copy_length = param_3 - param_2;
  void *new_data = NULL;
  if (allocate(copy_length + 1, 0, &new_data, 0x1af14) == 0) {
    memset(new_data, 0, copy_length + 1);
    memcpy(new_data, (void *)((intptr_t)*param_1 + param_2), copy_length);
  }
  return new_data;
}

// Function: skipAlpha
int skipAlpha(int *param_1) {
  if (param_1 == NULL) {
    return -1;
  }
  char *data = (char *)(intptr_t)*param_1;
  int length = param_1[1];

  for (int i = param_1[2]; i < length; ++i) {
    if (!isalpha((unsigned char)data[i])) {
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
  char *data = (char *)(intptr_t)*param_1;
  int length = param_1[1];

  for (int i = param_1[2]; i < length; ++i) {
    char c = data[i];
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
  char *data = (char *)(intptr_t)*param_1;
  int length = param_1[1];

  for (int i = param_1[2]; i < length; ++i) {
    if (!isalnum((unsigned char)data[i])) {
      param_1[2] = i;
      return i;
    }
  }
  return -1;
}

// Function: incChar
undefined4 incChar(int param_1) {
  // param_1 is int, likely a pointer cast to int
  if (param_1 == 0) {
    return 0xffffffff;
  }
  int *p_struct = (int *)(intptr_t)param_1;
  if (p_struct[2] + 1 < p_struct[1]) {
    p_struct[2]++;
    return p_struct[2];
  }
  return 0xffffffff;
}

// Function: skipLength
undefined4 skipLength(int param_1, int param_2) {
  // param_1 is int, likely a pointer cast to int
  if (param_1 == 0) {
    return 0xffffffff;
  }
  int *p_struct = (int *)(intptr_t)param_1;
  if (p_struct[2] + param_2 < p_struct[1]) {
    p_struct[2] += param_2;
    return p_struct[2];
  }
  return 0xffffffff;
}

// Function: atChar
undefined4 atChar(int *param_1, char param_2) {
  if (param_1 == NULL) {
    return 0; // False
  }
  char *data = (char *)(intptr_t)*param_1;
  int current_pos = param_1[2];

  if (current_pos < param_1[1] && param_2 == data[current_pos]) {
    return 1; // True
  }
  return 0; // False
}

// Function: skipTo
int skipTo(int *param_1, char param_2) {
  if (param_1 == NULL) {
    return -1;
  }
  char *data = (char *)(intptr_t)*param_1;
  int length = param_1[1];

  for (int i = param_1[2]; i < length; ++i) {
    if (param_2 == data[i]) {
      param_1[2] = i;
      return i;
    }
  }
  return -1;
}

// Function: initString
int * initString(char *param_1) {
  if (param_1 == NULL) {
    return NULL;
  }

  size_t string_len = strlen(param_1);
  size_t data_len_with_null = string_len + 1;

  int *string_struct_ptr = NULL;
  void *temp_struct_ptr = NULL;

  // Allocate 0xc (12 bytes) for the struct: [data_ptr, length, current_pos]
  if (allocate(0xc, 0, &temp_struct_ptr, 0x1b302) != 0) {
    return NULL; // Allocation for struct failed
  }
  string_struct_ptr = (int *)temp_struct_ptr;

  void *char_data_ptr = NULL;
  // Allocate memory for the actual character data
  if (allocate(data_len_with_null, 0, &char_data_ptr, 0x1b302) != 0) {
    deallocate(string_struct_ptr, 0xc); // Free the struct if data allocation fails
    return NULL;
  }

  // Store the data pointer (as an int, due to original code pattern)
  string_struct_ptr[0] = (int)(intptr_t)char_data_ptr;
  // Initialize data buffer
  memset(char_data_ptr, 0, data_len_with_null);
  memcpy(char_data_ptr, param_1, string_len);

  // Store length and current_pos
  string_struct_ptr[1] = (int)data_len_with_null; // Total allocated size including null
  string_struct_ptr[2] = 0;                        // Initial current position

  return string_struct_ptr;
}

// Function: skipWhiteSpace
int skipWhiteSpace(int *param_1) {
  if (param_1 == NULL) {
    return -1;
  }
  char *data = (char *)(intptr_t)*param_1;
  int length = param_1[1];
  int i = param_1[2];

  if (i >= length) {
    return -1;
  }

  while (i < length) {
    char c = data[i];
    if (c != '\n' && c != '\t' && c != ' ' && c != '\r') {
      break;
    }
    i++;
  }
  param_1[2] = i; // Update current_pos
  return i;
}