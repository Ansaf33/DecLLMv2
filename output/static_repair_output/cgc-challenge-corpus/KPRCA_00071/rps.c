#include <stdio.h>    // For printf, stdin, fgetc, fgets, EOF, fprintf
#include <stdlib.h>   // For malloc, free
#include <string.h>   // For strcmp, strlen

// Global variables (as implied by the decompiled code)
int idx = 0;
unsigned char secret[4096]; // Assuming 0x1000 is the size of the secret array

// Custom freaduntil function (mimicking typical decompiled behavior)
// Reads up to max_size-1 characters or until 'delimiter' is found.
// Replaces the delimiter (e.g., newline) with a null terminator if found.
// Returns the number of characters read (excluding null terminator), or -1 on error/EOF.
size_t freaduntil(char *buf, size_t max_size, int delimiter, FILE *stream) {
    if (max_size == 0) return 0;
    if (fgets(buf, max_size, stream) == NULL) {
        return (size_t)-1; // Error or EOF
    }

    size_t len = strlen(buf);
    // If the last character is the delimiter, remove it.
    if (len > 0 && buf[len - 1] == (char)delimiter) {
        buf[len - 1] = '\0';
        len--;
    }
    return len;
}

// Function: play_rps
int play_rps(void) {
  char *input_buffer;
  unsigned char computer_choice;
  int player_choice = 8; // Initialized to an invalid choice to enter the loop

  // Update idx and get computer's choice from secret array
  int current_secret_idx = idx % 4096;
  idx = current_secret_idx + 1;
  computer_choice = secret[current_secret_idx] % 3;

  input_buffer = (char *)malloc(256); // 0x100 is 256 bytes
  if (input_buffer == NULL) {
      fprintf(stderr, "Failed to allocate memory.\n");
      return 0;
  }

  while (player_choice == 8) { // Loop until valid input is received
    printf("Choose [HAMMER, SHEET, SHEERS]: ");
    if (freaduntil(input_buffer, 256, '\n', stdin) == (size_t)-1) { // 10 is ASCII for newline
        fprintf(stderr, "Error reading input or EOF.\n");
        free(input_buffer);
        return 0;
    }

    if (strcmp(input_buffer, "HAMMER") == 0) {
      player_choice = 0;
    } else if (strcmp(input_buffer, "SHEET") == 0) {
      player_choice = 1;
    } else if (strcmp(input_buffer, "SHEERS") == 0) {
      player_choice = 2;
    } else {
      player_choice = 8; // Invalid input, continue loop
    }
  }
  free(input_buffer);

  // Array of choice names for cleaner printing
  const char *choices[] = {"HAMMER", "SHEET", "SHEERS"};

  printf("%s v %s\n", choices[player_choice], choices[computer_choice]);

  // Determine outcome using modular arithmetic
  if (player_choice == computer_choice) {
    printf("It\'s a tie!\n");
    return 0;
  } else if (player_choice == (computer_choice + 1) % 3) {
    printf("You Win!\n");
    return 1000;
  } else { // player_choice == (computer_choice + 2) % 3 (Player Loses)
    printf("You Lose.\n");
    return 0;
  }
}

// A simple main function to make the code compilable and runnable for testing
int main() {
    // Initialize secret array with some dummy data for demonstration
    // In a real scenario, this would likely be populated with genuinely random or derived data.
    for (int i = 0; i < 4096; ++i) {
        secret[i] = (unsigned char)(i % 5); // Example: a simple pattern
    }

    printf("Welcome to Rock-Paper-Scissors!\n");

    int total_score = 0;
    // Play a few rounds for demonstration
    for (int i = 0; i < 3; ++i) {
        printf("\n--- Round %d ---\n", i + 1);
        total_score += play_rps();
        printf("Current total score: %d\n", total_score);
    }

    printf("\nGame Over. Final score: %d\n", total_score);

    return 0;
}