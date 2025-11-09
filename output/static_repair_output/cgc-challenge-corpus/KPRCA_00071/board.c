#include <stdio.h>   // For printf, scanf
#include <stdlib.h>  // For malloc, free, exit
#include <string.h>  // For memset

// Define the board structure
// The original snippet implies a size of 0x14 (20 bytes) for the board.
// A 3x3 char array (9 bytes) followed by two 4-byte integers (8 bytes)
// would be 17 bytes. Compilers often add padding for alignment.
// If int requires 4-byte alignment, 3 bytes of padding would be added after cells[9],
// making the total size 9 + 3 + 4 + 4 = 20 bytes.
typedef struct {
    char cells[9];     // 3x3 Tic-Tac-Toe board (indexed 0-8)
    // char padding[3]; // Implicit padding by compiler for alignment
    int move_count;    // Number of moves made so far (offset 0xc from start)
    int game_state;    // Current state of the game (offset 0x10 from start)
} Board;

// Player characters (as derived from the original snippet's values)
#define PLAYER_X_CHAR ((char)-0x40) // Original: -0x40 (e.g., 'X')
#define PLAYER_O_CHAR '@'           // Original: @ (e.g., 'O')

// Game states (as derived from the original snippet's return values)
#define GAME_IN_PROGRESS 0
#define GAME_X_WINS      8
#define GAME_O_WINS      0x10 // 16 decimal
#define GAME_FINAL_DRAW  0x20 // 32 decimal (when move_count == 9 and no winner)

// Function prototypes
Board* create_board(void);
void clear_board(Board* board);
int check_winning_board(Board* board);
int do_move(Board* board, int row, int col, char player_char);
int undo_move(Board* board, int row, int col);
void print_board(const Board* board);

// Function: create_board
// Allocates memory for a new board and initializes it.
Board* create_board(void) {
    Board* board = (Board*)malloc(sizeof(Board));
    if (board == NULL) {
        perror("Failed to allocate board memory");
        exit(EXIT_FAILURE); // Exit if memory allocation fails
    }
    clear_board(board); // Initialize the newly created board
    return board;
}

// Function: clear_board
// Resets the board cells, move count, and game state.
void clear_board(Board* board) {
    memset(board->cells, 0, sizeof(board->cells)); // Set all 9 cells to '\0'
    board->move_count = 0;                         // Reset move count
    board->game_state = GAME_IN_PROGRESS;          // Reset game state
}

// Function: check_winning_board
// Determines the current game status (win, draw, or in progress).
// Updates board->game_state and returns it.
int check_winning_board(Board* board) {
    char* cells = board->cells;

    // Check rows for a win
    for (int i = 0; i < 3; ++i) {
        if (cells[i * 3] != '\0' && cells[i * 3] == cells[i * 3 + 1] && cells[i * 3] == cells[i * 3 + 2]) {
            board->game_state = (cells[i * 3] == PLAYER_X_CHAR) ? GAME_X_WINS : GAME_O_WINS;
            return board->game_state;
        }
    }

    // Check columns for a win
    for (int i = 0; i < 3; ++i) {
        if (cells[i] != '\0' && cells[i] == cells[i + 3] && cells[i] == cells[i + 6]) {
            board->game_state = (cells[i] == PLAYER_X_CHAR) ? GAME_X_WINS : GAME_O_WINS;
            return board->game_state;
        }
    }

    // Check diagonals for a win
    if (cells[0] != '\0' && cells[0] == cells[4] && cells[0] == cells[8]) {
        board->game_state = (cells[0] == PLAYER_X_CHAR) ? GAME_X_WINS : GAME_O_WINS;
        return board->game_state;
    }
    if (cells[2] != '\0' && cells[2] == cells[4] && cells[2] == cells[6]) {
        board->game_state = (cells[2] == PLAYER_X_CHAR) ? GAME_X_WINS : GAME_O_WINS;
        return board->game_state;
    }

    // If no winner, check for a draw (board is full)
    if (board->move_count == 9) {
        board->game_state = GAME_FINAL_DRAW;
        return GAME_FINAL_DRAW;
    }

    // If no winner and not a draw, the game is still in progress
    board->game_state = GAME_IN_PROGRESS;
    return GAME_IN_PROGRESS;
}

// Function: do_move
// Attempts to place a player's character at the specified row and column.
// Returns:
//   - 4: If the board is already full before the move attempt.
//   - 0: If the move is invalid (out of bounds or cell already occupied).
//   - Otherwise: The updated game state (GAME_X_WINS, GAME_O_WINS, GAME_FINAL_DRAW, or GAME_IN_PROGRESS).
int do_move(Board* board, int row, int col, char player_char) {
    if (board->move_count >= 9) {
        return 4; // Board is full, as per original snippet's return value
    }

    // Check if coordinates are within the 0-2 range for a 3x3 board
    if (row < 0 || row > 2 || col < 0 || col > 2) {
        return 0; // Invalid coordinates, as per original snippet's return value
    }

    int index = row * 3 + col; // Calculate 1D array index from 2D coordinates
    if (board->cells[index] == '\0') {
        board->cells[index] = player_char; // Place the player's character
        board->move_count++;               // Increment move count
        return check_winning_board(board); // Check and return the new game state
    } else {
        return 0; // Cell already occupied, as per original snippet's return value
    }
}

// Function: undo_move
// Attempts to undo a move by clearing a cell and decrementing the move count.
// Returns:
//   - 0: If the undo is invalid (out of bounds or cell already empty).
//   - 1: If the undo is successful.
int undo_move(Board* board, int row, int col) {
    // Check if coordinates are within the 0-2 range for a 3x3 board
    // Original snippet had `3 < param_3` and `3 < param_2` which implies 0-3 range,
    // but for a 3x3 board, 0-2 is correct. Assuming 0-2.
    if (row < 0 || row > 2 || col < 0 || col > 2) {
        return 0; // Invalid coordinates, as per original snippet's return value
    }

    int index = row * 3 + col;
    if (board->cells[index] == '\0') {
        return 0; // Cell is already empty, nothing to undo, as per original snippet's return value
    } else {
        board->cells[index] = '\0'; // Clear the cell
        board->move_count--;        // Decrement move count
        board->game_state = GAME_IN_PROGRESS; // Reset game state as a move was undone
        return 1; // Successful undo, as per original snippet's return value
    }
}

// Function: print_board
// Prints the current state of the Tic-Tac-Toe board to the console.
void print_board(const Board* board) {
    // String literals for displaying cell content
    const char* PLAYER_O_DISPLAY = "O";
    const char* PLAYER_X_DISPLAY = "X";
    const char* EMPTY_CELL_DISPLAY = " ";
    const char* UNKNOWN_CELL_DISPLAY = "?"; // For unexpected characters

    for (int row = 0; row < 3; ++row) {
        printf("|");
        for (int col = 0; col < 3; ++col) {
            char cell_char = board->cells[row * 3 + col];
            const char* display_str;

            // Determine which string to display based on the cell's character
            if (cell_char == PLAYER_O_CHAR) {
                display_str = PLAYER_O_DISPLAY;
            } else if (cell_char == PLAYER_X_CHAR) {
                display_str = PLAYER_X_DISPLAY;
            } else if (cell_char == '\0') {
                display_str = EMPTY_CELL_DISPLAY;
            } else {
                display_str = UNKNOWN_CELL_DISPLAY; // Fallback for unexpected data
            }
            printf("%s|", display_str); // Print cell content and separator
        }
        printf("\n"); // Newline after each row
        if (row < 2) {
            printf("-------\n"); // Separator line between rows
        }
    }
    printf("\n"); // Extra newline for spacing after the board
}

// Main function to run a simple Tic-Tac-Toe game
int main() {
    Board* game_board = create_board(); // Create and initialize the game board
    if (game_board == NULL) {
        return EXIT_FAILURE; // Exit if board creation failed
    }

    char current_player_char = PLAYER_X_CHAR; // Player X starts
    int game_status = GAME_IN_PROGRESS;       // Initial game state
    int row, col;                              // Variables for player input
    int move_result;                           // Stores the result of do_move

    printf("Welcome to Tic-Tac-Toe!\n");
    // Display which characters represent Player X and Player O
    printf("Player X is represented by character '%c', Player O by '%c'\n", PLAYER_X_CHAR, PLAYER_O_CHAR);

    // Main game loop: continues as long as the game is in progress
    while (game_status == GAME_IN_PROGRESS) {
        print_board(game_board); // Display the current board state
        printf("Player %s (%c), enter your move (row col, 0-2 for each): ",
               (current_player_char == PLAYER_X_CHAR) ? "X" : "O", current_player_char);

        // Read player input for row and column
        if (scanf("%d %d", &row, &col) != 2) {
            printf("Invalid input. Please enter two numbers (row col).\n");
            // Clear stdin buffer to prevent infinite loop on bad input
            while (getchar() != '\n');
            continue; // Ask for input again
        }
        while (getchar() != '\n'); // Clear any remaining characters in the input buffer

        // Attempt to make the move
        move_result = do_move(game_board, row, col, current_player_char);

        if (move_result == 0) { // Invalid move (out of bounds or cell already taken)
            printf("Invalid move. Cell already taken or coordinates out of bounds. Try again.\n");
            // Game status remains GAME_IN_PROGRESS, loop continues for current player
        } else if (move_result == 4) { // Board was full before this move attempt
            // This scenario should ideally be caught by the while loop condition,
            // but included for robustness if do_move is called when board is full.
            printf("The board was already full. Game over (Draw).\n");
            game_status = GAME_FINAL_DRAW; // Force end the game as a draw
        } else {
            // Move was successful, update the main game_status with the result
            game_status = move_result;

            // If the game is still in progress, switch to the other player
            if (game_status == GAME_IN_PROGRESS) {
                current_player_char = (current_player_char == PLAYER_X_CHAR) ? PLAYER_O_CHAR : PLAYER_X_CHAR;
            }
        }
    }

    // After the loop, the game has ended. Print final board and result.
    print_board(game_board); // Display the final board state

    if (game_status == GAME_X_WINS) {
        printf("Player X (%c) wins!\n", PLAYER_X_CHAR);
    } else if (game_status == GAME_O_WINS) {
        printf("Player O (%c) wins!\n", PLAYER_O_CHAR);
    } else if (game_status == GAME_FINAL_DRAW) {
        printf("It's a draw!\n");
    } else {
        printf("Game ended with an unknown status: %d\n", game_status);
    }

    free(game_board); // Free the allocated board memory
    return EXIT_SUCCESS; // Indicate successful program execution
}