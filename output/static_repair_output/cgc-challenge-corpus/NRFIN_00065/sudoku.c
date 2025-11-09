#include <stdio.h>   // For fread, printf, stdin
#include <string.h>  // For strchr
#include <stdlib.h>  // For size_t (though often included by stdio.h)

// Renaming types for clarity and standard C compliance
typedef char cell_t;     // Represents a single cell value ('1'-'9', ' ', '\n', '\0')
typedef int status_t;    // For function return values (0 for success, -1 for failure/error)

// Define board dimensions and layout
#define BOARD_ROWS 9
#define BOARD_COLS 9
#define ROW_LENGTH (BOARD_COLS + 1) // 9 digits + 1 newline character
#define TOTAL_BOARD_SIZE (BOARD_ROWS * ROW_LENGTH) // 9 * 10 = 90 characters

// Function: read_board
// Reads the Sudoku board from standard input.
// Expects 9 rows, each with 9 digits/spaces followed by a newline.
// Returns 0 on success, -1 on failure (incorrect size or invalid characters).
status_t read_board(cell_t *board_ptr) {
    size_t bytes_read = fread(board_ptr, 1, TOTAL_BOARD_SIZE, stdin);

    if (bytes_read == TOTAL_BOARD_SIZE) {
        for (int i = 0; i < TOTAL_BOARD_SIZE; ++i) {
            // Check for newline characters at the end of each logical row
            if ((i + 1) % ROW_LENGTH == 0) {
                if (board_ptr[i] != '\n') {
                    return -1; // Invalid board: missing newline
                }
            } else {
                // Check if the character is a valid digit ('1'-'9') or a space (' ')
                if (strchr("123456789 ", board_ptr[i]) == NULL) {
                    return -1; // Invalid character found
                }
            }
        }
        return 0; // Success
    }
    return -1; // Failed to read the expected number of bytes
}

// Function: get_square
// Retrieves the value of a specific cell on the board.
// Returns the character value of the cell, or '\0' for out-of-bounds access.
cell_t get_square(cell_t *board_ptr, int col, int row) {
    if (col < 0 || col >= BOARD_COLS || row < 0 || row >= BOARD_ROWS) {
        return '\0'; // Return null character for out-of-bounds access
    }
    // Board is stored as a 1D array: board_ptr[column + row * ROW_LENGTH]
    return board_ptr[col + row * ROW_LENGTH];
}

// Function: set_square
// Sets the value of a specific cell on the board.
// Only sets the value if coordinates are within bounds and the value is a digit '1'-'9'.
void set_square(cell_t *board_ptr, cell_t value, int col, int row) {
    // Check bounds for column and row
    if (col >= 0 && col < BOARD_COLS && row >= 0 && row < BOARD_ROWS) {
        // Check if the value is a valid digit ('1' through '9')
        if (value >= '1' && value <= '9') {
            board_ptr[col + row * ROW_LENGTH] = value;
        }
    }
    // No action for invalid inputs, matching original void return logic
}

// Function: get_super_square
// Populates a 3x3 array (sub_square_ptr) with values from a 3x3 block of the main board.
void get_super_square(cell_t *board_ptr, cell_t *sub_square_ptr, int start_row, int start_col) {
    for (int sub_row_idx = 0; sub_row_idx < 3; ++sub_row_idx) {
        for (int sub_col_idx = 0; sub_col_idx < 3; ++sub_col_idx) {
            // Calculate the absolute row and column on the main board
            int current_row = start_row + sub_row_idx;
            int current_col = start_col + sub_col_idx;
            // Get the value from the main board and store it in the sub_square array
            sub_square_ptr[sub_col_idx + sub_row_idx * 3] = get_square(board_ptr, current_col, current_row);
        }
    }
}

// Function: count_duplicates
// Counts duplicate digits ('1'-'9') in a 9-element array (representing a row, column, or 3x3 block).
// Returns the total count of duplicated digits. For example, if '5' appears 3 times, it adds 3 to the total.
int count_duplicates(cell_t *array_ptr) {
    int total_duplicates = 0;
    // Iterate through digits '1' to '9'
    for (char digit = '1'; digit <= '9'; ++digit) {
        int current_digit_count = 0;
        // Count occurrences of the current digit in the array
        for (int i = 0; i < 9; ++i) {
            if (digit == array_ptr[i]) {
                current_digit_count++;
            }
        }
        // If a digit appears more than once, add its count to total_duplicates
        if (current_digit_count > 1) {
            total_duplicates += current_digit_count;
        }
    }
    return total_duplicates;
}

// Function: validate_board
// Validates a Sudoku board according to Sudoku rules.
// Checks for duplicates in all rows, columns, and 3x3 sub-squares.
// Returns 0 for a valid board, or the negative of the count of duplicates found
// (first instance of duplicates encountered).
int validate_board(cell_t *board_ptr) {
    cell_t temp_line[9]; // Temporary array to hold a row, column, or 3x3 block

    // 1. Validate Rows
    for (int row = 0; row < BOARD_ROWS; ++row) {
        for (int col = 0; col < BOARD_COLS; ++col) {
            temp_line[col] = get_square(board_ptr, col, row);
        }
        int duplicates = count_duplicates(temp_line);
        if (duplicates != 0) {
            return -duplicates;
        }
    }

    // 2. Validate Columns
    for (int col = 0; col < BOARD_COLS; ++col) {
        for (int row = 0; row < BOARD_ROWS; ++row) {
            temp_line[row] = get_square(board_ptr, col, row);
        }
        int duplicates = count_duplicates(temp_line);
        if (duplicates != 0) {
            return -duplicates;
        }
    }

    // 3. Validate 3x3 Sub-squares
    for (int start_row = 0; start_row < BOARD_ROWS; start_row += 3) {
        for (int start_col = 0; start_col < BOARD_COLS; start_col += 3) {
            get_super_square(board_ptr, temp_line, start_row, start_col);
            int duplicates = count_duplicates(temp_line);
            if (duplicates != 0) {
                return -duplicates;
            }
        }
    }

    return 0; // Board is valid
}

// Function: solve_board
// Solves the Sudoku board using a backtracking algorithm.
// 'current_pos' points to the starting position for searching for empty squares.
// Returns 0 on success (board solved), -1 on failure (no solution found).
status_t solve_board(cell_t *board_ptr, cell_t *current_pos) {
    // Find the next empty square (represented by ' ') starting from current_pos
    cell_t *empty_square_ptr = strchr(current_pos, ' ');

    // If no empty squares are found, the board is completely filled.
    // Check if this filled board is a valid solution.
    if (empty_square_ptr == NULL) {
        return (validate_board(board_ptr) == 0) ? 0 : -1;
    }

    // Try placing digits '1' through '9' in the empty square
    for (char digit_to_try = '1'; digit_to_try <= '9'; ++digit_to_try) {
        *empty_square_ptr = digit_to_try; // Place the digit

        // Check if the board is valid after placing the digit.
        // This validation checks the entire board, matching the original inefficient logic.
        if (validate_board(board_ptr) == 0) {
            // If the board is still valid, recursively try to solve the rest of the board
            if (solve_board(board_ptr, empty_square_ptr + 1) == 0) {
                return 0; // Solution found
            }
        }
    }

    // If no digit leads to a solution from this empty square, backtrack:
    // Reset the square to ' ' to explore other possibilities.
    *empty_square_ptr = ' ';
    return -1; // No solution found from this path
}

// Function: do_sudoku
// Main logic for the Sudoku solver application.
// Prompts the user for a board, attempts to solve it, and prints the result.
status_t do_sudoku(void) {
    // Allocate space for the board, including a null terminator for printf %s
    cell_t board[TOTAL_BOARD_SIZE + 1];
    board[TOTAL_BOARD_SIZE] = '\0'; // Ensure null termination

    printf("Let me help you cheat at sudoku, enter your board n00b:\n");

    if (read_board(board) == 0) { // Successfully read the board
        // The original code performs an initial validation here.
        // Although its result was assigned to an unused global,
        // we keep the call as a pre-check before solving.
        validate_board(board);

        if (solve_board(board, board) == 0) { // Attempt to solve the board
            printf("Solved!\n%s\n", board);
        } else {
            printf("Nice try\n"); // No solution found
        }
    } else {
        printf("Invalid board\n"); // Failed to read the board
    }
    return 0;
}

// Main function
int main() {
    return do_sudoku();
}