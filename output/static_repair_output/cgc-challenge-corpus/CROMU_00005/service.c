#include <stdio.h>  // For printf, stdin, fgets
#include <stdlib.h> // For exit, abs, atoi
#include <string.h> // For memset, strlen

// Global variables
char ptr_array[64]; // 8x8 board, indexed as ptr_array[row + col * 8]
char current_team = 1; // 1 for White, 0 for Black (as per original logic with '\x01' and '\0')

// Function declarations (to avoid implicit declaration warnings)
int getColor(char piece);
void transmit(int fd, const void *buf, size_t count, int flags);
void displayboard(void);
void initboard(void);
char getPiece(int col, int row);
int checkNoCollision(int src_col, int src_row, int dest_col, int dest_row);
void swap(int src_col, int src_row, int dest_col, int dest_row);
int verifyFormat(char *input_buffer, int len);
int receive(int fd, char *buf, size_t max_len, int *actual_len_ptr);
int parseUserInput(int *coords);

// Function: getColor
int getColor(char piece) {
    if ((piece >= 'a') && (piece <= 'f')) { // White pieces ('a' to 'f')
        return 1;
    } else if ((piece >= 'g') && (piece <= 'l')) { // Black pieces ('g' to 'l')
        return 0;
    } else {
        return 0x2e; // '.' (empty spot)
    }
}

// Dummy transmit function to simulate printing a character.
// The original code uses transmit(0, ptr, 1, 0) to print a char.
void transmit(int fd, const void *buf, size_t count, int flags) {
    if (count > 0 && buf != NULL) {
        printf("%c", *(char*)buf);
    }
}

// Function: displayboard
void displayboard(void) {
    for (int row = 7; row >= 0; --row) {
        for (int col = 0; col < 8; ++col) {
            transmit(0, &ptr_array[row + col * 8], 1, 0);
        }
        printf("\n");
    }
}

// Function: initboard
void initboard(void) {
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            ptr_array[row + col * 8] = '.'; // 0x2e
        }
    }
    // Row 0 (White pieces)
    ptr_array[0 + 0 * 8] = 'd'; // King?
    ptr_array[0 + 1 * 8] = 'b'; // Knight?
    ptr_array[0 + 2 * 8] = 'c'; // Bishop?
    ptr_array[0 + 3 * 8] = 'f'; // Queen?
    ptr_array[0 + 4 * 8] = 'e'; // King?
    ptr_array[0 + 5 * 8] = 'c'; // Bishop?
    ptr_array[0 + 6 * 8] = 'b'; // Knight?
    ptr_array[0 + 7 * 8] = 'd'; // Rook?
    // Row 1 (White pawns)
    for (int col = 0; col < 8; ++col) {
        ptr_array[1 + col * 8] = 'a';
    }
    // Row 6 (Black pawns)
    for (int col = 0; col < 8; ++col) {
        ptr_array[6 + col * 8] = 'g';
    }
    // Row 7 (Black pieces)
    ptr_array[7 + 0 * 8] = 'j'; // Rook?
    ptr_array[7 + 1 * 8] = 'h'; // Knight?
    ptr_array[7 + 2 * 8] = 'i'; // Bishop?
    ptr_array[7 + 3 * 8] = 'l'; // Queen?
    ptr_array[7 + 4 * 8] = 'k'; // King?
    ptr_array[7 + 5 * 8] = 'i'; // Bishop?
    ptr_array[7 + 6 * 8] = 'h'; // Knight?
    ptr_array[7 + 7 * 8] = 'j'; // Rook?
}

// Function: getPiece
char getPiece(int col, int row) {
    return ptr_array[row + col * 8];
}

// Function: checkNoCollision
int checkNoCollision(int src_col, int src_row, int dest_col, int dest_row) {
    char src_piece = getPiece(src_col, src_row);
    char dest_piece = getPiece(dest_col, dest_row);

    // Knights ('b' and 'h') can jump over pieces. Only check destination color.
    if (src_piece == 'b' || src_piece == 'h') {
        return (getColor(src_piece) != getColor(dest_piece)); // 1 if different colors, 0 if same
    }

    // For other pieces, check if source and destination are the same color.
    // If they are, it's an invalid capture/move.
    if (getColor(src_piece) == getColor(dest_piece)) {
        return 0;
    }

    // Array to store (col, row) pairs of path squares. Max 7 intermediate squares + 1 dest = 8 pairs.
    int path_coords[16]; // Stores col, row, col, row...
    int path_len = 0;    // Number of (col, row) pairs added to path_coords

    if (src_col == dest_col) { // Vertical move
        int row_step = (dest_row > src_row) ? 1 : -1;
        for (int r = src_row + row_step; (row_step > 0) ? (r <= dest_row) : (r >= dest_row); r += row_step) {
            path_coords[path_len * 2] = src_col;
            path_coords[path_len * 2 + 1] = r;
            path_len++;
        }
    } else if (src_row == dest_row) { // Horizontal move
        int col_step = (dest_col > src_col) ? 1 : -1;
        for (int c = src_col + col_step; (col_step > 0) ? (c <= dest_col) : (c >= dest_col); c += col_step) {
            path_coords[path_len * 2] = c;
            path_coords[path_len * 2 + 1] = src_row;
            path_len++;
        }
    } else { // Diagonal move
        int col_diff = abs(dest_col - src_col);
        int row_diff = abs(dest_row - src_row);

        if (col_diff == row_diff) { // Valid diagonal
            int col_step = (dest_col > src_col) ? 1 : -1;
            int row_step = (dest_row > src_row) ? 1 : -1;

            for (int i = 1; i <= col_diff; ++i) { // Iterate up to and including destination
                path_coords[path_len * 2] = src_col + i * col_step;
                path_coords[path_len * 2 + 1] = src_row + i * row_step;
                path_len++;
            }
        }
        // If not straight or diagonal, path_len remains 0, which means no intermediate collision check.
        // This relies on performMove to validate the move type.
    }

    // Check for intermediate collisions (excluding the destination square).
    // The destination square is the last element in path_coords.
    for (int i = 0; i < path_len - 1; ++i) { // Iterate up to the square *before* the destination
        char piece_on_path = getPiece(path_coords[i * 2], path_coords[i * 2 + 1]);
        if (piece_on_path != '.') {
            return 0; // Collision detected on an intermediate square
        }
    }

    return 1; // No intermediate collisions
}

// Function: performMove
int performMove(int src_col, int src_row, int dest_col, int dest_row) {
    char piece = getPiece(src_col, src_row);

    // 1. Check if the piece belongs to the current team
    int piece_color = getColor(piece);
    if (current_team == 1) { // White's turn
        if (piece_color == 0) return 0; // Trying to move a black piece
    } else { // Black's turn
        if (piece_color == 1) return 0; // Trying to move a white piece
    }

    // 2. Check if destination is within board bounds and source is not empty
    if (dest_col < 0 || dest_col >= 8 || dest_row < 0 || dest_row >= 8 || piece == '.') {
        return 0;
    }

    // 3. Check specific piece move rules
    int valid_move_shape = 0; // Flag to indicate if the move is geometrically valid

    switch (piece) {
        case 'a': // White Pawn
            if (src_col == dest_col && dest_row == src_row + 1) { // Forward one step
                valid_move_shape = 1;
            }
            break;
        case 'g': // Black Pawn
            if (src_col == dest_col && dest_row == src_row - 1) { // Forward one step
                valid_move_shape = 1;
            }
            break;
        case 'b': // White Knight
        case 'h': // Black Knight
            // Knight L-shape moves: (2,1) or (1,2) in any direction
            int col_diff_knight = abs(dest_col - src_col);
            int row_diff_knight = abs(dest_row - src_row);
            if ((col_diff_knight == 2 && row_diff_knight == 1) ||
                (col_diff_knight == 1 && row_diff_knight == 2)) {
                valid_move_shape = 1;
            }
            break;
        case 'c': // White Bishop
        case 'i': // Black Bishop
            // Bishop moves diagonally
            if (abs(dest_col - src_col) == abs(dest_row - src_row)) {
                valid_move_shape = 1;
            }
            break;
        case 'd': // White Rook
        case 'j': // Black Rook
            // Rook moves horizontally or vertically
            if (src_col == dest_col || src_row == dest_row) {
                valid_move_shape = 1;
            }
            break;
        case 'e': // White King
        case 'k': // Black King
            // King moves one square in any direction
            if (abs(dest_col - src_col) < 2 && abs(dest_row - src_row) < 2) {
                valid_move_shape = 1;
            }
            break;
        case 'f': // White Queen
        case 'l': // Black Queen
            // Queen moves horizontally, vertically, or diagonally
            if (src_col == dest_col || src_row == dest_row ||
                abs(dest_col - src_col) == abs(dest_row - src_row)) {
                valid_move_shape = 1;
            }
            break;
        default:
            // Unknown piece or invalid piece character
            return 0;
    }

    if (!valid_move_shape) {
        return 0;
    }

    // 4. Check for collisions on the path (handled by checkNoCollision)
    if (!checkNoCollision(src_col, src_row, dest_col, dest_row)) {
        return 0;
    }

    // 5. If all checks pass, perform the swap (as per original snippet logic)
    swap(src_col, src_row, dest_col, dest_row);
    return 1;
}

// Function: swap
void swap(int src_col, int src_row, int dest_col, int dest_row) {
    char temp_piece = ptr_array[dest_row + dest_col * 8]; // Save dest piece
    ptr_array[dest_row + dest_col * 8] = ptr_array[src_row + src_col * 8]; // Move source to dest
    ptr_array[src_row + src_col * 8] = temp_piece; // Move saved dest piece to source
}

// Function: verifyFormat
int verifyFormat(char *input_buffer, int len) {
    // Expected format: "X,Y X,Y" where X,Y are single digits '0'-'9'
    // Total length 7 (digit,comma,digit,space,digit,comma,digit)
    if (len != 7) {
        return 0;
    }

    // Ensure null termination for safety with atoi, although specific char checks precede.
    input_buffer[len] = '\0';

    // Check characters at specific positions
    if (!('0' <= input_buffer[0] && input_buffer[0] <= '9')) return 0;
    if (input_buffer[1] != ',') return 0;
    if (!('0' <= input_buffer[2] && input_buffer[2] <= '9')) return 0;
    if (input_buffer[3] != ' ') return 0;
    if (!('0' <= input_buffer[4] && input_buffer[4] <= '9')) return 0;
    if (input_buffer[5] != ',') return 0;
    if (!('0' <= input_buffer[6] && input_buffer[6] <= '9')) return 0;

    return 1;
}

// Dummy receive function to simulate reading a line from stdin.
// It reads a line from stdin into buf, up to max_len.
// It stores the actual length read (excluding newline, if present) into *actual_len_ptr.
// Returns 0 on success, -1 on error.
int receive(int fd, char *buf, size_t max_len, int *actual_len_ptr) {
    // max_len is the buffer capacity *after* buf, so fgets should use max_len + 1 for null terminator.
    if (fgets(buf, max_len + 1, stdin) != NULL) {
        size_t len = strlen(buf);
        if (len > 0 && buf[len - 1] == '\n') {
            buf[len - 1] = '\0'; // Remove newline
            *actual_len_ptr = len - 1;
        } else {
            *actual_len_ptr = len;
        }
        return 0; // Success
    }
    return -1; // Error or EOF
}

// Function: parseUserInput
int parseUserInput(int *coords) {
    char input_buffer[16]; // Buffer to hold user input
    int bytes_read_len = 0;
    int receive_status;

    memset(input_buffer, 0, sizeof(input_buffer)); // Clear the buffer

    // Read input into input_buffer[1] onwards, leaving input_buffer[0] unused.
    // This matches the original decompiler output's indexing (acStack_2c + 1).
    receive_status = receive(0, input_buffer + 1, 0xe, &bytes_read_len);

    if (receive_status == -1) { // If receive failed (e.g., EOF)
        return -1;
    }

    // Handle special commands based on input length and content
    if (bytes_read_len == 1) {
        if (input_buffer[1] == '9') { // '9' command to display board
            return 2;
        }
    } else if (bytes_read_len == 3) {
        if (input_buffer[1] == '6' && input_buffer[2] == '6' && input_buffer[3] == '6') { // '666' command to quit
            return 3;
        }
    } else if (bytes_read_len != 7) { // Expected move format length "X,Y X,Y"
        printf("incorrect input\n");
        return 0;
    }

    // Verify move format "X,Y X,Y"
    // Pass input_buffer + 1 as the start of the string to verifyFormat
    if (!verifyFormat(input_buffer + 1, bytes_read_len)) {
        printf("incorrect input\n");
        return 0;
    }

    // Parse coordinates (src_col, src_row, dest_col, dest_row)
    // input_buffer + 1 points to the first digit
    // input_buffer + 3 points to the second digit
    // input_buffer + 5 points to the third digit
    // input_buffer + 7 points to the fourth digit
    coords[0] = atoi(input_buffer + 1); // src_col
    coords[1] = atoi(input_buffer + 3); // src_row
    coords[2] = atoi(input_buffer + 5); // dest_col
    coords[3] = atoi(input_buffer + 7); // dest_row

    return 1; // Successfully parsed move
}

// Function: main
int main(void) {
    int move_coords[4]; // Array to store parsed coordinates (src_col, src_row, dest_col, dest_row)
    void (*display_board_func_ptr)(void) = displayboard; // Function pointer to displayboard
    int parse_result; // Result from parseUserInput

    initboard();
    display_board_func_ptr(); // Initial board display

    while (1) {
        if (current_team == 1) { // White's turn
            printf("WHITE: ");
        } else { // Black's turn
            printf("BLACK: ");
        }

        parse_result = parseUserInput(move_coords);

        if (parse_result == 0) { // Incorrect format or invalid input
            printf("Not a legal move format\n");
            continue; // Go back to the start of the loop for new input
        }

        if (parse_result == 2) { // '9' command to display board
            display_board_func_ptr();
            continue; // Go back to the start of the loop for new input
        }

        if (parse_result == 3) { // '666' command to quit
            printf("good game\n");
            exit(0); // Terminate program
        }

        if (parse_result == -1) { // Error from receive (e.g., EOF)
            exit(0); // Terminate program
        }

        // Attempt to perform the move
        int move_successful = performMove(move_coords[0], move_coords[1], move_coords[2], move_coords[3]);

        if (move_successful == 0) {
            printf("Not a legal move\n");
        } else {
            printf("Move successful\n");
            // Switch turns
            if (current_team == 1) {
                current_team = 0;
            } else {
                current_team = 1;
            }
        }
    }

    return 0; // Should not be reached in this infinite loop
}