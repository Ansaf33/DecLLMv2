#include <sys/socket.h> // For recv, socket types
#include <unistd.h>     // For close, etc.
#include <stdio.h>      // For NULL, basic I/O (though not strictly used by this snippet)
#include <stdint.h>     // For specific integer types if preferred (e.g., uint32_t)

// Forward declarations
// Assuming 'handle_command' is defined elsewhere and takes a char* buffer and its length.
void handle_command(char *command_buffer, unsigned int command_len);

// Function: receive_wrapper
// This function mimics the behavior suggested by the original snippet's usage of 'receive'.
// It returns 0 on success, -1 on error, and sets *bytes_received_ptr to the actual bytes read.
int receive_wrapper(int sockfd, void *buf, size_t len, int *bytes_received_ptr) {
    ssize_t bytes_read = recv(sockfd, buf, len, 0);
    if (bytes_read == -1) {
        *bytes_received_ptr = 0; // Indicate no bytes read on actual socket error
        return -1; // Indicate an error from this wrapper function
    }
    *bytes_received_ptr = (int)bytes_read;
    return 0; // Indicate success (0 bytes read implies EOF, 1 byte read implies data)
}

// Function: read_n
// Reads up to 'max_len' bytes into 'buf' from 'sockfd'.
// Stops on error, EOF, or after reading 'max_len' bytes.
// Returns the number of bytes successfully read.
int read_n(int sockfd, char *buf, unsigned int max_len) {
    unsigned int i;
    int receive_ret_code;
    int bytes_read_val;
    char *current_buf_ptr = buf;

    for (i = 0; i < max_len; ++i) {
        receive_ret_code = receive_wrapper(sockfd, current_buf_ptr, 1, &bytes_read_val);
        // Break if receive_wrapper indicates an error (non-zero ret_code)
        // or if 0 bytes were read (EOF on socket).
        if ((receive_ret_code != 0) || (bytes_read_val == 0)) {
            break;
        }
        current_buf_ptr++;
    }
    return (int)(current_buf_ptr - buf); // Return total bytes read
}

// Function: read_until
// Reads bytes into 'buf' from 'sockfd' until 'delimiter' is found, 'max_len' is reached,
// or an error/EOF occurs.
// Returns the number of bytes read (including the delimiter, which is replaced by '\0'),
// or -1 on error/EOF before any byte is read.
int read_until(int sockfd, char *buf, unsigned int max_len, char delimiter) {
    unsigned int i;
    int receive_ret_code;
    int bytes_read_val;
    char *current_buf_ptr = buf;

    for (i = 0; i < max_len; ++i) {
        receive_ret_code = receive_wrapper(sockfd, current_buf_ptr, 1, &bytes_read_val);
        // If an error or EOF occurs, return -1 as per original logic.
        if ((receive_ret_code != 0) || (bytes_read_val == 0)) {
            return -1;
        }
        if (delimiter == *current_buf_ptr) {
            current_buf_ptr++; // Move past the delimiter
            break; // Stop reading
        }
        current_buf_ptr++;
    }

    // Null-terminate the string.
    // current_buf_ptr points one byte past the last byte read (which could be the delimiter).
    // So current_buf_ptr[-1] refers to the last byte read.
    // This correctly replaces the delimiter or the last character if max_len was reached.
    // Ensure at least one byte was processed to avoid out-of-bounds access if current_buf_ptr == buf.
    if (current_buf_ptr > buf) {
        current_buf_ptr[-1] = '\0';
    }
    return (int)(current_buf_ptr - buf);
}

// Function: main
int main(void) {
    int bytes_read_count;
    char command_buffer[8192]; // Buffer to store incoming commands
    unsigned int command_len;   // Variable to store the 4-byte length prefix

    // In a real application, 'sockfd' would be obtained from `socket()` and `accept()` or `connect()`.
    // Using 0 (stdin) as a placeholder for demonstration.
    int sockfd = 0;

    // Main loop for receiving and processing commands.
    // Continues as long as a valid 4-byte length prefix can be read,
    // and the indicated command length is within buffer limits.
    while (1) {
        // Attempt to read a 4-byte length prefix into 'command_len'.
        bytes_read_count = read_n(sockfd, (char*)&command_len, sizeof(command_len));

        // If not exactly 4 bytes were read, it signifies an error or EOF on the socket.
        if (bytes_read_count != sizeof(command_len)) {
            break; // Exit the loop.
        }

        // Validate the received command length.
        // It must be non-zero and fit within the 'command_buffer'.
        if (command_len == 0 || command_len >= sizeof(command_buffer)) {
            // Handle invalid length (e.g., protocol error, potential attack).
            // For this example, we break the loop.
            break;
        }

        // Read the actual command data based on the 'command_len'.
        bytes_read_count = read_n(sockfd, command_buffer, command_len);

        // If the command was successfully read (more than 0 bytes), process it.
        if (bytes_read_count > 0) {
            handle_command(command_buffer, (unsigned int)bytes_read_count);
        } else {
            // If read_n returned 0 or less, it indicates an error or EOF during command data reading.
            break; // Exit the loop.
        }
    }
    return 0;
}

// Dummy implementation for handle_command to allow compilation.
// In a real program, this function would parse and execute the received command.
void handle_command(char *command_buffer, unsigned int command_len) {
    // Example: Print the received command (for debugging/demonstration)
    // printf("Received command (len %u): %.*s\n", command_len, command_len, command_buffer);
}