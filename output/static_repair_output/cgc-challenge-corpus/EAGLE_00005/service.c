#include <stdio.h>   // For printf, getchar, fread, fopen, fclose, perror, stdin
#include <string.h>  // For strlen, strcpy, strchr, memmove
#include <stdlib.h>  // For exit
#include <ctype.h>   // For tolower, isalpha
#include <stdbool.h> // For bool

// --- Global variables (inferred from memory addresses and game logic) ---

// Hangman board display elements - actual lines of the ASCII art
static char g_board_line_0[80] = " _______     ";
static char g_board_line_1[80] = "|       |    ";
static char g_board_line_2[80] = "|            "; // Head 'O'
static char g_board_line_3[80] = "|            "; // Body '|'
static char g_board_line_4[80] = "|            "; // Arms '/' '\'
static char g_board_line_5[80] = "|            "; // Legs '/' '\'
static char g_board_line_6[80] = "|_____       ";

// Pointers to the specific characters within the board lines that change
// These are the places where hangman parts are drawn.
// Max 7 parts (0-6)
static char *g_hangman_part_locations[7] = {
    &g_board_line_2[2], // Part 0: Head
    &g_board_line_3[2], // Part 1: Body
    &g_board_line_4[1], // Part 2: Left Arm
    &g_board_line_4[3], // Part 3: Right Arm
    &g_board_line_5[1], // Part 4: Left Leg
    &g_board_line_5[3], // Part 5: Right Leg
    &g_board_line_2[3]  // Part 6: Noose (or other final part)
};

// Characters to draw for each part (corresponding to g_hangman_part_locations)
static const char g_part_draw_chars[7] = {'O', '|', '/', '\\', '/', '\\', '|'}; // Example parts

// Available letters for guessing (initially "abcdefghijklmnopqrstuvwxyz")
static char g_available_letters[27];

// The actual word chosen for the current game
static char g_current_word[64];

// The word shown to the player, with unguessed letters as underscores
static char g_revealed_word[64];

// Counter for incorrect guesses (max 7, 0-6)
static int g_incorrect_guesses;

// History of letters guessed in the current game
static char g_guessed_letters_history[27];

// Input buffer for the main menu
static char g_main_input_buffer[256];

// File pointer for reading random data (e.g., from /dev/urandom)
static FILE *g_random_source_fp = NULL;

// Word list for the game
static char *g_word_list[] = {
    "apple", "banana", "cherry", "date", "elderberry", "fig", "grape",
    "honeydew", "kiwi", "lemon", "mango", "nectarine", "orange", "papaya",
    "quince", "raspberry", "strawberry", "tangerine", "ugli", "vanilla",
    "watermelon", "xigua", "yam", "zucchini"
};
static const int g_word_count = sizeof(g_word_list) / sizeof(g_word_list[0]);

// --- String literals (inferred from original code's magic addresses) ---
static const char *STR_GET_HIGHSCORE_PROMPT1 = "Enter your name for the Hall of Fame: ";
static const char *STR_GET_HIGHSCORE_PROMPT2 = "Your score will be recorded.\n"; // Adjusted based on context
static const char *STR_GET_HIGHSCORE_FORMAT1 = "HOF entry: ";
static const char *STR_GET_HIGHSCORE_FORMAT3 = " added.\n";

static const char *STR_WIN_MESSAGE_PART1 = "Congratulations! You guessed the word: ";
static const char *STR_LOSE_MESSAGE_PART1 = "You lose! The word was: ";
static const char *STR_ALREADY_GUESSED = "Invalid input or letter already guessed. Please try again.\n";
static const char *STR_GUESS_PROMPT = "Guess a letter: ";
static const char *STR_AVAILABLE_LETTERS_PROMPT = "Available letters: %s\n";
static const char *STR_GUESSED_LETTERS_PROMPT = "Guessed letters: %s\n";
static const char *STR_ASK_YN_PROMPT = "Play again? (y/n): ";

static const char *STR_MAIN_PROMPT = "Welcome to Hangman!\nType 'play' to start, or 'hof' to view Hall of Fame: "; // Adjusted for clarity
static const char *STR_MAIN_HOF_CHECK_INPUT = "hof"; // Original was "er of the HOF: " which looks like a partial string
static const char *STR_MAIN_HOF_MESSAGE = "Hall of Fame functionality not implemented yet.\n"; // Original was "F: "
static const char *STR_MAIN_PLAY_AGAIN_MSG = "Another round? "; // Adjusted for clarity

// --- Helper function for terminating the program ---
void _terminate(void) {
    if (g_random_source_fp) {
        fclose(g_random_source_fp);
    }
    exit(1);
}

// Function: clearBoard
void clearBoard(void) {
  for (int i = 0; i < 7; i++) {
    // Clear the mutable part of each hangman line to a space
    *g_hangman_part_locations[i] = ' ';
  }
}

// Function: reset
void reset(void) {
  clearBoard();

  // Initialize available letters ('a' through 'z')
  for (int i = 0; i < 26; i++) {
    g_available_letters[i] = (char)i + 'a';
  }
  g_available_letters[26] = '\0'; // Null-terminate

  // Clear guessed letters history
  g_guessed_letters_history[0] = '\0';

  // Reset incorrect guesses counter
  g_incorrect_guesses = 0;

  // Clear current word and revealed word buffers
  g_current_word[0] = '\0';
  g_revealed_word[0] = '\0';
}

// Function: addPart
// Adds a part of the hangman figure based on the number of incorrect guesses.
// param_1 is the index of the part to add (0-6)
void addPart(int part_index) {
  if (part_index >= 0 && part_index < 7) {
    *g_hangman_part_locations[part_index] = g_part_draw_chars[part_index];
  }
}

// Function: printBoard
// Prints the current state of the hangman board.
void printBoard(void) {
  printf("%s\n", g_board_line_0);
  printf("%s\n", g_board_line_1);
  printf("%s\n", g_board_line_2);
  printf("%s\n", g_board_line_3);
  printf("%s\n", g_board_line_4);
  printf("%s\n", g_board_line_5);
  printf("%s\n", g_board_line_6);
}

// Function: getWord
// Selects a random word and initializes the revealed word.
void getWord(void) {
  // Ensure random source is open
  if (g_random_source_fp == NULL) {
    g_random_source_fp = fopen("/dev/urandom", "rb");
    if (g_random_source_fp == NULL) {
      perror("Error opening /dev/urandom");
      _terminate();
    }
  }

  unsigned int random_val;
  size_t bytes_read = fread(&random_val, sizeof(random_val), 1, g_random_source_fp);
  if (bytes_read != 1) {
    fprintf(stderr, "Error reading from random source.\n");
    _terminate();
  }

  // Select a random word from the list
  int word_index = random_val % g_word_count;
  strcpy(g_current_word, g_word_list[word_index]);

  // Initialize revealed word with underscores
  size_t word_len = strlen(g_current_word);
  for (size_t i = 0; i < word_len; i++) {
    g_revealed_word[i] = '_';
  }
  g_revealed_word[word_len] = '\0'; // Null-terminate
}

// Function: getHighScore
// Prompts user for a name and prints a placeholder message for Hall of Fame.
void getHighScore(void) {
  char name_buffer[32]; // Buffer for user's name
  char *newline_pos;

  printf(STR_GET_HIGHSCORE_PROMPT1);
  printf(STR_GET_HIGHSCORE_PROMPT2); // Placeholder text

  // Read input, cap at buffer size - 1 to prevent overflow
  if (fgets(name_buffer, sizeof(name_buffer), stdin) == NULL) {
    _terminate(); // Error reading input
  }

  // Remove newline character if present
  newline_pos = strchr(name_buffer, '\n');
  if (newline_pos != NULL) {
    *newline_pos = '\0';
  }

  // Print the entered name with a placeholder message
  printf(STR_GET_HIGHSCORE_FORMAT1);
  printf("%s", name_buffer);
  printf(STR_GET_HIGHSCORE_FORMAT3);
}

// Function: isValid
// Checks if a guessed character is valid (i.e., not already guessed and is a letter)
// If valid, removes it from the list of available letters.
bool isValid(char guess_char) {
  char *found_char_ptr;
  char lower_char = tolower(guess_char);

  // Check if it's an alphabet character
  if (!isalpha(lower_char)) {
      return false;
  }

  found_char_ptr = strchr(g_available_letters, lower_char);
  if (found_char_ptr == NULL) {
    return false; // Character not in available letters (already guessed or invalid)
  } else {
    // Remove the character from g_available_letters by shifting subsequent characters
    memmove(found_char_ptr, found_char_ptr + 1, strlen(found_char_ptr));
    return true; // Character was valid and removed
  }
}

// Function: makeGuess
// Processes a player's guess. Returns true if the game ends (win/lose), false otherwise.
bool makeGuess(char guess_char) {
  bool char_found_in_word = false;
  size_t word_len = strlen(g_current_word);
  char lower_guess_char = tolower(guess_char);

  // Check if the guessed character is in the current word
  for (size_t i = 0; i < word_len; i++) {
    if (lower_guess_char == tolower(g_current_word[i])) {
      g_revealed_word[i] = lower_guess_char;
      char_found_in_word = true;
    }
  }

  if (char_found_in_word) {
    // Check if the word is fully revealed (no underscores left)
    bool word_guessed = true;
    for (size_t i = 0; i < word_len; i++) {
      if (g_revealed_word[i] == '_') {
        word_guessed = false;
        break;
      }
    }

    if (word_guessed) {
      printf(STR_WIN_MESSAGE_PART1);
      printf("%s\n", g_current_word);
      getHighScore();
      return true; // Game won
    } else {
      return false; // Word not fully guessed yet, game continues
    }
  } else {
    // Incorrect guess
    if (g_incorrect_guesses < 7) { // Max 7 incorrect guesses (0-6)
      addPart(g_incorrect_guesses); // Add a part to the hangman figure
      g_incorrect_guesses++;
      
      if (g_incorrect_guesses == 7) {
        printBoard(); // Show final hangman figure
        printf(STR_LOSE_MESSAGE_PART1);
        printf("%s\n", g_current_word);
        return true; // Game lost
      }
    }
    return false; // Game continues
  }
}

// Function: askYN
// Asks a yes/no question and returns true for 'y', false for 'n'.
bool askYN(const char *prompt) {
  char response_char;
  int c;

  printf("%s", prompt);
  printf(STR_ASK_YN_PROMPT);

  response_char = (char)tolower(getchar()); // Read first char and convert to lowercase

  // Clear input buffer (read and discard remaining characters until newline or EOF)
  while ((c = getchar()) != '\n' && c != EOF) {}

  return response_char == 'y';
}

// Function: doTurn
// Executes one turn of the game. Returns true if the game ends, false otherwise.
bool doTurn(void) {
  char guess_char;
  int c; // For getchar

  printBoard();
  printf("\nWord: %s\n", g_revealed_word); // Show the current revealed word
  printf(STR_AVAILABLE_LETTERS_PROMPT, g_available_letters);
  printf(STR_GUESSED_LETTERS_PROMPT, g_guessed_letters_history);
  printf(STR_GUESS_PROMPT);

  c = getchar();
  guess_char = (char)c;

  // Read and discard remaining characters in the line
  int next_c = getchar();
  if (next_c != '\n' && next_c != EOF) {
    // If more than one character was typed before newline, it's considered invalid input
    while ((next_c = getchar()) != '\n' && next_c != EOF) {} // Clear rest of line
    printf(STR_ALREADY_GUESSED);
    return false; // Turn not completed successfully, game continues
  }

  if (isValid(guess_char)) {
    // Add to guessed letters history
    size_t history_len = strlen(g_guessed_letters_history);
    g_guessed_letters_history[history_len] = tolower(guess_char);
    g_guessed_letters_history[history_len + 1] = '\0';
    
    // Process the guess
    return makeGuess(guess_char); // Returns true if game won/lost, false if continues
  } else {
    printf(STR_ALREADY_GUESSED);
    return false; // Invalid guess, game continues
  }
}

// Function: playGame
// Manages a single game of Hangman.
void playGame(void) {
  bool game_over;

  reset();
  getWord();
  
  printf("The word has %zu letters.\n", strlen(g_current_word));

  do {
    game_over = doTurn(); // doTurn returns true if game is over (win/lose), false if continues
  } while (!game_over);
}

// Function: main
// Main entry point of the program.
int main(void) {
  char *newline_pos;

  // Open random source early, handle error gracefully if it fails
  g_random_source_fp = fopen("/dev/urandom", "rb");
  if (g_random_source_fp == NULL) {
      fprintf(stderr, "Warning: Could not open /dev/urandom. Game might not be truly random.\n");
      // Continue without random source or use a fallback if desired
  }

  printf(STR_MAIN_PROMPT);

  // Read user input into g_main_input_buffer
  if (fgets(g_main_input_buffer, sizeof(g_main_input_buffer), stdin) == NULL) {
    // Handle EOF or error on stdin
    if (g_random_source_fp) fclose(g_random_source_fp);
    return 1;
  }

  // Remove newline character
  newline_pos = strchr(g_main_input_buffer, '\n');
  if (newline_pos != NULL) {
    *newline_pos = '\0';
  }

  // Compare input for "hof" (Hall of Fame) or "play"
  if (strcmp(g_main_input_buffer, STR_MAIN_HOF_CHECK_INPUT) == 0) {
    printf(STR_MAIN_HOF_MESSAGE);
  } else if (strcmp(g_main_input_buffer, "play") == 0) { // Assuming 'play' is the other option
      do {
        playGame();
      } while (askYN(STR_MAIN_PLAY_AGAIN_MSG));
  } else {
      printf("Invalid option. Exiting.\n");
  }
  
  // Close random source if it was opened
  if (g_random_source_fp) fclose(g_random_source_fp);
  return 0;
}