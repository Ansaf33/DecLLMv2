#include <stdio.h>    // For sprintf, NULL, printf (for dummy functions), stderr
#include <stdlib.h>   // For malloc, free, exit
#include <string.h>   // For memset, strlen
#include <unistd.h>   // For read, write (used by dummy read_n_bytes/transmit_all)
#include <stdint.h>   // For fixed-width integer types if needed (e.g., uint8_t)

// --- Type definitions from decompiler (adjusted for standard C) ---
typedef unsigned char byte;
// undefined4 is often a 4-byte value, typically an int or unsigned int.
// Using unsigned int to be safe with raw memory interpretations.
typedef unsigned int undefined4;
typedef char undefined; // undefined is often a single byte, char is appropriate.

// --- Global variables from original snippet ---
int flag_index = 0;
// Dummy data for DAT_4347c000. It's assumed to be a large array of bytes.
// The original snippet accesses `(&DAT_4347c000)[index]`, which is equivalent to `DAT_4347c000[index]`.
// The maximum index used is `10 + flag_index`, where `flag_index` goes up to `99`.
// So an array size of at least 110 (10+99+1) is needed.
byte DAT_4347c000[120] = {
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A,
    0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14,
    0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E,
    0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
    0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32,
    0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C,
    0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
    0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50,
    0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A,
    0x5B, 0x5C, 0x5D, 0x5E, 0x5F, 0x60, 0x61, 0x62, 0x63, 0x64,
    0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E,
    0x6F, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
};

// --- Struct definitions ---

// Represents a single high score entry in a linked list.
typedef struct Score {
    char *name;         // Dynamically allocated string for the player's name
    int moves;          // Number of moves taken to complete the dungeon
    struct Score *next; // Pointer to the next score in the list
} Score;

// Represents the overall state and configuration of the game.
// This struct consolidates all the `local_xx` variables and parameters
// inferred from the decompiled code's usage.
typedef struct GameState {
    // Menu navigation characters (used in sendMenuInstruction and main loop)
    char menu_quit_char;
    char menu_play_char;
    char menu_instructions_char;
    char menu_highscores_char;

    // In-game movement characters (used in sendGameDescription, makeMove, playerDied)
    char game_move_left_char;
    char game_move_right_char;
    char game_move_jump_char;
    char game_move_jump_left_char;
    char game_move_jump_right_char;
    char game_move_wait_char;
    char game_quit_char; // Character to quit the game itself during play

    // Game data pointers (e.g., for dungeon map, high score list head)
    void *dungeon_data;               // Placeholder for dungeon map/state
    Score *high_scores_list_head; // Head of the linked list of high scores
} GameState;


// --- External function declarations (dummy implementations provided for compilation) ---

// Terminates the program with a given exit code.
void _terminate(int exit_code) {
    fprintf(stderr, "Program terminated with code %d\n", exit_code);
    exit(exit_code);
}

// Reads `count` bytes from `fd` into `buf`. Stores actual bytes read in `bytes_read_ptr`.
// Returns 0 on success, non-zero on error.
int read_n_bytes(int fd, char *buf, size_t count, int *bytes_read_ptr) {
    // Dummy implementation: only handles stdin (fd 0)
    if (fd != 0) {
        *bytes_read_ptr = 0;
        return -1; // Error for unsupported fd
    }
    ssize_t bytes_read = read(fd, buf, count);
    if (bytes_read == -1) {
        *bytes_read_ptr = 0;
        perror("read_n_bytes error");
        return -1; // Error reading
    }
    *bytes_read_ptr = (int)bytes_read;
    return 0; // Success
}

// Transmits `count` bytes from `buf` to `fd`.
// Returns 0 on success, non-zero on error.
int transmit_all(int fd, const char *buf, size_t count) {
    // Dummy implementation: only handles stdout (fd 1)
    if (fd != 1) {
        fprintf(stderr, "transmit_all only supports stdout in dummy implementation.\n");
        return -1; // Error for unsupported fd
    }
    size_t total_written = 0;
    while (total_written < count) {
        ssize_t written = write(fd, buf + total_written, count - total_written);
        if (written == -1) {
            perror("transmit_all error");
            return -1; // Error writing
        }
        total_written += written;
    }
    return 0; // Success
}

// Displays the current dungeon view to the player.
void sendCurrentDungeonView(void *dungeon_data) {
    printf("[DEBUG] Sending current dungeon view (data at %p)...\n", dungeon_data);
    // Dummy implementation: print a message
}

// Processes a player's move.
// Returns 1 for player won, 2 for player died, 0 for game continues.
int makeMove(GameState *game_state, char move_char) {
    printf("[DEBUG] Making move '%c' for game state at %p...\n", move_char, (void*)game_state);
    // Dummy implementation: always continue for now.
    // In a real game, this would update dungeon_data, player position, etc.
    // For testing, uncomment lines below to simulate win/lose conditions
    /*
    static int move_count = 0;
    move_count++;
    if (move_count % 5 == 0) return 1; // Simulate a win
    if (move_count % 7 == 0) return 2; // Simulate a death
    */
    return 0;
}

// Handles the event when the player dies.
void playerDied(char game_quit_char, char move_left, char move_right,
                char move_jump, char move_jump_left, char move_jump_right) {
    printf("[DEBUG] Player died! Game config: Quit:'%c', Left:'%c', Right:'%c', Jump:'%c', JumpLeft:'%c', JumpRight:'%c'\n",
           game_quit_char, move_left, move_right, move_jump, move_jump_left, move_jump_right);
    // Dummy implementation: print a message
}

// Handles the event when the player wins.
void playerWon(GameState *game_state) {
    printf("[DEBUG] Player won! Game state at %p.\n", (void*)game_state);
    // Dummy implementation: print a message
}

// Initializes the dungeon for a new game.
void buildDungeon(GameState *game_state) {
    printf("[DEBUG] Building dungeon for game state at %p...\n", (void*)game_state);
    // Dummy implementation: populate GameState with default/initial values
    game_state->menu_quit_char = 'q';
    game_state->menu_play_char = 'p';
    game_state->menu_instructions_char = 'i';
    game_state->menu_highscores_char = 'h';

    game_state->game_move_left_char = 'a';
    game_state->game_move_right_char = 'd';
    game_state->game_move_jump_char = 'w';
    game_state->game_move_jump_left_char = 'z';
    game_state->game_move_jump_right_char = 'x';
    game_state->game_move_wait_char = 's';
    game_state->game_quit_char = 'Q'; // Different char for quitting game vs menu

    game_state->dungeon_data = (void*)0xDEADBEEF; // Dummy address for dungeon data
    // high_scores_list_head is initialized by initScoreboard
}

// Cleans up resources associated with the dungeon.
void destroyDungeon(GameState *game_state) {
    printf("[DEBUG] Destroying dungeon for game state at %p...\n", (void*)game_state);
    // Dummy implementation: cleanup dungeon data
    game_state->dungeon_data = NULL;
}

// Inserts a new score into the sorted high scores linked list (lower moves are better).
// Returns the new head of the list.
Score *insertNewScore(Score *new_score, Score *head_score) {
    printf("[DEBUG] Inserting new score: %s (moves: %d)\n", new_score->name, new_score->moves);
    if (head_score == NULL || new_score->moves < head_score->moves) {
        new_score->next = head_score;
        return new_score;
    }
    Score *current = head_score;
    while (current->next != NULL && new_score->moves >= current->next->moves) {
        current = current->next;
    }
    new_score->next = current->next;
    current->next = new_score;
    return head_score;
}


// --- Original functions, refactored ---

// Function: getRandomName
// Generates a random 10-character name (A-Z or a-z) and returns it as a
// dynamically allocated string. The string is null-terminated.
char *getRandomName(void) {
  char *name_buf = (char *)malloc(11); // 10 chars + null terminator
  if (name_buf == NULL) {
    return NULL; // Return NULL if memory allocation fails
  }
  
  memset(name_buf, 0, 11); // Initialize buffer with zeros
  
  for (int i = 0; i < 10; i++) {
    // Determine if character should be uppercase or lowercase based on a bit in DAT_4347c000
    if ((DAT_4347c000[i + flag_index] & 1) == 0) {
      name_buf[i] = DAT_4347c000[i + flag_index] % 26 + 'A'; // Uppercase (A-Z)
    } else {
      name_buf[i] = DAT_4347c000[i + flag_index] % 26 + 'a'; // Lowercase (a-z)
    }
  }
  flag_index = (flag_index + 10) % 100; // Update global index for next call
  
  return name_buf;
}

// Function: getMove
// Reads a single character from standard input.
// Returns the character read, or '\0' if no character was read (e.g., EOF).
// Terminates the program on read errors.
char getMove(void) {
  char input_buffer[8]; // Buffer to hold input, only the first byte is relevant
  int bytes_read_count = 0; // Stores the number of bytes successfully read
  
  memset(input_buffer, 0, sizeof(input_buffer)); // Clear the buffer
  
  // Attempt to read 1 byte from file descriptor 0 (stdin)
  if (read_n_bytes(0, input_buffer, 1, &bytes_read_count) != 0) {
    _terminate(1); // Terminate if read_n_bytes indicates an error
  }
  
  if (bytes_read_count == 0) {
    return '\0'; // Return null character if no bytes were read (e.g., EOF or empty input)
  }
  
  return input_buffer[0]; // Return the first character read
}

// Function: sendGameDescription
// Sends a formatted string to stdout describing the available game moves.
// Replaces custom '!C' format specifiers with standard '%c'.
void sendGameDescription(char move_left, char move_right, char move_jump,
                         char move_jump_left, char move_jump_right, char move_wait,
                         char move_quit) {
  char buffer[1024]; // Buffer for the formatted string
  
  memset(buffer, 0, sizeof(buffer)); // Initialize buffer with zeros
  
  // Format the game description string
  sprintf(buffer,
          "Game moves\n----------\nLeft: %c\nRight: %c\nJump: %c\nJump Left: %c\nJump Right: %c\nWait: %c\nQuit game: %c\n",
          move_left, move_right, move_jump, move_jump_left, move_jump_right, move_wait, move_quit);
  
  size_t len = strlen(buffer); // Get the length of the formatted string
  // Transmit the string to stdout (fd 1)
  if (transmit_all(1, buffer, len) != 0) {
    _terminate(2); // Terminate if transmission fails
  }
}

// Function: sendMenuInstruction
// Sends a formatted string to stdout describing the main menu options.
// Replaces custom '!C' format specifiers with standard '%c'.
void sendMenuInstruction(char play_game_char, char get_instructions_char,
                         char high_scores_char, char quit_game_char) {
  char buffer[1024]; // Buffer for the formatted string
  
  memset(buffer, 0, sizeof(buffer)); // Initialize buffer with zeros
  
  // Format the menu instruction string
  sprintf(buffer,
          "Menu\n-----\nPlay game: %c\nGet instructions: %c\nHigh Scores: %c\nQuit game: %c\n",
          play_game_char, get_instructions_char, high_scores_char, quit_game_char);
  
  size_t len = strlen(buffer); // Get the length of the formatted string
  // Transmit the string to stdout (fd 1)
  if (transmit_all(1, buffer, len) != 0) {
    _terminate(2); // Terminate if transmission fails
  }
}

// Function: playGame
// Manages the main game loop, handling player input and game state transitions.
// Returns 1 if player won, 2 if player died, 3 if player quit mid-game.
int playGame(GameState *game_state) {
  sendCurrentDungeonView(game_state->dungeon_data); // Display the initial dungeon state
  
  while (1) { // Infinite loop for game play, broken by return statements
    char move_char = getMove(); // Get player's move input
    
    if (move_char == game_state->game_quit_char) {
      return 3; // Player chose to quit the game
    }
    
    int move_result = makeMove(game_state, move_char); // Process the player's move
    
    if (move_result == 1) { // Player won the game
      playerWon(game_state);
      return 1;
    }
    if (move_result == 2) { // Player died in the game
      // Call playerDied with relevant game configuration characters
      playerDied(game_state->game_quit_char, game_state->game_move_left_char,
                 game_state->game_move_right_char, game_state->game_move_jump_char,
                 game_state->game_move_jump_left_char, game_state->game_move_jump_right_char);
      return 2;
    }
    // If move_result is 0, the game continues, and the loop reiterates.
  }
}

// Function: sendHighScores
// Displays the current high scores list to stdout.
void sendHighScores(Score *head_score) {
  char buffer[61]; // Buffer for formatting individual score entries (0x3d = 61 bytes)
  
  if (head_score == NULL) {
    const char *no_scores_msg = "NO HIGH SCORES!\n";
    // Transmit message directly if no scores exist
    if (transmit_all(1, no_scores_msg, strlen(no_scores_msg)) != 0) {
      _terminate(2);
    }
    return;
  }

  // Display the top score in a special format
  memset(buffer, 0, sizeof(buffer));
  // Replaced custom '!U' with '%u' (unsigned int) and '!X' with '%s' (string)
  sprintf(buffer, "Dungeon conquered in %u moves %s\n", head_score->moves, head_score->name);
  if (transmit_all(1, buffer, strlen(buffer)) != 0) {
    _terminate(2);
  }

  // Display a header for the high scores table
  const char *header_msg = "\n-------------------\n moves   |   name  \n-------------------\n";
  if (transmit_all(1, header_msg, strlen(header_msg)) != 0) {
    _terminate(2);
  }

  // Iterate through the linked list of scores and display each one
  int rank = 1; // Start rank from 1
  Score *current_score = head_score;
  while (current_score != NULL) {
    memset(buffer, 0, sizeof(buffer));
    // Format each score entry with rank, moves, and name
    sprintf(buffer, "%2d. %5u  %s\n", rank, current_score->moves, current_score->name);
    if (transmit_all(1, buffer, strlen(buffer)) != 0) {
      _terminate(2);
    }
    rank++;
    current_score = current_score->next; // Move to the next score
  }
}

// Function: initScoreboard
// Initializes the game's high scoreboard with a few predefined dummy scores.
void initScoreboard(GameState *game_state) {
  Score *new_score;

  // Create and insert the first dummy score
  new_score = (Score *)malloc(sizeof(Score));
  if (new_score == NULL) {
    _terminate(1); // Terminate if malloc fails
  }
  memset(new_score, 0, sizeof(Score)); // Initialize allocated memory
  new_score->name = getRandomName();   // Get a random name for the score
  if (new_score->name == NULL) { // Check if getRandomName failed to allocate memory
      free(new_score); // Free the Score struct itself
      _terminate(1);
  }
  new_score->moves = 600;
  new_score->next = NULL;
  game_state->high_scores_list_head = insertNewScore(new_score, game_state->high_scores_list_head);

  // Create and insert the second dummy score
  new_score = (Score *)malloc(sizeof(Score));
  if (new_score == NULL) {
    _terminate(1);
  }
  memset(new_score, 0, sizeof(Score));
  new_score->name = getRandomName();
  if (new_score->name == NULL) {
      free(new_score);
      _terminate(1);
  }
  new_score->moves = 0x259; // 601 in decimal
  new_score->next = NULL;
  game_state->high_scores_list_head = insertNewScore(new_score, game_state->high_scores_list_head);

  // Create and insert the third dummy score
  new_score = (Score *)malloc(sizeof(Score));
  if (new_score == NULL) {
    _terminate(1);
  }
  memset(new_score, 0, sizeof(Score));
  new_score->name = getRandomName();
  if (new_score->name == NULL) {
      free(new_score);
      _terminate(1);
  }
  new_score->moves = 999999; // A very high score (bad)
  new_score->next = NULL;
  game_state->high_scores_list_head = insertNewScore(new_score, game_state->high_scores_list_head);
}

// Function: main
// Entry point of the program. Manages the main menu and game flow.
int main(void) {
  // Declare a single GameState struct to hold all game configuration and dynamic data.
  // This replaces the multiple `local_xx` undefined4 variables from the original snippet.
  GameState game_state;
  memset(&game_state, 0, sizeof(GameState)); // Initialize the struct's memory to zeros

  // Initialize the dungeon and game configuration
  buildDungeon(&game_state);
  // Initialize the scoreboard and populate it with initial scores
  initScoreboard(&game_state);

  while (1) { // Main menu loop, continues until the player chooses to quit
    // Display menu instructions using characters from the game_state config
    sendMenuInstruction(game_state.menu_play_char, game_state.menu_instructions_char,
                        game_state.menu_highscores_char, game_state.menu_quit_char);
    
    char menu_choice = getMove(); // Get player's menu choice

    if (menu_choice == game_state.menu_play_char) {
      // Player chose to play the game
      int game_result = playGame(&game_state);
      if (game_result == 3) { // Player quit the game mid-play
        // No action needed, loop back to main menu
      } else { // Game ended (player won or died)
        destroyDungeon(&game_state); // Clean up current dungeon resources
        buildDungeon(&game_state);   // Build a new dungeon for the next game
      }
    } else if (menu_choice == game_state.menu_instructions_char) {
      // Player chose to view game instructions/description
      sendGameDescription(game_state.game_move_left_char, game_state.game_move_right_char,
                          game_state.game_move_jump_char, game_state.game_move_jump_left_char,
                          game_state.game_move_jump_right_char, game_state.game_move_wait_char,
                          game_state.game_quit_char);
    } else if (menu_choice == game_state.menu_highscores_char) {
      // Player chose to view high scores
      sendHighScores(game_state.high_scores_list_head);
    } else if (menu_choice == game_state.menu_quit_char) {
      // Player chose to quit the program
      
      // Free memory allocated for score names and Score structs
      Score *current_score = game_state.high_scores_list_head;
      while (current_score != NULL) {
          Score *next_score = current_score->next;
          free(current_score->name); // Free the name string
          free(current_score);       // Free the Score struct
          current_score = next_score;
      }
      // Clean up dungeon resources, if any were dynamically allocated
      destroyDungeon(&game_state); 
      return 0; // Exit successfully
    }
    // If an invalid choice is made, the loop continues, displaying the menu again.
  }
  // This line should theoretically not be reached as the loop always returns or terminates.
  return 0;
}