#include <stdio.h>    // For printf
#include <stdlib.h>   // For exit
#include <string.h>   // For memcpy, strlen, memcmp, strsep
#include <ctype.h>    // For toupper
#include <unistd.h>   // For read (standard replacement for custom 'receive')

// --- Assumed external functions (must be defined elsewhere or mocked for compilation) ---
// These declarations are based on their usage in the provided snippet.
extern void print_assigned_cells(void);
extern int strtrim(char *str, size_t max_len, int mode);
extern int clear_cell(char *cell_id);
extern int valid_cell_id(char *cell_id);
extern char* show_cell(char *cell_id, int repr_flag, char *buffer, size_t buffer_len);
extern int set_cell(char *cell_id, char *value, size_t max_len);
extern void init_sheet(void);

// --- Command definitions and lengths for parse_line ---
#define SHOW_CMD_STR "SHOW "
#define REPR_CMD_STR "REPR "
#define CLEAR_CMD_STR "CLEAR "
#define EXIT_CMD_STR "EXIT"
#define TABLE_CMD_STR "TABLE"

const size_t SHOW_CMD_LEN = sizeof(SHOW_CMD_STR) - 1;
const size_t REPR_CMD_LEN = sizeof(REPR_CMD_STR) - 1;
const size_t CLEAR_CMD_LEN = sizeof(CLEAR_CMD_STR) - 1;
const size_t EXIT_CMD_LEN = sizeof(EXIT_CMD_STR) - 1;
const size_t TABLE_CMD_LEN = sizeof(TABLE_CMD_STR) - 1;

// Helper function to convert first n characters of a string to uppercase
static void to_upper_n(char *str, size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (str[i] == '\0') break; // Don't go past null terminator
        str[i] = (char)toupper((unsigned char)str[i]);
    }
}

// Function: print_table
void print_table(void) {
  print_assigned_cells();
  return;
}

// Function: readline
// Reads a line from the given file descriptor `fd` into `buffer`, up to `max_len` characters.
// Null-terminates the string if a newline is found.
// Exits the program if read error or EOF occurs.
// Returns: 0 on successful read (newline found), 0xFFFFFFFF if buffer full without newline.
unsigned int readline(int fd, char *buffer, unsigned int max_len) {
  unsigned int i;
  ssize_t bytes_read;
  
  for (i = 0; i < max_len; i++) {
    bytes_read = read(fd, buffer + i, 1); // Read one byte
    if (bytes_read <= 0) { // Error or EOF
      exit(0); // Original code exits on error or EOF
    }
    if (buffer[i] == '\n') {
      buffer[i] = '\0'; // Null-terminate the string
      return 0; // Success, line read
    }
  }
  
  // If loop finishes, max_len characters were read without a newline.
  // This is considered a "bad input" condition as per the original logic.
  return 0xFFFFFFFF;
}

// Function: parse_line
// Parses a command line and executes the corresponding action.
// Returns: 0 for success, 1 for exit command, or various error codes (negative values).
unsigned int parse_line(char *input_line) {
  char command_prefix_buffer[32]; // Buffer to hold command prefixes for comparison
  int parse_result;
  int repr_mode = 0; // 0 for SHOW, 1 for REPR

  // Trim leading/trailing whitespace
  parse_result = strtrim(input_line, 0x200, 1);
  if (parse_result == -1) {
    return 0xFFFFFFFF; // Bad input due to trim error
  }

  int is_show_or_repr = 0;
  char *arg_start = input_line; // Pointer to the argument part of the command

  // Check for "SHOW "
  if (strlen(input_line) >= SHOW_CMD_LEN) {
    memcpy(command_prefix_buffer, input_line, SHOW_CMD_LEN);
    to_upper_n(command_prefix_buffer, SHOW_CMD_LEN);
    if (memcmp(command_prefix_buffer, SHOW_CMD_STR, SHOW_CMD_LEN) == 0) {
      is_show_or_repr = 1;
      repr_mode = 0;
      arg_start = input_line + SHOW_CMD_LEN;
    }
  }

  // Check for "REPR " (only if not already matched as SHOW)
  if (!is_show_or_repr && strlen(input_line) >= REPR_CMD_LEN) {
    memcpy(command_prefix_buffer, input_line, REPR_CMD_LEN);
    to_upper_n(command_prefix_buffer, REPR_CMD_LEN);
    if (memcmp(command_prefix_buffer, REPR_CMD_STR, REPR_CMD_LEN) == 0) {
      is_show_or_repr = 1;
      repr_mode = 1;
      arg_start = input_line + REPR_CMD_LEN;
    }
  }

  if (is_show_or_repr) {
    // Common logic for SHOW and REPR commands
    strtrim(arg_start, 0x200, 2); // Trim additional whitespace within the argument part
    
    // Check for "TABLE" argument
    if (strlen(arg_start) >= TABLE_CMD_LEN) {
      memcpy(command_prefix_buffer, arg_start, TABLE_CMD_LEN);
      to_upper_n(command_prefix_buffer, TABLE_CMD_LEN);
      if (memcmp(command_prefix_buffer, TABLE_CMD_STR, TABLE_CMD_LEN) == 0) {
        print_table();
        return 0; // Success
      }
    }
    
    // Assume it's a cell ID if not "TABLE"
    if (valid_cell_id(arg_start) == -1) {
      return 0xFFFFFFFE; // Error showing data, invalid cell ID
    }
    
    char cell_output_buffer[512]; // Buffer for cell output
    char *cell_output = show_cell(arg_start, repr_mode, cell_output_buffer, sizeof(cell_output_buffer));
    if (repr_mode == 0) {
      printf("Cell Value: %s\n", cell_output);
    } else {
      printf("Cell Repr: %s\n", cell_output);
    }
    return 0; // Success
  }

  // If not SHOW or REPR, check for other commands

  // Check for "CLEAR "
  if (strlen(input_line) >= CLEAR_CMD_LEN) {
    memcpy(command_prefix_buffer, input_line, CLEAR_CMD_LEN);
    to_upper_n(command_prefix_buffer, CLEAR_CMD_LEN);
    if (memcmp(command_prefix_buffer, CLEAR_CMD_STR, CLEAR_CMD_LEN) == 0) {
      char *arg_ptr = input_line + CLEAR_CMD_LEN;
      parse_result = clear_cell(arg_ptr);
      if (parse_result != 0) {
        return 0xFFFFFFF8; // Error clearing cell
      }
      return 0; // Success
    }
  }

  // Check for "EXIT" (the original DAT_000171b3 likely refers to "EXIT")
  if (strlen(input_line) >= EXIT_CMD_LEN) {
    memcpy(command_prefix_buffer, input_line, EXIT_CMD_LEN);
    to_upper_n(command_prefix_buffer, EXIT_CMD_LEN);
    if (memcmp(command_prefix_buffer, EXIT_CMD_STR, EXIT_CMD_LEN) == 0) {
      return 1; // Exit command
    }
  }

  // If none of the above commands matched, try assignment (CELL=VALUE)
  char *cell_id_part = input_line; // strsep modifies the pointer, so use a temporary one
  char *value_part = strsep(&cell_id_part, "="); // After strsep, cell_id_part points to the value part
  
  // Check if both parts exist after splitting by '='
  if (value_part != NULL && cell_id_part != NULL) {
    parse_result = set_cell(value_part, cell_id_part, 0x200); // 0x200 is max_len for value
    if (parse_result != 0) {
      return 0xFFFFFFFC; // Error assigning cell
    }
    return 0; // Success
  }
  
  // If no known command matched and it's not a valid assignment
  return 0xFFFFFFFF; // Bad input
}

// Function: main
unsigned int main(void) {
  unsigned int parse_status;
  char line_buffer[512]; // Buffer for reading input line
  int readline_status;
  int exit_flag = 0; // Flag to control main loop exit
  
  init_sheet(); // Initialize the spreadsheet environment

  do {
    printf("Accel:-$ ");
    readline_status = readline(STDIN_FILENO, line_buffer, sizeof(line_buffer)); // Use STDIN_FILENO for stdin
    
    if (readline_status == 0) { // Readline successful
      parse_status = parse_line(line_buffer);
      switch(parse_status) {
      case 0:
        printf("Success.\n");
        break;
      case 1:
        exit_flag = 1; // Set flag to exit loop
        printf("Exiting...\n");
        // Returning 0 here immediately exits, so the final printf is skipped.
        // If the intention is to print "Unsupported signal. Exiting..." then
        // the return 0 should be at the end of main.
        // Based on "Exiting...", it seems a clean exit is intended.
        return 0; 
      case 0xFFFFFFF8:
        printf("Error clearing cell\n");
        break;
      case 0xFFFFFFFC:
        printf("Error assigning cell. Valid Cells: A0-ZZ99\n");
        break;
      case 0xFFFFFFFE:
        printf("Error showing data. Try SHOW TABLE or SHOW [A0-ZZ99]\n");
        break;
      case 0xFFFFFFFF: // Default case from original was `0xffffffff` for "Bad input"
      default: // Catch any other unexpected return values
        printf("Bad input\n"); // Original had "Unknown Input" for default, and "Bad input" for 0xffffffff
      }
    }
    else { // Readline failed (e.g., buffer full without newline)
      printf("\n"); // Original code printed a newline on readline failure
      printf("Bad input\n"); // Add explicit "Bad input" message
    }
  } while (exit_flag == 0);

  // This part of the code would only be reached if exit_flag was set to 1
  // but the main loop didn't return immediately, e.g., if parse_status was 1
  // and exit_flag was set, then the loop condition `exit_flag == 0` would fail.
  // Given the `return 0;` inside the switch for `case 1:`, this line is unreachable.
  // If the intent was for a signal to reach here, the original code had an "Unsupported signal. Exiting..."
  // This implies the `do-while` loop might have been intended to break out due to a signal,
  // but the current C code only breaks on `exit_flag = 1` or `exit(0)`.
  // For standard C, the main loop should just terminate.
  // I will leave it as unreachable for now, reflecting the immediate return for `case 1`.
  // If a signal handling mechanism was truly intended, it would need `signal.h`.
  printf("Unsupported signal. Exiting...\n"); 
  return 0;
}