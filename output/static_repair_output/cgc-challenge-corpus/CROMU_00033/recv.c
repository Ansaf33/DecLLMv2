#include <stdio.h>    // For printf
#include <string.h>   // For memset
#include <unistd.h>   // For ssize_t (though sys/types.h is also common)
#include <sys/types.h> // For ssize_t

// Dummy 'receive' function to make the code compilable.
// In a real application, this would be a system call like `read` or `recv`.
// It simulates reading up to `nbytes_to_read` bytes and stores the actual
// count in `bytes_read_out`. Returns 0 on success, -1 on error.
static int receive(int fd, void *buf, size_t nbytes_to_read, ssize_t *bytes_read_out) {
    if (fd <= 0) { // Simulate an invalid file descriptor error
        *bytes_read_out = 0;
        return -1;
    }

    // Simulate receiving a fixed small amount of data, or less if not enough is requested.
    size_t simulated_read_size = 5;
    if (nbytes_to_read < simulated_read_size) {
        simulated_read_size = nbytes_to_read;
    }

    if (simulated_read_size == 0) {
        *bytes_read_out = 0;
        return 0; // Simulate EOF or no data available
    }

    // Fill the buffer with dummy data for demonstration
    memset(buf, 'A' + fd, simulated_read_size);
    *bytes_read_out = simulated_read_size;

    printf("[DUMMY_RECEIVE] fd: %d, requested: %zu, received: %zd\n", fd, nbytes_to_read, *bytes_read_out);
    return 0; // Success
}

// Renamed from recv to my_recv to avoid conflict with the standard library function
ssize_t my_recv(int __fd, void *__buf, size_t __n, int __flags) {
    ssize_t total_received = 0;
    ssize_t current_received = 0;

    // Original logic: if __fd == 0, return 0. Keeping this behavior.
    if (__fd == 0) {
        return 0;
    }

    // Loop until the requested number of bytes (__n) has been received
    // or an error/EOF occurs.
    while (total_received < (ssize_t)__n) {
        size_t remaining_bytes = __n - total_received;

        // Call the internal 'receive' function.
        // The buffer pointer is advanced by `total_received` bytes for each call.
        int result = receive(__fd, (char *)__buf + total_received, remaining_bytes, &current_received);

        if (result != 0) {
            printf("[ERROR] Failed to receive data (internal 'receive' call failed)\n");
            // If some data was read before the error, return that amount. Otherwise, return -1.
            return (total_received > 0) ? total_received : -1;
        }

        if (current_received <= 0) {
            // 0 bytes received typically indicates EOF or a graceful peer shutdown.
            // A negative value (if `receive` could return it here) would indicate an error.
            if (current_received == 0) {
                printf("[INFO] Peer disconnected or no more data available.\n");
            } else {
                printf("[ERROR] Internal 'receive' function returned negative bytes received.\n");
            }
            break; // Exit loop, return whatever was received so far.
        }

        total_received += current_received;
    }

    return total_received;
}

// Main function for testing purposes
int main() {
    char buffer[100];
    ssize_t bytes_read;

    printf("--- Test Case 1: Read 10 bytes from fd=1 ---\n");
    memset(buffer, 0, sizeof(buffer)); // Clear buffer
    bytes_read = my_recv(1, buffer, 10, 0);
    printf("my_recv returned: %zd\n", bytes_read);
    printf("Buffer content: '%.*s'\n", (int)bytes_read, buffer);

    printf("\n--- Test Case 2: Read 20 bytes from fd=2 ---\n");
    memset(buffer, 0, sizeof(buffer)); // Clear buffer
    bytes_read = my_recv(2, buffer, 20, 0);
    printf("my_recv returned: %zd\n", bytes_read);
    printf("Buffer content: '%.*s'\n", (int)bytes_read, buffer);

    printf("\n--- Test Case 3: Read 5 bytes from fd=0 (should return 0) ---\n");
    memset(buffer, 0, sizeof(buffer)); // Clear buffer
    bytes_read = my_recv(0, buffer, 5, 0);
    printf("my_recv returned: %zd\n", bytes_read);
    printf("Buffer content: '%.*s'\n", (int)bytes_read, buffer);

    printf("\n--- Test Case 4: Read 3 bytes from fd=3 (small read) ---\n");
    memset(buffer, 0, sizeof(buffer)); // Clear buffer
    bytes_read = my_recv(3, buffer, 3, 0);
    printf("my_recv returned: %zd\n", bytes_read);
    printf("Buffer content: '%.*s'\n", (int)bytes_read, buffer);
    
    printf("\n--- Test Case 5: Read 1 byte from fd=4 (even smaller read) ---\n");
    memset(buffer, 0, sizeof(buffer)); // Clear buffer
    bytes_read = my_recv(4, buffer, 1, 0);
    printf("my_recv returned: %zd\n", bytes_read);
    printf("Buffer content: '%.*s'\n", (int)bytes_read, buffer);

    return 0;
}