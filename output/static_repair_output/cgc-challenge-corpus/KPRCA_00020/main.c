#include <stddef.h> // For size_t
#include <stdlib.h> // For exit
#include <string.h> // For memset
#include <stdio.h>  // For fprintf, stderr, printf
#include <stdarg.h> // For va_list in fdprintf mock

// --- Forward declarations for custom/missing functions ---
// These functions are assumed to be implemented elsewhere or are custom wrappers.
// For compilation, mock implementations are provided below.

// `receive`: A custom function, likely a wrapper around `recv` or similar.
// It attempts to read `len` bytes into `buf` from `fd`.
// It stores the actual number of bytes read in `bytes_received`.
// Returns 0 on success (some bytes read), non-zero on error.
int receive(int fd, void *buf, size_t len, int *bytes_received);

// `fdprintf`: A custom function to print formatted output to a given file descriptor.
// Similar to `fprintf` but uses an integer file descriptor instead of a FILE pointer.
int fdprintf(int fd, const char *format, ...);

// `decode`: A custom function to process a buffer of data.
// Takes pointers to the start and end of the data to decode.
// Returns a non-zero integer on success, 0 on failure.
int decode(const char *start, const char *end);

// `pprint`: A custom function to print a decoded integer value.
void pprint(int value);

// --- Mock/Placeholder implementations for compilation ---
// In a real application, these would be actual library calls or custom logic.

int receive(int fd, void *buf, size_t len, int *bytes_received) {
    if (fd == 0) { // Simulate reading from stdin
        size_t read_count = fread(buf, 1, len, stdin);
        *bytes_received = (int)read_count;
        if (read_count < len && feof(stdin)) {
            // EOF reached before reading all bytes. Treat as an error for `read_exactly`.
            return -1;
        }
        if (read_count < len && ferror(stdin)) {
            // An error occurred during reading from stdin.
            return -1;
        }
        return 0; // Success, even if less than `len` bytes were read (e.g., non-blocking behavior)
    }
    // For other file descriptors, just simulate success and fill with dummy data.
    // In a real scenario, this would typically involve `recv` for sockets.
    memset(buf, 0xAA, len); // Fill buffer with dummy data
    *bytes_received = (int)len; // Assume all requested bytes were "received"
    return 0;
}

int fdprintf(int fd, const char *format, ...) {
    FILE *target_stream = NULL;
    if (fd == 1) target_stream = stdout;
    else if (fd == 2) target_stream = stderr;
    else target_stream = stderr; // Default to stderr for any other FD

    va_list args;
    va_start(args, format);
    int ret = vfprintf(target_stream, format, args);
    va_end(args);
    return ret;
}

int decode(const char *start, const char *end) {
    if (start >= end) {
        fprintf(stderr, "Mock decode: Empty or invalid buffer range provided.\n");
        return 0; // Indicate failure
    }
    fprintf(stderr, "Mock decode: Processing %zu bytes from %p.\n", (size_t)(end - start), (const void*)start);
    // Simulate some basic decoding logic.
    if ((end - start) > 0 && start[0] == 0xAA) { // Check for dummy data pattern
        return 123; // Simulate a successful decode result
    }
    return 456; // Simulate another successful decode result
}

void pprint(int value) {
    printf("Mock pprint: Decoded value: %d\n", value);
}

// --- End of Mock functions ---


// Function: read_exactly
// Reads exactly `nbytes_to_read` bytes into the buffer `buf` from the file descriptor `fd`.
// Returns 0 on complete success, -1 (representing the original 0xffffffff) on any error.
int read_exactly(int fd, char *buf, size_t nbytes_to_read) {
    size_t bytes_remaining = nbytes_to_read;
    int bytes_received_this_call;

    while (bytes_remaining > 0) {
        // Attempt to receive the remaining bytes into the current position of the buffer.
        int res = receive(fd, buf, bytes_remaining, &bytes_received_this_call);

        if (res != 0) {
            // An error occurred during the receive call.
            return -1; // Propagate the error.
        }

        if (bytes_received_this_call <= 0) {
            // No bytes were received, but `receive` didn't report an explicit error.
            // This usually indicates EOF or a non-blocking operation that couldn't read immediately.
            // For `read_exactly`, this means we cannot fulfill the request.
            return -1; // Indicate failure to read the exact amount.
        }

        // Update the buffer pointer and the count of remaining bytes.
        bytes_remaining -= bytes_received_this_call;
        buf += bytes_received_this_call;
    }
    return 0; // All bytes successfully read.
}

// Function: main
int main(void) {
    // The original code contained complex stack pointer arithmetic and `undefined` types,
    // which are characteristic of decompiled assembly. This has been simplified
    // into standard C variable declarations and direct memory access.

    // `data_buffer` serves as the main buffer for incoming data, with a maximum size of 0x8000.
    char data_buffer[0x8000]; // 32768 bytes

    // `metadata_code` stores an arbitrary integer, potentially an error identifier.
    int metadata_code = 0x12316;

    // `data_len` will store the length of the actual data to be read, received initially.
    unsigned int data_len = 0;

    // `actual_bytes_read_for_len` stores the count of bytes read for `data_len`.
    int actual_bytes_read_for_len = 0;

    // `decode_status` will hold the return value from the `decode` function.
    int decode_status = 0;

    // Initialize the main data buffer to zeros.
    memset(data_buffer, 0, sizeof(data_buffer));

    // First `receive` call: Attempt to read the 4-byte `data_len` value.
    // The file descriptor `0` typically refers to standard input or a default socket.
    int res = receive(0, &data_len, sizeof(data_len), &actual_bytes_read_for_len);

    if (res != 0) {
        fprintf(stderr, "Error: Initial receive of data length failed.\n");
        exit(1);
    }
    if (actual_bytes_read_for_len != sizeof(data_len)) {
        fprintf(stderr, "Error: Did not receive exactly %zu bytes for data length.\n", sizeof(data_len));
        exit(1);
    }

    // Validate the received `data_len` against the buffer's capacity.
    if (data_len > sizeof(data_buffer)) {
        // Use `fdprintf` to report the error, incorporating `metadata_code`.
        fdprintf(2, "Error: Data length %u exceeds maximum buffer size %zu (code: %x).\n",
                 data_len, sizeof(data_buffer), metadata_code + 0x2006);
        exit(1);
    }

    // Second `receive` operation: Read the exact amount of data specified by `data_len`.
    res = read_exactly(0, data_buffer, data_len);

    if (res != 0) {
        fprintf(stderr, "Error: Failed to read exactly %u bytes of data into buffer.\n", data_len);
        exit(1);
    }

    // Decode the received data.
    // The `decode` function is called with the start of the buffer and an end pointer
    // calculated based on `data_len`.
    decode_status = decode(data_buffer, data_buffer + data_len);

    if (decode_status == 0) {
        fprintf(stderr, "Error: Data decoding failed.\n");
        exit(1);
    }

    // Print the result obtained from the decoding process.
    pprint(decode_status);

    return 0; // Indicate successful execution.
}