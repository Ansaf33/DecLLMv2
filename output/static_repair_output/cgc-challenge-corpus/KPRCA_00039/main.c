#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h> // For read, sleep, STDIN_FILENO
#include <stdbool.h> // For bool type
#include <limits.h> // For UINT_MAX

// Type aliases for clarity and to match original code's intent
typedef unsigned int uint;
typedef unsigned char byte;
typedef unsigned short ushort;
typedef unsigned long long ulonglong;
// undefined4 and undefined8 are usually 32-bit and 64-bit unsigned integers
typedef unsigned int undefined4;
typedef unsigned long long undefined8;

// Global variables from the snippet
uint pcnt = 0; // Initialized to 0, common for counters
uint prng_s = 0; // Seed, will be read from input
uint DAT_0001808c = 0; // PRNG state, initialized to 0
unsigned int ztable[0x169 * 2 * 2]; // Zobrist table, 0x169 * 2 entries, each 8 bytes (two uints)

// Format strings
const char *DAT_00015000 = "%c";
const char *DAT_0001507f = "Ticks: %u:%u\n";
const char *DAT_00015083 = "Black (1) move: ";
const char *DAT_00015087 = "White (2) move: ";
const char *DAT_00015095 = "Board:\n";
const char *DAT_00015098 = "%u ";
const char *DAT_0001509b = "\n";

// Helper for CONCAT44 - combines two 32-bit unsigned ints into a 64-bit unsigned long long
static inline unsigned long long CONCAT44(unsigned int high, unsigned int low) {
    return ((unsigned long long)high << 32) | low;
}

// Game state structure offsets (approximate, based on usage):
// 0x000 - 0x5a3: Board (19x19 * 4 bytes/cell = 1444 bytes)
// 0x5a4: Black captured stones (uint)
// 0x5a8: White captured stones (uint)
// 0x5ac: Black total score (uint)
// 0x5b0: White total score (uint)
// 0x5b4: Black stones placed (uint)
// 0x5b8: White stones placed (uint)
// 0x5bc: Ticks low (uint)
// 0x5c0: Ticks high (uint)
// 0x5c4: Pass count (byte)
// 0x5cc: Zobrist history list head (void*)
// Total size: 0x5d0 (1488 bytes)

// Forward declarations for functions
uint prand(void);
void INIT_LIST(void **list_head_ptr_addr, unsigned int val1, unsigned int val2);
void EXTD_LIST(unsigned int *list_head_ptr, unsigned int val1, unsigned int val2);
unsigned int CONT_LIST(unsigned int *list_head_ptr, unsigned int val1, unsigned int val2);
unsigned int init_zobrist(void);
undefined8 hash_zobrist(int game_state_ptr);
void push_game_state(int game_state_ptr);
void *init_game(int initial_board_ptr);
void *copy_game(const void *source_game_state);
unsigned int get_color(int game_state_ptr, byte col, byte row);
bool has_happened(unsigned int *game_state_ptr, int current_game_state_ptr);
int remove_captures(int game_state_ptr, int player_color);
void end_game(void *game_state_ptr, unsigned char *player_name, byte reason);


// Function: prand
uint prand(void) {
  pcnt++;
  ulonglong product = (ulonglong)prng_s * 0x41c64e6d;
  unsigned int low_word_product = (unsigned int)product; // uVar1
  prng_s = low_word_product + 0x3039; // uVar2
  
  // DAT_0001808c update:
  // (product >> 32) is the high 32 bits of (prng_s * 0x41c64e6d)
  // (low_word_product > 0xffffcfc6) is a carry-like flag.
  DAT_0001808c = DAT_0001808c * 0x41c64e6d + (unsigned int)(product >> 32) + (low_word_product > 0xffffcfc6);
  
  return (prng_s >> 16) | (DAT_0001808c << 16);
}

// List node structure inferred from usage
typedef struct List {
  unsigned int val1;
  unsigned int val2;
  struct List *next;
} List;

// Function: INIT_LIST
void INIT_LIST(void **list_head_ptr_addr, unsigned int val1, unsigned int val2) {
  List *new_node = (List *)calloc(1, sizeof(List));
  if (new_node == NULL) {
    exit(1);
  }
  *list_head_ptr_addr = new_node; // Assign the new node to the head pointer
  
  new_node->val1 = val1;
  new_node->val2 = val2;
  new_node->next = new_node; // Circular list
}

// Function: EXTD_LIST
void EXTD_LIST(unsigned int *list_head_ptr, unsigned int val1, unsigned int val2) {
  List *new_node = (List *)calloc(1, sizeof(List));
  if (new_node == NULL) {
    exit(1);
  }
  
  List *current_node = (List *)list_head_ptr;
  
  while(true) {
    if (current_node == current_node->next) { // Reached the end of the list (points to itself)
      new_node->next = new_node; // New node points to itself (if it becomes the new end)
      new_node->val1 = val1;
      new_node->val2 = val2;
      current_node->next = new_node; // Attach new node to the end
      return;
    }
    if ((current_node->val1 == val1) && (current_node->val2 == val2)) break; // Found duplicate
    current_node = current_node->next;
  }
  free(new_node); // If duplicate, free the new node
}

// Function: CONT_LIST
unsigned int CONT_LIST(unsigned int *list_head_ptr, unsigned int val1, unsigned int val2) {
  List *current_node = (List *)list_head_ptr;
  while(true) {
    if ((current_node->val1 == val1) && (current_node->val2 == val2)) {
      return 1;
    }
    if (current_node == current_node->next) break;
    current_node = current_node->next;
  }
  return 0;
}

// Function: init_zobrist
unsigned int init_zobrist(void) {
  for (ushort k = 0; k < 0x169; k++) {
    for (ushort j = 0; j < 2; j++) {
      unsigned long long rand_val = prand();
      int index = k * 2 + j;
      ztable[index * 2] = (unsigned int)rand_val;       // Low 32 bits
      ztable[index * 2 + 1] = (unsigned int)(rand_val >> 32); // High 32 bits
    }
  }
  return 0;
}

// Function: hash_zobrist
undefined8 hash_zobrist(int game_state_ptr) {
  unsigned int hash_low = 0;
  unsigned int hash_high = 0;
  for (ushort i = 0; i < 0x169; i++) {
    int board_val = get_color(game_state_ptr, i % 0x13, i / 0x13); // col, row
    if (board_val != 3) { // If not empty
      int zobrist_idx = (i * 2) + (board_val - 1); // board_val 1 or 2 -> index 0 or 1
      hash_low ^= ztable[zobrist_idx * 2];
      hash_high ^= ztable[zobrist_idx * 2 + 1];
    }
  }
  return CONCAT44(hash_high, hash_low);
}

// Function: push_game_state
void push_game_state(int game_state_ptr) {
  undefined8 hash_val = hash_zobrist(game_state_ptr);
  EXTD_LIST((unsigned int *)(game_state_ptr + 0x5cc), (unsigned int)hash_val, (unsigned int)(hash_val >> 32));
}

// Function: ndigits
char ndigits(int num) {
  if (num == 0) {
    return 1;
  }
  char count = 0;
  for (; num != 0; num /= 10) {
    count++;
  }
  return count;
}

// Function: init_game
void *init_game(int initial_board_ptr) {
  void *game_state = calloc(1, 0x5d0); // 0x5d0 = 1488 bytes
  if (game_state == NULL) {
    return NULL;
  }

  unsigned int *board = (unsigned int *)game_state;

  if (initial_board_ptr == 0) { // Initialize empty board
    for (uint row = 0; row < 0x13; row++) {
      for (uint col = 0; col < 0x13; col++) {
        board[row * 0x13 + col] = 3; // 3 for empty
      }
    }
  } else { // Copy existing board
    unsigned int *initial_board = (unsigned int *)initial_board_ptr;
    for (uint row = 0; row < 0x13; row++) {
      for (uint col = 0; col < 0x13; col++) {
        board[row * 0x13 + col] = initial_board[row * 0x13 + col];
      }
    }
  }
  
  // Initialize scores, captures, stones placed, ticks, pass count
  *(uint *)((char *)game_state + 0x5a4) = 0; // Black captured stones
  *(uint *)((char *)game_state + 0x5a8) = 0; // White captured stones
  *(uint *)((char *)game_state + 0x5ac) = 0; // Black total score
  *(uint *)((char *)game_state + 0x5b0) = 0; // White total score
  *(uint *)((char *)game_state + 0x5b4) = 0; // Black stones placed
  *(uint *)((char *)game_state + 0x5b8) = 0; // White stones placed
  *(uint *)((char *)game_state + 0x5bc) = 0; // Ticks low
  *(uint *)((char *)game_state + 0x5c0) = 0; // Ticks high
  *(byte *)((char *)game_state + 0x5c4) = 0; // Pass count

  undefined8 initial_hash = hash_zobrist((int)game_state);
  INIT_LIST((void **)((char *)game_state + 0x5cc), (unsigned int)initial_hash, (unsigned int)(initial_hash >> 32));
  
  return game_state;
}

// Function: copy_game
void *copy_game(const void *source_game_state) {
  void *dest = calloc(1, 0x5d0); // 0x5d0 = 1488 bytes
  if (dest == NULL) {
    exit(1);
  }
  memcpy(dest, source_game_state, 0x5d0);
  return dest;
}

// Function: get_color
unsigned int get_color(int game_state_ptr, byte col, byte row) {
  if ((col < 0x13) && (row < 0x13)) {
    return ((unsigned int *)game_state_ptr)[row * 0x13 + col];
  }
  return 4; // 4 for "Off" (out of bounds)
}

// Function: has_liberty
unsigned int has_liberty(unsigned int *game_state_ptr, char *visited_board, byte col, byte row, int player_color) {
  int current_color = get_color((int)game_state_ptr, col, row);
  if (current_color == 4) { // Out of bounds
    return 0;
  }
  if (visited_board[row * 0x13 + col] == 1) { // Already visited
    return 0;
  }
  
  if (current_color != player_color) { // Not the same color, cannot have liberty for this group
      return current_color == 3; // If it's empty, it's a liberty
  }

  visited_board[row * 0x13 + col] = 1; // Mark as visited

  // Check neighbors
  if (has_liberty(game_state_ptr, visited_board, col, row + 1, player_color)) return 1;
  if (has_liberty(game_state_ptr, visited_board, col, row - 1, player_color)) return 1;
  if (has_liberty(game_state_ptr, visited_board, col - 1, row, player_color)) return 1;
  if (has_liberty(game_state_ptr, visited_board, col + 1, row, player_color)) return 1;
  
  return 0;
}

// Function: surrounded_by
unsigned int surrounded_by(int game_state_ptr, char *visited_board, byte col, byte row, int player_color) {
  if ((game_state_ptr == 0) || (visited_board == NULL)) { // Should not happen
    exit(1);
  }

  int current_color = get_color(game_state_ptr, col, row);
  if (current_color == 4) { // Out of bounds
    return 1; // Considered surrounded by "other" (boundary)
  }
  if (visited_board[row * 0x13 + col] == 1) { // Already visited
    return 1; // Already counted as surrounded
  }
  
  if (current_color == player_color) { // Same color as the surrounding player
    visited_board[row * 0x13 + col] = 1;
    return 2; // Part of the surrounding group
  }
  
  int opponent_color = (player_color == 1) ? 2 : 1;
  if (current_color == opponent_color) { // Opponent's stone
    visited_board[row * 0x13 + col] = 1;
    return 0; // Not surrounded by 'player_color'
  }
  
  // If current_color is 3 (empty)
  visited_board[row * 0x13 + col] = 1;
  
  unsigned int sum_surrounded = 
    surrounded_by(game_state_ptr, visited_board, col, row + 1, player_color) +
    surrounded_by(game_state_ptr, visited_board, col, row - 1, player_color) +
    surrounded_by(game_state_ptr, visited_board, col + 1, row, player_color) +
    surrounded_by(game_state_ptr, visited_board, col - 1, row, player_color);
  
  if (sum_surrounded < 5) { // If any neighbor wasn't surrounded (0 or 1), then this isn't surrounded
    return 0;
  }
  return 2; // All neighbors are surrounded by `player_color` or boundary
}

// Function: remove_captures
int remove_captures(int game_state_ptr, int player_color) {
  int captures_count = 0;
  char visited_board[0x169]; // 0x169 = 361 bytes for 19x19
  void *temp_game_state = copy_game((void *)game_state_ptr);
  
  for (byte row = 0; row < 0x13; row++) {
    for (byte col = 0; col < 0x13; col++) {
      memset(visited_board, 0, sizeof(visited_board));
      int current_color = get_color((int)temp_game_state, col, row);
      if (player_color == current_color) {
        if (has_liberty((unsigned int *)temp_game_state, visited_board, col, row, player_color) == 0) {
          captures_count++;
          ((unsigned int *)game_state_ptr)[row * 0x13 + col] = 3; // Remove stone
        }
      }
    }
  }
  free(temp_game_state);
  return captures_count;
}

// Function: has_happened
bool has_happened(unsigned int *game_state_ptr, int current_game_state_ptr) {
  undefined8 hash_val = hash_zobrist((int)game_state_ptr);
  return CONT_LIST((unsigned int *)(current_game_state_ptr + 0x5cc), (unsigned int)hash_val, (unsigned int)(hash_val >> 32)) != 0;
}

// Function: score
undefined4 score(int game_state_ptr, int *black_score, int *white_score) {
  *black_score = *(int *)((char *)game_state_ptr + 0x5a4); // Black captures
  *white_score = *(int *)((char *)game_state_ptr + 0x5a8); // White captures

  char visited_board[0x169]; // 0x169 = 361 bytes for 19x19
  
  for (byte row = 0; row < 0x13; row++) {
    for (byte col = 0; col < 0x13; col++) {
      memset(visited_board, 0, sizeof(visited_board));
      int current_color = get_color(game_state_ptr, col, row);
      if (current_color == 2) {
        (*white_score)++;
      } else if (current_color == 1) {
        (*black_score)++;
      } else { // Empty (color 3)
        // Check if empty spot is surrounded by Black (1)
        if (surrounded_by(game_state_ptr, visited_board, col, row, 1) > 1) {
          (*black_score)++;
        } else {
          memset(visited_board, 0, sizeof(visited_board)); // Reset for White check
          // Check if empty spot is surrounded by White (2)
          if (surrounded_by(game_state_ptr, visited_board, col, row, 2) > 1) {
            (*white_score)++;
          }
        }
      }
    }
  }
  return 0;
}

// Function: check_and_play
unsigned int check_and_play(void **game_state_ptr_addr, uint col, uint row, int player_color, char commit_move) {
  // Check if move is within bounds and on an empty spot
  if ((col >= 0x13) || (row >= 0x13) || (get_color((int)*game_state_ptr_addr, col, row) != 3)) {
    return 0xffffffff; // Invalid move
  }

  void *temp_game_state = copy_game(*game_state_ptr_addr);
  ((unsigned int *)temp_game_state)[row * 0x13 + col] = player_color; // Place stone

  int opponent_color = (player_color == 1) ? 2 : 1;

  unsigned int captured_by_opponent = remove_captures((int)temp_game_state, opponent_color);
  unsigned int captured_by_self = remove_captures((int)temp_game_state, player_color); // Self-capture check

  // Check for Ko rule (repeated board state)
  if (has_happened((unsigned int *)temp_game_state, (int)*game_state_ptr_addr)) {
    free(temp_game_state);
    return 0xffffffff; // Ko rule violation
  }
  
  if (commit_move == 0) {
    free(temp_game_state);
    return 0; // Valid move, but not committed
  }

  // Commit the move
  free(*game_state_ptr_addr);
  *game_state_ptr_addr = temp_game_state;

  // Update captured stones counts
  if (player_color == 1) {
    *(uint *)((char *)*game_state_ptr_addr + 0x5a4) += captured_by_opponent; // Black captures opponent
    *(uint *)((char *)*game_state_ptr_addr + 0x5b4) += 1; // Black stones placed
  } else { // player_color == 2
    *(uint *)((char *)*game_state_ptr_addr + 0x5a8) += captured_by_opponent; // White captures opponent
    *(uint *)((char *)*game_state_ptr_addr + 0x5b8) += 1; // White stones placed
  }

  // Reset pass count as a move was made
  *(byte *)((char *)*game_state_ptr_addr + 0x5c4) = 0;

  return 0;
}

// Function: print_char
void print_char(char c) {
  dprintf(1, DAT_00015000, c);
}

// Function: print_rchar
void print_rchar(char c, byte count) {
  for (byte i = 0; i < count; i++) {
    print_char(c);
  }
}

// Function: print_player
void print_player(int game_state_ptr, int player_color) {
  const char *player_name_str = (player_color == 2) ? "White" : "Black";
  uint captured_stones = (player_color == 1) ? *(uint *)((char *)game_state_ptr + 0x5a4) : *(uint *)((char *)game_state_ptr + 0x5a8);
  
  dprintf(1, "%s: %u", player_name_str, captured_stones);
  char digits_count = ndigits(captured_stones);
  print_rchar(' ', 0x0e - digits_count); // 0x0e = 14
}

// Function: print_point
void print_point(unsigned int *game_state_ptr, char col, unsigned char row) {
  if (col != 0) {
    print_char(' ');
  }
  uint color = get_color((int)game_state_ptr, col, row);
  if (color == 3) { // Empty
    print_char('-');
  } else if (color == 1) { // Black
    print_char('B');
  } else if (color == 2) { // White
    print_char('W');
  }
  // If color is 4 (out of bounds), do nothing

  if (col == 0x12) { // Last column
    print_char('\n');
  } else {
    print_char(' ');
  }
}

// Function: draw_game
void draw_game(int game_state_ptr) {
  print_rchar('\n', 5);
  for (uint row = 0; row < 0x13; row++) { // 0x13 = 19
    int diff_from_center = abs((int)row - 9); // 9 is center row (0-18)
    if (diff_from_center == 3) { // Rows 6 and 12
      if (row < 9) { // Row 6
        print_player(game_state_ptr, 2); // White
      } else { // Row 12
        print_player(game_state_ptr, 1); // Black
      }
    } else if (row == 9) { // Center row
      dprintf(1, DAT_0001507f, *(uint *)((char *)game_state_ptr + 0x5bc), *(uint *)((char *)game_state_ptr + 0x5c0));
      char digits_count = ndigits(*(uint *)((char *)game_state_ptr + 0x5bc));
      print_rchar(' ', 0x0e - digits_count); // 0x0e = 14
    } else {
      print_rchar(' ', 0x15); // 0x15 = 21 spaces
    }
    for (uint col = 0; col < 0x13; col++) {
      print_point((unsigned int *)game_state_ptr, (char)col, (unsigned char)row);
    }
  }
  print_rchar('\n', 5);
}

// Function: sleep
uint sleep(uint seconds) {
  return sleep(seconds); // Use standard sleep
}

// Function: read_n_bytes
uint read_n_bytes(int fd, uint count, char *buffer, int stop_on_char, char stop_char) {
  if (count == 0 || buffer == NULL) {
    return 0xffffffff;
  }
  
  uint total_read = 0;
  while (total_read < count) {
    ssize_t bytes_read_now = read(fd, buffer + total_read, 1);
    if (bytes_read_now == -1) { // Error
      return 0xffffffff;
    }
    if (bytes_read_now == 0) { // EOF
      return total_read;
    }
    total_read += bytes_read_now;
    if (stop_on_char != 0 && buffer[total_read - 1] == stop_char) {
      buffer[total_read - 1] = '\0'; // Null-terminate at stop_char
      return total_read - 1;
    }
  }
  if (total_read < count) { // Ensure null termination if buffer not full
      buffer[total_read] = '\0';
  } else { // If buffer is full, null-terminate at the end
      buffer[count - 1] = '\0';
  }
  return total_read;
}

// Function: read_move
undefined4 read_move(unsigned char *col_out, unsigned char *row_out, unsigned char *pass_flag_out) {
  char input_buffer[8]; // Max 8 bytes input for "pass" or "XX YY"
  *pass_flag_out = 0;
  memset(input_buffer, 0, sizeof(input_buffer));
  read_n_bytes(STDIN_FILENO, sizeof(input_buffer), input_buffer, 1, '\n');

  if (strncmp("pass", input_buffer, 4) == 0) {
    *pass_flag_out = 1;
    return 0;
  }

  char *endptr = input_buffer;
  long val1 = strtol(input_buffer, &endptr, 10);
  
  if (endptr == input_buffer) { // No number found
    return 0xffffffff;
  }
  
  // Skip non-digit characters after the first number
  while (*endptr != '\0' && !isdigit((int)*endptr)) {
    endptr++;
  }

  long val2 = strtol(endptr, NULL, 10);

  if ((val1 >= 0 && val1 < 0x13) && (val2 >= 0 && val2 < 0x13)) {
    *col_out = (unsigned char)val1;
    *row_out = (unsigned char)val2;
    return 0;
  }
  return 0xffffffff;
}

// Function: end_game
void end_game(void *game_state_ptr, unsigned char *player_name, byte reason) {
  free(game_state_ptr);
  dprintf(1, "Game Over ");
  if (reason == 2) { // Stones Exhausted
    dprintf(1, "Stones Exhausted\n");
  } else if (reason == 0) { // Both Passed
    dprintf(1, "Both Passed\n");
  } else if (reason == 1) { // Game Time Exceeded
    dprintf(1, "Game Time Exceeded\n");
  }

  dprintf(1, "You are a ");
  // Assuming user is player 1 (Black)
  uint black_score = *(uint *)((char *)game_state_ptr + 0x5ac);
  uint white_score = *(uint *)((char *)game_state_ptr + 0x5b0);
  
  if (black_score > white_score) {
    dprintf(1, "Winner, ");
  } else {
    dprintf(1, "Loser, ");
  }
  dprintf(1, "%s\n", player_name);
  exit(0);
}

// Function: prompt_move
void prompt_move(int game_state_ptr, int player_color) {
  dprintf(1, DAT_0001507f, *(uint *)((char *)game_state_ptr + 0x5bc), *(uint *)((char *)game_state_ptr + 0x5c0));
  if (player_color == 1) {
    dprintf(1, DAT_00015083);
  } else if (player_color == 2) {
    dprintf(1, DAT_00015087);
  }
}

// Function: color_to_string
char *color_to_string(uint color) {
  if (color == 4) return "Off";
  if (color == 3) return "Empty";
  if (color == 1) return "Black";
  if (color == 2) return "White";
  exit(1); // Should not happen
}

// Function: pass_for
undefined4 pass_for(int game_state_ptr, int player_color) {
  *(byte *)((char *)game_state_ptr + 0x5c4) += 1; // Increment pass count
  if ((player_color == 2) && (*(byte *)((char *)game_state_ptr + 0x5c4) > 1)) {
    return 0xffffffff; // Game over due to double pass
  }
  return 0;
}

// Function: interact
uint interact(int *game_state_ptr_addr, undefined4 player_color) {
  unsigned char col, row, pass_flag;
  
  prompt_move(*game_state_ptr_addr, player_color);
  int read_result = read_move(&col, &row, &pass_flag);
  
  if (read_result < 0) { // Error reading move
    return 0xffffffff;
  }
  
  if (pass_flag == 0) { // A move was played
    int check_result = check_and_play(game_state_ptr_addr, col, row, player_color, 1);
    if (check_result < 0) { // Invalid move (Ko, self-capture, etc.)
      return 0xffffffff;
    }
    push_game_state(*game_state_ptr_addr);
    return 0;
  } else { // Player passed
    return (pass_for(*game_state_ptr_addr, player_color) != 0); // Returns 1 if game over, 0 otherwise
  }
}

// Function: calculate_move
undefined4 calculate_move(unsigned int **game_state_ptr_addr, byte *col_out, byte *row_out, int ai_player_color) {
  int best_score = -100; // Arbitrary low score
  byte best_col = 0;
  byte best_row = 0;
  int selected_moves_count = 0;

  char visited_board[0x169]; // 0x169 = 361 bytes for 19x19

  for (byte row = 0; row < 0x13; row++) {
    for (byte col = 0; col < 0x13; col++) {
      int current_move_score = 0;
      
      // Heuristic: check neighbor colors
      int opponent_color = (ai_player_color == 1) ? 2 : 1;
      
      if (get_color((int)*game_state_ptr_addr, col + 1, row) == ai_player_color) current_move_score++;
      if (get_color((int)*game_state_ptr_addr, col - 1, row) == ai_player_color) current_move_score++;
      if (get_color((int)*game_state_ptr_addr, col, row + 1) == ai_player_color) current_move_score++;
      if (get_color((int)*game_state_ptr_addr, col, row - 1) == ai_player_color) current_move_score++;

      // Adjust score based on neighbor count
      if (current_move_score == 4) current_move_score -= 3; // Surrounded by self, likely bad
      else if (current_move_score == 2) current_move_score += 1;
      else if (current_move_score == 1) current_move_score += 2;
      
      // Check if the move is valid without committing
      if (check_and_play((void **)game_state_ptr_addr, col, row, ai_player_color, 0) >= 0) {
        if (current_move_score > best_score) {
          best_score = current_move_score;
          best_col = col;
          best_row = row;
          selected_moves_count = 1;
        } else if (current_move_score == best_score) {
          // Randomly pick one of the equally good moves
          unsigned int rand_val = prand();
          // If current_move_score == best_score, 90% chance to update (or 10% not to)
          // If current_move_score > best_score, 75% chance to update (or 25% not to)
          // This logic seems reversed from standard "pick best with some probability"
          // Original: if ( (equal_score_prob < rand_val % 100) || (better_score_prob < rand_val % 100) )
          // This means a low probability to pick it.
          // Let's interpret it as: if rand_val % 100 is in the top 10% (for equal score)
          // or top 25% (for better score), then update. This makes it a random selection strategy.
          if (prand() % 100 < 10) { // 10% chance to replace with another equally good move
            best_col = col;
            best_row = row;
            selected_moves_count++;
          }
        }
        if (selected_moves_count > 5) { // Limit exploration of equally good moves
          goto end_loops; // Break from both loops
        }
      }
    }
  }

end_loops:; // Label for goto

  if (selected_moves_count == 0) {
    return 2; // No valid move found, AI passes
  } else {
    *col_out = best_col;
    *row_out = best_row;
    return 1; // Move found
  }
}

// Function: ai
undefined4 ai(int *game_state_ptr_addr, undefined4 ai_player_color) {
  unsigned char col, row;
  int move_result = calculate_move((unsigned int **)game_state_ptr_addr, &col, &row, ai_player_color);
  
  if (move_result == 1) { // A move was found
    check_and_play(game_state_ptr_addr, col, row, ai_player_color, 1);
    push_game_state(*game_state_ptr_addr);
  } else if (move_result == 2) { // AI passes
    *(byte *)((char *)*game_state_ptr_addr + 0x5c4) += 1; // Increment pass count
  } else { // Error or unexpected result
    exit(1);
  }
  return 0;
}

// Function: pb (Print Board to stderr)
void pb(int game_state_ptr) {
  dprintf(2, DAT_00015095);
  for (ushort row = 0; row < 0x13; row++) {
    for (ushort col = 0; col < 0x13; col++) {
      dprintf(2, DAT_00015098, get_color(game_state_ptr, col, row));
    }
  }
  dprintf(2, DAT_0001509b);
}

// Function: main
void main(void) {
  unsigned char player_name_buffer[8]; // Max 7 chars + null terminator for name
  int game_state_ptr;
  
  // Read PRNG seed from stdin
  if (read_n_bytes(STDIN_FILENO, sizeof(prng_s), (char *)&prng_s, 0, 0) == 0xffffffff) {
      dprintf(2, "Error reading PRNG seed.\n");
      exit(1);
  }

  if (init_zobrist() != 0) {
    exit(1);
  }
  
  game_state_ptr = (int)init_game(0); // Initialize new game
  if (game_state_ptr == 0) {
    exit(1);
  }
  
  dprintf(1, "What is your name?\n");
  if (read_n_bytes(STDIN_FILENO, sizeof(player_name_buffer), (char *)player_name_buffer, 1, '\n') == 0xffffffff) {
    dprintf(1, "Need a name, bro\n");
    exit(1);
  }
  
  dprintf(1, "Hi, %s\n", player_name_buffer);
  draw_game(game_state_ptr);
  
  while(true) {
    int player_move_result;
    do {
      player_move_result = interact(&game_state_ptr, 1); // Player 1 (Black) turn
    } while (player_move_result == -1); // Keep prompting if invalid move

    if (player_move_result == 0) { // Player made a valid move or passed without game end
      score(game_state_ptr, (int *)((char *)game_state_ptr + 0x5ac), (int *)((char *)game_state_ptr + 0x5b0));
      draw_game(game_state_ptr);
      
      ai(&game_state_ptr, 2); // Player 2 (White) AI turn
      score(game_state_ptr, (int *)((char *)game_state_ptr + 0x5ac), (int *)((char *)game_state_ptr + 0x5b0));

      // Increment 64-bit tick counter
      uint low_ticks = *(uint *)((char *)game_state_ptr + 0x5bc);
      *(uint *)((char *)game_state_ptr + 0x5bc) = low_ticks + 1;
      if (low_ticks == UINT_MAX) { // Check for overflow
          *(uint *)((char *)game_state_ptr + 0x5c0) += 1;
      }
      
      // Check game time limit (150 ticks)
      unsigned long long current_ticks = CONCAT44(*(uint *)((char *)game_state_ptr + 0x5c0), *(uint *)((char *)game_state_ptr + 0x5bc));
      if (current_ticks > 150) { // 0x96 is 150
          end_game((void *)game_state_ptr, player_name_buffer, 1); // Reason 1: Time exceeded
      }
      
      // Check stones exhausted limit (90 stones)
      if (*(uint *)((char *)game_state_ptr + 0x5b4) > 90 || *(uint *)((char *)game_state_ptr + 0x5b8) > 90) { // 0x5a is 90
          end_game((void *)game_state_ptr, player_name_buffer, 2); // Reason 2: Stones exhausted
      }
    } else if (player_move_result == 1) { // Player passed, and game is over due to double pass
      end_game((void *)game_state_ptr, player_name_buffer, 0); // Reason 0: Both passed
    }
  }
}