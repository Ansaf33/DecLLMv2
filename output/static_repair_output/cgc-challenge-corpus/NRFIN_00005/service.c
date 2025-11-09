#include <stdio.h>    // For printf, fputs, sscanf, snprintf
#include <stdlib.h>   // For malloc, free, exit, atoi
#include <string.h>   // For strlen, strcmp, memset
#include <unistd.h>   // For read (used in prompt_user)

// Dummy global variable for game state
// The original code implies 'game' is an int** where:
// game[0] points to the board data (char[9])
// game[1] points to the scores data (char[2])
int **game_state_ptr = NULL;

// --- Helper Functions (replacing decompiler artifacts and simplifying I/O) ---

// Replaces the problematic send calls. Prints string to stdout.
void send_str(const char *s) {
    fputs(s, stdout);
    fflush(stdout); // Ensure output is flushed immediately
}

// Replaces prompt_user. Prints prompt, reads from stdin.
void prompt_user(const char* prompt, char* buffer, size_t max_len) {
    fputs(prompt, stdout);
    fflush(stdout);
    if (fgets(buffer, max_len + 1, stdin) == NULL) {
        // Handle EOF or error, e.g., exit or clear error state
        if (feof(stdin)) {
            exit(0); // Exit on EOF
        }
        // Clear error if any
        clearerr(stdin);
        buffer[0] = '\0'; // Ensure buffer is empty
    } else {
        // Remove trailing newline character if present
        buffer[strcspn(buffer, "\n")] = 0;
    }
}

// Replaces streq. Returns 1 if strings are equal, 0 otherwise.
int streq(const char* s1, const char* s2) {
    return strcmp(s1, s2) == 0;
}

// Replaces str2int. Converts string to integer.
int str2int(const char* s) {
    return atoi(s);
}

// Replaces _terminate. Exits the program with an error.
void terminate(void) {
    fprintf(stderr, "Fatal error or termination condition met.\n");
    exit(1);
}

// --- Dummy Game Logic Functions (to make code compilable) ---
// These functions provide minimal implementations or placeholders
// to allow the provided snippet to compile and run its control flow.

// Dummy allocate function
// Allocates a block of memory for the int** array, board, and scores.
// Assumes 32-bit pointers (sizeof(int*) == 4) for the 0x15 offset.
// Total size: 2 * sizeof(int*) + 9 chars (board) + 2 chars (scores) + 2 bytes padding = 21 bytes.
int** allocate(void) {
    size_t total_size = (2 * sizeof(int*)) + (9 * sizeof(char)) + (2 * sizeof(char));
    // Pad to 21 bytes for 0x15 offset if total_size is less.
    if (total_size < 21) total_size = 21; 

    int** ptr_array = (int**)malloc(total_size);
    if (ptr_array == NULL) {
        perror("Failed to allocate game state");
        return NULL;
    }
    
    // Set up the pointers within the allocated block as per original decompiler's logic
    // game_state_ptr[0] points to the board (char[9])
    // game_state_ptr[1] points to the scores (char[2])
    ptr_array[0] = (int*)((char*)ptr_array + (2 * sizeof(int*))); // Board starts after the pointer array
    ptr_array[1] = (int*)((char*)ptr_array + 0x15);               // Scores start at offset 0x15 (21 bytes)

    return ptr_array;
}

// Dummy init_game_data
void init_game_data(void) {
    if (game_state_ptr && (char*)game_state_ptr[0]) {
        // Initialize 3x3 board with spaces
        memset((char*)game_state_ptr[0], ' ', 9);
    }
}

// Dummy reset_scores
void reset_scores(void) {
    if (game_state_ptr && (char*)game_state_ptr[1]) {
        memset((char*)game_state_ptr[1], 0, 2); // Reset player_wins and computer_wins to 0
    }
}

// Dummy set_player_chars
void set_player_chars(int** game_ptr, char player_char) {
    // In a real game, this would set player and computer characters in game_ptr[0] or a dedicated struct
    fprintf(stderr, "DEBUG: Player selected char: %c\n", player_char);
}

// Dummy make_board
void make_board(char* board, char* buffer) {
    // A simple 3x3 tic-tac-toe board representation
    snprintf(buffer, 256,
             "\n %c | %c | %c \n"
             "---+---+---\n"
             " %c | %c | %c \n"
             "---+---+---\n"
             " %c | %c | %c \n",
             board[0], board[1], board[2],
             board[3], board[4], board[5],
             board[6], board[7], board[8]);
}

// Dummy is_players_turn
int is_players_turn(char* board) {
    // Placeholder: alternate turns, starting with player
    static int turn_counter = 0;
    turn_counter++;
    return (turn_counter % 2 != 0); // Player's turn if odd
}

// Dummy computer_move
void computer_move(char* board, char* row, char* col) {
    // Simple placeholder: find the first empty spot
    for (int i = 0; i < 9; i++) {
        if (board[i] == ' ') {
            *row = (char)(i / 3);
            *col = (char)(i % 3);
            board[i] = 'O'; // Computer plays 'O'
            return;
        }
    }
    *row = -1; *col = -1; // No move possible
}

// Dummy update_board
int update_board(char* board, char row, char col) {
    if (row < 0 || row >= 3 || col < 0 || col >= 3) return 1; // Invalid coordinates
    int index = row * 3 + col;
    if (board[index] != ' ') return 1; // Spot already taken
    
    // Determine current player's char (assuming X for player, O for computer)
    // This logic should be more robust in a real game (e.g., stored in game_state_ptr)
    static int player_turn = 1; // 1 for player, 0 for computer
    board[index] = player_turn ? 'X' : 'O';
    player_turn = !player_turn; // Switch turn
    
    return 0; // Success
}

// Dummy move_complete
void move_complete(int** game_ptr) {
    // In a real game, this might switch turns, check for winner, etc.
    fprintf(stderr, "DEBUG: Move completed.\n");
}

// Dummy have_winner
int have_winner(char* board) {
    // Very basic check for a winner (horizontal, vertical, diagonal)
    // For X or O
    for (char p_char = 'X'; p_char <= 'O'; p_char++) { // Check for 'X' then 'O'
        // Rows
        for (int i = 0; i < 9; i += 3) {
            if (board[i] == p_char && board[i+1] == p_char && board[i+2] == p_char) return 1;
        }
        // Columns
        for (int i = 0; i < 3; i++) {
            if (board[i] == p_char && board[i+3] == p_char && board[i+6] == p_char) return 1;
        }
        // Diagonals
        if (board[0] == p_char && board[4] == p_char && board[8] == p_char) return 1;
        if (board[2] == p_char && board[4] == p_char && board[6] == p_char) return 1;
    }
    return 0; // No winner
}

// Dummy have_moves_remaining
int have_moves_remaining(char* board) {
    for (int i = 0; i < 9; i++) {
        if (board[i] == ' ') return 1; // Found an empty spot
    }
    return 0; // No empty spots left
}

// Dummy is_player_winner
int is_player_winner(char* board) {
    // Simplified: check if 'X' (player) is winner based on `have_winner`
    // In a real game, this would be more nuanced, checking last player's move.
    char temp_board[9];
    memcpy(temp_board, board, 9);
    // Temporarily replace 'O's with ' ' to check if only 'X' wins
    for(int i=0; i<9; ++i) if(temp_board[i] == 'O') temp_board[i] = ' ';
    return have_winner(temp_board); // If 'X' forms a line, player wins
}

// Dummy update_score
void update_score(int** game_ptr) {
    if (game_ptr && (char*)game_ptr[1]) {
        char* scores = (char*)game_ptr[1];
        if (is_player_winner((char*)game_ptr[0])) {
            scores[0]++; // Increment player wins
            fprintf(stderr, "DEBUG: Player score updated. Current: %d wins, %d losses.\n", scores[0], scores[1]);
        } else if (have_winner((char*)game_ptr[0])) { // If there's a winner, but not player, it's computer
            scores[1]++; // Increment computer wins (player losses)
            fprintf(stderr, "DEBUG: Computer score updated. Current: %d wins, %d losses.\n", scores[0], scores[1]);
        } else {
            fprintf(stderr, "DEBUG: Draw - no score updated.\n");
        }
    }
}

// --- Original Functions (fixed and refactored) ---

// Function: do_menu
void do_menu(int param_1) {
  const char* menu_str;
  
  if (param_1 == 0) {
    menu_str = "Make your selection:\nPLAY\nQUIT\n";
  } else {
    menu_str = "Make your selection:\nPLAY AGAIN\nSTART OVER\nQUIT\n";
  }
  send_str(menu_str);
}

// Function: do_quit
void do_quit(void) {
  send_str("Give up?. Don\'t be a stranger!\n");
}

// Function: do_select_char
void do_select_char(void) {
  char selected_char = '\0';
  char input_buffer[4]; // Enough for 'P', 'Q', '\n', '\0'

  while (selected_char == '\0') {
    send_str("P goes first. Do you want P or Q?\n");
    prompt_user("Enter P or Q: ", input_buffer, sizeof(input_buffer) - 1);
    
    char choice = input_buffer[0]; // Assuming single character input
    if (choice == 'P' || choice == 'p') {
      selected_char = 'P';
    } else if (choice == 'Q' || choice == 'q') {
      selected_char = 'Q';
    } else {
      send_str("Wat?\n");
      // The original code also had a longer error message, but "Wat?" is sufficient for this branch.
      // send_str("\nI don\'t think that means what you think it means.\n");
    }
  }
  set_player_chars(game_state_ptr, selected_char);
}

// Function: send_current_board
void send_current_board(void) {
  char board_buffer[256]; // Sufficiently large buffer for board representation
  make_board((char*)game_state_ptr[0], board_buffer);
  send_str(board_buffer);
}

// Function: make_move
void make_move(void) {
  char row_char = -1, col_char = -1; // For coordinates, initialized to invalid
  char input_buffer[10]; // For "row col" input (e.g., "1 2\n")

  int is_player_turn_flag = is_players_turn((char*)game_state_ptr[0]);
  if (is_player_turn_flag == 0) { // Computer's turn
    computer_move((char*)game_state_ptr[0], &row_char, &col_char);
    char computer_move_str[40];
    snprintf(computer_move_str, sizeof(computer_move_str), "Computer's move: %d %d.\n", (int)row_char, (int)col_char);
    send_str(computer_move_str);
  } else { // Player's turn
    send_str("It\'s your move. Enter \'row col\'\n");
    prompt_user("Enter row col: ", input_buffer, sizeof(input_buffer) - 1);
    
    // Parse two integers from input_buffer
    // Using %hhd for signed char (small integer)
    if (sscanf(input_buffer, "%hhd %hhd", &row_char, &col_char) != 2) {
        send_str("Invalid input format. Please enter two numbers.\n");
        // Set row_char/col_char to invalid values to ensure update_board fails
        row_char = -1; 
        col_char = -1;
    }
  }
  
  int update_result = update_board((char*)game_state_ptr[0], row_char, col_char);
  if (update_result == 0) {
    move_complete(game_state_ptr);
  } else {
    // Only show "Invalid coordinates!" if it was player's turn and the move failed
    if (is_player_turn_flag != 0) {
      send_str("Invalid coordinates or spot taken!\n");
    }
  }
}

// Function: send_game_results
void send_game_results(void) {
  int player_is_winner_flag = is_player_winner((char*)game_state_ptr[0]);
  if (player_is_winner_flag == 0) {
    send_str("Nice effort. Maybe next time.\n");
  } else {
    send_str("Congratulations! You Win!\n");
  }
  
  char score_msg[64];
  char* scores = (char*)game_state_ptr[1]; // game_state_ptr[1] points to char[2] for scores
  snprintf(score_msg, sizeof(score_msg), "%d wins. %d losses.\n", 
           (int)scores[0], (int)scores[1]);
  send_str(score_msg);
}

// Function: do_play
void do_play(void) {
  send_str("Game on!\n");
  do_select_char();
  
  int has_winner_flag;
  int moves_remaining_flag;
  do {
    send_current_board();
    make_move();
    has_winner_flag = have_winner((char*)game_state_ptr[0]);
    if (has_winner_flag != 0) break;
    moves_remaining_flag = have_moves_remaining((char*)game_state_ptr[0]);
  } while (moves_remaining_flag != 0);
  
  update_score(game_state_ptr);
  send_current_board(); // Display final board after game concludes
  send_game_results();
}

// Function: main
int main(void) {
  char user_input_buffer[32];
  int game_in_progress = 0; // Flag to indicate if a game has been played in current session

  send_str("Welcome to Tick-A-Tack.\n");

  game_state_ptr = allocate();
  if (game_state_ptr == NULL) {
      terminate(); // Handle allocation failure
  }
  // No need for explicit reset_scores() here if allocate() initializes them.
  // If allocate just allocates raw memory, then reset_scores() should be called.
  // Given `memset((char*)ptr_array[1], 0, 2);` in allocate, it's initialized.

  while (1) { // Outer game session loop
    init_game_data(); // Always initialize board for a new game round

    int menu_loop_continue = 1;
    while (menu_loop_continue) { // Inner menu loop
        do_menu(game_in_progress);
        prompt_user("Enter your choice: ", user_input_buffer, sizeof(user_input_buffer) - 1);

        if (streq(user_input_buffer, "PLAY")) {
            do_play();
            game_in_progress = 1;
            menu_loop_continue = 0; // Break inner loop, restart outer loop for a new game round
        } else if (game_in_progress && streq(user_input_buffer, "PLAY AGAIN")) {
            do_play();
            // game_in_progress remains 1
            menu_loop_continue = 0; // Break inner loop, restart outer loop for a new game round
        } else if (game_in_progress && streq(user_input_buffer, "START OVER")) {
            game_in_progress = 0;
            reset_scores();
            menu_loop_continue = 0; // Break inner loop, restart outer loop for a new game round
        } else if (streq(user_input_buffer, "QUIT")) {
            do_quit();
            // Free allocated memory before exiting
            free((char*)game_state_ptr[0]); // Board
            free((char*)game_state_ptr[1]); // Scores (might be part of the main block)
            free(game_state_ptr); // Main pointer array
            return 0; // Exit program
        } else {
            send_str("Wat?\n");
        }
    }
  }
  
  // This part of the code should not be reached as the loop is infinite or exits via return 0.
  return 0; 
}