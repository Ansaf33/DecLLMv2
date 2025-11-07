#include <stdio.h>   // For printf, fflush, stdin, stdout, FILE, feof, fgets
#include <string.h>  // For strchr, strcmp
#include <stdlib.h>  // For exit

// --- Global variables (stub definitions for missing external data) ---
#define BUFFER_SIZE 4096 // Corresponds to 0x1000 in original snippet
char buf_0[BUFFER_SIZE];
const char *DAT_00016154 = "White"; // Placeholder for string at 0x00016154
const char *DAT_0001615c = "Black"; // Placeholder for string at 0x0001615c
const char *DAT_00016164 = "move";  // Placeholder for string at 0x00016164

// --- Stub functions (for missing external functions) ---

// Placeholder for get_flag_byte. Returns a byte value.
unsigned int get_flag_byte(unsigned int index) {
    // Simple placeholder implementation for compilation
    return (index % 256);
}

// Placeholder for seed_ai.
void seed_ai(char *data) {
    (void)data; // Suppress unused parameter warning
    // In a real program, this would initialize AI state.
}

// Placeholder for init_bitboard.
void init_bitboard(char *board) {
    (void)board; // Suppress unused parameter warning
    // In a real program, this would set up the game board.
}

// Placeholder for print_bitboard.
void print_bitboard(char *board, unsigned int player_turn) {
    (void)board; // Suppress unused parameter warning
    // In a real program, this would display the game board.
    printf("[DEBUG] Board printed for player %u.\n", player_turn);
}

// Placeholder for fread_until.
// Reads up to max_size-1 characters or until EOF/error into buffer.
// It does NOT handle the delimiter internally for null-termination,
// consistent with the original code's subsequent strchr call.
char *fread_until(char *buffer, int delimiter, int max_size, FILE *stream) {
    // fgets reads until newline or EOF or max_size-1 chars.
    if (fgets(buffer, max_size, stream) == NULL) {
        if (feof(stream)) {
            exit(0); // Exit gracefully on EOF
        }
        return NULL; // Indicate error
    }
    (void)delimiter; // Suppress unused parameter warning as it's not used in this simplified stub
    return buffer;   // Indicate success by returning the buffer
}

// Placeholder for parse_san.
// Returns 0 for success, non-zero for failure.
int parse_san(char *board, unsigned int player_turn, char *input_san, char *move_data) {
    (void)board;        // Suppress unused parameter warning
    (void)player_turn;  // Suppress unused parameter warning
    (void)input_san;    // Suppress unused parameter warning
    (void)move_data;    // Suppress unused parameter warning
    // For compilation, let's assume it always succeeds.
    return 0;
}

// Placeholder for make_move.
// Returns 3 for stalemate, 1 for check, 2 for checkmate, 0 for normal move, -1 for invalid move.
int make_move(char *board, char *move_data) {
    (void)board;      // Suppress unused parameter warning
    (void)move_data;  // Suppress unused parameter warning
    // For compilation, let's return a normal move (0).
    return 0;
}

// --- Original functions, fixed ---

// Function: calculate_csum
unsigned int calculate_csum(unsigned int param_1) {
  unsigned int checksum = 0; // local_10 reduced to checksum
  for (int i = 0; i < 0x400; ++i) { // local_14 reduced to i
    checksum ^= get_flag_byte(i) & 0xff;
  }
  return checksum ^ param_1;
}

// Function: main
int main(void) {
  // local_10 = &stack0x00000004; removed as it's a decompilation artifact
  char board_state[124]; // local_a0
  char move_data[52];    // local_d4
  char ai_seed_data[4];  // local_24
  
  int init_flag = 1;     // local_18, controls board initialization
  unsigned int player_turn = 0; // local_14, indicates current player (0 or 1)
  
  seed_ai(ai_seed_data);

  while(1) { // Changed while(true) to while(1) for standard C
    char *newline_ptr; // local_1c
    int parse_result_code;  // iVar2 for strcmp and parse_san return
    int move_outcome;   // local_20
    
    do {
      if (init_flag != 0) {
        init_bitboard(board_state);
        player_turn = 0; // Reset player turn on new game
        init_flag = 0;   // Clear init flag
      }
      print_bitboard(board_state, player_turn);

      // Reduced intermediate variable puVar1 by using a direct conditional expression
      printf("%s>%s ", (player_turn == 0) ? DAT_00016154 : DAT_0001615c, DAT_00016164);
      
      fflush(stdout);
      fread_until(buf_0, '\n', BUFFER_SIZE, stdin); // '\n' is ASCII for 10
      newline_ptr = strchr(buf_0, '\n'); // Find the newline character
    } while (newline_ptr == NULL); // Loop until a newline is found in input
    
    *newline_ptr = '\0'; // Null-terminate the string at the newline

    parse_result_code = strcmp(buf_0, "quit");
    if (parse_result_code == 0) {
        break; // Exit loop if input is "quit"
    }

    // parse_result_code now holds the result of parse_san
    parse_result_code = parse_san(board_state, player_turn, buf_0, move_data);
    if (parse_result_code == 0) { // If parse_san was successful (returns 0)
      move_outcome = make_move(board_state, move_data);
      if (move_outcome != -1) { // If make_move was successful (doesn't return -1)
        printf("\n");
        printf(buf_0); // Print the move entered by the user
        
        // Reduced intermediate variable uVar3 by directly calling calculate_csum
        printf("\n%x\n", calculate_csum(move_outcome));
        
        if (move_outcome == 3) {
          printf("STALEMATE!\n");
          init_flag = 1; // Set flag to re-initialize for a new game
        } else if (move_outcome < 4) { // Check for Check (1) or Checkmate (2)
          if (move_outcome == 1) {
            printf("CHECK!\n");
          } else if (move_outcome == 2) {
            printf("CHECKMATE!\n");
            init_flag = 1; // Set flag to re-initialize for a new game
          }
        }
        player_turn = (player_turn == 0); // Toggle player turn (0 to 1, 1 to 0)
      } else {
        printf("INVALID MOVE!\n"); // make_move returned -1
      }
    } else {
      printf("INVALID MOVE!\n"); // parse_san returned non-zero
    }
  }
  return 0;
}