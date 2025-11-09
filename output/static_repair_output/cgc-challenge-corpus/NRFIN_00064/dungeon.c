#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h> // For srand/time

// Define types based on common decompilation patterns
typedef uint8_t byte;
typedef uint32_t uint;

// Global variables from the snippet (stubs)
void *flag_buf = NULL; // Used in addMove
byte DAT_4347c000[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J' }; // Example data
const char DAT_000163f5[] = " to get the treasure!";

// Global for the main dungeon pointer, used implicitly by many functions
int main_dungeon_ptr = 0;

// Forward declarations for functions that call each other
void sendCurrentDungeonView(int dungeon_ptr);
void addRoom(int room_ptr, char *map_data, uint start_col, int start_row, int param_5);
void destroyObject(void *obj);
int makeObject(char type, int p2, int p3, int x, int y, int p6, int p7);
int getObjectById(int dungeon_ptr, int object_id);
int getRoom(int dungeon_ptr, int room_index);
int getObjectByCoord(int dungeon_ptr, uint x, uint y);
void setObjectByCoord(int dungeon_ptr, uint x, int y, int object_ptr);
int moveEnemies(int dungeon_ptr, int current_moves);
int checkFloor(int dungeon_ptr, int player_object_ptr);
int moveDown(int dungeon_ptr, int player_object_ptr);
int moveUp(int dungeon_ptr, int player_object_ptr);
int moveLeft(int dungeon_ptr, int player_object_ptr);
int moveRight(int dungeon_ptr, int player_object_ptr);
int insertNewScore(int head_score_ptr, int new_score_ptr);
void addHighScore(int game_data_ptr, int score);
void playerWon(int game_data_ptr);

// Stubs for missing functions
void _terminate(int status) {
    fprintf(stderr, "Program terminated with status %d\n", status);
    exit(status);
}

int transmit_all(int fd, const void *buf, size_t count, uint flags) {
    // Mimic writing to stdout/stderr
    if (fd == 1) {
        fprintf(stdout, "TRANSMIT (stdout): %.*s", (int)count, (char*)buf);
    } else if (fd == 2) {
        fprintf(stderr, "TRANSMIT (stderr): %.*s", (int)count, (char*)buf);
    } else {
        fprintf(stderr, "TRANSMIT (unknown FD %d): %.*s", fd, (int)count, (char*)buf);
    }
    return 0; // Success
}

int read_until_delim_or_n(int fd, char *buffer, size_t n, char delim) {
    // Mimic reading from stdin
    if (fd == 0) {
        if (fgets(buffer, n, stdin) == NULL) {
            return -1; // Error
        }
        buffer[strcspn(buffer, "\n")] = 0; // Remove newline if present
        return 0; // Success
    }
    return -1; // Unsupported FD
}

long random(void) {
    return rand(); // Using standard C rand()
}

// Function: sendKillMessage
void sendKillMessage(char *message) {
  if (transmit_all(1, message, strlen(message), 0x1100c) != 0) {
    _terminate(2);
  }
}

// Function: makeObject
// Object structure:
// +0: char display_char
// +4: int type_id
// +8: int* ptr_to_coords (points to an array of 2 ints: x, y)
// +0xc: int* ptr_to_state (points to an array of 2 ints: state_val1, state_val2)
// +0x10: int moves_or_last_turn_or_hp
int makeObject(char display_char, int type_id, int state_val1, int x, int y, int state_val2, int moves_hp) {
  int *obj_ptr = (int *)malloc(0x14); // Object itself is 20 bytes
  if (obj_ptr == NULL) {
    _terminate(1);
  }

  int *coords_ptr = (int *)malloc(8); // Coordinates array (2 ints)
  if (coords_ptr == NULL) {
    free(obj_ptr);
    _terminate(1);
  }

  int *state_ptr = (int *)malloc(8); // State array (2 ints)
  if (state_ptr == NULL) {
    free(coords_ptr);
    free(obj_ptr);
    _terminate(1);
  }

  *(char *)obj_ptr = display_char; // Offset 0
  obj_ptr[1] = type_id;             // Offset 4
  obj_ptr[2] = (int)coords_ptr;     // Offset 8
  obj_ptr[3] = (int)state_ptr;      // Offset 0xc
  obj_ptr[4] = moves_hp;            // Offset 0x10

  coords_ptr[0] = x;
  coords_ptr[1] = y;
  state_ptr[0] = state_val1;
  state_ptr[1] = state_val2;

  return (int)obj_ptr;
}

// Function: destroyObject
void destroyObject(void *obj_ptr) {
    if (obj_ptr == NULL) return;

    int *obj = (int *)obj_ptr;
    free((void *)obj[2]); // Free coordinates array
    free((void *)obj[3]); // Free state array
    free(obj_ptr);       // Free object itself
}

// Function: extendDungeon
// Dungeon structure:
// +0..+9: char move_chars[10]
// +0xc: int room_list_head_ptr
// +0x10: int move_history_buf_ptr
// +0x14: int high_score_list_head_ptr
// Room structure:
// +0..+0x35f: int object_pointers[9*24]
// +0x360: int next_room_ptr
// +0x29c: int room_specific_object_ptr (used in extendDungeon)
void extendDungeon(int dungeon_ptr, int room_index, char move_char, int current_moves) {
  void *new_room_ptr = malloc(0x364);
  if (new_room_ptr == NULL) {
    _terminate(1);
  }
  *(int *)((int)new_room_ptr + 0x360) = 0; // Initialize next_room_ptr

  int room_type_idx = 2; // Default room type index
  if (move_char == *(char *)(dungeon_ptr + 1)) { room_type_idx = 0; }
  else if (move_char == *(char *)(dungeon_ptr + 2)) { room_type_idx = 1; }
  else if (move_char == *(char *)(dungeon_ptr + 3)) { room_type_idx = 0; }
  else if (move_char == *(char *)(dungeon_ptr + 4)) { room_type_idx = 1; }
  else if (move_char == *(char *)(dungeon_ptr + 5)) { room_type_idx = 0; }
  else if (move_char == *(char *)(dungeon_ptr + 6)) {
    room_type_idx = (current_moves < 0x12d) ? 1 : 2;
  }

  const char *map_strings[] = {
    "------------------------|                    v ||       --             ||     -     -          ||            |        v||   -        |  -      |             | -       ||------&     --&      -|------------------------------------------------|&       &            v||--------------  |-    ||                |     ||---    ---------|     ||   ---          |     |       ---       |     ||                |&  --|------------------------------------------------|                     @||       -           ---||     v- -        v-   ||     -   -       -    ||    -     -     -     |    -       -   -      ||  &|       |&  |      |------------------------!H",
    "------------------------|R                   v ||-                     || -                   v||  -                   ||   -                  ||    -                 ||------&     --       -|------------------------",
    "------------------------|R                   v ||-                     || -                   v||  -                   ||   -                  ||    -                 ||------&     --       -|------------------------"
  };
  addRoom((int)new_room_ptr, (char*)map_strings[room_type_idx], 0, 0, 0);

  int current_room_list_ptr = *(int *)(dungeon_ptr + 0xc); // Head of room list
  if (current_room_list_ptr == 0) {
      *(int *)(dungeon_ptr + 0xc) = (int)new_room_ptr;
  } else {
      while (*(int *)(current_room_list_ptr + 0x360) != 0) {
          current_room_list_ptr = *(int *)(current_room_list_ptr + 0x360);
      }
      *(int *)(current_room_list_ptr + 0x360) = (int)new_room_ptr;
  }

  // Original code destroys an object related to the last room.
  // Assuming it's a previous room's specific object that is being replaced.
  // This logic is a bit unclear, but preserving the call.
  // destroyObject((void *)(*(int *)(current_room_list_ptr + 0x29c)));

  // Create a new object (e.g., exit point for the room) and assign it
  int room_obj = makeObject(0x20, 7, 6, room_index * 0x18 -1, 0, 0, current_moves);
  *(int *)(current_room_list_ptr + 0x29c) = room_obj;
}

// Function: addMove
void addMove(int dungeon_ptr, int player_object_ptr, char move_char) {
  int *dungeon_data = (int *)dungeon_ptr;
  int *player_obj = (int *)player_object_ptr;

  player_obj[4]++; // Increment moves counter (offset 0x10)

  // Extend move history buffer if needed
  if ((player_obj[4] & 0x3f) == 0) { // If moves count is a multiple of 64
    int old_moves_buf_ptr = dungeon_data[4]; // dungeon_ptr + 0x10
    uint current_block_idx = player_obj[4] >> 6;
    size_t new_size = (current_block_idx + 1) * 0x100;

    void *new_moves_buf = malloc(new_size);
    if (new_moves_buf == NULL) {
      _terminate(1);
    }
    bzero(new_moves_buf, new_size);

    if (old_moves_buf_ptr != 0) {
      memcpy(new_moves_buf, (void *)old_moves_buf_ptr, strlen((char *)old_moves_buf_ptr));
      free((void *)old_moves_buf_ptr);
    }
    dungeon_data[4] = (int)new_moves_buf;

    // Call extendDungeon
    extendDungeon(dungeon_ptr, current_block_idx, move_char, player_obj[4]);

    if (flag_buf == NULL) {
      flag_buf = malloc(0x200);
      if (flag_buf == NULL) {
        _terminate(1);
      }
      bzero(flag_buf, 0x200);
      for (uint i = 0; i < 10; i++) {
        sprintf((char *)((int)flag_buf + i * 4), "%c", DAT_4347c000[i]); // Original used "!H"
      }
    }
  }
}

// Function: setObjectByCoord
void setObjectByCoord(int dungeon_ptr, uint x_coord, int y_coord, int object_ptr) {
  int current_room_ptr = dungeon_ptr;
  int room_index = x_coord / 0x18; // 0x18 is 24

  for (int i = 0; (current_room_ptr != 0 && (i < room_index)); i++) {
    current_room_ptr = *(int *)(current_room_ptr + 0x360);
  }

  if (current_room_ptr != 0) {
    *(int *)(current_room_ptr + (x_coord % 0x18 + y_coord * 0x18) * 4) = object_ptr;

    int *obj_coords = (int *)(*(int *)(object_ptr + 8));
    obj_coords[0] = x_coord;
    obj_coords[1] = y_coord;
  }
}

// Function: getRoom
int getRoom(int dungeon_ptr, int room_index) {
  if (room_index < 0) {
    return 0;
  }
  int current_room_ptr = *(int *)(dungeon_ptr + 0xc); // Head of room list
  for (int i = 0; (current_room_ptr != 0 && (i < room_index)); i++) {
    current_room_ptr = *(int *)(current_room_ptr + 0x360);
  }
  return current_room_ptr;
}

// Function: getObjectByCoord
int getObjectByCoord(int dungeon_ptr, uint x_coord, uint y_coord) {
  int room_ptr = getRoom(dungeon_ptr, x_coord / 0x18);
  if (room_ptr == 0) {
    return 0;
  } else {
    // Original: (y_coord % 9 * 0x18 + x_coord % 0x18) * 4
    // Assumes 9 rows per room. This seems to be the lookup formula.
    return *(int *)(room_ptr + ((y_coord % 9) * 0x18 + x_coord % 0x18) * 4);
  }
}

// Function: getObjectById
int getObjectById(int dungeon_ptr, int object_id) {
  int current_room_ptr = *(int *)(dungeon_ptr + 0xc); // Head of room list
  while (current_room_ptr != 0) {
    for (int r = 0; r < 9; r++) {
      for (int c = 0; c < 0x18; c++) {
        int obj_ptr = *(int *)(current_room_ptr + (r * 0x18 + c) * 4);
        if (obj_ptr != 0 && *(int *)(obj_ptr + 4) == object_id) { // Object ID is at offset 4
          return obj_ptr;
        }
      }
    }
    current_room_ptr = *(int *)(current_room_ptr + 0x360); // Next room
  }
  return 0;
}

// Function: playerDied
void playerDied(void) {
  char buffer[1024];
  bzero(buffer, sizeof(buffer));

  int player_obj_ptr = getObjectById(main_dungeon_ptr, 3); // Assuming player ID is 3
  if (player_obj_ptr == 0) {
    _terminate(1);
  }

  int *player_obj = (int *)player_obj_ptr;
  int *coords = (int *)(player_obj[2]);
  sprintf(buffer, " at position x:%u y:%u after %u moves\n", coords[0], coords[1], player_obj[4]);

  if (transmit_all(1, buffer, strlen(buffer), 0) != 0) {
    _terminate(2);
  }
}

// Function: getName
void * getName(void) {
  char name_buffer[51];
  bzero(name_buffer, sizeof(name_buffer));

  if (transmit_all(1, "Please enter your name: ", strlen("Please enter your name: "), 0) != 0) {
    _terminate(1);
  }
  if (read_until_delim_or_n(0, name_buffer, sizeof(name_buffer) -1, '\n') != 0) {
    _terminate(1);
  }

  size_t name_len = strlen(name_buffer);
  void *name_ptr = malloc(name_len + 1);
  if (name_ptr == NULL) {
    _terminate(1);
  }
  memcpy(name_ptr, name_buffer, name_len + 1);
  return name_ptr;
}

// Function: insertNewScore
// Score object: +0: char* name_ptr, +4: int score, +8: int next_score_ptr
int insertNewScore(int head_score_ptr, int new_score_ptr) {
  uint new_score_val = *(uint *)(new_score_ptr + 4);

  if (head_score_ptr == 0 || new_score_val < *(uint *)(head_score_ptr + 4)) {
    *(int *)(new_score_ptr + 8) = head_score_ptr;
    return new_score_ptr;
  } else {
    int current_score_ptr = head_score_ptr;
    int prev_score_ptr = head_score_ptr;
    while (current_score_ptr != 0) {
      if (new_score_val < *(uint *)(current_score_ptr + 4)) {
        *(int *)(prev_score_ptr + 8) = new_score_ptr;
        *(int *)(new_score_ptr + 8) = current_score_ptr;
        return head_score_ptr;
      }
      prev_score_ptr = current_score_ptr;
      current_score_ptr = *(int *)(current_score_ptr + 8);
    }
    *(int *)(prev_score_ptr + 8) = new_score_ptr;
    return head_score_ptr;
  }
}

// Function: addHighScore
void addHighScore(int game_data_ptr, int score) {
  int *score_obj = (int *)malloc(0xc); // name_ptr, score, next_ptr
  if (score_obj == NULL) {
    _terminate(1);
  }
  bzero(score_obj, 0xc);

  score_obj[0] = (int)getName();
  score_obj[1] = score;
  score_obj[2] = 0;

  *(int *)(game_data_ptr + 0x14) = insertNewScore(*(int *)(game_data_ptr + 0x14), (int)score_obj);
}

// Function: playerWon
void playerWon(int game_data_ptr) {
  char buffer[1024];
  bzero(buffer, sizeof(buffer));

  int player_obj_ptr = getObjectById(main_dungeon_ptr, 3);
  if (player_obj_ptr == 0) {
    _terminate(1);
  }

  int *player_obj = (int *)player_obj_ptr;
  int *coords = (int *)(player_obj[2]);
  sprintf(buffer, "You found the treasure at position x:%u y:%u after %u moves\n", coords[0], coords[1], player_obj[4]);
  if (transmit_all(1, buffer, strlen(buffer), 0) != 0) {
    _terminate(2);
  }

  size_t total_len = strlen("Move list: ") + strlen((char *)(*(int *)(game_data_ptr + 0x10))) + strlen(DAT_000163f5) + 2; // +2 for '\n' and null
  char *move_list_msg = (char *)malloc(total_len);
  if (move_list_msg == NULL) {
    _terminate(1);
  }
  sprintf(move_list_msg, "Move list: %s%s\n", (char *)(*(int *)(game_data_ptr + 0x10)), DAT_000163f5);

  if (transmit_all(1, move_list_msg, strlen(move_list_msg), 0) != 0) {
    _terminate(2);
  }
  free(move_list_msg);

  addHighScore(game_data_ptr, player_obj[4]);
}

// Function: moveDown
// Returns 0 for continue, 1 for won, 2 for died
int moveDown(int dungeon_ptr, int player_object_ptr) {
  int *player_obj = (int *)player_object_ptr;
  int *player_coords = (int *)(player_obj[2]);

  uint target_x = player_coords[0];
  int target_y = player_coords[1] + 1;

  int target_obj_ptr = getObjectByCoord(dungeon_ptr, target_x, target_y);
  if (target_obj_ptr == 0) { _terminate(1); }

  int target_obj_type = *(int *)(target_obj_ptr + 4);

  if (target_obj_type == 1 || target_obj_type == 2) { return 0; }
  if (target_obj_type == 4 && player_obj[1] == 3) { return 1; }
  if (target_obj_type == 5 && player_obj[1] == 3) { sendKillMessage("You were killed by a bat"); return 2; }
  if (target_obj_type == 6 && player_obj[1] == 3) { sendKillMessage("You were killed by a zombie"); return 2; }
  if (target_obj_type == 3) { return 2; } // Another player/unmovable
  if (target_obj_type == 7) { // Empty space
    setObjectByCoord(dungeon_ptr, target_x, target_y, player_object_ptr);
    setObjectByCoord(dungeon_ptr, player_coords[0], player_coords[1], target_obj_ptr);
  }
  return 0;
}

// Function: moveUp
// Returns 0 for continue, 1 for won, 2 for died
int moveUp(int dungeon_ptr, int player_object_ptr) {
  int *player_obj = (int *)player_object_ptr;
  int *player_coords = (int *)(player_obj[2]);

  uint target_x = player_coords[0];
  int target_y = player_coords[1] - 1;

  int target_obj_ptr = getObjectByCoord(dungeon_ptr, target_x, target_y);
  if (target_obj_ptr == 0) { _terminate(1); }

  int target_obj_type = *(int *)(target_obj_ptr + 4);

  if (target_obj_type == 1 || target_obj_type == 2) { return 0; }
  if (target_obj_type == 4 && player_obj[1] == 3) { return 1; }
  if (target_obj_type == 5 && player_obj[1] == 3) { sendKillMessage("You were killed by a bat"); return 2; }
  if (target_obj_type == 6 && player_obj[1] == 3) { sendKillMessage("You were killed by a zombie"); return 2; }
  if (target_obj_type == 3) { return 2; }
  if (target_obj_type == 7) { // Empty space
    setObjectByCoord(dungeon_ptr, target_x, target_y, player_object_ptr);
    setObjectByCoord(dungeon_ptr, player_coords[0], player_coords[1], target_obj_ptr);
  }
  return 0;
}

// Function: moveLeft
// Returns 0 for continue, 1 for won, 2 for died
int moveLeft(int dungeon_ptr, int player_object_ptr) {
  int *player_obj = (int *)player_object_ptr;
  int *player_coords = (int *)(player_obj[2]);
  int *player_state = (int *)(player_obj[3]);

  uint target_x = player_coords[0] - 1;
  int target_y = player_coords[1];

  int target_obj_ptr = getObjectByCoord(dungeon_ptr, target_x, target_y);
  if (target_obj_ptr == 0) { _terminate(1); }

  if (player_obj[1] == 3) { // Player type
    addMove(dungeon_ptr, player_object_ptr, *(char *)(dungeon_ptr + 1));
  }

  int target_obj_type = *(int *)(target_obj_ptr + 4);
  int result = 0;

  if (target_obj_type == 1 || target_obj_type == 2) {
    if (player_obj[1] == 6 || player_obj[1] == 5) { player_state[0] = 2; } // Enemy change direction
  } else if (target_obj_type == 4) { // Treasure
    if (player_obj[1] == 6 || player_obj[1] == 5) { player_state[0] = 2; }
    if (player_obj[1] == 3) { result = 1; } // Player wins
  } else if (target_obj_type == 5) { // Bat
    if (player_obj[1] == 3) { sendKillMessage("You were killed by a bat"); result = 2; }
    else if (player_obj[1] == 6 || player_obj[1] == 5) { player_state[0] = 2; }
  } else if (target_obj_type == 6) { // Zombie
    if (player_obj[1] == 3) { sendKillMessage("You were killed by a zombie"); result = 2; }
    else if (player_obj[1] == 6 || player_obj[1] == 5) { player_state[0] = 2; }
  } else if (target_obj_type == 3) { result = 2; }
  else if (target_obj_type == 7) { // Empty space
    setObjectByCoord(dungeon_ptr, target_x, target_y, player_object_ptr);
    setObjectByCoord(dungeon_ptr, player_coords[0], player_coords[1], target_obj_ptr);
  }
  return result;
}

// Function: moveRight
// Returns 0 for continue, 1 for won, 2 for died
int moveRight(int dungeon_ptr, int player_object_ptr) {
  int *player_obj = (int *)player_object_ptr;
  int *player_coords = (int *)(player_obj[2]);
  int *player_state = (int *)(player_obj[3]);

  uint target_x = player_coords[0] + 1;
  int target_y = player_coords[1];

  int target_obj_ptr = getObjectByCoord(dungeon_ptr, target_x, target_y);
  if (target_obj_ptr == 0) { _terminate(1); }

  if (player_obj[1] == 3) { // Player type
    addMove(dungeon_ptr, player_object_ptr, *(char *)(dungeon_ptr + 2));
  }

  int target_obj_type = *(int *)(target_obj_ptr + 4);
  int result = 0;

  if (target_obj_type == 1 || target_obj_type == 2) {
    if (player_obj[1] == 6 || player_obj[1] == 5) { player_state[0] = 1; } // Enemy change direction
  } else if (target_obj_type == 4) { // Treasure
    if (player_obj[1] == 6 || player_obj[1] == 5) { player_state[0] = 1; }
    if (player_obj[1] == 3) { result = 1; } // Player wins
  } else if (target_obj_type == 5) { // Bat
    if (player_obj[1] == 3) { sendKillMessage("You were killed by a bat"); result = 2; }
    else if (player_obj[1] == 6 || player_obj[1] == 5) { player_state[0] = 1; }
  } else if (target_obj_type == 6) { // Zombie
    if (player_obj[1] == 3) { sendKillMessage("You were killed by a zombie"); result = 2; }
    else if (player_obj[1] == 6 || player_obj[1] == 5) { player_state[0] = 1; }
  } else if (target_obj_type == 3) { result = 2; }
  else if (target_obj_type == 7) { // Empty space
    setObjectByCoord(dungeon_ptr, target_x, target_y, player_object_ptr);
    setObjectByCoord(dungeon_ptr, player_coords[0], player_coords[1], target_obj_ptr);
  }
  return result;
}

// Function: checkFloor
// Returns 0 for continue, 1 for won, 2 for died
int checkFloor(int dungeon_ptr, int player_object_ptr) {
  int *player_obj = (int *)player_object_ptr;
  int *player_coords = (int *)(player_obj[2]);
  int *player_state = (int *)(player_obj[3]);

  int result = 0;
  // Loop while player is in air (floor object type is not a solid surface like wall/boundary)
  while (2 < *(uint *)(getObjectByCoord(dungeon_ptr, player_coords[0], player_coords[1] + 1) + 4)) {
    result = moveDown(dungeon_ptr, player_object_ptr);
    if (result != 0) return result;

    if (player_state[0] == 1) { // Left bias for falling
      result = moveLeft(dungeon_ptr, player_object_ptr);
      if (result != 0) return result;
    } else if (player_state[0] == 2) { // Right bias for falling
      result = moveRight(dungeon_ptr, player_object_ptr);
      if (result != 0) return result;
    }

    if (player_obj[1] == 3) { // Only player adds move and moves enemies
      addMove(dungeon_ptr, player_object_ptr, *(char *)(dungeon_ptr + 1)); // Original used param_1+1 for a move char
      result = moveEnemies(dungeon_ptr, player_obj[4]);
      if (result != 0) return result;
      sendCurrentDungeonView(dungeon_ptr);
    }
  }
  return result;
}

// Function: moveZombie
// Returns 0 for continue, 1 for won, 2 for died
int moveZombie(int dungeon_ptr, int zombie_object_ptr, int current_moves) {
  int *zombie_obj = (int *)zombie_object_ptr;
  int *zombie_state = (int *)(zombie_obj[3]);

  if (current_moves == zombie_obj[4]) { return 0; } // Zombie already moved this turn

  int result = 0;
  if (zombie_state[0] == 1) { // Move left
    result = moveLeft(dungeon_ptr, zombie_object_ptr);
    if (result != 0) return result;
    result = checkFloor(dungeon_ptr, zombie_object_ptr);
  } else if (zombie_state[0] == 2) { // Move right
    result = moveRight(dungeon_ptr, zombie_object_ptr);
    if (result != 0) return result;
    result = checkFloor(dungeon_ptr, zombie_object_ptr);
  }

  if (result != 0) { return result; }
  zombie_obj[4]++; // Increment zombie's last_move_turn
  return 0;
}

// Function: moveBat
// Returns 0 for continue, 1 for won, 2 for died
int moveBat(int dungeon_ptr, int bat_object_ptr, int current_moves) {
  int *bat_obj = (int *)bat_object_ptr;
  int *bat_state = (int *)(bat_obj[3]);

  if (current_moves == bat_obj[4]) { return 0; } // Bat already moved this turn

  int result = 0;
  if (bat_state[0] == 1) { result = moveLeft(dungeon_ptr, bat_object_ptr); }
  else if (bat_state[0] == 2) { result = moveRight(dungeon_ptr, bat_object_ptr); }
  if (result != 0) return result;

  if (bat_state[1] == 3) { // Move down
    result = moveDown(dungeon_ptr, bat_object_ptr);
    if (result != 0) return result;
    bat_state[1] = 4; // Change vertical direction to up
  } else { // Move up
    result = moveUp(dungeon_ptr, bat_object_ptr);
    if (result != 0) return result;
    bat_state[1] = 3; // Change vertical direction to down
  }

  if (result != 0) return result;
  bat_obj[4]++; // Increment bat's last_move_turn
  return 0;
}

// Function: moveEnemies
// Returns 0 for continue, 1 for won, 2 for died
int moveEnemies(int dungeon_ptr, int current_moves) {
  int current_room_ptr = *(int *)(dungeon_ptr + 0xc);
  while (current_room_ptr != 0) {
    for (int r = 0; r < 9; r++) {
      for (int c = 0; c < 0x18; c++) {
        int obj_ptr = *(int *)(current_room_ptr + (r * 0x18 + c) * 4);
        if (obj_ptr == 0) continue;

        int obj_type = *(int *)(obj_ptr + 4);
        int result = 0;

        if (obj_type == 5) { // Bat
          result = moveBat(dungeon_ptr, obj_ptr, current_moves);
          if (result == 2) { sendKillMessage("You were killed by a bat"); }
          if (result != 0) { return result; }
        } else if (obj_type == 6) { // Zombie
          result = moveZombie(dungeon_ptr, obj_ptr, current_moves);
          if (result == 2) { sendKillMessage("You were killed by a zombie"); }
          if (result != 0) { return result; }
        }
      }
    }
    current_room_ptr = *(int *)(current_room_ptr + 0x360);
  }
  return 0;
}

// Function: jump
// Returns 0 for continue, 1 for won, 2 for died
int jump(int dungeon_ptr, int player_object_ptr) {
  int *player_obj = (int *)player_object_ptr;
  int *player_state = (int *)(player_obj[3]);

  player_state[0] = 0; // No horizontal bias
  player_state[1] = 3; // Initial vertical state for jump (move down)

  int result = 0;
  for (int i = 1; i <= 2; i++) {
    if (player_obj[1] == 3) { // Player type
      addMove(dungeon_ptr, player_object_ptr, *(char *)(dungeon_ptr + 3));
    }
    result = moveUp(dungeon_ptr, player_object_ptr);
    if (result != 0) return result;
    result = moveEnemies(dungeon_ptr, player_obj[4]);
    if (result != 0) return result;
    sendCurrentDungeonView(dungeon_ptr);
  }

  player_state[1] = 4; // Set vertical state to move up (for falling check)
  return checkFloor(dungeon_ptr, player_object_ptr);
}

// Function: jumpLeft
// Returns 0 for continue, 1 for won, 2 for died
int jumpLeft(int dungeon_ptr, int player_object_ptr) {
  int *player_obj = (int *)player_object_ptr;
  int *player_state = (int *)(player_obj[3]);

  player_state[1] = 3; // Initial vertical state (move down)
  player_state[0] = 1; // Left bias

  int result = 0;
  for (int i = 1; i <= 2; i++) {
    result = moveUp(dungeon_ptr, player_object_ptr);
    if (result != 0) return result;
    result = moveLeft(dungeon_ptr, player_object_ptr);
    if (result != 0) return result;
    result = moveEnemies(dungeon_ptr, player_obj[4]);
    if (result != 0) return result;
    sendCurrentDungeonView(dungeon_ptr);
  }

  player_state[1] = 4; // Set vertical state to move up
  result = checkFloor(dungeon_ptr, player_object_ptr);
  if (result == 0) { player_state[0] = 0; } // Reset horizontal bias
  return result;
}

// Function: jumpRight
// Returns 0 for continue, 1 for won, 2 for died
int jumpRight(int dungeon_ptr, int player_object_ptr) {
  int *player_obj = (int *)player_object_ptr;
  int *player_state = (int *)(player_obj[3]);

  player_state[1] = 3; // Initial vertical state (move down)
  player_state[0] = 2; // Right bias

  int result = 0;
  for (int i = 1; i <= 2; i++) {
    result = moveUp(dungeon_ptr, player_object_ptr);
    if (result != 0) return result;
    result = moveRight(dungeon_ptr, player_object_ptr);
    if (result != 0) return result;
    result = moveEnemies(dungeon_ptr, player_obj[4]);
    if (result != 0) return result;
    sendCurrentDungeonView(dungeon_ptr);
  }

  player_state[1] = 4; // Set vertical state to move up
  result = checkFloor(dungeon_ptr, player_object_ptr);
  if (result == 0) { player_state[0] = 0; } // Reset horizontal bias
  return result;
}

// Function: makeMove
// Returns 0 for continue, 1 for won, 2 for died, -1 for invalid move
int makeMove(int dungeon_ptr, char move_char) {
  int *dungeon_data = (int *)dungeon_ptr;

  if (dungeon_data[4] == 0) { // Initialize move history buffer
    void *move_buf = malloc(0x101); // 257 bytes
    if (move_buf == NULL) _terminate(1);
    bzero(move_buf, 0x101);
    dungeon_data[4] = (int)move_buf;
  }
  char *move_history_buf = (char *)dungeon_data[4];
  size_t current_len = strlen(move_history_buf);
  if (current_len < 0x100) {
      move_history_buf[current_len] = move_char;
      move_history_buf[current_len + 1] = '\0';
  }

  int player_obj_ptr = getObjectById(dungeon_ptr, 3);
  if (player_obj_ptr == 0) { _terminate(1); }
  int *player_obj = (int *)player_obj_ptr;

  int result = 0;
  if (move_char == *(char *)(dungeon_ptr + 1)) { // Move Left
    *(int *)(player_obj[3]) = 0; // Reset horizontal bias
    result = moveLeft(dungeon_ptr, player_obj_ptr);
    if (result != 0) return result;
    result = moveEnemies(dungeon_ptr, player_obj[4]);
    if (result != 0) return result;
    sendCurrentDungeonView(dungeon_ptr);
    result = checkFloor(dungeon_ptr, player_obj_ptr);
  } else if (move_char == *(char *)(dungeon_ptr + 2)) { // Move Right
    *(int *)(player_obj[3]) = 0; // Reset horizontal bias
    result = moveRight(dungeon_ptr, player_obj_ptr);
    if (result != 0) return result;
    result = moveEnemies(dungeon_ptr, player_obj[4]);
    if (result != 0) return result;
    sendCurrentDungeonView(dungeon_ptr);
    result = checkFloor(dungeon_ptr, player_obj_ptr);
  } else if (move_char == *(char *)(dungeon_ptr + 3)) { // Jump
    result = jump(dungeon_ptr, player_obj_ptr);
  } else if (move_char == *(char *)(dungeon_ptr + 4)) { // Jump Left
    result = jumpLeft(dungeon_ptr, player_obj_ptr);
  } else if (move_char == *(char *)(dungeon_ptr + 5)) { // Jump Right
    result = jumpRight(dungeon_ptr, player_obj_ptr);
  } else if (move_char == *(char *)(dungeon_ptr + 6)) { // Wait
    addMove(dungeon_ptr, player_obj_ptr, *(char *)(dungeon_ptr + 6));
    result = moveEnemies(dungeon_ptr, player_obj[4]);
    if (result != 0) return result;
    sendCurrentDungeonView(dungeon_ptr);
    result = 0;
  } else {
    return -1; // Invalid move
  }
  return result;
}

// Function: sendCurrentDungeonView
void sendCurrentDungeonView(int dungeon_ptr) {
  char view_buffer[25]; // 0x19 bytes for local_51
  
  int player_obj_ptr = getObjectById(dungeon_ptr, 3);
  if (player_obj_ptr == 0) { _terminate(1); }

  int *player_obj = (int *)player_obj_ptr;
  int *player_coords = (int *)(player_obj[2]);
  int player_x = player_coords[0];
  int player_y = player_coords[1];

  int view_radius_x = 12;
  int view_radius_y = 6;

  for (int dy = -view_radius_y; dy <= view_radius_y; dy++) {
    bzero(view_buffer, sizeof(view_buffer));
    for (int dx = -view_radius_x; dx <= view_radius_x; dx++) {
      char display_char = ' ';
      int current_y = player_y + dy;
      int current_x = player_x + dx;

      if (current_y >= 0 && current_y < 9) { // Assuming 9 rows per room
        int obj_ptr = getObjectByCoord(dungeon_ptr, current_x, current_y);
        if (obj_ptr != 0) {
          display_char = *(char *)obj_ptr; // Display char is at offset 0
        }
      }
      view_buffer[view_radius_x + dx] = display_char;
    }
    transmit_all(1, view_buffer, strlen(view_buffer), 0);
    transmit_all(1, "\n", 1, 0);
  }
}

// Function: addRoom
void addRoom(int room_ptr, char *map_data, uint start_col, int start_row, int param_5) {
  for (int row = start_row; row < 9; row++) {
    for (uint col = start_col; col < start_col + 0x18; col++) {
      char map_char = map_data[(row % 9) * 0x18 + col % 0x18];

      int obj_type = 7; // Default to empty space
      char display_char = ' ';
      int state1 = 0;
      int state2 = 0;

      switch (map_char) {
        case '-': obj_type = 1; display_char = '-'; break;
        case '|': obj_type = 2; display_char = '|'; break;
        case 'R': obj_type = 3; display_char = 'R'; break; // Player
        case '@': obj_type = 4; display_char = '@'; break; // Treasure
        case 'v': obj_type = 5; display_char = 'v'; state1 = 1; state2 = 3; break; // Bat (left, down)
        case '&': obj_type = 6; display_char = '&'; state1 = 1; state2 = 4; break; // Zombie (left, up)
        case ' ': obj_type = 7; display_char = ' '; break;
        default: _terminate(11); // Unknown map character
      }

      int new_obj_ptr = makeObject(display_char, obj_type, state1, col, row, state2, 0);
      *(int *)(room_ptr + (row * 0x18 + col % 0x18) * 4) = new_obj_ptr;
    }
  }
}

// Function: destroyRoom
void destroyRoom(void *room_ptr) {
  if (room_ptr == NULL) return;
  for (int r = 0; r < 9; r++) {
    for (int c = 0; c < 0x18; c++) {
      int obj_ptr = *(int *)((int)room_ptr + (r * 0x18 + c) * 4);
      if (obj_ptr != 0) {
        destroyObject((void *)obj_ptr);
        *(int *)((int)room_ptr + (r * 0x18 + c) * 4) = 0;
      }
    }
  }
}

// Function: buildDungeon
void buildDungeon(char *dungeon_ptr) {
  bzero(dungeon_ptr, 10); // Clear move characters area

  int first_room_ptr = (int)malloc(0x364);
  if (first_room_ptr == 0) {
    _terminate(1);
  }
  *(int *)(dungeon_ptr + 0xc) = first_room_ptr; // Store room list head
  *(int *)(first_room_ptr + 0x360) = 0; // No next room yet

  const char *initial_map = "------------------------|R                   v ||-                     || -                   v||  -                   ||   -                  ||    -                 ||------&     --       -|------------------------";
  addRoom(first_room_ptr, (char*)initial_map, 0, 0, 0);

  *(int *)(dungeon_ptr + 0x10) = 0; // Initialize move history buffer pointer
  *(int *)(dungeon_ptr + 0x14) = 0; // Initialize high score list head

  char assigned_chars[256] = {0}; // Boolean array for tracking used chars
  char possible_chars[] = "!@#$%^&*()abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  int num_possible_chars = strlen(possible_chars);

  for (int i = 0; i < 10; i++) { // Assign 10 unique random characters for moves
    char random_char;
    do {
      random_char = possible_chars[rand() % num_possible_chars];
    } while (assigned_chars[(unsigned char)random_char] != 0);
    
    assigned_chars[(unsigned char)random_char] = 1;
    dungeon_ptr[i] = random_char;
  }
}

// Function: destroyDungeon
void destroyDungeon(void *dungeon_ptr) {
  char *dungeon_data = (char *)dungeon_ptr;

  bzero(dungeon_data, 10); // Clear move characters

  if (*(int *)(dungeon_data + 0x10) != 0) { // Free move history buffer
    free((void *)(*(int *)(dungeon_data + 0x10)));
    *(int *)(dungeon_data + 0x10) = 0;
  }

  void *current_room_ptr = (void *)(*(int *)(dungeon_data + 0xc));
  while (current_room_ptr != NULL) { // Free all rooms
    void *next_room_ptr = (void *)(*(int *)((int)current_room_ptr + 0x360));
    destroyRoom(current_room_ptr);
    free(current_room_ptr);
    current_room_ptr = next_room_ptr;
  }
  *(int *)(dungeon_data + 0xc) = 0;

  int current_score_ptr = *(int *)(dungeon_data + 0x14);
  while (current_score_ptr != 0) { // Free high score list
      int *score_obj = (int *)current_score_ptr;
      int next_score_ptr = score_obj[2];
      if (score_obj[0] != 0) {
          free((void *)score_obj[0]);
      }
      free((void *)current_score_ptr);
      current_score_ptr = next_score_ptr;
  }
  *(int *)(dungeon_data + 0x14) = 0;
}

// Main function for testing
int main() {
    main_dungeon_ptr = (int)malloc(0x18);
    if (main_dungeon_ptr == 0) {
        fprintf(stderr, "Failed to allocate main dungeon struct\n");
        return 1;
    }
    bzero((void*)main_dungeon_ptr, 0x18);

    srand(time(NULL));

    buildDungeon((char*)main_dungeon_ptr);
    fprintf(stdout, "Dungeon built. Move characters: L:%c R:%c J:%c JL:%c JR:%c W:%c\n",
            *(char*)(main_dungeon_ptr + 1), *(char*)(main_dungeon_ptr + 2), *(char*)(main_dungeon_ptr + 3),
            *(char*)(main_dungeon_ptr + 4), *(char*)(main_dungeon_ptr + 5), *(char*)(main_dungeon_ptr + 6));

    sendCurrentDungeonView(main_dungeon_ptr);

    fprintf(stdout, "Making a move (Right)...\n");
    int result = makeMove(main_dungeon_ptr, *(char*)(main_dungeon_ptr + 2));
    if (result == 1) { playerWon(main_dungeon_ptr); }
    else if (result == 2) { playerDied(); }
    sendCurrentDungeonView(main_dungeon_ptr);

    fprintf(stdout, "Making another move (Jump)...\n");
    result = makeMove(main_dungeon_ptr, *(char*)(main_dungeon_ptr + 3));
    if (result == 1) { playerWon(main_dungeon_ptr); }
    else if (result == 2) { playerDied(); }
    sendCurrentDungeonView(main_dungeon_ptr);

    destroyDungeon((void*)main_dungeon_ptr);
    free((void*)main_dungeon_ptr);
    main_dungeon_ptr = 0;

    fprintf(stdout, "Program finished successfully.\n");
    return 0;
}