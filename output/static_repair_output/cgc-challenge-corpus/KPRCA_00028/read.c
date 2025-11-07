#include <sys/socket.h> // For recv
#include <sys/types.h>  // For ssize_t
#include <ctype.h>      // For isspace

int read_balanced_expression(int fd, char *buffer, int max_len) {
    int balance = 0;
    ssize_t bytes_received;
    char current_char;

    char *current_write_ptr = buffer;
    int remaining_capacity = max_len;

    while (remaining_capacity > 0 && (bytes_received = recv(fd, current_write_ptr, 1, 0)) > 0) {
        current_char = *current_write_ptr;

        if (balance == 0 && current_char != '(') {
            return -1;
        }

        if (current_char == '(') {
            balance++;
        } else if (current_char == ')') {
            if (balance == 0) {
                return -1;
            }
            balance--;
        }

        remaining_capacity--;
        current_write_ptr++;

        if (remaining_capacity == 0 || balance == 0) {
            break;
        }
    }

    if (bytes_received == -1) {
        return -1;
    }

    while ((bytes_received = recv(fd, &current_char, 1, 0)) > 0 && current_char != '\n') {
        if (!isspace((unsigned char)current_char)) {
            return -1;
        }
    }

    if (bytes_received == -1) {
        return -1;
    }

    *current_write_ptr = '\0';

    return balance;
}