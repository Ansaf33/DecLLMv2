#include <stdio.h>    // For dprintf (if using), exit, NULL
#include <stdlib.h>   // For exit, strtol, strsep
#include <string.h>   // For strlen, strsep
#include <ctype.h>    // For isdigit
#include <unistd.h>   // For ssize_t, dprintf (if using)
#include <sys/types.h> // For ssize_t (sometimes)

// Define fdprintf as dprintf for Linux compilability.
// If dprintf is not available (e.g., older glibc), it would need to be replaced
// with fprintf(stderr, ...) or similar, or a custom implementation.
#define fdprintf dprintf

// Global variables
char g_buf[512]; // Max size 0x200 (512) for readline buffer
long g_num_steps_left;
int g_shots_left;
int g_bombs_left;
long g_current_score;
long g_high_score = 0; // Initialize high score
int g_board_revealed;

// PRNG state variables
unsigned int a = 0x5a;
unsigned int b = 0x5a;
unsigned int c = 0x5a;
unsigned int d = 0x5a;

// Forward declarations for game board functions, assuming a raw pointer and byte offsets
// `g_game_board` is likely a `void*` or `char*` to some structure/object.
// Function pointers are accessed via byte offsets from this base pointer.
typedef void (*GameFunc_place_runner)(void *board_ptr, int x, int y, int val1, int val2);
typedef int (*GameFunc_run_steps)(int steps);
typedef int (*GameFunc_take_shot)(long x, long y);
typedef int (*GameFunc_place_bomb)(long x, long y);

// Macro for accessing function pointers at specific byte offsets from the board_ptr
// This mimics the `(**(code **)(g_game_board + OFFSET))` pattern.
#define GET_FUNC_PTR(board_ptr, offset, type) (*((type*)((char*)board_ptr + offset)))

void *g_game_board; // `g_game_board` is a pointer to the "game board object"

// External functions (gld_*) - assumed to be globally available
extern void gld_clear_board(void);
extern void *gld_init_game(void); // Returns the g_game_board pointer
extern void gld_print_board(const char *title);

// Placeholder for `receive` function (not a standard C function).
// Mocked to behave like `read` but with an additional `bytes_read_ptr` output.
// Returns 0 on success, -1 on error.
ssize_t receive(int fd, void *buf, size_t count, int *bytes_read_ptr) {
    ssize_t bytes = read(fd, buf, count);
    if (bytes == -1) {
        *bytes_read_ptr = 0; // Indicate no bytes read on error
        return -1; // Indicate error
    } else {
        *bytes_read_ptr = (int)bytes;
        return 0; // Indicate success (like original iVar1 == 0)
    }
}

// Function prototypes
unsigned int readopt(int fd);
int readline(int fd, char *buffer, unsigned int max_len);
long parse_num_steps(char *str);
long *parse_coordinate(long *coords, char *str);
void take_shot(void);
void place_bomb(void);
void reveal_board(void);
void place_runner(int x, int y);
void create_runner(void);
void new_game(void);
void run(void);
int quit(void);
void print_menu(void);
int game_selection(void);
void init_randomness(void);
int main(void);


// Function: readopt
unsigned int readopt(int fd) {
  unsigned char option_char;
  char next_char;
  int bytes_read;
  
  if (receive(fd, &option_char, 1, &bytes_read) != 0 || bytes_read == 0) {
    return 0xffffffff;
  }
  
  if (receive(fd, &next_char, 1, &bytes_read) != 0 || bytes_read == 0) {
    return 0xffffffff;
  }
  
  if (next_char == '\n') {
    return (unsigned int)option_char;
  } else {
    // Consume the rest of the line until newline or error/EOF
    while (next_char != '\n' && (receive(fd, &next_char, 1, &bytes_read) == 0 && bytes_read != 0));
    return 0xffffffff;
  }
}

// Function: readline
int readline(int fd, char *buffer, unsigned int max_len) {
  unsigned int i;
  int bytes_read;
  
  for (i = 0; i < max_len; ++i) {
    if (receive(fd, &buffer[i], 1, &bytes_read) != 0 || bytes_read == 0) {
      exit(0); // Original code exits on read error/EOF
    }
    if (buffer[i] == '\n') {
      buffer[i] = '\0'; // Null-terminate the string
      return 0; // Success
    }
  }
  // If loop completes, max_len characters were read without a newline.
  // Original implies 0xffffffff (-1) for this case.
  return -1; 
}

// Function: parse_num_steps
long parse_num_steps(char *str) {
  char *ptr = str;
  while (*ptr != '\0') {
    if (!isdigit((unsigned char)*ptr)) {
      return -1;
    }
    ptr++;
  }
  long num_steps = strtol(str, NULL, 10);
  if ((num_steps > 99) && (num_steps < 1001 /* 0x3e9 */)) {
    if (g_num_steps_left < num_steps) {
      return g_num_steps_left;
    }
    return num_steps;
  }
  return -1;
}

// Function: parse_coordinate
long * parse_coordinate(long *coords, char *str) {
  char *x_str = strsep(&str, ",");
  char *y_str = strsep(&str, " ");
  
  // Original logic: if a third token exists (str is not NULL), use it as y_str
  if (str != NULL) {
    y_str = str;
  }
  
  if (x_str == NULL || y_str == NULL || strlen(x_str) == 0 || strlen(y_str) == 0) {
    coords[0] = -1;
    coords[1] = -1;
    return coords;
  }
  
  // Validate x_str
  char *temp_ptr = x_str;
  while (*temp_ptr != '\0') {
    if (!isdigit((unsigned char)*temp_ptr)) {
      coords[0] = -1;
      coords[1] = -1;
      return coords;
    }
    temp_ptr++;
  }
  
  // Validate y_str
  temp_ptr = y_str;
  while (*temp_ptr != '\0') {
    if (!isdigit((unsigned char)*temp_ptr)) {
      coords[0] = -1;
      coords[1] = -1;
      return coords;
    }
    temp_ptr++;
  }
  
  coords[0] = strtol(x_str, NULL, 10);
  coords[1] = strtol(y_str, NULL, 10);
  return coords;
}

// Function: take_shot
void take_shot(void) {
  long coords[2]; // local_18 and local_14 correspond to coords[0] and coords[1]
  int score_gained;
  
  fdprintf(1,"Enter Shot Coordinates Ex: 100, 100:: ");
  if (readline(0, g_buf, sizeof(g_buf)) == 0) {
    parse_coordinate(coords, g_buf); // Corrected arguments
    if ((coords[0] < 0) || (coords[1] < 0)) {
      fdprintf(1,"Invalid coordinates\n");
    }
    else if (g_shots_left == 0) {
      fdprintf(1,"You're out of bullets.\n");
    }
    else {
      score_gained = GET_FUNC_PTR(g_game_board, 0x30, GameFunc_take_shot)(coords[0], coords[1]);
      g_shots_left--; // g_shots_left = g_shots_left + -1;
      if (score_gained == 0) {
        fdprintf(1,"You missed. Better luck next time.\n");
      }
      else {
        fdprintf(1,"Great shot! You got one\n");
        fdprintf(1,"Scored: %d points\n", score_gained);
      }
      g_current_score += score_gained; // g_current_score = local_10 + g_current_score;
    }
  }
  else {
    fdprintf(1,"Invalid coordinates\n");
  }
}

// Function: place_bomb
void place_bomb(void) {
  long coords[2]; // local_14 and local_10 correspond to coords[0] and coords[1]
  int result;
  
  fdprintf(1,"Enter Bomb Coordinates Ex: 100, 100:: ");
  if (readline(0, g_buf, sizeof(g_buf)) == 0) {
    parse_coordinate(coords, g_buf); // Corrected arguments
    if ((((coords[0] < 0) || (coords[1] < 0)) || (0x1fff < coords[0])) || (0x1fff < coords[1])) {
      fdprintf(1,"Invalid coordinates\n");
    }
    else if (g_bombs_left == 0) {
      fdprintf(1,"You're out of bombs.\n");
    }
    else {
      result = GET_FUNC_PTR(g_game_board, 0x34, GameFunc_place_bomb)(coords[0], coords[1]);
      if (result == 0) {
        fdprintf(1,"There is already a bomb here. Try somewhere else.\n");
      }
      else {
        fdprintf(1,"Successfully planted bomb.\n");
        g_bombs_left--; // g_bombs_left = g_bombs_left + -1;
      }
    }
  }
  else {
    fdprintf(1,"Invalid coordinates\n");
  }
}

// Function: reveal_board
void reveal_board(void) {
  g_board_revealed = 1;
  gld_print_board("GAME BOARD\n");
  fdprintf(1,"Press enter to continue\n"); // Assuming DAT_0001719b is this string
}

// Function: place_runner
void place_runner(int x, int y) {
  GET_FUNC_PTR(g_game_board, 0x24, GameFunc_place_runner)(g_game_board, x, y, 1, 0);
  if ((x < 0x1000) && (y < 0x1000)) {
    GET_FUNC_PTR(g_game_board, 0x24, GameFunc_place_runner)(g_game_board, x, y + 1, 1, 0);
    GET_FUNC_PTR(g_game_board, 0x24, GameFunc_place_runner)(g_game_board, x, y + 2, 1, 0);
    GET_FUNC_PTR(g_game_board, 0x24, GameFunc_place_runner)(g_game_board, x - 1, y + 2, 1, 0);
    GET_FUNC_PTR(g_game_board, 0x24, GameFunc_place_runner)(g_game_board, x - 2, y + 1, 1, 0);
  }
  else if ((x < 0x1000) || (y > 0xfff)) { // 0xfff < y is y > 0xfff
    if ((x < 0x1000) && (y > 0xfff)) {
      GET_FUNC_PTR(g_game_board, 0x24, GameFunc_place_runner)(g_game_board, x, y + 1, 1, 0);
      GET_FUNC_PTR(g_game_board, 0x24, GameFunc_place_runner)(g_game_board, x, y + 2, 1, 0);
      GET_FUNC_PTR(g_game_board, 0x24, GameFunc_place_runner)(g_game_board, x - 1, y, 1, 0);
      GET_FUNC_PTR(g_game_board, 0x24, GameFunc_place_runner)(g_game_board, x - 2, y + 1, 1, 0);
    }
    else if ((x > 0xfff) && (y > 0xfff)) { // 0xfff < x is x > 0xfff
      GET_FUNC_PTR(g_game_board, 0x24, GameFunc_place_runner)(g_game_board, x, y + 1, 1, 0);
      GET_FUNC_PTR(g_game_board, 0x24, GameFunc_place_runner)(g_game_board, x, y + 2, 1, 0);
      GET_FUNC_PTR(g_game_board, 0x24, GameFunc_place_runner)(g_game_board, x + 1, y, 1, 0);
      GET_FUNC_PTR(g_game_board, 0x24, GameFunc_place_runner)(g_game_board, x + 2, y + 1, 1, 0);
    }
  }
  else { // This branch covers (x >= 0x1000 && y < 0x1000)
    GET_FUNC_PTR(g_game_board, 0x24, GameFunc_place_runner)(g_game_board, x, y + 1, 1, 0);
    GET_FUNC_PTR(g_game_board, 0x24, GameFunc_place_runner)(g_game_board, x, y + 2, 1, 0);
    GET_FUNC_PTR(g_game_board, 0x24, GameFunc_place_runner)(g_game_board, x + 1, y + 2, 1, 0);
    GET_FUNC_PTR(g_game_board, 0x24, GameFunc_place_runner)(g_game_board, x + 2, y + 1, 1, 0);
  }
}

// Function: create_runner
void create_runner(void) {
  unsigned int rand1, rand2;
  
  rand1 = a ^ (a << 0xb);
  rand1 = rand1 >> 8 ^ rand1 ^ (d >> 0x13) ^ d;
  rand2 = b ^ (b << 0xb);
  
  a = c;
  b = d;
  d = rand2 >> 8 ^ rand2 ^ (rand1 >> 0x13) ^ rand1;
  c = rand1;
  place_runner(rand1 & 0x1fff, d & 0x1fff);
}

// Function: new_game
void new_game(void) {
  gld_clear_board(); // External call
  if (g_high_score < g_current_score) {
    g_high_score = g_current_score;
  }
  for (int i = 0; i < 0x19; ++i) { // 0x19 = 25
    create_runner();
  }
  g_board_revealed = 0;
  g_current_score = 0;
  g_num_steps_left = 10000;
  g_shots_left = 0x7d; // 125 shots
  g_bombs_left = 5;
  fdprintf(1,"New Game -- Ready, set, go!\n");
}

// Function: run
void run(void) {
  fdprintf(1,"Enter amount of time to run: [%d-%d]:: ",100,1000);
  if (readline(0, g_buf, sizeof(g_buf)) == 0) {
    int steps_to_run = parse_num_steps(g_buf);
    if (steps_to_run == -1) {
      fdprintf(1,"Invalid amount of time\n");
    }
    else {
      int score_from_bombs = GET_FUNC_PTR(g_game_board, 0x2c, GameFunc_run_steps)(steps_to_run);
      if (score_from_bombs > 0) {
        fdprintf(1,"Nice bombing! You racked up %d points.\n", score_from_bombs);
      }
      if (steps_to_run < g_num_steps_left) {
        g_num_steps_left -= steps_to_run;
      }
      else {
        g_num_steps_left = 0;
      }
      g_current_score += score_from_bombs;
      g_board_revealed = 0;
    }
  }
  else {
    fdprintf(1,"Invalid amount of time\n");
  }
}

// Function: quit
int quit(void) {
  fdprintf(1,"Thanks for Playing\n");
  return 0;
}

// Function: print_menu
void print_menu(void) {
  if ((g_num_steps_left < 1) || ((g_shots_left == 0 && (g_bombs_left == 0)))) {
    g_num_steps_left = 0; // Ensure it's zero if conditions met
    fdprintf(1,"GAME OVER!\n");
    if (g_high_score < g_current_score) {
      fdprintf(1,"A new high score!!\n\n");
      g_high_score = g_current_score;
    }
    fdprintf(1,"                              --GAME OVER--\n");
    fdprintf(1,"1. New Game\n");
    fdprintf(1,"2. Quit\n\n");
    fdprintf(1,"High Score = %ld\n\n",g_high_score); // Changed %d to %ld for long
  }
  else if (g_board_revealed == 0) {
    fdprintf(1,"                              --Board: HIDDEN--\n");
    fdprintf(1,"1. Shoot!\n");
    fdprintf(1,"2. Reveal Board\n");
    fdprintf(1,"3. New Game\n");
    fdprintf(1,"4. Quit\n");
    fdprintf(1,"Current Score = %ld | Total Time Remaining: %ld | High Score = %ld \n",g_current_score,
             g_num_steps_left,g_high_score); // Changed %d to %ld for long
    fdprintf(1,"Shots Remaining: %d | Bombs Remaining: %d\n\n",g_shots_left,g_bombs_left);
  }
  else { // g_board_revealed == 1
    fdprintf(1,"                              --Board: REVEALED--\n");
    fdprintf(1,"1. Place Bomb\n");
    fdprintf(1,"2. Reveal Board\n");
    fdprintf(1,"3. Let them Run!\n");
    fdprintf(1,"4. New Game\n");
    fdprintf(1,"5. Quit\n");
    fdprintf(1,"Current Score = %ld | Total Time Remaining: %ld | High Score = %ld \n",g_current_score,
             g_num_steps_left,g_high_score); // Changed %d to %ld for long
    fdprintf(1,"Shots Remaining: %d | Bombs Remaining: %d\n\n",g_shots_left,g_bombs_left);
  }
  fdprintf(1,"Selection-: ");
}

// Function: game_selection
int game_selection(void) {
  char selection = readopt(0);
  if (selection == (char)0xff) { // 0xffffffff becomes -1 when cast to char
    fdprintf(1,"Bad Input\n");
    return -1;
  }
  
  if (g_num_steps_left < 1) { // GAME OVER menu
    if (selection == '1') {
      new_game();
    } else if (selection == '2') {
      quit();
      return 0; // Signal main to exit
    } else {
      fdprintf(1,"Bad Choice\n");
      return -1;
    }
  }
  else if (g_board_revealed == 0) { // HIDDEN board menu
    if (selection == '1') {
      take_shot();
    } else if (selection == '2') {
      reveal_board();
    } else if (selection == '3') {
      new_game();
    } else if (selection == '4') {
      quit();
      return 0; // Signal main to exit
    } else {
      fdprintf(1,"Bad Choice\n");
      return -1;
    }
  }
  else { // REVEALED board menu
    switch(selection) {
      case '1':
        place_bomb();
        break;
      case '2':
        reveal_board();
        break;
      case '3':
        run();
        break;
      case '4':
        new_game();
        break;
      case '5':
        quit();
        return 0; // Signal main to exit
      default:
        fdprintf(1,"Bad Choice\n");
        return -1;
    }
  }
  return 1; // Signal main to continue game loop
}

// Function: init_randomness
void init_randomness(void) {
  while(1) {
    fdprintf(1,
             "Before we begin. Enter a number [1 - 4294967295]\nOr any number. We'll figure it out: "
            );
    int result;
    while(1) {
      result = readline(0,&g_buf,sizeof(g_buf));
      if (result == 0) break;
      fdprintf(1,"Bad input\n");
    }
    char *ptr = g_buf;
    int is_valid_num = 1;
    if (*ptr == '\0') { // Check for empty string
        is_valid_num = 0;
    } else {
        while (*ptr != '\0') {
            if (!isdigit((unsigned char)*ptr)) {
                is_valid_num = 0;
                break;
            }
            ptr++;
        }
    }
    if (is_valid_num) {
      a = strtol(g_buf,NULL,10);
      return;
    }
    fdprintf(1,"Invalid number\n");
  }
}

// Function: main
int main(void) {
  init_randomness();
  g_game_board = gld_init_game(); // External call to initialize game board
  new_game();
  int game_status;
  do {
    print_menu();
    game_status = game_selection();
  } while (game_status != 0); // Loop until game_selection returns 0 (quit)
  fdprintf(1,"EXITING...\n");
  return 0;
}