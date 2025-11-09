#include <stdio.h>   // For printf, fprintf
#include <stdlib.h>  // For malloc, calloc, free
#include <string.h>  // For strlen, strncpy, strcpy, strcat
#include <limits.h>  // For USHRT_MAX

// Define the Response structure based on the original access patterns
// and calloc(8) implying 32-bit architecture or specific padding.
// On a 32-bit system (e.g., x86), sizeof(unsigned short) is 2, sizeof(char*) is 4.
// The compiler will typically add 2 bytes of padding after 'length' to align 'data' to a 4-byte boundary,
// making sizeof(Response) = 2 (length) + 2 (padding) + 4 (data pointer) = 8 bytes.
// On a 64-bit system (e.g., x86_64), sizeof(unsigned short) is 2, sizeof(char*) is 8.
// The compiler will typically add 6 bytes of padding after 'length' to align 'data' to an 8-byte boundary,
// making sizeof(Response) = 2 (length) + 6 (padding) + 8 (data pointer) = 16 bytes.
// Using sizeof(Response) in calloc is the standard and correct way, allowing for platform differences.
typedef struct Response {
    unsigned short length;
    char *data;
} Response;

// --- Mock functions for compilation ---
// In a real application, these would be linked from an external library or defined elsewhere.
// For this problem, they are provided as static mocks to allow compilation.
static int SendBytes(const void *buf, size_t len) {
    // For demonstration purposes, this mock function simulates a successful send
    // by returning the number of bytes requested to send.
    // In a real scenario, network or device I/O would occur here.
    // (Optional: Uncomment the printf for debugging what would be sent)
    // printf("Mock SendBytes: Attempted to send %zu bytes.\n", len);
    return (int)len; // Simulate successful send
}

// Mock global variable, representing DAT_0001504b
static const char DAT_0001504b = 0x42; // Example value, could be any char
// --- End Mock functions ---


// Function: DestroyResponse
int DestroyResponse(void *param_1) {
    Response *resp = (Response *)param_1;
    if (resp != NULL) {
        if (resp->data != NULL) {
            free(resp->data);
            resp->data = NULL; // Clear pointer after freeing
        }
        free(resp);
    }
    return 0;
}

// Function: GenerateBlankResponse
void *GenerateBlankResponse(void) {
    // calloc(1, sizeof(Response)) allocates memory for one Response struct
    // and initializes all its members to zero.
    // This means resp->length will be 0 and resp->data will be NULL.
    return calloc(1, sizeof(Response));
}

// Function: AddToResponse
int AddToResponse(Response *resp, const char *new_segment) {
    if (resp == NULL || new_segment == NULL) {
        return -1; // Return -1 for error
    }

    size_t new_segment_len = strlen(new_segment);
    size_t total_len = new_segment_len;

    if (resp->data != NULL) {
        total_len += resp->length;
    }

    // Check for potential overflow if the combined length exceeds what
    // an unsigned short can hold.
    if (total_len > USHRT_MAX) {
        fprintf(stderr, "Error: New response length (%zu) exceeds maximum for unsigned short (%hu).\n", total_len, USHRT_MAX);
        return -1;
    }

    char *new_data_buffer = (char *)calloc(total_len + 1, sizeof(char));
    if (new_data_buffer == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for new response data.\n");
        return -1;
    }

    if (resp->data != NULL) {
        // Copy existing data. calloc ensures new_data_buffer is zero-initialized,
        // so new_data_buffer[resp->length] will be '\0' after this copy.
        strncpy(new_data_buffer, resp->data, resp->length);
        // strncpy does not guarantee null-termination if source is longer than n.
        // Here, resp->length is the exact length of resp->data, which is null-terminated.
        // The byte at index resp->length in new_data_buffer is already '\0' from calloc.
    }

    // Concatenate the new segment.
    // This appends starting from the null terminator (or implicit null from calloc).
    strcat(new_data_buffer, new_segment);

    free(resp->data); // Free the old data buffer
    resp->data = new_data_buffer;
    resp->length = (unsigned short)total_len;

    return 0; // Return 0 for success
}

// Function: DumpResponse
int DumpResponse(const Response *resp) {
    if (resp == NULL) {
        return -1; // Error
    }
    printf("Response Length: %u\n", (unsigned int)resp->length);
    printf("Response Data: %s\n", resp->data != NULL ? resp->data : "(null)");
    return 0; // Success
}

// Function: SendResponse
int SendResponse(const Response *resp) {
    if (resp == NULL) {
        return -1; // Error
    }

    char header_byte = 5;
    if (SendBytes(&header_byte, 1) != 1) {
        return -1;
    }

    // Send the length field (sizeof(unsigned short) bytes, typically 2 bytes)
    if (SendBytes(&(resp->length), sizeof(resp->length)) != (int)sizeof(resp->length)) {
        return -1;
    }

    // Send the response data if it exists and has length
    if (resp->data != NULL && resp->length > 0) {
        if (SendBytes(resp->data, resp->length) != (int)resp->length) {
            return -1;
        }
    } else if (resp->length > 0) {
        // This case indicates an inconsistency: length > 0 but data is NULL.
        fprintf(stderr, "Warning: Response length is %u but data pointer is NULL.\n", (unsigned int)resp->length);
        return -1;
    }

    // Send the trailer byte
    if (SendBytes(&DAT_0001504b, 1) != 1) {
        return -1;
    }

    return 0; // Success
}

// Main function for demonstration
int main() {
    Response *my_response = GenerateBlankResponse();
    if (my_response == NULL) {
        fprintf(stderr, "Error: Failed to generate blank response.\n");
        return 1;
    }

    printf("--- Initial Response ---\n");
    DumpResponse(my_response);

    printf("\n--- Adding 'Hello' ---\n");
    if (AddToResponse(my_response, "Hello") == -1) {
        fprintf(stderr, "Error: Failed to add 'Hello'.\n");
        DestroyResponse(my_response);
        return 1;
    }
    DumpResponse(my_response);

    printf("\n--- Adding ' World!' ---\n");
    if (AddToResponse(my_response, " World!") == -1) {
        fprintf(stderr, "Error: Failed to add ' World!'.\n");
        DestroyResponse(my_response);
        return 1;
    }
    DumpResponse(my_response);

    printf("\n--- Sending Response ---\n");
    if (SendResponse(my_response) == -1) {
        fprintf(stderr, "Error: Failed to send response.\n");
        DestroyResponse(my_response);
        return 1;
    }
    printf("Response sent successfully (mock).\n");

    printf("\n--- Destroying Response ---\n");
    DestroyResponse(my_response);
    my_response = NULL; // Clear pointer after freeing to prevent use-after-free

    printf("Demonstration complete.\n");

    return 0;
}