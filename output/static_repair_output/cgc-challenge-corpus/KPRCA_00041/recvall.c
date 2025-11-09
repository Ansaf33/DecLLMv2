#include <sys/socket.h> // For recv
#include <stddef.h>     // For size_t
#include <unistd.h>     // For ssize_t

// Function: recvall
// Original signature: undefined4 recvall(undefined4 param_1,int param_2,int param_3)
// Interpreted as:
// param_1: int sockfd (socket file descriptor)
// param_2: void* buf_start (pointer to the beginning of the buffer)
// param_3: size_t len_to_read (total number of bytes to read, this value will be decremented)
// Returns: int (0 for success, -1 for failure)
int recvall(int sockfd, void *buf_start, size_t len_to_read) {
    // current_buf_ptr tracks the current position in the buffer to write to.
    // It's initialized to the start of the buffer and advances with each read.
    char *current_buf_ptr = (char *)buf_start;
    // bytes_received_this_call stores the return value of each recv() call.
    ssize_t bytes_received_this_call;

    // Loop until all requested bytes have been read (len_to_read becomes 0).
    while (len_to_read > 0) {
        // Attempt to receive data into the current buffer position.
        // The amount to read is the remaining length (len_to_read).
        bytes_received_this_call = recv(sockfd, current_buf_ptr, len_to_read, 0);

        // Check for errors or peer closing the connection.
        // If recv returns -1, an error occurred.
        // If recv returns 0, the peer has performed an orderly shutdown or no data is available.
        // In both cases, for a 'recvall' function, this signifies a failure to receive all data.
        if (bytes_received_this_call <= 0) {
            return -1; // Return -1 for failure (original 0xffffffff)
        }

        // Data was successfully received.
        // Advance the buffer pointer by the number of bytes just received.
        current_buf_ptr += bytes_received_this_call;
        // Decrease the remaining length by the number of bytes just received.
        len_to_read -= bytes_received_this_call;
    }

    // If the loop completes, all bytes have been successfully received.
    return 0; // Return 0 for success
}