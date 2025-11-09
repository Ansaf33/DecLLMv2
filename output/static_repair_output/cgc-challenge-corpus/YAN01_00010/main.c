#include <stdio.h>   // For printf (in mock transmit/receive)
#include <stdlib.h>  // For exit
#include <unistd.h>  // For write, read (in mock transmit/receive)

// Type definitions based on typical decompiler output
typedef unsigned int uint;
typedef unsigned int undefined4; // Assuming 4-byte unsigned integer
typedef char undefined;          // Assuming 1-byte character/byte

// Global variables
uint gBoard[32]; // Assuming gBoard is an array of 32 unsigned integers
static uint DAT_0001417c;
static uint DAT_00014178;
static uint mask[32];
static uint neg_mask[32];

// Helper function to initialize masks
static void initialize_masks(void) {
    for (int i = 0; i < 32; ++i) {
        mask[i] = 1U << i;
        neg_mask[i] = ~(1U << i);
    }
}

// Mock function for transmit. Implement actual I/O or desired behavior.
// handle: file descriptor or custom handle (e.g., 1 for stdout)
// buffer: pointer to data to send
// max_bytes_to_send: maximum number of bytes to attempt to send
// actual_sent_bytes: pointer to an integer to store the actual number of bytes sent
// Returns 0 on success, non-zero on error.
int transmit(uint handle, char* buffer, uint max_bytes_to_send, int* actual_sent_bytes) {
    if (handle == 1) { // Assuming handle 1 corresponds to stdout
        ssize_t sent = write(STDOUT_FILENO, buffer, max_bytes_to_send);
        if (sent == -1) {
            *actual_sent_bytes = 0;
            return -1; // Error
        }
        *actual_sent_bytes = (int)sent;
        return 0; // Success
    }
    // For other handles, provide mock behavior or actual implementation
    *actual_sent_bytes = (int)max_bytes_to_send; // Assume full send for simplicity
    return 0;
}

// Mock function for receive. Implement actual I/O or desired behavior.
// handle: file descriptor or custom handle (e.g., 0 for stdin)
// buffer: pointer to buffer to store received data
// max_bytes_to_receive: maximum number of bytes to attempt to receive
// actual_received_bytes: pointer to an integer to store the actual number of bytes received
// Returns 0 on success, non-zero on error.
int receive(uint handle, char* buffer, uint max_bytes_to_receive, int* actual_received_bytes) {
    if (handle == 0) { // Assuming handle 0 corresponds to stdin
        ssize_t received = read(STDIN_FILENO, buffer, max_bytes_to_receive);
        if (received == -1) {
            *actual_received_bytes = 0;
            return -1; // Error
        }
        *actual_received_bytes = (int)received;
        return 0; // Success
    }
    // For other handles, provide mock behavior or actual implementation
    if (max_bytes_to_receive > 0) {
        buffer[0] = '\n'; // Default mock input, change as needed for testing
        *actual_received_bytes = 1;
    } else {
        *actual_received_bytes = 0;
    }
    return 0;
}

// Function: transmit_all
uint transmit_all(uint handle, char* buffer, uint total_bytes_to_send) {
    int current_transmitted_bytes;
    uint total_transmitted_bytes = 0;

    if (buffer == NULL || total_bytes_to_send == 0) {
        return 0;
    }

    do {
        int ret_code = transmit(handle, buffer + total_transmitted_bytes, total_bytes_to_send - total_transmitted_bytes, &current_transmitted_bytes);
        if (ret_code != 0 || current_transmitted_bytes == 0) {
            return 0; // Error or no bytes transmitted in this call
        }
        total_transmitted_bytes += current_transmitted_bytes;
    } while (total_transmitted_bytes < total_bytes_to_send);

    return total_bytes_to_send; // Returns the initially requested amount if successful
}

// Function: receive_all
uint receive_all(uint handle, char* buffer, uint total_bytes_to_receive) {
    int current_received_bytes;
    uint total_received_bytes = 0;

    if (buffer == NULL || total_bytes_to_receive == 0) {
        return 0;
    }

    do {
        int ret_code = receive(handle, buffer + total_received_bytes, total_bytes_to_receive - total_received_bytes, &current_received_bytes);
        if (ret_code != 0 || current_received_bytes == 0) {
            return 0; // Error or no bytes received in this call
        }
        total_received_bytes += current_received_bytes;
    } while (total_received_bytes < total_bytes_to_receive);

    return total_bytes_to_receive; // Returns the initially requested amount if successful
}

// Function: rotate_right
void rotate_right(int column_idx) { // Renamed param_1 to column_idx for clarity
    if (column_idx >= 0 && column_idx < 32) { // 0x20 is 32
        // This operation performs a left rotate by 1 bit: (val << 1) | (val >> 31)
        gBoard[column_idx] = (gBoard[column_idx] << 1) | (gBoard[column_idx] >> 31);
    }
}

// Function: rotate_down
void rotate_down(int column_idx) { // Renamed param_1 to column_idx for clarity
    uint first_row_bit_for_column = DAT_0001417c;

    if (column_idx >= 0 && column_idx < 32) {
        // Iterate from the second to last row (31) up to the first row (1)
        for (int row = 31; row > 0; --row) {
            // Update the bit at column_idx in the current row (row)
            // It takes the bit from the row above (row - 1)
            gBoard[row] = (gBoard[row] & neg_mask[column_idx]) | (mask[column_idx] & gBoard[row - 1]);
        }
        // Update the bit at column_idx in the top row (gBoard[0])
        // It takes the bit from 'first_row_bit_for_column' (DAT_0001417c)
        gBoard[0] = (mask[column_idx] & first_row_bit_for_column) | (gBoard[0] & neg_mask[column_idx]);
    }
}

// Function: printBoard
void printBoard(void) {
    char buffer[1088]; // 32 rows * 32 cols + 32 newlines + 1 null terminator = 1057. 1088 is safe.
    int buffer_idx = 0;

    for (int row = 0; row < 32; ++row) {
        for (int col = 0; col < 32; ++col) {
            if ((mask[col] & gBoard[row]) == 0) {
                buffer[buffer_idx++] = '.';
            } else {
                buffer[buffer_idx++] = '1';
            }
        }
        buffer[buffer_idx++] = '\n';
    }
    buffer[buffer_idx] = '\0'; // Null-terminate the string

    // Transmit the board representation (handle 1 typically means stdout)
    transmit_all(1, buffer, buffer_idx);
}

// Function: initBoard
void initBoard(void) {
    // Initialize DAT_0001417c and DAT_00014178 to all bits set
    DAT_0001417c = 0xffffffff;
    DAT_00014178 = 0xffffffff;

    // Initialize the gBoard array (optional, but good practice)
    for (int i = 0; i < 32; ++i) {
        gBoard[i] = 0; // Or some other default value
    }
    // Example: set a specific bit for testing
    gBoard[0] = 1; // Set the LSB of the first row
    gBoard[1] = 2; // Set the second bit of the second row
}

// Function: remove_bits
void remove_bits(void) {
    // If the last two bits of both DAT_0001417c and DAT_00014178 are set (i.e., value 3)
    if ((DAT_0001417c & 0x3) == 0x3 && (DAT_00014178 & 0x3) == 0x3) {
        // Clear the last two bits of both variables
        DAT_0001417c &= ~0x3; // Equivalent to DAT_0001417c & 0xfffffffc
        DAT_00014178 &= ~0x3; // Equivalent to DAT_00014178 & 0xfffffffc
    }
}

// Function: makeMove
void makeMove(uint move_param) { // Renamed param_1 to move_param for clarity
    if ((move_param & 32) == 0) { // If the 6th bit (0x20) is not set
        rotate_down(move_param & 31); // Use lower 5 bits (0x1f) as index for rotate_down
    } else { // If the 6th bit (0x20) is set
        rotate_right(move_param & 31); // Use lower 5 bits (0x1f) as index for rotate_right
    }
    remove_bits();
}

// Function: isWinner
undefined4 isWinner(void) {
    // Iterate through all rows of the board
    for (int row = 0; row < 32; ++row) {
        if (gBoard[row] != 0) {
            return 0; // Found a non-zero row, so not a winner
        }
    }
    return 1; // All rows are zero, so it's a winner
}

// Function: main
int main(void) {
    char input_char;

    initialize_masks(); // Initialize global mask arrays
    initBoard();

    while (1) {
        if (isWinner()) {
            transmit_all(1, "You Win\n", 8); // Send "You Win\n" to stdout
            exit(0); // Exit successfully
        }

        // Receive one byte of input (handle 0 typically means stdin)
        if (receive_all(0, &input_char, 1) != 1) {
            // If receive fails or doesn't receive 1 byte, break loop
            break;
        }

        // Check for a special quit character (e.g., -1 or 0xFF if char is unsigned)
        // Assuming -1 is a sentinel value for quitting, perhaps from an EOF or error state.
        // For interactive input, '\n' or 'q' might be more practical.
        if (input_char == -1 || input_char == 'q') {
            printBoard();
            return 0; // Exit successfully
        }

        makeMove((uint)input_char); // Cast input char to uint for makeMove
    }

    printBoard(); // Print board before exiting if loop breaks due to receive error
    return 0; // Exit successfully
}