#include <stdio.h>    // For dprintf, fprintf, snprintf, vsnprintf
#include <stdlib.h>   // For malloc, free, strtoul, rand, srand
#include <string.h>   // For memcpy, strlen, strcmp, strsep, strcpy
#include <stdbool.h>  // For bool
#include <stddef.h>   // For size_t
#include <stdint.h>   // For uint8_t, uint32_t, etc.
#include <unistd.h>   // For write, read
#include <stdarg.h>   // For va_list in variadic functions

// --- Type definitions ---
typedef unsigned int uint;
typedef unsigned int undefined4; // Assuming undefined4 is a 4-byte unsigned integer
typedef void undefined;          // Assuming undefined is just a generic void type

// Function pointer types for commands and functions
typedef unsigned int (*command_handler_t)(char *);
typedef void (*function_handler_t)(char *); // Original returns void, adjusted to return unsigned int if needed by main

// --- Global variables ---
char *g_output_buf = NULL;
unsigned int g_output_len = 0;
unsigned long g_token = 0;
int g_auth = 0;

// Structure for linked list of variables
typedef struct Variable {
    struct Variable *next;
    char *name;  // Pointer to name string within allocated block
    char *value; // Pointer to value string within allocated block
} Variable;

Variable *variables = NULL;

// Call counters for functions
unsigned int home_call_count = 0;
unsigned int root64_call_count = 0;
unsigned int parcour_call_count = 0;

// --- String literals (from DAT_ addresses) ---
const char DAT_00014073[] = "BYE\n";
const char DAT_00014081[] = "OK\n";
const char DAT_000140b3[] = "--------------------\n";
const char DAT_000140b5[] = "mode";
const char DAT_000140ba[] = "data";
const char DAT_000140cd[] = "key";

// --- Dummy implementations for external functions ---
// Using dprintf for fdprintf, which is a common Linux extension.
// If not available, replace with fprintf(fd == 1 ? stdout : stderr, ...)
int fdprintf(int fd, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vdprintf(fd, format, args);
    va_end(args);
    return ret;
}

// Simple dummy readuntil
int readuntil(int fd, char *buf, size_t max_len, char delimiter) {
    size_t bytes_read = 0;
    char c;
    while (bytes_read < max_len - 1 && read(fd, &c, 1) == 1) {
        buf[bytes_read++] = c;
        if (c == delimiter) {
            break;
        }
    }
    buf[bytes_read] = '\0';
    if (bytes_read == 0 && c != delimiter) {
        return -1; // Indicate error or EOF
    }
    return bytes_read;
}

// Simple dummy writeall
int writeall(int fd, const void *buf, size_t count) {
    size_t total_written = 0;
    const char *ptr = (const char *)buf;
    while (total_written < count) {
        ssize_t written = write(fd, ptr + total_written, count - total_written);
        if (written == -1) {
            return -1; // Error
        }
        total_written += written;
    }
    return total_written;
}

// Dummy random (using standard library rand)
long int random(void) {
    return rand();
}

// Dummy root64_encode (copies for now, assuming dest buffer is 256 bytes)
undefined4 root64_encode(void *dest, const char *src) {
    size_t len = strlen(src);
    if (len >= 256) { // Ensure dest buffer capacity
        return 0; // Indicate failure or truncation
    }
    strcpy((char*)dest, src);
    return len;
}

// Dummy root64_decode (copies for now, assuming dest buffer is 256 bytes)
undefined4 root64_decode(void *dest, const char *src) {
    size_t len = strlen(src);
    if (len >= 256) { // Ensure dest buffer capacity
        return 0; // Indicate failure or truncation
    }
    strcpy((char*)dest, src);
    return len;
}

// Dummy parcour_init
void parcour_init(const char *key, size_t len) {
    (void)key; // Suppress unused parameter warning
    (void)len; // Suppress unused parameter warning
    // In a real implementation, this would initialize a cipher state
}

// Dummy parcour_byte
uint8_t parcour_byte(void) {
    return (uint8_t)rand(); // Return a random byte
}

// --- Original functions ---

// Function: output_clear
void output_clear(void) {
  g_output_len = 0;
}

// Function: output_write
void output_write(void *param_1, uint param_2) {
  if (0xffffU - g_output_len < param_2) {
    param_2 = 0xffffU - g_output_len;
  }
  if (param_2 != 0) {
    memcpy(g_output_buf + g_output_len, param_1, param_2);
    g_output_len += param_2;
  }
}

// Function: output_append
void output_append(char *param_1, ...) {
  va_list args;
  va_start(args, param_1);
  int written_len = vsnprintf(g_output_buf + g_output_len, 0xffff - g_output_len, param_1, args);
  va_end(args);
  if (written_len > 0) {
    g_output_len += written_len;
  }
}

// Function: output_transmit
undefined4 output_transmit(void) {
  int ret_code = 1; // Default to error
  if (fdprintf(1, "OK %u\n", g_output_len) >= 4) { // "OK %u\n" minimum length 4 (e.g. "OK 0\n")
    if (writeall(1, g_output_buf, g_output_len) == (int)g_output_len) {
      ret_code = 0;
    }
  }
  return ret_code;
}

// Function: send_error
bool send_error(undefined4 param_1) {
  return fdprintf(1, "ERROR %3d\n", param_1) < 10;
}

// Function: variable_get
char *variable_get(char *param_1) {
  Variable *current_var = variables;
  while(current_var != NULL) {
    if (strcmp(param_1, current_var->name) == 0) {
      return current_var->value;
    }
    current_var = current_var->next;
  }
  return NULL;
}

// Forward declarations for handlers
unsigned int handle_hello(char *param_1);
unsigned int handle_auth(char *param_1);
unsigned int handle_set(char *param_1);
unsigned int handle_call(char *param_1); // Adjusted to return unsigned int

// Command table (char* command_name, command_handler_t handler_function)
const void *commands[] = {
    "HELLO", (command_handler_t)handle_hello,
    "AUTH",  (command_handler_t)handle_auth,
    "SET",   (command_handler_t)handle_set,
    "CALL",  (command_handler_t)handle_call,
    NULL, NULL // Sentinel
};

// Forward declarations for page handlers
unsigned int page_home(void);
unsigned int page_root64(void);
unsigned int page_parcour(void);

// Function table entry: { "function_name", "description", handler_function, call_count_ptr }
const void *functions[] = {
    "home",    "Home page",            (function_handler_t)page_home,    &home_call_count,
    "root64",  "Root64 encoder/decoder", (function_handler_t)page_root64,  &root64_call_count,
    "parcour", "Parcour cipher",       (function_handler_t)page_parcour, &parcour_call_count,
    NULL, NULL, NULL, NULL // Sentinel
};

// Function: handle_hello
unsigned int handle_hello(char *param_1) {
  if (param_1 == NULL || *param_1 == '\0') {
    while (g_token == 0) {
      g_token = random();
    }
    return fdprintf(1, "OK %08X\n", g_token) < 0xc;
  } else {
    return send_error(500);
  }
}

// Function: handle_auth
unsigned int handle_auth(char *param_1) {
  char *token_str = strsep(&param_1, " ");
  if (token_str == NULL) {
    return send_error(500);
  }
  
  unsigned long auth_token = strtoul(token_str, NULL, 0x10);
  if (auth_token == g_token) {
    g_auth = 1;
    return fdprintf(1, DAT_00014081) < 3;
  } else {
    return send_error(0x193); // 403 Forbidden
  }
}

// Function: handle_set
unsigned int handle_set(char *param_1) {
  if (g_auth == 0) {
    return send_error(0x193);
  }

  char *name_str = strsep(&param_1, " ");
  char *value_str = param_1;
  
  if (value_str == NULL || *value_str == '\0') {
    return send_error(500);
  }

  size_t name_len = strlen(name_str);
  size_t value_len = strlen(value_str);
  
  // Allocate space for Variable struct + name string + null + value string + null
  Variable *new_var = (Variable *)malloc(sizeof(Variable) + name_len + 1 + value_len + 1);
  if (new_var == NULL) {
    return send_error(500);
  }
  
  // Calculate pointers within the allocated block
  new_var->name = (char *)(new_var + 1);
  new_var->value = new_var->name + name_len + 1;
  
  strcpy(new_var->name, name_str);
  strcpy(new_var->value, value_str);
  
  new_var->next = variables;
  variables = new_var;

  return fdprintf(1, DAT_00014081) < 3;
}

// Function: handle_call
unsigned int handle_call(char *param_1) {
  if (g_auth == 0) {
    send_error(0x193);
    return 1; // Indicate error
  }

  char *function_name = strsep(&param_1, " ");
  if (function_name == NULL) {
    send_error(500);
    return 1; // Indicate error
  }

  const void **current_func_entry = functions;
  while (*current_func_entry != NULL) {
    if (strcmp((char *)*current_func_entry, function_name) == 0) {
      break;
    }
    current_func_entry += 4;
  }

  if (*current_func_entry == NULL) {
    send_error(0x194); // 404 Not Found
    return 1; // Indicate error
  } else {
    output_clear();
    ((function_handler_t)current_func_entry[2])(param_1);
    (*(unsigned int *)current_func_entry[3])++;
    return output_transmit(); // Return value of output_transmit
  }
}

// Function: page_home
undefined4 page_home(void) {
  output_append("Directory listing\n");
  const void **current_func_entry = functions;
  while (*current_func_entry != NULL) {
    output_append("\t%s (called %d times)\t\t%s\n",
                  (char *)*current_func_entry,
                  *(unsigned int *)current_func_entry[3],
                  (char *)current_func_entry[1]);
    current_func_entry += 4;
  }
  output_append(DAT_000140b3);
  return 0;
}

// Function: page_root64
undefined4 page_root64(void) {
  char *mode = variable_get((char *)DAT_000140b5);
  char *data = variable_get((char *)DAT_000140ba);

  if (mode == NULL || data == NULL) {
    send_error(500);
    return 1;
  }

  unsigned char temp_buffer[256];
  unsigned int output_len = 0;

  if (strcmp(mode, "encode") == 0) {
    size_t data_len = strlen(data);
    // Rough estimate for base64 output size (4/3 factor)
    if ((data_len / 3 + 1) * 4 > sizeof(temp_buffer)) {
      send_error(0x1f5); // 500 + 53 = 553 (Payload Too Large)
      return 1;
    }
    output_len = root64_encode(temp_buffer, data);
    output_write(temp_buffer, output_len);
  } else if (strcmp(mode, "decode") == 0) {
    size_t data_len = strlen(data);
    if (data_len > sizeof(temp_buffer)) { // Check if input would exceed buffer (for dummy copy)
      send_error(0x1f5);
      return 1;
    }
    output_len = root64_decode(temp_buffer, data);
    output_write(temp_buffer, output_len);
  } else {
    send_error(500);
    return 1;
  }
  return 0;
}

// Function: page_parcour
undefined4 page_parcour(void) {
  char *key_str = variable_get((char *)DAT_000140cd);
  char *data_str = variable_get((char *)DAT_000140ba);

  if (key_str == NULL || data_str == NULL) {
    send_error(500);
    return 1;
  }

  size_t key_len = strlen(key_str);
  if (key_len > 2) {
    key_len = 2;
  }
  
  parcour_init(key_str, key_len);
  
  size_t data_len = strlen(data_str);

  // Copy data_str to a mutable buffer as it's modified in-place
  char temp_data_buffer[1024]; // Assuming max data length, 0x400 in main
  if (data_len >= sizeof(temp_data_buffer)) {
      send_error(0x1f5); // Data too large for temp buffer
      return 1;
  }
  strcpy(temp_data_buffer, data_str);

  for (uint i = 0; i < data_len; ++i) {
    temp_data_buffer[i] ^= parcour_byte();
  }
  output_write(temp_data_buffer, data_len);

  // Re-initialize and perform XOR again (decryption), but don't transmit the result
  parcour_init(key_str, key_len);
  for (uint i = 0; i < data_len; ++i) {
    temp_data_buffer[i] ^= parcour_byte();
  }
  
  return 0;
}

// Function: main
undefined4 main(void) {
  bool exit_program = false;
  char line_buffer[1024];
  char *command_line_ptr;
  char *command_token;
  const void **command_entry;
  
  g_output_buf = (char *)malloc(0x10000);
  if (g_output_buf == NULL) {
      fprintf(stderr, "Failed to allocate output buffer.\n");
      return 1;
  }

  do {
    command_entry = NULL; // Reset for each iteration
    
    // Read command line
    if (readuntil(0, line_buffer, sizeof(line_buffer), '\n') == -1) {
      exit_program = true;
      break;
    }
    
    // Check for empty line
    size_t line_len = strlen(line_buffer);
    if (line_len == 0) {
      exit_program = true;
      break;
    }
    
    // Remove newline character
    if (line_len > 0 && line_buffer[line_len - 1] == '\n') {
      line_buffer[line_len - 1] = '\0';
    }
    
    command_line_ptr = line_buffer;
    command_token = strsep(&command_line_ptr, " ");

    // Check for "BYE" command or no command
    if (command_token == NULL || strcmp(command_token, "BYE") == 0) {
      exit_program = true;
      break;
    }

    // Find command handler
    command_entry = commands;
    while (*command_entry != NULL) {
      if (strcmp((char *)*command_entry, command_token) == 0) {
        break; // Command found
      }
      command_entry += 2;
    }

    if (*command_entry == NULL) { // Command not found
      if (send_error(400)) { // If send_error itself indicates an error
        exit_program = true;
      }
    } else {
      // Execute command handler; 0 means continue, non-zero means exit
      if (((command_handler_t)command_entry[1])(command_line_ptr) != 0) {
        exit_program = true;
      }
    }
  } while (!exit_program);

  fdprintf(1, DAT_00014073);
  free(g_output_buf);
  
  // Free variables linked list
  Variable *current = variables;
  while (current != NULL) {
      Variable *next = current->next;
      free(current);
      current = next;
  }

  return 0;
}