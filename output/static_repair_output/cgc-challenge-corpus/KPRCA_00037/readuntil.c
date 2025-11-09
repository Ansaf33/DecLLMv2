#include <unistd.h>    // For read, STDIN_FILENO
#include <sys/types.h> // For ssize_t
#include <stddef.h>    // For size_t
#include <stdio.h>     // For printf, perror, fflush
#include <stdlib.h>    // For EXIT_SUCCESS
#include <string.h>    // For strlen

// Function: read_until
// Reads from file descriptor `fd` into `buffer` until `separator` is found or `max_len` bytes are read.
// The character at the position of the separator (or the last character read if no separator)
// is overwritten with a null terminator.
// Returns the total number of bytes read (including the separator if found), or -1 on error/EOF.
ssize_t read_until(int fd, char *buffer, size_t max_len, char separator) {
    char *current_pos = buffer;
    ssize_t bytes_read_total = 0;
    ssize_t n_read;

    for (size_t i = 0; i < max_len; ++i) {
        n_read = read(fd, current_pos, 1);
        if (n_read <= 0) { // Error (-1) or EOF (0)
            return -1; // Original code implies -1 for both error and EOF encountered during read.
        }
        bytes_read_total++;
        if (*current_pos == separator) {
            break;
        }
        current_pos++;
    }

    // Original logic: if any bytes were read, null-terminate the last character read.
    // After the loop, `current_pos` points to `buffer + bytes_read_total`.
    // So `current_pos - 1` points to `buffer + bytes_read_total - 1`, which is the last character read.
    if (bytes_read_total > 0) {
        *(current_pos - 1) = '\0';
    }
    return bytes_read_total;
}

// Function: read_n
// Reads up to `count` bytes from file descriptor `fd` into `buffer`.
// Stops early on error or EOF.
// Returns the number of bytes successfully read.
// If an error or EOF occurs on the very first read, it returns -1 or 0 respectively.
ssize_t read_n(int fd, char *buffer, size_t count) {
    char *current_pos = buffer;
    ssize_t bytes_read_total = 0;
    ssize_t n_read;

    for (size_t i = 0; i < count; ++i) {
        n_read = read(fd, current_pos, 1);
        if (n_read <= 0) { // Error (-1) or EOF (0)
            if (bytes_read_total == 0) {
                return n_read; // Return -1 for error, 0 for EOF, if nothing was read yet.
            }
            break; // Stop reading, return bytes_read_total (partial read).
        }
        bytes_read_total++;
        current_pos++;
    }
    return bytes_read_total;
}

// Main function for compilation and demonstration
#define BUFFER_SIZE 256

int main() {
    char buffer[BUFFER_SIZE];
    ssize_t bytes;

    printf("--- Testing read_until ---\n");
    printf("Enter text (will read until 'X' or %zu chars, last char overwritten with null): \n", BUFFER_SIZE - 1);
    fflush(stdout); // Ensure prompt is displayed before read
    bytes = read_until(STDIN_FILENO, buffer, BUFFER_SIZE - 1, 'X');
    if (bytes == -1) {
        perror("read_until error");
    } else if (bytes == 0) {
        printf("read_until: No bytes read.\n");
    } else {
        printf("read_until: Read %zd bytes. Content (length %zu): \"%s\"\n", bytes, strlen(buffer), buffer);
    }

    printf("\n--- Testing read_n ---\n");
    printf("Enter text (will read up to 10 chars): \n");
    fflush(stdout);
    bytes = read_n(STDIN_FILENO, buffer, 10);
    if (bytes == -1) {
        perror("read_n error");
    } else {
        buffer[bytes] = '\0'; // Null-terminate for printing, as read_n doesn't do it.
        printf("read_n: Read %zd bytes. Content: \"%s\"\n", bytes, buffer);
    }

    return EXIT_SUCCESS;
}