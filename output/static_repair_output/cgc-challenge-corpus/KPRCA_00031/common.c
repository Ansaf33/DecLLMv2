#include <stddef.h> // For size_t
#include <string.h> // For strchr, memcpy, memset
#include <unistd.h> // For read, STDIN_FILENO
#include <stdio.h>  // For printf, fprintf
#include <stdlib.h> // For EXIT_SUCCESS, EXIT_FAILURE

// Custom receive function stub.
// Assumes it works like `recv` but returns 0 for success and -1 for error,
// with bytes read via `out_bytes_read`.
// For compilation, it uses `read` from unistd.h, assuming `fd` is a generic file descriptor.
int receive(int fd, char *buf, size_t max_len, size_t *out_bytes_read) {
    ssize_t ret = read(fd, buf, max_len);
    if (ret < 0) {
        *out_bytes_read = 0;
        return -1; // Error
    } else {
        *out_bytes_read = (size_t)ret;
        return 0; // Success
    }
}

// Function: readline
// Reads a line from a file descriptor (sockfd) into a buffer.
// Parameters:
//   sockfd: The file descriptor to read from.
//   buffer: The destination buffer to store the line.
//   max_len: The maximum number of bytes to read into the buffer (target buffer capacity).
//   total_read_count_ptr: A pointer to a size_t variable that tracks total bytes read for the current line.
// Returns:
//   0 on success (line read, or connection closed gracefully with no more data).
//   -1 (representing 0xffffffff) on error.
int readline(int sockfd, char *buffer, size_t max_len, size_t *total_read_count_ptr) {
    char temp_buffer[1025]; // Temporary buffer for received data (0x401 bytes)
    size_t bytes_received;  // Bytes received in the current `receive` call
    char *newline_ptr;      // Pointer to the newline character in `temp_buffer`

    *total_read_count_ptr = 0; // Initialize total bytes read for this line

    do {
        // If the target buffer has no more space, return success (partial line or buffer full).
        if (max_len <= *total_read_count_ptr) {
            return 0;
        }

        // Attempt to receive data into `temp_buffer`. Max 1024 bytes (0x400).
        // The `receive` function is assumed to return 0 for success, -1 for error,
        // and populate `bytes_received` with the actual number of bytes read.
        if (receive(sockfd, temp_buffer, 1024, &bytes_received) < 0) {
            return -1; // Receive error
        }

        // If 0 bytes were received, it means the connection was closed or no data is available.
        if (bytes_received == 0) {
            return 0; // End of file/connection closed gracefully
        }

        // Search for a newline character in the received data.
        newline_ptr = strchr(temp_buffer, '\n');

        // Special error condition: if this is the first read and the first character is a newline.
        // This implies an empty line was received at the very beginning of the buffer.
        // The original code treated this as an error.
        if ((*total_read_count_ptr == 0) && (newline_ptr == temp_buffer)) {
            return -1; // Error: empty line received at the start
        }

        size_t current_segment_len = bytes_received; // Initially assume we copy all received bytes

        if (newline_ptr != NULL) {
            // If a newline is found, calculate bytes to copy up to (but not including) the newline.
            // The original code calculated `(int)local_14 - (int)local_415`, which is the offset.
            current_segment_len = (size_t)(newline_ptr - temp_buffer);
        }

        // Ensure we do not exceed the target buffer's capacity (`max_len`).
        // Adjust `current_segment_len` if the remaining space is insufficient.
        if (max_len < *total_read_count_ptr + current_segment_len) {
            current_segment_len = max_len - *total_read_count_ptr;
        }

        // Copy the determined number of bytes to the output buffer.
        memcpy(buffer, temp_buffer, current_segment_len);

        // Advance the output buffer pointer and update the total bytes read count.
        buffer += current_segment_len;
        *total_read_count_ptr += current_segment_len;

        // Clear the temporary buffer for the next receive operation.
        // Original code used 0x401 (1025) for memset, matching `temp_buffer[1025]`.
        memset(temp_buffer, 0, sizeof(temp_buffer));

    } while (newline_ptr == NULL); // Continue looping until a newline character is found

    return 0; // Successfully read a line (newline found and processed)
}

// Main function for demonstration and compilation.
int main() {
    char line_buffer[256];  // Buffer to store the line
    size_t total_bytes = 0; // To store the total bytes read by readline
    int result;

    printf("Enter lines of text (type 'quit' to exit):\n");
    printf("Lines will be read up to the newline character.\n");
    printf("Example: 'hello world\\n' will output 'hello world'.\n");

    // Loop to read multiple lines
    while (1) {
        // Reset buffer and total_bytes for each new line
        memset(line_buffer, 0, sizeof(line_buffer));
        total_bytes = 0;

        // Call readline to read a line from stdin (fd 0)
        // We pass sizeof(line_buffer) - 1 to leave space for a null terminator
        // that we will add manually, as readline itself does not null-terminate.
        result = readline(STDIN_FILENO, line_buffer, sizeof(line_buffer) - 1, &total_bytes);

        if (result == -1) {
            fprintf(stderr, "Error reading line.\n");
            return EXIT_FAILURE;
        }
        
        // If readline returns 0 and total_bytes is 0, it means EOF or no more data.
        if (total_bytes == 0 && result == 0) {
            printf("End of input.\n");
            break;
        }

        // Manually null-terminate the buffer for safe printing.
        // Ensure total_bytes does not exceed buffer capacity before null-terminating.
        if (total_bytes < sizeof(line_buffer)) {
            line_buffer[total_bytes] = '\0';
        } else {
            // This case should ideally not happen if max_len was correctly passed,
            // but as a safeguard, ensure null termination at the very end.
            line_buffer[sizeof(line_buffer) - 1] = '\0';
        }

        printf("Read %zu bytes: '%s'\n", total_bytes, line_buffer);

        // Check for exit condition (e.g., if user types "quit")
        if (strcmp(line_buffer, "quit") == 0) {
            break;
        }
    }

    return EXIT_SUCCESS;
}