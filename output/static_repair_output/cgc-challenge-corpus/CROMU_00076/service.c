#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> // For _Bool

// Decompiled types mapping
typedef unsigned int uint;
// `undefined` and `undefined4` are decompilation artifacts.
// `undefined` is usually `void` or `char` if used for byte-level access.
// `undefined4` is usually `unsigned int` or `int`.
// `byte` is `unsigned char`.
// `bool` is `_Bool` or `bool` from `<stdbool.h>`.

// Global variables (mocked or assumed types/values)
char easteregg[] = "SUPER_SECRET_EGG"; // Example value
int eggindex = 0;
unsigned int page_index = 0;
// secret_page: Assumed to be a source of "randomness" based on page_index.
// Mock with some arbitrary data.
unsigned char secret_page[256] = {
    0x1A, 0x2B, 0x3C, 0x4D, 0x5E, 0x6F, 0x7A, 0x8B, 0x9C, 0x0D, 0x1E, 0x2F, 0x3A, 0x4B, 0x5C, 0x6D,
    0x7E, 0x8F, 0x9A, 0x0B, 0x1C, 0x2D, 0x3E, 0x4F, 0x5A, 0x6B, 0x7C, 0x8D, 0x9E, 0x0F, 0x1A, 0x2B,
    0x3C, 0x4D, 0x5E, 0x6F, 0x7A, 0x8B, 0x9C, 0x0D, 0x1E, 0x2F, 0x3A, 0x4B, 0x5C, 0x6D, 0x7E, 0x8F,
    0x9A, 0x0B, 0x1C, 0x2D, 0x3E, 0x4F, 0x5A, 0x6B, 0x7C, 0x8D, 0x9E, 0x0F, 0x1A, 0x2B, 0x3C, 0x4D,
    0x5E, 0x6F, 0x7A, 0x8B, 0x9C, 0x0D, 0x1E, 0x2F, 0x3A, 0x4B, 0x5C, 0x6D, 0x7E, 0x8F, 0x9A, 0x0B,
    0x1C, 0x2D, 0x3E, 0x4F, 0x5A, 0x6B, 0x7C, 0x8D, 0x9E, 0x0F, 0x1A, 0x2B, 0x3C, 0x4D, 0x5E, 0x6F,
    0x7A, 0x8B, 0x9C, 0x0D, 0x1E, 0x2F, 0x3A, 0x4B, 0x5C, 0x6D, 0x7E, 0x8F, 0x9A, 0x0B, 0x1C, 0x2D,
    0x3E, 0x4F, 0x5A, 0x6B, 0x7C, 0x8D, 0x9E, 0x0F, 0x1A, 0x2B, 0x3C, 0x4D, 0x5E, 0x6F, 0x7A, 0x8B,
    0x9C, 0x0D, 0x1E, 0x2F, 0x3A, 0x4B, 0x5C, 0x6D, 0x7E, 0x8F, 0x9A, 0x0B, 0x1C, 0x2D, 0x3E, 0x4F,
    0x5A, 0x6B, 0x7C, 0x8D, 0x9E, 0x0F, 0x1A, 0x2B, 0x3C, 0x4D, 0x5E, 0x6F, 0x7A, 0x8B, 0x9C, 0x0D,
    0x1E, 0x2F, 0x3A, 0x4B, 0x5C, 0x6D, 0x7E, 0x8F, 0x9A, 0x0B, 0x1C, 0x2D, 0x3E, 0x4F, 0x5A, 0x6B,
    0x7C, 0x8D, 0x9E, 0x0F, 0x1A, 0x2B, 0x3C, 0x4D, 0x5E, 0x6F, 0x7A, 0x8B, 0x9C, 0x0D, 0x1E, 0x2F,
    0x3A, 0x4B, 0x5C, 0x6D, 0x7E, 0x8F, 0x9A, 0x0B, 0x1C, 0x2D, 0x3E, 0x4F, 0x5A, 0x6B, 0x7C, 0x8D,
    0x9E, 0x0F, 0x1A, 0x2B, 0x3C, 0x4D, 0x5E, 0x6F, 0x7A, 0x8B, 0x9C, 0x0D, 0x1E, 0x2F, 0x3A, 0x4B,
    0x5C, 0x6D, 0x7E, 0x8F, 0x9A, 0x0B, 0x1C, 0x2D, 0x3E, 0x4F, 0x5A, 0x6B, 0x7C, 0x8D, 0x9E, 0x0F,
    0x1A, 0x2B, 0x3C, 0x4D, 0x5E, 0x6F, 0x7A, 0x8B, 0x9C, 0x0D, 0x1E, 0x2F, 0x3A, 0x4B, 0x5C, 0x6D,
};
void *queue_matrix = NULL; // Will point to a char array
// root: head of the queue, defined later as struct QueueNode *
char *names[] = { // Array of monster names
    "Goblin", "Orc", "Slime", "Dragon", "Phoenix", "Griffin", "Unicorn", "Sphinx",
    "Minotaur", "Hydra", "Golem", "Basilisk", NULL // NULL terminated
};

// Data Structures
struct QueueNode {
    int x;
    int y;
    struct QueueNode *next;
};
struct QueueNode *root = NULL; // Global queue root

struct Map {
    unsigned int width; // *param_1
    unsigned int height; // param_1[1]
    unsigned int startX; // param_1[2]
    unsigned int startY; // param_1[3]
    unsigned int endX; // param_1[4]
    unsigned int endY; // param_1[5]
    unsigned int currentX; // param_1[6]
    unsigned int currentY; // param_1[7]
    unsigned int markerX; // param_1[8]
    unsigned int markerY; // param_1[9]
    char *map_data; // param_1[10]
};

#define MAX_MONSTERS 5
struct Monster {
    char *name;         // *param_1
    unsigned int health;      // param_1[1] (current health)
    unsigned int max_health;  // param_1[2] (initial health / hit points)
    unsigned int power;       // param_1[3]
    unsigned int xp;          // param_1[4] (experience points)
    unsigned int level;       // param_1[5]
};

struct Player {
    char name[16];
    unsigned int level;
    unsigned int num_monsters;
    struct Monster *monsters[MAX_MONSTERS];
    // These fields are inferred from `check_egg`'s usage of `param_1 + 0x18`
    // and subsequent offsets. Assuming `param_1` points to player struct.
    unsigned int prize_field1; // e.g., Player->inventory_slot1
    unsigned int prize_field2; // e.g., Player->inventory_slot2
    unsigned int prize_field3; // e.g., Player->inventory_slot3
};

// Function Prototypes
void _terminate(void);
int receive(int *bytes_read, char *out_char); // Mocked receive function
void update_page_index(void);
char* select_name(void);
struct Monster* generate_monster(void);
struct Monster* generate_boss(void);
void print_monster(struct Monster *monster);
struct Monster* select_monster(struct Player *player);
int oneup_monster(struct Monster *monster);
void set_marker(unsigned int x, unsigned int y, struct Map *map, char marker_char);
void print_map(struct Map *map);
void initialize_queue_matrix(struct Map *map);
int find_path(unsigned int x, unsigned int y, struct Map *map);
int daboss(struct Player *player);
int fight(struct Player *player);
int capture_monster(struct Monster *monster, struct Player *player);
int capture_boss(struct Monster *boss, struct Player *player);
int change_monster_name(char **monster_name_ptr);


// Mock external functions
void _terminate(void) {
    printf("Program terminated.\n");
    exit(1);
}

// Mock receive function based on decompiled usage:
// It seems `receive()` reads one character and returns 0 on success, non-zero on error.
// `bytes_read` is updated by `receive` to indicate how many bytes were read (e.g., 1 for success).
// `out_char` is where the character read is stored.
int receive(int *bytes_read, char *out_char) {
    int c = getchar();
    if (c == EOF) {
        *bytes_read = 0;
        return 1; // Error
    }
    *out_char = (char)c;
    *bytes_read = 1;
    return 0; // Success
}

// Function: check_egg
void check_egg(struct Player *player, char param_2) {
  if (param_2 == easteregg[eggindex]) {
    eggindex = eggindex + 1;
  }
  else {
    eggindex = 0;
  }
  if (easteregg[eggindex] == '\0') {
    printf("YOU FOUND THE EGG!!!! Have a prize.\n");
    // Assuming player is the struct containing these fields
    player->prize_field1 = 99;
    player->prize_field2 = 99;
    player->prize_field3 = 99;
    eggindex = 0;
  }
  return;
}

// Function: read_line
unsigned int read_line(char *buffer, unsigned int max_len) {
  if (buffer == NULL) {
    printf("[ERROR] invalid arg\n");
    _terminate();
  }
  
  unsigned int current_len = 0;
  char c = '\0';
  int bytes_read = 0;

  while (current_len < max_len) {
    int ret = receive(&bytes_read, &c);
    if (ret != 0) { // Failed to read byte
      printf("[ERROR] Failed to read byte\n");
      _terminate();
    }
    if (bytes_read == 0) { // Error in receive
      printf("[ERROR] Error in receive\n");
      _terminate();
    }
    
    if (c == '\n') {
      break;
    }
    
    buffer[current_len] = c;
    current_len++;
  }
  
  // Null-terminate the string if space allows
  if (current_len < max_len) {
    buffer[current_len] = '\0';
  } else if (max_len > 0) { // Ensure buffer is null-terminated if it's not empty
    buffer[max_len - 1] = '\0';
  }
  
  return current_len;
}

// Function: read_line_u
// Modified to take buffer argument as original code implies it fills a buffer
unsigned int read_line_u(char *buffer) {
  if (buffer == NULL) {
    printf("[ERROR] read_line_u() invalid arg\n");
    _terminate();
  }
  
  unsigned int current_len = 0;
  char c = '\0';
  int bytes_read = 0;

  while (c != '\n') { // Loop until newline
    int ret = receive(&bytes_read, &c);
    if (ret != 0) { // Failed to read byte
      printf("[ERROR] Failed to read byte\n");
      _terminate();
    }
    if (bytes_read == 0) { // Error in receive
      printf("[ERROR] Error in receive\n");
      _terminate();
    }
    
    if (c != '\n') { // Only store if not newline
      buffer[current_len] = c;
      current_len++;
      // A real implementation would need a max_len check to prevent overflow
    }
  }
  buffer[current_len] = '\0'; // Null-terminate
  return current_len;
}

// Function: add_queue
void add_queue(int x, int y, int map_width) {
  // `map_width` is inferred from `param_3` in original `add_queue` call context (find_path)
  // `local_18 = param_1 + (param_3 + 1) * param_2;` -> `x + (map_width + 1) * y`
  // This looks like a 1D index calculation `x + y * map_width` but with `map_width + 1`
  // The use of `*(char *)(local_18 + queue_matrix)` suggests `queue_matrix` is a char array.
  // Let's assume queue_matrix stores visited status for BFS/DFS.
  
  // Adjusted index calculation based on common 2D array mapping and usage with queue_matrix
  // The original `param_1 + (param_3 + 1) * param_2` might be `x + (width) * y`
  // if `param_3` was width. `param_1` is x, `param_2` is y.
  // The original `find_path` passes `*param_3 - 1` for `param_3` which is `map->width - 1`.
  // This is confusing. Let's assume `queue_matrix` is indexed by `y * width + x`.
  // The expression `param_1 + (param_3 + 1) * param_2` seems incorrect for typical 2D indexing.
  // The most common 1D index for (x,y) in a 2D array [height][width] is `y * width + x`.
  // Let's use that for `queue_matrix` access.
  int index = y * map_width + x;

  if (((char *)queue_matrix)[index] != 1) { // If not visited
    struct QueueNode *new_node = (struct QueueNode *)malloc(sizeof(struct QueueNode));
    if (new_node == NULL) {
      printf("[ERROR] malloc() queue structure failed.\n");
      _terminate();
    }
    memset(new_node, 0, sizeof(struct QueueNode));
    new_node->x = x;
    new_node->y = y;
    new_node->next = NULL;

    if (root == NULL) {
      root = new_node;
    }
    else {
      struct QueueNode *current = root;
      while (current->next != NULL) {
        current = current->next;
      }
      current->next = new_node;
    }
    ((char *)queue_matrix)[index] = 1; // Mark as visited
  }
  return;
}

// Function: dequeue
struct QueueNode* dequeue(void) {
  struct QueueNode *node_to_dequeue = root;
  if (root != NULL) {
    root = root->next;
  }
  return node_to_dequeue;
}

// Function: check_adjacent
int check_adjacent(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) {
  if (x1 == x2) {
    if ((y2 == y1 + 1) || (y2 == y1 - 1)) {
      return 1;
    }
  }
  else if ((y1 == y2) && ((x2 == x1 + 1 || (x2 == x1 - 1)))) {
    return 1;
  }
  return 0;
}

// Function: print_map
void print_map(struct Map *map) {
  if (map == NULL) {
    printf("[ERROR] print_map() invalid argument.\n");
    return;
  }

  unsigned int map_size = map->width * map->height;
  char *display_buffer = (char *)malloc(map_size + map->height + 1); // +height for newlines, +1 for null terminator
  if (display_buffer == NULL) {
    printf("[ERROR] Failed to allocate map display buffer.\n");
    _terminate();
  }
  memset(display_buffer, 0, map_size + map->height + 1);

  unsigned int buffer_idx = 0;
  for (unsigned int i = 0; i < map_size; i++) {
    if ((i != 0) && (i % map->width == 0)) {
      display_buffer[buffer_idx++] = '\n';
    }
    
    // Original: *(char *)(local_10 + param_1[10])
    // map->map_data is param_1[10]
    if (map->map_data[i] == '\0') {
      display_buffer[buffer_idx++] = '.';
    }
    else {
      display_buffer[buffer_idx++] = map->map_data[i];
    }
  }
  display_buffer[buffer_idx++] = '\n'; // Final newline
  display_buffer[buffer_idx] = '\0';

  printf("%s", display_buffer);
  free(display_buffer);
  return;
}

// Function: find_path
int find_path(unsigned int x, unsigned int y, struct Map *map) {
  if (map == NULL) {
    return 0;
  }
  
  int adjacent = check_adjacent(x, y, map->endX, map->endY);
  if (adjacent == 1) {
    return 1;
  }
  else {
    // Add adjacent valid cells to queue
    if (y > 0) { // Up
      add_queue(x, y - 1, map->width);
    }
    if (x < map->width - 1) { // Right
      add_queue(x + 1, y, map->width);
    }
    if (y < map->height - 1) { // Down
      add_queue(x, y + 1, map->width);
    }
    if (x > 0) { // Left
      add_queue(x - 1, y, map->width);
    }
    
    struct QueueNode *current_node = dequeue();
    while (current_node != NULL) {
      int path_found = find_path(current_node->x, current_node->y, map);
      if (path_found == 1) {
        free(current_node);
        return 1;
      }
      free(current_node);
      current_node = dequeue();
    }
    return 0;
  }
}

// Function: update_page_index
void update_page_index(void) {
  page_index = (page_index + 3) & 0xfff;
  return;
}

// Function: place_marker
void place_marker(struct Map *map) {
  unsigned int attempts = 0;
  unsigned int index;
  
  // Place markerX, markerY
  do {
    map->markerX = (unsigned int)secret_page[page_index] % map->width;
    update_page_index();
    map->markerY = (unsigned int)secret_page[page_index] % map->height;
    update_page_index();
    index = map->markerY * map->width + map->markerX;
    attempts++;
  } while (map->map_data[index] != '\0' && attempts < 100);

  if (attempts == 100) {
    // If 100 attempts failed, find the first empty spot
    index = 0;
    while (map->map_data[index] != '\0' && index < (map->width * map->height)) {
      index++;
    }
    if (index == (map->width * map->height)) {
      printf("FAILED\n");
      print_map(map);
      _terminate();
    }
    map->markerY = index / map->width;
    map->markerX = index % map->width;
  }
  map->map_data[map->markerY * map->width + map->markerX] = '#'; // Use '#' for marker as in original
  return;
}

// Function: set_marker
void set_marker(unsigned int x, unsigned int y, struct Map *map, char marker_char) {
  if (map != NULL && map->map_data != NULL) {
    map->map_data[y * map->width + x] = marker_char;
  }
  return;
}

// Function: initialize_map
void initialize_map(struct Map *map) {
  if (map == NULL) {
    printf("[ERROR] initialize_map() invalid argument.\n");
    _terminate();
  }
  
  unsigned int map_size = map->width * map->height;
  map->map_data = (char *)malloc(map_size);
  if (map->map_data == NULL) {
    printf("[ERROR] Failed to allocate map data.\n");
    _terminate();
  }
  memset(map->map_data, 0, map_size); // Initialize map data to '\0'

  map->startX = 0;
  map->startY = 0;
  map->endX = 0;
  map->endY = 0;

  // Generate start and end points ensuring they are not adjacent
  while (true) {
    map->startX = (unsigned int)secret_page[page_index] % map->width;
    update_page_index();
    map->startY = (unsigned int)secret_page[page_index] % map->height;
    update_page_index();
    map->endX = (unsigned int)secret_page[page_index] % map->width;
    update_page_index();
    map->endY = (unsigned int)secret_page[page_index] % map->height;
    update_page_index();

    if (!((map->startX == map->endX || (map->startX - 1 == map->endX) || (map->startX + 1 == map->endX)) &&
          (map->startY == map->endY || (map->startY + 1 == map->endY) || (map->startY - 1 == map->endY)))) {
        break; // Found non-adjacent points
    }
  }
  
  // Set start ('@') and end ('X') markers
  set_marker(map->startX, map->startY, map, '@');
  set_marker(map->endX, map->endY, map, 'X');
  
  map->currentX = map->startX;
  map->currentY = map->startY;
  return;
}

// Function: initialize_queue_matrix
void initialize_queue_matrix(struct Map *map) {
  if (map == NULL) {
    printf("[ERROR] initialize_queue_matrix() invalid argument.\n");
    _terminate();
  }

  unsigned int matrix_size = map->width * map->height;
  if (queue_matrix != NULL) {
    free(queue_matrix);
  }
  queue_matrix = malloc(matrix_size);
  if (queue_matrix == NULL) {
    printf("[ERROR] Failed to allocate queue matrix\n");
    _terminate();
  }
  memset(queue_matrix, 0, matrix_size); // Initialize to 0 (unvisited)

  // Mark blocked areas in queue_matrix
  for (unsigned int i = 0; i < matrix_size; i++) {
    if (map->map_data[i] != '\0') { // If it's a marker ('#' for egg, '@' for start, 'X' for end)
      ((char *)queue_matrix)[i] = 1; // Mark as visited/blocked for pathfinding
    }
  }
  return;
}

// Function: generate_map
struct Map* generate_map(unsigned int width, unsigned int height) {
  struct Map *new_map = (struct Map *)malloc(sizeof(struct Map));
  if (new_map == NULL) {
    printf("[ERROR] Failed to allocate map structure\n");
    _terminate();
  }
  memset(new_map, 0, sizeof(struct Map));

  new_map->width = width;
  new_map->height = height;
  
  initialize_map(new_map);

  printf("Width: %d Height: %d\n", new_map->width, new_map->height);
  printf("StartX: %d StartY: %d\n", new_map->startX, new_map->startY);
  printf("EndX: %d EndY: %d\n\n", new_map->endX, new_map->endY);

  int path_found_loop = 0;
  while (path_found_loop == 0) {
    place_marker(new_map);
    initialize_queue_matrix(new_map);
    
    // Check if a path exists from current position to end position
    // For path generation, we typically want to check from start to end,
    // not current position to end. Let's assume this is correct for generating a solvable map.
    // The original code calls find_path(map->currentX, map->currentY, map)
    int path_exists = find_path(new_map->currentX, new_map->currentY, new_map);
    
    if (path_exists != 1) {
      // If no path, remove the placed marker and retry
      new_map->map_data[new_map->markerY * new_map->width + new_map->markerX] = '\0';
    } else {
      path_found_loop = 1; // Path found, exit loop
    }
    
    // Clear the queue for the next iteration of marker placement/pathfinding
    while (root != NULL) {
      struct QueueNode *temp = root;
      root = root->next;
      free(temp);
    }
    root = NULL; // Ensure root is NULL
  }
  
  free(queue_matrix); // Free queue matrix as it's no longer needed after map generation
  queue_matrix = NULL;

  print_map(new_map);
  return new_map;
}

// Function: select_monster
struct Monster* select_monster(struct Player *player) {
  if (player == NULL) {
    return NULL;
  }
  
  _Bool has_living_monsters = false;
  for (unsigned int i = 0; i < player->num_monsters; i++) {
    if (player->monsters[i] != NULL && player->monsters[i]->health > 0) {
      has_living_monsters = true;
      break;
    }
  }

  if (!has_living_monsters) {
    return NULL;
  }

  int selection_made = 0;
  unsigned int chosen_idx = 0;
  char input_buffer[4]; // For "Selection: " input

  printf("Monsters: \n");
  for (unsigned int i = 0; i < player->num_monsters; i++) {
    if (player->monsters[i] != NULL) { // Only print if slot is not empty
        printf("\t%d} \n", i + 1);
        printf("\tType: %s\n", player->monsters[i]->name);
        printf("\tLevel: %d\n", player->monsters[i]->level);
        printf("\tHealth: %d\n", player->monsters[i]->health);
        printf("\tPower: %d\n\n", player->monsters[i]->power);
    }
  }

  while (selection_made == 0) {
    printf("Selection: ");
    read_line(input_buffer, sizeof(input_buffer));
    chosen_idx = atoi(input_buffer); // Convert 1-based index to 0-based
    
    if (chosen_idx < 1 || chosen_idx > player->num_monsters) {
      printf("bad choice: %s\n", input_buffer);
    }
    else if (player->monsters[chosen_idx - 1] == NULL) {
      printf("bad choice: %s\n", input_buffer); // Slot is empty
    }
    else if (player->monsters[chosen_idx - 1]->health < 1) {
      printf("he dead\n");
    }
    else {
      selection_made = 1;
    }
  }
  return player->monsters[chosen_idx - 1];
}

// Function: reset_monsters
void reset_monsters(struct Player *player) {
  if (player != NULL) {
    for (unsigned int i = 0; i < player->num_monsters; i++) {
        if (player->monsters[i] != NULL) {
            player->monsters[i]->health = player->monsters[i]->max_health;
        }
    }
  }
  return;
}

// Function: print_monster
void print_monster(struct Monster *monster) {
  if (monster != NULL) {
    printf("\tType: %s\n", monster->name);
    printf("\tLevel: %d\n", monster->level);
    printf("\tHealth: %d\n", monster->health);
    printf("\tHit Points: %d\n", monster->max_health); // Original uses param_1[2] for this
    printf("\tPower: %d\n\n", monster->power);
  }
  return;
}

// Function: oneup_monster
int oneup_monster(struct Monster *monster) {
  if (monster == NULL) {
    return 0;
  }
  
  monster->xp = monster->xp + 1;
  // Level up every 15 XP (original: ((uint)param_1[4] / 0xf) * 0xf)
  if (monster->xp >= 15) { // Simplified level up logic
    printf("%s gained a level\n", monster->name);
    monster->max_health = monster->max_health + 1; // Hit Points
    monster->power = monster->power + 1;
    monster->health = monster->max_health; // Restore health on level up
    monster->level = monster->level + 1;
    monster->xp = 0; // Reset XP
  }
  return 1;
}

// Function: generate_boss
struct Monster* generate_boss(void) {
  struct Monster *boss = (struct Monster *)malloc(sizeof(struct Monster));
  if (boss == NULL) {
    printf("[ERROR] Failed to allocate boss monster structure\n");
    _terminate();
  }
  memset(boss, 0, sizeof(struct Monster));
  
  boss->name = select_name();
  // Health: random between 9 and 18 (original: ((uint)*(byte *)(page_index + secret_page) + (uint)(*(byte *)(page_index + secret_page) / 10) * -10 & 0xff) + 9)
  boss->health = (secret_page[page_index] % 10) + 9;
  boss->max_health = boss->health;
  update_page_index();
  // Power: random between 4 and 9 (original: ((uint)*(byte *)(page_index + secret_page) + (uint)(*(byte *)(page_index + secret_page) / 6) * -6 & 0xff) + 4)
  boss->power = (secret_page[page_index] % 6) + 4;
  update_page_index();
  boss->level = 4; // Boss starts at level 4
  boss->xp = 0; // Boss has no XP

  return boss;
}

// Function: change_monster_name
int change_monster_name(char **monster_name_ptr) {
  char new_name_buffer[32]; // Max 31 chars + null
  memset(new_name_buffer, 0, sizeof(new_name_buffer));
  
  printf("New name: ");
  unsigned int name_len = read_line_u(new_name_buffer);
  
  char *new_name = (char *)malloc(name_len + 1);
  if (new_name == NULL) {
    printf("[ERROR] Failed to malloc name buffer\n");
    _terminate();
  }
  memset(new_name, 0, name_len + 1);
  memcpy(new_name, new_name_buffer, name_len);
  
  // Free old name if it was dynamically allocated
  if (*monster_name_ptr != NULL && *monster_name_ptr != names[0]) { // Check if it's from `names` array
      free(*monster_name_ptr);
  }
  *monster_name_ptr = new_name;
  return 1;
}

// Function: capture_boss
int capture_boss(struct Monster *boss, struct Player *player) {
  if (boss == NULL || player == NULL) {
    return 0;
  }
  
  char input_buffer[4];
  printf("capture monster? (y/n): ");
  read_line(input_buffer, sizeof(input_buffer));
  
  if (input_buffer[0] == 'y') {
    printf("update boss name? (y/n): ");
    read_line(input_buffer, sizeof(input_buffer));
    if (input_buffer[0] == 'y') {
      change_monster_name(&(boss->name));
    }

    if (player->num_monsters < MAX_MONSTERS) {
      player->monsters[player->num_monsters] = boss;
      player->num_monsters++;
      return 1;
    }
    else {
      printf("your cart is full.\n");
      for (unsigned int i = 0; i < MAX_MONSTERS; i++) {
        printf("%d} \n", i + 1);
        print_monster(player->monsters[i]);
      }
      printf("*********************************\n");
      boss->health = boss->max_health; // Restore boss health before offering replacement
      print_monster(boss);
      printf("*********************************\n");
      
      printf("replace one of yours? (y/n): ");
      read_line(input_buffer, sizeof(input_buffer));
      if (input_buffer[0] == 'y') {
        printf("which one: ");
        read_line(input_buffer, sizeof(input_buffer));
        unsigned int selection = atoi(input_buffer);
        
        if (selection == 0 || selection > MAX_MONSTERS) {
          printf("invalid\n");
          // Free the boss if not captured
          if (boss->name != NULL && boss->name != names[0]) free(boss->name);
          free(boss);
          return 0;
        }
        else {
          // Free the monster being replaced
          struct Monster *replaced_monster = player->monsters[selection - 1];
          if (replaced_monster != NULL) {
              if (replaced_monster->name != NULL && replaced_monster->name != names[0]) free(replaced_monster->name);
              free(replaced_monster);
          }
          player->monsters[selection - 1] = boss;
          return 1;
        }
      }
      else {
        // Free the boss if not captured
        if (boss->name != NULL && boss->name != names[0]) free(boss->name);
        free(boss);
        return 0;
      }
    }
  }
  else {
    // Free the boss if not captured
    if (boss->name != NULL && boss->name != names[0]) free(boss->name);
    free(boss);
    return 0;
  }
}

// Function: daboss
int daboss(struct Player *player) {
  if (player == NULL) {
    return 0;
  }
  
  struct Monster *boss = generate_boss();
  reset_monsters(player); // Reset player monsters' health
  
  printf("\nDUN DUN DUUUUUUUUUUUUUUN\n");
  printf("You have reached the boss!!!!!\n\n");
  print_monster(boss);
  
  while (boss->health > 0) {
    struct Monster *selected_monster = select_monster(player);
    if (selected_monster == NULL) {
      printf("You have no living monsters. You lose.\n");
      if (boss->name != NULL && boss->name != names[0]) free(boss->name);
      free(boss);
      return 0; // Player lost
    }
    
    // Player's monster attacks boss
    unsigned int player_damage = secret_page[page_index] % selected_monster->power;
    update_page_index();
    boss->health -= player_damage;
    printf("You hit for %d. %d left\n", player_damage, boss->health);
    oneup_monster(selected_monster); // Player monster gains XP
    
    if (boss->health < 1) {
      printf("You destroyed the boss!!!!\n");
      reset_monsters(player);
      capture_boss(boss, player);
      return 1; // Player won
    }
    
    // Boss attacks player's monster
    unsigned int boss_damage = secret_page[page_index] % boss->power;
    update_page_index();
    printf("%s hits %s for %d\n", boss->name, selected_monster->name, boss_damage);
    selected_monster->health -= boss_damage;
    
    if (selected_monster->health < 1) {
      printf("%s was knocked out\n", selected_monster->name);
    }
  }
  return 1; // Should not reach here if loop condition is correct
}

// Function: capture_monster
int capture_monster(struct Monster *monster, struct Player *player) {
  if (monster == NULL || player == NULL) {
    return 0;
  }
  
  char input_buffer[4];
  printf("capture monster? (y/n): ");
  read_line(input_buffer, sizeof(input_buffer));
  
  if (input_buffer[0] == 'y') {
    if (player->num_monsters < MAX_MONSTERS) {
      player->monsters[player->num_monsters] = monster;
      player->num_monsters++;
      return 1;
    }
    else {
      printf("your cart is full.\n");
      for (unsigned int i = 0; i < MAX_MONSTERS; i++) {
        printf("%d} \n", i + 1);
        print_monster(player->monsters[i]);
      }
      printf("*********************************\n");
      print_monster(monster);
      printf("*********************************\n");
      
      printf("replace one of yours? (y/n): ");
      read_line(input_buffer, sizeof(input_buffer));
      if (input_buffer[0] == 'y') {
        printf("which one: ");
        read_line(input_buffer, sizeof(input_buffer));
        unsigned int selection = atoi(input_buffer);
        
        if (selection == 0 || selection > MAX_MONSTERS) {
          printf("invalid\n");
          // Free the monster if not captured
          if (monster->name != NULL && monster->name != names[0]) free(monster->name);
          free(monster);
          return 0;
        }
        else {
          // Free the monster being replaced
          struct Monster *replaced_monster = player->monsters[selection - 1];
          if (replaced_monster != NULL) {
              if (replaced_monster->name != NULL && replaced_monster->name != names[0]) free(replaced_monster->name);
              free(replaced_monster);
          }
          player->monsters[selection - 1] = monster;
          return 1;
        }
      }
      else {
        // Free the monster if not captured
        if (monster->name != NULL && monster->name != names[0]) free(monster->name);
        free(monster);
        return 0;
      }
    }
  }
  else {
    // Free the monster if not captured
    if (monster->name != NULL && monster->name != names[0]) free(monster->name);
    free(monster);
    return 0;
  }
}

// Function: fight
int fight(struct Player *player) {
  if (player == NULL) {
    return 0;
  }
  
  reset_monsters(player); // Reset player's monsters' health
  struct Monster *enemy_monster = generate_monster();
  
  printf("You are being attacked!!!\n");
  print_monster(enemy_monster);
  
  while (enemy_monster->health > 0) {
    struct Monster *selected_monster = select_monster(player);
    if (selected_monster == NULL) {
      printf("You have no living monsters. You lose.\n");
      if (enemy_monster->name != NULL && enemy_monster->name != names[0]) free(enemy_monster->name);
      free(enemy_monster);
      return 0; // Player lost
    }
    
    // Player's monster attacks enemy
    unsigned int player_damage = secret_page[page_index] % selected_monster->power;
    update_page_index();
    enemy_monster->health -= player_damage;
    printf("You hit for %d. %d left\n", player_damage, enemy_monster->health);
    oneup_monster(selected_monster); // Player monster gains XP
    
    if (enemy_monster->health < 1) {
      printf("You knocked out %s\n", enemy_monster->name);
      reset_monsters(player);
      capture_monster(enemy_monster, player);
      return 1; // Player won
    }
    
    // Enemy attacks player's monster
    unsigned int enemy_damage = secret_page[page_index] % enemy_monster->power;
    update_page_index();
    printf("%s hits %s for %d\n", enemy_monster->name, selected_monster->name, enemy_damage);
    selected_monster->health -= enemy_damage;
    
    if (selected_monster->health < 1) {
      printf("%s was knocked out\n", selected_monster->name);
    }
  }
  return 1; // Should not reach here if loop condition is correct
}

// Function: movement_loop
int movement_loop(struct Map *map, struct Player *player) {
  if (map == NULL || player == NULL) {
    return 0;
  }
  
  int game_over = 0;
  char direction_input[4]; // Buffer for 'u', 'd', 'l', 'r' + newline + null
  
  while (game_over == 0) {
    memset(direction_input, 0, sizeof(direction_input));
    printf("Move (u/d/l/r): ");
    read_line(direction_input, sizeof(direction_input));
    
    check_egg(player, direction_input[0]); // Check for Easter egg with player context
    
    unsigned int old_x = map->currentX;
    unsigned int old_y = map->currentY;
    
    switch (direction_input[0]) {
      case 'u': // Up
        if (map->currentY == 0) {
          printf("off map\n");
        }
        else if (map->map_data[(map->currentY - 1) * map->width + map->currentX] == '#') {
          printf("blocked\n");
        }
        else {
          set_marker(old_x, old_y, map, '\0'); // Clear old marker
          map->currentY--;
          set_marker(map->currentX, map->currentY, map, '@'); // Set new marker
        }
        break;
      case 'd': // Down
        if (map->currentY == map->height - 1) {
          printf("off map\n");
        }
        else if (map->map_data[(map->currentY + 1) * map->width + map->currentX] == '#') {
          printf("blocked\n");
        }
        else {
          set_marker(old_x, old_y, map, '\0');
          map->currentY++;
          set_marker(map->currentX, map->currentY, map, '@');
        }
        break;
      case 'l': // Left
        if (map->currentX == 0) {
          printf("off map\n");
        }
        else if (map->map_data[map->currentY * map->width + (map->currentX - 1)] == '#') {
          printf("blocked\n");
        }
        else {
          set_marker(old_x, old_y, map, '\0');
          map->currentX--;
          set_marker(map->currentX, map->currentY, map, '@');
        }
        break;
      case 'r': // Right
        if (map->currentX == map->width - 1) {
          printf("off map\n");
        }
        else if (map->map_data[map->currentY * map->width + (map->currentX + 1)] == '#') {
          printf("blocked\n");
        }
        else {
          set_marker(old_x, old_y, map, '\0');
          map->currentX++;
          set_marker(map->currentX, map->currentY, map, '@');
        }
        break;
      default:
        printf("[ERROR] Invalid direction: %c\n", direction_input[0]);
        break;
    }
    
    print_map(map); // Always print map after movement attempt

    if (map->currentX == map->endX && map->currentY == map->endY) {
      printf("reached the end\n");
      int boss_result = daboss(player);
      if (boss_result == 1) {
        printf("You won!!!\n");
      }
      else {
        printf("You failed!!!\n");
      }
      game_over = 1;
    }
    else {
      // Random encounter check (original: (byte)(*(byte *)(page_index + secret_page) + (byte)((ushort)((ushort)*(byte *)(page_index + secret_page) * 0x29) >> 0xc) * -100) < 10)
      // Simplified: 10% chance of encounter
      if ((secret_page[page_index] % 100) < 10) {
        update_page_index();
        int fight_result = fight(player);
        if (fight_result == 1) {
          player->level++;
          printf("player gains a level. now %d\n", player->level);
        }
        print_map(map); // Print map again after fight
      }
      else {
        update_page_index(); // Still consume a random byte even if no fight
      }
    }
  }
  return 1;
}

// Function: select_name
char* select_name(void) {
  unsigned char rand_val = secret_page[page_index];
  update_page_index();
  
  unsigned int name_idx = 0;
  for (unsigned int i = 0; i < rand_val; i++) {
    name_idx++;
    if (names[name_idx] == NULL) { // Loop back to start if end of list
      name_idx = 0;
    }
  }
  return names[name_idx];
}

// Function: generate_monster
struct Monster* generate_monster(void) {
  struct Monster *monster = (struct Monster *)malloc(sizeof(struct Monster));
  if (monster == NULL) {
    printf("[ERROR] Failed to allocate monster structure\n");
    _terminate();
  }
  memset(monster, 0, sizeof(struct Monster));
  
  monster->name = select_name();
  // Health: random between 4 and 13 (original: ((uint)*(byte *)(page_index + secret_page) + (uint)(*(byte *)(page_index + secret_page) / 10) * -10 & 0xff) + 4)
  monster->health = (secret_page[page_index] % 10) + 4;
  monster->max_health = monster->health;
  update_page_index();
  // Power: random between 2 and 7 (original: ((uint)*(byte *)(page_index + secret_page) + (uint)(*(byte *)(page_index + secret_page) / 6) * -6 & 0xff) + 2)
  monster->power = (secret_page[page_index] % 6) + 2;
  update_page_index();
  monster->level = 1; // Monsters start at level 1
  monster->xp = 0; // Monsters start with 0 XP

  return monster;
}

// Function: generate_player
struct Player* generate_player(void) {
  char player_name_buffer[16]; // Max 15 chars + null
  memset(player_name_buffer, 0, sizeof(player_name_buffer));
  
  printf("Enter your name|| ");
  unsigned int name_len = read_line(player_name_buffer, sizeof(player_name_buffer));
  if (name_len == 0) {
    strncpy(player_name_buffer, "Player One", sizeof(player_name_buffer) - 1);
    player_name_buffer[sizeof(player_name_buffer) - 1] = '\0';
  }
  
  struct Player *player = (struct Player *)malloc(sizeof(struct Player));
  if (player == NULL) {
    printf("[ERROR] Failed to malloc player structure\n");
    _terminate();
  }
  memset(player, 0, sizeof(struct Player));
  
  strncpy(player->name, player_name_buffer, sizeof(player->name) - 1);
  player->name[sizeof(player->name) - 1] = '\0';
  
  // Generate initial 3 monsters
  for (unsigned int i = 0; i < 3; i++) {
    player->monsters[player->num_monsters] = generate_monster();
    player->num_monsters++;
  }
  
  player->level = 1;
  return player;
}

// Function: print_player
void print_player(struct Player *player) {
  if (player != NULL) {
    printf("Name: %s\n", player->name);
    printf("Level: %d\n", player->level);
    printf("Monsters: \n");
    for (unsigned int i = 0; i < player->num_monsters; i++) {
      if (player->monsters[i] != NULL) {
        printf("\tType: %s\n", player->monsters[i]->name);
        printf("\tHealth: %d\n", player->monsters[i]->health);
        printf("\tPower: %d\n\n", player->monsters[i]->power);
      }
    }
  }
  return;
}

// Main function (inferred structure)
int main() {
    // Seed random number generator (for testing secret_page, if not using actual random source)
    // srand(time(NULL));

    printf("Welcome to the adventure!\n");

    struct Player *player = generate_player();
    print_player(player);

    struct Map *game_map = generate_map(10, 10); // Example map size

    // Start the movement loop
    movement_loop(game_map, player);

    // Cleanup resources
    if (game_map != NULL) {
        if (game_map->map_data != NULL) {
            free(game_map->map_data);
        }
        free(game_map);
    }
    if (player != NULL) {
        for (unsigned int i = 0; i < player->num_monsters; i++) {
            if (player->monsters[i] != NULL) {
                // Free monster name if it was dynamically allocated (not from `names` array)
                bool is_static_name = false;
                for (int j = 0; names[j] != NULL; j++) {
                    if (player->monsters[i]->name == names[j]) {
                        is_static_name = true;
                        break;
                    }
                }
                if (!is_static_name && player->monsters[i]->name != NULL) {
                    free(player->monsters[i]->name);
                }
                free(player->monsters[i]);
            }
        }
        free(player);
    }
    
    printf("Game Over. Thanks for playing!\n");

    return 0;
}