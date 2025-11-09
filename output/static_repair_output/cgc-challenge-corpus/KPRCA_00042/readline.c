#include <stdio.h>   // For fgetc, stdin, printf, fflush, perror, feof
#include <stdlib.h>  // For EXIT_SUCCESS, EXIT_FAILURE
#include <string.h>  // For strcmp
#include <sys/types.h> // For ssize_t

// Mock receive function based on the original snippet's usage.
// It simulates reading one character at a time from stdin.
// In a real application, 'fd' would be a socket descriptor or file descriptor,
// and this function would wrap `read()` or `recv()`.
//
// Parameters:
//   fd: A file descriptor or socket descriptor (unused in this mock).
//   buffer: Pointer to the buffer to read into.
//   count: The maximum number of bytes to read (expected to be 1 for this readline).
//   bytes_read_ptr: Pointer to store the number of bytes actually read.
//
// Returns:
//   0 on success (even if 0 bytes were read due to EOF).
//   Non-zero on an error (e.g., if the read operation itself failed).
static int receive(int fd, char *buffer, size_t count, ssize_t *bytes_read_ptr) {
    if (count == 0) {
        *bytes_read_ptr = 0;
        return 0;
    }

    int c = fgetc(stdin); // Read one character from standard input

    if (c == EOF) {
        *bytes_read_ptr = 0; // Indicate no bytes read (EOF)
        return 0;            // No error, just end of stream
    }

    *buffer = (char)c;
    *bytes_read_ptr = 1; // Indicate one byte was successfully read
    return 0;            // Success
}

// Function: readline
// Reads a line from the specified file descriptor into a buffer.
// It stops when a newline character is encountered, the buffer is full,
// or an error/EOF occurs.
//
// Parameters:
//   fd: A file descriptor or socket descriptor.
//   buf: Pointer to the character buffer where the line will be stored.
//   maxlen: Maximum size of the buffer, including the null terminator.
//
// Returns:
//   0: Successfully read a line ending with '\n'.
//   1: Buffer full before '\n' was encountered. The buffer is null-terminated.
//  -1: Error during read operation or EOF/stream closed. The buffer is null-terminated if any data was read.
int readline(int fd, char *buf, size_t maxlen) {
    size_t i = 0; // Current index in the buffer
    ssize_t bytes_read; // Number of bytes actually read by receive()
    int receive_status; // Return status of receive()

    while (1) {
        // Check for buffer overflow *before* attempting to read the next character.
        // `maxlen` includes the null terminator. If `i` reaches `maxlen - 1`,
        // it's the last position for a character, and the null terminator will go
        // at that same position if the buffer becomes full.
        if (i >= maxlen) {
            if (maxlen > 0) {
                buf[maxlen - 1] = '\0'; // Null terminate at the last valid position for data
            }
            return 1; // Buffer full
        }

        // Attempt to read one byte
        receive_status = receive(fd, buf + i, 1, &bytes_read);

        // Check for errors or EOF
        if (receive_status != 0 || bytes_read == 0) {
            // If bytes_read is 0 and receive_status is 0, it indicates EOF.
            // If receive_status is non-zero, it indicates an error.
            if (i > 0 && i < maxlen) { // If some data was read, null-terminate it
                buf[i] = '\0';
            } else if (maxlen > 0) { // If buffer is empty, ensure it's null-terminated
                buf[0] = '\0';
            }
            return -1; // Error or EOF
        }

        // Check if the received character is a newline
        if (buf[i] == '\n') {
            buf[i] = '\0'; // Null terminate at the newline's position
            return 0;      // Successfully read a line
        }

        i++; // Move to the next position in the buffer
    }
}

// Example main function to demonstrate usage of readline
int main() {
    char buffer[256]; // A buffer to store the read line
    int result;

    printf("Enter lines (type 'quit' to exit, max 255 chars per line):\n");

    while (1) {
        printf("> ");
        fflush(stdout); // Ensure prompt is displayed before input

        // Use file descriptor 0 for standard input
        result = readline(0, buffer, sizeof(buffer));

        if (result == 0) {
            printf("Read line (success): '%s'\n", buffer);
            if (strcmp(buffer, "quit") == 0) {
                break; // Exit loop if "quit" is entered
            }
        } else if (result == 1) {
            printf("Read line (buffer full): '%s' (truncated)\n", buffer);
        } else { // result == -1 (Error or EOF)
            if (feof(stdin)) {
                printf("EOF reached or input stream closed.\n");
            } else {
                perror("Error reading line");
            }
            break; // Exit loop on error or EOF
        }
    }

    return EXIT_SUCCESS;
}