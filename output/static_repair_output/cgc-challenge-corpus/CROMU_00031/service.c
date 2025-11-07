#include <stdio.h>   // For puts, printf, fgets
#include <string.h>  // For strcpy, strcmp, strcspn
#include <ctype.h>   // For isalpha, isdigit, isupper, islower

// Global variable to store the current player's name
char CURR_PLAYER[6]; // "WHITE" or "BLACK"

// Function prototypes
void InitBoard(char board[64]);
void PrintBoard(char board[64]);
int AcceptMove(char board[64], char *move_input_buffer, char *from_col, char *from_row, char *to_col, char *to_row);
int IsValidMove(char board[64], char from_col, char from_row, char to_col, char to_row);
void MakeMove(char board[64], char from_col, char from_row, char to_col, char to_row);

// Initializes the game board with a starting configuration.
void InitBoard(char board[64]) {
    // For a standard 8x8 chess board, 64 characters are used.
    // Uppercase for White pieces, lowercase for Black pieces, '.' for empty squares.
    // R N B Q K B N R
    // P P P P P P P P
    // . . . . . . . .
    // . . . . . . . .
    // . . . . . . . .
    // . . . . . . . .
    // p p p p p p p p
    // r n b q k b n r
    
    // Initialize empty squares
    memset(board, '.', 64);

    // Set up White pieces (uppercase)
    board[0] = 'R'; board[1] = 'N'; board[2] = 'B'; board[3] = 'Q';
    board[4] = 'K'; board[5] = 'B'; board[6] = 'N'; board[7] = 'R';
    for (int i = 0; i < 8; i++) board[8 + i] = 'P';

    // Set up Black pieces (lowercase)
    for (int i = 0; i < 8; i++) board[48 + i] = 'p';
    board[56] = 'r'; board[57] = 'n'; board[58] = 'b'; board[59] = 'q';
    board[60] = 'k'; board[61] = 'b'; board[62] = 'n'; board[63] = 'r';
}

// Prints the current state of the game board to the console.
void PrintBoard(char board[64]) {
    printf("\n--- Current Board ---\n");
    printf("Current Player: %s\n", CURR_PLAYER);
    printf("  a b c d e f g h\n");
    for (int r = 0; r < 8; r++) {
        printf("%d ", 8 - r); // Print rank (row number)
        for (int c = 0; c < 8; c++) {
            printf("%c ", board[r * 8 + c]);
        }
        printf("%d\n", 8 - r);
    }
    printf("  a b c d e f g h\n");
    printf("---------------------\n");
}

// Prompts the current player for a move, reads it, and parses the coordinates.
// Returns 1 if a valid move format is accepted, 0 if the player quits.
int AcceptMove(char board[64], char *move_input_buffer, char *from_col, char *from_row, char *to_col, char *to_row) {
    printf("%s's turn. Enter move (e.g., e2e4) or 'quit': ", CURR_PLAYER);
    
    if (fgets(move_input_buffer, 13, stdin) == NULL) {
        return 0; // EOF or error reading input
    }
    
    // Remove trailing newline character if present
    move_input_buffer[strcspn(move_input_buffer, "\n")] = 0;

    if (strcmp(move_input_buffer, "quit") == 0) {
        return 0; // Player wants to quit
    }

    // Expected move format is "cRcN" (e.g., "e2e4") - 4 characters
    if (strlen(move_input_buffer) != 4 ||
        !isalpha(move_input_buffer[0]) || !isdigit(move_input_buffer[1]) ||
        !isalpha(move_input_buffer[2]) || !isdigit(move_input_buffer[3])) {
        puts("Invalid move format. Please use 'cRcN' format (e.g., e2e4) or 'quit'.");
        // Re-prompt for input by recursively calling itself
        return AcceptMove(board, move_input_buffer, from_col, from_row, to_col, to_row);
    }

    // Parse the input string into individual coordinate characters
    *from_col = move_input_buffer[0];
    *from_row = move_input_buffer[1];
    *to_col = move_input_buffer[2];
    *to_row = move_input_buffer[3];

    return 1; // Move format accepted
}

// Checks if the proposed move is valid according to game rules.
// Returns 1 if valid, 0 otherwise.
int IsValidMove(char board[64], char from_col, char from_row, char to_col, char to_row) {
    // Basic validation: check if coordinates are within the board bounds ('a'-'h', '1'-'8')
    if (from_col < 'a' || from_col > 'h' ||
        to_col < 'a' || to_col > 'h' ||
        from_row < '1' || from_row > '8' ||
        to_row < '1' || to_row > '8') {
        puts("Invalid move: Coordinates out of board bounds.");
        return 0;
    }

    // Convert character coordinates to 0-7 integer indices for array access
    // Chess ranks (rows) are 1-8, with 8 at the top (index 0)
    int fr_idx = '8' - from_row; 
    int fc_idx = from_col - 'a'; // Chess files (columns) are 'a'-'h', 'a' is index 0
    int tr_idx = '8' - to_row;
    int tc_idx = to_col - 'a';

    // Get the piece at the starting position
    char piece = board[fr_idx * 8 + fc_idx];

    if (piece == '.') {
        puts("Invalid move: No piece at the starting position.");
        return 0;
    }

    // Check if the piece belongs to the current player
    if (strcmp(CURR_PLAYER, "WHITE") == 0) {
        if (!isupper(piece)) { // White pieces are uppercase
            puts("Invalid move: Not your piece (White player).");
            return 0;
        }
    } else { // Current player is BLACK
        if (!islower(piece)) { // Black pieces are lowercase
            puts("Invalid move: Not your piece (Black player).");
            return 0;
        }
    }

    // Prevent moving to the same square
    if (fr_idx == tr_idx && fc_idx == tc_idx) {
        puts("Invalid move: Cannot move a piece to its current square.");
        return 0;
    }

    // For a real game, complex piece-specific movement rules would be implemented here
    // (e.g., pawn forward, knight L-shape, rook straight, etc.)
    // For this example, we'll assume any basic move passing the above checks is valid.

    return 1; // Assume the move is valid for this simplified example
}

// Executes the move on the board.
void MakeMove(char board[64], char from_col, char from_row, char to_col, char to_row) {
    // Convert character coordinates to 0-7 integer indices
    int fr_idx = '8' - from_row;
    int fc_idx = from_col - 'a';
    int tr_idx = '8' - to_row;
    int tc_idx = to_col - 'a';

    // Move the piece from the starting square to the destination square
    board[tr_idx * 8 + tc_idx] = board[fr_idx * 8 + fc_idx];
    // Clear the original starting square
    board[fr_idx * 8 + fc_idx] = '.';
}

// Main function: Orchestrates the game loop.
int main(void) {
  char board[64]; // Represents the 8x8 game board
  char move_input_buffer[13]; // Buffer for user input (e.g., "e2e4\n" or "quit\n")
  char from_col, from_row, to_col, to_row; // Parsed coordinates of the move

  InitBoard(board); // Set up the initial board
  strcpy(CURR_PLAYER, "WHITE"); // White player starts
  PrintBoard(board); // Display the initial board

  // Game loop: Continues as long as AcceptMove returns 1 (i.e., player doesn't quit)
  while (AcceptMove(board, move_input_buffer, &from_col, &from_row, &to_col, &to_row)) {
    // Check if the accepted move is valid
    if (IsValidMove(board, from_col, from_row, to_col, to_row)) {
      MakeMove(board, from_col, from_row, to_col, to_row); // Execute the move
      
      // Switch player
      if (strcmp(CURR_PLAYER, "WHITE") == 0) {
        strcpy(CURR_PLAYER, "BLACK");
      } else {
        strcpy(CURR_PLAYER, "WHITE");
      }
      puts("OK"); // Indicate successful move
    } else {
      puts("NO"); // Indicate invalid move
    }
    PrintBoard(board); // Display the board after the move attempt
  }
  
  puts("good game"); // Message when the game ends
  return 0; // Program exits successfully
}