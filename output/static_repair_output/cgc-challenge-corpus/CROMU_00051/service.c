#include <stdio.h>   // For printf, puts
#include <string.h>  // For strcmp, memset
// #include <stdlib.h> // Potentially for other utilities, not directly used in main's logic

// --- External Declarations (assuming these are defined elsewhere) ---

// Global variables
extern unsigned int DAT_4347c000;
extern char DAT_0002a921; // Assuming this is the start of a memory block
extern int ENV;

// Functions
extern void seed_prng(unsigned int seed);
extern void InitFilesystem(void);
extern void Login(void);
extern void PrintPrompt(void);
extern int ReadShellPrompt(int fd, char *buffer, int max_len);
extern void PrependCommandHistory(char *command);
// Adjusted ParseCli signature: takes input, output buffer for command, output buffer for arguments
extern char ParseCli(char *input, char *command_output, char *args_output);
extern void HandleListFiles(char *args);
extern void ChUser(char *args);
extern void ChPw(char *args);
extern void HandleExit(char *args);
extern void ID(char *args);
extern void HandleHelp(char *args);
extern void NewUser(char *args);
extern void DelUser(char *args);
extern void HandleDump(char *args);
extern void HandlePrint(char *args);

// --- Main Function ---

int main(void) {
  char input_buffer[256];   // Corresponds to local_31c
  char args_buffer[256];    // Corresponds to local_21c, enlarged for practical use
  char command_buffer[515]; // Corresponds to acStack_218

  // Original: local_10 = &stack0x00000004; - Removed, unused and non-standard.
  // Original: local_14 = &DAT_4347c000; - Removed, value used directly.
  seed_prng(DAT_4347c000); // Pass the global variable directly

  InitFilesystem();
  memset(&DAT_0002a921, 0, 0x1000); // Use memset for bzero
  ENV = 0;
  Login();

  while (1) { // Main shell loop
    while (1) { // Loop for reading input until valid
      PrintPrompt();
      // ReadShellPrompt returns 0 on failure (e.g., empty input), break on success
      if (ReadShellPrompt(0, input_buffer, sizeof(input_buffer) - 1) != 0) {
        break;
      }
      printf("\b\b  \b\b\n\r"); // Clear line for empty input
    }
    printf("\b\b  \b\b\n\r"); // Clear line after successful input

    PrependCommandHistory(input_buffer);

    // Call ParseCli with adjusted signature to populate command_buffer and args_buffer
    char parse_status = ParseCli(input_buffer, command_buffer, args_buffer);

    if (parse_status == '\0') {
      break; // Exit main loop if ParseCli indicates termination
    }
    if (parse_status == '\x01') {
      puts("Invalid input");
    } else {
      // Use direct strcmp comparisons instead of intermediate iVar1
      if (strcmp(command_buffer, "list") == 0) {
        HandleListFiles(args_buffer);
      } else if (strcmp(command_buffer, "chuser") == 0) {
        ChUser(args_buffer);
      } else if (strcmp(command_buffer, "chpw") == 0) {
        ChPw(args_buffer);
      } else if (strcmp(command_buffer, "exit") == 0) {
        HandleExit(args_buffer);
      } else if (strcmp(command_buffer, "id") == 0) {
        ID(args_buffer);
      } else if (strcmp(command_buffer, "?") == 0 || strcmp(command_buffer, "help") == 0) {
        HandleHelp(args_buffer);
      } else if (strcmp(command_buffer, "newuser") == 0) {
        NewUser(args_buffer);
      } else if (strcmp(command_buffer, "deluser") == 0) {
        DelUser(args_buffer);
      } else if (strcmp(command_buffer, "dump") == 0) {
        HandleDump(args_buffer);
      } else if (strcmp(command_buffer, "print") == 0) {
        HandlePrint(args_buffer);
      } else {
        printf("Invalid command: %s\n\r", input_buffer); // Use %s for string formatting
      }
    }
  }
  return 0;
}