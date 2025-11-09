#include <stdio.h>    // For puts, printf, fopen, fclose, fwrite, FILE
#include <stdlib.h>   // For exit, strdup
#include <string.h>   // For strlen, strcpy, strcat, strcmp, strncpy
#include <sys/stat.h> // For mkdir, mode_t
#include <unistd.h>   // For rmdir, unlink, write, getcwd
#include <ctype.h>    // For isxdigit

// --- Dummy function declarations for external dependencies ---
// These functions are not defined in the supplied snippet and are assumed to exist elsewhere.
// Basic declarations are provided to allow compilation.
// For a real system, these would be implemented or linked from a library.

/**
 * @brief Simulates receiving bytes.
 * @param fd File descriptor (e.g., 0 for stdin).
 * @param buf Buffer to store received bytes.
 * @param len Maximum number of bytes to receive.
 * @param bytes_read_count Pointer to store the actual number of bytes read.
 * @return 0 on success, -1 on error.
 */
int receive(int fd, unsigned char *buf, int len, int *bytes_read_count) {
    // Dummy implementation: simulates reading from stdin
    // In a real scenario, this would interact with a communication channel or device.
    ssize_t actual_read = read(fd, buf, len);
    if (actual_read >= 0) {
        if (bytes_read_count) {
            *bytes_read_count = (int)actual_read;
        }
        return 0; // Success
    }
    if (bytes_read_count) {
        *bytes_read_count = 0;
    }
    perror("receive (dummy)");
    return -1; // Error
}

/**
 * @brief Simulates program termination.
 */
void _terminate(void) {
    // Dummy implementation: exits the program
    exit(0);
}

/**
 * @brief Simulates listing directory contents.
 * @param path The path to list.
 * @return 0 on success, -1 on error.
 */
int ls(const char *path) {
    printf("ls %s (dummy)\n", path);
    // Real implementation would open and read directory entries.
    return 0;
}

/**
 * @brief Simulates reading and printing file contents.
 * @param path The path of the file to read.
 * @return 0 on success, -1 on error.
 */
int ReadFile(const char *path) {
    printf("dump %s (dummy)\n", path);
    FILE *f = fopen(path, "r");
    if (f) {
        char buffer[1024];
        size_t bytes;
        while ((bytes = fread(buffer, 1, sizeof(buffer), f)) > 0) {
            fwrite(buffer, 1, bytes, stdout);
        }
        fclose(f);
        return 0;
    }
    perror("Error reading file (dummy)");
    return -1;
}

/**
 * @brief Simulates displaying filesystem status.
 */
void StatusFS(void) {
    puts("Filesystem status (dummy)");
    // Real implementation would query filesystem information (e.g., using statfs).
}

// --- Global variables ---
// These variables are used to mimic command-line arguments and environment.
int ARGC;                 // Total number of arguments (mimics main's argc)
char *CWD;                // Current Working Directory
char *DAT_0001a9e4;       // Corresponds to argv[1] (first argument after command name)
char *DAT_0001a9e8;       // Corresponds to argv[2]
char *DAT_0001a9ec;       // Corresponds to argv[3]

// Forward declarations for handler functions, needed for the command table.
// These functions are defined below.
int ExitHandler(void);
int MkdirHandler(void);
int RmdirHandler(void);
int LsHandler(void);
int RmHandler(void);
int EchoHandler(void);
int CatHandler(void);
int HelpHandler(void);
int DfHandler(void);

// This 'cmds' array is specifically structured to be used by the HelpHandler,
// which expects elements at index 0 (name) and index 2 (description).
// The value at index 1 is ignored by HelpHandler but could conceptually be a function pointer.
char *cmds[] = {
    "mkfolder", (char *)MkdirHandler, "Create a new folder",
    "delfolder", (char *)RmdirHandler, "Delete an empty folder",
    "show", (char *)LsHandler, "List directory contents",
    "delete", (char *)RmHandler, "Delete a file",
    "echo", (char *)EchoHandler, "Print text or write to file",
    "dump", (char *)CatHandler, "Display file contents",
    "fsinfo", (char *)DfHandler, "Show file system information",
    "help", (char *)HelpHandler, "Display this help message",
    "bye", (char *)ExitHandler, "Exit the shell",
    NULL, NULL, NULL // Sentinel to mark the end of the array
};

// --- Helper functions ---

/**
 * @brief Checks if a character is a hexadecimal digit.
 * @param c The character to check.
 * @return 1 if hex digit, 0 otherwise.
 */
int IsHex(char c) {
    return isxdigit((unsigned char)c) ? 1 : 0;
}

/**
 * @brief Converts a hexadecimal character to its nibble value.
 * @param c The hex character ('0'-'9', 'a'-'f', 'A'-'F').
 * @return The integer value (0-15) of the nibble.
 */
static char hex_to_nibble(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 0; // Should not happen if IsHex is used correctly
}

/**
 * @brief Converts a two-character hexadecimal string (e.g., "AB") to a single byte.
 * @param hex_pair A pointer to a two-character string (e.g., "3F").
 * @return The converted byte value.
 */
char Hex2Char(const char *hex_pair) {
    char high_nibble = hex_to_nibble(hex_pair[0]);
    char low_nibble = hex_to_nibble(hex_pair[1]);
    return (high_nibble << 4) | low_nibble;
}

/**
 * @brief Unescapes hexadecimal sequences (e.g., "\xAB") in a string in-place.
 * @param str The string to unescape.
 * @return The new length of the string on success, -1 on error (e.g., NULL input).
 */
int Unescape(char *str) {
    if (str == NULL) {
        return -1;
    }

    char *read_ptr = str;
    char *write_ptr = str;

    while (*read_ptr != '\0') {
        if (*read_ptr == '\\' && read_ptr[1] == 'x') {
            // Check for valid hex sequence: \xAB
            if (IsHex(read_ptr[2]) && IsHex(read_ptr[3])) {
                *write_ptr = Hex2Char(read_ptr + 2); // Convert hex pair to char
                read_ptr += 4;                       // Advance read_ptr past \xAB
                write_ptr++;                         // Advance write_ptr by 1 for the new char
            } else {
                // Invalid hex sequence, copy '\x' literally
                *write_ptr++ = *read_ptr++; // Copy '\'
                *write_ptr++ = *read_ptr++; // Copy 'x'
            }
        } else {
            *write_ptr++ = *read_ptr++; // Copy character as is
        }
    }
    *write_ptr = '\0'; // Null-terminate the modified string

    return (int)(write_ptr - str); // Return new length
}

// --- Handler Functions ---

// Function: readUntil
unsigned int readUntil(unsigned char *buffer, unsigned int max_len, char delimiter) {
    int bytes_read;
    unsigned char current_byte;
    unsigned int i;

    for (i = 0; i < max_len; ++i) {
        if (receive(0, &current_byte, 1, &bytes_read) != 0 || bytes_read != 1) {
            return 0xFFFFFFFF; // Error or did not receive exactly 1 byte
        }
        if (current_byte == (unsigned char)delimiter) {
            break; // Delimiter found
        }
        buffer[i] = current_byte;
    }
    buffer[i] = 0; // Null-terminate the buffer
    return i;      // Return number of bytes read (excluding delimiter)
}

// Function: ExitHandler
int ExitHandler(void) {
    puts("bye");
    _terminate(); // This function is assumed to terminate the program.
                  // Any code after this point is unreachable.
    return 0;     // Unreachable
}

// Function: MkdirHandler
int MkdirHandler(void) {
    int result = 0;
    char path_buffer[256];
    // Default permissions for a new directory (rwxr-xr-x)
    mode_t default_mode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;

    if (ARGC == 2) {
        if (DAT_0001a9e4[0] == '/') { // Absolute path
            result = mkdir(DAT_0001a9e4, default_mode);
        } else { // Relative path
            size_t cwd_len = strlen(CWD);
            size_t arg_len = strlen(DAT_0001a9e4);

            if (cwd_len + arg_len + 1 >= sizeof(path_buffer)) {
                puts("command too long");
                result = -1;
            } else {
                strcpy(path_buffer, CWD);
                strcat(path_buffer, DAT_0001a9e4);
                result = mkdir(path_buffer, default_mode);
            }
        }
    } else {
        puts("usage: mkfolder <directory>");
        result = -1;
    }
    return result;
}

// Function: RmdirHandler
int RmdirHandler(void) {
    int result = 0;
    char path_buffer[256];

    if (ARGC == 2) {
        if (DAT_0001a9e4[0] == '/') { // Absolute path
            result = rmdir(DAT_0001a9e4);
        } else { // Relative path
            size_t cwd_len = strlen(CWD);
            size_t arg_len = strlen(DAT_0001a9e4);

            if (cwd_len + arg_len + 1 >= sizeof(path_buffer)) {
                puts("command too long");
                result = -1;
            } else {
                strcpy(path_buffer, CWD);
                strcat(path_buffer, DAT_0001a9e4);
                result = rmdir(path_buffer);
            }
        }
    } else {
        puts("usage: delfolder <directory>");
        result = -1;
    }
    return result;
}

// Function: LsHandler
int LsHandler(void) {
    int result = 0;
    char path_buffer[256];

    if (ARGC == 1) {
        result = ls(CWD);
    } else if (ARGC == 2) {
        if (DAT_0001a9e4[0] == '/') { // Absolute path
            result = ls(DAT_0001a9e4);
        } else { // Relative path
            size_t cwd_len = strlen(CWD);
            size_t arg_len = strlen(DAT_0001a9e4);

            if (cwd_len + arg_len + 1 >= sizeof(path_buffer)) {
                puts("command too long");
                result = -1;
            } else {
                strcpy(path_buffer, CWD);
                strcat(path_buffer, DAT_0001a9e4);
                result = ls(path_buffer);
            }
        }
    } else {
        puts("usage: show <directory>");
        result = -1;
    }
    return result;
}

// Function: RmHandler
int RmHandler(void) {
    int result = 0;
    char path_buffer[256];

    if (ARGC == 2) {
        if (DAT_0001a9e4[0] == '/') { // Absolute path
            result = unlink(DAT_0001a9e4);
        } else { // Relative path
            size_t cwd_len = strlen(CWD);
            size_t arg_len = strlen(DAT_0001a9e4);

            if (cwd_len + arg_len + 1 >= sizeof(path_buffer)) {
                puts("command too long");
                result = -1;
            } else {
                strcpy(path_buffer, CWD);
                strcat(path_buffer, DAT_0001a9e4);
                result = unlink(path_buffer);
            }
        }
    } else {
        puts("usage: delete <file>");
        result = -1;
    }
    return result;
}

// Function: EchoHandler
int EchoHandler(void) {
    int result = 0;
    char text_buffer[256];
    char file_path_buffer[256];
    FILE *file_ptr = NULL;
    int unescaped_len;

    if (ARGC < 2 || ARGC == 3 || ARGC > 4) { // Invalid number of arguments
        puts("usage: echo <text> [>|>>] [file]");
        return 0xFFFFFFFF; // Error
    }

    // Always process the text to be echoed/written
    strncpy(text_buffer, DAT_0001a9e4, sizeof(text_buffer) - 1);
    text_buffer[sizeof(text_buffer) - 1] = '\0'; // Ensure null termination
    unescaped_len = Unescape(text_buffer);

    if (unescaped_len == -1) { // Assuming -1 for error from Unescape
        puts("Error unescaping text.");
        return 0xFFFFFFFF;
    }

    if (ARGC == 2) { // echo <text> -> print to stdout
        puts(text_buffer);
        return 0;
    }

    // ARGC == 4: echo <text> > file or echo <text> >> file
    // Construct file path
    if (DAT_0001a9ec[0] == '/') { // Absolute path
        if (strlen(DAT_0001a9ec) >= sizeof(file_path_buffer)) {
            puts("invalid file name (too long)");
            return 0xFFFFFFFF;
        }
        strcpy(file_path_buffer, DAT_0001a9ec);
    } else { // Relative path
        size_t cwd_len = strlen(CWD);
        size_t arg_len = strlen(DAT_0001a9ec);
        if (cwd_len + arg_len + 1 >= sizeof(file_path_buffer)) {
            puts("command too long");
            return 0xFFFFFFFF;
        }
        strcpy(file_path_buffer, CWD);
        strcat(file_path_buffer, DAT_0001a9ec);
    }

    // Determine write mode (overwrite or append)
    if (strcmp(DAT_0001a9e8, ">") == 0) {
        file_ptr = fopen(file_path_buffer, "w");
    } else if (strcmp(DAT_0001a9e8, ">>") == 0) {
        file_ptr = fopen(file_path_buffer, "a");
    } else {
        puts("usage: echo <text> [>|>>] [file]");
        return 0xFFFFFFFF;
    }

    if (file_ptr == NULL) {
        perror("unable to write to file");
        return 0xFFFFFFFF;
    }

    // Write the unescaped text to the file
    if (fwrite(text_buffer, 1, unescaped_len, file_ptr) != (size_t)unescaped_len) {
        puts("Error writing to file.");
        result = 0xFFFFFFFF;
    }

    fclose(file_ptr);
    return result;
}

// Function: CatHandler
int CatHandler(void) {
    int result = 0;
    char path_buffer[256];

    if (ARGC == 2) {
        if (DAT_0001a9e4[0] == '/') { // Absolute path
            result = ReadFile(DAT_0001a9e4);
        } else { // Relative path
            size_t cwd_len = strlen(CWD);
            size_t arg_len = strlen(DAT_0001a9e4);

            if (cwd_len + arg_len + 1 >= sizeof(path_buffer)) {
                puts("command too long");
                result = -1;
            } else {
                strcpy(path_buffer, CWD);
                strcat(path_buffer, DAT_0001a9e4);
                result = ReadFile(path_buffer);
            }
        }
    } else {
        puts("usage: dump <file>");
        result = -1;
    }
    return result;
}

// Function: HelpHandler
int HelpHandler(void) {
    char **cmd_entry;

    for (cmd_entry = cmds; *cmd_entry != NULL; cmd_entry += 3) {
        // Print command name left-justified in a 20-character field, then the description
        printf("%-20s%s\n", *cmd_entry, cmd_entry[2]);
    }
    return 0;
}

// Function: DfHandler
int DfHandler(void) {
    if (ARGC == 1) {
        StatusFS();
    } else {
        puts("usage: fsinfo");
        return -1; // Indicate error for wrong usage
    }
    return 0;
}

// --- Main function and command dispatch ---

// Structure to map command names to their handler functions
typedef int (*CommandHandler)(void);
struct Command {
    const char *name;
    CommandHandler handler;
    // const char *description; // Description is in the 'cmds' array for HelpHandler
};

struct Command command_table[] = {
    {"mkfolder", MkdirHandler},
    {"delfolder", RmdirHandler},
    {"show", LsHandler},
    {"delete", RmHandler},
    {"echo", EchoHandler},
    {"dump", CatHandler},
    {"fsinfo", DfHandler},
    {"help", HelpHandler},
    {"bye", ExitHandler},
    {NULL, NULL} // Sentinel
};

int main(int argc, char *argv[]) {
    // Initialize global ARGC and DAT_ variables from main's arguments.
    // The handler functions expect ARGC to be the total count of arguments
    // passed to the shell, and DAT_ variables to point to argv[1], argv[2] etc.
    ARGC = argc;
    DAT_0001a9e4 = (argc > 1) ? argv[1] : NULL;
    DAT_0001a9e8 = (argc > 2) ? argv[2] : NULL;
    DAT_0001a9ec = (argc > 3) ? argv[3] : NULL;

    // Initialize CWD (Current Working Directory)
    char cwd_buffer[256];
    if (getcwd(cwd_buffer, sizeof(cwd_buffer)) != NULL) {
        CWD = strdup(cwd_buffer); // Allocate memory for CWD
        if (CWD == NULL) {
            perror("Failed to allocate memory for CWD");
            return 1;
        }
    } else {
        perror("Failed to get current working directory");
        return 1;
    }

    int result = 0;

    if (argc < 2) {
        puts("Usage: shell <command> [args...]");
        HelpHandler(); // Show help if no command is given
        result = 1;
    } else {
        // Command dispatch: find and execute the corresponding handler
        const char *command_name = argv[1];
        struct Command *cmd = command_table;
        int found = 0;

        for (; cmd->name != NULL; ++cmd) {
            if (strcmp(command_name, cmd->name) == 0) {
                found = 1;
                // For internal consistency, the global ARGC/DAT_ variables
                // are set relative to the *shell* invocation, not the internal command.
                // This matches the original code's logic (e.g., ARGC == 2 for `mkfolder <dir>`).
                // So, no adjustment is needed here.
                result = cmd->handler();
                break;
            }
        }

        if (!found) {
            printf("Unknown command: %s\n", command_name);
            HelpHandler();
            result = 1;
        }
    }

    free(CWD); // Free the dynamically allocated CWD buffer
    return result;
}