#include <stddef.h> // For size_t
#include <stdint.h> // For uintptr_t (if converting int to ptr, but char* is better)
#include <stdio.h>  // For printf, fprintf
#include <stdlib.h> // For EXIT_SUCCESS, EXIT_FAILURE
#include <string.h> // For memcpy, memset
#include <unistd.h> // For ssize_t (though int is used in original)
#include <stdbool.h> // For bool

// --- Dummy I/O functions for compilation and testing ---
// In a real application, these would be actual system calls or library functions.

// A simple global buffer to simulate data transfer.
// This is not thread-safe and very basic, just for demonstration.
#define DUMMY_IO_BUFFER_SIZE 512
static char dummy_io_buffer[DUMMY_IO_BUFFER_SIZE];
static size_t dummy_io_buffer_data_len = 0; // Amount of valid data in the buffer
static size_t dummy_io_buffer_read_pos = 0; // Position for next read

// Counter to simulate errors for demonstration purposes
static int io_call_counter = 0;

// Dummy transmit function
// Simulates writing data. Returns 0 on success, -1 on error.
// The actual number of bytes transferred is written to bytes_transferred.
// fd is ignored in this dummy implementation, but kept for signature compatibility.
int transmit(int fd, const void *buf, size_t len, int *bytes_transferred) {
    io_call_counter++;
    printf("DUMMY: transmit(fd=%d, buf=%p, len=%zu) - Call %d\n", fd, buf, len, io_call_counter);

    // Simulate an error after a few calls to test error handling
    if (io_call_counter == 7) { // Example: error on 7th call
        fprintf(stderr, "DUMMY: Simulating transmit error for fd %d\n", fd);
        *bytes_transferred = 0;
        return -1;
    }

    // Simulate partial write if buffer is almost full
    size_t available_space = DUMMY_IO_BUFFER_SIZE - dummy_io_buffer_data_len;
    size_t bytes_to_write = (len < available_space) ? len : available_space;

    if (bytes_to_write == 0) {
        *bytes_transferred = 0;
        printf("DUMMY: No space in buffer, transmitted 0 bytes.\n");
        return 0; // No error, just no space
    }

    memcpy(dummy_io_buffer + dummy_io_buffer_data_len, buf, bytes_to_write);
    dummy_io_buffer_data_len += bytes_to_write;
    *bytes_transferred = (int)bytes_to_write; // Cast to int as per original snippet's local_18 type
    printf("DUMMY: Transmitted %d bytes. Buffer data len: %zu\n", *bytes_transferred, dummy_io_buffer_data_len);
    return 0; // Success
}

// Dummy receive function
// Simulates reading data. Returns 0 on success, -1 on error.
// The actual number of bytes transferred is written to bytes_transferred.
// fd is ignored in this dummy implementation, but kept for signature compatibility.
int receive(int fd, void *buf, size_t len, int *bytes_transferred) {
    io_call_counter++;
    printf("DUMMY: receive(fd=%d, buf=%p, len=%zu) - Call %d\n", fd, buf, len, io_call_counter);

    // Simulate an error after a few calls to test error handling
    if (io_call_counter == 10) { // Example: error on 10th call
        fprintf(stderr, "DUMMY: Simulating receive error for fd %d\n", fd);
        *bytes_transferred = 0;
        return -1;
    }

    size_t bytes_available = dummy_io_buffer_data_len - dummy_io_buffer_read_pos;
    size_t bytes_to_read = (len < bytes_available) ? len : bytes_available;

    if (bytes_to_read == 0) {
        *bytes_transferred = 0;
        printf("DUMMY: No data in buffer, received 0 bytes.\n");
        return 0; // No error, just no data (might indicate EOF)
    }

    memcpy(buf, dummy_io_buffer + dummy_io_buffer_read_pos, bytes_to_read);
    dummy_io_buffer_read_pos += bytes_to_read;
    *bytes_transferred = (int)bytes_to_read; // Cast to int as per original snippet's local_18 type
    printf("DUMMY: Received %d bytes. Buffer read pos: %zu, data len: %zu\n", *bytes_transferred, dummy_io_buffer_read_pos, dummy_io_buffer_data_len);

    // If all data has been read, reset the buffer state
    if (dummy_io_buffer_read_pos == dummy_io_buffer_data_len) {
        dummy_io_buffer_read_pos = 0;
        dummy_io_buffer_data_len = 0;
        printf("DUMMY: Buffer cleared after full read.\n");
    }
    return 0; // Success
}

// --- Refactored functions from the snippet ---

// Function: SendAll
// Sends 'length' bytes from 'buffer' using file descriptor 1.
// Returns 0 on success, -1 on failure.
int SendAll(const char *buffer, size_t length) {
    size_t total_sent = 0;
    int bytes_this_call;
    int ret_code;

    while (total_sent < length) {
        ret_code = transmit(1, buffer + total_sent, length - total_sent, &bytes_this_call);

        if (ret_code != 0 || bytes_this_call == 0) {
            // Error occurred or transmit returned 0 bytes, cannot complete.
            fprintf(stderr, "SendAll error: transmit failed or sent 0 bytes. ret_code=%d, bytes_this_call=%d\n", ret_code, bytes_this_call);
            return -1;
        }
        total_sent += bytes_this_call;
    }
    return 0; // All bytes sent successfully
}

// Function: SendNull
// Sends 'count' null bytes using file descriptor 1.
// Returns 0 on success, -1 on failure.
int SendNull(size_t count) {
    size_t total_sent = 0;
    const char null_byte = 0; // The byte to send (a single null byte)
    int bytes_this_call;
    int ret_code;

    while (total_sent < count) {
        ret_code = transmit(1, &null_byte, 1, &bytes_this_call);

        if (ret_code != 0 || bytes_this_call != 1) {
            // Error occurred or transmit didn't send exactly 1 byte, cannot complete.
            fprintf(stderr, "SendNull error: transmit failed or did not send 1 byte. ret_code=%d, bytes_this_call=%d\n", ret_code, bytes_this_call);
            return -1;
        }
        total_sent++;
    }
    return 0; // All null bytes sent successfully
}

// Function: ReceiveAll
// Receives 'length' bytes into 'buffer' using file descriptor 0.
// Returns 0 on success, -1 on failure.
int ReceiveAll(char *buffer, size_t length) {
    size_t total_received = 0;
    int bytes_this_call;
    int ret_code;

    while (total_received < length) {
        ret_code = receive(0, buffer + total_received, length - total_received, &bytes_this_call);

        if (ret_code != 0 || bytes_this_call == 0) {
            // Error occurred or receive returned 0 bytes (e.g., EOF or actual error), cannot complete.
            fprintf(stderr, "ReceiveAll error: receive failed or received 0 bytes. ret_code=%d, bytes_this_call=%d\n", ret_code, bytes_this_call);
            return -1;
        }
        total_received += bytes_this_call;
    }
    return 0; // All bytes received successfully
}

// --- Main function for testing ---
#define TEST_DATA_SIZE 64
char my_data[TEST_DATA_SIZE];
char received_data[TEST_DATA_SIZE];

int main() {
    printf("--- Starting Test Cases ---\n\n");

    // Initialize test data
    for (int i = 0; i < TEST_DATA_SIZE; ++i) {
        my_data[i] = 'A' + (i % 26);
    }
    my_data[TEST_DATA_SIZE - 1] = '\0'; // Null-terminate for printing

    printf("Test 1: SendAll %d bytes\n", TEST_DATA_SIZE);
    if (SendAll(my_data, TEST_DATA_SIZE) == 0) {
        printf("SendAll successful.\n");
    } else {
        printf("SendAll failed.\n");
        return EXIT_FAILURE;
    }
    printf("\n");

    printf("Test 2: ReceiveAll %d bytes\n", TEST_DATA_SIZE);
    memset(received_data, 0, TEST_DATA_SIZE); // Clear buffer
    if (ReceiveAll(received_data, TEST_DATA_SIZE) == 0) {
        printf("ReceiveAll successful. Received data: '%.*s'\n", TEST_DATA_SIZE, received_data);
        if (memcmp(my_data, received_data, TEST_DATA_SIZE) == 0) {
            printf("Received data matches sent data.\n");
        } else {
            printf("Received data DOES NOT match sent data.\n");
        }
    } else {
        printf("ReceiveAll failed.\n");
        return EXIT_FAILURE;
    }
    printf("\n");

    printf("Test 3: SendNull 10 bytes\n");
    if (SendNull(10) == 0) {
        printf("SendNull successful.\n");
    } else {
        printf("SendNull failed.\n");
        return EXIT_FAILURE;
    }
    printf("\n");

    printf("Test 4: ReceiveAll 10 null bytes\n");
    memset(received_data, 0xFF, 10); // Fill with non-zero to check for nulls
    if (ReceiveAll(received_data, 10) == 0) {
        printf("ReceiveAll successful for nulls. Received: ");
        for (int i = 0; i < 10; ++i) {
            printf("%02x ", (unsigned char)received_data[i]);
        }
        printf("\n");
        bool all_null = true;
        for (int i = 0; i < 10; ++i) {
            if (received_data[i] != 0) {
                all_null = false;
                break;
            }
        }
        if (all_null) {
            printf("All 10 bytes are null.\n");
        } else {
            printf("Not all 10 bytes are null.\n");
        }
    } else {
        printf("ReceiveAll for nulls failed.\n");
        return EXIT_FAILURE;
    }
    printf("\n");

    // Test error case for SendAll (will hit transmit_call_counter == 7)
    printf("Test 5: SendAll with simulated error (should fail)\n");
    if (SendAll(my_data, TEST_DATA_SIZE) == 0) {
        printf("SendAll unexpectedly succeeded (error simulation failed).\n");
        return EXIT_FAILURE;
    } else {
        printf("SendAll correctly failed due to simulated error.\n");
    }
    printf("\n");

    // Test error case for ReceiveAll (will hit io_call_counter == 10)
    // First, put some data into the buffer for receive to attempt to read
    printf("Test 6: SendAll some data to set up receive error test.\n");
    if (SendAll(my_data, 10) != 0) {
        printf("Setup SendAll failed, cannot proceed with receive error test.\n");
        return EXIT_FAILURE;
    }
    printf("Test 7: ReceiveAll with simulated error (should fail)\n");
    memset(received_data, 0, 10);
    if (ReceiveAll(received_data, 10) == 0) {
        printf("ReceiveAll unexpectedly succeeded (error simulation failed).\n");
        return EXIT_FAILURE;
    } else {
        printf("ReceiveAll correctly failed due to simulated error.\n");
    }
    printf("\n");

    printf("--- All Tests Completed ---\n");
    return EXIT_SUCCESS;
}