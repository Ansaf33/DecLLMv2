#include <stdio.h>    // For printf, snprintf, fprintf
#include <string.h>   // For strlen, strtok
#include <stdlib.h>   // For exit, malloc (implied by allocate)
#include <stdint.h>   // For uint8_t, uint16_t
#include <stdbool.h>  // For bool

// --- External Function Declarations ---
// These functions are not provided in the snippet, so they are declared as extern.
// Their exact signatures are inferred from their usage in the provided code.

// prompt_user: Reads user input. The second argument's type varies in the original snippet.
// Using `char*` as a generic buffer, the caller is responsible for ensuring type compatibility.
extern void prompt_user(const char *prompt_str, char *buffer);

// Game state management structure
typedef struct {
    // These fields are inferred from the decompiled code's access patterns.
    // The exact layout and types are speculative without the full source.
    void *board_data;    // Placeholder for pointer to actual board data
    uint8_t *scores_ptr; // Placeholder for pointer to wins/losses array (e.g., uint8_t[2])
    // Add other fields as necessary if more specific game logic is known.
} GameState;

// Global pointer to the game state. This is accessed by all game logic functions.
static GameState *game_ctx = NULL;

// Helper functions for score access, to abstract the complex `game[1]` access
// seen in the original decompiled code. These assume `game_ctx->scores_ptr`
// points to `uint8_t wins` followed by `uint8_t losses`.
uint8_t get_wins(void *game_ptr) {
    return ((GameState*)game_ptr)->scores_ptr[0];
}

uint8_t get_losses(void *game_ptr) {
    return ((GameState*)game_ptr)->scores_ptr[1];
}

extern void set_player_chars(void *game_ptr, int player_char);
extern void make_board(void *game_ptr, char *buffer);
extern int is_players_turn(void *game_ptr);
extern void computer_move(void *game_ptr, uint16_t *move_coords);
extern uint8_t str2int(const char *s); // Parses a number from a string.
extern int update_board(void *game_ptr, uint16_t *move_coords);
extern void move_complete(void *game_ptr);
extern int have_winner(void *game_ptr);
extern int have_moves_remaining(void *game_ptr);
extern void update_score(void *game_ptr);
extern char is_player_winner(void *game_ptr); // Returns '\0' for loss, non-'\0' for win.
extern void init_game_data(void *game_ptr);
// `streq` returns 0 if strings are equal, non-zero otherwise (like `strcmp == 0`).
// The original decompiled code's `if (iVar4 != 0) break;` for equality implies
// `streq` was inverted `strcmp`, but standard `streq` usually means equality.
// We assume `streq` means `strcmp == 0`.
extern int streq(const char *s1, const char *s2);
extern void reset_scores(void *game_ptr); // Takes game_ptr to reset scores within.

// `allocate_game_state`: Allocates memory for GameState and initializes `game_ctx`
// and its internal pointers (e.g., `board_data`, `scores_ptr`).
// Returns 0 on success, 1 on failure.
extern int allocate_game_state(void);

// `_terminate` function. Original suggests program exit upon a fatal error.
void _terminate(void) {
    fprintf(stderr, "Fatal error: Game initialization failed.\n");
    exit(1);
}

// --- Global Data (Inferred from DAT_xxxxxx addresses) ---
// These are likely string literals used as prompts or comparison strings.
extern const char DAT_000140cc[]; // E.g., a generic prompt string or empty string.
extern const char DAT_0001419b[]; // String literal for "PLAY".
extern const char DAT_000141b6[]; // String literal for "QUIT".

// --- Function Implementations ---

// Function: do_menu
void do_menu(int param_1) {
  if (param_1 == 0) { // First game menu
    printf("Make your selection:\nPLAY\nQUIT\n");
  } else {          // Subsequent game menu (after a game has been played)
    printf("Make your selection:\nPLAY AGAIN\nSTART OVER\nQUIT\n");
  }
}

// Function: do_quit
void do_quit(void) {
  printf("Give up?. Don't be a stranger!\n");
}

// Function: do_select_char
void do_select_char(void) {
  char selected_char = '\0';
  char input_buffer[2]; // Buffer to read a single character + null terminator

  while (selected_char == '\0') {
    printf("P goes first. Do you want P or Q?\n");
    // DAT_000140cc is used as a prompt string for prompt_user.
    prompt_user(DAT_000140cc, input_buffer); 

    if (input_buffer[0] == 'P') {
      selected_char = 'P';
    } else if (input_buffer[0] == 'Q') {
      selected_char = 'Q';
    } else {
      printf("Wat?\n");
    }
  }
  set_player_chars(game_ctx, selected_char);
}

// Function: send_current_board
void send_current_board(void) {
  char board_buffer[256]; // Sufficiently large buffer for board string representation

  // `make_board` is assumed to fill `board_buffer` with a null-terminated string.
  make_board(game_ctx, board_buffer);
  printf("%s", board_buffer);
}

// Function: make_move
void make_move(void) {
  uint16_t move_coords = 0; // Represents row and col, e.g., (col << 8) | row
  char input_buffer[16];    // Buffer for "row col" input, e.g., "1 2"
  int update_result;

  if (is_players_turn(game_ctx) == 0) { // Computer's turn
    computer_move(game_ctx, &move_coords);
    // Original format for printing: (low byte, row) and (high byte, col)
    printf("Computer's move: %hhu %hhu.\n", (uint8_t)move_coords, (uint8_t)(move_coords >> 8));
  } else { // Player's turn
    printf("It's your move. Enter 'row col'\n");
    prompt_user(DAT_000140cc, input_buffer); // DAT_000140cc is likely a prompt string.

    uint8_t row = 0, col = 0;
    char *token = strtok(input_buffer, " "); // Parse the first number (row)
    if (token) {
        row = str2int(token);
        token = strtok(NULL, " "); // Parse the second number (col)
        if (token) {
            col = str2int(token);
        }
    }
    move_coords = (col << 8) | row; // Store (col, row) in a single 16-bit value
  }

  update_result = update_board(game_ctx, &move_coords);
  if (update_result == 0) { // Move was successful
    move_complete(game_ctx);
  } else { // Invalid move
    if (is_players_turn(game_ctx) != 0) { // If it's still the player's turn (i.e., invalid move)
      printf("Invalid coordinates!\n");
    }
  }
}

// Function: send_game_results
void send_game_results(void) {
  if (is_player_winner(game_ctx) == '\0') {
    printf("Nice effort. Maybe next time.\n");
  } else {
    printf("Congratulations! You Win!\n");
  }

  char score_message[64]; // Buffer for the score message
  snprintf(score_message, sizeof(score_message), "%hhu wins. %hhu losses.\n",
           get_wins(game_ctx), get_losses(game_ctx));
  printf("%s", score_message);
}

// Function: do_play
void do_play(void) {
  do_select_char();
  printf("Game on!\n");

  do {
    make_move();
    if (have_winner(game_ctx) != 0) break; // Exit loop if there's a winner
  } while (have_moves_remaining(game_ctx) != 0); // Continue if moves are still available

  update_score(game_ctx);
  send_game_results();
}

// Function: main
int main(void) {
  bool game_played_once = false; // Flag to track if a game has been played
  char user_input[14];           // Buffer for user input (e.g., "PLAY", "QUIT")
  bool running = true;           // Main loop control flag
  
  printf("Welcome to Tick-A-Tack.\n");

  while (running) {
    // Game state allocation and initial setup on the first run.
    if (game_ctx == NULL) {
        if (allocate_game_state() != 0) { // Assumed to allocate and initialize `game_ctx` globally.
            _terminate(); // Handle allocation failure.
        }
    }

    // Always initialize game data for a new round (board, current player, etc.)
    init_game_data(game_ctx);
    
    bool menu_processed = false; // Flag to control the inner menu selection loop
    while (!menu_processed) {
        do_menu(game_played_once);
        // DAT_000140cc is used as a prompt string for prompt_user.
        prompt_user(DAT_000140cc, user_input); // Get user selection

        if (streq(DAT_0001419b, user_input) == 0) { // User input is "PLAY"
            do_play();
            game_played_once = true;
            menu_processed = true; // Exit inner menu loop, continue outer loop for next round
        } else if (game_played_once && streq("PLAY AGAIN", user_input) == 0) { // User input is "PLAY AGAIN"
            do_play();
            menu_processed = true; // Exit inner menu loop, continue outer loop for next round
        } else if (game_played_once && streq("START OVER", user_input) == 0) { // User input is "START OVER"
            game_played_once = false;
            reset_scores(game_ctx);
            menu_processed = true; // Exit inner menu loop, continue outer loop for next round
        } else if (streq(DAT_000141b6, user_input) == 0) { // User input is "QUIT"
            do_quit();
            running = false;    // Set flag to exit the main game loop
            menu_processed = true; // Exit inner menu loop
        } else {
            printf("Wat?\n"); // Invalid input, stay in the menu loop to prompt again
        }
    }
  }
  
  // In a complete application, memory allocated for `game_ctx` and its internal
  // pointers (like `board_data`, `scores_ptr`) should be freed here.
  // This is omitted as the original snippet did not include explicit free calls.

  return 0;
}