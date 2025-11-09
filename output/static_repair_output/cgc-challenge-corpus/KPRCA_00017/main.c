#include <stdio.h>    // For dprintf, vdprintf (and potentially printf for fallback)
#include <stdlib.h>   // For exit, strtoul
#include <string.h>   // For memset, strcpy, strlen
#include <ctype.h>    // For isalpha, tolower
#include <unistd.h>   // For read
#include <stdarg.h>   // For va_list

// Global variables
int win = 0;
int total = 0;
const char *words[] = {
    "apple", "banana", "cherry", "date", "elderberry",
    "fig", "grape", "honeydew", "kiwi", "lemon",
    "mango", "nectarine", "orange", "papaya", "quince",
    "raspberry", "strawberry", "tangerine", "ugli", "vanilla"
};
const char *DAT_000140d9 = "%s"; // Assumed format string for banner

// Type definitions for decompiled types to standard C types
typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned long ulong;
typedef void (*code_ptr_void)(void); // Represents a function pointer with no args
typedef void (*code_ptr_int)(int);   // Represents a function pointer with an int arg

// Mock fdprintf function using vdprintf (GNU extension, common on Linux)
int fdprintf(int fd, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vdprintf(fd, format, args);
    va_end(args);
    return ret;
}

// Function: read_until
int read_until(int fd, char *buffer, uint max_len, char delimiter) {
  uint count = 0;
  char *current_ptr = buffer;
  ssize_t bytes_read;

  while (count < max_len) {
    bytes_read = read(fd, current_ptr, 1); // Read one byte
    if (bytes_read <= 0) { // EOF or error
      break;
    }
    // A character was successfully read into *current_ptr
    if (*current_ptr == delimiter) {
      break; // Delimiter found, stop reading
    }
    current_ptr++; // Move to the next position in the buffer
    count++;       // Increment the count of characters read
  }
  // Null-terminate the string at the current position
  *current_ptr = '\0';
  return count; // Return the number of characters read (excluding delimiter)
}

// Function: parse_input
int parse_input(char *input_buffer) {
  int chars_read = read_until(0, input_buffer, 0x80, '\n'); // fd 0 is stdin, 0x80 is 128
  if (chars_read < 1) {
    exit(0); // Exit if no input or error
  }
  char first_char = *input_buffer;
  if (first_char == '?') {
    return 1;
  }
  if (first_char < '@') {
    if (first_char == '+') {
      return 0;
    }
    if (first_char == '/') {
      return 2;
    }
  }
  // If not a special character, check if it's an alphabet
  if (isalpha((int)first_char) == 0) {
    return 9; // Not an alphabet
  }
  return tolower((int)first_char); // Return lowercase value
}

// Function: banner
void banner(void) {
  char buffer[2048];
  int current_pos = 0;
  
  memset(buffer, 0, sizeof(buffer)); // Use sizeof for buffer size
  
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 33; j++) { // 0x21 is 33
      buffer[current_pos++] = (j & 1) == 0 ? '_' : '-'; // Reduce intermediate variable
    }
    buffer[current_pos++] = '\n';
  }
  
  strcpy(buffer + current_pos, "     Sh41l w3 p14y a g4m3?\n");
  current_pos += strlen("     Sh41l w3 p14y a g4m3?\n"); // Use strlen for length
  
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 33; j++) {
      buffer[current_pos++] = (j & 1) == 0 ? '_' : '-'; // Reduce intermediate variable
    }
    buffer[current_pos++] = '\n';
  }
  fdprintf(1, DAT_000140d9, buffer); // fd 1 is stdout
}

// Function: record_winner
void record_winner(void) {
  char comment_buf[2047]; // Use a descriptive name
  
  fdprintf(1,"###### Good Work! ######\n");
  fdprintf(1,"# Leave your comment!! #\n");
  fdprintf(1,"########################\n");
  fdprintf(1,"Comment: ");
  if (read_until(0, comment_buf, sizeof(comment_buf), '\n') > 0) {
    fdprintf(1,"You wrote: %s\n", comment_buf);
    total++; // Increment total directly
    return;
  }
  exit(0); // Exit on error or no input
}

// Function: new_challenge
// param_1 is expected to be a pointer to the start of the challenge data on the stack of play_game.
// This implementation assumes a 32-bit architecture for pointer sizes and stack offsets.
void new_challenge(int ptr_to_play_game_stack_frame) {
  char seed_str[32];
  ulong u_seed_val;
  ushort seed_derived;
  uint word_index;
  size_t challenge_word_len;

  // Map pointers based on assumed stack layout and 32-bit types
  // This is fragile and depends on the specific compiler/architecture/ABI.
  // In a robust implementation, these would be passed explicitly or as part of a struct.
  char *challenge_word_buf = (char *)((char*)ptr_to_play_game_stack_frame + 8);
  char *masked_word_buf = (char *)((char*)ptr_to_play_game_stack_frame + 0x1c);
  int *tries_ptr = (int *)((char*)ptr_to_play_game_stack_frame + 0x30);

  fdprintf(1,"\n@ @ @ @ @  New Challenge  @ @ @ @ @\n");
  fdprintf(1,"Seed? ");
  if (read_until(0, seed_str, sizeof(seed_str), '\n') < 1) {
    exit(0);
  }
  u_seed_val = strtoul(seed_str, NULL, 10); // NULL for endptr
  
  // LFSR-like seed derivation
  seed_derived = (ushort)(u_seed_val >> 0x10) & 0xff | (ushort)(u_seed_val << 8);
  if (seed_derived == 0) {
    seed_derived = 0xace1;
  }
  word_index = (uint)(seed_derived >> 1) |
               ((ushort)(seed_derived >> 5 ^ seed_derived >> 2 ^ seed_derived ^ seed_derived >> 3) & 1) << 0xf;
  
  // Clear buffers for challenge word and masked word
  memset(challenge_word_buf, 0, 0x14); // 0x14 is 20 bytes
  memset(masked_word_buf, 0, 0x14);

  // Copy a word from the global words array
  strcpy(challenge_word_buf, words[word_index % (sizeof(words) / sizeof(words[0]))]);
  
  // Initialize masked word with '_' characters
  challenge_word_len = strlen(challenge_word_buf);
  memset(masked_word_buf, '_', challenge_word_len);
  masked_word_buf[challenge_word_len] = '\0'; // Ensure null termination
  
  // Reset tries
  *tries_ptr = 0;
}

// Function: quit_game
void quit_game(void) {
  fdprintf(1,"\n * * * * Thank you for playing! You\'ve won %d times! * * * *\n", total);
  exit(0);
}

// Function: play_game
void play_game(void) {
  char input_buffer[128];
  code_ptr_int challenge_func_ptr = (code_ptr_int)new_challenge; // Default to new_challenge
  code_ptr_void quit_func_ptr = (code_ptr_void)quit_game; // Default to quit_game
  char challenge_word[20]; // Corresponds to local_44
  char masked_word[20];    // Corresponds to local_30
  int tries = 0;           // Corresponds to local_1c
  
  while (1) { // Main game loop, replaces LAB_0001152a
    int special_input_flag = 0; // Corresponds to local_18, reset each turn
    size_t current_word_len;

    // Condition for a new challenge: win flag is set OR challenge_word is empty
    if (win != 0 || strlen(challenge_word) == 0) { // Corresponds to LAB_000115cc
      if (challenge_func_ptr == NULL) challenge_func_ptr = (code_ptr_int)new_challenge;
      if (quit_func_ptr == NULL) quit_func_ptr = (code_ptr_void)quit_game;
      
      // Call new_challenge with the address of quit_func_ptr.
      // This allows new_challenge to modify challenge_word, masked_word, and tries
      // by using pointer arithmetic relative to the location of quit_func_ptr on the stack.
      challenge_func_ptr((int)&quit_func_ptr);
      win = 0; // Reset win flag after new challenge
    }

    fdprintf(1,"[[[ Your challenge: %s ]]]\n", masked_word);
    fdprintf(1,"Guess a letter: ");
    char input_char_code = parse_input(input_buffer);

    if (input_char_code == '\t') { // Special input: tab
      special_input_flag = 1;
    } else if (input_char_code == '\x02') { // Special input: ASCII ETX (Ctrl+C-like) -> Quit
      if (quit_func_ptr == NULL) quit_func_ptr = (code_ptr_void)quit_game;
      quit_func_ptr(); // This function calls exit(), so it does not return.
    } else if (input_char_code == '\0') { // Special input: NUL (empty input or error) -> New challenge
      continue; // Restart loop, will trigger new challenge setup
    } else if (input_char_code == '\x01') { // Special input: ASCII SOH (Ctrl+A-like) -> Reset challenge
      tries = 0;
      current_word_len = strlen(challenge_word);
      memset(masked_word, '_', current_word_len);
      masked_word[current_word_len] = '\0'; // Ensure null termination
      fdprintf(1,"\n^^^^^ RESET ^^^^^\n\n");
      continue; // Restart loop
    } else { // Regular letter guess
      tries++;
    }

    // This block of code corresponds to the logic after LAB_0001167b
    if (special_input_flag == 0) { // Only process if not a special input like '\t'
      int char_found_in_word = 0; // Flag for character found in word
      current_word_len = strlen(challenge_word);

      // Check if guessed letter is in the challenge word
      for (uint i = 0; i < current_word_len; i++) {
        if (input_buffer[0] == challenge_word[i]) {
          masked_word[i] = input_buffer[0];
          char_found_in_word = 1;
        }
      }

      if (char_found_in_word != 0) {
        int word_is_complete = 1; // Assume complete until '_' is found
        current_word_len = strlen(masked_word);

        // Check if masked word still contains '_' (meaning not complete)
        for (uint i = 0; i < current_word_len; i++) {
          if (masked_word[i] == '_') {
            word_is_complete = 0; // Found an underscore, word is not complete
            break;
          }
        }
        
        if (word_is_complete == 1) { // Word is fully guessed
          fdprintf(1,">>> You got it!! \"%s\" (%d tries) <<<\n", challenge_word, tries);
          win = 1; // Set win flag
          return; // Exit play_game, main loop will call record_winner
        }
      }
    }
  }
}

// Function: main
int main(void) { // main should return int
  fdprintf(1,"\nWelcome to HackMan v13.37\n\n");
  do {
    if (win == 0) {
      banner();
    } else {
      record_winner();
    }
    play_game(); // play_game returns when a win occurs
  } while(1); // Infinite loop
  return 0; // Unreachable code due to infinite loop, but good practice for main
}