#include <stdio.h>    // For fprintf, fflush, stdin, stdout, FILE, printf, fgetc, EOF
#include <stdlib.h>   // For calloc, malloc, free, exit, EXIT_FAILURE
#include <string.h>   // For strcmp
#include <stdbool.h>  // For bool type
// #include <unistd.h> // For usleep, if a delay is added

// Global variables
char *playerName = NULL;
unsigned int seed1 = 0;
unsigned int seed2 = 0;
int rowCount = 0;     // Number of rows for enemies array
int columns = 0;      // Number of columns for board
int rows = 0;         // Number of rows for board
int **enemies = NULL; // Stores enemy types (1-4, 0 for destroyed)
// Game board state (0: empty, 2: ship, 3: enemy, 4: user bullet, 5: enemy bullet)
int **board = NULL;
int *shipCords = NULL; // Ship's coordinates [x, y]
// Stores user bullet coordinates [x, y], -1 for inactive
int **userBullets = NULL;
// Stores enemy bullet coordinates [y, x], -1 for inactive
int **enemyBullets = NULL;
int lost = 0;         // 0: playing, 1: lost, -1: won
int turnCounter = 0;

// Helper function: Reads from stream until 'terminator' character or 'size' bytes are read.
// Includes a null terminator.
size_t freaduntil(char *buffer, size_t size, char terminator, FILE *stream) {
    size_t count = 0;
    int c;
    while (count < size - 1 && (c = fgetc(stream)) != EOF && c != terminator) {
        buffer[count++] = (char)c;
    }
    buffer[count] = '\0'; // Null-terminate the string
    // If the terminator was not read (buffer was full), clear the rest of the line
    if (c != EOF && c != terminator) {
        while ((c = fgetc(stream)) != EOF && c != terminator);
    }
    return count;
}

// Function: seedRandomGen
void seedRandomGen(unsigned int param_1, unsigned int param_2) {
  if (playerName != NULL && strcmp(playerName, "magic") == 0) {
    seed1 = 0;
    seed2 = 0;
  } else {
    seed1 = param_1;
    seed2 = param_2;
  }
}

// Function: getRandomInt
int getRandomInt(void) {
  seed1 = (seed1 & 0xffff) * 0xa777 + (seed1 >> 0x10);
  seed2 = (seed2 & 0xffff) * 0x6e0c + (seed2 >> 0x10);
  return (int)(seed2 + seed1 * 0x10000); // Cast to int as return type is int
}

// Function: createEnemies
void createEnemies(void) {
  enemies = calloc(rowCount, sizeof(int*));
  if (!enemies) {
      fprintf(stderr, "Memory allocation failed for enemies rows.\n");
      exit(EXIT_FAILURE);
  }
  for (int i = 0; i < rowCount; ++i) {
    enemies[i] = calloc(columns / 2, sizeof(int));
    if (!enemies[i]) {
        fprintf(stderr, "Memory allocation failed for enemies columns.\n");
        exit(EXIT_FAILURE);
    }
    for (int j = 0; j < columns / 2; ++j) {
      enemies[i][j] = 1; // Initialize with enemy type 1
    }
  }
}

// Function: createBoard
int** createBoard(int num_rows, int num_cols) {
  int **new_board = malloc(num_rows * sizeof(int*));
  if (!new_board) {
      fprintf(stderr, "Memory allocation failed for board rows.\n");
      exit(EXIT_FAILURE);
  }
  for (int i = 0; i < num_rows; ++i) {
    new_board[i] = malloc(num_cols * sizeof(int));
    if (!new_board[i]) {
        fprintf(stderr, "Memory allocation failed for board columns.\n");
        exit(EXIT_FAILURE);
    }
    // Initialize board cells to 0
    for (int j = 0; j < num_cols; ++j) {
      new_board[i][j] = 0;
    }
  }
  return new_board;
}

// Function: clearEnemies
void clearEnemies(void) {
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      if (board[i][j] == 3) { // If cell contains an enemy representation
        board[i][j] = 0;
      }
    }
  }
}

// Function: setEnemies
void setEnemies(void) {
  int turn_offset = turnCounter / 20; // 0x14 is 20
  for (int i = 0; i < rowCount; ++i) {
    for (int j = 0; j < columns / 2; ++j) {
      int enemy_type = enemies[i][j];
      if (enemy_type != 0) { // Only place active enemies
          // Calculate board coordinates for the enemy based on its type and current turn offset
          int board_row_base = i * 2 + turn_offset;
          int board_col_base = j * 2;

          if (enemy_type == 1) { // Top-left part of a 2x2 grid for enemy
              board[board_row_base][board_col_base] = 3;
          } else if (enemy_type == 2) { // Top-right
              board[board_row_base][board_col_base + 1] = 3;
          } else if (enemy_type == 3) { // Bottom-left
              board[board_row_base + 1][board_col_base + 1] = 3;
          } else if (enemy_type == 4) { // Bottom-right
              board[board_row_base + 1][board_col_base] = 3;
          }
      }
    }
  }
}

// Function: movePieces
int movePieces(void) {
  // This function appears to be a placeholder or has no functionality in the provided snippet.
  return 0;
}

// Function: canEnemyFire
int canEnemyFire(int enemy_row_idx, int enemy_col_idx) {
  if (enemy_row_idx < rowCount - 1) { // If not in the last row of enemies
    for (int i = enemy_row_idx + 1; i < rowCount; ++i) {
      if (enemies[i][enemy_col_idx] != 0) { // If there's an enemy below
        return 1; // Cannot fire (blocked)
      }
    }
  }
  return 0; // Can fire
}

// Function: enemyFire
void enemyFire(int bullet_x, int bullet_y) {
  int bullet_slot = -1;
  for (int i = 0; i < rows * (columns / 2); ++i) {
    if (enemyBullets[i][0] == -1) { // Check if slot is empty (y-coord is -1)
      bullet_slot = i;
      break;
    }
  }
  if (bullet_slot != -1) {
    enemyBullets[bullet_slot][0] = bullet_y; // Store y-coordinate
    enemyBullets[bullet_slot][1] = bullet_x; // Store x-coordinate
  }
}

// Function: updateEnemies
void updateEnemies(void) {
  bool any_enemies_left = false;
  int turn_offset = turnCounter / 20;

  for (int i = 0; i < rowCount; ++i) {
    for (int j = 0; j < columns / 2; ++j) {
      if (enemies[i][j] != 0) { // If enemy is active
        any_enemies_left = true;
        unsigned int rand_val = getRandomInt();

        if (rand_val % 7 == 1) { // Enemy fires with a 1/7 chance
          if (canEnemyFire(i, j) == 0) { // Check if not blocked by another enemy
            int bullet_x = -1;
            int bullet_y = -1;
            int enemy_type = enemies[i][j];
            
            // Determine bullet starting position based on enemy type
            // Coordinates are relative to the enemy's 2x2 block on the board
            if (enemy_type == 4) { // Bottom-right
              bullet_x = j * 2;
              bullet_y = i * 2 + turn_offset + 1;
            } else if (enemy_type == 3) { // Bottom-left
              bullet_x = j * 2 + 1;
              bullet_y = i * 2 + turn_offset + 1;
            } else if (enemy_type == 1) { // Top-left
              bullet_x = j * 2;
              bullet_y = i * 2 + turn_offset;
            } else if (enemy_type == 2) { // Top-right
              bullet_x = j * 2 + 1;
              bullet_y = i * 2 + turn_offset;
            }
            if (bullet_x != -1 && bullet_y != -1) {
                enemyFire(bullet_x, bullet_y);
            }
          }
        } else if (rand_val % 3 == 0) { // Enemy moves left (change type)
          if (enemies[i][j] == 1) {
            enemies[i][j] = 4; // Wrap around from 1 to 4
          } else {
            enemies[i][j]--;
          }
        } else if (rand_val % 3 == 1) { // Enemy moves right (change type)
          if (enemies[i][j] == 4) {
            enemies[i][j] = 1; // Wrap around from 4 to 1
          } else {
            enemies[i][j]++;
          }
        }
      }
    }
  }
  if (!any_enemies_left) {
    lost = -1; // Indicate a win condition
  }
}

// Function: clearScreen
void clearScreen(void) {
  // ANSI escape codes to clear screen and move cursor to top-left
  printf("\033[H\033[J");
}

// Function: setupNewGame
int setupNewGame(void) {
  // Initialize game parameters (example values)
  rows = 20;
  columns = 20;
  rowCount = 5; // Number of enemy rows, implies enemy_rows * 2 + offset for board rows

  board = createBoard(rows, columns);

  shipCords = calloc(2, sizeof(int));
  if (!shipCords) {
      fprintf(stderr, "Memory allocation failed for shipCords.\n");
      exit(EXIT_FAILURE);
  }
  shipCords[0] = columns / 2; // Initial X position (center)
  shipCords[1] = rows - 1;    // Initial Y position (bottom row)
  board[shipCords[1]][shipCords[0]] = 2; // Place ship (value 2)

  fprintf(stdout, "Please input your name:\n");
  playerName = calloc(1, 256); // Max 255 chars + null
  if (!playerName) {
      fprintf(stderr, "Memory allocation failed for playerName.\n");
      exit(EXIT_FAILURE);
  }
  fflush(stdout);
  freaduntil(playerName, 256, '\n', stdin);

  // Clean up newline characters from playerName
  for (int i = 0; i < 256 && playerName[i] != '\0'; ++i) {
    if (playerName[i] == '\n') {
      playerName[i] = '\0';
      break;
    }
  }

  createEnemies(); // Initializes 'enemies' global variable
  setEnemies();    // Places enemies on the 'board'

  userBullets = calloc(rows, sizeof(int*));
  if (!userBullets) {
      fprintf(stderr, "Memory allocation failed for userBullets rows.\n");
      exit(EXIT_FAILURE);
  }
  for (int i = 0; i < rows; ++i) {
    userBullets[i] = calloc(2, sizeof(int));
    if (!userBullets[i]) {
        fprintf(stderr, "Memory allocation failed for userBullets columns.\n");
        exit(EXIT_FAILURE);
    }
    userBullets[i][0] = -1; // X-coordinate (invalid/inactive)
    userBullets[i][1] = -1; // Y-coordinate (invalid/inactive)
  }

  enemyBullets = calloc(rows * (columns / 2), sizeof(int*));
  if (!enemyBullets) {
      fprintf(stderr, "Memory allocation failed for enemyBullets rows.\n");
      exit(EXIT_FAILURE);
  }
  for (int i = 0; i < rows * (columns / 2); ++i) {
    enemyBullets[i] = calloc(2, sizeof(int));
    if (!enemyBullets[i]) {
        fprintf(stderr, "Memory allocation failed for enemyBullets columns.\n");
        exit(EXIT_FAILURE);
    }
    enemyBullets[i][0] = -1; // Y-coordinate (invalid/inactive)
    enemyBullets[i][1] = -1; // X-coordinate (invalid/inactive)
  }
  return 0;
}

// Function: getUserInput
char getUserInput(void) {
  fflush(stdout);
  int c = fgetc(stdin); // Read a single character
  if (c == EOF) {
      return 0; // Or handle EOF appropriately
  }
  // Clear the rest of the line until newline or EOF
  int temp_c;
  while ((temp_c = fgetc(stdin)) != '\n' && temp_c != EOF);
  
  return (char)c;
}

// Function: clearShip
void clearShip(void) {
  // Ensure ship coordinates are within board bounds before accessing
  if (shipCords[0] >= 0 && shipCords[0] < columns && shipCords[1] >= 0 && shipCords[1] < rows) {
      board[shipCords[1]][shipCords[0]] = 0;
  }
}

// Function: setShip
void setShip(void) {
  // Ensure ship coordinates are within board bounds before accessing
  if (shipCords[0] >= 0 && shipCords[0] < columns && shipCords[1] >= 0 && shipCords[1] < rows) {
      if (board[shipCords[1]][shipCords[0]] == 5) { // If ship moves into an enemy bullet
        lost = 1; // Player loses
      }
      board[shipCords[1]][shipCords[0]] = 2; // Place ship (value 2)
  }
}

// Function: userFire
void userFire(void) {
  int bullet_slot = -1;
  for (int i = 0; i < rows; ++i) {
    if (userBullets[i][0] == -1) { // Find an inactive bullet slot (x-coord is -1)
      bullet_slot = i;
      break;
    }
  }

  // If a slot is found and ship is not at the top edge (y > 0)
  if (bullet_slot != -1 && shipCords[1] > 0) {
    userBullets[bullet_slot][0] = shipCords[0]; // Bullet starts at ship's X
    userBullets[bullet_slot][1] = shipCords[1]; // Bullet starts at ship's Y
  }
}

// Function: moveUser
void moveUser(void) {
  char input_char = getUserInput();
  
  clearShip(); // Clear ship from current position on the board

  if (input_char == 'w') {
    if (shipCords[1] > 0) { // Move up
      shipCords[1]--;
    }
  } else if (input_char == 's') {
    if (shipCords[1] < rows - 1) { // Move down
      shipCords[1]++;
    }
  } else if (input_char == 'a') {
    if (shipCords[0] > 0) { // Move left
      shipCords[0]--;
    }
  } else if (input_char == 'd') {
    if (shipCords[0] < columns - 1) { // Move right
      shipCords[0]++;
    }
  } else { // Any other key fires a bullet
    setShip(); // Re-set ship at current location (in case it moved into a bullet)
    userFire();
    return; // Skip the final setShip call for movement actions
  }
  setShip(); // Set ship at its new position (after movement)
}

// Function: updateBullets
void updateBullets(void) {
  // Update user bullets (move up)
  for (int i = 0; i < rows; ++i) {
    if (userBullets[i][0] != -1) { // If bullet is active
      if (userBullets[i][1] == 0) { // If bullet reaches the top row
        userBullets[i][0] = -1; // Deactivate bullet
        userBullets[i][1] = -1;
      } else {
        userBullets[i][1]--; // Move bullet up one row
      }
    }
  }

  // Update enemy bullets (move down)
  for (int i = 0; i < rows * (columns / 2); ++i) {
    if (enemyBullets[i][0] != -1) { // If bullet is active
      if (enemyBullets[i][0] == rows - 1) { // If bullet reaches the bottom row
        enemyBullets[i][0] = -1; // Deactivate bullet
        enemyBullets[i][1] = -1;
      } else {
        enemyBullets[i][0]++; // Move bullet down one row
      }
    }
  }
}

// Function: clearBullets
void clearBullets(void) {
  int bullet_x, bullet_y;
  
  // Clear user bullet representations from the board
  for (int i = 0; i < rows; ++i) {
    if (userBullets[i][0] != -1) { // If bullet is active
      bullet_x = userBullets[i][0];
      bullet_y = userBullets[i][1];
      // Check bounds before accessing board
      if (bullet_x >= 0 && bullet_x < columns && bullet_y >= 0 && bullet_y < rows) {
          if (board[bullet_y][bullet_x] == 4) { // If the cell contains a user bullet
            board[bullet_y][bullet_x] = 0; // Clear it
          }
      }
    }
  }

  // Clear enemy bullet representations from the board
  for (int i = 0; i < rows * (columns / 2); ++i) {
    if (enemyBullets[i][0] != -1) { // If bullet is active
      bullet_y = enemyBullets[i][0];
      bullet_x = enemyBullets[i][1];
      // Check bounds before accessing board
      if (bullet_x >= 0 && bullet_x < columns && bullet_y >= 0 && bullet_y < rows) {
          if (board[bullet_y][bullet_x] == 5) { // If the cell contains an enemy bullet
            board[bullet_y][bullet_x] = 0; // Clear it
          }
      }
    }
  }
}

// Function: setBullets
void setBullets(void) {
  int bullet_x, bullet_y;
  int enemy_row_idx, enemy_col_idx;
  int turn_offset = turnCounter / 20;

  // Set user bullets on the board and handle collisions
  for (int i = 0; i < rows; ++i) {
    if (userBullets[i][0] != -1) { // If bullet is active
      bullet_x = userBullets[i][0];
      bullet_y = userBullets[i][1];

      // Check bounds before accessing board
      if (bullet_x >= 0 && bullet_x < columns && bullet_y >= 0 && bullet_y < rows) {
          if (board[bullet_y][bullet_x] == 3) { // User bullet hits an enemy
            board[bullet_y][bullet_x] = 0; // Clear enemy representation from board

            // Calculate enemy array index from board coordinates
            enemy_row_idx = (bullet_y - turn_offset) / 2;
            enemy_col_idx = bullet_x / 2;

            if (enemy_row_idx >= 0 && enemy_row_idx < rowCount &&
                enemy_col_idx >= 0 && enemy_col_idx < columns / 2) {
                enemies[enemy_row_idx][enemy_col_idx] = 0; // Destroy enemy in logic array
            }

            // Deactivate user bullet
            userBullets[i][0] = -1;
            userBullets[i][1] = -1;
          } else {
            board[bullet_y][bullet_x] = 4; // Set user bullet representation on board
          }
      } else { // Bullet went out of bounds, deactivate
          userBullets[i][0] = -1;
          userBullets[i][1] = -1;
      }
    }
  }

  // Set enemy bullets on the board and handle collisions
  for (int i = 0; i < rows * (columns / 2); ++i) {
    if (enemyBullets[i][0] != -1) { // If bullet is active
      bullet_y = enemyBullets[i][0];
      bullet_x = enemyBullets[i][1];

      // Check bounds before accessing board
      if (bullet_x >= 0 && bullet_x < columns && bullet_y >= 0 && bullet_y < rows) {
          if (board[bullet_y][bullet_x] == 2) { // Enemy bullet hits the ship
            lost = 1; // Player loses
          } else {
            board[bullet_y][bullet_x] = 5; // Set enemy bullet representation on board
          }
      } else { // Bullet went out of bounds, deactivate
          enemyBullets[i][0] = -1;
          enemyBullets[i][1] = -1;
      }
    }
  }
}

// Function to print the game board
void printBoard(void) {
    clearScreen();
    printf("Turn: %d | Player: %s | Status: %s\n", turnCounter, playerName, lost == 0 ? "Playing" : (lost == 1 ? "Lost" : "Won"));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j) {
            switch (board[i][j]) {
                case 0: printf(" . "); break; // Empty space
                case 2: printf(" ^ "); break; // Ship
                case 3: printf(" X "); break; // Enemy
                case 4: printf(" | "); break; // User bullet
                case 5: printf(" v "); break; // Enemy bullet
                default: printf(" ? "); break; // Unknown state
            }
        }
        printf("\n");
    }
    printf("\n");
}

// Function to free all allocated dynamic memory
void cleanup(void) {
    if (playerName) free(playerName);
    if (shipCords) free(shipCords);

    if (enemies) {
        for (int i = 0; i < rowCount; ++i) {
            if (enemies[i]) free(enemies[i]);
        }
        free(enemies);
    }

    if (board) {
        for (int i = 0; i < rows; ++i) {
            if (board[i]) free(board[i]);
        }
        free(board);
    }

    if (userBullets) {
        for (int i = 0; i < rows; ++i) {
            if (userBullets[i]) free(userBullets[i]);
        }
        free(userBullets);
    }

    if (enemyBullets) {
        for (int i = 0; i < rows * (columns / 2); ++i) {
            if (enemyBullets[i]) free(enemyBullets[i]);
        }
        free(enemyBullets);
    }
}

// Main game loop
int main(void) {
    // Seed the random number generator
    // For true randomness: seedRandomGen((unsigned int)time(NULL), (unsigned int)getpid());
    seedRandomGen(12345, 67890); // Using fixed seeds for reproducible behavior

    if (setupNewGame() != 0) {
        fprintf(stderr, "Failed to set up new game. Exiting.\n");
        cleanup();
        return 1;
    }

    // Main game loop
    while (lost == 0) { // Continue as long as the player hasn't lost or won
        turnCounter++;

        clearBullets(); // Remove bullet representations from board
        clearEnemies(); // Remove enemy representations from board (they will be redrawn)
        clearShip();    // Remove ship representation from board

        moveUser();     // Handle player input (move ship or fire)
        updateBullets(); // Update bullet positions
        updateEnemies(); // Update enemy positions and make them fire

        setEnemies();    // Place enemies on the board at their new positions
        setBullets();    // Place bullets on the board at their new positions
        setShip();       // Place ship on the board at its new position

        printBoard(); // Render the current state of the board

        // Check for win condition (all enemies destroyed)
        bool all_enemies_destroyed = true;
        for (int i = 0; i < rowCount; ++i) {
            for (int j = 0; j < columns / 2; ++j) {
                if (enemies[i][j] != 0) {
                    all_enemies_destroyed = false;
                    break;
                }
            }
            if (!all_enemies_destroyed) break;
        }

        if (all_enemies_destroyed) {
            lost = -1; // Indicate a win
        }

        // Add a small delay for better readability during gameplay
        // #ifdef _WIN32
        //     Sleep(200); // Windows
        // #else
        //     usleep(200000); // Linux/Unix, 200 milliseconds
        // #endif
    }

    // Game over message
    if (lost == 1) {
        printf("Game Over! You lost, %s!\n", playerName);
    } else if (lost == -1) {
        printf("Congratulations, %s! You won!\n", playerName);
    }

    cleanup(); // Free all dynamically allocated memory
    return 0;
}