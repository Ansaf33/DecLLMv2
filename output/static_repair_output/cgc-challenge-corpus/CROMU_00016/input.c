#include <stdint.h> // For uint32_t and uint8_t
#include <stdio.h>  // For printf
#include <string.h> // For strlen and memset

// Define types
typedef uint32_t uint;
typedef uint8_t byte;

// --- Mock `receive` function for compilation and demonstration ---
// In a real application, this would be a system call or a library function.
// This mock function simulates reading from a static buffer.
static const char *mock_input_buffer = "hello world\nThis is a test character:X\nAnother line.";
static size_t mock_input_pos = 0;
static size_t mock_input_len = 0;

// Mock implementation of receive.
// Arguments: fd (ignored), buf, len, bytes_received_ptr
// Returns 0 on success, -1 on error (or no more data).
int receive(int fd, void *buf, size_t len, int *bytes_received_ptr) {
    (void)fd; // Unused parameter

    if (mock_input_len == 0) {
        mock_input_len = strlen(mock_input_buffer);
    }

    if (mock_input_pos < mock_input_len) {
        if (len >= 1) {
            *((byte *)buf) = mock_input_buffer[mock_input_pos];
            mock_input_pos++;
            *bytes_received_ptr = 1;
            return 0; // Success
        } else {
            *bytes_received_ptr = 0;
            return -1; // Error: len is too small
        }
    } else {
        *bytes_received_ptr = 0;
        return -1; // End of mock input
    }
}
// --- End of mock `receive` function ---

// Function: readUntil
uint readUntil(byte *param_1, uint param_2, char param_3) {
    uint i;
    int bytes_received;
    byte received_byte;

    for (i = 0; i < param_2; ++i) {
        if (receive(0, &received_byte, 1, &bytes_received) != 0) {
            return 0xffffffff; // Error from receive (or end of data)
        }
        if (bytes_received != 1) {
            return 0xffffffff; // Did not receive exactly one byte
        }
        if (received_byte == (byte)param_3) {
            break;
        }
        *param_1++ = received_byte;
    }
    *param_1 = 0; // Null-terminate the buffer
    return i;
}

// Main function for compilation and testing
int main() {
    byte buffer[50];
    uint max_len = sizeof(buffer) - 1; // Reserve one byte for null terminator
    char delimiter;
    uint bytes_read;

    // Test Case 1: Delimiter 'X' found
    printf("--- Test Case 1: Read until 'X' ---\n");
    memset(buffer, 0, sizeof(buffer)); // Clear buffer
    mock_input_pos = 0; // Reset mock input
    mock_input_len = 0; // Re-calculate length on first call to receive
    delimiter = 'X';
    bytes_read = readUntil(buffer, max_len, delimiter);
    if (bytes_read == 0xffffffff) {
        printf("readUntil failed or encountered an error.\n");
    } else {
        printf("readUntil returned: %u bytes read.\n", bytes_read);
        printf("Buffer content: \"%s\"\n", buffer);
    }

    // Test Case 2: Delimiter '\n' found
    printf("\n--- Test Case 2: Read until '\\n' ---\n");
    memset(buffer, 0, sizeof(buffer));
    mock_input_pos = 0;
    mock_input_len = 0;
    delimiter = '\n';
    bytes_read = readUntil(buffer, max_len, delimiter);
    if (bytes_read == 0xffffffff) {
        printf("readUntil failed or encountered an error.\n");
    } else {
        printf("readUntil returned: %u bytes read.\n", bytes_read);
        printf("Buffer content: \"%s\"\n", buffer);
    }

    // Test Case 3: Delimiter 'Z' not found within max_len
    printf("\n--- Test Case 3: Read until 'Z' (not found) ---\n");
    memset(buffer, 0, sizeof(buffer));
    mock_input_pos = 0;
    mock_input_len = 0;
    delimiter = 'Z';
    bytes_read = readUntil(buffer, max_len, delimiter);
    if (bytes_read == 0xffffffff) {
        printf("readUntil failed or encountered an error.\n");
    } else {
        printf("readUntil returned: %u bytes read.\n", bytes_read);
        printf("Buffer content: \"%s\"\n", buffer);
    }

    // Test Case 4: max_len is hit before delimiter
    printf("\n--- Test Case 4: Read max 5 chars (delimiter 'X' not reached) ---\n");
    memset(buffer, 0, sizeof(buffer));
    mock_input_pos = 0;
    mock_input_len = 0;
    delimiter = 'X';
    max_len = 5;
    bytes_read = readUntil(buffer, max_len, delimiter);
    if (bytes_read == 0xffffffff) {
        printf("readUntil failed or encountered an error.\n");
    } else {
        printf("readUntil returned: %u bytes read.\n", bytes_read);
        printf("Buffer content: \"%s\"\n", buffer);
    }

    return 0;
}