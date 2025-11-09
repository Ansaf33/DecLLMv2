#include <stdio.h>   // For printf, stdin, fgets
#include <stdlib.h>  // For malloc, free, strtol
#include <math.h>    // For log2f, roundf
#include <string.h>  // For strcspn (to remove newline from fgets)

// Define uint as unsigned int
typedef unsigned int uint;

// Global variables as implied by the snippet
// Initialized for a compilable example.
uint secret[1024]; // 0x400 elements
int idx = 0;

// Function: set_bits_high
// Calculates 2^N - 1 where 2^N is the smallest power of 2 greater than or equal to value.
// If value is 0, returns 0.
uint set_bits_high(uint value) {
  if (value == 0) {
      return 0;
  }

  int bit_count = 0;
  // First loop: count bits (effectively floor(log2(value)) + 1)
  // This loop modifies 'value'.
  for (; value != 0; value >>= 1) {
    bit_count++;
  }

  // Second loop: create a bitmask of 'bit_count' ones
  // 'value' is 0 here from the previous loop.
  for (; bit_count != 0; bit_count--) {
    value = (value << 1) | 1;
  }
  return value;
}

// Function: get_random
uint get_random(uint *max_value_out) {
  char input_buffer[256]; // Use a fixed-size buffer on the stack, 0x100 = 256 bytes
  unsigned int current_max_value = 0x10000; 
  unsigned int random_value;                

  // Calculate index for 'secret' array and update global 'idx'
  random_value = secret[idx % 1024]; 
  idx = (idx % 1024) + 1;

  // Loop to get max value from user
  // The original condition (0xffff < (int)local_10) means if current_max_value > 0xffff (65535)
  // Since current_max_value starts at 0x10000 (65536), this loop runs at least once.
  while (0xffff < current_max_value) {
    printf("Enter max value (Default=%d): ", 0xffff);
    
    // Replace freaduntil with fgets and strtol
    if (fgets(input_buffer, sizeof(input_buffer), stdin) != NULL) {
      // Remove trailing newline character
      input_buffer[strcspn(input_buffer, "\n")] = 0;
      current_max_value = set_bits_high(strtol(input_buffer, NULL, 10));
    } else {
      // Handle EOF or error, e.g., break and use default
      printf("Error reading input, using default max value.\n");
      current_max_value = 0xffff; 
      break; 
    }
  }

  // Apply default if current_max_value became 0 (e.g., set_bits_high(0) -> 0)
  if (current_max_value == 0) {
    current_max_value = 0xffff;
  } else {
    printf("We've changed the max value to %d.\n", current_max_value);
    printf("Keep in mind only the games using the default max value are scored.\n");
  }

  // Apply bitwise AND if random_value exceeds current_max_value
  // This effectively masks random_value to the number of bits in current_max_value (if current_max_value is 2^N - 1)
  if (current_max_value < random_value) {
    random_value = random_value & current_max_value;
  }
  
  *max_value_out = current_max_value; // Return the chosen max value via pointer
  return random_value;               // Return the generated random number
}

// Function: play_hi_lo
int play_hi_lo(void) {
  const size_t buffer_size = 256; // 0x100 = 256
  char *input_buffer = (char *)malloc(buffer_size);
  if (input_buffer == NULL) {
      perror("Failed to allocate buffer");
      return -1; // Indicate an error
  }

  unsigned int max_random_value; // Stores the user-chosen max value (via get_random)
  int random_number = get_random(&max_random_value); // The actual number to guess

  // Calculate initial number of guesses based on max_random_value
  int remaining_guesses = (int)roundf(log2f((float)max_random_value) + 1.0f);
  
  // Game loop
  while (remaining_guesses > 0) {
    printf("Guess the number [%d guesses left]: ", remaining_guesses);
    
    if (fgets(input_buffer, buffer_size, stdin) == NULL) {
      printf("Error reading input. Exiting game.\n");
      free(input_buffer);
      return 0; // Lost
    }
    // Remove trailing newline character
    input_buffer[strcspn(input_buffer, "\n")] = 0;
    long user_guess_val = strtol(input_buffer, NULL, 10);

    if (user_guess_val < random_number) {
      printf("Too Low\n");
    } else if (user_guess_val > random_number) {
      printf("Too High\n");
    } else { // user_guess_val == random_number
      printf("You won!\n");
      free(input_buffer);
      if (max_random_value == 0xffff) {
        return 1000; // Scored game
      }
      return 0; // Won, but not scored
    }
    remaining_guesses--;
  }

  // If loop finishes, player ran out of guesses
  printf("Sorry, you lost. Try practicing with a lower max value.\n");
  free(input_buffer);
  return 0; // Lost
}

// Main function to make it a complete compilable program
int main() {
    // Initialize secret array with some dummy values for demonstration
    for (int i = 0; i < 1024; ++i) {
        secret[i] = i * 13 + 7; // Simple linear progression
    }
    // Ensure idx starts from 0 for the first call
    idx = 0;

    printf("Welcome to the Hi-Lo game!\n");
    int score = play_hi_lo();
    if (score == 1000) {
        printf("Congratulations, you scored 1000 points!\n");
    } else if (score == -1) {
        printf("An error occurred during the game.\n");
    } else {
        printf("Game over. Your score: %d\n", score);
    }
    return 0;
}