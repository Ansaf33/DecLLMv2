#include <stddef.h> // For size_t
#include <stdio.h>  // For NULL, printf, getchar, fflush, stderr, fwrite
#include <stdlib.h> // For exit, malloc, free

// --- Mock/Helper Functions (to make code compilable on Linux) ---
// In a real scenario, these would be provided by the environment.

// Mock transmit function
// Assumes it writes to stdout and returns 0 on success, non-zero on error.
// Fills bytes_transferred with the actual number of bytes written.
int transmit(void *buffer, unsigned int len, int *bytes_transferred) {
    if (buffer == NULL || bytes_transferred == NULL) {
        fprintf(stderr, "transmit: Invalid arguments.\n");
        return -1;
    }
    int written = fwrite(buffer, 1, len, stdout);
    if (written < 0) {
        fprintf(stderr, "transmit: Write error.\n");
        *bytes_transferred = 0;
        return -1;
    }
    *bytes_transferred = written;
    return 0; // Success
}

// Mock receive function
// Assumes it reads from stdin and returns 0 on success, non-zero on error.
// Fills bytes_transferred with the actual number of bytes read.
int receive(void *buffer, unsigned int len, int *bytes_transferred) {
    if (buffer == NULL || bytes_transferred == NULL) {
        fprintf(stderr, "receive: Invalid arguments.\n");
        return -1;
    }
    int read_count = 0;
    for (unsigned int i = 0; i < len; ++i) {
        int c = getchar();
        if (c == EOF) {
            if (i == 0) { // No bytes read
                fprintf(stderr, "receive: EOF or read error.\n");
                *bytes_transferred = 0;
                return -1;
            }
            break; // Stop if EOF mid-read
        }
        ((char*)buffer)[i] = (char)c;
        read_count++;
        // If reading a line, stop at newline
        if ((char)c == '\n') {
            break;
        }
    }
    *bytes_transferred = read_count;
    return 0; // Success
}

// Mock allocate function
// Maps to malloc and stores the pointer at out_ptr.
int allocate(size_t size, int flags, void **out_ptr) {
    (void)flags; // flags is unused in this mock
    if (out_ptr == NULL) {
        fprintf(stderr, "allocate: Invalid out_ptr.\n");
        return -1;
    }
    *out_ptr = malloc(size);
    if (*out_ptr == NULL) {
        fprintf(stderr, "allocate: Memory allocation failed for size %zu.\n", size);
        return -1;
    }
    return 0; // Success
}

// Mock _terminate function
void _terminate(void) {
    fprintf(stderr, "_terminate: Program terminated due to error.\n");
    exit(1);
}

// --- Global Variables (declarations) ---
// gValidChars: Used in rotN.
char gValidChars[62] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

// gRandRegister: Used in RANDOM, srand.
static unsigned int gRandRegister;

// gWords: A pointer to an array of char pointers, allocated by init. Max 1024 pointers (0x400).
char **gWords = NULL; // Initialize to NULL

// gSeedWords: Initial seed words. Max 51 pointers (0x33).
// Placeholder data - actual content would be defined elsewhere or loaded.
char *gSeedWords[51] = {
    "apple", "banana", "cherry", "date", "elderberry",
    "fig", "grape", "honeydew", "kiwi", "lemon",
    "mango", "nectarine", "orange", "papaya", "quince",
    "raspberry", "strawberry", "tangerine", "ugli", "vanilla",
    "watermelon", "xigua", "yam", "zucchini", "apricot",
    "blueberry", "cantaloupe", "dragonfruit", "eggplant", "feijoa",
    "guava", "huckleberry", "ilama", "jicama", "kumquat",
    "lime", "mulberry", "nutmeg", "olive", "passionfruit",
    "plum", "rambutan", "satsuma", "tomato", "uva",
    "victoria", "wolfberry", "xemxem", "yellowfruit", "ziziphus",
    "ambrosia" // 51st word
};

// gWordData: Contiguous memory block for all rotated word strings, allocated by init.
char *gWordData = NULL; // Initialize to NULL

// --- Original Functions (fixed) ---

// Function: strlen
size_t strlen(const char *s) {
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
unsigned int transmit_all(void *buffer, unsigned int total_len) {
  if (buffer == NULL) {
    return 0;
  }
  unsigned int sent_total = 0;
  int sent_current;
  int ret;
  
  while (sent_total < total_len) {
    sent_current = 0;
    ret = transmit((char *)buffer + sent_total, total_len - sent_total, &sent_current);
    
    if (ret != 0 || sent_current == 0) {
      _terminate();
    }
    sent_total += sent_current;
  }
  return total_len;
}

// Function: transmit_str
void transmit_str(const char *s) {
  if (s == NULL) {
    return;
  }
  transmit_all((void *)s, strlen(s));
  return;
}

// Function: receive_all
unsigned int receive_all(void *buffer, unsigned int total_len) {
  if (buffer == NULL) {
    return 0;
  }
  unsigned int received_total = 0;
  int received_current;
  int ret;
  
  while (received_total < total_len) {
    received_current = 0;
    ret = receive((char *)buffer + received_total, total_len - received_total, &received_current);
    
    if (ret != 0 || received_current == 0) {
      _terminate();
    }
    received_total += received_current;
  }
  return total_len;
}

// Function: readline
unsigned int readline(char *buffer, unsigned int max_len) {
  if (buffer == NULL) {
    return 0;
  }
  unsigned int received_total = 0;
  char current_char = '\0';
  int bytes_read_single;
  int ret;
  
  while (received_total < max_len) {
    bytes_read_single = 0;
    ret = receive(&current_char, 1, &bytes_read_single);
    
    if (ret != 0 || bytes_read_single == 0) {
      _terminate();
    }
    buffer[received_total] = current_char;
    received_total++;
    if (current_char == '\n') {
      break;
    }
  }
  return received_total;
}

// Function: rotN
char rotN(char c, unsigned int rotation_amount) {
  int base_idx;
  
  if (c >= 'a' && c <= 'z') { // Lowercase a-z -> 0-25
    base_idx = c - 'a';
  } else if (c >= 'A' && c <= 'Z') { // Uppercase A-Z -> 26-51
    base_idx = c - 'A' + 26;
  } else if (c >= '0' && c <= '9') { // Digits 0-9 -> 52-61
    base_idx = c - '0' + 52;
  } else { // Not an alphanumeric character
    return c;
  }
  
  return gValidChars[(base_idx + rotation_amount) % 62];
}

// Function: strrotcpy
int strrotcpy(char *dest, const char *src, unsigned int rotation_amount) {
  if (dest == NULL || src == NULL) {
    return 0;
  }
  int i;
  for (i = 0; src[i] != '\0'; i++) {
    dest[i] = rotN(src[i], rotation_amount);
  }
  dest[i] = '\0';
  return i;
}

// Function: init
int init(unsigned int rotation_amount) {
  int ret;
  size_t total_word_data_len = 0;
  unsigned int i;
  
  // Allocate space for gWords pointers (1024 pointers for 0x400)
  ret = allocate(0x400 * sizeof(char *), 0, (void **)&gWords);
  if (ret != 0) {
    return -1;
  }
  
  // Calculate total length needed for all word data
  for (i = 0; i < 0x33; i++) { // 51 words
    total_word_data_len += strlen(gSeedWords[i]) + 1; // +1 for null terminator
  }
  
  // Allocate contiguous memory for all word data
  ret = allocate(total_word_data_len, 0, (void **)&gWordData);
  if (ret != 0) {
    free(gWords); // Clean up gWords if gWordData allocation fails
    gWords = NULL;
    return -1;
  }
  
  char *current_offset = gWordData;
  for (i = 0; i < 0x33; i++) { // 51 words
    gWords[i] = current_offset;
    int copied_len = strrotcpy(current_offset, gSeedWords[i], rotation_amount);
    current_offset += copied_len + 1;
  }
  
  // Initialize remaining gWords pointers to NULL (up to 0x400 total)
  for (; i < 0x400; i++) {
    gWords[i] = NULL;
  }
  
  return 0; // Success
}

// Function: toInt
int toInt(char d1, char d2) {
  if (d1 < '0' || d1 > '9' || d2 < '0' || d2 > '9') {
    return 0;
  }
  return (d1 - '0') * 10 + (d2 - '0');
}

// Function: RANDOM
unsigned int RANDOM(void) {
  // LFSR implementation
  gRandRegister =
       gRandRegister >> 1 |
       (gRandRegister ^
       gRandRegister >> 0x1f ^ gRandRegister >> 6 ^ gRandRegister >> 4 ^ gRandRegister >> 2 ^
       gRandRegister >> 1) << 0x1f;
  return gRandRegister;
}

// Function: strcmp
int strcmp(const char *s1, const char *s2) {
  if (s1 == NULL) {
    return (s2 == NULL) ? 0 : -1;
  }
  if (s2 == NULL) {
    return 1;
  }
  
  int i = 0;
  while (s1[i] != '\0') {
    if (s2[i] == '\0') {
      return 1; // s1 is longer
    }
    if (s1[i] < s2[i]) {
      return -1;
    }
    if (s2[i] < s1[i]) {
      return 1;
    }
    i++;
  }
  // If s1 ended, check if s2 also ended
  return (s2[i] == '\0') ? 0 : -1; // s2 is longer
}

// Function: srand
void srand(unsigned int seed) {
  gRandRegister = seed;
  return;
}

// Function: scramble
void scramble(char *dest, const char *src, unsigned int max_len) {
  unsigned int i = 0;
  unsigned int random_val = RANDOM();
  
  while (src[i] != '\0' && i < max_len - 1) { // -1 to leave space for null terminator
    if (!((src[i] >= 'a' && src[i] <= 'z') ||
          (src[i] >= 'A' && src[i] <= 'Z') ||
          (src[i] >= '0' && src[i] <= '9'))) {
      dest[i] = src[i];
    }
    else if (i % (random_val % 3 + 2) == 0) {
      dest[i] = '_'; // 0x5f
    }
    else {
      dest[i] = src[i];
    }
    i++;
  }
  dest[i] = '\0';
  return;
}

// Function: main
int main(void) {
  int ret_code;
  char input_buffer[64]; // Max 63 chars + null terminator
  char newline_char = '\n';
  int bytes_read;
  unsigned int seed_value;
  unsigned int word_index;

  transmit_str("Enter a two-digit number for the seed: ");
  fflush(stdout);

  bytes_read = readline(input_buffer, sizeof(input_buffer) - 1);
  if (bytes_read == 0) {
    _terminate();
  }
  input_buffer[bytes_read] = '\0';

  seed_value = toInt(input_buffer[0], input_buffer[1]);
  if (seed_value == 0) {
    transmit_str("Invalid seed. Please enter a two-digit number.\n");
    _terminate();
  }

  ret_code = init(seed_value + 1);
  if (ret_code != 0) {
    return -1; // init failed
  }

  srand(seed_value);

  for (int round_num = 0; round_num < 10; round_num++) {
    if (round_num == 0) {
      transmit_str("Word: ");
    } else {
      transmit_str("Next Word: ");
    }
    fflush(stdout);

    word_index = RANDOM() % 0x33;
    scramble(input_buffer, gWords[word_index], sizeof(input_buffer));
    transmit_str(input_buffer);
    transmit_all(&newline_char, 1);
    fflush(stdout);

    bytes_read = readline(input_buffer, sizeof(input_buffer) - 1);
    if (bytes_read == 0) {
      _terminate();
    }
    input_buffer[bytes_read] = '\0';

    if (strcmp(input_buffer, gWords[word_index]) != 0) {
      transmit_str("You Lose\n");
      fflush(stdout);
      
      if (gWords != NULL) {
          free(gWords);
          gWords = NULL;
      }
      if (gWordData != NULL) {
          free(gWordData);
          gWordData = NULL;
      }
      return 0;
    }
  }

  transmit_str("Final Round. Choose another 2 digit number for word selection: ");
  fflush(stdout);

  bytes_read = readline(input_buffer, sizeof(input_buffer) - 1);
  if (bytes_read == 0) {
    _terminate();
  }
  input_buffer[bytes_read] = '\0';

  word_index = toInt(input_buffer[0], input_buffer[1]);
  if (word_index == 0) {
    transmit_str("Invalid number. Please enter a two-digit number.\n");
    _terminate();
  }
  
  if (word_index >= 0x33) {
      word_index %= 0x33;
  }
  
  scramble(input_buffer, gWords[word_index], sizeof(input_buffer));
  transmit_str(input_buffer);
  transmit_all(&newline_char, 1);
  fflush(stdout);

  bytes_read = readline(input_buffer, sizeof(input_buffer) - 1);
  if (bytes_read == 0) {
    _terminate();
  }
  input_buffer[bytes_read] = '\0';

  if (strcmp(input_buffer, gWords[word_index]) == 0) {
    transmit_str("You Win!\n");
  } else {
    transmit_str("You Lose\n");
  }
  fflush(stdout);
  
  if (gWords != NULL) {
      free(gWords);
      gWords = NULL;
  }
  if (gWordData != NULL) {
      free(gWordData);
      gWordData = NULL;
  }

  return 0;
}