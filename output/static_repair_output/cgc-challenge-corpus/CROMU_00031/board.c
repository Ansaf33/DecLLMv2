#include <stdio.h> // Required for printf

// Function: PrintBoard
// `board` is treated as a pointer to the board data.
// The board is assumed to be 8x8, stored such that elements are accessed as board[col * 8 + row].
void PrintBoard(char *board) {
  char row_buffer[9]; // Buffer to hold one row of characters, plus null terminator
  int row, col;       // Loop counters for rows and columns

  // Loop through rows from 7 down to 0 (representing top to bottom of a standard board)
  for (row = 7; row >= 0; --row) {
    // Loop through columns from 0 to 7 (representing left to right)
    for (col = 0; col < 8; ++col) {
      // Check if the board cell is empty (represented by the null character '\0')
      if (board[col * 8 + row] == '\0') {
        row_buffer[col] = '.'; // Use '.' (ASCII 0x2e) for empty cells
      } else {
        // Otherwise, use the character stored in the board
        row_buffer[col] = board[col * 8 + row];
      }
    }
    row_buffer[8] = '\0'; // Null-terminate the string for printf
    printf("%s\n", row_buffer); // Print the current row
  }
}

// Function: InitBoard
// `board` is treated as a pointer to the board data (an 8x8 character array of 64 bytes).
// The board is initialized with specific piece characters and empty cells.
// Elements are accessed as board[col * 8 + row].
void InitBoard(char *board) {
  int row, col; // Loop counters

  // Initialize specific pieces by direct assignment.
  // The original code copies values from one board position to another,
  // which is preserved here as it reflects the original logic.
  // Character values are commented for clarity (based on common ASCII mappings).

  board[56] = 100; // 'd'
  board[0] = board[56]; // Copies 'd' to board[0]

  board[48] = 0x62; // 'b'
  board[8] = board[48]; // Copies 'b' to board[8]

  board[40] = 99; // 'c'
  board[16] = board[40]; // Copies 'c' to board[16]

  board[24] = 0x66; // 'f'
  board[32] = 0x65; // 'e'

  // Initialize pawns for a specific row (row 1)
  // This sets board[col * 8 + 1] = 'a' for all columns
  for (col = 0; col < 8; ++col) {
    board[col * 8 + 1] = 0x61; // 'a'
  }

  // Clear the middle rows (rows 2, 3, 4, 5) by setting them to '\0'
  for (row = 2; row < 6; ++row) {
    for (col = 0; col < 8; ++col) {
      board[col * 8 + row] = '\0'; // Set to null (empty cell)
    }
  }

  // Initialize pawns for another specific row (row 6)
  // This sets board[col * 8 + 6] = 'g' for all columns
  for (col = 0; col < 8; ++col) {
    board[col * 8 + 6] = 0x67; // 'g'
  }

  // Initialize specific pieces for the other side by direct assignment.
  board[63] = 0x6a; // 'j'
  board[7] = board[63]; // Copies 'j' to board[7]

  board[55] = 0x68; // 'h'
  board[15] = board[55]; // Copies 'h' to board[15]

  board[47] = 0x69; // 'i'
  board[23] = board[47]; // Copies 'i' to board[23]

  board[31] = 0x6c; // 'l'
  board[39] = 0x6b; // 'k'
}

// Main function to demonstrate board initialization and printing.
int main() {
  char board[64]; // Declare an 8x8 board (64 characters)
  InitBoard(board); // Initialize the board
  PrintBoard(board); // Print the initialized board
  return 0;          // Indicate successful execution
}