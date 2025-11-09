#include <stdio.h>    // For puts, printf, getchar, fflush
#include <string.h>   // For strlen, memset
#include <stdlib.h>   // For exit
#include <stdbool.h>  // For bool
#include <unistd.h>   // For read, write, STDIN_FILENO, STDOUT_FILENO

// Stub for transmit_all
// Assumes it writes to a given file descriptor (e.g., STDOUT_FILENO)
// Returns 0 on success, -1 on failure
int transmit_all(int fd, const char *message, size_t length) {
    if (message == NULL) {
        return -1;
    }
    ssize_t bytes_written = write(fd, message, length);
    if (bytes_written == -1 || (size_t)bytes_written != length) {
        return -1; // Error during write or incomplete write
    }
    return 0; // Success
}

// Stub for receive_delim
// Assumes it reads from a given file descriptor (e.g., STDIN_FILENO)
// into a buffer, up to max_len, until delimiter or EOF.
// Returns 0 on success, -1 on failure (EOF or read error).
// The delimiter is included in the buffer if read.
int receive_delim(int fd, char *buffer, size_t max_len, char delimiter) {
    if (buffer == NULL || max_len == 0) {
        return -1;
    }

    size_t i = 0;
    char c;
    ssize_t bytes_read;

    while (i < max_len - 1) { // Leave space for null terminator
        bytes_read = read(fd, &c, 1);
        if (bytes_read == -1) {
            return -1; // Read error
        }
        if (bytes_read == 0) { // EOF
            break;
        }
        buffer[i++] = c;
        if (c == delimiter) {
            break;
        }
    }
    buffer[i] = '\0'; // Null-terminate the string

    if (i == 0 && bytes_read == 0) { // Nothing read and EOF encountered
        return -1;
    }
    return 0; // Success
}

// Function: check
int check(void) {
    char input_buffer[64]; // Max 63 characters + null terminator
    
    // Clear the buffer
    memset(input_buffer, 0, sizeof(input_buffer));

    // Receive input from stdin (fd 0), up to sizeof(input_buffer) - 1 characters, until newline (10)
    // The original code used 0x80 (128) as max_len with a 64-byte buffer, which is a bug.
    // Fixed to use sizeof(input_buffer) to prevent buffer overflow.
    if (receive_delim(STDIN_FILENO, input_buffer, sizeof(input_buffer), '\n') != 0) {
        return -1; // Indicate error or EOF
    }

    size_t len = strlen(input_buffer);

    // Palindrome check logic
    // The original code includes the newline character in the palindrome check if present.
    // This implementation retains that behavior.
    bool is_palindrome = true;
    if (len > 0) { // Only check if string is not empty
        for (size_t i = 0; i < len / 2; ++i) {
            if (input_buffer[i] != input_buffer[len - 1 - i]) {
                is_palindrome = false;
                break;
            }
        }
    } else {
        // An empty string is typically considered a palindrome.
        // The original loop for length 0 would set local_10 to -1, then loop wouldn't run,
        // local_18 (result) would remain 1.
        is_palindrome = true;
    }

    // Easter egg check
    // If the first character is '^' and transmit_all fails, terminate.
    if (len > 0 && input_buffer[0] == '^') {
        if (transmit_all(STDOUT_FILENO, "\n\nEASTER EGG!\n\n", strlen("\n\nEASTER EGG!\n\n")) != 0) {
            exit(0); // Original code called _terminate(0) on success of transmit_all for easter egg.
                     // The condition was `(iVar1 = transmit_all(1, ..., 0xf), iVar1 != 0)`
                     // which means exit if transmit_all fails.
        }
    }

    return is_palindrome ? 1 : 0;
}

// Function: main
int main(void) {
    // Initial welcome message
    if (transmit_all(STDOUT_FILENO, "\nWelcome to Palindrome Finder\n\n", strlen("\nWelcome to Palindrome Finder\n\n")) != 0) {
        exit(1); // Exit on transmit error
    }

    // Main loop for palindrome finding, replacing the original goto
    while (true) {
        // Prompt for input
        if (transmit_all(STDOUT_FILENO, "\tPlease enter a possible palindrome: ", strlen("\tPlease enter a possible palindrome: ")) != 0) {
            exit(1); // Exit on transmit error
        }

        int check_result = check();

        if (check_result == -1) {
            // Error during receive_delim or EOF encountered
            return 0; // Original code returned 0 here, indicating a graceful exit after input error.
        }

        if (check_result == 0) {
            // Not a palindrome
            if (transmit_all(STDOUT_FILENO, "\t\tNope, that's not a palindrome\n\n", strlen("\t\tNope, that's not a palindrome\n\n")) != 0) {
                exit(1); // Exit on transmit error
            }
        } else { // check_result == 1
            // Is a palindrome
            if (transmit_all(STDOUT_FILENO, "\t\tYes, that's a palindrome!\n\n", strlen("\t\tYes, that's a palindrome!\n\n")) != 0) {
                exit(1); // Exit on transmit error
            }
        }
        // The loop continues indefinitely as per the original do-while(true) with goto.
    }
    // This part is unreachable due to while(true)
    // return 0;
}