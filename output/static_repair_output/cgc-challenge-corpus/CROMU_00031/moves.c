#include <stdio.h>   // For printf, puts, fprintf, stdin, fgets
#include <string.h>  // For strcmp, strchr, strlen, memset, strcpy
#include <stdlib.h>  // For abs, exit

// --- Global variables and dummy functions (for compilation) ---
// In a real application, these would be defined elsewhere or linked.
// Assuming CURR_PLAYER is a string buffer for player turn.
char CURR_PLAYER[10] = "WHITE"; // Example initial value for testing

// String literals inferred from original snippet's usage
const char *DAT_00014050 = "Enter move: ";
const char *DAT_00014055 = "quit";
const char *DAT_00014057 = "exit";

// Global buffer for receive_until (as implied by original snippet's lack of args and usage)
static char input_buffer_global[16]; // Max 15 chars + null terminator

// Dummy implementation for receive_until
// Original code implies it reads into `local_34` (now `input_buffer_global`)
// and returns 0 on failure. It seems to read up to 10 chars, but `bzero` was 15,
// and `0x10` (16) was a value before the call, suggesting a buffer size of 16.
int receive_until() {
    // Simulate reading input. In a real application, this would be network or file I/O.
    // For compilation, we'll use fgets.
    if (fgets(input_buffer_global, sizeof(input_buffer_global), stdin) == NULL) {
        return 0; // Indicate failure
    }
    return 1; // Indicate success
}

// Dummy implementation for _terminate
void _terminate() {
    fprintf(stderr, "Program terminated due to critical error.\n");
    exit(1);
}

// Dummy implementation for PrintBoard
void PrintBoard() {
    printf("--- Board state (printing not implemented in snippet) ---\n");
    // In a real chess game, this would print the current board state.
}

// --- Original functions, refactored ---

// Function: abs (keeping custom implementation as provided)
int abs(int __x) {
  if (__x < 0) {
    __x = -__x;
  }
  return __x;
}

// Function: IsDiagClear
// param_1 is assumed to be a pointer to the start of an 8x8 char board.
int IsDiagClear(char *board, char param_2, char param_3, char param_4, char param_5) {
  char current_row;
  char current_col;
  
  if (param_2 < param_4) { // Moving down rows
    if (param_3 < param_5) { // Moving right columns
      current_row = param_2 + 1;
      for (current_col = param_3 + 1; (current_row < param_4 && (current_col < param_5));
          current_col = current_col + 1) {
        if (board[current_row * 8 + current_col] != '\0') {
          return 0; // Obstruction found
        }
        current_row = current_row + 1;
      }
    }
    else { // Moving left columns (param_3 > param_5)
      current_row = param_2 + 1;
      for (current_col = param_3 - 1; (current_row < param_4 && (param_5 < current_col));
          current_col = current_col - 1) {
        if (board[current_row * 8 + current_col] != '\0') {
          return 0; // Obstruction found
        }
        current_row = current_row + 1;
      }
    }
  }
  else { // Moving up rows (param_2 > param_4)
    if (param_3 < param_5) { // Moving right columns
      current_row = param_2 - 1;
      for (current_col = param_3 + 1; (param_4 < current_row && (current_col < param_5));
          current_col = current_col + 1) {
        if (board[current_row * 8 + current_col] != '\0') {
          return 0; // Obstruction found
        }
        current_row = current_row - 1;
      }
    }
    else { // Moving left columns (param_3 > param_5)
      current_row = param_2 - 1;
      for (current_col = param_3 - 1; (param_4 < current_row && (param_5 < current_col)); current_col = current_col - 1)
      {
        if (board[current_row * 8 + current_col] != '\0') {
          return 0; // Obstruction found
        }
        current_row = current_row - 1;
      }
    }
  }
  return 1; // Path is clear
}

// Function: IsLateralClear
// param_1 is assumed to be a pointer to the start of an 8x8 char board.
int IsLateralClear(char *board, char param_2, char param_3, char param_4, char param_5) {
  char current_coord; // Used for either row or column iteration
  
  if (param_2 < param_4) { // Moving down rows (fixed column param_3)
    for (current_coord = param_2 + 1; current_coord < param_4; current_coord = current_coord + 1) {
      if (board[current_coord * 8 + param_3] != '\0') {
        return 0; // Obstruction found
      }
    }
  }
  else if (param_4 < param_2) { // Moving up rows (fixed column param_3)
    for (current_coord = param_4 + 1; current_coord < param_2; current_coord = current_coord + 1) {
      if (board[current_coord * 8 + param_3] != '\0') {
        return 0; // Obstruction found
      }
    }
  }
  else if (param_3 < param_5) { // Moving right columns (fixed row param_2)
    for (current_coord = param_3 + 1; current_coord < param_5; current_coord = current_coord + 1) {
      if (board[param_2 * 8 + current_coord] != '\0') {
        return 0; // Obstruction found
      }
    }
  }
  else if (param_5 < param_3) { // Moving left columns (fixed row param_2)
    for (current_coord = param_5 + 1; current_coord < param_3; current_coord = current_coord + 1) {
      if (board[param_2 * 8 + current_coord] != '\0') {
        return 0; // Obstruction found
      }
    }
  }
  return 1; // Path is clear
}

// Function: IsMemberPiece
// Checks if `param_2` (a piece character) is present in the string `param_1`.
// The original logic accounted for `param_2` being '\0', but piece chars are not '\0'.
int IsMemberPiece(char *param_1, char param_2) {
  // `strchr` returns NULL if the character is not found.
  // Since `param_2` represents a piece character (e.g., 'a', 'b'), it will not be '\0'.
  // Thus, checking for `NULL` is sufficient to determine if it's a member.
  return strchr(param_1, param_2) != NULL;
}

// Function: IsValidMove
// param_1 is assumed to be a pointer to the start of an 8x8 char board.
int IsValidMove(char *board, char param_2, char param_3, char param_4, char param_5) {
  
  // 1. Check for no move (source and destination are the same)
  if ((param_2 == param_4) && (param_3 == param_5)) {
    return 0;
  }

  // 2. Determine current player's piece set
  const char *player_pieces;
  // const char *opponent_pieces; // Not directly used in this function's simplified logic
  if (strcmp(CURR_PLAYER, "WHITE") == 0) {
      player_pieces = "dbcfea"; // White pieces
      // opponent_pieces = "jhilkg"; // Black pieces
  } else if (strcmp(CURR_PLAYER, "BLACK") == 0) {
      player_pieces = "jhilkg"; // Black pieces
      // opponent_pieces = "dbcfea"; // White pieces
  } else {
      // Unknown player, or error. Assume invalid move.
      return 0;
  }

  char source_piece = board[param_2 * 8 + param_3];
  char dest_piece = board[param_4 * 8 + param_5];

  // 3. Check if source piece is empty
  if (source_piece == '\0') {
    return 0;
  }

  // 4. Check if source piece belongs to current player
  if (!IsMemberPiece((char *)player_pieces, source_piece)) {
    return 0;
  }
  
  // 5. Check if destination square contains a piece of the current player (cannot capture own piece)
  if (IsMemberPiece((char *)player_pieces, dest_piece)) {
    return 0;
  }

  // 6. Check if piece is within expected ASCII range ('a' to 'l')
  // This filters out any unknown or invalid piece characters.
  if (source_piece < 'a' || source_piece > 'l') {
    return 0;
  }

  int move_is_valid = 0; // Initialize return value to 0 (invalid)

  // 7. Evaluate move based on piece type
  switch(source_piece) {
    case 'a': // White Pawn
      // Simplified pawn move: only forward one square
      if ((param_2 == param_4) && (param_3 + 1 == param_5)) {
        move_is_valid = 1;
      }
      // Additional pawn rules (captures, initial two-square move, en passant, promotion)
      // would go here if implemented.
      break;
    case 'g': // Black Pawn
      // Simplified pawn move: only forward one square
      if ((param_2 == param_4) && (param_3 - 1 == param_5)) {
        move_is_valid = 1;
      }
      // Additional pawn rules would go here.
      break;
    case 'b': // White Knight
    case 'h': // Black Knight
      // Knight moves in an L-shape (2 squares in one direction, 1 in perpendicular)
      if (((abs((int)param_2 - (int)param_4) == 2) && (abs((int)param_3 - (int)param_5) == 1)) ||
          ((abs((int)param_2 - (int)param_4) == 1) && (abs((int)param_3 - (int)param_5) == 2))) {
        move_is_valid = 1;
      }
      break;
    case 'c': // White Bishop
    case 'i': // Black Bishop
      // Bishop moves diagonally
      if (abs((int)param_4 - (int)param_2) == abs((int)param_5 - (int)param_3)) {
        move_is_valid = IsDiagClear(board, param_2, param_3, param_4, param_5);
      }
      break;
    case 'd': // White Rook
    case 'j': // Black Rook
      // Rook moves laterally (horizontally or vertically)
      if ((param_2 == param_4) || (param_3 == param_5)) {
        move_is_valid = IsLateralClear(board, param_2, param_3, param_4, param_5);
      }
      break;
    case 'e': // White King
    case 'k': // Black King
      // King moves one square in any direction
      if ((abs((int)param_2 - (int)param_4) < 2) && (abs((int)param_3 - (int)param_5) < 2)) {
        move_is_valid = 1;
      }
      break;
    case 'f': // White Queen
    case 'l': // Black Queen
      // Queen moves laterally or diagonally
      if ((param_2 == param_4) || (param_3 == param_5)) { // Lateral move
        move_is_valid = IsLateralClear(board, param_2, param_3, param_4, param_5);
      }
      else if (abs((int)param_4 - (int)param_2) == abs((int)param_5 - (int)param_3)) { // Diagonal move
        move_is_valid = IsDiagClear(board, param_2, param_3, param_4, param_5);
      }
      break;
    // No 'default' case is strictly needed here because the initial `source_piece < 'a' || source_piece > 'l'`
    // check ensures `source_piece` is one of the handled characters.
  }
  return move_is_valid;
}

// Function: AcceptMove
// Parses user input for a chess move (e.g., "1,2 3,4").
// Returns 1 on successful parsing, 0 if user wants to exit.
// Output parameters param_2, param_3, param_4, param_5 will contain the parsed coordinates.
// The first parameter (int param_1 in original) was unused and has been removed.
int AcceptMove(char *param_2, char *param_3, char *param_4, char *param_5) {
  char *dest_coords_ptr;    // Pointer to the destination part of the input string (e.g., "3,4")
  char *source_coords_ptr;  // Pointer to the source part of the input string (e.g., "1,2")
  unsigned int i;           // Loop counter for parsing input string
  const char *valid_coords = "01234567"; // String containing valid coordinate characters

  while (1) { // Loop indefinitely until a valid move or exit command is received
    printf("%s", DAT_00014050); // Prompt user: "Enter move: "
    
    memset(input_buffer_global, 0, sizeof(input_buffer_global)); // Clear input buffer
    
    if (receive_until() == 0) { // Read input into `input_buffer_global`
      _terminate(); // If `receive_until` fails, terminate the program
    }
    
    // Remove trailing newline character if present (from fgets)
    size_t input_len = strlen(input_buffer_global);
    if (input_len > 0 && input_buffer_global[input_len - 1] == '\n') {
      input_buffer_global[input_len - 1] = '\0';
      input_len--; // Adjust length after removing newline
    }

    // Check for "quit" command
    if (strcmp(input_buffer_global, DAT_00014055) == 0) {
      PrintBoard(); // Print board state and then continue to ask for a new move
      continue;
    }

    // Check for "exit" command
    if (strcmp(input_buffer_global, DAT_00014057) == 0) {
      return 0; // Indicate program exit
    }

    // Validate minimum input length (e.g., "1,2 3,4" is 7 characters)
    if (input_len < 7) {
      puts("incorrect input");
      puts("Not a legal move format (too short)");
      continue;
    }

    // Parse input string to find the space separator between source and destination coordinates
    dest_coords_ptr = NULL; 
    source_coords_ptr = NULL;
    i = 0; 
    
    while (i < input_len) {
      if (input_buffer_global[i] == ' ') {
        input_buffer_global[i] = '\0'; // Null-terminate the source part
        source_coords_ptr = input_buffer_global; // Source is the beginning of the buffer
        dest_coords_ptr = &input_buffer_global[i + 1]; // Destination starts after the space
        break;
      }
      i++;
    }

    // If no space was found or parsing error, it's an invalid format
    if (source_coords_ptr == NULL || dest_coords_ptr == NULL) {
      puts("incorrect input");
      puts("Not a legal move format (missing space)");
      continue;
    }

    // Validate source coordinates format (e.g., "1,2")
    // Checks: first char is a digit, second is ',', third is a digit, total length is 3.
    if (strchr(valid_coords, source_coords_ptr[0]) != NULL &&
        source_coords_ptr[1] == ',' &&
        strchr(valid_coords, source_coords_ptr[2]) != NULL &&
        strlen(source_coords_ptr) == 3)
    {
      *param_2 = source_coords_ptr[0] - '0'; // Convert char digit to int row
      *param_3 = source_coords_ptr[2] - '0'; // Convert char digit to int col

      // Validate destination coordinates format (e.g., "3,4")
      // Same checks as for source coordinates.
      if (strchr(valid_coords, dest_coords_ptr[0]) != NULL &&
          dest_coords_ptr[1] == ',' &&
          strchr(valid_coords, dest_coords_ptr[2]) != NULL &&
          strlen(dest_coords_ptr) == 3)
      {
        *param_4 = dest_coords_ptr[0] - '0'; // Convert char digit to int dest row
        *param_5 = dest_coords_ptr[2] - '0'; // Convert char digit to int dest col
        return 1; // Valid move successfully parsed
      }
    }

    // If any validation fails, print error messages and loop again
    puts("incorrect input");
    puts("Not a legal move format");
  }
}

// Function: MakeMove
// param_1 is assumed to be a pointer to the start of an 8x8 char board.
void MakeMove(char *board, char param_2, char param_3, char param_4, char param_5) {
  char temp_piece;
  
  // Get pointers to the source and destination squares on the board
  char *source_square = &board[param_2 * 8 + param_3];
  char *dest_square = &board[param_4 * 8 + param_5];

  if (*dest_square == '\0') { // Destination square is empty (standard move)
    *dest_square = *source_square; // Move piece to destination
    *source_square = '\0';        // Clear source square
  }
  else { // Destination square has a piece (original logic performs a swap)
    // Note: In standard chess, this would be a capture, meaning the destination piece
    // is removed. The original snippet performs a swap. Keeping original behavior.
    temp_piece = *dest_square;      // Store destination piece
    *dest_square = *source_square; // Move source piece to destination
    *source_square = temp_piece;   // Put destination piece back on source square (swap)
  }
}

// --- Main function to demonstrate usage and make code compilable ---
int main() {
    char board[8][8]; // Example 8x8 chess board
    
    // Initialize board with empty squares
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            board[i][j] = '\0';
        }
    }

    // Place some example pieces for demonstration
    board[1][1] = 'a'; // White pawn at (1,1)
    board[6][6] = 'g'; // Black pawn at (6,6)
    board[0][0] = 'd'; // White rook at (0,0)
    board[7][7] = 'j'; // Black rook at (7,7)
    board[0][2] = 'c'; // White bishop at (0,2)
    board[7][5] = 'i'; // Black bishop at (7,5)
    board[0][1] = 'b'; // White knight at (0,1)
    board[7][6] = 'h'; // Black knight at (7,6)
    board[0][3] = 'f'; // White queen at (0,3)
    board[7][4] = 'l'; // Black queen at (7,4)
    board[0][4] = 'e'; // White king at (0,4)
    board[7][3] = 'k'; // Black king at (7,3)

    char from_row, from_col, to_row, to_col;
    int move_accepted;

    printf("Welcome to Simple Chess!\n");
    printf("Current player: %s\n", CURR_PLAYER);
    PrintBoard(); // Display initial board state

    while (1) {
        // Accept user input for a move
        move_accepted = AcceptMove(&from_row, &from_col, &to_row, &to_col);

        if (move_accepted == 0) { // User entered "exit"
            printf("Exiting game.\n");
            break;
        }

        printf("Attempting move from (%d,%d) to (%d,%d)\n", from_row, from_col, to_row, to_col);

        // Validate the move
        if (IsValidMove((char *)board, from_row, from_col, to_row, to_col)) {
            MakeMove((char *)board, from_row, from_col, to_row, to_col);
            printf("Move successful!\n");
            PrintBoard(); // Display board after successful move
            
            // Switch current player for next turn
            if (strcmp(CURR_PLAYER, "WHITE") == 0) {
                strcpy(CURR_PLAYER, "BLACK");
            } else {
                strcpy(CURR_PLAYER, "WHITE");
            }
            printf("Current player: %s\n", CURR_PLAYER);
        } else {
            printf("Invalid move. Please try again.\n");
        }
    }

    return 0;
}