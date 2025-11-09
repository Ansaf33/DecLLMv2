#include <stdio.h>    // For exit()
#include <stdlib.h>   // For malloc(), free(), exit()
#include <string.h>   // For strnlen(), strncmp(), memcpy(), memset(), strlen()
#include <stdbool.h>  // For bool type
#include <stddef.h>   // For size_t

// --- Type Definitions (based on common decompilation output) ---
// Assuming undefined4 is typically an int or unsigned int
typedef int undefined4;
// Assuming undefined is a byte
typedef unsigned char undefined;

// For `uint` in executeProgram, using `unsigned int`

// --- External Function Declarations (placeholders) ---
// These functions are not provided in the snippet and need to be implemented or linked.
// Signatures are inferred from usage.

// From invalid_command, addLine, executeProgram
// transmit_all(fd, buffer, len)
int transmit_all(int fd, const char *buffer, size_t len);

// From receiveProgramSize, initProgram
// recvline(buffer, max_len)
// Returns bytes read, or < 1 on error.
int recvline(char *buffer, size_t max_len);

// From receiveProgramSize, executeProgram
// strn2int(str, max_len)
// Returns integer value.
int strn2int(const char *str, size_t max_len);

// From initProgram, executeProgram
// allocate(size, out_ptr)
// Returns 0 on success, non-zero on failure. `out_ptr` receives the allocated memory.
int allocate(size_t size, int **out_ptr);

// From initProgram
// addLine(program_ptr, line_data_buffer)
void addLine(int *program_ptr, const char *line_data);

// From executeProgram
// initStack(stack_handle, capacity_elements, element_size_bytes)
// Returns 0 on success.
int initStack(void *stack_handle, size_t capacity, size_t element_size);

// From executeProgram
// popElement(stack_handle, out_element_data)
// Returns 0 on success. `out_element_data` receives the popped element.
int popElement(void *stack_handle, void *out_element_data);

// From executeProgram
// pushElement(stack_handle, element_data)
// Returns 0 on success. `element_data` is the element to push.
int pushElement(void *stack_handle, const void *element_data);

// From executeProgram
// destroyStack(stack_handle)
// Returns 0 on success.
int destroyStack(void *stack_handle);

// From executeProgram
// itoaB10(value)
// Assumed to allocate a string on the heap and return a pointer to it.
char* itoaB10(long long value);

// From executeProgram
// deallocate(ptr, size)
// Returns 0 on success.
int deallocate(void *ptr, size_t size);

// --- Global Constant Data (placeholders) ---
// Inferred from DAT_000130xx usage. These would typically be defined in a .data section.
const char DAT_0001305e[] = "program_size"; // From receiveProgramSize
const char DAT_0001307f[] = "push";         // From executeProgram
const char DAT_00013084[] = "pop";          // From executeProgram
const char DAT_00013088[] = "add";          // From executeProgram
const char DAT_00013094[] = "dup";          // From executeProgram
const char DAT_00013092[] = "\n";           // From executeProgram (print newline)


// Function: invalid_command
void invalid_command(void) {
  if (transmit_all(1, "Invalid command!\n", 0x12) != 0) {
    exit(3);
  }
}

// Function: parseCmd
size_t parseCmd(const char *param_1, const char *param_2) {
  size_t len = strnlen(param_1, 0x10);
  if (strncmp(param_2, param_1, len) != 0) {
    len = 0;
  }
  return len;
}

// Function: receiveProgramSize
// param_1 is unused in the decompiled logic.
int receiveProgramSize(int unused_param) {
  char buffer[257 * sizeof(int)]; // Assuming undefined4 is int. Max 256 chars + null terminator.
                                 // Original code uses 0x100 undefined4.
  memset(buffer, 0, sizeof(buffer)); // Initialize buffer

  int bytes_received = recvline(buffer, sizeof(buffer) - 1); // Assuming recvline takes buffer and max_len
  if (bytes_received < 1) {
    exit(4);
  }
  buffer[bytes_received] = '\0'; // Ensure null termination for string operations

  // DAT_0001305e is likely the command string "program_size"
  // parseCmd(expected_command, received_string)
  size_t cmd_len = parseCmd(DAT_0001305e, buffer);

  if (cmd_len == 0) {
    invalid_command();
    exit(5);
  }

  // The integer to be parsed starts after the command.
  // The original code implies a max length of 8 for strn2int.
  return strn2int(buffer + cmd_len, 8);
}

// Function: initProgram
void initProgram(int **program_handle, int param_2_unused) {
  int program_size = receiveProgramSize(0); // param_2 is unused by receiveProgramSize

  if (program_size > 0 && program_size < 10000) {
    size_t allocation_size = (size_t)program_size * 0x10 + 0x14; // Each line is 0x10 bytes, plus 0x14 for header (3 ints)
    int *program_ptr;

    if (allocate(allocation_size, &program_ptr) != 0) {
      exit(6);
    }

    *program_handle = program_ptr; // Store the allocated pointer in the handle

    // Initialize program header
    program_ptr[0] = program_size;        // Total number of lines
    program_ptr[1] = program_size >> 0x1f; // Likely 0 for positive program_size
    program_ptr[2] = -1;                  // Initial state, line counter or status, -1 means not started or invalid.

    char line_buffer[0x10 + 1]; // Buffer for each line (16 bytes + null terminator)
    for (int i = 0; i < program_size; ++i) {
      memset(line_buffer, 0, sizeof(line_buffer)); // Clear buffer for each line

      int bytes_read = recvline(line_buffer, sizeof(line_buffer) - 1);
      if (bytes_read < 1) {
        exit(7);
      }
      line_buffer[bytes_read] = '\0'; // Ensure null termination

      addLine(program_ptr, line_buffer); // Add the received line to the program
    }
    program_ptr[2] = 0; // Program initialized, set line counter or status to 0.
  }
}

// Function: isProgramFull
bool isProgramFull(const int *program_ptr) {
  // program_ptr[2] holds the current line count (number of lines already added)
  // program_ptr[0] holds the maximum program size (capacity)
  return program_ptr[2] >= program_ptr[0];
}

// Function: addLine
void addLine(int *program_ptr, const char *line_data) {
  if (isProgramFull(program_ptr)) {
    if (transmit_all(1, "Too many lines in program!\n", 0x1c) != 0) {
      // If transmit_all fails, _terminate() is called in original.
      // If transmit_all succeeds, it just returns. This behavior might be
      // a bug in the original code, as it should probably exit if full.
      // For safety, we exit here.
    }
    exit(10);
  }

  program_ptr[2]++; // Increment the line counter (0-indexed index of the new line)
  int current_line_index = program_ptr[2];

  // Destination: (char*)program_ptr + (3 ints * sizeof(int)) + (current_line_index * 16 bytes)
  // Assuming int is 4 bytes, 3 ints = 12 bytes.
  memcpy((char*)program_ptr + (3 * sizeof(int)) + (current_line_index * 0x10), line_data, 0x10);
}

// Function: executeProgram
void executeProgram(unsigned int *program_ptr) {
  // Allocate memory for the stack. Assuming 8-byte elements.
  // Capacity: program_ptr[0] (total lines) * 2 (as each line is 16 bytes, and element is 8 bytes)
  void *stack_handle = malloc((size_t)program_ptr[0] * 2 * 8);
  if (!stack_handle) {
      exit(1); // Memory allocation error
  }

  // Initialize the stack. The '8' in original `initStack` call implies 8-byte elements.
  if (initStack(stack_handle, (size_t)program_ptr[0] * 2, 8) != 0) {
    free(stack_handle);
    exit(1); // Stack initialization failed
  }

  program_ptr[2] = 0; // Reset line counter for program execution

  // Loop through program lines
  while (program_ptr[2] < program_ptr[0]) { // while current line index < total lines
    // current_line_data_ptr points to the start of the current line's data.
    // program_ptr[0,1,2] are 3 unsigned ints (12 bytes). Lines start after that.
    // Each line is 16 bytes (0x10).
    const char *current_line_data_ptr = (const char*)program_ptr + (3 * sizeof(unsigned int)) + (program_ptr[2] * 0x10);

    size_t cmd_len;
    long long value1, value2; // Use long long for 8-byte stack elements
    char *string_to_process;

    // "push" command
    cmd_len = parseCmd(DAT_0001307f, current_line_data_ptr);
    if (cmd_len > 0) {
      value1 = strn2int(current_line_data_ptr + cmd_len, 8); // Value to push, max 8 chars
      if (pushElement(stack_handle, &value1) != 0) {
          exit(1); // Stack error
      }
    }
    // "pop" command
    else {
      cmd_len = parseCmd(DAT_00013084, current_line_data_ptr);
      if (cmd_len > 0) {
        if (popElement(stack_handle, &value1) != 0) { // Pop 8-byte value
            exit(1); // Stack error
        }
        // The original code just pops and discards.
      }
      // "add" command
      else {
        cmd_len = parseCmd(DAT_00013088, current_line_data_ptr);
        if (cmd_len > 0) {
          if (popElement(stack_handle, &value1) != 0) { // Pop first operand
              exit(1); // Stack error
          }
          if (popElement(stack_handle, &value2) != 0) { // Pop second operand
              exit(1); // Stack error
          }
          long long sum = value1 + value2;
          char* sum_str = itoaB10(sum); // Assuming itoaB10 allocates and returns char*
          if (pushElement(stack_handle, &sum_str) != 0) { // Push pointer to string
              exit(1); // Stack error
          }
          // The original code deallocates the string AFTER pushing it.
          // This implies `pushElement` makes a copy of the string, and the original can be freed.
          if (deallocate(sum_str, strlen(sum_str)) != 0) {
            exit(11);
          }
        }
        // "print" command
        else {
          cmd_len = parseCmd("print", current_line_data_ptr);
          if (cmd_len > 0) {
            if (popElement(stack_handle, &string_to_process) != 0) { // Pop string pointer
                exit(1); // Stack error
            }
            if (transmit_all(1, string_to_process, strnlen(string_to_process, 8)) != 0) { // Max 8 chars?
              exit(14);
            }
            if (transmit_all(1, DAT_00013092, strnlen(DAT_00013092, 8)) != 0) { // Print newline
              exit(15);
            }
            // Clear the string on the stack after printing.
            // This implies the popped string is mutable memory.
            memset(string_to_process, 0, strnlen(string_to_process, 8));
          }
          // "dup" command
          else {
            cmd_len = parseCmd(DAT_00013094, current_line_data_ptr);
            if (cmd_len > 0) {
              int count = strn2int(current_line_data_ptr + cmd_len, 8); // Number of times to duplicate
              if (count >= 1) { // Only duplicate if count is valid
                void *element_to_dup;
                if (popElement(stack_handle, &element_to_dup) != 0) { // Pop the element to duplicate
                    exit(1); // Stack error
                }
                for (int i = 0; i < count; ++i) {
                  if (pushElement(stack_handle, &element_to_dup) != 0) { // Push it back 'count' times
                      exit(1); // Stack error
                  }
                }
              }
            }
            // If no command matched, just move to the next line.
            // No explicit action, program_ptr[2] will be incremented below.
          }
        }
      }
    }
    program_ptr[2]++; // Move to the next line
  }

  destroyStack(stack_handle); // Clean up stack resources
  free(stack_handle); // Free the allocated memory for the stack itself
}