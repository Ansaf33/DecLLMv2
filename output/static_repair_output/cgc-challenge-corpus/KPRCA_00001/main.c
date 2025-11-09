#include <stdio.h>    // For printf, vfprintf, stdin, stdout, stderr
#include <stdlib.h>   // For malloc, free, strtoul, rand
#include <string.h>   // For memcpy, strlen, strsep, strcmp, strcpy, strncpy
#include <stdbool.h>  // For bool
#include <stdarg.h>   // For va_list, va_start, va_end

// Type definitions based on typical decompiler output
typedef void undefined;
typedef int undefined4; // Assuming 4-byte integers
typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned char byte;

// Forward declarations for custom functions
// Mock implementations are provided at the end for compilability.
int fdprintf(int fd, const char *format, ...);
int readuntil(int fd, char *buf, int max_len, char delimiter);
int writeall(int fd, const void *buf, size_t count);
size_t root64_encode(char *dest, const char *src);
size_t root64_decode(char *dest, const char *src);
void parcour_init(const char *key, size_t key_len);
byte parcour_byte(void);

// Global variables
uint g_output_len = 0;
char *g_output_buf = NULL; // Initialized in main
ulong g_token = 0;         // Initialized by handle_hello
int g_auth = 0;

// Structure for variable linked list
typedef struct Variable {
    struct Variable *next;
    char *name;  // Points to string after the struct
    char *value; // Points to string after the name
} Variable;

Variable *variables = NULL; // Head of the linked list

// Function pointer types for command and page handlers
typedef undefined4 (*CommandFunc)(char *);
typedef undefined4 (*PageFunc)(char *);

// Command table entry structure
typedef struct {
    const char *name;
    CommandFunc handler;
} CommandEntry;

// Function table entry structure (for pages)
typedef struct {
    const char *name;
    const char *description;
    PageFunc handler;
    int call_count;
} FunctionEntry;

// Forward declarations for handlers
// All handlers that process command line arguments take `char *` as per `main` logic.
undefined4 handle_hello(char *param_1);
undefined4 handle_auth(char *param_1);
undefined4 handle_set(char *param_1);
void handle_call(char *param_1);
undefined4 page_home(char *param_1);
undefined4 page_root64(char *param_1);
undefined4 page_parcour(char *param_1);

// Global command table
CommandEntry commands[] = {
    {"HELLO", handle_hello},
    {"AUTH", handle_auth},
    {"SET", handle_set},
    {"CALL", handle_call},
    {"BYE", NULL}, // BYE is handled as an exit condition, not a function call
    {NULL, NULL}   // Sentinel
};

// Global function table (pages)
FunctionEntry functions[] = {
    {"home", "Home page", page_home, 0},
    {"root64", "Base64 encode/decode", page_root64, 0},
    {"parcour", "XOR encryption/decryption", page_parcour, 0},
    {NULL, NULL, NULL, 0} // Sentinel
};

// String literals (data sections)
const char DAT_00014073[] = "Bye.\n";
const char DAT_00014081[] = "OK\n";
const char DAT_000140b3[] = "--------------------\n";
const char DAT_000140b5[] = "mode";
const char DAT_000140ba[] = "data";
const char DAT_000140cd[] = "key";

// Function: output_clear
void output_clear(void) {
  g_output_len = 0;
}

// Function: output_write
void output_write(void *param_1, uint param_2) {
  // Ensure we don't write beyond the buffer capacity (0x10000 bytes)
  if (0x10000U - g_output_len < param_2) {
    param_2 = 0x10000U - g_output_len;
  }
  if (param_2 > 0) {
    memcpy(g_output_buf + g_output_len, param_1, param_2);
    g_output_len += param_2;
  }
}

// Function: output_append (variadic)
void output_append(char *format, ...) {
  va_list args;
  va_start(args, format);
  // Calculate remaining space, leaving one byte for null terminator
  size_t remaining_space = 0x10000 - g_output_len;
  if (remaining_space == 0) { // No space left
      va_end(args);
      return;
  }
  int written_len = vsnprintf(g_output_buf + g_output_len, remaining_space, format, args);
  va_end(args);

  if (written_len > 0) {
    // vsnprintf returns the number of characters that would have been written
    // if the buffer had been large enough, *not* including the null terminator.
    // We update g_output_len by the actual number of characters written,
    // ensuring it doesn't exceed the buffer capacity.
    g_output_len += (uint)written_len;
    if (g_output_len >= 0x10000) {
        g_output_len = 0x10000 - 1; // Cap to max usable length, leave space for null
    }
  }
}

// Function: output_transmit
undefined4 output_transmit(void) {
  if (fdprintf(1, "OK %u\n", g_output_len) < 4) {
    return 1;
  }
  return (writeall(1, g_output_buf, g_output_len) == g_output_len) ? 0 : 1;
}

// Function: send_error
bool send_error(undefined4 param_1) {
  return fdprintf(1, "ERROR %3d\n", param_1) < 10;
}

// Function: handle_hello
undefined4 handle_hello(char *param_1) {
  // Check if any parameters were provided after "HELLO"
  if (param_1 != NULL && *param_1 != '\0') {
      return send_error(500); // Bad Request / Invalid Arguments
  }

  // Generate a non-zero token
  while (g_token == 0) {
      g_token = rand(); // Use rand() for simplicity. For better randomness, use srandom/random.
  }
  
  if (fdprintf(1, "OK %08lX\n", g_token) < 0xc) {
    return 1; // Error during output
  }
  return 0; // Success
}

// Function: handle_auth
undefined4 handle_auth(char *param_1) {
  char *auth_token_str = strsep(&param_1, " ");

  // Check if token string is missing or empty
  if (auth_token_str == NULL || *auth_token_str == '\0') {
    return send_error(500); // Bad Request / Missing Token
  }

  // Check for extraneous arguments
  if (param_1 != NULL && *param_1 != '\0') {
      return send_error(500); // Bad Request / Extraneous Arguments
  }

  ulong input_token = strtoul(auth_token_str, NULL, 16);

  if (input_token == g_token) {
    g_auth = 1;
    if (fdprintf(1, DAT_00014081) < 3) { // "OK\n"
      return 1; // Error during output
    }
    return 0; // Success
  }
  return send_error(0x193); // 403 Forbidden
}

// Function: handle_set
undefined4 handle_set(char *param_1) {
  if (g_auth == 0) {
    return send_error(0x193); // 403 Forbidden
  }

  char *var_name = strsep(&param_1, " ");
  char *var_value = param_1; // The rest of the string is the value

  // Check for missing name or value
  if (var_name == NULL || *var_name == '\0' || var_value == NULL || *var_value == '\0') {
    return send_error(500); // Bad Request / Missing Name or Value
  }

  size_t name_len = strlen(var_name);
  size_t value_len = strlen(var_value);

  // Allocate space for Variable struct + name string + null + value string + null
  Variable *new_var = (Variable *)malloc(sizeof(Variable) + name_len + 1 + value_len + 1);
  if (new_var == NULL) {
    return send_error(500); // Internal Server Error / Malloc failed
  }

  new_var->name = (char *)(new_var + 1);
  strcpy(new_var->name, var_name);

  new_var->value = new_var->name + name_len + 1;
  strcpy(new_var->value, var_value);

  // Add to the head of the linked list
  new_var->next = variables;
  variables = new_var;

  if (fdprintf(1, DAT_00014081) < 3) { // "OK\n"
    return 1; // Error during output
  }
  return 0; // Success
}

// Function: handle_call
void handle_call(char *param_1) {
  if (g_auth == 0) {
    send_error(0x193); // 403 Forbidden
    return;
  }

  char *func_name = strsep(&param_1, " ");

  if (func_name == NULL || *func_name == '\0') {
    send_error(500); // Bad Request / Missing Function Name
    return;
  }

  FunctionEntry *current_func = functions;
  while (current_func->name != NULL) {
    if (strcmp(current_func->name, func_name) == 0) {
      break;
    }
    current_func++;
  }

  if (current_func->name == NULL) {
    send_error(0x194); // 404 Not Found
  } else {
    output_clear();
    current_func->handler(param_1); // Pass remaining arguments to page handler
    current_func->call_count++;
    output_transmit();
  }
}

// Function: variable_get
char *variable_get(char *param_1) {
  Variable *current_var = variables;
  while (current_var != NULL) {
    if (strcmp(param_1, current_var->name) == 0) {
      return current_var->value;
    }
    current_var = current_var->next;
  }
  return NULL;
}

// Function: page_home
undefined4 page_home(char *param_1) {
    // Check for extraneous arguments
    if (param_1 != NULL && *param_1 != '\0') {
        send_error(500); // Bad Request / Extraneous Arguments
        return 1;
    }

    output_append("Directory listing\n");
    FunctionEntry *current_func = functions;
    while (current_func->name != NULL) {
        output_append("\t%s (called %d times)\t\t%s\n",
                      current_func->name,
                      current_func->call_count,
                      current_func->description);
        current_func++;
    }
    output_append(DAT_000140b3); // "--------------------\n"
    return 0;
}

// Function: page_root64
undefined4 page_root64(char *param_1) {
    // Check for extraneous arguments
    if (param_1 != NULL && *param_1 != '\0') {
        send_error(500); // Bad Request / Extraneous Arguments
        return 1;
    }

    char local_buf[256]; // Temporary buffer for encode/decode output

    char *mode = variable_get((char *)DAT_000140b5); // "mode"
    char *data = variable_get((char *)DAT_000140ba); // "data"

    if (mode == NULL || *mode == '\0' || data == NULL || *data == '\0') {
        send_error(500); // Internal Server Error / Missing variables
        return 1;
    }

    if (strcmp(mode, "encode") == 0) {
        size_t data_len = strlen(data);
        // Base64 output length for N bytes is ceil(N/3)*4.
        // For local_buf[256], max output is 255 chars + null.
        // Max input that fits (255/4)*3 = 63*3 = 189 bytes.
        // The original check `0x100 < (sVar2 / 3) * 4` implies input_len is limited such that output fits 256.
        if (data_len > 189) { // If input is too large, output will overflow 256 bytes
            send_error(0x1f5); // 501 Not Implemented (or too large)
            return 1;
        }
        size_t encoded_len = root64_encode(local_buf, data);
        output_write(local_buf, encoded_len);
    } else if (strcmp(mode, "decode") == 0) {
        size_t data_len = strlen(data);
        // For decode, output is typically smaller or equal to input.
        if (data_len >= sizeof(local_buf)) {
            send_error(0x1f5); // 501 Not Implemented (or too large)
            return 1;
        }
        size_t decoded_len = root64_decode(local_buf, data);
        output_write(local_buf, decoded_len);
    } else {
        send_error(500); // Internal Server Error / Invalid mode
        return 1;
    }
    return 0;
}

// Function: page_parcour
undefined4 page_parcour(char *param_1) {
    // Check for extraneous arguments
    if (param_1 != NULL && *param_1 != '\0') {
        send_error(500); // Bad Request / Extraneous Arguments
        return 1;
    }

    char *key = variable_get((char *)DAT_000140cd); // "key"
    char *data = variable_get((char *)DAT_000140ba); // "data"

    if (key == NULL || *key == '\0' || data == NULL || *data == '\0') {
        send_error(500); // Internal Server Error / Missing variables
        return 1;
    }

    size_t key_len = strlen(key);
    if (key_len > 2) { // Key length capped at 2
        key_len = 2;
    }

    parcour_init(key, key_len);

    size_t data_len = strlen(data);
    // Assuming `data` points to a mutable buffer (e.g., from malloc'd Variable struct)
    char *mutable_data = data;

    for (uint i = 0; i < data_len; i++) {
        mutable_data[i] ^= parcour_byte();
    }
    // The original code applies the cipher twice, effectively undoing it.
    // This will result in the original 'data' being written to output.
    parcour_init(key, key_len); // Re-initialize with the same key
    for (uint i = 0; i < data_len; i++) {
        mutable_data[i] ^= parcour_byte();
    }

    output_write(mutable_data, data_len);
    return 0;
}

// Function: main
undefined4 main(void) {
  char command_buffer[1024];
  char *cmd_line_ptr; // Pointer to current position in command_buffer for strsep
  char *command_name;
  int handler_result;

  g_output_buf = (char *)malloc(0x10000); // Allocate 64KB for output buffer
  if (g_output_buf == NULL) {
      fdprintf(2, "ERROR: Failed to allocate output buffer.\n");
      return 1;
  }
  // Optional: Seed random number generator for better g_token uniqueness
  // srand(time(NULL)); // Requires <time.h>

  while (true) {
    // Read a line from input, delimited by newline.
    // If readuntil returns 0, it means EOF or error, so break.
    if (readuntil(0, command_buffer, sizeof(command_buffer), '\n') == 0) {
      break; // Exit main loop on EOF/error
    }

    cmd_line_ptr = command_buffer;
    // If the line is empty after stripping newline (done by readuntil mock)
    if (strlen(command_buffer) == 0) {
      continue; // Process next command
    }

    // Extract the command name (e.g., "HELLO", "AUTH")
    command_name = strsep(&cmd_line_ptr, " ");

    // If command_name is NULL (e.g., empty line, or just spaces)
    // or if the command is "BYE", then exit.
    if (command_name == NULL || strcmp(command_name, "BYE") == 0) {
      break; // Exit main loop
    }

    // Lookup the command in the global commands table
    CommandEntry *current_command = commands;
    while (current_command->name != NULL) {
      if (strcmp(current_command->name, command_name) == 0) {
        break; // Command found
      }
      current_command++;
    }

    handler_result = 0; // Default to success

    if (current_command->name != NULL) { // Command found
      // Execute the associated handler function, passing the rest of the line as argument
      handler_result = current_command->handler(cmd_line_ptr);
    } else { // Command not found
      handler_result = send_error(400); // 400 Bad Request
    }

    if (handler_result != 0) { // If handler indicates an error or termination, break
      break; // Exit main loop
    }
  }

  fdprintf(1, DAT_00014073); // Print "Bye.\n"

  // Cleanup: Free allocated resources
  free(g_output_buf);
  // Free variables linked list
  Variable *current_var = variables;
  while (current_var != NULL) {
      Variable *next_var = current_var->next;
      free(current_var); // Free the block containing struct, name, and value
      current_var = next_var;
  }

  return 0;
}

// Mock implementations for custom functions to make the code compilable
// In a real scenario, these would be provided externally or implemented fully.

// Mock fdprintf: prints to stdout/stderr based on fd
int fdprintf(int fd, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vfprintf(fd == 1 ? stdout : (fd == 2 ? stderr : stdout), format, args);
    va_end(args);
    return ret;
}

// Mock readuntil: reads a line from stdin
int readuntil(int fd, char *buf, int max_len, char delimiter) {
    if (fd != 0) { // Only mock stdin (fd 0)
        return 0;
    }
    if (fgets(buf, max_len, stdin) == NULL) {
        return 0; // EOF or error
    }
    // Remove trailing delimiter (e.g., newline) if present
    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == delimiter) {
        buf[len - 1] = '\0';
        len--; // Adjust length
    }
    return len; // Return number of characters read (excluding null terminator)
}

// Mock writeall: writes to stdout
int writeall(int fd, const void *buf, size_t count) {
    if (fd != 1) { // Only mock stdout (fd 1)
        return 0;
    }
    return fwrite(buf, 1, count, stdout);
}

// Mock root64_encode: simple copy for demonstration
size_t root64_encode(char *dest, const char *src) {
    // In a real scenario, this would perform base64 encoding.
    // For now, just copy, ensuring dest is null-terminated and not overflowing.
    strncpy(dest, src, 255); // Max 255 chars for local_buf[256]
    dest[255] = '\0';
    return strlen(dest);
}

// Mock root64_decode: simple copy for demonstration
size_t root64_decode(char *dest, const char *src) {
    // In a real scenario, this would perform base64 decoding.
    // For now, just copy, ensuring dest is null-terminated and not overflowing.
    strncpy(dest, src, 255); // Max 255 chars for local_buf[256]
    dest[255] = '\0';
    return strlen(dest);
}

// Mock parcour_init
void parcour_init(const char *key, size_t key_len) {
    // Placeholder for actual initialization logic
    (void)key; // Suppress unused parameter warning
    (void)key_len;
}

// Mock parcour_byte
byte parcour_byte(void) {
    // Placeholder for actual byte generation logic
    return (byte)rand();
}