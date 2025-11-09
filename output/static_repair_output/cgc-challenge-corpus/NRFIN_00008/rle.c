#include <stdint.h> // For fixed-width integer types like uint32_t, etc.
#include <stdio.h>  // For standard input/output functions like printf, fprintf
#include <stdlib.h> // For general utilities like malloc, free, and exit

// Define custom types as observed in the original code snippet
typedef unsigned int uint;
typedef unsigned char byte;

// Define a structure based on the observed memory offsets in the original code.
// This structure is an educated guess to make the code more readable and type-safe.
// Padding fields are included to ensure correct byte offsets for the members.
struct Context {
    char _padding_88[0x88]; // Padding bytes to reach offset 0x88
    int *data_ptr;          // @0x88: Pointer to an array of integers (or bytes, depending on access)
    uint data_len;          // @0x8c: Length of the data, likely in bytes
    int *buffer_ptr;        // @0x90: Pointer to a buffer where decoded data is written
    uint status_flags;      // @0x94: Status or flags, where lower 24 bits are often manipulated
    byte key_byte;          // @0x97: A single byte used as a key for operations
    char _padding_98[0x98 - 0x97 - 1]; // Padding to align for the next field
    int expected_checksum;  // @0x98: An integer representing the expected checksum
};

// --- Placeholder/Stub functions to make the code compilable and runnable ---

// Simulates a memory allocation operation.
// Returns 1 on simulated success, 0 on simulated failure.
int allocate() {
    printf("[STUB] allocate() called.\n");
    return 1; // Simulate success
}

// Simulates sending a line of data or a message.
// The arguments appear to be: a size, a message string, and a status/flag.
// Returns 0 on simulated success, -1 on simulated failure.
int sendline(int size, const char *message, uint status) {
    printf("[STUB] sendline(size=%d, message=\"%s\", status=%u) called.\n", size, message, status);
    return 0; // Simulate success
}

// Simulates a memory deallocation operation.
// Returns 1 on simulated success, 0 on simulated failure.
int deallocate() {
    printf("[STUB] deallocate() called.\n");
    return 1; // Simulate success
}

// Simulates program termination. In a real application, this might perform cleanup.
// Here, it uses exit() from stdlib.h.
void _terminate(int exit_code) {
    printf("[STUB] _terminate() called with code %d. Exiting.\n", exit_code);
    exit(exit_code);
}

// --- End of Placeholder functions ---

// Function: decode
// param_1 is treated as a pointer to a Context structure.
// param_2 is a character used in decoding logic.
void decode(int param_1, char param_2) {
    struct Context *ctx = (struct Context *)param_1; // Cast param_1 to Context pointer
    char input_char = param_2;                       // Rename for clarity

    // Basic validation for the context pointer
    if (ctx == NULL) {
        fprintf(stderr, "Error: Context pointer (param_1) is NULL.\n");
        _terminate(1); // Terminate with an error code
    }

    if (input_char == ctx->key_byte) {
        ctx->status_flags &= 0xff000000; // Clear the lower 24 bits of status_flags

        int current_checksum = 0;
        uint i;

        // Create a 32-bit value by repeating the input_char four times (0xCCCC CCCC)
        uint repeated_char_val = (uint)input_char | ((uint)input_char << 8) |
                                 ((uint)input_char << 16) | ((uint)input_char << 24);

        // Loop 1: Calculate checksum
        // ctx->data_len is in bytes, so divide by 4 to get the number of uints
        for (i = 0; i < ctx->data_len / 4; ++i) {
            current_checksum += ((uint *)ctx->data_ptr)[i] ^ repeated_char_val;
        }

        if (current_checksum == ctx->expected_checksum) {
            // Loop 2: Update status_flags based on data_ptr contents
            // Iterate over data_ptr in 2-byte (char) increments
            for (i = 0; i < ctx->data_len; i += 2) {
                uint val_from_data = (uint)(ctx->key_byte ^ ((byte *)ctx->data_ptr)[i]);

                // Add val_from_data to the lower 24 bits of status_flags, then mask
                ctx->status_flags = (ctx->status_flags & 0xff000000) |
                                     ((val_from_data + (ctx->status_flags & 0xffffff)) & 0xffffff);

                // Increment the lower 24 bits of status_flags by 1, then mask
                ctx->status_flags = (ctx->status_flags & 0xff000000) |
                                     (((ctx->status_flags & 0xffffff) + 1) & 0xffffff);
            }

            // Attempt to allocate and send a message
            if (allocate() != 0) { // If allocate succeeds
                if (sendline(0xc, "Clever girl.", 1) < 0) { // If sendline fails
                    _terminate(6); // Terminate with specific code 6
                }
                _terminate(9); // Terminate with specific code 9 (as per original logic)
            }
            // If allocate fails, this block is skipped.

            // Loop 3: Decode data into buffer_ptr
            int current_write_offset = 0;
            for (i = 0; i < ctx->data_len; i += 2) {
                byte repeat_count = ctx->key_byte ^ ((byte *)ctx->data_ptr)[i];

                uint j;
                for (j = 0; j <= repeat_count; ++j) {
                    // Write decoded byte to buffer_ptr, incrementing offset
                    // Assumes buffer_ptr points to a byte array
                    ((byte *)ctx->buffer_ptr)[current_write_offset++] =
                        ((byte *)ctx->data_ptr)[i + 1] ^ ctx->key_byte;
                }
            }
        }
    } else { // This branch is taken if input_char != ctx->key_byte
        // Check if buffer_ptr is non-NULL before attempting deallocation
        if (ctx->buffer_ptr != NULL) {
            // Attempt to deallocate and send a message
            if (deallocate() != 0) { // If deallocate succeeds
                if (sendline(0xc, "Clever girl.", 1) < 0) { // If sendline fails
                    _terminate(6); // Terminate with specific code 6
                }
                _terminate(0x12); // Terminate with specific code 0x12
            }
            // If deallocate fails, this block is skipped.

            ctx->buffer_ptr = NULL;          // Clear the buffer pointer
            ctx->status_flags &= 0xff000000; // Clear the lower 24 bits of status_flags
        }
    }
    return;
}

// Main function to demonstrate the decode function's behavior.
int main() {
    // Allocate memory for a Context struct
    struct Context *my_context = (struct Context *)malloc(sizeof(struct Context));
    if (my_context == NULL) {
        fprintf(stderr, "Failed to allocate context.\n");
        return 1;
    }

    // --- Test Case 1: Successful Decode Scenario ---
    printf("--- Test Case 1: Successful Decode ---\n");
    // Initialize context fields for a test where the first 'if' and inner 'if' are true.
    my_context->key_byte = 'A';
    my_context->data_len = 8; // Represents 2 uints or 8 bytes
    my_context->status_flags = 0xDEADBEEF; // Initial flags

    // Allocate and initialize data_ptr buffer
    my_context->data_ptr = (int *)malloc(my_context->data_len);
    if (my_context->data_ptr == NULL) {
        fprintf(stderr, "Failed to allocate data_ptr for Test 1.\n");
        free(my_context);
        return 1;
    }
    ((uint *)my_context->data_ptr)[0] = 0x01020304;
    ((uint *)my_context->data_ptr)[1] = 0x05060708;

    // Calculate the expected checksum for 'A'
    uint repeated_A = (uint)'A' | ((uint)'A' << 8) | ((uint)'A' << 16) | ((uint)'A' << 24);
    my_context->expected_checksum = (((uint *)my_context->data_ptr)[0] ^ repeated_A) +
                                    (((uint *)my_context->data_ptr)[1] ^ repeated_A);

    // Allocate a buffer for decoded output
    my_context->buffer_ptr = (int *)malloc(100); // Sufficient size for this example
    if (my_context->buffer_ptr == NULL) {
        fprintf(stderr, "Failed to allocate buffer_ptr for Test 1.\n");
        free(my_context->data_ptr);
        free(my_context);
        return 1;
    }

    // Call decode with matching key_byte and checksum
    printf("Initial status flags: 0x%x\n", my_context->status_flags);
    decode((int)my_context, 'A');
    printf("Status flags after successful decode: 0x%x\n", my_context->status_flags);
    printf("Buffer ptr after successful decode: %p\n", (void*)my_context->buffer_ptr);
    // Note: The _terminate(9) call inside decode will exit the program here.
    // For demonstration, you might comment out _terminate calls in the decode function
    // to see all test cases, but the original code implies immediate exit.

    // If _terminate was commented out:
    // --- Test Case 2: Mismatched Key Byte (should trigger deallocation) ---
    printf("\n--- Test Case 2: Mismatched Key Byte ---\n");
    // Re-initialize for this test case
    my_context->key_byte = 'B'; // Key byte is 'B'
    my_context->status_flags = 0xCAFEF00D;
    // Ensure buffer_ptr is non-NULL to trigger the deallocation path
    if (my_context->buffer_ptr == NULL) { // If it was already freed/set to NULL by previous test
        my_context->buffer_ptr = (int *)malloc(50);
        if (my_context->buffer_ptr == NULL) {
            fprintf(stderr, "Failed to allocate buffer_ptr for Test 2.\n");
            free(my_context->data_ptr); free(my_context); return 1;
        }
    }
    printf("Initial status flags: 0x%x\n", my_context->status_flags);
    printf("Initial buffer ptr: %p\n", (void*)my_context->buffer_ptr);
    decode((int)my_context, 'C'); // Input 'C' does not match key_byte 'B'
    printf("Status flags after mismatched key: 0x%x\n", my_context->status_flags);
    printf("Buffer ptr after mismatched key: %p (should be NULL)\n", (void*)my_context->buffer_ptr);
    // Note: The _terminate(0x12) call inside decode will exit the program here.

    // If _terminate was commented out:
    // --- Test Case 3: Mismatched Checksum (no deallocation, flags unchanged after Loop 1) ---
    printf("\n--- Test Case 3: Mismatched Checksum ---\n");
    // Re-initialize for this test case
    my_context->key_byte = 'X';
    my_context->data_len = 4; // One uint
    ((uint *)my_context->data_ptr)[0] = 0x12345678;
    my_context->expected_checksum = 0xBAD; // Deliberately wrong checksum
    my_context->status_flags = 0x87654321;
    // Ensure buffer_ptr is allocated, it should not be deallocated in this path
    if (my_context->buffer_ptr == NULL) {
        my_context->buffer_ptr = (int *)malloc(20);
        if (my_context->buffer_ptr == NULL) {
            fprintf(stderr, "Failed to allocate buffer_ptr for Test 3.\n");
            free(my_context->data_ptr); free(my_context); return 1;
        }
    }
    printf("Initial status flags: 0x%x\n", my_context->status_flags);
    printf("Initial buffer ptr: %p\n", (void*)my_context->buffer_ptr);
    decode((int)my_context, 'X'); // Key matches, but checksum will fail
    // Status flags should only have the lower 24 bits cleared, then remain unchanged
    printf("Status flags after mismatched checksum: 0x%x (lower 24 bits cleared, then no further change)\n", my_context->status_flags);
    printf("Buffer ptr after mismatched checksum: %p (should NOT be NULL)\n", (void*)my_context->buffer_ptr);

    // Clean up allocated memory if _terminate was commented out
    if (my_context->data_ptr) free(my_context->data_ptr);
    if (my_context->buffer_ptr) free(my_context->buffer_ptr);
    free(my_context);

    return 0; // This line is reached only if _terminate calls are commented out in decode.
}