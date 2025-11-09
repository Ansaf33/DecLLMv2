#include <stdbool.h> // For bool type
#include <stddef.h>  // For NULL
#include <limits.h>  // For INT_MIN

// Function: itoa
// Converts an integer to a null-terminated string.
// value: The integer to convert.
// buffer: Pointer to the character array to store the string.
// buf_size: The maximum size of the buffer, including the null terminator.
// Returns: A pointer to the buffer on success, or NULL if the buffer is too small.
char* itoa(int value, char* buffer, unsigned int buf_size) {
    if (buffer == NULL || buf_size == 0) {
        return NULL; // Invalid buffer or size
    }

    unsigned int i = 0; // Current index in the buffer
    bool is_negative = false;

    // Handle INT_MIN (e.g., -2147483648) specifically, as -value would overflow
    if (value == INT_MIN) {
        // String "-2147483648" requires 11 digits + 1 sign + 1 null terminator = 13 characters
        if (buf_size < 13) {
            return NULL;
        }
        const char* int_min_str = "-2147483648";
        for (i = 0; i < 12; ++i) { // Copy 12 characters including the sign
            buffer[i] = int_min_str[i];
        }
        buffer[i] = '\0'; // Null-terminate
        return buffer;
    }

    // Handle negative values
    if (value < 0) {
        is_negative = true;
        value = -value; // Make value positive for digit extraction
    }
    // Handle zero explicitly (after INT_MIN check)
    else if (value == 0) {
        if (buf_size < 2) { // Need space for "0" and '\0'
            return NULL;
        }
        buffer[0] = '0';
        buffer[1] = '\0';
        return buffer;
    }

    // Extract digits in reverse order (e.g., 123 -> '3', '2', '1')
    // and store them in the buffer from index 0
    while (value != 0) {
        // Check if space is available for the digit + null terminator
        if (i >= buf_size - 1) {
            return NULL; // Buffer too small
        }
        buffer[i++] = (value % 10) + '0';
        value /= 10;
    }

    // Add the negative sign if applicable
    if (is_negative) {
        // Check if space is available for the sign + null terminator
        if (i >= buf_size - 1) {
            return NULL; // Buffer too small
        }
        buffer[i++] = '-';
    }

    // Null-terminate the string
    buffer[i] = '\0';

    // Reverse the string in place
    unsigned int start = 0;
    unsigned int end = i - 1; // 'i' is the length of the string (excluding null terminator)

    while (start < end) {
        char temp_char = buffer[start];
        buffer[start] = buffer[end];
        buffer[end] = temp_char;
        start++;
        end--;
    }

    return buffer;
}