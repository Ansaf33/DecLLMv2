#include <stdio.h>    // For FILE, stdout, stderr, printf, fprintf, fflush, fread, fwrite
#include <stdlib.h>   // For exit, malloc, free, NULL
#include <stddef.h>   // For size_t
#include <stdint.h>   // For uint32_t (if needed, but unsigned int is used for uint)
#include <string.h>   // For standard string functions (though custom versions are provided)

// Define 'uint' if not already defined by a header
#ifndef __UINT_DEFINED
#define __UINT_DEFINED
typedef unsigned int uint;
#endif

// Global variables (stubs/definitions)
char gValidChars[62] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
uint gRandRegister = 1; // Initial seed
char **gWords = NULL;
char *gWordData = NULL;

// Dummy seed words (actual data would come from somewhere else in a real program)
// The original code implies 0x33 (51) seed words.
#define NUM_SEED_WORDS 51
char *gSeedWords_actual[NUM_SEED_WORDS] = {
    "apple", "banana", "cherry", "date", "elderberry", "fig", "grape", "honeydew",
    "kiwi", "lemon", "mango", "nectarine", "orange", "papaya", "quince", "raspberry",
    "strawberry", "tangerine", "ugli", "vanilla", "watermelon", "xigua", "yam", "zucchini",
    "apricot", "blueberry", "cantaloupe", "dragonfruit", "eggplant", "feijoa", "gooseberry",
    "huckleberry", "iceapple", "jackfruit", "kalamansi", "lime", "mulberry", "nut",
    "olive", "peach", "plum", "raisin", "satsuma", "tomato", "uva", "victoria",
    "walnut", "xanthorrhoea", "yellow", "zelkova", "almond"
};
char **gSeedWords = gSeedWords_actual; // Set the global pointer

// Forward declarations for functions
size_t custom_strlen(const char *s);
int custom_strcmp(const char *s1, const char *s2);
uint RANDOM(void);
void srand_custom(uint seed);
char rotN(char c, uint rotation_amount);

// Stubs for external functions to make the code compilable.
// In a real application, these would link to actual system/library functions.

// transmit: Writes data to a file descriptor. Returns 0 on success, non-zero on error.
//           bytes_written is updated with the actual number of bytes written.
int transmit(int fd, const char* buf, uint len, int* bytes_written) {
    if (buf == NULL || bytes_written == NULL) {
        fprintf(stderr, "transmit: Invalid arguments (NULL buffer or bytes_written ptr)\n");
        if (bytes_written) *bytes_written = 0;
        return -1;
    }
    FILE *output_stream = NULL;
    if (fd == 1) { // stdout
        output_stream = stdout;
    } else if (fd == 2) { // stderr
        output_stream = stderr;
    } else {
        // For other FDs, just simulate success for now
        *bytes_written = (int)len;
        return 0;
    }

    size_t written_count = fwrite(buf, 1, len, output_stream);
    *bytes_written = (int)written_count;
    if (written_count != len) {
        // Partial write or error
        return -1;
    }
    fflush(output_stream); // Ensure output is flushed
    return 0;
}

// receive: Reads data from a file descriptor. Returns 0 on success, non-zero on error.
//          bytes_read is updated with the actual number of bytes read.
int receive(int fd, char* buf, uint len, int* bytes_read) {
    if (buf == NULL || bytes_read == NULL) {
        fprintf(stderr, "receive: Invalid arguments (NULL buffer or bytes_read ptr)\n");
        if (bytes_read) *bytes_read = 0;
        return -1;
    }
    FILE *input_stream = NULL;
    if (fd == 0) { // stdin
        input_stream = stdin;
    } else {
        // For other FDs, just simulate no data or error
        *bytes_read = 0;
        return -1;
    }

    size_t read_count = fread(buf, 1, len, input_stream);
    *bytes_read = (int)read_count;
    if (ferror(input_stream)) {
        return -1; // Error during read
    }
    return 0;
}

// allocate: Allocates memory. Returns 0 on success, non-zero on error.
int allocate(size_t size, int flags, void** ptr) {
    (void)flags; // flags parameter is unused
    if (ptr == NULL) {
        fprintf(stderr, "allocate: Invalid arguments (NULL ptr address)\n");
        return -1;
    }
    *ptr = malloc(size);
    if (*ptr == NULL) {
        fprintf(stderr, "allocate: Memory allocation failed for size %zu\n", size);
        return -1;
    }
    return 0;
}

// _terminate: Terminates the program with an error status.
void _terminate(void) {
    fprintf(stderr, "Program terminated unexpectedly.\n");
    exit(1);
}

// Function: custom_strlen (renamed to avoid conflict with standard library strlen)
size_t custom_strlen(const char *s) {
  if (s == NULL) {
    return 0;
  }
  size_t len = 0;
  while (s[len] != '\0') {
    len++;
  }
  return len;
}

// Function: transmit_all
// Assumed signature based on usage: transmit_all(int fd, const char *buffer, uint length)
uint transmit_all(int fd, const char *buffer, uint length) {
  uint total_transmitted = 0;
  int bytes_transmitted = 0;
  int status = 0;

  if (buffer == NULL || length == 0) {
    return 0;
  }

  do {
    status = transmit(fd, buffer + total_transmitted, length - total_transmitted, &bytes_transmitted);
    if (status != 0 || bytes_transmitted == 0) {
      _terminate(); // Error or no bytes transmitted
    }
    total_transmitted += bytes_transmitted;
  } while (total_transmitted < length);

  return total_transmitted;
}

// Function: transmit_str
// Assumed signature based on usage: transmit_str(int fd, const char *s)
void transmit_str(int fd, const char *s) {
  if (s == NULL) return;
  transmit_all(fd, s, (uint)custom_strlen(s));
}

// Function: receive_all
// Assumed signature based on usage: receive_all(int fd, char *buffer, uint length)
uint receive_all(int fd, char *buffer, uint length) {
  uint total_received = 0;
  int bytes_received = 0;
  int status = 0;

  if (buffer == NULL || length == 0) {
    return 0;
  }

  do {
    status = receive(fd, buffer + total_received, length - total_received, &bytes_received);
    if (status != 0 || bytes_received == 0) {
      _terminate(); // Error or no bytes received
    }
    total_received += bytes_received;
  } while (total_received < length);

  return total_received;
}

// Function: readline
// Assumed signature based on usage: readline(int fd, char *buffer, uint max_len)
uint readline(int fd, char *buffer, uint max_len) {
  if (buffer == NULL || max_len == 0) {
    return 0;
  }

  uint total_read = 0;
  int bytes_read = 0;
  char current_char = '\0';
  int status = 0;

  do {
    status = receive(fd, &current_char, 1, &bytes_read); // Read one char at a time
    if (status != 0 || bytes_read == 0) {
      _terminate(); // Error or no bytes received
    }
    
    if (total_read < max_len - 1) { // Leave space for null terminator
        buffer[total_read] = current_char;
    }
    total_read++;

  } while ((current_char != '\n') && (total_read < max_len));
  
  // Ensure null-termination
  if (total_read <= max_len) {
      buffer[total_read -1] = '\0'; // Replace newline with null, or last char if max_len reached
  } else {
      buffer[max_len - 1] = '\0'; // Ensure the buffer is null-terminated at its max capacity
  }

  return total_read - 1; // Return count excluding newline, or max_len-1 if buffer full
}

// Function: rotN
char rotN(char c, uint rotation_amount) {
  int index;
  if (c >= 'a' && c <= 'z') {
    index = c - 'a';
  } else if (c >= 'A' && c <= 'Z') {
    index = c - 'A' + 26; // After 'z'
  } else if (c >= '0' && c <= '9') {
    index = c - '0' + 52; // After 'Z'
  } else {
    return c; // Not a rotatable character, return as is
  }

  return gValidChars[(index + rotation_amount) % 62];
}

// Function: strrotcpy
// Assumed signature: strrotcpy(char *dest, const char *src, uint rotation_amount)
int strrotcpy(char *dest, const char *src, uint rotation_amount) {
  if (dest == NULL || src == NULL) {
    return 0;
  }
  int i = 0;
  while (src[i] != '\0') {
    dest[i] = rotN(src[i], rotation_amount);
    i++;
  }
  dest[i] = '\0';
  return i;
}

// Function: init
// Assumed signature: init(uint rotation_key)
int init(uint rotation_key) {
  int status;
  
  // Allocate gWords (array of char pointers)
  status = allocate(NUM_SEED_WORDS * sizeof(char *), 0, (void**)&gWords);
  if (status != 0) {
    return -1;
  }

  size_t total_word_data_len = 0;
  for (uint i = 0; i < NUM_SEED_WORDS; i++) {
    total_word_data_len += custom_strlen(gSeedWords[i]) + 1; // +1 for null terminator
  }

  // Allocate gWordData (single block for all word strings)
  status = allocate(total_word_data_len, 0, (void**)&gWordData);
  if (status != 0) {
    // Free gWords if gWordData allocation fails
    free(gWords);
    gWords = NULL;
    return -1;
  }

  char *current_word_ptr = gWordData;
  for (uint i = 0; i < NUM_SEED_WORDS; i++) {
    gWords[i] = current_word_ptr; // Store pointer to word in gWords array
    int written_len = strrotcpy(current_word_ptr, gSeedWords[i], rotation_key);
    current_word_ptr += written_len + 1; // Move pointer past the current word + null terminator
  }
  
  // The original code initialized up to 0x400 (1024) elements.
  // Assuming 0x33 (51) are actual words and the rest are NULL.
  for (uint i = NUM_SEED_WORDS; i < 0x400; i++) {
    // Ensure gWords has enough allocated space if this loop runs
    // For now, assuming gWords was allocated for 0x400 elements if this is strictly necessary
    // But the allocate call was for 0x33 * sizeof(char*). This implies a discrepancy.
    // I will stick to the 0x33 allocation and only initialize up to that.
    // If the 0x400 was intended, the initial allocate call for gWords would need to be larger.
    // For now, I'll restrict to the allocated size.
    // If gWords was allocated for more than NUM_SEED_WORDS elements, this would be valid.
    // Let's assume the 0x400 loop limit in original code was for a larger array.
    // Given 'allocate(0xcc,0,&gWords);' where 0xcc is 204.
    // 0xcc / sizeof(char*) = 204 / 4 = 51 for 32-bit. So 0x33 words.
    // The `for (; local_14 < 0x400; local_14 = local_14 + 1)` loop is confusing.
    // It implies gWords is much larger. I'll stick to NUM_SEED_WORDS.
  }

  return 0; // Success
}

// Function: toInt
int toInt(char char1, char char2) {
  if (char1 < '0' || char1 > '9') {
    return 0; // First char not a digit
  }
  int val = char1 - '0';

  if (char2 == '\0' || char2 == '\n') { // Single digit number
    return val;
  }
  
  if (char2 < '0' || char2 > '9') {
      return 0; // Second char not a digit (and not null terminator/newline)
  }
  
  val = val * 10 + (char2 - '0');
  return val;
}

// Function: RANDOM (Linear Feedback Shift Register)
uint RANDOM(void) {
  gRandRegister =
       gRandRegister >> 1 |
       (gRandRegister ^
       gRandRegister >> 0x1f ^ gRandRegister >> 6 ^ gRandRegister >> 4 ^ gRandRegister >> 2 ^
       gRandRegister >> 1) << 0x1f;
  return gRandRegister;
}

// Function: custom_strcmp (renamed to avoid conflict with standard library strcmp)
int custom_strcmp(const char *s1, const char *s2) {
  if (s1 == NULL) {
    if (s2 == NULL) {
      return 0;
    } else {
      return -1;
    }
  } else if (s2 == NULL) {
    return 1;
  } else {
    size_t i = 0;
    while (s1[i] != '\0' && s2[i] != '\0') {
      if (s1[i] < s2[i]) {
        return -1;
      }
      if (s1[i] > s2[i]) {
        return 1;
      }
      i++;
    }
    if (s1[i] == '\0' && s2[i] == '\0') {
      return 0;
    } else if (s1[i] == '\0') {
      return -1;
    } else { // s2[i] == '\0'
      return 1;
    }
  }
}

// Function: srand (renamed to avoid conflict with standard library srand)
void srand_custom(uint seed) {
  gRandRegister = seed;
  return;
}

// Function: scramble
// Assumed signature: scramble(char *dest, const char *src, uint max_len)
void scramble(char *dest, const char *src, uint max_len) {
  if (dest == NULL || src == NULL) return;

  uint i = 0;
  uint random_val = RANDOM(); // Call RANDOM once per scramble operation
  
  while (src[i] != '\0' && i < max_len - 1) { // Leave space for null terminator
    // Check if character is alphanumeric
    if (!((src[i] >= 'a' && src[i] <= 'z') ||
          (src[i] >= 'A' && src[i] <= 'Z') ||
          (src[i] >= '0' && src[i] <= '9'))) {
      dest[i] = src[i]; // Copy non-alphanumeric characters directly
    } else if (i % (random_val % 3 + 2) == 0) { // Scramble based on random_val
      dest[i] = '_'; // Replace with underscore
    } else {
      dest[i] = src[i]; // Copy alphanumeric characters
    }
    i++;
  }
  dest[i] = '\0';
  return;
}

// Function: main
int main(void) {
  char input_buffer[64]; // Buffer for user input
  char scrambled_buffer[64]; // Buffer for scrambled words
  char newline_char = '\n'; // For transmitting newlines
  int bytes_read;
  uint rotation_key;
  int random_word_index;
  int strcmp_result;

  // Initial prompt (replacing the empty transmit_str())
  transmit_str(1, "Welcome to the Word Scramble Challenge!\n");
  transmit_str(1, "Enter a 2-digit number for the initial seed (e.g., 12): ");
  
  // Read initial seed
  bytes_read = readline(0, input_buffer, sizeof(input_buffer));
  if (bytes_read == 0) {
    _terminate(); // No input received
  }
  // readline already null-terminates, but if it read exactly max_len chars
  // and the last was not a newline, it might not be perfect.
  // The readline implementation should handle this.

  // Convert input to integer for rotation_key
  rotation_key = (uint)toInt(input_buffer[0], input_buffer[1]);

  // Initialize the word list
  if (init(rotation_key) != 0) {
    fprintf(stderr, "Initialization failed.\n");
    return 1; // Return non-zero for error
  }

  // Seed the random number generator
  // The original code used a 4-byte `undefined4` variable `local_60` as the seed.
  // We'll use the first 4 bytes of the input buffer, or a fallback.
  uint seed_from_input = 0;
  if (bytes_read >= 4) {
      seed_from_input = *(uint*)input_buffer;
  } else if (bytes_read > 0) {
      // If input is shorter than 4 bytes, use a simpler seed
      seed_from_input = (uint)input_buffer[0];
  } else {
      seed_from_input = 1; // Default seed if no valid input for seed
  }
  srand_custom(seed_from_input);

  for (int round_num = 0; round_num < 10; round_num++) {
    if (round_num == 0) {
      transmit_str(1, "Word: ");
    } else {
      transmit_str(1, "Next Word: ");
    }

    random_word_index = RANDOM() % NUM_SEED_WORDS; // Get a random index for gWords
    const char *correct_word = gWords[random_word_index];
    
    scramble(scrambled_buffer, correct_word, sizeof(scrambled_buffer));
    
    transmit_str(1, scrambled_buffer);
    transmit_all(1, &newline_char, 1); // Transmit a newline

    bytes_read = readline(0, input_buffer, sizeof(input_buffer));
    if (bytes_read == 0) {
      _terminate();
    }

    strcmp_result = custom_strcmp(input_buffer, correct_word);
    if (strcmp_result != 0) {
      transmit_str(1, "You Lose\n");
      // Free allocated memory before exiting
      if (gWordData) free(gWordData);
      if (gWords) free(gWords);
      return 0;
    }
  }

  transmit_str(1, "Final Round. Choose another 2 digit number (e.g., 05): \n");
  
  bytes_read = readline(0, input_buffer, sizeof(input_buffer));
  if (bytes_read == 0) {
    _terminate();
  }

  // Convert input to integer for final word index
  int final_word_index = toInt(input_buffer[0], input_buffer[1]);
  if (final_word_index < 0 || final_word_index >= NUM_SEED_WORDS) {
      transmit_str(1, "Invalid number chosen for final round. You Lose.\n");
      // Free allocated memory before exiting
      if (gWordData) free(gWordData);
      if (gWords) free(gWords);
      return 0;
  }

  const char *correct_word_final = gWords[final_word_index];
  
  scramble(scrambled_buffer, correct_word_final, sizeof(scrambled_buffer));
  
  transmit_str(1, scrambled_buffer);
  transmit_all(1, &newline_char, 1); // Transmit a newline

  bytes_read = readline(0, input_buffer, sizeof(input_buffer));
  if (bytes_read == 0) {
    _terminate();
  }

  strcmp_result = custom_strcmp(input_buffer, correct_word_final);
  if (strcmp_result == 0) {
    transmit_str(1, "You Win!\n");
  } else {
    transmit_str(1, "You Lose\n");
  }
  
  // Free allocated memory
  if (gWordData) free(gWordData);
  if (gWords) free(gWords);

  return 0;
}