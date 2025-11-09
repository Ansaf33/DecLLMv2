#include <stdio.h>      // For printf, perror
#include <stdlib.h>     // For EXIT_SUCCESS, EXIT_FAILURE
#include <string.h>     // For memset, strlen
#include <unistd.h>     // For close, ssize_t, pipe, write
#include <sys/socket.h> // For socket, connect, recv, AF_INET, SOCK_STREAM, send
#include <netinet/in.h> // For sockaddr_in, INADDR_LOOPBACK, htons, htonl
#include <arpa/inet.h>  // For inet_pton (optional, if using specific IP string)
#include <stddef.h>     // For size_t

// Function: read_until
// Reads from the socket until 'delimiter' is found, 'max_len' bytes are read,
// or an error/EOF occurs.
// The delimiter character itself is replaced by a null terminator.
// Returns the total number of bytes processed (including the delimiter if found),
// or -1 on error/EOF before any character could be successfully read.
int read_until(int sockfd, char *buffer, size_t max_len, char delimiter) {
    size_t i;
    for (i = 0; i < max_len; ++i) {
        ssize_t bytes_received = recv(sockfd, buffer + i, 1, 0);
        if (bytes_received <= 0) { // Error (-1) or connection closed (0)
            if (i > 0) { // If some bytes were read before error/EOF, null-terminate them
                buffer[i] = '\0';
            }
            return -1; // Indicate error or unexpected EOF
        }
        // Character is now in buffer[i]
        if (buffer[i] == delimiter) {
            buffer[i] = '\0'; // Replace delimiter with null terminator
            return (int)(i + 1); // Return count including the (now replaced) delimiter
        }
    }
    // Loop finished (max_len bytes read) without finding the delimiter
    if (max_len > 0) { // Only null-terminate if buffer has space
        buffer[max_len - 1] = '\0'; // Null-terminate the last character read
    }
    return (int)max_len; // Return total bytes read
}

// Function: read_n
// Reads 'len' bytes from the socket into 'buffer', one byte at a time.
// Stops early on error or EOF. Does not null-terminate.
// Returns the number of bytes successfully read.
int read_n(int sockfd, char *buffer, size_t len) {
    size_t i;
    for (i = 0; i < len; ++i) {
        ssize_t bytes_received = recv(sockfd, buffer + i, 1, 0);
        if (bytes_received <= 0) { // Error (-1) or connection closed (0)
            break; // Stop reading
        }
    }
    return (int)i; // Return the number of bytes successfully read
}

// Main function to demonstrate usage and ensure compilability
int main() {
    int sockfd;
    struct sockaddr_in serv_addr;
    char buffer[256];
    int bytes_read;
    int is_mocked = 0; // Flag to indicate if socket is mocked with a pipe

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        return EXIT_FAILURE;
    }

    // Initialize server address structure
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080); // Example port
    serv_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1

    // Connect to the server
    // This example will connect to localhost:8080. If no server is listening,
    // it will fail and switch to a pipe-based mock for demonstration.
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR connecting to server. Mocking socket connection with pipe for testing.");
        // If connection fails, close the socket and create a pipe to mock it
        close(sockfd);
        int pipe_fds[2];
        if (pipe(pipe_fds) == -1) {
            perror("Failed to create pipe for mocking socket");
            return EXIT_FAILURE;
        }
        sockfd = pipe_fds[0]; // Use read end of pipe as "socket"
        is_mocked = 1;

        // Write some test data into the pipe's write end
        const char *mock_data = "Hello World!\nThis is a test message.";
        write(pipe_fds[1], mock_data, strlen(mock_data));
        close(pipe_fds[1]); // Close write end
    } else {
        printf("Connected to server on 127.0.0.1:8080.\n");
        // Send some data to the server if actual connection is made
        const char *test_msg = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
        send(sockfd, test_msg, strlen(test_msg), 0);
        printf("Sent test message.\n");
    }

    printf("\n--- Testing read_until ---\n");
    memset(buffer, 0, sizeof(buffer)); // Clear buffer
    // Read until newline or buffer full. (sizeof(buffer) - 1 to leave space for null)
    bytes_read = read_until(sockfd, buffer, sizeof(buffer) - 1, '\n');
    if (bytes_read == -1) {
        perror("read_until failed or connection closed unexpectedly");
    } else {
        printf("read_until processed %d bytes. String: '%s'\n", bytes_read, buffer);
    }

    printf("\n--- Testing read_n ---\n");
    memset(buffer, 0, sizeof(buffer)); // Clear buffer
    bytes_read = read_n(sockfd, buffer, 10); // Read exactly 10 bytes
    if (bytes_read <= 0) { // If 0 or less bytes read, it might be EOF or error
        printf("read_n read %d bytes (possibly EOF or error).\n", bytes_read);
    } else {
        buffer[bytes_read] = '\0'; // Manually null-terminate for printing
        printf("read_n read %d bytes. Data: '%s'\n", bytes_read, buffer);
    }

    close(sockfd); // Close the socket or pipe read end
    return EXIT_SUCCESS;
}