#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h> // For isprint

// --- Type Definitions ---
// Standard integer types to replace decompiler's undefined types
typedef uint32_t undefined4;
typedef uint16_t ushort;
typedef uint8_t undefined;

// Forward declarations for function pointer types for object methods
typedef int (*SetDataFunc_L)(void*, uint32_t, uint32_t); // For Liquids, Cereals, Toppings
typedef int (*SetDataFunc_S)(void*, uint32_t, const char*); // For GenericString, PrinterString
typedef int (*SetDataFunc_CR)(void*, uint32_t, uint16_t, uint32_t*); // For CommandRunner
typedef void (*DestructorFunc)(void*);

// Function pointer type for the command to be executed by os_exec
typedef uint32_t (*CommandToExecuteFunc)(uint32_t, uint32_t, uint32_t);

// --- Global String Literals (Decompiler artifacts, assuming they are const char arrays) ---
const char typeName_liquids[] = "Liquids";
const char typeName_cereals[] = "Cereals";
const char typeName_toppings[] = "Toppings";
const char typeName_generic_string[] = "GenericString";
const char typeName_printer_string[] = "PrinterString";
const char typeName_command_runner[] = "CommandRunner";

// --- Helper Functions ---
// Replaced _terminate with a standard exit function
void _terminate(int status) {
    fprintf(stderr, "Fatal error, terminating with status: %d\n", status);
    exit(EXIT_FAILURE);
}

// Replaced is_printable with a standard C library check for printable characters
// Assumes it checks if *all* characters in a string are printable
static int all_chars_printable(const char* str) {
    if (str == NULL) return 0;
    for (size_t i = 0; str[i] != '\0'; ++i) {
        if (!isprint((unsigned char)str[i])) {
            return 0; // Not printable
        }
    }
    return 1; // All characters are printable
}

// --- Struct Definitions based on memory layout and field access ---

// Base structure for Liquids/Cereals
typedef struct {
    uint32_t data1;
    char type_name[8]; // Max 7 chars + null, e.g., "Liquids", "Cereals"
    uint32_t data2;
    SetDataFunc_L set_data;
    DestructorFunc destructor;
} LiquidCereal_Object; // Expected size 0x18 (24 bytes)

// Toppings structure
typedef struct {
    uint32_t data1;
    char type_name[12]; // Max 8 chars + null, e.g., "Toppings"
    uint32_t data2;
    SetDataFunc_L set_data;
    DestructorFunc destructor;
} Toppings_Object; // Expected size 0x1C (28 bytes)

// Generic String / Printer String structure
typedef struct {
    uint32_t data1;
    char type_name[16]; // Max 13 chars + null, e.g., "GenericString", "PrinterString"
    char* string_data;  // Pointer to dynamically allocated string content
    SetDataFunc_S set_data;
    DestructorFunc destructor;
} GenericString_Object; // Expected size 0x20 (32 bytes)

// Command Runner structure
typedef struct {
    uint32_t data1;
    char type_name[16]; // Max 13 chars + null, e.g., "CommandRunner"
    CommandToExecuteFunc command_func; // Function pointer to be called by os_exec
    uint16_t num_args; // Number of arguments provided for the command_func
    uint16_t padding;  // To ensure 4-byte alignment for args array
    uint32_t args[3];  // Max 3 arguments (as used by os_exec)
    SetDataFunc_CR set_data;
    DestructorFunc destructor;
} CommandRunner_Object; // Expected size 0x30 (48 bytes)

// --- Function Implementations ---

// Function: set_data_liquids
int set_data_liquids(void *param_1, uint32_t param_2, uint32_t param_3) {
  LiquidCereal_Object* obj = (LiquidCereal_Object*)param_1;
  obj->data1 = param_2;
  strncpy(obj->type_name, typeName_liquids, sizeof(obj->type_name) - 1);
  obj->type_name[sizeof(obj->type_name) - 1] = '\0'; // Ensure null-termination
  obj->data2 = param_3;
  return 0;
}

// Function: destructor_liquids
void destructor_liquids(void *param_1) {
  free(param_1);
}

// Function: constructor_liquids
void * constructor_liquids(void) {
  LiquidCereal_Object *obj = (LiquidCereal_Object*)malloc(sizeof(LiquidCereal_Object));
  if (obj == NULL) {
    _terminate(0xfffffffb);
  }
  obj->set_data = set_data_liquids;
  obj->destructor = destructor_liquids;
  return obj;
}

// Function: set_data_cereals
int set_data_cereals(void *param_1, uint32_t param_2, uint32_t param_3) {
  LiquidCereal_Object* obj = (LiquidCereal_Object*)param_1;
  obj->data1 = param_2;
  strncpy(obj->type_name, typeName_cereals, sizeof(obj->type_name) - 1);
  obj->type_name[sizeof(obj->type_name) - 1] = '\0';
  obj->data2 = param_3;
  return 0;
}

// Function: destructor_cereals
void destructor_cereals(void *param_1) {
  free(param_1);
}

// Function: constructor_cereals
void * constructor_cereals(void) {
  LiquidCereal_Object *obj = (LiquidCereal_Object*)malloc(sizeof(LiquidCereal_Object));
  if (obj == NULL) {
    _terminate(0xfffffffb);
  }
  obj->set_data = set_data_cereals;
  obj->destructor = destructor_cereals;
  return obj;
}

// Function: set_data_toppings
int set_data_toppings(void *param_1, uint32_t param_2, uint32_t param_3) {
  Toppings_Object* obj = (Toppings_Object*)param_1;
  obj->data1 = param_2;
  strncpy(obj->type_name, typeName_toppings, sizeof(obj->type_name) - 1);
  obj->type_name[sizeof(obj->type_name) - 1] = '\0';
  obj->data2 = param_3;
  return 0;
}

// Function: destructor_toppings
void destructor_toppings(void *param_1) {
  free(param_1);
}

// Function: constructor_toppings
void * constructor_toppings(void) {
  Toppings_Object *obj = (Toppings_Object*)malloc(sizeof(Toppings_Object));
  if (obj == NULL) {
    _terminate(0xfffffffb);
  }
  obj->set_data = set_data_toppings;
  obj->destructor = destructor_toppings;
  return obj;
}

// Function: set_data_generic_string
int set_data_generic_string(void *param_1, uint32_t param_2, const char *param_3) {
  GenericString_Object* obj = (GenericString_Object*)param_1;
  size_t string_len = strlen(param_3);

  obj->data1 = param_2;
  strncpy(obj->type_name, typeName_generic_string, sizeof(obj->type_name) - 1);
  obj->type_name[sizeof(obj->type_name) - 1] = '\0';

  obj->string_data = (char*)malloc(string_len + 1); // +1 for null terminator
  if (obj->string_data == NULL) {
    _terminate(0); // Original code used 0 for this specific terminate status
  }
  memcpy(obj->string_data, param_3, string_len);
  obj->string_data[string_len] = '\0'; // Ensure null termination
  return 0;
}

// Function: destructor_generic_string
void destructor_generic_string(void *param_1) {
  GenericString_Object* obj = (GenericString_Object*)param_1;
  if (obj->string_data != NULL) {
    free(obj->string_data);
  }
  free(param_1);
}

// Function: constructor_generic_string
void * constructor_generic_string(void) {
  GenericString_Object *obj = (GenericString_Object*)calloc(1, sizeof(GenericString_Object)); // calloc to zero-initialize string_data
  if (obj == NULL) {
    _terminate(0xfffffffb);
  }
  obj->set_data = set_data_generic_string;
  obj->destructor = destructor_generic_string;
  return obj;
}

// Function: set_data_printer_string
int set_data_printer_string(void *param_1, uint32_t param_2, const char *param_3) {
  GenericString_Object* obj = (GenericString_Object*)param_1;
  size_t string_len = strlen(param_3);

  obj->data1 = param_2;
  strncpy(obj->type_name, typeName_printer_string, sizeof(obj->type_name) - 1);
  obj->type_name[sizeof(obj->type_name) - 1] = '\0';

  // Check if all characters are printable before proceeding
  if (!all_chars_printable(param_3)) {
    return 0xffffffff; // Return error if not printable
  }

  obj->string_data = (char*)malloc(string_len + 1); // +1 for null terminator
  if (obj->string_data == NULL) {
    _terminate(0);
  }
  memcpy(obj->string_data, param_3, string_len);
  obj->string_data[string_len] = '\0'; // Ensure null termination
  return 0;
}

// Function: destructor_printer_string
void destructor_printer_string(void *param_1) {
  // Same destructor logic as generic_string
  destructor_generic_string(param_1);
}

// Function: constructor_printer_string
void * constructor_printer_string(void) {
  GenericString_Object *obj = (GenericString_Object*)calloc(1, sizeof(GenericString_Object));
  if (obj == NULL) {
    _terminate(0xfffffffb);
  }
  obj->set_data = set_data_printer_string;
  obj->destructor = destructor_printer_string;
  return obj;
}

// Function: os_exec (corrected based on call site in set_data_command_runner)
uint32_t os_exec(CommandToExecuteFunc func_ptr, uint32_t arg1, uint32_t arg2, uint32_t arg3) {
  if (func_ptr != NULL) {
    return func_ptr(arg1, arg2, arg3);
  }
  return 0xffffffff; // Indicate error if function pointer is null
}

// Function: set_data_command_runner
int set_data_command_runner(void *param_1, uint32_t param_2, ushort num_args_for_cmd, uint32_t *args_array) {
  CommandRunner_Object* obj = (CommandRunner_Object*)param_1;

  obj->data1 = param_2;
  strncpy(obj->type_name, typeName_command_runner, sizeof(obj->type_name) - 1);
  obj->type_name[sizeof(obj->type_name) - 1] = '\0';
  obj->num_args = num_args_for_cmd;

  // Original check: (num_args_for_cmd < 2) || (4 < num_args_for_cmd)
  // This means valid num_args_for_cmd are 2, 3, 4.
  // os_exec only uses 3 arguments (obj->args[0], obj->args[1], obj->args[2]).
  if ((num_args_for_cmd < 2) || (4 < num_args_for_cmd)) {
    return 0xffffffff; // Error: Invalid number of arguments
  }

  // args_array is assumed to be `[command_func_ptr, arg1, arg2, arg3, ...]`
  obj->command_func = (CommandToExecuteFunc)args_array[0];
  // Copy up to 3 arguments, even if num_args_for_cmd is 4, as os_exec only uses 3.
  for (int i = 0; i < num_args_for_cmd && i < 3; ++i) {
    obj->args[i] = args_array[1 + i];
  }

  // Execute the command immediately after setting data
  os_exec(obj->command_func, obj->args[0], obj->args[1], obj->args[2]);
  return 0;
}

// Function: destructor_command_runner
void destructor_command_runner(void *param_1) {
  free(param_1);
}

// Function: constructor_command_runner
void * constructor_command_runner(void) {
  CommandRunner_Object *obj = (CommandRunner_Object*)calloc(1, sizeof(CommandRunner_Object));
  if (obj == NULL) {
    _terminate(0xfffffffb);
  }
  obj->set_data = set_data_command_runner;
  obj->destructor = destructor_command_runner;
  return obj;
}

// --- Deserialize Functions ---
// Changed param_1 to char** to allow byte-wise pointer advancement

// Function: deserialize_liquids
int deserialize_liquids(char **param_1) {
  uint32_t data1 = *(uint32_t*)*param_1;
  *param_1 += sizeof(uint32_t); // Advance past data1

  size_t name_len = strlen(typeName_liquids);
  *param_1 += name_len; // Skip the type name in the buffer

  uint32_t data2 = *(uint32_t*)*param_1;
  *param_1 += sizeof(uint32_t); // Advance past data2

  LiquidCereal_Object* obj = (LiquidCereal_Object*)constructor_liquids();
  if (obj == NULL) return 0;

  if (obj->set_data(obj, data1, data2) < 0) {
    obj->destructor(obj);
    return 0;
  }
  return (int)obj;
}

// Function: deserialize_cereals
int deserialize_cereals(char **param_1) {
  uint32_t data1 = *(uint32_t*)*param_1;
  *param_1 += sizeof(uint32_t);

  size_t name_len = strlen(typeName_cereals);
  *param_1 += name_len;

  uint32_t data2 = *(uint32_t*)*param_1;
  *param_1 += sizeof(uint32_t);

  LiquidCereal_Object* obj = (LiquidCereal_Object*)constructor_cereals();
  if (obj == NULL) return 0;

  if (obj->set_data(obj, data1, data2) < 0) {
    obj->destructor(obj);
    return 0;
  }
  return (int)obj;
}

// Function: deserialize_toppings
int deserialize_toppings(char **param_1) {
  uint32_t data1 = *(uint32_t*)*param_1;
  *param_1 += sizeof(uint32_t);

  size_t name_len = strlen(typeName_toppings);
  *param_1 += name_len;

  uint32_t data2 = *(uint32_t*)*param_1;
  *param_1 += sizeof(uint32_t);

  Toppings_Object* obj = (Toppings_Object*)constructor_toppings();
  if (obj == NULL) return 0;

  if (obj->set_data(obj, data1, data2) < 0) {
    obj->destructor(obj);
    return 0;
  }
  return (int)obj;
}

// Function: deserialize_generic_string
int deserialize_generic_string(char **param_1) {
  uint32_t data1 = *(uint32_t*)*param_1;
  *param_1 += sizeof(uint32_t);

  size_t name_len = strlen(typeName_generic_string);
  *param_1 += name_len;

  char *str_val = *param_1;
  size_t str_val_len = strlen(str_val);
  *param_1 += str_val_len + 1; // Advance past string and its null terminator

  GenericString_Object* obj = (GenericString_Object*)constructor_generic_string();
  if (obj == NULL) return 0;

  if (obj->set_data(obj, data1, str_val) < 0) {
    obj->destructor(obj);
    return 0;
  }
  return (int)obj;
}

// Function: deserialize_printer_string
int deserialize_printer_string(char **param_1) {
  uint32_t data1 = *(uint32_t*)*param_1;
  *param_1 += sizeof(uint32_t);

  size_t name_len = strlen(typeName_printer_string);
  *param_1 += name_len;

  char *str_val = *param_1;
  size_t str_val_len = strlen(str_val);
  *param_1 += str_val_len + 1;

  GenericString_Object* obj = (GenericString_Object*)constructor_printer_string();
  if (obj == NULL) return 0;

  if (obj->set_data(obj, data1, str_val) < 0) {
    obj->destructor(obj);
    return 0;
  }
  return (int)obj;
}

// Function: deserialize_command_runner
int deserialize_command_runner(char **param_1) {
  uint32_t data1 = *(uint32_t*)*param_1;
  *param_1 += sizeof(uint32_t);

  size_t name_len = strlen(typeName_command_runner);
  *param_1 += name_len;

  ushort num_args_for_cmd = *(ushort*)*param_1;
  *param_1 += sizeof(ushort);

  // Original check: (num_args_for_cmd < 2) || (4 < num_args_for_cmd)
  if ((num_args_for_cmd < 2) || (4 < num_args_for_cmd)) {
    return 0; // Invalid number of arguments
  }

  // Assume args_data points to `[command_func_ptr, arg1, arg2, ...]`
  // So, total `(1 + num_args_for_cmd)` uint32_t elements.
  uint32_t *args_data = (uint32_t*)*param_1;
  *param_1 += (1 + num_args_for_cmd) * sizeof(uint32_t);

  CommandRunner_Object* obj = (CommandRunner_Object*)constructor_command_runner();
  if (obj == NULL) return 0;

  if (obj->set_data(obj, data1, num_args_for_cmd, args_data) < 0) {
    obj->destructor(obj);
    return 0;
  }
  return (int)obj;
}

// Function: check_inherited_types
int check_inherited_types(uint32_t param_1, char *param_2) {
  int obj_ptr = 0; // Renamed iVar1 to obj_ptr for clarity

  switch (param_1) {
    case 5: // CommandRunner
      obj_ptr = deserialize_command_runner(&param_2);
      if (obj_ptr == 0) {
        return 0xffffffff;
      }
      ((CommandRunner_Object*)obj_ptr)->destructor((void*)obj_ptr);
      break;
    case 3: // GenericString
      obj_ptr = deserialize_generic_string(&param_2);
      if (obj_ptr == 0) {
        return 0xffffffff;
      }
      ((GenericString_Object*)obj_ptr)->destructor((void*)obj_ptr);
      break;
    case 4: // PrinterString
      obj_ptr = deserialize_printer_string(&param_2);
      if (obj_ptr == 0) {
        return 0xffffffff;
      }
      ((GenericString_Object*)obj_ptr)->destructor((void*)obj_ptr);
      break;
    default: // Other types (0, 1, 2) or invalid
      // Original code returns 0xffffffff for other cases, implying they are not handled here.
      return 0xffffffff;
  }
  return 0;
}