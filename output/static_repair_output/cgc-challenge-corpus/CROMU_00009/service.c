#include <stdio.h>    // For puts, printf
#include <stdlib.h>   // For malloc, free, strdup
#include <string.h>   // For strlen, strcpy, strcat, memset, strtok, strcmp
#include <unistd.h>   // For read (if readUntil uses it, but we'll mock it)

// Global variables as inferred from usage
// ARGV can hold up to 9 arguments (indices 0-8)
char *ARGV[9];
int ARGC;
char CWD[256];

// Mock functions for compilation
// These replace 'allocate' and 'deallocate' from the original snippet
int allocate(size_t size, int flags, char **ptr) {
    (void)flags; // Unused parameter
    *ptr = (char *)malloc(size);
    return (*ptr != NULL) ? 0 : -1;
}

void deallocate(char *ptr, size_t size) {
    (void)size; // Unused parameter, free doesn't need size
    free(ptr);
}

// Mock for filesystem initialization
int InitFS(int size) {
    (void)size; // Unused parameter
    // printf("[Mock] Initializing filesystem with size %d...\n", size);
    return 0; // Simulate success
}

// Mock for filesystem destruction
int DestroyFS() {
    // printf("[Mock] Destroying filesystem...\n");
    return 0; // Simulate success
}

// Mock for reading input
// Simplistic mock: reads a line from stdin
int readUntil(char *buffer, int max_len, char delimiter) {
    int i = 0;
    int c;
    while (i < max_len && (c = getchar()) != EOF && c != delimiter) {
        buffer[i++] = (char)c;
    }
    buffer[i] = '\0';
    if (c == EOF && i == 0) { // EOF encountered at start of read
        return -1;
    }
    return i; // Return number of characters read
}

// Mock command functions
void mock_help_func() { puts("Mock help command executed."); }
void mock_ls_func() { puts("Mock ls command executed."); }
void mock_cat_func() { puts("Mock cat command executed."); }

// Array of commands for the original loop structure in main.
// Each command entry consists of: command name (char*), function pointer (void*),
// and a placeholder (void*), making each entry 3 'void*' in size.
void *cmds[] = {
    (void *)"help", (void *)mock_help_func, (void *)NULL,
    (void *)"ls",   (void *)mock_ls_func,   (void *)NULL,
    (void *)"cat",  (void *)mock_cat_func,  (void *)NULL,
    (void *)NULL // Terminator for the loop
};

// Function: FreeArgs
int FreeArgs(void) {
  for (int i = 0; i < 9 && ARGV[i] != NULL; ++i) {
    deallocate(ARGV[i], strlen(ARGV[i]));
    ARGV[i] = NULL;
  }
  return 0;
}

// Function: ParseArgs
unsigned int ParseArgs(char *param_1) {
  char *token;
  char *new_arg_buf;
  size_t current_arg_len, token_len;
  int ret_val;
  int in_quote_mode = 0; // Tracks if currently parsing a multi-word quoted argument

  memset(ARGV, 0, sizeof(ARGV));
  ARGC = 0;

  token = strtok(param_1, " ");

  if (token == NULL) {
    return 0; // No arguments
  }

  // Handle the first token
  if (*token == '\"') {
      in_quote_mode = 1;
      // If the first token is a complete quoted argument (e.g., "arg")
      if (token[strlen(token) - 1] == '\"' && strlen(token) > 1) {
          token[strlen(token) - 1] = '\0'; // Remove trailing quote
          ARGV[ARGC++] = strdup(token + 1); // Duplicate without leading quote
          in_quote_mode = 0; // Exit quote mode as it was a complete argument
      } else { // Only leading quote, start of a multi-word quoted argument
          ARGV[ARGC] = strdup(token); // Store with leading quote, ARGC not incremented yet
      }
  } else { // Regular argument
      ARGV[ARGC++] = strdup(token);
  }

  // Handle subsequent tokens
  while ((token = strtok(NULL, " ")) != NULL && ARGC < 9) {
    if (!in_quote_mode) { // Not currently building a quoted argument
      if (*token == '\"') {
          in_quote_mode = 1;
          // If it's a complete quoted argument (e.g., "arg")
          if (token[strlen(token) - 1] == '\"' && strlen(token) > 1) {
              token[strlen(token) - 1] = '\0';
              ARGV[ARGC++] = strdup(token + 1);
              in_quote_mode = 0;
          } else { // Only leading quote, start of multi-word quoted arg
              ARGV[ARGC] = strdup(token); // ARGC not incremented
          }
      } else { // Regular argument
          ARGV[ARGC++] = strdup(token);
      }
    } else { // In quote mode, building a multi-word quoted argument
      current_arg_len = strlen(ARGV[ARGC]);
      token_len = strlen(token);

      if (token[token_len - 1] == '\"') { // Current token ends the quote
        // Calculate total length: (current_arg_len - 1 for leading quote) + 1 (for space)
        // + (token_len - 1 for trailing quote) + 1 (for null terminator)
        // This simplifies to current_arg_len + token_len - 1
        size_t total_len_needed = current_arg_len + token_len - 1;

        ret_val = allocate(total_len_needed, 0, &new_arg_buf);
        if (ret_val != 0) { FreeArgs(); ARGC = 0; return 0; }

        // Copy ARGV[ARGC] starting from the second character (skip leading quote)
        strcpy(new_arg_buf, ARGV[ARGC] + 1);
        strcat(new_arg_buf, " ");
        token[token_len - 1] = '\0'; // Remove trailing quote from token
        strcat(new_arg_buf, token);

        deallocate(ARGV[ARGC], current_arg_len);
        ARGV[ARGC++] = new_arg_buf; // Assign and increment ARGC
        in_quote_mode = 0; // Exit quote mode
      } else { // Current token does not end the quote, just concatenate
        // Calculate total length: current_arg_len + 1 (for space) + token_len + 1 (for null terminator)
        size_t total_len_needed = current_arg_len + token_len + 2;

        ret_val = allocate(total_len_needed, 0, &new_arg_buf);
        if (ret_val != 0) { FreeArgs(); ARGC = 0; return 0; }

        strcpy(new_arg_buf, ARGV[ARGC]);
        strcat(new_arg_buf, " ");
        strcat(new_arg_buf, token);
        deallocate(ARGV[ARGC], current_arg_len);
        ARGV[ARGC] = new_arg_buf; // ARGC not incremented, still building this argument
      }
    }
  }

  // Post-loop checks
  if (in_quote_mode) {
    puts("missing quote");
    FreeArgs();
    ARGC = 0;
  } else if (ARGC == 9) { // If ARGC reached 9, it means 9 args were successfully parsed (0-8), which is considered too many.
    puts("too many args");
    FreeArgs();
    ARGC = 0;
  }
  return ARGC;
}

// Function: main
int main(void) {
  int ret_val;
  char input_buffer[256];

  memset(CWD, 0, sizeof(CWD));
  strcpy(CWD, "/");

  ret_val = InitFS(0x80000); // 524288 bytes
  if (ret_val == 0) {
    puts("Welcome to the cgcfs shell!");
    puts("Type help for a list of available commands.");

    while (1) {
      printf("%% "); // Escaped % to print literal %
      fflush(stdout); // Ensure prompt is displayed before readUntil blocks
      memset(input_buffer, 0, sizeof(input_buffer));

      ret_val = readUntil(input_buffer, sizeof(input_buffer) - 1, '\n');
      if (ret_val == -1) break; // Error or EOF

      ParseArgs(input_buffer);

      if (ARGC == 0) {
        FreeArgs();
      } else {
        int command_found = 0;
        void **current_cmd_entry;

        // Iterate through commands array (each entry is 3 void* elements)
        for (current_cmd_entry = cmds; *current_cmd_entry != NULL; current_cmd_entry += 3) {
          if (strcmp((char *)*current_cmd_entry, ARGV[0]) == 0) {
            // Cast the second element of the entry to a function pointer and call it
            ((void (*)(void))current_cmd_entry[1])();
            command_found = 1;
            break;
          }
        }

        if (!command_found) {
          puts("Invalid command");
        }
        FreeArgs();
      }
    }

    ret_val = DestroyFS();
    if (ret_val == 0) {
      return 0;
    } else {
      puts("Failed to destroy the RAM file system\n");
      return 1; // Return non-zero for error
    }
  } else {
    puts("Failed to initialize the RAM file system\n");
    return 1; // Return non-zero for error
  }
}