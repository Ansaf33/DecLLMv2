#include <stdlib.h> // For malloc, free, calloc, exit
#include <string.h> // For strlen, memcpy
#include <stdint.h> // For uint32_t, uint16_t, intptr_t
#include <stdio.h>  // For fprintf
#include <ctype.h>  // For isprint

// Define custom types
typedef uint32_t undefined4;
typedef uint16_t ushort;
// 'code' is a generic function pointer type.
// Specific function signatures will be used via explicit casts at call sites.
typedef undefined4 (*code)();

// Global variables (string literals)
const char *typeName = "Liquids";
const char *PTR_s_Cereals_00017004 = "Cereals";
const char *PTR_s_Toppings_00017008 = "Toppings";
const char *PTR_s_GenericString_0001700c = "GenericString";
const char *PTR_s_PrinterString_00017010 = "PrinterString";
const char *PTR_s_CommandRunner_00017014 = "CommandRunner";

// Dummy _terminate function
static void _terminate(int error_code) {
    fprintf(stderr, "Error: Program terminated with code 0x%x.\n", error_code);
    exit(EXIT_FAILURE);
}

// Dummy is_printable function.
// The original snippet's loop for `is_printable()` is structurally flawed
// as it doesn't pass a character to check. In set_data_printer_string,
// `isprint` from <ctype.h> is used on characters of `param_3`.
// This dummy function is not used in the fixed code.
static int is_printable(void) {
    return 1; // Dummy implementation: always assumes characters are printable.
}

// Function: set_data_liquids
undefined4 set_data_liquids(uint32_t *param_1, undefined4 param_2, undefined4 param_3) {
  *param_1 = param_2;
  memcpy(param_1 + 1, typeName, strlen(typeName));
  param_1[3] = param_3;
  return 0;
}

// Function: destructor_liquids
void destructor_liquids(void *param_1) {
  free(param_1);
}

// Function: constructor_liquids
void * constructor_liquids(void) {
  void *pvVar1 = malloc(0x18); // 24 bytes
  if (pvVar1 == NULL) {
    _terminate(0xfffffffb);
  }
  *(code **)((char *)pvVar1 + 0x10) = (code)set_data_liquids;
  *(code **)((char *)pvVar1 + 0x14) = (code)destructor_liquids;
  return pvVar1;
}

// Function: set_data_cereals
undefined4 set_data_cereals(uint32_t *param_1, undefined4 param_2, undefined4 param_3) {
  *param_1 = param_2;
  memcpy(param_1 + 1, PTR_s_Cereals_00017004, strlen(PTR_s_Cereals_00017004));
  param_1[3] = param_3;
  return 0;
}

// Function: destructor_cereals
void destructor_cereals(void *param_1) {
  free(param_1);
}

// Function: constructor_cereals
void * constructor_cereals(void) {
  void *pvVar1 = malloc(0x18); // 24 bytes
  if (pvVar1 == NULL) {
    _terminate(0xfffffffb);
  }
  *(code **)((char *)pvVar1 + 0x10) = (code)set_data_cereals;
  *(code **)((char *)pvVar1 + 0x14) = (code)destructor_cereals;
  return pvVar1;
}

// Function: set_data_toppings
undefined4 set_data_toppings(uint32_t *param_1, undefined4 param_2, undefined4 param_3) {
  *param_1 = param_2;
  memcpy(param_1 + 1, PTR_s_Toppings_00017008, strlen(PTR_s_Toppings_00017008));
  param_1[4] = param_3;
  return 0;
}

// Function: destructor_toppings
void destructor_toppings(void *param_1) {
  free(param_1);
}

// Function: constructor_toppings
void * constructor_toppings(void) {
  void *pvVar1 = malloc(0x1c); // 28 bytes
  if (pvVar1 == NULL) {
    _terminate(0xfffffffb);
  }
  *(code **)((char *)pvVar1 + 0x14) = (code)set_data_toppings;
  *(code **)((char *)pvVar1 + 0x18) = (code)destructor_toppings;
  return pvVar1;
}

// Function: set_data_generic_string
undefined4 set_data_generic_string(uint32_t *param_1, undefined4 param_2, char *param_3) {
  size_t string_len = strlen(param_3);
  *param_1 = param_2;
  memcpy(param_1 + 1, PTR_s_GenericString_0001700c, strlen(PTR_s_GenericString_0001700c));
  
  // param_1[5] is expected to store a pointer to the newly allocated string buffer (Offset 0x14)
  void **string_ptr_location = (void **)((char *)param_1 + 0x14); 
  *string_ptr_location = malloc(string_len + 1); // +1 for null terminator
  if (*string_ptr_location == NULL) {
    _terminate(0xfffffffb); // Memory allocation failure
  }
  memcpy(*string_ptr_location, param_3, string_len + 1); // Copy with null terminator
  return 0;
}

// Function: destructor_generic_string
void destructor_generic_string(void *param_1) {
  // Offset 0x14 (20 bytes) is param_1[5], which stores the allocated string pointer.
  void *string_buffer = *(void **)((char *)param_1 + 0x14);
  if (string_buffer != NULL) {
    free(string_buffer);
  }
  free(param_1);
}

// Function: constructor_generic_string
void * constructor_generic_string(void) {
  void *pvVar1 = calloc(1, 0x20); // 32 bytes, initialized to zero
  if (pvVar1 == NULL) {
    _terminate(0xfffffffb);
  }
  *(code **)((char *)pvVar1 + 0x18) = (code)set_data_generic_string;
  *(code **)((char *)pvVar1 + 0x1c) = (code)destructor_generic_string;
  return pvVar1;
}

// Function: set_data_printer_string
undefined4 set_data_printer_string(uint32_t *param_1, undefined4 param_2, char *param_3) {
  size_t string_len = strlen(param_3);
  *param_1 = param_2;
  memcpy(param_1 + 1, PTR_s_PrinterString_00017010, strlen(PTR_s_PrinterString_00017010));

  // The original loop for is_printable() is structurally flawed as it doesn't pass a character to check.
  // Assuming the *intent* was to check each character of `param_3` for printability.
  for (size_t i = 0; i < string_len; ++i) {
    if (!isprint((unsigned char)param_3[i])) { // Using isprint from <ctype.h>
      return 0xffffffff; // Not all characters are printable
    }
  }

  // Offset 0x14 (20 bytes) is param_1[5]
  void **string_ptr_location = (void **)((char *)param_1 + 0x14); 
  *string_ptr_location = malloc(string_len + 1); // +1 for null terminator
  if (*string_ptr_location == NULL) {
    _terminate(0xfffffffb); // Memory allocation failure
  }
  memcpy(*string_ptr_location, param_3, string_len + 1); // Copy with null terminator
  return 0;
}

// Function: destructor_printer_string
void destructor_printer_string(void *param_1) {
  // Offset 0x14 (20 bytes) is param_1[5]
  void *string_buffer = *(void **)((char *)param_1 + 0x14); 
  if (string_buffer != NULL) {
    free(string_buffer);
  }
  free(param_1);
}

// Function: constructor_printer_string
void * constructor_printer_string(void) {
  void *pvVar1 = calloc(1, 0x20); // 32 bytes, initialized to zero
  if (pvVar1 == NULL) {
    _terminate(0xfffffffb);
  }
  *(code **)((char *)pvVar1 + 0x18) = (code)set_data_printer_string;
  *(code **)((char *)pvVar1 + 0x1c) = (code)destructor_printer_string;
  return pvVar1;
}

// Function: os_exec
// This function takes a function pointer and other arguments, but only passes its 4th argument
// to the function pointer.
undefined4 os_exec(code func_ptr_to_execute, undefined4 unused_param2, undefined4 unused_param3, undefined4 actual_param) {
  // Cast `func_ptr_to_execute` to a function pointer type that takes one `undefined4` argument.
  return ((undefined4 (*)(undefined4))func_ptr_to_execute)(actual_param);
}

// Function: set_data_command_runner
undefined4 set_data_command_runner(uint32_t *param_1, undefined4 param_2, ushort param_3, uint32_t *param_4) {
  *param_1 = param_2;
  memcpy(param_1 + 1, PTR_s_CommandRunner_00017014, strlen(PTR_s_CommandRunner_00017014));
  
  // Offset 0x18 (24 bytes) is param_1[6]
  *(ushort *)((char *)param_1 + 0x18) = param_3;

  if ((param_3 < 2) || (4 < param_3)) {
    return 0xffffffff;
  }

  // Offset 0x14 (20 bytes) is param_1[5]
  *(code *)((char *)param_1 + 0x14) = (code)*param_4; // Store function pointer

  // Copy arguments: param_4 points to the function pointer, followed by arguments.
  // The function pointer itself is `param_4[0]`. The actual arguments start from `param_4[1]`.
  // `param_3` is the count of arguments *including* the function pointer (e.g., if param_3=2, it means 1 func ptr + 1 arg).
  // The loop copies `param_3 - 1` arguments. Max `param_3` is 4, so max 3 args are copied.
  uint32_t *current_arg_ptr = param_4 + 1; // Skip the function pointer itself
  for (uint32_t i = 0; i < param_3 - 1; ++i) { // Copy `param_3 - 1` arguments
    param_1[i + 7] = current_arg_ptr[i]; // Arguments stored at param_1[7], param_1[8], param_1[9]
  }

  // Call os_exec with the stored function pointer and its arguments.
  // os_exec will only use param_1[9] as the actual argument for the command.
  os_exec(*(code *)((char *)param_1 + 0x14), param_1[7], param_1[8], param_1[9]);
  
  return 0;
}

// Function: destructor_command_runner
void destructor_command_runner(void *param_1) {
  free(param_1);
}

// Function: constructor_command_runner
void * constructor_command_runner(void) {
  void *pvVar1 = calloc(1, 0x30); // 48 bytes, initialized to zero
  if (pvVar1 == NULL) {
    _terminate(0xfffffffb);
  }
  *(code **)((char *)pvVar1 + 0x28) = (code)set_data_command_runner;
  *(code **)((char *)pvVar1 + 0x2c) = (code)destructor_command_runner;
  return pvVar1;
}

// Function: deserialize_liquids
void * deserialize_liquids(char **param_1) {
  uint32_t val1 = *(uint32_t *)*param_1;
  *param_1 += sizeof(uint32_t); // Advance pointer by 4 bytes

  *param_1 += strlen(typeName); // Advance pointer by string length

  uint32_t val2 = *(uint32_t *)*param_1;
  *param_1 += sizeof(uint32_t); // Advance pointer by 4 bytes

  void *obj = constructor_liquids();
  if (obj == NULL) return NULL;
  // Cast function pointer to the correct signature for the call
  int call_result = ((undefined4 (*)(void*, undefined4, undefined4))(*(code *)((char *)obj + 0x10)))(obj, val1, val2);
  if (call_result < 0) {
    ((void (*)(void*))(*(code *)((char *)obj + 0x14)))(obj); // Call destructor
    obj = NULL;
  }
  return obj;
}

// Function: deserialize_cereals
void * deserialize_cereals(char **param_1) {
  uint32_t val1 = *(uint32_t *)*param_1;
  *param_1 += sizeof(uint32_t);

  *param_1 += strlen(PTR_s_Cereals_00017004);

  uint32_t val2 = *(uint32_t *)*param_1;
  *param_1 += sizeof(uint32_t);

  void *obj = constructor_cereals();
  if (obj == NULL) return NULL;
  int call_result = ((undefined4 (*)(void*, undefined4, undefined4))(*(code *)((char *)obj + 0x10)))(obj, val1, val2);
  if (call_result < 0) {
    ((void (*)(void*))(*(code *)((char *)obj + 0x14)))(obj);
    obj = NULL;
  }
  return obj;
}

// Function: deserialize_toppings
void * deserialize_toppings(char **param_1) {
  uint32_t val1 = *(uint32_t *)*param_1;
  *param_1 += sizeof(uint32_t);

  *param_1 += strlen(PTR_s_Toppings_00017008);

  uint32_t val2 = *(uint32_t *)*param_1;
  *param_1 += sizeof(uint32_t);

  void *obj = constructor_toppings();
  if (obj == NULL) return NULL;
  // Note: deserialize_toppings calls offset 0x14 (set_data) and 0x18 (destructor)
  // which is consistent with constructor_toppings.
  int call_result = ((undefined4 (*)(void*, undefined4, undefined4))(*(code *)((char *)obj + 0x14)))(obj, val1, val2);
  if (call_result < 0) {
    ((void (*)(void*))(*(code *)((char *)obj + 0x18)))(obj);
    obj = NULL;
  }
  return obj;
}

// Function: deserialize_generic_string
void * deserialize_generic_string(char **param_1) {
  uint32_t val1 = *(uint32_t *)*param_1;
  *param_1 += sizeof(uint32_t);

  *param_1 += strlen(PTR_s_GenericString_0001700c);

  char *str_data = *param_1;
  *param_1 += strlen(str_data) + 1; // Advance past string and its null terminator

  void *obj = constructor_generic_string();
  if (obj == NULL) return NULL;
  // Note: deserialize_generic_string calls offset 0x18 (set_data) and 0x1c (destructor)
  // which is consistent with constructor_generic_string.
  int call_result = ((undefined4 (*)(void*, undefined4, char*))(*(code *)((char *)obj + 0x18)))(obj, val1, str_data);
  if (call_result < 0) {
    ((void (*)(void*))(*(code *)((char *)obj + 0x1c)))(obj);
    obj = NULL;
  }
  return obj;
}

// Function: deserialize_printer_string
void * deserialize_printer_string(char **param_1) {
  uint32_t val1 = *(uint32_t *)*param_1;
  *param_1 += sizeof(uint32_t);

  *param_1 += strlen(PTR_s_PrinterString_00017010);

  char *str_data = *param_1;
  *param_1 += strlen(str_data) + 1; // Advance past string and its null terminator

  void *obj = constructor_printer_string();
  if (obj == NULL) return NULL;
  // Note: deserialize_printer_string calls offset 0x18 (set_data) and 0x1c (destructor)
  // which is consistent with constructor_printer_string.
  int call_result = ((undefined4 (*)(void*, undefined4, char*))(*(code *)((char *)obj + 0x18)))(obj, val1, str_data);
  if (call_result < 0) {
    ((void (*)(void*))(*(code *)((char *)obj + 0x1c)))(obj);
    obj = NULL;
  }
  return obj;
}

// Function: deserialize_command_runner
void * deserialize_command_runner(char **param_1) {
  uint32_t val1 = *(uint32_t *)*param_1;
  *param_1 += sizeof(uint32_t);

  *param_1 += strlen(PTR_s_CommandRunner_00017014);

  ushort uVar1 = *(ushort *)*param_1;
  *param_1 += sizeof(ushort); // Advance pointer by 2 bytes for ushort

  if ((uVar1 < 2) || (4 < uVar1)) {
    return NULL; // Return NULL on invalid count
  }

  uint32_t *args_ptr = (uint32_t *)*param_1; // Pointer to the function pointer and its args
  // Advance param_1 past the function pointer + uVar1 arguments.
  // Assuming each argument (including the function pointer) is a uint32_t (4 bytes).
  *param_1 += (uVar1 + 1) * sizeof(uint32_t);

  void *obj = constructor_command_runner();
  if (obj == NULL) return NULL;
  // Note: deserialize_command_runner calls offset 0x28 (set_data) and 0x2c (destructor)
  // which is consistent with constructor_command_runner.
  int call_result = ((undefined4 (*)(void*, undefined4, ushort, uint32_t*))(*(code *)((char *)obj + 0x28)))(obj, val1, uVar1, args_ptr);
  if (call_result < 0) {
    ((void (*)(void*))(*(code *)((char *)obj + 0x2c)))(obj);
    obj = NULL;
  }
  return obj;
}

// Function: check_inherited_types
undefined4 check_inherited_types(uint32_t param_1, char **param_2) {
  void *obj = NULL;

  if (param_1 == 5) { // Command Runner
    obj = deserialize_command_runner(param_2);
    if (obj == NULL) {
      return 0xffffffff;
    }
    ((void (*)(void*))(*(code *)((char *)obj + 0x2c)))(obj); // Call destructor
  } else if (param_1 == 3) { // Generic String
    obj = deserialize_generic_string(param_2);
    if (obj == NULL) {
      return 0xffffffff;
    }
    ((void (*)(void*))(*(code *)((char *)obj + 0x1c)))(obj); // Call destructor
  } else if (param_1 == 4) { // Printer String
    obj = deserialize_printer_string(param_2);
    if (obj == NULL) {
      return 0xffffffff;
    }
    ((void (*)(void*))(*(code *)((char *)obj + 0x1c)))(obj); // Call destructor
  } else {
    // Covers cases where param_1 is < 3, or param_1 is > 5, or param_1 is 0, 1, 2.
    return 0xffffffff;
  }
  return 0;
}