#include <stdio.h>    // For NULL, snprintf, STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO
#include <stdlib.h>   // For malloc, free
#include <string.h>   // For strlen, strcpy
#include <math.h>     // For NAN, isnan, roundf
#include <stdbool.h>  // For bool
#include <limits.h>   // For UINT_MAX
#include <unistd.h>   // For read, write, ssize_t

// --- Global Constants (inferred from original snippet) ---

// Constants for snprintfloat
static const float FLOAT_PRECISION_MULTIPLIER = 1000000.0f; // Used to get 6 decimal places

// Constants for strToDouble
// Array for powers of ten for fractional part calculation. Max index 8 (for 8 fractional digits).
static const long double POWERS_OF_TEN_LD[] = {
    1.0L, 10.0L, 100.0L, 1000.0L, 10000.0L, 100000.0L, 1000000.0L, 10000000.0L, 100000000.0L
};

// Constants for fillPage
static const float GRAVITY_ACCEL = 9.81f; // Example value for DAT_00012044
static const float DRAG_FACTOR = 2.0f;    // Example value for DAT_00012048
static const unsigned int MAX_ITEMS_PER_PAGE = 512; // Inferred from 0x200 in original code, represents number of float pairs

// String literals inferred from DAT_ addresses in main
static const char* X_SEP_STR = ", Y="; // DAT_0001203a
static const char* Y_END_STR = "\n";   // DAT_0001203d

// --- Function Prototypes ---
// (Defined before main to ensure proper scope and compilation order)
int snprintdecimal(char *s, size_t size, unsigned int value, unsigned int min_digits);
int snprintfloat(char *s, size_t size, float value);
unsigned int strToUint32(const char *s);
long double strToDouble(const char *s);
void fillPage(float initial_vel_x, float initial_vel_y, void *page_buffer_ptr, unsigned int *current_count, unsigned int items_per_block);

// --- Custom/Wrapper Functions for System Interaction ---

// Placeholder for original allocate function, using malloc
// int *error_code parameter is used to mimic the original `&local_bc` where `local_bc` was `int`.
// In this context, `error_code` points to an int to store success/failure.
void* allocate(size_t size, int flags, int *error_code) {
    void* ptr = malloc(size);
    if (ptr == NULL && error_code != NULL) {
        *error_code = -1; // Indicate allocation failure
    } else if (error_code != NULL) {
        *error_code = 0; // Indicate success
    }
    // `flags` parameter from original `allocate` is ignored as malloc doesn't use it directly.
    return ptr;
}

// Wrapper for `strlen` to match original `myStrLen` signature
size_t myStrLen(const char *s) {
    return strlen(s);
}

// Wrapper for `read` to match original `readLine` signature
// Reads up to `max_len - 1` characters into `buf`, null-terminates, and converts newline to null.
// Returns number of characters before newline/null, or -1 on error.
ssize_t readLine(int fd, char *buf, size_t max_len) {
    ssize_t bytes_read = read(fd, buf, max_len - 1); // Leave space for null terminator
    if (bytes_read > 0) {
        // Look for a newline character and replace it with a null terminator
        for (ssize_t i = 0; i < bytes_read; ++i) {
            if (buf[i] == '\n') {
                buf[i] = '\0';
                return i; // Return length up to the newline
            }
        }
        buf[bytes_read] = '\0'; // Null-terminate if no newline was found
    } else if (bytes_read == 0) {
        buf[0] = '\0'; // Empty line or EOF
    }
    return bytes_read;
}

// --- Implemented Functions ---

// Function: snprintdecimal
// Converts an unsigned integer to its decimal string representation.
// s: Output buffer (char array).
// size: Size of the output buffer, including space for the null terminator.
// value: The unsigned integer to convert.
// min_digits: Minimum number of digits to print, padding with leading zeros if necessary.
// Returns the number of characters written to the buffer (excluding null terminator),
// or -1 if an error occurs (e.g., buffer too small).
int snprintdecimal(char *s, size_t size, unsigned int value, unsigned int min_digits) {
    char temp_buf[24]; // Temporary buffer for digits (max 10 for uint + safety)
    unsigned int i = 0; // Index for temp_buf
    int result_len;

    // Basic validation of input parameters
    if (s == NULL || size < 2) { // Need at least 2 bytes for "0\0"
        return -1;
    }

    // Convert integer to string (digits are stored in reverse order in temp_buf)
    if (value == 0) {
        temp_buf[i++] = '0'; // Special handling for value 0
    } else {
        while (value != 0) {
            temp_buf[i++] = (char)(value % 10) + '0';
            value /= 10;
        }
    }

    // Pad with leading zeros if the actual number of digits is less than min_digits
    while (i < min_digits) {
        temp_buf[i++] = '0';
    }

    // Check if the total number of digits (including padding) exceeds the buffer size
    // `i` now holds the total number of characters required for the number.
    if (i >= size) { // `i` characters for digits + 1 for null terminator
        return -1; // Buffer is too small
    }

    // Copy digits from temp_buf to the output buffer `s` in the correct order
    result_len = 0;
    while (i > 0) {
        s[result_len++] = temp_buf[--i];
    }

    s[result_len] = '\0'; // Null-terminate the string
    return result_len;
}

// Function: snprintfloat
// Converts a float to its decimal string representation with a fixed number of fractional digits.
// s: Output buffer (char array).
// size: Size of the output buffer, including space for the null terminator.
// value: The float value to convert.
// Returns the number of characters written to the buffer (excluding null terminator),
// or -1 if an error occurs (e.g., buffer too small, NaN input).
int snprintfloat(char *s, size_t size, float value) {
    int current_len;
    unsigned int int_part;
    unsigned int frac_part;

    if (s == NULL || size < 2) { // Need space for at least "0\0"
        return -1;
    }

    // Handle NaN values
    if (isnan(value)) {
        if (size >= 4) { // "nan\0"
            strcpy(s, "nan");
            return 3;
        }
        return -1;
    }

    // Extract and print the integer part of the float
    // roundf is used to handle potential floating point inaccuracies at integer boundaries
    int_part = (unsigned int)roundf(value); // Assuming positive numbers based on context
    current_len = snprintdecimal(s, size, int_part, 0);
    if (current_len < 0) {
        return -1;
    }

    // Check if there is enough space for the decimal point and fractional part
    // We need at least `current_len` for integer, `1` for '.', `6` for fractional digits, `1` for '\0'.
    // Total needed: `current_len + 1 + 6 + 1`.
    // The size passed to snprintdecimal for fractional part is `size - current_len - 1`.
    // This value must be at least `6 + 1 = 7`.
    if (size < (unsigned int)current_len + 1 + 6 + 1) { // Not enough space for "I.FFFFFF\0"
        return -1;
    }

    s[current_len] = '.'; // Add the decimal point
    current_len++;        // Increment length to account for the decimal point

    // Extract and print the fractional part
    // Multiply by FLOAT_PRECISION_MULTIPLIER (e.g., 1,000,000 for 6 decimal places)
    // Then round to nearest integer and cast to unsigned int.
    frac_part = (unsigned int)roundf(FLOAT_PRECISION_MULTIPLIER * (value - (float)int_part));
    
    // Call snprintdecimal for the fractional part.
    // The buffer starts at `s + current_len`.
    // The available size for the fractional part is `size - current_len`.
    // `min_digits` is 6 as per original code context.
    int frac_len = snprintdecimal(s + current_len, size - current_len, frac_part, 6);
    if (frac_len < 0) {
        return -1; // Error during fractional part printing
    }
    current_len += frac_len; // Add the length of the fractional part

    return current_len;
}

// Function: strToUint32
// Converts a string to an unsigned 32-bit integer.
// s: Input string.
// Returns the converted unsigned integer, or 0 if the string is invalid, empty,
// contains non-digit characters (before delimiters), or overflows unsigned int.
unsigned int strToUint32(const char *s) {
    unsigned int result = 0;
    int i = 0;

    if (s == NULL) {
        return 0;
    }

    // Iterate through the string until a null terminator, newline, dot, or comma is found.
    for (i = 0; s[i] != '\0' && s[i] != '\n' && s[i] != '.' && s[i] != ','; ++i) {
        // Check if the current character is a digit
        if (s[i] < '0' || s[i] > '9') {
            return 0; // Invalid character, return 0 as error
        }

        unsigned int digit = s[i] - '0';

        // Check for potential overflow before performing multiplication and addition.
        // If `result * 10 + digit` would exceed `UINT_MAX`.
        if (result > UINT_MAX / 10 || (result == UINT_MAX / 10 && digit > UINT_MAX % 10)) {
            return 0; // Overflow detected, return 0 as error
        }

        result = result * 10 + digit;
    }

    return result;
}

// Function: strToDouble
// Converts a string to a long double floating-point number.
// s: Input string.
// Returns the converted long double, or NAN if the string is invalid, empty,
// contains non-digit characters (before delimiters), or has too many digits.
long double strToDouble(const char *s) {
    unsigned int current_num_part = 0; // Accumulates digits for either integer or fractional part
    unsigned int integer_part = 0;
    unsigned int fractional_part = 0;
    int i = 0; // Loop index
    int decimal_point_idx = -1; // Index of the character *after* the decimal point
    bool has_decimal = false; // Flag to track if a decimal point has been encountered

    if (s == NULL) {
        return NAN; // Return NaN for null input string
    }

    // Iterate through the string until a null terminator or newline is found
    for (i = 0; s[i] != '\0' && s[i] != '\n'; ++i) {
        if (s[i] == '.' || s[i] == ',') {
            if (has_decimal) { // Already found a decimal point, multiple decimals are invalid
                return NAN;
            }
            has_decimal = true;
            decimal_point_idx = i + 1; // Mark the start of the fractional part
            integer_part = current_num_part; // Store the accumulated integer part
            current_num_part = 0; // Reset for accumulating fractional digits
        } else {
            // Apply digit limits as observed in the original code
            if (!has_decimal && i >= 9) { // Max 9 integer digits
                return NAN;
            }
            if (has_decimal && (i - decimal_point_idx) >= 8) { // Max 8 fractional digits
                return NAN;
            }

            // Check if the character is a digit
            if (s[i] < '0' || s[i] > '9') {
                return NAN; // Invalid character, return NaN
            }
            current_num_part = current_num_part * 10 + (s[i] - '0');
        }
    }

    // After parsing, assign the final accumulated number part
    if (!has_decimal) {
        integer_part = current_num_part; // No decimal, so all digits form the integer part
        return (long double)integer_part;
    } else {
        fractional_part = current_num_part; // Digits after decimal form the fractional part
        int num_frac_digits = i - decimal_point_idx;
        if (num_frac_digits < 0) num_frac_digits = 0; // Handle cases like "123." with no fractional digits

        // Ensure num_frac_digits does not exceed the bounds of POWERS_OF_TEN_LD array
        if (num_frac_digits >= (int)(sizeof(POWERS_OF_TEN_LD) / sizeof(POWERS_OF_TEN_LD[0]))) {
            return NAN; // Too many fractional digits for our powers array
        }

        // Combine integer and fractional parts
        return (long double)integer_part + (long double)fractional_part / POWERS_OF_TEN_LD[num_frac_digits];
    }
}

// Function: fillPage
// Fills a portion of a memory buffer (page) with calculated X and Y coordinates.
// initial_vel_x: Initial velocity in the X direction.
// initial_vel_y: Initial velocity in the Y direction.
// page_buffer_ptr: Pointer to the memory buffer (treated as an array of float pairs).
// current_count: Pointer to an unsigned integer that tracks the total count of items.
// items_per_block: Number of items (float pairs) to process in each block within the page.
void fillPage(float initial_vel_x, float initial_vel_y, void *page_buffer_ptr, unsigned int *current_count, unsigned int items_per_block) {
    // Basic validation
    if (page_buffer_ptr == NULL || current_count == NULL || items_per_block == 0) {
        return;
    }

    // Cast the void pointer to a float pointer for array access
    float *page_buffer = (float *)page_buffer_ptr;

    unsigned int initial_count_val = *current_count; // Store the count value at the start of this function call
    unsigned int i, j;
    float time_val;
    float y_calc;

    // The outer loop iterates over "blocks" within the logical page.
    // MAX_ITEMS_PER_PAGE is 512. If items_per_block is 10, this loop runs 51 times.
    for (i = 0; i < MAX_ITEMS_PER_PAGE / items_per_block; ++i) {
        // Calculate the starting index for the current block within the `page_buffer`.
        // This ensures items are written in blocks. The `roundf` implies block alignment.
        unsigned int block_start_item_idx = (unsigned int)roundf((float)(*current_count - initial_count_val) / items_per_block) * items_per_block;

        // The inner loop fills `items_per_block` items (pairs of X, Y floats)
        for (j = 0; j < items_per_block; ++j) {
            // Calculate `time_val` based on the current overall count and items per block.
            time_val = (float)*current_count / (float)items_per_block;

            // Store X coordinate. Each item is a pair of floats, so index * 2 for X, index * 2 + 1 for Y.
            page_buffer[(j + block_start_item_idx) * 2] = initial_vel_x * time_val;

            // Calculate Y coordinate, applying a simplified gravity and drag model.
            y_calc = initial_vel_y * time_val - (GRAVITY_ACCEL * time_val * time_val) / DRAG_FACTOR;
            if (y_calc < 0.0f) {
                y_calc = 0.0f; // Ensure Y coordinate does not go below zero
            }
            page_buffer[(j + block_start_item_idx) * 2 + 1] = y_calc;

            (*current_count)++; // Increment the global item counter
        }
    }
}

// Function: main
// Entry point of the program.
int main(void) {
    int read_result;
    unsigned int current_count = 0;
    float initial_vel_x;
    float initial_vel_y;
    void *page_buffer_ptr = NULL; // Pointer to the allocated memory page
    int allocate_error_code = 0;  // To capture allocation errors

    // Allocate 0x1000 bytes (4096 bytes) for the page buffer.
    page_buffer_ptr = allocate(0x1000, 0, &allocate_error_code);
    if (page_buffer_ptr == NULL) {
        return 0xffffffff; // Return -1 on allocation failure
    }

    char input_buf[100];         // Buffer for user input strings
    long double temp_long_double; // Temporary storage for strToDouble result

    // Prompt for and read Initial Velocity X, loop until a valid number is entered
    do {
        write(STDOUT_FILENO, "Initial Velocity X: ", myStrLen("Initial Velocity X: "));
        read_result = readLine(STDIN_FILENO, input_buf, sizeof(input_buf));
        if (read_result < 0) {
            free(page_buffer_ptr);
            return 0xffffffff; // Error reading input
        }
        temp_long_double = strToDouble(input_buf);
        initial_vel_x = (float)temp_long_double;
    } while (isnan(temp_long_double)); // Check if the parsed value is Not-a-Number

    // Prompt for and read Initial Velocity Y, loop until a valid number is entered
    do {
        write(STDOUT_FILENO, "Initial Velocity Y: ", myStrLen("Initial Velocity Y: "));
        read_result = readLine(STDIN_FILENO, input_buf, sizeof(input_buf));
        if (read_result < 0) {
            free(page_buffer_ptr);
            return 0xffffffff; // Error reading input
        }
        temp_long_double = strToDouble(input_buf);
        initial_vel_y = (float)temp_long_double;
    } while (isnan(temp_long_double));

    // Prompt for and read Initial Count
    write(STDOUT_FILENO, "Initial Count: ", myStrLen("Initial Count: "));
    read_result = readLine(STDIN_FILENO, input_buf, sizeof(input_buf));
    if (read_result < 0) {
        free(page_buffer_ptr);
        return 0xffffffff; // Error reading input
    }
    current_count = strToUint32(input_buf);

    char print_line_buf[128]; // Buffer to construct the full output string "X=val, Y=val\n"
    char x_val_str[24];       // Buffer for snprintfloat output for X coordinate
    char y_val_str[24];       // Buffer for snprintfloat output for Y coordinate
    int i;                    // Loop index for printing coordinates

    // Main program loop: fill a page, then wait for user command ('p' to print, 'c' to continue, other to exit)
    while (true) {
        // Fill the page buffer with calculated coordinates
        fillPage(initial_vel_x, initial_vel_y, page_buffer_ptr, &current_count, 10); // Use 10 items per block

        write(STDOUT_FILENO, "Enter command (c for continue, p for print, other to exit): ", myStrLen("Enter command (c for continue, p for print, other to exit): "));
        read_result = readLine(STDIN_FILENO, input_buf, sizeof(input_buf));
        if (read_result < 0) {
            free(page_buffer_ptr);
            return 0xffffffff; // Error reading input
        }

        if (input_buf[0] == 'p') {
            // Iterate through the stored coordinates in the page buffer and print them
            // The original loop bound was 0x1fe (510). MAX_ITEMS_PER_PAGE is 512.
            // This prints 510 pairs of floats.
            for (i = 0; i < MAX_ITEMS_PER_PAGE - 2; ++i) { // Iterating 510 items (pairs of floats)
                float x_coord = ((float*)page_buffer_ptr)[i * 2];       // X coordinate
                float y_coord = ((float*)page_buffer_ptr)[i * 2 + 1];   // Y coordinate

                // Convert X coordinate to string
                int x_len = snprintfloat(x_val_str, sizeof(x_val_str), x_coord);
                if (x_len < 0) { // Fallback if snprintfloat fails
                    snprintf(x_val_str, sizeof(x_val_str), "ERR");
                }
                
                // Convert Y coordinate to string
                int y_len = snprintfloat(y_val_str, sizeof(y_val_str), y_coord);
                if (y_len < 0) { // Fallback if snprintfloat fails
                    snprintf(y_val_str, sizeof(y_val_str), "ERR");
                }

                // Construct the full output string "X=val, Y=val\n"
                int total_len = snprintf(print_line_buf, sizeof(print_line_buf), "X=%s%s%s%s",
                                         x_val_str, X_SEP_STR, y_val_str, Y_END_STR);
                
                // Write the constructed string to standard output
                if (total_len > 0 && (size_t)total_len < sizeof(print_line_buf)) {
                    write(STDOUT_FILENO, print_line_buf, total_len);
                } else if (total_len < 0) {
                    // Handle snprintf error if it occurs
                    write(STDERR_FILENO, "Error formatting output.\n", myStrLen("Error formatting output.\n"));
                } else { // total_len >= sizeof(print_line_buf)
                    write(STDERR_FILENO, "Output buffer too small for coordinate string.\n", myStrLen("Output buffer too small for coordinate string.\n"));
                    write(STDOUT_FILENO, print_line_buf, sizeof(print_line_buf) - 1); // Print truncated string
                    write(STDOUT_FILENO, "\n", 1);
                }
            }
        } else if (input_buf[0] == 'c') {
            // User chose to continue, loop will proceed to fillPage again
            continue;
        } else {
            // Any other input character, exit the program loop
            break;
        }
    }

    free(page_buffer_ptr); // Free the allocated memory before exiting
    return 0; // Indicate successful program termination
}