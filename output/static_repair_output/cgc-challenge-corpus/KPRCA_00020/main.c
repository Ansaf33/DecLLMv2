#include <stdarg.h>   // For va_list in fdprintf
#include <stdio.h>    // For fprintf, printf
#include <stdlib.h>   // For exit
#include <string.h>   // For memset
#include <unistd.h>   // For ssize_t, STDIN_FILENO, STDERR_FILENO
#include <sys/socket.h> // For recv

// Standard C types for decompiled 'undefined' types
typedef unsigned int undefined4; // Represents a 4-byte unsigned integer
typedef char undefined;          // Represents a single byte
typedef unsigned int uint;       // Standard C alias for unsigned int

// Forward declarations for external functions
// Assuming 'receive' is equivalent to 'recv' for both calls in the snippet
ssize_t receive(int fd, void *buf, size_t count, int flags);

// Custom functions with minimal definitions for compilation
int fdprintf(int fd, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vfprintf(stderr, format, args); // Print to stderr for error messages
    va_end(args);
    return ret;
}

int decode(void *start_ptr, void *end_ptr) {
    (void)start_ptr; // Suppress unused parameter warning
    (void)end_ptr;   // Suppress unused parameter warning
    // Placeholder for actual decoding logic
    return 1; // Assume success for compilation
}

void pprint(int value) {
    // Placeholder for actual pretty printing logic
    printf("Decoded value: %d\n", value);
}

// Function: read_exactly
// Reads exactly 'count' bytes from 'fd' into 'buf'.
// Returns 0 on success, 0xffffffff (which is -1 for signed int) on error or EOF.
undefined4 read_exactly(undefined4 param_1, int param_2, int param_3) {
    int fd = (int)param_1;
    char *buf = (char *)(long)param_2; // Cast int to long then to char* for pointer safety on 64-bit
    size_t count = (size_t)param_3;
    size_t total_read = 0;
    ssize_t bytes_received_this_call;

    while (total_read < count) {
        bytes_received_this_call = receive(fd, buf + total_read, count - total_read, 0);

        if (bytes_received_this_call <= 0) {
            // Error (-1) or EOF (0). For read_exactly, both are failures if not all bytes are read.
            return 0xffffffff;
        }
        total_read += bytes_received_this_call;
    }
    return 0; // Success
}

// Function: main
undefined4 main(void) {
    // Decompiler artifacts like stack pointer manipulation (local_10, ppuVar1, puVar4) are removed.
    // Variables are given standard C types and more descriptive names.

    char header_buf[12];        // Corresponds to local_801c
    char data_buf[32756];       // Corresponds to local_8010 (approx 0x8000 bytes)

    int bytes_read_for_length;  // Corresponds to local_1c, stores bytes read for the initial length field
    uint data_length = 0;       // Corresponds to local_18, the actual data length read from the header
    int decode_status;          // Corresponds to local_14, result of the decode function

    int input_fd = STDIN_FILENO; // Assuming standard input, or a socket FD.
    int error_fd = STDERR_FILENO; // For error messages

    // Initialize the main data buffer to zeros.
    // The original snippet's `memset` likely targeted the larger buffer `local_8010` (data_buf)
    // with a size close to 0x8000.
    memset(data_buf, 0, sizeof(data_buf));

    // Read the 4-byte data length into `data_length`.
    bytes_read_for_length = receive(input_fd, &data_length, sizeof(data_length), 0);

    if (bytes_read_for_length <= 0) {
        fdprintf(error_fd, "Error: Failed to read data length header.\n");
        exit(1);
    }
    if (bytes_read_for_length != sizeof(data_length)) {
        fdprintf(error_fd, "Error: Incomplete read of data length header. Expected %zu bytes, got %d.\n", sizeof(data_length), bytes_read_for_length);
        exit(1);
    }

    // Check if the received data_length exceeds the buffer capacity (0x8000 bytes).
    if (0x8000 < data_length) {
        fdprintf(error_fd, "Error: Data length (0x%x) exceeds buffer capacity (0x%x).\n", data_length, 0x8000);
        exit(1);
    }

    // Read the actual data (of size `data_length`) into `header_buf` using `read_exactly`.
    // The original parameters imply passing integer representations of the pointer and size.
    if (read_exactly(input_fd, (int)(long)header_buf, (int)data_length) != 0) {
        fdprintf(error_fd, "Error: Failed to read exactly %u bytes of data.\n", data_length);
        exit(1);
    }

    // Decode the received data.
    decode_status = decode(header_buf, header_buf + data_length);
    if (decode_status == 0) {
        fdprintf(error_fd, "Error: Data decoding failed.\n");
        exit(1);
    }

    // Print the result of the decoding.
    pprint(decode_status);

    return 0;
}

// Minimal definition for 'receive' function, assuming it's `recv`.
ssize_t receive(int fd, void *buf, size_t count, int flags) {
    return recv(fd, buf, count, flags);
}