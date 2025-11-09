#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>

// Function: read_until
// param_1: file descriptor (int)
// param_2: buffer (char*)
// param_3: max length (size_t)
// param_4: delimiter (char)
int read_until(int fd, char *buffer, size_t max_len, char delimiter) {
    size_t i;
    ssize_t bytes_read_count;

    for (i = 0; i < max_len; ++i) {
        bytes_read_count = recv(fd, buffer + i, 1, 0);

        if (bytes_read_count <= 0) { // Error (-1) or EOF (0)
            return -1;
        }

        if (buffer[i] == delimiter) {
            buffer[i] = '\0';
            return i + 1; // Total bytes read including the delimiter
        }
    }

    // If max_len characters were read without finding the delimiter
    buffer[max_len - 1] = '\0'; // Null-terminate the buffer at the last position
    return max_len;             // Return max_len bytes read
}

// Function: read_n
// param_1: file descriptor (int)
// param_2: buffer (char*)
// param_3: number of bytes to read (size_t)
int read_n(int fd, char *buffer, size_t num_bytes_to_read) {
    size_t i;
    ssize_t bytes_read_count;

    for (i = 0; i < num_bytes_to_read; ++i) {
        bytes_read_count = recv(fd, buffer + i, 1, 0);

        if (bytes_read_count <= 0) { // Error (-1) or EOF (0)
            break; // Stop reading
        }
    }
    return i; // Total bytes read
}

// Main function for demonstration
int main() {
    int pipefd[2]; // pipefd[0] for read, pipefd[1] for write
    char buffer[256];
    int bytes_returned;

    printf("--- Testing read_until ---\n");

    // Test 1: Delimiter found
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }
    const char *test_str1 = "Hello,World!\n";
    write(pipefd[1], test_str1, strlen(test_str1));
    memset(buffer, 0, sizeof(buffer)); // Clear buffer
    bytes_returned = read_until(pipefd[0], buffer, sizeof(buffer), ',');
    printf("read_until (delimiter ','): Returned %d, Buffer: \"%s\"\n", bytes_returned, buffer);
    close(pipefd[0]);
    close(pipefd[1]);

    // Test 2: Delimiter not found, max_len reached
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }
    const char *test_str2 = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    write(pipefd[1], test_str2, strlen(test_str2));
    memset(buffer, 0, sizeof(buffer));
    bytes_returned = read_until(pipefd[0], buffer, 10, 'Z');
    printf("read_until (no delimiter, max_len 10): Returned %d, Buffer: \"%s\"\n", bytes_returned, buffer);
    close(pipefd[0]);
    close(pipefd[1]);

    // Test 3: EOF before delimiter/max_len
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }
    const char *test_str3 = "Short";
    write(pipefd[1], test_str3, strlen(test_str3));
    close(pipefd[1]); // Close write end to signal EOF
    memset(buffer, 0, sizeof(buffer));
    bytes_returned = read_until(pipefd[0], buffer, sizeof(buffer), '!');
    printf("read_until (EOF): Returned %d, Buffer: \"%s\"\n", bytes_returned, buffer);
    close(pipefd[0]);

    printf("\n--- Testing read_n ---\n");

    // Test 4: Read specific number of bytes
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }
    const char *test_str4 = "Some data to read.";
    write(pipefd[1], test_str4, strlen(test_str4));
    memset(buffer, 0, sizeof(buffer));
    bytes_returned = read_n(pipefd[0], buffer, 8);
    printf("read_n (read 8 bytes): Returned %d, Buffer: \"%s\"\n", bytes_returned, buffer);
    close(pipefd[0]);
    close(pipefd[1]);

    // Test 5: Read more than available (EOF)
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }
    const char *test_str5 = "Small";
    write(pipefd[1], test_str5, strlen(test_str5));
    close(pipefd[1]); // Close write end to signal EOF
    memset(buffer, 0, sizeof(buffer));
    bytes_returned = read_n(pipefd[0], buffer, 10);
    printf("read_n (EOF): Returned %d, Buffer: \"%s\"\n", bytes_returned, buffer);
    close(pipefd[0]);

    return EXIT_SUCCESS;
}