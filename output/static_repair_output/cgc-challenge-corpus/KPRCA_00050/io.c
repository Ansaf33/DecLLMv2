#include <stddef.h>  // For size_t
#include <stdbool.h> // For bool and true
#include <string.h>  // For strlen

// Global key variables (assuming they are 1-byte unsigned integers)
unsigned char ikey;
unsigned char okey;

// Mock implementations for compilation. In a real scenario, these would
// be actual I/O functions (e.g., wrappers around recv/send or custom drivers).
// They mimic the return value behavior implied by the original snippet:
// `int` return for status (0 on success, non-zero on error).
// `size_t *` for the actual bytes transferred.
int receive(int fd, char *buf, size_t len, size_t *bytes_received) {
    (void)fd; // Unused parameter
    if (len == 0) {
        *bytes_received = 0;
        return 0; // Success, 0 bytes read
    }
    // Simulate receiving at least 1 byte if available, or less than `len`.
    // This allows `read_bytes` to potentially loop multiple times.
    *bytes_received = (len >= 1) ? 1 : 0; // Simulate reading 1 byte for simplicity
    if (*bytes_received > 0) {
        // Fill with dummy data for compilation.
        // In a real application, this would involve actual I/O.
        static unsigned char mock_input_char = 'A';
        buf[0] = mock_input_char++;
        if (mock_input_char > 'Z') mock_input_char = 'A'; // Cycle through alphabet
    }
    return 0; // 0 indicates success, as per snippet's iVar1 check
}

int transmit(int fd, const char *buf, size_t len, size_t *bytes_sent) {
    (void)fd;  // Unused parameter
    (void)buf; // Unused parameter
    if (len == 0) {
        *bytes_sent = 0;
        return 0; // Success, 0 bytes written
    }
    // Simulate transmitting at least 1 byte if available, or less than `len`.
    *bytes_sent = (len >= 1) ? 1 : 0; // Simulate writing 1 byte for simplicity
    return 0; // 0 indicates success, as per snippet's iVar1 check
}

// Function: read_bytes
// Reads `len` bytes into `buf`, applying XOR encryption and updating `ikey`.
// Returns 1 on complete success (all bytes read), 0 on failure or partial read.
int read_bytes(char *buf, size_t len) {
    size_t bytes_received_count;
    int status = 0; // Initialize status to success

    while (true) {
        if (len == 0) {
            return 1; // All requested bytes read successfully
        }
        status = receive(0, buf, len, &bytes_received_count);
        if (status != 0 || bytes_received_count == 0) {
            // Error from receive or EOF (0 bytes received)
            return 0; // Failure
        }
        for (size_t i = 0; i < bytes_received_count; ++i) {
            buf[i] = ikey ^ buf[i];
            ikey += buf[i]; // Update ikey with the modified byte
        }
        len -= bytes_received_count;
        buf += bytes_received_count; // Advance buffer pointer
    }
}

// Function: read_until
// Reads bytes into `buf` until `delimiter` is found or `max_len` bytes are read.
// Null-terminates the buffer. Returns 1 on success, 0 on I/O error.
int read_until(char *buf, size_t max_len, char delimiter) {
    size_t i; // Loop counter, needs to persist after loop

    for (i = 0; i < max_len; ++i) {
        // Read one byte into buf[i]
        int status = read_bytes(buf + i, 1);
        if (status == 0) { // read_bytes returns 0 on failure
            return 0; // Propagate failure
        }
        if (delimiter == buf[i]) {
            break; // Delimiter found, 'i' now holds its index
        }
    }

    // After the loop, 'i' will be either the index of the delimiter,
    // or 'max_len' if the delimiter was not found.
    if (i < max_len) { // Delimiter was found at index 'i'
        buf[i] = '\0'; // Null-terminate at the delimiter's position
    } else { // Delimiter was NOT found, loop completed (i == max_len)
        if (max_len > 0) { // Ensure there's space to null-terminate
            buf[max_len - 1] = '\0'; // Null-terminate at the end of the buffer
        }
    }
    return 1; // Success (either found delimiter or read max_len bytes)
}

// Function: write_bytes
// Writes `len` bytes from `buf`, applying XOR encryption and updating `okey`.
// `buf` must be a mutable buffer as its contents are modified before transmission.
void write_bytes(char *buf, size_t len) {
    // First loop: modify bytes in place and update okey
    for (size_t i = 0; i < len; ++i) {
        unsigned char current_byte_orig = (unsigned char)buf[i]; // Store original byte
        buf[i] = okey ^ buf[i];                                  // Modify byte in buffer
        okey += current_byte_orig;                               // Update okey with original byte
    }

    // Second loop: transmit bytes
    char *current_buf_ptr = buf;
    size_t remaining_len = len;
    size_t bytes_sent_count;
    int status;

    while (remaining_len > 0) {
        status = transmit(1, current_buf_ptr, remaining_len, &bytes_sent_count);
        if (status != 0 || bytes_sent_count == 0) {
            // Error from transmit or no bytes sent (e.g., connection closed)
            break;
        }
        remaining_len -= bytes_sent_count;
        current_buf_ptr += bytes_sent_count;
    }
    // No return value for void function
}

// Function: write_string
// Writes a null-terminated string using `write_bytes`.
// The `str` parameter must point to a mutable buffer, as `write_bytes` modifies its contents.
void write_string(char *str) {
    write_bytes(str, strlen(str));
}