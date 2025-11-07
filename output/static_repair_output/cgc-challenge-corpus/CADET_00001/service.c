#include <stdio.h>   // For standard I/O if needed, though transmit_all/receive_delim replace direct use
#include <string.h>  // For strlen, memset
#include <stdlib.h>  // For exit

// Declare external functions.
// These are assumed to be provided by the environment or a linked library.
// transmit_all: Writes 'len' bytes from 'buf' to file descriptor 'fd'. Returns 0 on success, non-zero on error.
int transmit_all(int fd, const char *buf, int len);
// receive_delim: Reads into 'buf' from file descriptor 'fd' up to 'max_len' bytes or until 'delimiter' is found.
//                Null-terminates 'buf'. Returns 0 on success, non-zero on error.
int receive_delim(int fd, char *buf, int max_len, char delimiter);
// _terminate: Exits the program with the given status.
void _terminate(int status);

// Function: check
int check(void) {
    char input_buffer[64];
    int is_palindrome = 1; // 1: true, 0: false, -1: error
    int len;
    int i;

    // Initialize buffer to nulls to ensure clean state
    memset(input_buffer, 0, sizeof(input_buffer));

    // Receive input from stdin (fd 0), delimited by newline (ASCII 10).
    // Max length is sizeof(input_buffer) - 1 to ensure space for null terminator.
    if (receive_delim(0, input_buffer, sizeof(input_buffer) - 1, 10) != 0) {
        return -1; // Indicate an error in receiving input
    }

    // Get the length of the string after receiving
    len = strlen(input_buffer);

    // Palindrome check: compare characters from start and end towards the middle.
    // The original decompiler output for array indexing was likely incorrect;
    // this implements a standard, correct palindrome check.
    for (i = 0; i < len / 2; ++i) {
        if (input_buffer[i] != input_buffer[len - 1 - i]) {
            is_palindrome = 0; // Not a palindrome
            break;             // No need to check further
        }
    }

    // Easter egg check: if the first character is '^'
    // The original logic terminated if transmit_all failed. Keeping this behavior.
    if (input_buffer[0] == '^') {
        if (transmit_all(1, "\n\nEASTER EGG!\n\n", 0xf) != 0) {
            _terminate(0); // Terminate if the Easter Egg message transmission fails
        }
    }

    return is_palindrome;
}

// Function: main
int main(void) {
    int check_result;

    // Display welcome message
    // fd 1 is stdout, 0x1f (31) is the length of the string
    if (transmit_all(1, "\nWelcome to Palindrome Finder\n\n", 0x1f) != 0) {
        _terminate(0); // Terminate if welcome message transmission fails
    }

    // Main program loop: repeatedly prompt for and check palindromes
    while (1) {
        // Prompt user for input
        // fd 1 is stdout, 0x25 (37) is the length of the string
        if (transmit_all(1, "\tPlease enter a possible palindrome: ", 0x25) != 0) {
            _terminate(0); // Terminate if prompt transmission fails
        }

        // Call the check function to process input and determine if it's a palindrome
        check_result = check();

        // Handle the result from the check function
        if (check_result == -1) {
            // An error occurred during input reception in check()
            return 0; // Exit main gracefully as requested by original code's `return 0;` for -1
        } else if (check_result == 0) {
            // The input is not a palindrome
            // fd 1 is stdout, 0x21 (33) is the length of the string
            if (transmit_all(1, "\t\tNope, that\'s not a palindrome\n\n", 0x21) != 0) {
                _terminate(0); // Terminate if message transmission fails
            }
        } else { // check_result == 1
            // The input is a palindrome
            // fd 1 is stdout, 0x1d (29) is the length of the string
            if (transmit_all(1, "\t\tYes, that\'s a palindrome!\n\n", 0x1d) != 0) {
                _terminate(0); // Terminate if message transmission fails
            }
        }
        // Loop continues for the next palindrome check
    }

    // This part of the code is unreachable because of the infinite while(1) loop,
    // but a main function typically returns 0 on successful termination.
    return 0;
}