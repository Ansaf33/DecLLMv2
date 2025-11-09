#include <stdint.h> // For uint32_t
#include <stdio.h>  // For printf
#include <stdlib.h> // For EXIT_SUCCESS

// Global array of error messages.
// Using const char *const for read-only string literals.
static const char *const err_msgs[] = {
    "No error",
    "File not found",
    "Permission denied",
    "Out of memory",
    "Invalid argument",
    "Network unreachable",
    "Connection refused",
    "Timeout",
    "Disk full",
    "Bad file descriptor",
    "Operation not permitted",
    "Resource busy",
    "Interrupted system call",
    "No space left on device",
    "Broken pipe",
    "Read-only file system"
};

// Function: error_to_string
// Converts an error code (param_1) to a descriptive string.
// The original code's 'uint' is replaced with 'uint32_t' for explicit sizing.
// The original 'param_1 * 4' logic is replaced with idiomatic array indexing,
// assuming 'err_msgs' is an array of char pointers, where indexing handles
// pointer size correctly.
char * error_to_string(uint32_t param_1) {
  // Check if the error code is within the bounds of known messages.
  // The original 0x10 (16) limit is robustly handled by checking array size.
  if (param_1 < (sizeof(err_msgs) / sizeof(err_msgs[0]))) {
    // Return the corresponding message directly, eliminating an intermediate variable.
    // A cast to char* is used to match the function's return type,
    // indicating these are read-only strings.
    return (char *)err_msgs[param_1];
  } else {
    // For unknown error codes, return a generic message.
    return "Unknown error.";
  }
}

// Main function to demonstrate the error_to_string function.
int main() {
    printf("Error 0: %s\n", error_to_string(0));
    printf("Error 1: %s\n", error_to_string(1));
    printf("Error 15: %s\n", error_to_string(15)); // The last defined error
    printf("Error 16: %s\n", error_to_string(16)); // Beyond the defined errors (0x10)
    printf("Error 99: %s\n", error_to_string(99)); // Another unknown error

    return EXIT_SUCCESS;
}