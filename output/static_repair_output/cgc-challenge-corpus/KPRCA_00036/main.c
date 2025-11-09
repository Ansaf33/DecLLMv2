#include <stdio.h>    // For standard I/O, typically where dprintf-like functions are declared if available
#include <stdlib.h>   // For malloc, free, exit, size_t
#include <string.h>   // For strlen, memcpy, memset
#include <stdbool.h>  // For bool type
#include <unistd.h>   // For ssize_t

// --- External Function Prototypes (Placeholders) ---
// These functions are assumed to be defined elsewhere and linked.
// Their exact implementation is not part of this task.
// `fdprintf` is like `dprintf` (a GNU extension) or a custom printf to a file descriptor.
int fdprintf(int fd, const char *format, ...);

// `receive` function based on original usage: returns 0 on success, non-zero on error.
// `bytes_read` pointer stores the actual number of bytes read (0 for EOF).
ssize_t receive(int fd, void *buf, size_t count, int *bytes_read);

// Other external functions used by the provided code snippets
void init_trex(void);
int retorpn(const char *search_str, size_t len, int *rpn_handle);
int evalrpn(int rpn_handle);
void debug_state(int state);
void match(int rpn_handle, const char *text, int match_mode);

// --- Function Declarations (for functions defined in this file) ---
size_t resize_buf(void **buffer_ptr, unsigned int current_size);
unsigned int readopt(int fd);
unsigned int readtrex(int fd, char *buffer, unsigned int max_len);
int create_text(int fd, char **text_buf);
void print_menu(int line_matching_mode, char *text_content, char *search_string);
void test(void); // Declared but not called from main in the provided snippet
int main(void);

// --- Function Definitions ---

// Function: resize_buf
size_t resize_buf(void **buffer_ptr, unsigned int current_size) {
  // If current_size is too large, return 0 and do not resize
  if (current_size >= 250001 /* 0x3d091 */) {
    return 0;
  }

  size_t new_size = (size_t)current_size * 2;
  void *new_buf = malloc(new_size);

  if (new_buf == NULL) {
    // Malloc failed. Original code implicitly allowed *buffer_ptr to become NULL.
    // This revised version returns 0 to indicate failure and leaves *buffer_ptr untouched.
    return 0; // Indicate failure to resize
  }

  if (*buffer_ptr != NULL) {
    memcpy(new_buf, *buffer_ptr, current_size); // Copy original data
    free(*buffer_ptr); // Free old buffer
  }
  *buffer_ptr = new_buf; // Update pointer to new buffer
  return new_size;
}

// Function: readopt
unsigned int readopt(int fd) {
  char c1;
  char c2;
  int bytes_read;

  // Read first character
  if (receive(fd, &c1, 1, &bytes_read) != 0 || bytes_read == 0) {
    return 0xffffffff; // Error or EOF
  }

  // Read second character
  if (receive(fd, &c2, 1, &bytes_read) != 0 || bytes_read == 0) {
    return 0xffffffff; // Error or EOF
  }

  if (c2 == '\n') {
    return (unsigned int)c1; // Return the first character if the second is newline
  } else {
    // If the second character is not a newline, discard the rest of the line.
    // The original code *always* returns 0xffffffff in this scenario,
    // regardless of whether a newline is eventually found while discarding.
    while (c2 != '\n') {
      if (receive(fd, &c2, 1, &bytes_read) != 0 || bytes_read == 0) {
        break; // Error or EOF while discarding
      }
    }
    return 0xffffffff; // Indicate failure (second char was not newline)
  }
}

// Function: readtrex
unsigned int readtrex(int fd, char *buffer, unsigned int max_len) {
  int bytes_read;
  unsigned int i;

  for (i = 0; i < max_len; ++i) {
    if (receive(fd, buffer + i, 1, &bytes_read) != 0 || bytes_read == 0) {
      return 0xffffffff; // Error or EOF
    }
    if (buffer[i] == '\n') {
      break; // Newline found
    }
  }

  if (i < max_len && buffer[i] == '\n') {
    buffer[i] = '\0'; // Null-terminate if newline was read within max_len
    return i; // Return length without the newline
  } else {
    // If loop finished because max_len was reached or error/EOF,
    // and no newline was found within max_len.
    // Discard remaining characters until newline or error/EOF.
    char discard_char = '\0'; // Placeholder to read discarded characters
    while (discard_char != '\n') {
      if (receive(fd, &discard_char, 1, &bytes_read) != 0 || bytes_read == 0) {
        break; // Error or EOF while discarding
      }
    }
    return 0xffffffff; // Indicate failure (buffer overflow or error)
  }
}

// Function: create_text
int create_text(int fd, char **text_buf) {
  size_t bytes_received = 0;
  size_t current_buf_size = 16; // Initial buffer size (0x10)
  char *newline_matcher = "\n\n\n"; // Sequence to match for termination
  int result_len = 0; // The length of the text if successful, 0 otherwise

  // Initial buffer allocation
  current_buf_size = resize_buf((void**)text_buf, 16);
  if (current_buf_size == 0) { // Failed initial allocation
      *text_buf = NULL;
      return 0; // Indicate failure
  }

  int bytes_read;
  bool operation_success = true; // Flag to track overall success

  while (operation_success && current_buf_size != 0) {
    char *current_ptr = *text_buf; // Pointer to the current buffer

    // Read one byte into the buffer
    if (receive(fd, current_ptr + bytes_received, 1, &bytes_read) != 0 || bytes_read == 0) {
      operation_success = false;
      break; // Error or EOF during receive
    }

    // Check for the termination newline sequence "\n\n\n"
    if (*(current_ptr + bytes_received) == *newline_matcher) {
      newline_matcher++;
      if (*newline_matcher == '\0') { // Full sequence matched
        bytes_received++; // Account for the last character read
        break; // Termination sequence found, exit loop
      }
    } else {
      newline_matcher = "\n\n\n"; // Reset matcher if sequence is broken
    }

    bytes_received++;

    // Check if buffer needs to be resized
    if (bytes_received == current_buf_size) {
      current_buf_size = resize_buf((void**)text_buf, current_buf_size);
      if (current_buf_size == 0) { // Resize failed
        operation_success = false;
        break;
      }
    }
  }

  // Final processing based on success flag and loop termination reason
  if (!operation_success || bytes_received < 3 || current_buf_size == 0) {
    // If any failure occurred (receive error, resize error, or text too short to terminate)
    result_len = 0; // Indicate failure
    if (*text_buf != NULL) { // If memory was allocated, free it
      free(*text_buf);
      *text_buf = NULL; // Ensure pointer is NULL
    }
  } else {
    // Success case: termination sequence found and text is long enough
    // Null-terminate the string, effectively removing the last two newlines of "\n\n\n"
    (*text_buf)[bytes_received - 2] = '\0';
    result_len = bytes_received - 2;
  }
  return result_len;
}

// Function: print_menu
void print_menu(int line_matching_mode, char *text_content, char *search_string) {
  fdprintf(1, "Text Searcher: Main Menu\n");
  fdprintf(1, "1. Enter Text\n");
  fdprintf(1, "2. Enter Search String\n");
  fdprintf(1, "3. Line Search - Partial Matches\n");
  fdprintf(1, "4. Line Search - Exact Matches\n");
  fdprintf(1, "5. Print Text\n");
  fdprintf(1, "6. Print Search String\n");
  fdprintf(1, "7. Search Text\n");
  fdprintf(1, "8. Quit\n\n");

  if (text_content != NULL) {
    size_t len = strlen(text_content);
    if (len < 16 /* 0x10 */) {
      fdprintf(1, "[Text=%s]\n", text_content);
    } else {
      char original_char = text_content[15]; // Save char at index 15
      text_content[15] = '\0'; // Temporarily null-terminate
      fdprintf(1, "[Text=%s...]\n", text_content);
      text_content[15] = original_char; // Restore original char
    }
  }

  // Check if search_string is not empty
  size_t search_len = strlen(search_string);
  if (search_len != 0) {
    if (search_len < 16 /* 0x10 */) {
      fdprintf(1, "[SearchString=%s]\n", search_string);
    } else {
      char original_char = search_string[15]; // Save char at index 15
      search_string[15] = '\0'; // Temporarily null-terminate
      fdprintf(1, "[SearchString=%s...]\n", search_string);
      search_string[15] = original_char; // Restore original char
    }
  }

  if (line_matching_mode == 0) {
    fdprintf(1, "[LineMatching=Partial]\n");
  }
  if (line_matching_mode == 1) {
    fdprintf(1, "[LineMatching=Exact]\n");
  }
  fdprintf(1, "Select: ");
}

// Function: test (Included as per request, but not called from main in original snippet)
void test(void) {
  int rpn_expr_handle = 0;
  char *search_string = "a";
  char *test_string = "aaaaaaaa";
  int ret_val;
  int eval_result = 0;

  // The original code calls retorpn twice with the same arguments. This is preserved.
  ret_val = retorpn(search_string, strlen(search_string) + 1, &rpn_expr_handle);
  ret_val = retorpn(search_string, strlen(search_string) + 1, &rpn_expr_handle);

  if (ret_val < 0) {
    fdprintf(1, "Invalid search string\n");
  } else {
    eval_result = evalrpn(rpn_expr_handle);
  }

  if (eval_result == 0) {
    fdprintf(1, "Bad rpn\n");
  }
  debug_state(eval_result);
  fdprintf(1, "trex = %s\n", search_string);
  fdprintf(1, "rpn = %d\n", rpn_expr_handle); // rpn_expr_handle is an integer handle, not a string
  fdprintf(1, "Test string = %s\n", test_string);
  fdprintf(1, "Partial Matching:\n");
  match(rpn_expr_handle, test_string, 0);
  fdprintf(1, "Full Line Matching:\n");
  match(rpn_expr_handle, test_string, 1);

  exit(0); // WARNING: Subroutine does not return
}

// Function: main
int main(void) {
  char menu_selection;
  char search_buffer[500]; // Buffer for search string
  char *text_buffer = NULL; // Dynamically allocated buffer for text
  int rpn_handle_for_search = 0; // Handle for the RPN expression
  int retorpn_result; // Result from retorpn function
  int evalrpn_result; // Result from evalrpn function
  int text_len = 0; // Length of the entered text
  int search_string_len = 0; // Length of the entered search string
  int line_matching_mode = 0; // 0 for partial, 1 for exact

  // Initialize search_buffer to all zeros (similar to original decompiler output)
  memset(search_buffer, 0, sizeof(search_buffer));

  init_trex(); // Initialize external library/state

  do {
    fdprintf(1, "\n"); // Print a newline before the menu, matching DAT_000151c0 usage
    print_menu(line_matching_mode, text_buffer, search_buffer);
    menu_selection = (char)readopt(0); // Read option from stdin (fd 0)

    switch(menu_selection) {
    case '1':
      fdprintf(1, "Enter text to search\n");
      if (text_buffer != NULL) {
        free(text_buffer);
        text_buffer = NULL;
      }
      text_len = create_text(0, &text_buffer); // Read text from stdin (fd 0)
      break;
    case '2':
      fdprintf(1, "Enter search string\n");
      // Read search string from stdin (fd 0) into search_buffer
      search_string_len = readtrex(0, search_buffer, sizeof(search_buffer));
      if (search_string_len < 0) { // readtrex returns 0xffffffff on error
        search_string_len = 0;
        search_buffer[0] = '\0'; // Ensure buffer is an empty string
      }
      break;
    case '3':
      line_matching_mode = 0; // Set to Partial Matches
      break;
    case '4':
      line_matching_mode = 1; // Set to Exact Matches
      break;
    case '5':
      fdprintf(1, "--Text--\n");
      if (text_buffer == NULL || text_len == 0) { // Check if text is available
        fdprintf(1, "\n");
      } else {
        fdprintf(1, "%s\n", text_buffer);
      }
      break;
    case '6':
      fdprintf(1, "--Search String--\n");
      if (search_string_len == 0 || search_buffer[0] == '\0') { // Check if search string is available
        fdprintf(1, "\n");
      } else {
        fdprintf(1, "%s\n", search_buffer);
      }
      break;
    case '7':
      if (text_len == 0) {
        fdprintf(1, "Input text has not been received\n");
      } else if (search_string_len == 0) {
        fdprintf(1, "Search text has not been received\n");
      } else {
        // Convert search string to RPN (Reverse Polish Notation)
        retorpn_result = retorpn(search_buffer, (size_t)search_string_len + 1, &rpn_handle_for_search);
        if (retorpn_result < 0) {
          fdprintf(1, "Invalid search string\n");
        } else {
          evalrpn_result = evalrpn(rpn_handle_for_search);
          if (evalrpn_result == 0) { // evalrpn returns 0 on error
            fdprintf(1, "Bad rpn\n");
          } else {
            match(evalrpn_result, text_buffer, line_matching_mode);
          }
        }
      }
      break;
    case '8':
      break; // Exit the loop
    default:
      fdprintf(1, "Bad Selection\n");
    }
  } while (menu_selection != '8');

  fdprintf(1, "Exiting...\n");

  // Free any dynamically allocated text buffer
  if (text_buffer != NULL) {
      free(text_buffer);
  }

  return 0;
}