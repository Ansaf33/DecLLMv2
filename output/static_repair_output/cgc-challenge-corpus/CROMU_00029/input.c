#include <stdio.h>   // For puts
#include <string.h>  // For strchr
#include <stdlib.h>  // For NULL, general utilities
#include <stddef.h>  // For size_t

// --- Type Definitions (assuming common Ghidra decompiler output) ---
typedef unsigned int uint;
typedef char undefined;
typedef int undefined4; // Used for return values, status codes

// --- Mock/External Function Declarations ---
// These functions are not provided in the snippet, so mock declarations are necessary
// to allow compilation. Implementations would depend on the target system/library.

// Mock for a receive function, similar to `recv` but with an `int *` for bytes received.
int receive(int fd, char *buf, size_t len, int *bytes_received) {
    // Placeholder: Simulate no data or error for compilation.
    // In a real system, this would read from a file descriptor.
    if (bytes_received) *bytes_received = 0;
    return 0; // Simulate success, no data
}

// Mock for a string-to-long double conversion function.
long double cgcatof(const char *s) {
    // Placeholder: Convert string to double.
    return strtold(s, NULL);
}

// Mock for a custom select-like function.
// `readfds`, `writefds`, `exceptfds` are typically `fd_set *` but Ghidra often
// represents them as `uint *` for an array of bitmasks.
int fdwait(int nfds, uint *readfds, uint *writefds, int *timeout_ms, uint *exceptfds) {
    // Placeholder: Simulate no activity for compilation.
    // In a real system, this would use select/poll.
    if (readfds) readfds[0] = 0; // Clear stdin bit
    return 0; // Simulate success
}

// --- Global Variables/Constants (mocked based on typical Ghidra output) ---
// Example dimensions for temperature data
const int X = 10;
const int Y = 10;
const int Z = 5;

// Example delimiter string for read_until (e.g., newline, carriage return, space)
const char DAT_00016000[] = "\n\r ";

// Example temperature range constants (e.g., in Celsius)
const double DAT_00016018 = -50.0; // Lower bound
const double DAT_00016020 = 150.0; // Upper bound

// Example offset constant for storing temperature (e.g., converting Celsius to Kelvin)
const double _DAT_00016028 = 273.15;

// --- Function: read_until ---
// Reads characters into 'buffer' until a delimiter from 'delimiters' is found
// or 'max_len' is reached.
// param_1 (char *buffer): Buffer to store the read characters.
// param_2 (const char *delimiters): Null-terminated string of delimiter characters.
// param_3 (int max_len): Maximum number of characters to read (including null terminator).
uint read_until(char *buffer, const char *delimiters, int max_len) {
    unsigned int bytes_read_count = 0;
    char received_char;
    int bytes_received_len;

    while (bytes_read_count < (unsigned int)max_len - 1) {
        if (receive(0, &received_char, 1, &bytes_received_len) != 0 || bytes_received_len == 0) {
            return 0xFFFFFFFF; // Error or EOF
        }
        if (strchr(delimiters, received_char) != NULL) {
            break; // Delimiter found
        }
        buffer[bytes_read_count++] = received_char;
    }
    buffer[bytes_read_count] = '\0'; // Null-terminate the string
    return bytes_read_count;
}

// --- Function: StoreTemp ---
// Stores a temperature value into a 3D array of doubles.
// param_1 (double *temp_data_base): Base pointer to the 3D array (flattened).
// param_2 (int x_idx): X-coordinate index.
// param_3 (int y_idx): Y-coordinate index.
// param_4 (int z_idx): Z-coordinate index.
// param_5 (const char *temp_str): String representation of the temperature.
undefined4 StoreTemp(double *temp_data_base, int x_idx, int y_idx, int z_idx, const char *temp_str) {
    double temp_d = (double)cgcatof(temp_str);

    if (temp_d < DAT_00016018 || DAT_00016020 < temp_d) {
        return 0xFFFFFFFF; // Invalid temperature range
    }

    // Calculate the 1D index for the 3D array (assuming row-major for Y, then X)
    // Original formula: param_1 + (param_2 + X * param_3 + Y * X * param_4) * 8
    // This translates to temp_data_base[x_idx + X * y_idx + Y * X * z_idx] if param_1 is double*
    temp_data_base[x_idx + (long long)X * y_idx + (long long)Y * X * z_idx] = _DAT_00016028 + temp_d;
    return 0;
}

// --- Function: flush_stdin ---
// Flushes any pending input from stdin.
undefined4 flush_stdin(void) {
    char received_char;
    int bytes_received_len;
    uint fd_set_read[32];
    int timeout_ms;

    do {
        for (uint i = 0; i < 32; ++i) {
            fd_set_read[i] = 0;
        }
        fd_set_read[0] |= 1; // Set bit for stdin (fd 0)

        timeout_ms = 0; // Non-blocking check
        if (fdwait(1, fd_set_read, NULL, &timeout_ms, NULL) != 0) {
            return 0xFFFFFFFF; // Error in fdwait
        }

        if (!(fd_set_read[0] & 1)) {
            return 0; // No data to read on stdin
        }
        // Data available, attempt to read one byte
        if (receive(0, &received_char, 1, &bytes_received_len) != 0) {
            return 0xFFFFFFFF; // Error in receive
        }
    } while (bytes_received_len > 0); // Continue flushing until no more data

    return 0; // Successfully flushed or no data initially
}

// --- Function: read_temps ---
// Reads temperature data for all X, Y, Z dimensions.
// param_1 (double *temp_data_base): Base pointer to the 3D array of doubles.
undefined4 read_temps(double *temp_data_base) {
    char temp_buffer[100];
    unsigned int z_idx = 0;

    do {
        if (z_idx >= (unsigned int)Z) {
            flush_stdin();
            return 0; // All Z layers processed
        }

        for (unsigned int y_idx = 0; y_idx < (unsigned int)Y; ++y_idx) {
            for (unsigned int x_idx = 0; x_idx < (unsigned int)X; ++x_idx) {
                if (read_until(temp_buffer, DAT_00016000, sizeof(temp_buffer)) == 0xFFFFFFFF) {
                    return 0xFFFFFFFF; // Error in reading temperature string
                }
                if (StoreTemp(temp_data_base, x_idx, y_idx, z_idx, temp_buffer) != 0) {
                    puts("Invalid temperature");
                    return 0xFFFFFFFF; // Error in storing temperature
                }
            }
        }
        ++z_idx;
    } while (1); // Loop indefinitely until Z condition is met or an error occurs
}

// --- Function: kbhit ---
// Checks if a key has been pressed (data available on stdin) and consumes it.
undefined4 kbhit(void) {
    char received_char;
    int bytes_received_len;
    uint fd_set_read[32];
    int timeout_ms;

    while (1) {
        for (uint i = 0; i < 32; ++i) {
            fd_set_read[i] = 0;
        }
        fd_set_read[0] |= 1; // Set bit for stdin (fd 0)

        timeout_ms = 1; // Short timeout for polling
        if (fdwait(1, fd_set_read, NULL, &timeout_ms, NULL) != 0) {
            return 0xFFFFFFFF; // Error in fdwait
        }

        if (fd_set_read[0] & 1) { // Data available on stdin
            // Read one byte to consume it and determine if a key was hit
            if (receive(0, &received_char, 1, &bytes_received_len) == 0) {
                return 0; // Key hit (and consumed)
            } else {
                return 0xFFFFFFFF; // Error in receive
            }
        }
        // No data available, continue busy-waiting
    }
}