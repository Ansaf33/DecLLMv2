#include <stdio.h>   // For printf, stdin, fgetc, ferror, fprintf
#include <stdlib.h>  // For calloc, malloc, free, strtol
#include <string.h>  // For memset, strcpy
#include <stdbool.h> // For bool type

// Define a Player struct to represent player data
typedef struct Player {
    char name[24]; // Name of the player
    void (*flair_func)(void); // Pointer to a flair function
    int current_score;      // Player's current score
    int ttt_wins;           // Three in a row wins
    int rps_wins;           // Hammer, Sheet, Sheers wins
    int hi_lo_wins;         // Guess my number wins
    int ttt_draws;          // Three in a row draws
    int flair_set;          // Flag indicating if flair is set (boolean)
    void (*print_stats_func)(struct Player*); // Pointer to print stats function
} Player;

// Define a minimal Board struct for ttt
typedef struct Board {
    // Other board data would go here
    int game_state; // At offset 0x10 in the original code's interpretation
} Board;

// --- Stub Functions for compilation ---
// These functions are not provided in the snippet and are necessary for compilation.
// They provide minimal functionality to allow the program to run.

Board* create_board() {
    Board* board = (Board*)malloc(sizeof(Board));
    if (board) {
        board->game_state = 0; // 0 means game is ongoing
        printf("[Board created]\n");
    }
    return board;
}

void print_board(Board* board) {
    if (board) {
        printf("[Board printed, current state: %d]\n", board->game_state);
    }
}

int player_move(Board* board) {
    printf("Player makes a move...\n");
    // Simulate a valid move after one retry
    static int player_move_tries = 0;
    player_move_tries++;
    if (player_move_tries % 2 == 0) {
        return 1; // Valid move
    }
    return 0; // Invalid move, try again
}

int computer_move(Board* board) {
    printf("Computer makes a move...\n");
    // Simulate a valid move after one retry
    static int computer_move_tries = 0;
    computer_move_tries++;
    if (computer_move_tries % 2 == 0) {
        return 1; // Valid move
    }
    return 0; // Invalid move, try again
}

int play_rps() {
    printf("Playing Hammer, Sheet, Sheers...\n");
    return 1000; // Simulate a win score
}

int play_hi_lo() {
    printf("Playing Guess my number...\n");
    return 500; // Simulate a win score
}

// Minimal freaduntil implementation
// Reads from stream into buffer until max_len-1 characters are read,
// delimiter is encountered, or EOF. Null-terminates the buffer.
int freaduntil(char *buffer, int max_len, char delimiter, FILE *stream) {
    int i = 0;
    int c;
    while ((i < max_len - 1) && ((c = fgetc(stream)) != EOF)) {
        if (c == delimiter) {
            break;
        }
        buffer[i++] = (char)c;
    }
    buffer[i] = '\0';
    if (ferror(stream)) {
        return -1; // Error occurred
    }
    return i; // Number of characters read (excluding null terminator)
}

// --- Original Functions (fixed) ---

// Function: terminator
void terminator(void) {
  printf("The system goes online August 4th, 1997.\n");
  return;
}

// Function: hal
void hal(void) {
  printf("Dave, my mind is going. I can feel it... I can feel it\n");
  return;
}

// Function: collosus
void collosus(void) {
  printf("Where is Doctor Forbin?\n");
  return;
}

// Function: score_to_l_code
unsigned int score_to_l_code(unsigned int param_1) {
  return param_1 ^ 0xdeadbeef;
}

// Function: ttt
void ttt(Player* player, int player_turn_flag) {
  Board* board = create_board();
  if (!board) {
      fprintf(stderr, "Failed to create board for Three in a row.\n");
      return;
  }

  printf("You\'re playing 3 in a row. Good luck!\n");
  if (player_turn_flag == -0x40) { // Assuming -0x40 means display initial board
    print_board(board);
  }

  int move_result;
  while (board->game_state == 0) { // 0 means game is ongoing
    if (player_turn_flag == -0x40) { // Player's turn
      do {
        move_result = player_move(board);
      } while (move_result == 0); // Loop until a valid move is made
    }
    else if (player_turn_flag == 0x40) { // Computer's turn
      do {
        move_result = computer_move(board);
      } while (move_result == 0); // Loop until a valid move is made
    }
    printf("\n");
    print_board(board);
    player_turn_flag = -player_turn_flag; // Toggle turn
  }

  // Game over, check result
  if (board->game_state == 0x20) { // Tie
    player->ttt_draws++;
    printf("It\'s a tie!\n");
  }
  else if (board->game_state == 0x10) { // Player lost
    player->current_score = 0; // Reset score on loss
    printf("You lost.\n");
  }
  else if (board->game_state == 8) { // Player won
    player->ttt_wins++;
    player->current_score += 1000000;
    printf("You won!\n");
  }
  free(board); // Clean up board memory
  return;
}

// Function: rps
void rps(Player* player) {
  int game_result = play_rps();
  if (game_result == 1000) {
    player->rps_wins++;
  }
  player->current_score += game_result;
  return;
}

// Function: hi_lo
void hi_lo(Player* player) {
  int game_result = play_hi_lo();
  if (game_result != 0) {
    player->current_score += game_result;
    player->hi_lo_wins++;
  }
  return;
}

// Function: player_print_stats
void player_print_stats(Player* player) {
  printf("---%s\'s stats---\n", player->name);
  printf("Current Score: %d\n", player->current_score);
  printf("Number of Three in a row draws: %d\n", player->ttt_draws);
  printf("Number of Three in a row wins: %d\n", player->ttt_wins);
  printf("Number of Hammer, Sheet, Sheers wins: %d\n", player->rps_wins);
  printf("Number of Guess my number wins: %d\n", player->hi_lo_wins);
  printf("\n");
  return;
}

// Function: reset_player
void reset_player(Player* player) {
  // Clear the entire struct, which includes the name array and all integer fields.
  // The original code cleared 0x18 bytes then individually reset other fields.
  // Clearing the whole struct to zero covers all fields.
  memset(player, 0, sizeof(Player));
  return;
}

// Function: init_player
void init_player(Player* player) {
  char input_buffer[33]; // local_31
  long flair_choice; // local_10
  int read_status;

  reset_player(player);

  do {
    do {
      printf("Flair 1 = Terminator\n");
      printf("Flair 2 = 2001\n");
      printf("Flair 3 = Collosus\n");
      printf("Enter player information: [name,flair]\n");
      read_status = freaduntil(input_buffer, sizeof(input_buffer), ',', stdin);
      if (read_status == -1) {
          printf("Error reading name. Please try again.\n");
      }
    } while (read_status == -1);
    strcpy(player->name, input_buffer);

    do {
      read_status = freaduntil(input_buffer, sizeof(input_buffer), '\n', stdin); // Read flair choice
      if (read_status == -1) {
          printf("Error reading flair. Please try again.\n");
      }
    } while (read_status == -1);

    flair_choice = strtol(input_buffer, NULL, 10);

    if (flair_choice == 1) {
      player->flair_func = terminator;
      player->flair_set = 1;
    }
    else if (flair_choice == 2) {
      player->flair_func = hal;
      player->flair_set = 1;
    }
    else if (flair_choice == 3) {
      player->flair_func = collosus;
      player->flair_set = 1;
    } else {
        printf("Invalid flair choice. Please choose 1, 2, or 3.\n");
        player->flair_set = 0; // Ensure flair_set is 0 if choice is invalid
    }
  } while (player->flair_set == 0); // Loop until a valid flair is set

  if (player->print_stats_func == NULL) { // Check if not already set (e.g., by a default)
    player->print_stats_func = player_print_stats;
  }
  return;
}

// Function: main
int main(void) {
  Player *player = calloc(1, sizeof(Player)); // Allocate memory for player
  char *input_buffer = malloc(0x100);       // Buffer for user input
  int max_input_len = 0x100;                 // Max length for input buffer

  bool exit_program = false;     // Flag to exit the main loop and program
  bool player_registered = false; // Flag for player registration status
  int ttt_turn_flag = -0x40;    // Initial turn flag for Three in a row (-0x40 or 0x40)
  unsigned int l_code = 0;       // Stores the l-code if brute-forced

  if (!player || !input_buffer) {
    fprintf(stderr, "Memory allocation failed. Exiting.\n");
    return 1;
  }

  while (!exit_program) {
    if (!player_registered) {
      // Main menu: Register New Player or Exit
      long choice = 0;
      int input_valid = 0;
      while (!input_valid) {
        printf("1. Register New Player\n");
        printf("2. Exit\n");
        printf("> ");
        if (freaduntil(input_buffer, max_input_len, '\n', stdin) != -1) {
          choice = strtol(input_buffer, NULL, 10);
          input_valid = 1; // Input successfully read and converted
        } else {
          printf("Invalid input. Please try again.\n");
        }
      }

      if (choice == 1) {
        init_player(player);
        player_registered = true;
        printf("\nWelcome to WarGames, %s. It\'s just you vs. me\n", player->name);
        ttt_turn_flag = -0x40; // Reset TTT turn flag for a new player
      } else if (choice == 2) {
        exit_program = true;
      } else {
        printf("Invalid choice. Please enter 1 or 2.\n");
      }
    } else {
      // Player is registered, show game menu
      player->print_stats_func(player);
      printf("Choose a game to play:\n");
      printf("--Easy--\n");
      printf("1. Hammer, Sheet, Sheers\n");
      printf("--Medium--\n");
      printf("2. Guess my number\n");
      printf("--Impossible--\n");
      printf("3. Three in a row\n");
      printf("4. Give up\n\n");
      printf("Shall we play a game?\n");
      printf("Game # ");

      long game_choice = 0;
      int input_valid = 0;
      while (!input_valid) {
        if (freaduntil(input_buffer, max_input_len, '\n', stdin) != -1) {
          game_choice = strtol(input_buffer, NULL, 10);
          input_valid = 1;
        } else {
          printf("Invalid input. Please try again.\n");
        }
      }

      if (game_choice == 4) { // Give up option
        player_registered = false;
        printf("See you next time, %s\n", player->name);
      } else {
        bool game_choice_valid = true; // Flag for valid game choice
        if (game_choice == 1) {
          rps(player);
        } else if (game_choice == 2) {
          hi_lo(player);
        } else if (game_choice == 3) {
          ttt(player, ttt_turn_flag);
          ttt_turn_flag = -ttt_turn_flag; // Toggle turn for next TTT game
        } else {
          game_choice_valid = false;
          printf("Invalid Game\n");
        }

        // Win condition check (if a valid game was played)
        if (game_choice_valid && (player->ttt_draws > 100) && (player->rps_wins != 0) &&
            (player->hi_lo_wins != 0)) {
          printf("I\'ve been beat. I realize now - the only winning move is not to play\n");
          printf("What say you, %s?\n", player->name);
          if (l_code == 0) { // Calculate l-code only once
            l_code = score_to_l_code(player->current_score);
          }
          player->flair_func(); // Call the player's flair function
          printf("Just in time. I was able to brute force the l-code. l-code = %u\n", l_code);
        }
      }
    }
  }

  printf("Exited\n");
  free(player);
  free(input_buffer);
  return 0;
}