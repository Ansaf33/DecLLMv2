#include <stdint.h> // For uint, intptr_t
#include <stdlib.h> // For malloc, free, exit
#include <string.h> // For memcpy
#include <stdio.h>  // For getchar (mock receive)

// Assume 'uint' means unsigned int
typedef unsigned int uint;
// Assume 'undefined' means char for byte operations
typedef char undefined;
// Assume 'undefined4' means int for 4-byte generic parameter
typedef int undefined4;

// Mock external data (from decompiled view)
// These are likely read-only data segments.
static const char DAT_00012000[] = "Name1"; // 5 bytes including null terminator
static const char DAT_00012005[] = "Name2"; // 5 bytes
static const char DAT_0001200a[] = "Name3"; // 5 bytes
static const char DAT_0001200f[] = "Name4"; // 5 bytes

// Mock allocate function (based on observed behavior)
// Returns 0 on success, non-zero on failure. Stores allocated ptr in *out_ptr.
int mock_allocate(size_t size, int unused_param, char** out_ptr) {
    *out_ptr = (char*)malloc(size);
    return (*out_ptr == NULL); // Return 0 for success, 1 for failure
}

// Mock terminate function
void _terminate(int status) {
    exit(status);
}

// Function: get_name
int get_name(uint *param_1) {
    const char *names[] = {
        DAT_00012000,
        DAT_00012005,
        DAT_0001200a,
        DAT_0001200f
    };
    
    char *allocated_name_ptr;
    
    // Allocate 5 bytes (e.g., for "NameX\0")
    if (mock_allocate(5, 0, &allocated_name_ptr) != 0) {
        _terminate(1);
    }
    
    uint name_idx = *param_1 & 3; // Use the lower 2 bits of param_1 as an index
    
    // Copy 4 characters from the selected name string
    memcpy(allocated_name_ptr, names[name_idx], 4);
    allocated_name_ptr[4] = '\0'; // Null-terminate the string in the allocated buffer

    // Return the address as an int. Using intptr_t for portability to convert
    // pointer to integer, then casting to int as per original signature.
    return (int)(intptr_t)allocated_name_ptr;
}

// Mock receive function based on observed behavior
// It simulates reading a single character from input.
// Returns 0 on success, non-zero on error.
// Populates *bytes_read_out with 1 (if char read) or 0 (if EOF).
// Populates *char_out with the character read.
int mock_receive(undefined4 param_1_val, int* bytes_read_out, char* char_out) {
    int c = getchar(); // Read one character from stdin
    if (c == EOF) {
        *bytes_read_out = 0; // No bytes read
        *char_out = 0;       // No character
        return 0;            // Indicate EOF as "not an error to terminate" but no data
    }
    *bytes_read_out = 1; // 1 byte read
    *char_out = (char)c; // Store the character
    return 0;            // Success
}

// Function: receive_line
// param_1: an undefined 4-byte value (its purpose is unclear from the snippet, passed to mock_receive)
// param_2: a pointer to the destination buffer (char*)
// param_3: maximum number of characters to read (uint)
uint receive_line(undefined4 param_1, char* param_2, uint param_3) {
    uint current_len = 0; // Tracks the number of characters read
    char received_char;   // Stores the character received by mock_receive
    int bytes_read;       // Stores the number of bytes read by mock_receive

    do {
        // Call mock_receive to get one character
        if (mock_receive(param_1, &bytes_read, &received_char) != 0) {
            _terminate(1); // Terminate if mock_receive indicates an error
        }

        if (bytes_read == 0) { // If no bytes were read (e.g., EOF)
            return 0;
        }

        if (current_len >= param_3) { // If the buffer is full
            return 0;
        }
        
        // Store the received character in the buffer and increment the length
        param_2[current_len++] = received_char;

    } while (received_char != '\n'); // Continue until a newline character is received

    return current_len; // Return the total number of characters read (including newline)
}

// Function: copy
// This function performs a byte-by-byte copy from src to dst for 'count' bytes.
// EVP_PKEY_CTX is treated as a generic pointer type (void*) as its definition is unknown
// and the operation is a generic memory copy.
// 'in_stack_0000000c' is interpreted as a 'count' parameter, added to the signature.
void* copy(void* dst, const void* src, int count) {
    char* d = (char*)dst;
    const char* s = (const char*)src;

    while (count > 0) {
        *d = *s; // Copy one byte
        count--;
        s++;     // Move to the next source byte
        d++;     // Move to the next destination byte
    }
    return dst; // Return the destination pointer
}