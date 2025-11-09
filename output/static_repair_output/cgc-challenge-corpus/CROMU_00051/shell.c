#include <stdio.h>    // For printf, puts, fopen, fwrite, fclose, FILE
#include <string.h>   // For strtok_r, strlen, strcat, strchr, strncpy, strcmp, strcpy, strcspn
#include <stdlib.h>   // For exit
#include <stdbool.h>  // For bool type

// Define custom types to match the original snippet's inferred types
typedef int undefined4;     // Represents a 4-byte undefined type, used for return values (0, 1, 2)
typedef unsigned int uint;  // Represents an unsigned integer
typedef unsigned char byte; // Represents a single byte

// --- Global Variables (replaces DAT_xxxxxx and ENV) ---

// Current user, used in PrintPrompt and HandleHelp
char current_user[64] = "user";

// Command history storage
#define MAX_COMMAND_HISTORY 17          // Number of history entries (0x10 + 1 for indices)
#define COMMAND_HISTORY_ENTRY_LEN 256   // Max length of each history entry (0x100)
char command_history[MAX_COMMAND_HISTORY][COMMAND_HISTORY_ENTRY_LEN];

// Current number of valid entries in command history
unsigned int command_history_count = 0;

// --- Structure for Parsed Command Line Arguments ---

#define MAX_CLI_ARGS 9          // Maximum number of arguments (including command name)
#define CLI_ARG_MAX_LEN 64      // Maximum length for each argument string (0x40)

// Structure to hold parsed command line arguments
typedef struct {
    uint count; // Total number of parsed arguments (args[0] is command name, args[1+] are parameters)
    char args[MAX_CLI_ARGS][CLI_ARG_MAX_LEN]; // Array of argument strings
} ParsedCliArgs;

// Helper macro to calculate the address of an argument string within ParsedCliArgs.
// `base_ptr` is expected to point to the `count` field of a ParsedCliArgs structure.
// The `+ sizeof(uint)` skips the `count` field.
// `+ (index) * CLI_ARG_MAX_LEN` calculates the offset for the specific argument string.
#define GET_ARG_PTR(base_ptr, index) ((char*)(((unsigned char*)(base_ptr)) + sizeof(uint) + (index) * CLI_ARG_MAX_LEN))

// --- Dummy Functions (to make the code compilable) ---

void ListFiles(void) {
  puts("Listing files...");
}

int Dump(char *filename) {
  printf("Dumping file: %s\n", filename);
  // Simulate success for compilation
  return 1;
}

// _terminate is replaced by exit from stdlib.h for standard compliance
#define _terminate(code) exit(code)

// --- Original Functions (Refactored and Fixed) ---

// Function: PrintPrompt
void PrintPrompt(void) {
  // Corrected printf format string and argument
  printf("%s@CB> ", current_user);
}

// Function: ParseCli
// Parses the input command string into arguments, handling quotes.
// input_cmd_str: The command line string to parse (will be modified by strtok_r).
// args_count_ptr: Pointer to the 'count' field of a ParsedCliArgs structure.
undefined4 ParseCli(char *input_cmd_str, uint *args_count_ptr) {
  bool in_quote_mode = false;   // Tracks if currently parsing a multi-word quoted argument
  undefined4 return_val = 2;    // Default success value (0: invalid params, 1: parsing error)
  char *token;                  // Pointer to the current token returned by strtok_r
  size_t token_len;             // Length of the current token
  char *strtok_context = NULL;  // Context pointer for strtok_r (for reentrancy)
  
  if ((input_cmd_str == NULL) || (args_count_ptr == NULL)) {
    return 0; // Invalid parameters
  }
  
  *args_count_ptr = 0; // Initialize argument count

  // Loop through tokens separated by spaces. Stop if no more tokens or max arguments reached.
  while (((token = strtok_r(input_cmd_str, " ", &strtok_context)), token != NULL) && (*args_count_ptr < MAX_CLI_ARGS)) {
    input_cmd_str = NULL; // For subsequent strtok_r calls, pass NULL to continue parsing the same string

    // current_target_arg_ptr points to the memory location for the argument being processed.
    // If in_quote_mode, *args_count_ptr already holds the index of the argument being built.
    // If not in_quote_mode, *args_count_ptr is the index for the new argument.
    char *current_target_arg_ptr = GET_ARG_PTR(args_count_ptr, *args_count_ptr);

    if (in_quote_mode) {
      // Currently inside a multi-word quoted argument
      token_len = strlen(token);
      if (token[token_len - 1] == '\"') {
        // Found closing quote for the multi-word argument
        token[token_len - 1] = '\0'; // Remove the trailing quote

        size_t current_arg_len = strlen(current_target_arg_ptr);
        // Check for buffer overflow before appending
        if ((CLI_ARG_MAX_LEN - 1) < (current_arg_len + strlen(token) + 1)) { // +1 for space
          return 1; // Buffer overflow risk
        }
        strcat(current_target_arg_ptr, " "); // Append space
        strcat(current_target_arg_ptr, token); // Append token
        
        (*args_count_ptr)++; // Argument completed, increment count for the next argument
        in_quote_mode = false; // Exit quote mode
      } else if ((*token == '\"') && (token[1] == '\0')) {
        // Token is just '"', which the original code treats as completing a quoted argument.
        // This handles cases like `command "arg1" " " "arg2"`, where " " would yield '"' token.
        (*args_count_ptr)++; // Argument completed, increment count
        in_quote_mode = false; // Exit quote mode
      } else {
        // Still inside a multi-word quoted string, no closing quote found in this token
        if (strchr(token, '\"') != NULL) {
          return 1; // Error: Unexpected internal quote within a multi-word quoted string
        }
        size_t current_arg_len = strlen(current_target_arg_ptr);
        // Check for buffer overflow before appending
        if ((CLI_ARG_MAX_LEN - 1) < (current_arg_len + strlen(token) + 1)) { // +1 for space
          return 1; // Buffer overflow risk
        }
        strcat(current_target_arg_ptr, " "); // Append space
        strcat(current_target_arg_ptr, token); // Append token
      }
    } else if (*token == '\"') {
      // Not in quote mode, but the token starts with a quote
      token_len = strlen(token);
      if (token_len > 1 && token[token_len - 1] == '\"') {
        // Single-word quoted string (e.g., "foo")
        strncpy(current_target_arg_ptr, token + 1, CLI_ARG_MAX_LEN - 1); // Copy without leading quote
        current_target_arg_ptr[CLI_ARG_MAX_LEN - 1] = '\0'; // Ensure null termination
        // Remove trailing quote if it was copied (e.g., from "foo")
        if (strlen(current_target_arg_ptr) > 0 && current_target_arg_ptr[strlen(current_target_arg_ptr)-1] == '\"') {
          current_target_arg_ptr[strlen(current_target_arg_ptr)-1] = '\0';
        }
        (*args_count_ptr)++; // Argument completed, increment count
      } else {
        // Start of a multi-word quoted string (e.g., "foo bar)
        if (strchr(token + 1, '\"') != NULL) {
          return 1; // Error: Unexpected internal quote when starting a quoted string
        }
        in_quote_mode = true; // Enter quote mode
        strncpy(current_target_arg_ptr, token + 1, CLI_ARG_MAX_LEN - 1); // Copy without leading quote
        current_target_arg_ptr[CLI_ARG_MAX_LEN - 1] = '\0'; // Ensure null termination
        // *args_count_ptr is NOT incremented here; it will be incremented when the quote is closed.
      }
    } else {
      // Regular unquoted token
      strncpy(current_target_arg_ptr, token, CLI_ARG_MAX_LEN - 1); // Copy the token
      current_target_arg_ptr[CLI_ARG_MAX_LEN - 1] = '\0'; // Ensure null termination
      (*args_count_ptr)++; // Argument completed, increment count
    }
  }
  
  if (in_quote_mode) {
      return 1; // Error: Unclosed quote at the end of the command string
  }
  return return_val; // Return 2 for successful parsing
}

// Function: HandleListFiles
// args_count_ptr: Pointer to the 'count' field of a ParsedCliArgs structure.
undefined4 HandleListFiles(uint *args_count_ptr) {
  if (args_count_ptr == NULL) {
    return 0; // Invalid parameters
  } else if (*args_count_ptr == 1) { // Only command name, no extra arguments
    ListFiles();
    return 1;
  } else {
    puts("Invalid arguments");
    return 0;
  }
}

// Function: HandleExit
// args_count_ptr: Pointer to the 'count' field of a ParsedCliArgs structure.
undefined4 HandleExit(uint *args_count_ptr) {
  if (args_count_ptr == NULL) {
    return 0; // Invalid parameters
  } else if (*args_count_ptr == 1) { // Only command name, no extra arguments
    puts("logout");
    _terminate(0); // Calls exit(0), program terminates here
    // This line will not be reached
    return 1; 
  } else {
    puts("Invalid arguments");
    return 0;
  }
}

// Function: HandleHelp
// args_count_ptr: Pointer to the 'count' field of a ParsedCliArgs structure.
undefined4 HandleHelp(uint *args_count_ptr) {
  if (args_count_ptr == NULL) {
    return 0; // Invalid parameters
  } else {
    if (*args_count_ptr == 1) { // No specific command requested, print general help
      puts("Available commands:");
      puts("chuser");
      puts("chpw");
      puts("exit");
      puts("id");
      puts("help");
      puts("?");
      puts("newuser");
      puts("deluser");
      puts("dump");
      puts("print");
    } else if (*args_count_ptr == 2) { // Help for a specific command (command name + one argument)
      // The specific command for help is at args[1]
      char *help_cmd = GET_ARG_PTR(args_count_ptr, 1);
      
      if (strcmp(help_cmd, "chuser") == 0) {
        puts("Usage: chuser <username>");
      } else if (strcmp(help_cmd, "passwd") == 0) {
        // Usage depends on current user (root has more options)
        if (strcmp(current_user, "root") == 0) {
          puts("Usage: passwd <username>");
        } else {
          puts("Usage: passwd");
        }
      } else if (strcmp(help_cmd, "exit") == 0) {
        puts("Usage: exit");
      } else if (strcmp(help_cmd, "id") == 0) {
        puts("Usage: id");
      } else if (strcmp(help_cmd, "help") == 0) {
        puts("Usage: help <command>");
      } else if (strcmp(help_cmd, "?") == 0) {
        puts("Usage: ? <command>");
      } else if (strcmp(help_cmd, "newuser") == 0) {
        puts("Usage: newuser <username> <group>");
      } else if (strcmp(help_cmd, "deluser") == 0) {
        puts("Usage: deluser <username>");
      } else if (strcmp(help_cmd, "dump") == 0) {
        puts("Usage: dump <filename>");
      } else if (strcmp(help_cmd, "print") == 0) {
        puts("Usage: print <text> [> file]");
      } else {
        // Corrected printf format string
        printf("Unknown command: %s\n", help_cmd);
      }
    } else {
        puts("Invalid arguments for help command."); // More than two arguments for help is an error
    }
    return 1;
  }
}

// Function: HandleDump
// args_count_ptr: Pointer to the 'count' field of a ParsedCliArgs structure.
undefined4 HandleDump(uint *args_count_ptr) {
  if (args_count_ptr == NULL) {
    return 0; // Invalid parameters
  } else if (*args_count_ptr == 2) { // Command name + 1 argument (filename)
    // Filename is at args[1]
    return Dump(GET_ARG_PTR(args_count_ptr, 1));
  } else {
    puts("Input error");
    puts("Usage: dump <filename>");
    return 0;
  }
}

// Function: HandlePrint
// args_count_ptr: Pointer to the 'count' field of a ParsedCliArgs structure.
undefined4 HandlePrint(uint *args_count_ptr) {
  FILE *output_file_handle = NULL; // File handle for redirection
  uint i;                           // Loop counter
  
  if (args_count_ptr == NULL) {
    return 0; // Invalid parameters
  } else if (*args_count_ptr < 2) { // Need at least command + text to print
    puts("Input error");
    puts("Usage: print \"<text>\" [> file]");
    return 0;
  } else {
    char *output_filename_ptr = NULL; // Stores filename if redirection is found

    // Check for redirection operator '>'
    // Arguments start from index 1 (args[0] is the command "print")
    for (i = 1; i < *args_count_ptr; i++) {
      char *current_arg = GET_ARG_PTR(args_count_ptr, i);
      if (strcmp(current_arg, ">") == 0) {
        // Redirection operator found at index 'i'
        // The filename must be at index 'i + 1' and must be the last argument.
        if (i + 1 >= *args_count_ptr) { // Check if filename is missing
            puts("Input error: Missing filename for redirection.");
            puts("Usage: print \"<text>\" [> file]");
            return 0;
        }
        if (i + 1 != *args_count_ptr - 1) { 
            // Original code: `if (local_14 + 1 != *param_1 - 1)`
            // This means if the filename is NOT the very last argument, it's an error.
            puts("Input error: Redirection filename must be the last argument.");
            puts("Usage: print \"<text>\" [> file]");
            return 0;
        }
        output_filename_ptr = GET_ARG_PTR(args_count_ptr, i + 1);
        break; // Found redirection, no need to check further arguments
      }
    }

    if (output_filename_ptr == NULL) {
      // No redirection, print to stdout
      printf("%s", GET_ARG_PTR(args_count_ptr, 1)); // Print the first text argument
      for (i = 2; i < *args_count_ptr; i++) {
        printf(" %s", GET_ARG_PTR(args_count_ptr, i)); // Print subsequent text arguments with leading space
      }
      puts(""); // Add a newline at the end
    } else {
      // Redirection to file
      output_file_handle = fopen(output_filename_ptr, "w");
      if (output_file_handle == NULL) {
        printf("Unable to open file '%s'\n", output_filename_ptr); // Corrected format
        return 0;
      }
      
      // Write all text arguments to the file, up to the '>' symbol
      for (i = 1; i < *args_count_ptr; i++) {
        char *current_arg_to_write = GET_ARG_PTR(args_count_ptr, i);
        if (strcmp(current_arg_to_write, ">") == 0) {
            break; // Stop writing when '>' is encountered
        }
        size_t arg_len = strlen(current_arg_to_write);
        if (fwrite(current_arg_to_write, 1, arg_len, output_file_handle) != arg_len) {
          fclose(output_file_handle);
          return 0; // Error writing
        }
        // Add a space between arguments, unless it's the last argument before '>'
        if (i + 1 < *args_count_ptr) { // Check if there's a next argument
            char *next_arg = GET_ARG_PTR(args_count_ptr, i + 1);
            if (strcmp(next_arg, ">") != 0) { // If the next arg is not '>', add a space
                if (fwrite(" ", 1, 1, output_file_handle) != 1) {
                    fclose(output_file_handle);
                    return 0; // Error writing space
                }
            }
        }
      }
      // Add a newline at the end of the file, consistent with puts("") for stdout
      if (fwrite("\n", 1, 1, output_file_handle) != 1) {
          fclose(output_file_handle);
          return 0; // Error writing newline
      }
      fclose(output_file_handle);
    }
    return 1;
  }
}

// Function: PrependCommandHistory
// Adds a new command to the history, shifting older commands down.
void PrependCommandHistory(char *new_command) {
  unsigned char i; // Loop counter for history array indices
  
  // Shift existing history entries down (from index MAX-1 down to 1)
  for (i = MAX_COMMAND_HISTORY - 1; i > 0; i--) {
    // Only copy if the source entry is not empty.
    // If command_history[i-1] is empty, then command_history[i] should also become empty.
    if (command_history[i - 1][0] != '\0') {
      strcpy(command_history[i], command_history[i - 1]);
    } else {
      command_history[i][0] = '\0'; // Clear the target slot
    }
  }
  
  // Copy the new command to the first slot (index 0)
  strncpy(command_history[0], new_command, COMMAND_HISTORY_ENTRY_LEN - 1);
  command_history[0][COMMAND_HISTORY_ENTRY_LEN - 1] = '\0'; // Ensure null termination

  // Recalculate command_history_count (replaces ENV logic)
  // Count how many entries are non-empty from the beginning of the array
  unsigned int new_count = 0;
  for (unsigned int j = 0; j < MAX_COMMAND_HISTORY; j++) {
      if (command_history[j][0] != '\0') {
          new_count++;
      } else {
          break; // Stop at the first empty slot, as history is contiguous from index 0
      }
  }
  command_history_count = new_count;
}

// --- Main Function (for demonstration) ---
int main() {
    char input_buffer[256]; // Buffer to store user input
    ParsedCliArgs cli_args; // Structure to hold parsed arguments

    printf("Welcome to the shell!\n");

    // Main shell loop
    while (true) {
        PrintPrompt(); // Display the command prompt
        if (fgets(input_buffer, sizeof(input_buffer), stdin) == NULL) {
            puts("\nExiting...");
            break; // Exit on EOF or error
        }
        // Remove trailing newline character from input if present
        input_buffer[strcspn(input_buffer, "\n")] = 0;

        if (strlen(input_buffer) == 0) {
            continue; // Skip empty commands
        }

        PrependCommandHistory(input_buffer); // Add command to history

        // Create a copy of the input buffer because ParseCli (via strtok_r) modifies it
        char cmd_copy[256];
        strncpy(cmd_copy, input_buffer, sizeof(cmd_copy) - 1);
        cmd_copy[sizeof(cmd_copy) - 1] = '\0';

        // Parse the command line input
        undefined4 parse_result = ParseCli(cmd_copy, &cli_args.count);

        if (parse_result == 0) {
            puts("Error: Invalid CLI parameters during parsing.");
            continue;
        } else if (parse_result == 1) {
            puts("Error: CLI parsing error (e.g., unclosed quote, internal quote, or buffer overflow).");
            continue;
        }

        if (cli_args.count == 0) {
            continue; // No command parsed
        }

        // Get the command name (first argument)
        char *command_name = GET_ARG_PTR(&cli_args.count, 0);

        // Dispatch commands
        if (strcmp(command_name, "ls") == 0) {
            HandleListFiles(&cli_args.count);
        } else if (strcmp(command_name, "exit") == 0) {
            HandleExit(&cli_args.count);
            // HandleExit calls exit(0), so if successful, control won't return here.
            // If it returns (e.g., due to invalid args), the loop continues.
        } else if (strcmp(command_name, "help") == 0 || strcmp(command_name, "?") == 0) {
            HandleHelp(&cli_args.count);
        } else if (strcmp(command_name, "dump") == 0) {
            HandleDump(&cli_args.count);
        } else if (strcmp(command_name, "print") == 0) {
            HandlePrint(&cli_args.count);
        } else if (strcmp(command_name, "history") == 0) {
            // Simple command to display history
            for (unsigned int k = 0; k < command_history_count; k++) {
                printf("%u: %s\n", k + 1, command_history[k]);
            }
        } else {
            printf("Unknown command: %s\n", command_name);
        }
    }

    return 0; // Program exits
}