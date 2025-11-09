#include <unistd.h> // For read, ssize_t
#include <stddef.h> // For size_t

// Function: splitat
// Reads characters from a file descriptor into a buffer until a delimiter is found,
// the maximum length is reached, or EOF occurs.
// param_1 (fd): File descriptor to read from (e.g., 0 for stdin).
// param_2 (buffer): Character buffer to store the read data.
// param_3 (max_len): Maximum number of characters to read (including null terminator).
// param_4 (delimiter): The character to stop reading at.
// Returns: The number of characters read before the null terminator (or delimiter),
//          or -1 on error (e.g., read error or reading an explicit null byte).
ssize_t splitat(int fd, char *buffer, size_t max_len, char delimiter) {
    size_t i;
    ssize_t bytes_read_count;

    // Loop up to max_len - 1 to ensure there's always space for the null terminator.
    for (i = 0; i < max_len - 1; ++i) {
        bytes_read_count = read(fd, &buffer[i], 1); // Read one byte into the buffer

        if (bytes_read_count == -1) {
            // Error occurred during the read operation.
            return -1;
        }
        if (bytes_read_count == 0) {
            // End Of File (EOF) reached.
            // Null-terminate the string and return the length read so far.
            buffer[i] = '\0';
            return i;
        }

        // If a non-EOF read operation returns a null byte, treat it as an error.
        if (buffer[i] == '\0') {
            return -1;
        }

        // Check if the current character is the delimiter.
        if (buffer[i] == delimiter) {
            break; // Delimiter found, exit the loop.
        }
    }

    // Null-terminate the string at the current position 'i'.
    // 'i' will be the index of the delimiter or max_len-1 if no delimiter was found.
    buffer[i] = '\0';
    return i; // Return the number of characters successfully read/processed.
}

// Function: read_line
// Reads a line from standard input (file descriptor 0) into the provided buffer.
// param_1 (buffer): Character buffer to store the line read from stdin.
// Returns: 0 on success, or -1 on error.
int read_line(char *buffer) {
    // Call splitat to read a line from stdin (fd 0) into the buffer.
    // Max length is 0x1000 (4096 bytes), delimiter is newline (ASCII 10).
    ssize_t result = splitat(0, buffer, 0x1000, '\n');

    // Translate the splitat result to the expected read_line return value.
    if (result < 0) {
        return -1; // Return -1 for error.
    } else {
        return 0;  // Return 0 for success.
    }
}