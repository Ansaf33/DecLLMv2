#include <stdio.h>   // For printf, puts, getchar
#include <string.h>  // For strlen, memset
#include <stdlib.h>  // For exit

// --- Stubs for missing functions ---

// transmit_all: Simulates printing a message to stdout.
// Takes a flag (unused in this stub), a message string, and its length.
// Returns 0 on success, 1 on failure (e.g., if message is NULL).
int transmit_all(int flag, const char* message, int length) {
    if (message == NULL) {
        fprintf(stderr, "transmit_all: NULL message\n");
        return 1; // Failure
    }
    // Using printf with precision to print exactly 'length' characters.
    printf("%.*s", length, message);
    fflush(stdout); // Ensure output is flushed immediately
    return 0; // Success
}

// receive_delim: Simulates reading from stdin until a delimiter or max_len is reached.
// fd: file descriptor (0 for stdin).
// buffer: buffer to store the input.
// max_len: maximum number of characters to read (including null terminator).
// delimiter: character to stop reading at (e.g., '\n').
// Returns 0 on success, -1 on EOF or error.
int receive_delim(int fd, char* buffer, int max_len, char delimiter) {
    if (buffer == NULL || max_len <= 0) {
        fprintf(stderr, "receive_delim: Invalid arguments\n");
        return -1;
    }

    int i = 0;
    int c;

    // Read characters until max_len-1, EOF, or delimiter
    while (i < max_len - 1 && (c = getchar()) != EOF && c != delimiter) {
        buffer[i++] = (char)c;
    }

    buffer[i] = '\0'; // Null-terminate the string

    if (c == EOF && i == 0) {
        // If EOF was reached immediately and no characters were read
        return -1;
    }
    
    // If the delimiter was found, or we hit max_len-1, and not EOF, it's a success.
    // If EOF was found after reading some characters, that's also considered a success.
    return 0;
}

// _terminate: Exits the program.
// status: exit status code.
void _terminate(int status) {
    exit(status);
}

// Function: check
int check(void) {
    char buffer[64];
    int is_palindrome = 1; // Assume it's a palindrome initially
    int receive_status;
    
    // Initialize buffer to null characters
    memset(buffer, 0, sizeof(buffer));

    // Receive input, cap length to buffer size - 1 for null terminator.
    // Assuming 10 is ASCII for newline ('\n').
    receive_status = receive_delim(0, buffer, sizeof(buffer), '\n');

    if (receive_status == 0) {
        // Successfully received input
        size_t len = strlen(buffer);

        // Standard palindrome check logic
        for (size_t i = 0; i < len / 2; ++i) {
            if (buffer[i] != buffer[len - 1 - i]) {
                is_palindrome = 0; // Not a palindrome
                break;
            }
        }

        // Easter egg check: if the first character is '^'
        if (len > 0 && buffer[0] == '^') {
            // Check if transmit_all fails for the Easter Egg message
            // 0xf is 15, the length of "\n\nEASTER EGG!\n\n"
            if (transmit_all(1, "\n\nEASTER EGG!\n\n", 0xf) != 0) {
                _terminate(0); // Terminate if transmit fails
            }
        }
    } else {
        // receive_delim failed (e.g., EOF or error)
        is_palindrome = -1; // Special return value indicating an issue
    }
    return is_palindrome;
}

// Function: main
int main(void) {
    int transmit_status;
    int check_result;

    // Initial welcome message
    // 0x1f is 31, the length of "\nWelcome to Palindrome Finder\n\n"
    transmit_status = transmit_all(1, "\nWelcome to Palindrome Finder\n\n", 0x1f);
    if (transmit_status != 0) {
        _terminate(0);
    }

    // Main loop for palindrome checking (replaces the do-while loop with goto)
    while (1) {
        // Prompt for input
        // 0x25 is 37, the length of "\tPlease enter a possible palindrome: "
        transmit_status = transmit_all(1, "\tPlease enter a possible palindrome: ", 0x25);
        if (transmit_status != 0) {
            _terminate(0);
        }

        check_result = check(); // Call the check function

        if (check_result == -1) {
            // Error during receive_delim or EOF, exit program.
            // Original decompiled code returned 0 here.
            return 0; 
        } else if (check_result == 0) {
            // Not a palindrome
            // 0x21 is 33, the length of "\t\tNope, that\'s not a palindrome\n\n"
            transmit_status = transmit_all(1, "\t\tNope, that\'s not a palindrome\n\n", 0x21);
            if (transmit_status != 0) {
                _terminate(0);
            }
            // Loop continues for another input
        } else { // check_result == 1
            // It is a palindrome
            // 0x1d is 29, the length of "\t\tYes, that\'s a palindrome!\n\n"
            transmit_status = transmit_all(1, "\t\tYes, that\'s a palindrome!\n\n", 0x1d);
            if (transmit_status != 0) {
                _terminate(0);
            }
            // Loop continues for another input
        }
    }

    // This part is theoretically unreachable due to the infinite while(1) loop,
    // but included for completeness if a specific exit condition were added.
    return 0;
}