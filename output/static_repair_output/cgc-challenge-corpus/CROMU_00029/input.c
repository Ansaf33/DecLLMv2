#include <stdio.h>    // For puts, NULL, perror
#include <stdlib.h>   // For atof, malloc, free
#include <string.h>   // For strchr
#include <unistd.h>   // For read, ssize_t
#include <sys/select.h> // For select, fd_set, FD_ZERO, FD_SET, FD_ISSET
#include <sys/time.h>   // For struct timeval

// Type definitions to match original snippet's decompiler output
typedef unsigned int uint;
typedef int undefined4; // Commonly used for 32-bit return values like 0 or -1
typedef char undefined;  // Used for single bytes

// Global constants derived from DAT_00016018, DAT_00016020, _DAT_00016028, X, Y, Z, DAT_00016000
// These are mock values for compilation; in a real scenario, they would be defined elsewhere.
const double MIN_TEMP = -100.0;     // Corresponds to DAT_00016018
const double MAX_TEMP = 100.0;      // Corresponds to DAT_00016020
const double TEMP_OFFSET = 273.15;  // Corresponds to _DAT_00016028 (e.g., Celsius to Kelvin)
const int X_DIM = 5;                // Corresponds to X
const int Y_DIM = 5;                // Corresponds to Y
const int Z_DIM = 2;                // Corresponds to Z
const char *DELIMITERS = " \t\n\r"; // Corresponds to DAT_00016000 (common whitespace delimiters)

// Mock/wrapper for the 'receive' function found in the snippet.
// Assumes it's a wrapper around `read` from `<unistd.h>`.
// Returns 0 on success, non-zero on error. Fills `bytes_read_ptr` with actual bytes read.
static int custom_receive(int fd, char *buf, int count, int *bytes_read_ptr) {
    ssize_t res = read(fd, buf, count);
    if (res == -1) {
        perror("receive: read error");
        return -1; // Indicate error
    }
    *bytes_read_ptr = (int)res;
    return 0; // Indicate success
}

// Mock/wrapper for the 'fdwait' function found in the snippet.
// Assumes it's a wrapper around `select` from `<sys/select.h>`.
// Returns 0 on success (file descriptors ready or timeout), non-zero on error.
static int custom_fdwait(int nfds, fd_set *readfds, fd_set *writefds, int *timeout_ms, fd_set *exceptfds) {
    struct timeval tv;
    tv.tv_sec = *timeout_ms / 1000;
    tv.tv_usec = (*timeout_ms % 1000) * 1000;

    int ret = select(nfds, readfds, writefds, exceptfds, &tv);
    if (ret == -1) {
        perror("fdwait: select error");
        return -1; // Indicate error
    }
    return 0; // Indicate success
}

// Function: read_until
// Reads characters from `param_1` (stdin, fd 0) into `buffer` until a delimiter from `delimiters` is found
// or `max_len - 1` characters are read. Null-terminates the `buffer`.
// Returns the number of characters read (excluding null terminator) or 0xffffffff on error/EOF.
uint read_until(char *buffer, const char *delimiters, int max_len) {
    unsigned int bytes_read_count = 0;
    char current_char;
    int chars_received;

    while (bytes_read_count < (unsigned int)(max_len - 1)) {
        if (custom_receive(0, &current_char, 1, &chars_received) != 0) {
            return 0xffffffff; // Error from receive itself
        }
        if (chars_received == 0) {
            return 0xffffffff; // No bytes received (EOF or no data)
        }
        if (strchr(delimiters, current_char) != NULL) {
            break; // Delimiter found
        }
        buffer[bytes_read_count++] = current_char; // Store char and increment count
    }
    buffer[bytes_read_count] = '\0'; // Null-terminate the string
    return bytes_read_count;
}

// Function: StoreTemp
// Converts a string `temp_str` to a double, validates its range, adds an offset,
// and stores it into a 3D array `temp_array` at the given `x_idx, y_idx, z_idx`.
// Returns 0 on success, 0xffffffff on error (invalid temperature range).
undefined4 StoreTemp(double *temp_array, int x_idx, int y_idx, int z_idx, const char *temp_str) {
    double temp_val = atof(temp_str); // Assuming cgcatof is equivalent to atof or strtod
    if ((temp_val < MIN_TEMP) || (MAX_TEMP < temp_val)) {
        return 0xffffffff; // Invalid temperature range
    }
    // Calculate 1D index for a 3D array [Z_DIM][Y_DIM][X_DIM]
    // The original expression (param_2 + X * param_3 + Y * X * param_4) corresponds to:
    // (x_idx + X_DIM * y_idx + Y_DIM * X_DIM * z_idx)
    temp_array[z_idx * (Y_DIM * X_DIM) + y_idx * X_DIM + x_idx] = TEMP_OFFSET + temp_val;
    return 0; // Success
}

// Function: flush_stdin
// Clears pending input from stdin by repeatedly reading single characters
// until no more data is available or an error occurs.
// Returns 0 on success, 0xffffffff on error.
undefined4 flush_stdin(void) {
    fd_set fds;
    char buffer_char;
    int bytes_read_val;
    int fdwait_timeout_ms = 0; // Non-blocking check for `select`

    do {
        FD_ZERO(&fds);
        FD_SET(0, &fds); // Set stdin (fd 0) for reading

        if (custom_fdwait(1, &fds, NULL, &fdwait_timeout_ms, NULL) != 0) {
            return 0xffffffff; // Error from fdwait
        }
        if (!FD_ISSET(0, &fds)) {
            break; // Stdin not ready, nothing more to flush
        }
        // Stdin is ready, try to read a character
        if (custom_receive(0, &buffer_char, 1, &bytes_read_val) != 0) {
            return 0xffffffff; // Error from receive
        }
    } while (bytes_read_val > 0); // Continue as long as bytes are being read
    return 0; // Successfully flushed or no more data
}

// Function: read_temps
// Reads temperature values from stdin, parses them, and stores them into the `temp_array`.
// It iterates through Z_DIM, Y_DIM, X_DIM dimensions.
// Returns 0 on success, 0xffffffff on error (parsing, range validation, or I/O).
undefined4 read_temps(double *temp_array) {
    char buffer[100]; // Buffer to hold temperature string read from stdin
    unsigned int z_idx;
    unsigned int y_idx;
    unsigned int x_idx;

    for (z_idx = 0; z_idx < Z_DIM; z_idx++) {
        for (y_idx = 0; y_idx < Y_DIM; y_idx++) {
            for (x_idx = 0; x_idx < X_DIM; x_idx++) {
                if (read_until(buffer, DELIMITERS, sizeof(buffer)) == 0xffffffff) {
                    return 0xffffffff; // Error from read_until
                }
                if (StoreTemp(temp_array, x_idx, y_idx, z_idx, buffer) != 0) {
                    puts("Invalid temperature");
                    return 0xffffffff; // Error from StoreTemp
                }
            }
        }
    }
    flush_stdin(); // Flush any remaining input after reading all temperatures
    return 0; // Success
}

// Function: kbhit
// Checks if a key has been pressed on stdin. If a key is pressed, it is consumed.
// This function polls stdin with a 1ms timeout.
// Returns 0 if a key was hit and consumed, 0xffffffff on error.
undefined4 kbhit(void) {
    fd_set fds;
    char buffer_char; // Character is consumed by receive
    int bytes_read_val;
    int fdwait_timeout_ms;

    while (1) { // Loop indefinitely until a key is hit or an error occurs
        FD_ZERO(&fds);
        FD_SET(0, &fds); // Set stdin (fd 0) for reading

        fdwait_timeout_ms = 1; // Poll with 1ms timeout

        if (custom_fdwait(1, &fds, NULL, &fdwait_timeout_ms, NULL) != 0) {
            return 0xffffffff; // Error from fdwait
        }

        if (FD_ISSET(0, &fds)) { // Stdin is ready, a key was hit
            // Attempt to read and consume the character
            if (custom_receive(0, &buffer_char, 1, &bytes_read_val) == 0) {
                return 0; // Key hit and consumed successfully
            } else {
                return 0xffffffff; // Error receiving the character
            }
        }
        // If stdin not ready, loop again after 1ms timeout
    }
}

// Main function to demonstrate the usage of the refactored functions.
int main() {
    // Allocate memory for the 3D temperature array: Z_DIM * Y_DIM * X_DIM doubles
    double *temperatures = (double *)malloc(Z_DIM * Y_DIM * X_DIM * sizeof(double));
    if (temperatures == NULL) {
        perror("Failed to allocate memory for temperatures");
        return 1;
    }

    printf("Please enter %d temperature values (separated by spaces/tabs/newlines):\n", Z_DIM * Y_DIM * X_DIM);
    printf("Example: 20.5 21.0 -5.2 ... (ensure values are between %.1f and %.1f)\n", MIN_TEMP, MAX_TEMP);

    if (read_temps(temperatures) == 0) {
        printf("Successfully read and stored temperatures.\n");
        // Optional: print some temperatures to verify
        // for (int z = 0; z < Z_DIM; z++) {
        //     for (int y = 0; y < Y_DIM; y++) {
        //         for (int x = 0; x < X_DIM; x++) {
        //             printf("Temp[%d][%d][%d]: %.2f (%.2f original)\n", z, y, x, 
        //                    temperatures[z * (Y_DIM * X_DIM) + y * X_DIM + x],
        //                    temperatures[z * (Y_DIM * X_DIM) + y * X_DIM + x] - TEMP_OFFSET);
        //         }
        //     }
        // }
    } else {
        fprintf(stderr, "Failed to read temperatures.\n");
    }

    printf("\nPress any key to test kbhit()...\n");
    if (kbhit() == 0) {
        printf("Key was hit!\n");
    } else {
        fprintf(stderr, "kbhit() error or no key hit after polling.\n");
    }

    free(temperatures); // Free allocated memory
    return 0;
}