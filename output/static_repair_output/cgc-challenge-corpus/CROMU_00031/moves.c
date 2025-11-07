#include <stdio.h>   // For printf, puts, fgets, fprintf
#include <stdlib.h>  // For exit
#include <string.h>  // For strcmp, strchr, strlen, memset

// --- Global Variables and External Functions for Compilation ---
// In a real application, these would be defined in appropriate header/source files.

// Chess board representation: 8x8 array of chars. '\0' for empty.
// This is a global variable, accessible by all functions.
char board[8][8];

// Current player (e.g., "WHITE" or "BLACK")
char CURR_PLAYER[10] = "WHITE"; // Default to WHITE for demonstration

// Prompt message for user input
const char * const PROMPT_MSG = "Enter your move (e.g., 0,0 0,1): ";

// Quit command
const char * const QUIT_CMD = "quit";

// Invalid format message
const char * const INVALID_FORMAT_MSG = "Not a legal move format";

// Dummy PrintBoard function (uses the global 'board' variable)
void PrintBoard() {
    printf("  0 1 2 3 4 5 6 7\n");
    for (int i = 0; i < 8; ++i) {
        printf("%d ", i);
        for (int j = 0; j < 8; ++j) {
            printf("%c ", board[i][j] == '\0' ? '.' : board[i][j]); // Use '.' for empty squares
        }
        printf("\n");
    }
    printf("Current player: %s\n", CURR_PLAYER);
}

// Dummy receive_until function (simulates reading from network or stdin)
// Reads up to max_len-1 characters from stdin or until stop_char is found.
// Removes trailing newline if present.
int receive_until(char *buffer, int stop_char, size_t max_len) {
    if (fgets(buffer, max_len, stdin) != NULL) {
        // Remove trailing newline if present, as fgets keeps it
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len-1] == '\n') {
            buffer[len-1] = '\0';
        }
        // For simplicity, we assume fgets already handled reading up to a newline.
        return 1; // Success
    }
    return 0; // Failure (e.g., EOF)
}

// Dummy _terminate function (replaces original _terminate)
void _terminate(int status) {
    fprintf(stderr, "Program terminated with status %d\n", status);
    exit(status);
}

// --- Provided and Fixed Functions ---

// Function: custom_abs (renamed to avoid conflict with stdlib.h's abs)
int custom_abs(int __x) {
  if (__x < 0) {
    __x = -__x;
  }
  return __x;
}

// Function: IsDiagClear
// param_1 is now char (*board_ptr)[8] to correctly represent a 2D array
int IsDiagClear(char (*board_ptr)[8], char start_row, char start_col, char end_row, char end_col) {
  char current_row;
  char current_col;
  
  if (start_row < end_row) { // Moving down (increasing row index)
    if (start_col < end_col) { // Moving right (increasing column index)
      current_row = start_row + 1;
      for (current_col = start_col + 1; (current_row < end_row && (current_col < end_col));
          current_col = current_col + 1) {
        if (board_ptr[current_row][current_col] != '\0') {
          return 0; // Path blocked
        }
        current_row = current_row + 1;
      }
    }
    else { // Moving left (decreasing column index)
      current_row = start_row + 1;
      for (current_col = start_col - 1; (current_row < end_row && (end_col < current_col));
          current_col = current_col - 1) {
        if (board_ptr[current_row][current_col] != '\0') {
          return 0; // Path blocked
        }
        current_row = current_row + 1;
      }
    }
  }
  else if (start_col < end_col) { // Moving up (decreasing row index), then right (increasing column index)
    current_row = start_row - 1;
    for (current_col = start_col + 1; (end_row < current_row && (current_col < end_col));
        current_col = current_col + 1) {
      if (board_ptr[current_row][current_col] != '\0') {
        return 0; // Path blocked
      }
      current_row = current_row - 1;
    }
  }
  else { // Moving up (decreasing row index), then left (decreasing column index)
    current_row = start_row - 1;
    for (current_col = start_col - 1; (end_row < current_row && (end_col < current_col)); current_col = current_col - 1)
    {
      if (board_ptr[current_row][current_col] != '\0') {
        return 0; // Path blocked
      }
      current_row = current_row - 1;
    }
  }
  return 1; // Path is clear
}

// Function: IsLateralClear
// param_1 is now char (*board_ptr)[8]
int IsLateralClear(char (*board_ptr)[8], char start_row, char start_col, char end_row, char end_col) {
  char iter;
  
  if (start_row < end_row) { // Moving down (increasing row)
    for (iter = start_row + 1; iter < end_row; iter = iter + 1) {
      if (board_ptr[iter][start_col] != '\0') {
        return 0; // Path blocked
      }
    }
  }
  else if (end_row < start_row) { // Moving up (decreasing row)
    for (iter = end_row + 1; iter < start_row; iter = iter + 1) {
      if (board_ptr[iter][start_col] != '\0') {
        return 0; // Path blocked
      }
    }
  }
  else if (start_col < end_col) { // Moving right (increasing column)
    for (iter = start_col + 1; iter < end_col; iter = iter + 1) {
      if (board_ptr[start_row][iter] != '\0') {
        return 0; // Path blocked
      }
    }
  }
  else if (end_col < start_col) { // Moving left (decreasing column)
    for (iter = end_col + 1; iter < start_col; iter = iter + 1) {
      if (board_ptr[start_row][iter] != '\0') {
        return 0; // Path blocked
      }
    }
  }
  return 1; // Path is clear
}

// Function: IsMemberPiece
int IsMemberPiece(const char *piece_set, char piece) {
  char *found_char = strchr(piece_set, (int)piece);
  // Returns 1 if 'piece' is found in 'piece_set', 0 otherwise.
  // Assuming 'piece' will not be the null terminator itself in this context.
  return found_char != NULL;
}

// Function: IsValidMove
// param_1 is now char (*board_ptr)[8]
int IsValidMove(char (*board_ptr)[8], char start_row, char start_col, char end_row, char end_col) {
  int diff_row, diff_col;
  int piece_char_at_src;
  
  if ((start_row == end_row) && (start_col == end_col)) {
    return 0; // Cannot move to the same square
  }

  // Check if current player owns the piece at source
  if (strcmp(CURR_PLAYER, "WHITE") == 0) {
    if (!IsMemberPiece("dbcfea", board_ptr[start_row][start_col])) {
      return 0; // White player must move a white piece
    }
  } else if (strcmp(CURR_PLAYER, "BLACK") == 0) {
    if (!IsMemberPiece("jhilkg", board_ptr[start_row][start_col])) {
      return 0; // Black player must move a black piece
    }
  }

  // Check if destination square contains a piece of the same color
  if (strcmp(CURR_PLAYER, "WHITE") == 0) {
    if (IsMemberPiece("dbcfea", board_ptr[end_row][end_col])) {
      return 0; // White player cannot capture own piece
    }
  } else if (strcmp(CURR_PLAYER, "BLACK") == 0) {
    if (IsMemberPiece("jhilkg", board_ptr[end_row][end_col])) {
      return 0; // Black player cannot capture own piece
    }
  }

  piece_char_at_src = board_ptr[start_row][start_col];
  if (piece_char_at_src == '\0') {
    return 0; // No piece at source to move
  }

  // Check if the piece character is within valid range ('a' to 'l')
  if (piece_char_at_src < 'a' || piece_char_at_src > 'l') {
      return 0; // Invalid piece character
  }

  switch(piece_char_at_src) {
    case 'a': // White Pawn (moves forward 1, assuming increasing column is forward for pawn 'a')
      if ((start_row == end_row) && (start_col + 1 == end_col)) {
        return 1;
      }
      return 0;
    case 'g': // Black Pawn (moves backward 1, assuming decreasing column is backward for pawn 'g')
      if ((start_row == end_row) && (start_col - 1 == end_col)) {
        return 1;
      }
      return 0;
    case 'b': // White Knight
    case 'h': // Black Knight
      diff_row = custom_abs(start_row - end_row);
      diff_col = custom_abs(start_col - end_col);
      if ((diff_row == 2 && diff_col == 1) || (diff_row == 1 && diff_col == 2)) {
        return 1; // L-shaped move
      }
      return 0;
    case 'c': // White Bishop
    case 'i': // Black Bishop
      diff_row = custom_abs(end_row - start_row);
      diff_col = custom_abs(end_col - start_col);
      if (diff_row == diff_col) { // Diagonal move
        return IsDiagClear(board_ptr, start_row, start_col, end_row, end_col);
      }
      return 0;
    case 'd': // White Rook
    case 'j': // Black Rook
      if ((start_row == end_row) || (start_col == end_col)) { // Lateral move
        return IsLateralClear(board_ptr, start_row, start_col, end_row, end_col);
      }
      return 0;
    case 'e': // White King
    case 'k': // Black King
      diff_row = custom_abs(start_row - end_row);
      diff_col = custom_abs(start_col - end_col);
      if ((diff_row < 2) && (diff_col < 2)) { // Moves one square in any direction
        return 1;
      }
      return 0;
    case 'f': // White Queen
    case 'l': // Black Queen
      if ((start_row == end_row) || (start_col == end_col)) { // Lateral move
        return IsLateralClear(board_ptr, start_row, start_col, end_row, end_col);
      } else { // Diagonal move
        diff_row = custom_abs(end_row - start_row);
        diff_col = custom_abs(end_col - start_col);
        if (diff_row == diff_col) {
          return IsDiagClear(board_ptr, start_row, start_col, end_row, end_col);
        }
      }
      return 0;
    default:
      return 0; // Should not be reached due to initial range check, but safe fallback.
  }
}

// Function: AcceptMove
// param_1 is now char (*board_ptr)[8]
int AcceptMove(char (*board_ptr)[8], char *out_src_row, char *out_src_col, char *out_dest_row, char *out_dest_col) {
  char input_buffer[16]; // Buffer for user input: max 15 chars + null terminator
  char *move_part1 = NULL;
  char *move_part2 = NULL;
  unsigned int i;
  const char *valid_digits = "01234567"; // String of valid digit characters for board coordinates

  while (1) { // Loop until a valid move is accepted or "quit" command is entered
    printf("%s", PROMPT_MSG);
    
    // Clear buffer before receiving input
    memset(input_buffer, 0, sizeof(input_buffer));

    // Receive input from the user
    if (receive_until(input_buffer, '\n', sizeof(input_buffer)) == 0) {
      _terminate(1); // Terminate if input reception fails
    }

    // Check for "quit" command
    if (strcmp(input_buffer, QUIT_CMD) == 0) {
      return 0; // Signal to the caller that the user wants to quit
    }
    
    // Check for "print" command
    if (strcmp(input_buffer, "print") == 0) {
        PrintBoard(); // Display the current board state
        continue; // Ask for input again
    }

    // Validate input length: Expecting "R,C R,C" format, which is 7 characters long
    if (strlen(input_buffer) != 7) {
      puts("incorrect input");
      puts(INVALID_FORMAT_MSG);
      continue; // Ask for input again
    }

    // Find the space separator to split the input into two parts (source and destination)
    move_part1 = input_buffer; // First part starts at the beginning of the buffer
    move_part2 = NULL;         // Initialize second part pointer to NULL
    for (i = 0; i < strlen(input_buffer); ++i) {
      if (input_buffer[i] == ' ') {
        input_buffer[i] = '\0'; // Null-terminate the first part at the space
        move_part2 = &input_buffer[i + 1]; // Second part starts after the space
        break; // Found the space, exit loop
      }
    }

    if (move_part2 == NULL) { // No space found, input is malformed
      puts("incorrect input");
      puts(INVALID_FORMAT_MSG);
      continue; // Ask for input again
    }

    // Validate and parse the first part (source coordinates)
    // Expected format: "R,C" where R and C are single digits '0'-'7'
    if (strchr(valid_digits, move_part1[0]) == NULL || // Row char must be a digit
        move_part1[1] != ',' ||                        // Second char must be a comma
        strchr(valid_digits, move_part1[2]) == NULL || // Column char must be a digit
        strlen(move_part1) != 3) {                     // Part length must be exactly 3
      puts("incorrect input");
      puts(INVALID_FORMAT_MSG);
      continue; // Ask for input again
    }
    
    // Convert character digits to integer values and store in output parameters
    *out_src_row = move_part1[0] - '0';
    *out_src_col = move_part1[2] - '0';

    // Validate and parse the second part (destination coordinates)
    // Expected format: "R,C" where R and C are single digits '0'-'7'
    if (strchr(valid_digits, move_part2[0]) == NULL || // Row char must be a digit
        move_part2[1] != ',' ||                        // Second char must be a comma
        strchr(valid_digits, move_part2[2]) == NULL || // Column char must be a digit
        strlen(move_part2) != 3) {                     // Part length must be exactly 3
      puts("incorrect input");
      puts(INVALID_FORMAT_MSG);
      continue; // Ask for input again
    }

    // Convert character digits to integer values and store in output parameters
    *out_dest_row = move_part2[0] - '0';
    *out_dest_col = move_part2[2] - '0';

    return 1; // Valid move parsed successfully
  }
}

// Function: MakeMove
// param_1 is now char (*board_ptr)[8]
void MakeMove(char (*board_ptr)[8], char start_row, char start_col, char end_row, char end_col) {
  char temp_piece;
  
  if (board_ptr[end_row][end_col] == '\0') { // Destination square is empty, simply move the piece
    board_ptr[end_row][end_col] = board_ptr[start_row][start_col];
    board_ptr[start_row][start_col] = '\0'; // Clear the source square
  }
  else { // Destination square has a piece, perform a swap (as per original logic)
         // Note: In typical chess, this would be a capture where the source piece replaces the
         // destination piece, and the source square is cleared. The original code performs a swap.
    temp_piece = board_ptr[end_row][end_col];
    board_ptr[end_row][end_col] = board_ptr[start_row][start_col];
    board_ptr[start_row][start_col] = temp_piece; // Swap the pieces
  }
  return;
}

// --- Main function to demonstrate usage ---
int main() {
    // Initialize board with empty squares
    memset(board, '\0', sizeof(board));
    
    // Setup initial chess pieces for demonstration
    // White pieces (using 'a' through 'f' characters)
    board[0][0] = 'd'; board[0][1] = 'b'; board[0][2] = 'c'; board[0][3] = 'f'; // Rook, Knight, Bishop, Queen
    board[0][4] = 'e'; board[0][5] = 'c'; board[0][6] = 'b'; board[0][7] = 'd'; // King, Bishop, Knight, Rook
    for (int j = 0; j < 8; ++j) {
        board[1][j] = 'a'; // White Pawns
    }

    // Black pieces (using 'g' through 'l' characters)
    board[7][0] = 'j'; board[7][1] = 'h'; board[7][2] = 'i'; board[7][3] = 'l'; // Rook, Knight, Bishop, Queen
    board[7][4] = 'k'; board[7][5] = 'i'; board[7][6] = 'h'; board[7][7] = 'j'; // King, Bishop, Knight, Rook
    for (int j = 0; j < 8; ++j) {
        board[6][j] = 'g'; // Black Pawns
    }

    PrintBoard(); // Display initial board state

    char src_row, src_col, dest_row, dest_col;
    int move_status;

    // Main game loop
    while (1) {
        move_status = AcceptMove(board, &src_row, &src_col, &dest_row, &dest_col);
        if (move_status == 0) { // User entered "quit"
            printf("Quitting game.\n");
            break;
        }

        printf("Attempting move from (%d,%d) to (%d,%d)\n", src_row, src_col, dest_row, dest_col);

        if (IsValidMove(board, src_row, src_col, dest_row, dest_col)) {
            printf("Move is valid. Making move.\n");
            MakeMove(board, src_row, src_col, dest_row, dest_col);
            PrintBoard(); // Display board after valid move

            // Switch current player for the next turn
            if (strcmp(CURR_PLAYER, "WHITE") == 0) {
                strcpy(CURR_PLAYER, "BLACK");
            } else {
                strcpy(CURR_PLAYER, "WHITE");
            }
        } else {
            printf("Invalid move. Try again.\n");
        }
    }

    return 0;
}