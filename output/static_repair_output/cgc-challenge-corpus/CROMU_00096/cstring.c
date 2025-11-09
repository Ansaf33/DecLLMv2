#include <stdlib.h> // For calloc, free, exit
#include <stdio.h>  // For printf, fprintf
#include <string.h> // For strcpy, size_t

// Dummy external functions for compilation
// These are simplified to return success and process the full requested length.
// In a real application, they would interact with network sockets.
// For ReceiveBytes:
// - If requesting size of unsigned short, it simulates receiving a length of 5.
// - Otherwise, it fills the buffer with 'X' characters.
// For TransmitBytes:
// - It just simulates successful transmission.
static int receive(void* buffer, int length, int* bytes_processed_out) {
    if (bytes_processed_out) {
        *bytes_processed_out = length; // Simulate reading all requested bytes
    }
    // For demonstration, provide specific data for CString length and content
    if (buffer != NULL) {
        if (length == sizeof(unsigned short)) {
            *(unsigned short*)buffer = 5; // Simulate receiving a length of 5
        } else {
            // Fill data with 'X' for string content
            for (int i = 0; i < length; ++i) {
                ((char*)buffer)[i] = 'X';
            }
        }
    }
    return 0; // Simulate success
}

static int transmit(const void* buffer, int length, int* bytes_processed_out) {
    if (bytes_processed_out) {
        *bytes_processed_out = length; // Simulate writing all requested bytes
    }
    // In a real scenario, this would send data over a network.
    // For this exercise, we just acknowledge the call.
    return 0; // Simulate success
}

static void _terminate() {
    fprintf(stderr, "FATAL: Program terminated due to an error.\n");
    exit(1);
}

// CString structure definition
typedef struct CString {
    unsigned short length; // Actual string length (excluding null terminator)
    char* data;            // Pointer to the character array (null-terminated)
} CString;

// Function: ReceiveBytes
int ReceiveBytes(void* buffer, int count) {
    if (buffer == NULL && count > 0) {
        return -1; // Attempt to read into a NULL buffer with a non-zero count
    }
    if (count == 0) {
        return 0; // Nothing to receive
    }

    int total_received = 0;
    int bytes_received_this_call;
    int status;

    while (total_received < count) {
        status = receive((char*)buffer + total_received, count - total_received, &bytes_received_this_call);

        if (status != 0) {
            _terminate(); // Error during receive operation
        }

        if (bytes_received_this_call == 0) {
            // If 0 bytes received, and we still need more, it's an error (EOF or peer closed)
            _terminate();
        }
        
        total_received += bytes_received_this_call;
    }
    return total_received;
}

// Function: TransmitBytes
int TransmitBytes(const void* buffer, int count) {
    if (buffer == NULL && count > 0) {
        return -1; // Attempt to transmit from a NULL buffer with a non-zero count
    }
    if (count == 0) {
        return 0; // Nothing to transmit
    }

    int total_transmitted = 0;
    int bytes_transmitted_this_call;
    int status;

    while (total_transmitted < count) {
        status = transmit((const char*)buffer + total_transmitted, count - total_transmitted, &bytes_transmitted_this_call);

        if (status != 0) {
            _terminate(); // Error during transmit operation
        }

        if (bytes_transmitted_this_call == 0) {
            // If 0 bytes transmitted, and we still need more, it's an error
            _terminate();
        }
        
        total_transmitted += bytes_transmitted_this_call;
    }
    return 0; // Original function returns 0
}

// Function: DestroyCString
void DestroyCString(CString* cs) {
    if (cs != NULL) {
        free(cs->data); // Free the string data buffer
        free(cs);       // Free the CString structure itself
    }
}

// Function: ReceiveCString
CString* ReceiveCString(unsigned int max_len) {
    CString* cs = (CString*)calloc(1, sizeof(CString));
    if (cs == NULL) {
        fprintf(stderr, "Error: Failed to allocate CString structure.\n");
        return NULL;
    }

    // Receive the length of the string (2 bytes for unsigned short)
    if (ReceiveBytes(&cs->length, sizeof(cs->length)) != sizeof(cs->length)) {
        fprintf(stderr, "Error: Failed to receive CString length.\n");
        DestroyCString(cs);
        return NULL;
    }

    // Check if received length exceeds maximum allowed length
    if (max_len != 0 && cs->length > max_len) {
        fprintf(stderr, "FATAL: Received string length %hu exceeds maximum allowed %u.\n", cs->length, max_len);
        _terminate();
    }

    if (cs->length > 0) {
        // Allocate space for string data + null terminator
        cs->data = (char*)calloc(cs->length + 1, sizeof(char));
        if (cs->data == NULL) {
            fprintf(stderr, "Error: Failed to allocate CString data buffer.\n");
            DestroyCString(cs);
            return NULL;
        }

        // Receive the actual string bytes
        if (ReceiveBytes(cs->data, cs->length) != cs->length) {
            fprintf(stderr, "Error: Failed to receive CString data.\n");
            DestroyCString(cs);
            return NULL;
        }
        // calloc already ensures null termination at cs->data[cs->length]
    } else {
        // If length is 0, allocate space for just a null terminator
        cs->data = (char*)calloc(1, sizeof(char));
        if (cs->data == NULL) {
            fprintf(stderr, "Error: Failed to allocate CString data for empty string.\n");
            DestroyCString(cs);
            return NULL;
        }
    }
    
    return cs;
}

// Function: TransmitCString
void TransmitCString(const CString* cs) {
    if (cs == NULL) {
        return; // Nothing to transmit if CString is NULL
    }

    // Transmit the length of the string (2 bytes for unsigned short)
    TransmitBytes(&cs->length, sizeof(cs->length));

    // Transmit the actual string data if length > 0
    if (cs->length > 0) {
        TransmitBytes(cs->data, cs->length);
    }
}

// Main function for compilation and basic demonstration
int main() {
    printf("Starting program...\n");

    // --- Demonstrate TransmitCString ---
    printf("\n--- Transmit CString Example ---\n");
    CString* test_send_string = (CString*)calloc(1, sizeof(CString));
    if (test_send_string == NULL) {
        fprintf(stderr, "Main: Calloc failed for test_send_string structure.\n");
        return 1;
    }
    test_send_string->length = 7;
    test_send_string->data = (char*)calloc(test_send_string->length + 1, sizeof(char));
    if (test_send_string->data == NULL) {
        fprintf(stderr, "Main: Calloc failed for test_send_string data.\n");
        DestroyCString(test_send_string); // Clean up the struct if data allocation fails
        return 1;
    }
    strcpy(test_send_string->data, "Testing");
    printf("Transmitting CString: length=%hu, data='%s'\n", test_send_string->length, test_send_string->data);
    TransmitCString(test_send_string);
    DestroyCString(test_send_string); // Free the heap-allocated CString struct and its data

    // --- Demonstrate ReceiveCString ---
    printf("\n--- Receive CString Example ---\n");
    // Dummy 'receive' is configured to return length 5 and data "XXXXX"
    CString* received_cs = ReceiveCString(10); // Max length 10
    if (received_cs) {
        printf("Received CString: length=%hu, data='%s'\n", received_cs->length, received_cs->data);
        DestroyCString(received_cs); // Free the heap-allocated CString
    } else {
        printf("Failed to receive CString.\n");
    }

    printf("\nProgram finished successfully.\n");
    return 0;
}