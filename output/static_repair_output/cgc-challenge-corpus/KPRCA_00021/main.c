#define _GNU_SOURCE // For vdprintf

#include <stdio.h>   // For dprintf, vdprintf, NULL, STDIN_FILENO, STDOUT_FILENO
#include <stdlib.h>  // For malloc, free, exit, strtol
#include <string.h>  // For strcpy, memset, strncpy, strlen
#include <unistd.h>  // For read
#include <stdarg.h>  // For va_list in fdprintf mock
#include <stdint.h>  // For uintptr_t

// --- Type Definitions ---
typedef unsigned int uint;
typedef unsigned int undefined4; // Assuming 4-byte unsigned int

// Function pointer types for parser
typedef void *(*kty_create_func)(void *);
typedef void (*kty_print_func)(void *);

// --- Global Variables ---
void *my_kty = NULL;
void **parser = NULL; // Will be malloc'd in main

// --- Mock Function Declarations ---
// Mock for the original 'receive' function.
// Original: `iVar3 = receive(param_1,local_14,1,&local_18);`
// Original: `if ((iVar3 != 0) || (local_18 == 0)) { return -1; }`
// This means: if receive returns non-zero (error) OR if it returns 0 bytes.
// Our mock will return 0 on success, -1 on error, and populate bytes_received.
ssize_t receive(int fd, char *buf, size_t count, int *bytes_received);

// Mock for fdprintf (assuming dprintf to a file descriptor)
int fdprintf(int fd, const char *format, ...);

// Mock for free_kty_item
void free_kty_item(void *kty_item);

// Mock KTY functions used by the parser
void kty_print_mock(void *kty_item);
void *kty_create_mock(void *data);

// Mock for kty_init
int kty_init(void **parser_ptr);

// Mock array functions used in nyan
// Use packed to ensure 32-bit offsets (4 for string, 8 for length) work on 64-bit systems.
struct NyanArrayItem {
    int dummy_0;
    char *string;
    int length;
} __attribute__((packed));

int array_length(void *arr);
void *array_get(void *arr, int index);

// --- Provided Function Definitions (Fixed/Refactored) ---

// Function: read_until
// Reads bytes from a file descriptor until a delimiter is found, max_len is reached, or an error occurs.
// fd: file descriptor to read from (e.g., STDIN_FILENO)
// buffer: buffer to store the read data
// max_len: maximum number of bytes to read into the buffer (excluding null terminator)
// delimiter: delimiter character to stop reading at. If 0, reads until null byte.
// Returns: The total number of characters read into the buffer (including the delimiter, which is then overwritten by '\0'), or -1 on error.
int read_until(int fd, char *buffer, uint max_len, char delimiter) {
    char *current_pos = buffer;
    uint i;
    for (i = 0; i < max_len; ++i) {
        int bytes_read_count;
        ssize_t result = receive(fd, current_pos, 1, &bytes_read_count);

        if (result != 0 || bytes_read_count == 0) {
            return -1; // Error from receive, or no bytes read (EOF/error state)
        }

        char received_char = *current_pos;
        current_pos++;

        if (delimiter == received_char) {
            break;
        }
    }
    
    // Null-terminate the string.
    // current_pos now points one byte *after* the last character read.
    // current_pos[-1] is the last character read (either delimiter or max_len-th char).
    // This overwrites the delimiter if found, or the last char if max_len reached.
    current_pos[-1] = '\0';

    // The original code returned `(int)local_14 - (int)param_2`.
    // `local_14` was `current_pos`, `param_2` was `buffer`.
    // So it returns the total number of bytes read *including* the character that was overwritten by '\0'.
    return (int)(current_pos - buffer);
}

// Function: import_kty
void import_kty(void *data_param) { // Changed signature from undefined4 to void* for 64-bit compatibility
  if (my_kty != NULL) {
    free_kty_item(my_kty);
    my_kty = NULL;
  }
  
  if (parser != NULL) {
      *(int*)parser[0] = 0; // Reset parser[0] (used as a counter/flag)
      my_kty = (*(kty_create_func *)&parser[3])(data_param);
  } else {
      my_kty = NULL; // Should not happen if kty_init works
  }

  if (my_kty == NULL) {
    fdprintf(STDOUT_FILENO,"Error!\n");
  }
}

// Function: print_kty
void print_kty(void) {
  if (my_kty == NULL) {
    fdprintf(STDOUT_FILENO,"Error!\n");
  }
  else {
    if (parser != NULL) {
        *(int*)parser[0] = 0; // Reset parser[0]
        (*(kty_print_func *)&parser[2])(my_kty);
    } else {
        fdprintf(STDOUT_FILENO,"Error: Parser not initialized!\n");
    }
  }
}

// Nyan Cat ASCII art (moved outside nyan function)
const char NyanCatArt[] =
    "\n\n+      o     +              o\n"
    "    +             o     +       +\n"
    "o          +\n"
    "    o  +           +        +\n"
    "+        o     o       +        o\n"
    "-_-_-_-_-_-_-_,------,      o\n"
    "_-_-_-_-_-_-_-|   /\\_/\\\n"
    "-_-_-_-_-_-_-~|__( ^ .^)  +     +\n"
    "_-_-_-_-_-_-_-\"\"  \"\"\n"
    "+      o         o   +       o\n"
    "    +         +\n"
    "o        o         o      o     +\n"
    "    o           +\n"
    "+      +     o        o      +\n\n";

// Function: nyan
void nyan(void) {
  char nyan_art_buffer[384]; // Corresponds to local_5a8 (96 * sizeof(undefined4))
  char message_buffer[1024]; // Corresponds to local_428 (256 * sizeof(undefined4))
  
  // Initialize message_buffer to zeros
  memset(message_buffer, 0, sizeof(message_buffer));

  // Copy Nyan Cat art
  strncpy(nyan_art_buffer, NyanCatArt, sizeof(nyan_art_buffer) - 1);
  nyan_art_buffer[sizeof(nyan_art_buffer) - 1] = '\0'; // Ensure null-termination

  if (my_kty == NULL) {
    fdprintf(STDOUT_FILENO,"Error!\n");
  }
  else {
    if (parser != NULL) {
        (*(kty_print_func *)&parser[2])(my_kty); // Call print_kty function
        
        // Check parser[0] value (used as *parser)
        if (*(int*)parser[0] > 2) {
            fdprintf(STDOUT_FILENO,"%s", nyan_art_buffer); // Print Nyan Cat art
            
            char *current_dest = message_buffer;
            int k = 0;
            while(k < array_length(parser[1])) {
                void *item_ptr = array_get(parser[1], k);
                if (item_ptr == NULL) break; // Safety check

                // Access string and length using byte offsets, assuming packed struct or 32-bit layout
                char *src_string = *(char **)((char *)item_ptr + 4);
                int advance_len = *(int *)((char *)item_ptr + 8); // Amount to advance current_dest

                // Check for buffer overflow before strcpy and advancing
                size_t current_len = current_dest - message_buffer;
                size_t str_len = strlen(src_string);

                // Ensure enough space for the string + null terminator
                if (current_len + str_len + 1 > sizeof(message_buffer)) {
                    break; // Not enough space, stop copying
                }
                
                strcpy(current_dest, src_string); // Original uses strcpy, assumes src_string is null-terminated

                // Advance by `advance_len` from the struct, not `strlen`.
                // This means `advance_len` might represent a fixed slot size.
                if (current_len + advance_len > sizeof(message_buffer)) {
                    // Prevent advancing past buffer boundary if advance_len is too large
                    current_dest = message_buffer + sizeof(message_buffer) -1;
                } else {
                    current_dest += advance_len;
                }
                k++;
            }
            fdprintf(STDOUT_FILENO,"NYAN SAYS...\n\"\n%s\n\"", message_buffer);
        }
    } else {
        fdprintf(STDOUT_FILENO,"Error: Parser not initialized!\n");
    }
  }
}

// Function: quit
void quit(void) {
  fdprintf(STDOUT_FILENO,"\n\n=^.^=// Bye!\n\n");
  exit(0); // WARNING: Subroutine does not return
}

// Function: menu
void menu(void) {
  fdprintf(STDOUT_FILENO,"=======================\n");
  fdprintf(STDOUT_FILENO," 1. Import KTY\n");
  fdprintf(STDOUT_FILENO," 2. Print KTY\n");
  fdprintf(STDOUT_FILENO," 3. Quit\n");
  fdprintf(STDOUT_FILENO,"=======================\n");
}

// Function: main
int main(void) {
  // Initialize global pointers
  parser = NULL; 
  my_kty = NULL;

  fdprintf(STDOUT_FILENO,"KTY Pretty Printer v0.1\n");
  
  // Allocate space for at least 4 void pointers, as parser[3] is accessed.
  // Original `malloc(0x10)` implies 4 pointers on a 32-bit system.
  // Using `4 * sizeof(void*)` ensures enough space on both 32-bit and 64-bit.
  parser = (void**)malloc(4 * sizeof(void*));
  if (parser == NULL) {
      fdprintf(STDOUT_FILENO, "Error: Failed to allocate parser.\n");
      exit(1);
  }

  if (kty_init(parser) != 0) {
    fdprintf(STDOUT_FILENO,"Error: KTY initialization failed!\n");
    quit(); // Exits the program
  }
  
  menu();

  while(1) { // Main menu loop
    char input_buffer[16]; // Corresponds to local_2024
    long choice;
    
    // Read user input for menu choice, terminated by newline
    int bytes_read = read_until(STDIN_FILENO, input_buffer, sizeof(input_buffer) - 1, '\n');
    
    if (bytes_read < 1) { // Error or EOF (e.g., Ctrl+D)
      fdprintf(STDOUT_FILENO, "\nExiting due to input error or EOF.\n");
      break; // Exit loop, then free resources and return 0
    }
    
    // Convert input string to long integer
    choice = strtol(input_buffer, NULL, 10);

    if (choice == 777) { // 0x309 is 777 decimal
      nyan();
      continue; // Continue to the next menu loop iteration
    }

    switch (choice) {
      case 1: // Import KTY
        {
          char kty_data_buffer[8192]; // Corresponds to local_2014
          // Read KTY data, terminated by a null byte (0)
          // This implies the KTY data is expected to be null-terminated from input source.
          int kty_bytes = read_until(STDIN_FILENO, kty_data_buffer, sizeof(kty_data_buffer) - 1, '\0');
          if (kty_bytes < 1) {
            fdprintf(STDOUT_FILENO,"Error: Failed to read KTY data!\n");
          } else {
            import_kty(kty_data_buffer); // Pass char* directly
          }
        }
        break;
      case 2: // Print KTY
        print_kty();
        break;
      case 3: // Quit
        quit(); // This function calls exit(0) and does not return.
      default: // Invalid menu choice
        fdprintf(STDOUT_FILENO,"Invalid menu. Try again.\n");
        break;
    }
    // Loop continues for the next menu interaction
  }
  
  // Free allocated resources before exiting
  if (my_kty != NULL) {
      free_kty_item(my_kty);
      my_kty = NULL;
  }
  if (parser != NULL) {
      free(parser);
      parser = NULL;
  }
  return 0;
}

// --- Mock Function Definitions ---

ssize_t receive(int fd, char *buf, size_t count, int *bytes_received) {
    if (fd == STDIN_FILENO) { // Mock for stdin
        ssize_t res = read(STDIN_FILENO, buf, count);
        if (res == -1) {
            *bytes_received = 0;
            return -1; // Indicate error as per original's != 0
        } else {
            *bytes_received = res;
            return 0; // Indicate success as per original's == 0
        }
    }
    // For other FDs, or if specific behavior is needed, implement here.
    // For now, assume only STDIN_FILENO is handled.
    *bytes_received = 0;
    return -1; // Error for unhandled file descriptors
}

int fdprintf(int fd, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vdprintf(fd, format, args);
    va_end(args);
    return ret;
}

void free_kty_item(void *kty_item) {
    if (kty_item) {
        // In a real scenario, this would free internal structures of kty_item
        free(kty_item);
    }
}

void kty_print_mock(void *kty_item) {
    if (kty_item) {
        fdprintf(STDOUT_FILENO, "Printing KTY item: '%s' (addr: %p)\n", (char*)kty_item, kty_item);
    } else {
        fdprintf(STDOUT_FILENO, "No KTY item to print.\n");
    }
}

void *kty_create_mock(void *data) {
    if (data) {
        fdprintf(STDOUT_FILENO, "Creating KTY item from data: '%s'\n", (char*)data);
        // In real code, this would parse data and return a new kty_item
        // Allocate space for a dummy KTY item (e.g., 16 bytes for a string)
        void *new_kty = malloc(16); 
        if (new_kty) {
            strncpy((char*)new_kty, (char*)data, 15);
            ((char*)new_kty)[15] = '\0';
        }
        return new_kty;
    }
    return NULL;
}

int kty_init(void **parser_ptr) {
    if (!parser_ptr) return -1;

    // Initialize parser slots with mock functions and data
    // parser[0] is used as an int counter/flag in nyan and print_kty/import_kty
    *(int*)parser_ptr[0] = 0; // Initialize to 0

    // parser[1] is used by array_length/array_get, can be a dummy value
    parser_ptr[1] = NULL; 

    // parser[2] is kty_print_func
    // Cast the address of the mock function to the function pointer type
    *(kty_print_func *)&parser_ptr[2] = kty_print_mock;

    // parser[3] is kty_create_func
    *(kty_create_func *)&parser_ptr[3] = kty_create_mock;

    return 0; // Success
}

// Dummy data for nyan's array functions
struct NyanArrayItem dummy_nyan_items[] = {
    {0, "Meow!", 5},
    {0, "Purrr...", 8},
    {0, "Nyan nyan!", 10}
};
const int DUMMY_NYAN_ARRAY_LEN = sizeof(dummy_nyan_items) / sizeof(dummy_nyan_items[0]);

int array_length(void *arr) {
    (void)arr; // Unused, for signature compatibility
    return DUMMY_NYAN_ARRAY_LEN;
}

void *array_get(void *arr, int index) {
    (void)arr; // Unused
    if (index >= 0 && index < DUMMY_NYAN_ARRAY_LEN) {
        return &dummy_nyan_items[index];
    }
    return NULL;
}