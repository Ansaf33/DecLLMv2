#include <stdio.h>      // For printf, getline, stdin, FILE, perror, feof
#include <stdlib.h>     // For malloc, free, size_t, ssize_t (indirectly via getline)
#include <string.h>     // For strlen
#include <stdbool.h>    // For bool, true
#include <sys/types.h>  // For ssize_t (explicitly, though often included by stdlib.h or stdio.h on Linux)
#include <stdint.h>     // For uintptr_t and uint32_t for explicit pointer handling

// Placeholder for logbook, assuming it's a global variable as implied by the original snippet.
void *logbook = NULL;

// Function: find_char
// Fixes: return type from undefined4 to int, loop condition (removes strlen from loop),
//        reduced intermediate variables.
int find_char(char search_char, char *str) {
  if (str == NULL) {
    return 0;
  }
  for (size_t i = 0; str[i] != '\0'; i++) {
    if (search_char == str[i]) {
      return 1;
    }
  }
  return 0;
}

// Function: process_menu
// Fixes: return type from undefined4 to int, parameters (void* for generic array, int for count),
//        input handling with getline, printf format specifiers,
//        accessing menu item members using raw pointer arithmetic (to preserve original 0xc stride),
//        removes 'goto' (not present but ensuring no new ones),
//        reduces intermediate variables, handles getline buffer.
int process_menu(void *menu_items_ptr, int num_items) {
  char *line_buffer = NULL; // Buffer for getline, allocated/reallocated by getline
  size_t buffer_size = 0;   // Current size of line_buffer
  ssize_t chars_read;       // Number of characters read by getline
  int i;                    // Loop counter

  // Outer loop for continuous menu display
  while (true) {
    printf("\n");
    // Loop to display menu items
    for (i = 0; i < num_items; i++) {
      // Accessing members using raw pointer arithmetic to match the original 0xc stride.
      // This assumes the menu_items_ptr points to an array where each item is 12 bytes.
      // The layout is assumed to be: char key (offset 0), char* label (offset 4), void(*func)(void*) (offset 8).
      // Note: On 64-bit systems, `char*` and `void(*)(void*)` are 8 bytes. Accessing them as 4-byte values
      // (implied by offset 4 and 8 within a 12-byte stride) might lead to truncation or incorrect behavior
      // unless the data itself was specifically prepared with 32-bit pointer values.
      // For this "fixed" code, we assume the data provided to `menu_items_ptr` conforms to this 12-byte layout.
      char current_key = *(char *)((char *)menu_items_ptr + i * 0xc);
      char *current_label = (char *)(uintptr_t)*(uint32_t *)((char *)menu_items_ptr + i * 0xc + 4);
      printf("%c - %s\n", current_key, current_label);
    }
    printf(":");

    // Read user input
    chars_read = getline(&line_buffer, &buffer_size, stdin);

    // Handle getline errors or EOF
    if (chars_read == -1) {
        if (feof(stdin)) { // Check for EOF
            printf("\nEnd of input detected. Exiting menu.\n");
        } else {
            perror("Error reading input");
        }
        if (line_buffer) { // Free buffer in case of error/EOF before exiting
            free(line_buffer);
        }
        return 0; // Exit the menu function on error/EOF
    }

    // Remove trailing newline character if present
    if (chars_read > 0 && line_buffer[chars_read - 1] == '\n') {
      line_buffer[chars_read - 1] = '\0';
      chars_read--;
    }

    // If input is empty after trimming newline, re-prompt
    if (chars_read == 0) {
      printf("\n");
      continue;
    }

    char input_char = line_buffer[0]; // Get the first character of the user's input

    printf("\n"); // Original code prints newline here before processing selection

    bool item_selected = false;
    // Loop through menu items to find a match
    for (i = 0; i < num_items; i++) {
      char *current_label = (char *)(uintptr_t)*(uint32_t *)((char *)menu_items_ptr + i * 0xc + 4);
      // Original logic: find input_char in the label string.
      if (find_char(input_char, current_label) != 0) {
        item_selected = true;
        void (*current_func)(void*) = (void (*)(void*))(uintptr_t)*(uint32_t *)((char *)menu_items_ptr + i * 0xc + 8);

        // Check if the function pointer is NULL (indicated by 0 in the 32-bit stored value)
        if (current_func == NULL) {
          free(line_buffer); // Free the buffer before exiting the function
          return 0; // Exit the menu function
        }
        // Call the associated function
        current_func(logbook);
        break; // Exit the for loop, go back to the main menu loop to re-display
      }
    }

    if (!item_selected) {
        printf("Invalid selection: '%c' (input: \"%s\")\n", input_char, line_buffer);
    }
    // line_buffer is managed by getline; it's freed only upon function exit.
  }
  // This point should only be reached if the loop condition `while(true)` is somehow broken,
  // or if the function exits via means other than `return 0;` within the loop.
  // For robustness, ensure line_buffer is freed.
  if (line_buffer) {
      free(line_buffer);
  }
  return 0;
}

// --- Dummy functions for a compilable example ---

void func_add_entry(void *data) {
    printf("Executing 'Add Entry' function. (logbook: %p)\n", data);
}

void func_view_entries(void *data) {
    printf("Executing 'View Entries' function. (logbook: %p)\n", data);
}

// Struct to represent a menu item conforming to the 12-byte stride (0xc)
// This structure explicitly defines 32-bit fields for pointer addresses to match
// the decompiled code's assumed memory layout, which typically comes from 32-bit binaries
// or systems with specific packing/pointer sizes.
// `__attribute__((packed))` ensures no compiler padding, making the size exactly 12 bytes.
typedef struct __attribute__((packed)) {
    char key;           // 1 byte
    char padding[3];    // 3 bytes padding for alignment (though packed might make it redundant)
    uint32_t label_ptr; // 4 bytes for string address (truncated on 64-bit if needed)
    uint32_t func_ptr;  // 4 bytes for function address (truncated on 64-bit if needed)
} MenuItem_12Byte; // Total size: 1 + 3 + 4 + 4 = 12 bytes.

int main() {
    // Initialize logbook (e.g., allocate memory for it)
    int dummy_logbook_data = 123;
    logbook = &dummy_logbook_data; // Point to some dummy data

    // Define menu items using the MenuItem_12Byte structure.
    // Pointers are explicitly cast to `uintptr_t` then `uint32_t` to match the 4-byte storage,
    // which may involve truncation on 64-bit systems. This is necessary to match the
    // decompiled code's memory access pattern (`0xc` stride and 4-byte pointer reads).
    MenuItem_12Byte menu_options_data[] = {
        {'a', {0}, (uint32_t)(uintptr_t)"Add new entry",    (uint32_t)(uintptr_t)func_add_entry},
        {'v', {0}, (uint32_t)(uintptr_t)"View all entries", (uint32_t)(uintptr_t)func_view_entries},
        {'x', {0}, (uint32_t)(uintptr_t)"Exit",             (uint32_t)(uintptr_t)NULL} // NULL func_ptr indicates exit
    };
    int num_options = sizeof(menu_options_data) / sizeof(MenuItem_12Byte);

    printf("Welcome to the Menu!\n");
    process_menu(menu_options_data, num_options);

    printf("Menu exited. Goodbye!\n");

    // No dynamic allocation for logbook in this example, so no free needed.
    logbook = NULL; // Reset global pointer

    return 0;
}