#include <stdio.h>
#include <stdlib.h> // For atoi, abs, exit
#include <string.h> // For strlen, memset
#include <stdbool.h> // For bool type

// Global board representation
char board[8][8];
char current_team = 1; // 1 for White, 0 for Black

// Function prototypes for functions defined later
void displayboard(void);
char getPiece(int row, int col);
void swap(int r1, int c1, int r2, int c2);

// Mock external functions
// In a real system, these would be linked from a library or implemented elsewhere.
// For compilation and basic functionality, we'll provide simple versions.
void transmit(int fd, const char *buf, size_t count, int flags) {
    // Assuming it's meant to print a single character from the board
    if (count == 1) {
        putchar(*buf);
    }
}

// Emulate receive to read from stdin
// Returns 0 on success (read something), -1 on error (or if no input, like EOF)
int receive(int fd, char *buf, size_t count, int *read_len) {
    if (fgets(buf, count, stdin) != NULL) {
        *read_len = strlen(buf);
        if (*read_len > 0 && buf[*read_len - 1] == '\n') {
            buf[*read_len - 1] = '\0'; // Remove newline
            *read_len -= 1;
        }
        return 0; // Success
    }
    return -1; // Error or EOF
}

void _terminate(void) {
    exit(0);
}

// Function: getColor
// Determines the color of a piece based on its character.
// Returns 1 for white (g-l), 0 for black (a-f), 0x2e ('.') for empty.
int getColor(char piece) {
    if ((piece >= 'a') && (piece <= 'f')) {
        return 0; // Black pieces
    } else if ((piece >= 'g') && (piece <= 'l')) {
        return 1; // White pieces
    }
    return 0x2e; // Empty square ('.')
}

// Function: displayboard
void displayboard(void) {
    printf("  0 1 2 3 4 5 6 7\n");
    printf(" -----------------\n");
    for (int row = 7; row >= 0; row--) { // Iterate rows from top (7) to bottom (0)
        printf("%d|", row);
        for (int col = 0; col < 8; col++) { // Iterate columns from left (0) to right (7)
            transmit(0, &board[row][col], 1, 0); // Print character
            printf(" ");
        }
        printf("|\n");
    }
    printf(" -----------------\n");
}

// Function: initboard
void initboard(void) {
    // Initialize all squares to empty
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            board[row][col] = '.'; // 0x2e is ASCII for '.'
        }
    }

    // Initialize specific pieces as per original snippet's logic
    // This setup is unusual for chess, but strictly follows the provided indices.
    // Indices are [col + row * 8] effectively, so board[row][col] is accessed as board_array[row*8 + col]
    // The original code sets specific columns across all rows.
    
    // Column 0
    board[0][0] = 'd'; // Black Rook
    board[1][0] = 'b'; // Black Knight
    board[2][0] = 'c'; // Black Bishop
    board[3][0] = 'f'; // Black Queen
    board[4][0] = 'e'; // Black King
    board[5][0] = 'c'; // Black Bishop
    board[6][0] = 'b'; // Black Knight
    board[7][0] = 'd'; // Black Rook

    // Column 1
    board[0][1] = 'a'; // Black Pawn
    board[1][1] = 'a'; // Black Pawn
    board[2][1] = 'a'; // Black Pawn
    board[3][1] = 'a'; // Black Pawn
    board[4][1] = 'a'; // Black Pawn
    board[5][1] = 'a'; // Black Pawn
    board[6][1] = 'a'; // Black Pawn
    board[7][1] = 'a'; // Black Pawn

    // Column 6
    board[0][6] = 'g'; // White Pawn
    board[1][6] = 'g'; // White Pawn
    board[2][6] = 'g'; // White Pawn
    board[3][6] = 'g'; // White Pawn
    board[4][6] = 'g'; // White Pawn
    board[5][6] = 'g'; // White Pawn
    board[6][6] = 'g'; // White Pawn
    board[7][6] = 'g'; // White Pawn

    // Column 7
    board[0][7] = 'j'; // White Rook
    board[1][7] = 'h'; // White Knight
    board[2][7] = 'i'; // White Bishop
    board[3][7] = 'l'; // White Queen
    board[4][7] = 'k'; // White King
    board[5][7] = 'i'; // White Bishop
    board[6][7] = 'h'; // White Knight
    board[7][7] = 'j'; // White Rook
}

// Function: getPiece
char getPiece(int row, int col) {
    // Ensure coordinates are within bounds, return '.' if out of bounds
    if (row < 0 || row >= 8 || col < 0 || col >= 8) {
        return '.';
    }
    return board[row][col];
}

// Function: checkNoCollision
// Checks for collisions between (r1, c1) and (r2, c2)
// Returns 1 if no collision, 0 if collision.
int checkNoCollision(int r1, int c1, int r2, int c2) {
    char piece_at_start = getPiece(r1, c1);
    char piece_at_dest = getPiece(r2, c2);

    // Knights ('b', 'h') jump over pieces, so only check destination for collision
    if (piece_at_start == 'b' || piece_at_start == 'h') {
        return (getColor(piece_at_start) != getColor(piece_at_dest));
    }

    // For all other pieces, if destination has a piece of the same color, it's a collision
    if (getColor(piece_at_start) == getColor(piece_at_dest)) {
        return 0;
    }

    // Array to store intermediate path coordinates (max path length 6 for 8x8 board)
    int path_coords[6][2];
    int path_length = 0;

    int dr = r2 - r1; // Delta row
    int dc = c2 - c1; // Delta col

    if (dr == 0) { // Horizontal move
        int step_c = (dc > 0) ? 1 : -1;
        for (int c = c1 + step_c; c != c2; c += step_c) {
            path_coords[path_length][0] = r1;
            path_coords[path_length][1] = c;
            path_length++;
        }
    } else if (dc == 0) { // Vertical move
        int step_r = (dr > 0) ? 1 : -1;
        for (int r = r1 + step_r; r != r2; r += step_r) {
            path_coords[path_length][0] = r;
            path_coords[path_length][1] = c1;
            path_length++;
        }
    } else if (abs(dr) == abs(dc)) { // Diagonal move
        int step_r = (dr > 0) ? 1 : -1;
        int step_c = (dc > 0) ? 1 : -1;
        int r = r1 + step_r;
        int c = c1 + step_c;
        while (r != r2) { // Loop until one step before destination
            path_coords[path_length][0] = r;
            path_coords[path_length][1] = c;
            path_length++;
            r += step_r;
            c += step_c;
        }
    }

    // Check intermediate squares for blockage
    for (int i = 0; i < path_length; i++) {
        if (getPiece(path_coords[i][0], path_coords[i][1]) != '.') {
            return 0; // Path is blocked
        }
    }

    return 1; // No collision or blockage
}

// Function: performMove
// Attempts to perform a move from (r1, c1) to (r2, c2).
// Returns 1 if move is successful, 0 otherwise.
int performMove(int r1, int c1, int r2, int c2) {
    // Boundary check for destination coordinates
    if (r2 < 0 || r2 >= 8 || c2 < 0 || c2 >= 8) {
        return 0;
    }

    char piece_at_start = getPiece(r1, c1);

    // Check if the piece at the start position is empty
    if (piece_at_start == '.') {
        return 0;
    }

    // Check if the current player is moving their own piece
    if (current_team == 1) { // White's turn
        if (getColor(piece_at_start) == 0) { // Moving a black piece
            return 0;
        }
    } else { // Black's turn
        if (getColor(piece_at_start) == 1) { // Moving a white piece
            return 0;
        }
    }

    int dr_abs = abs(r2 - r1);
    int dc_abs = abs(c2 - c1);

    bool move_valid = false;

    // Piece-specific move validation
    if (piece_at_start == 'b' || piece_at_start == 'h') { // Knight
        if ((dr_abs == 2 && dc_abs == 1) || (dr_abs == 1 && dc_abs == 2)) {
            move_valid = true;
        }
    } else if (piece_at_start == 'a') { // Black Pawn (moves 1 col right)
        if (r1 == r2 && c2 == c1 + 1) {
            move_valid = true;
        }
    } else if (piece_at_start == 'g') { // White Pawn (moves 1 col left)
        if (r1 == r2 && c2 == c1 - 1) {
            move_valid = true;
        }
    } else if (piece_at_start == 'c' || piece_at_start == 'i') { // Bishop (diagonal)
        if (dr_abs == dc_abs) {
            move_valid = true;
        }
    } else if (piece_at_start == 'f' || piece_at_start == 'l') { // Queen (straight or diagonal)
        if (dr_abs == dc_abs || r1 == r2 || c1 == c2) {
            move_valid = true;
        }
    } else if (piece_at_start == 'e' || piece_at_start == 'k') { // King (one square any direction)
        if (dr_abs < 2 && dc_abs < 2) {
            move_valid = true;
        }
    } else if (piece_at_start == 'j' || piece_at_start == 'd') { // Rook (straight)
        if (r1 == r2 || c1 == c2) {
            move_valid = true;
        }
    }

    if (!move_valid) {
        return 0; // Invalid move for this piece type
    }

    // Check for collisions (path blockage and friendly fire)
    if (checkNoCollision(r1, c1, r2, c2)) {
        swap(r1, c1, r2, c2);
        return 1;
    }

    return 0; // Collision detected or invalid move
}

// Function: swap
// Swaps the pieces at (r1, c1) and (r2, c2).
void swap(int r1, int c1, int r2, int c2) {
    char temp_piece = board[r2][c2];
    board[r2][c2] = board[r1][c1];
    board[r1][c1] = temp_piece;
}

// Function: verifyFormat
// Verifies the input string format "D,D D,D"
// param_1: input string (e.g., "1,2 3,4")
// param_2: length of the input string
// Returns 1 if format is valid, 0 otherwise.
int verifyFormat(char *input_str, int len) {
    // The original code null-terminates the string at the given length.
    // For fgets, this is usually handled, but for safety:
    if (len < 0 || len >= 15) { // Max 7 chars + null for D,D D,D, or 1 for '9', 3 for '666'
        return 0;
    }
    input_str[len] = '\0';

    // Check for "D,D D,D" format (length 7)
    if (len == 7 &&
        (input_str[0] >= '0' && input_str[0] <= '9') &&
        (input_str[1] == ',') &&
        (input_str[2] >= '0' && input_str[2] <= '9') &&
        (input_str[3] == ' ') &&
        (input_str[4] >= '0' && input_str[4] <= '9') &&
        (input_str[5] == ',') &&
        (input_str[6] >= '0' && input_str[6] <= '9')) {
        return 1;
    }
    return 0;
}

// Function: parseUserInput
// Reads and parses user input for moves or commands.
// param_1: An array of 4 ints to store start_row, start_col, end_row, end_col.
// Returns: 1 for valid move, 2 for '9' (display board), 3 for '666' (quit), 0 for invalid input, -1 for read error.
int parseUserInput(int *move_coords) {
    char input_buffer[16]; // Buffer for input (max "D,D D,D\n\0" is 9 chars)
    int input_len = 0;

    // Read input from stdin
    if (receive(0, input_buffer, sizeof(input_buffer) - 1, &input_len) == -1) {
        return -1; // Read error
    }

    // Handle special commands
    if (input_len == 1 && input_buffer[0] == '9') {
        return 2; // Display board
    }
    if (input_len == 3 && strcmp(input_buffer, "666") == 0) {
        return 3; // Quit game
    }

    // Verify format for a move
    if (input_len != 7 || !verifyFormat(input_buffer, input_len)) {
        printf("Incorrect input format.\n");
        return 0; // Invalid input
    }

    // Parse coordinates
    move_coords[0] = input_buffer[0] - '0'; // start_row
    move_coords[1] = input_buffer[2] - '0'; // start_col
    move_coords[2] = input_buffer[4] - '0'; // end_row
    move_coords[3] = input_buffer[6] - '0'; // end_col

    // Basic validation for coordinates
    if (move_coords[0] < 0 || move_coords[0] >= 8 ||
        move_coords[1] < 0 || move_coords[1] >= 8 ||
        move_coords[2] < 0 || move_coords[2] >= 8 ||
        move_coords[3] < 0 || move_coords[3] >= 8) {
        printf("Coordinates out of board range (0-7).\n");
        return 0;
    }
    
    return 1; // Valid move input
}

// Function: main
int main(void) {
    int move_info[4]; // Stores start_row, start_col, end_row, end_col
    int parse_result;

    initboard();
    displayboard();

    while (true) {
        while (true) {
            if (current_team == 1) { // White
                printf("WHITE: ");
            } else { // Black
                printf("BLACK: ");
            }

            parse_result = parseUserInput(move_info);
            if (parse_result != 0) { // If input is valid or a special command
                break;
            }
            // If parse_result is 0, it means incorrect format, so loop again.
            // parseUserInput already prints "Incorrect input format."
        }

        if (parse_result == 2) { // '9' command
            displayboard();
        } else if (parse_result == 3) { // '666' command
            printf("good game\n");
            _terminate();
        } else if (parse_result == -1) { // Error reading input
            printf("Input error, terminating.\n");
            _terminate();
        } else { // parse_result == 1, valid move input
            if (performMove(move_info[0], move_info[1], move_info[2], move_info[3]) == 0) {
                printf("Not a legal move\n");
            } else {
                printf("move done\n");
                // Switch team
                current_team = (current_team == 1) ? 0 : 1;
            }
        }
    }
    return 0; // Should not be reached due to _terminate()
}