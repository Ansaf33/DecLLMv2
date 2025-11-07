#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

// --- Type Definitions ---

// Forward declarations
typedef struct Object Object;
typedef struct Room Room;
typedef struct Score Score;
typedef struct Dungeon Dungeon;

// Coordinate/State Object (8 bytes)
typedef struct Coord {
    unsigned int x;
    unsigned int y;
} Coord;

// Main Object (20 bytes)
struct Object {
    char char_repr; // Offset 0
    int type;       // Offset 4 (e.g., 1:wall, 2:block, 3:player, 4:treasure, 5:bat, 6:zombie, 7:empty)
    Coord* coords;  // Offset 8 (x, y coordinates in the dungeon grid)
    Coord* state;   // Offset 12 (e.g., for enemies: x=direction, y=vertical_state; for player: x=jump_dir, y=jump_state)
    int id;         // Offset 16 (unique identifier, also used for moves_taken for player/enemies)
};

// Room Object (0x364 bytes = 868 bytes)
struct Room {
    Object* grid[9][24]; // 9 rows, 24 columns, storing Object pointers
    Room* next;          // Pointer to the next room in the dungeon list
};

// Score Object (0xc bytes = 12 bytes)
struct Score {
    char* name;
    int score_value;
    Score* next;
};

// Dungeon Object (inferred structure from main object)
struct Dungeon {
    char move_keys[10];      // Offset 0-9
    Room* rooms_head;        // Offset 12 (0xc)
    char* moves_buffer;      // Offset 16 (0x10) - stores player's move history
    Score* high_scores_head; // Offset 20 (0x14)
};

// --- Global Variables ---
void* flag_buf = NULL;
const char DAT_000163f5[] = "\n";
// Placeholder for DAT_4347c000, assuming it's 10 bytes used for flag_buf initialization
const unsigned char DAT_4347c000[10] = {0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A};

// Assuming PLAYER_OBJECT_ID is 0 based on initial 'R' object's ID in addRoom
#define PLAYER_OBJECT_ID 0

// --- Function Declarations ---
void _terminate(int code);
int transmit_all(int fd, const char *buf, size_t count);
int read_until_delim_or_n(int fd, char *buf, size_t count, char delim, int unknown_param);
void sendCurrentDungeonView(Dungeon* dungeon);
Object* getObjectById(Room* rooms_head, int id);
Room* getRoom(Room* rooms_head, int index);
void addMove(Dungeon* dungeon, Object* player_obj, char move_char);
int moveEnemies(Dungeon* dungeon, int player_moves_taken);
void destroyObject(Object* obj);
void playerWon(Dungeon* dungeon);
void playerDied(Dungeon* dungeon);

// Function: sendKillMessage
void sendKillMessage(char *message) {
    transmit_all(1, message, strlen(message));
    _terminate(2);
}

// Function: makeObject
Object* makeObject(char char_repr, int type, unsigned int coord_y, unsigned int coord_x,
                   unsigned int state_y, unsigned int state_x, int id) {
    Object* new_obj = (Object*)malloc(sizeof(Object));
    if (new_obj == NULL) {
        _terminate(1);
    }
    memset(new_obj, 0, sizeof(Object));

    new_obj->coords = (Coord*)malloc(sizeof(Coord));
    if (new_obj->coords == NULL) {
        free(new_obj);
        _terminate(1);
    }
    memset(new_obj->coords, 0, sizeof(Coord));

    new_obj->state = (Coord*)malloc(sizeof(Coord));
    if (new_obj->state == NULL) {
        free(new_obj->coords);
        free(new_obj);
        _terminate(1);
    }
    memset(new_obj->state, 0, sizeof(Coord));

    new_obj->char_repr = char_repr;
    new_obj->type = type;
    new_obj->coords->x = coord_x;
    new_obj->coords->y = coord_y;
    new_obj->state->x = state_x;
    new_obj->state->y = state_y;
    new_obj->id = id;

    return new_obj;
}

// Function: destroyObject
void destroyObject(Object* obj) {
    if (obj == NULL) return;
    free(obj->coords);
    free(obj->state);
    free(obj);
}

// Function: addRoom
void addRoom(Room* room_ptr, const char* layout_string, unsigned int string_start_idx, int start_row_idx, int object_id_counter) {
    for (int y_in_room = 0; y_in_room < 9; ++y_in_room) {
        for (unsigned int x_in_room_abs = string_start_idx; x_in_room_abs < string_start_idx + 24; ++x_in_room_abs) {
            char char_from_layout = layout_string[y_in_room * 24 + (x_in_room_abs % 24)];
            int obj_type = 0;
            char obj_char_repr = char_from_layout;
            int state_x = 0;
            int state_y = 0;

            switch (char_from_layout) {
                case '-': obj_type = 1; break; // Wall
                case '|': obj_type = 2; break; // Block
                case 'R': obj_type = 3; break; // Player (initial position)
                case '@': obj_type = 4; break; // Treasure
                case 'v': obj_type = 5; state_x = 1; state_y = 3; break; // Bat (initial direction 1 (left), vertical state 3 (up))
                case '&': obj_type = 6; state_x = 1; state_y = 4; break; // Zombie (initial direction 1 (left), vertical state 4 (down))
                case ' ': obj_type = 7; break; // Empty space
                default:
                    _terminate(11);
            }
            room_ptr->grid[y_in_room][x_in_room_abs % 24] = makeObject(obj_char_repr, obj_type, start_row_idx + y_in_room, x_in_room_abs, state_y, state_x, object_id_counter);
        }
    }
}

// Function: extendDungeon
void extendDungeon(Dungeon* dungeon, int room_idx, char move_char_type, int player_moves_taken) {
    int layout_segment_index = 2; // Default
    if (move_char_type == dungeon->move_keys[1]) { // 'left'
        layout_segment_index = 0;
    } else if (move_char_type == dungeon->move_keys[2]) { // 'right'
        layout_segment_index = 1;
    } else if (move_char_type == dungeon->move_keys[3]) { // 'jump' (straight)
        layout_segment_index = 0;
    } else if (move_char_type == dungeon->move_keys[4]) { // 'jumpLeft'
        layout_segment_index = 1;
    } else if (move_char_type == dungeon->move_keys[5]) { // 'jumpRight'
        layout_segment_index = 0;
    } else if (move_char_type == dungeon->move_keys[6]) { // 'stay'
        layout_segment_index = (player_moves_taken < 0x12d) ? 1 : 2;
    }

    Room* new_room = (Room*)malloc(sizeof(Room));
    if (new_room == NULL) {
        _terminate(1);
    }
    memset(new_room, 0, sizeof(Room));

    const char* room_layout_segment =
        "------------------------|                    v ||       --             ||     -     -          ||            |        v||   -        |  -      |             | -       ||------&     --&      -|------------------------------------------------|&       &            v||--------------  |-    ||                |     ||---    ---------|     ||   ---          |     |       ---       |     ||                |&  --|------------------------------------------------|                     @||       -           ---||     v- -        v-   ||     -   -       -    ||    -     -     -     |    -       -   -      ||  &|       |&  |      |------------------------!H"
        + layout_segment_index * 0xd8; // 0xd8 = 216 bytes, which is 9 rows * 24 cols.

    addRoom(new_room, room_layout_segment, 0, room_idx * 9, 0);

    Room* current_room = dungeon->rooms_head;
    if (current_room == NULL) {
        dungeon->rooms_head = new_room;
    } else {
        while (current_room->next != NULL) {
            current_room = current_room->next;
        }
        current_room->next = new_room;
    }
}

// Function: addMove
void addMove(Dungeon* dungeon, Object* player_obj, char move_char) {
    player_obj->id++; // Increment player's move count (id field is used for moves_taken)

    if ((player_obj->id & 0x3f) == 0) { // Every 64 moves (0x3f is 63)
        unsigned int new_buffer_chunk_count = (player_obj->id >> 6) + 1;
        char* old_moves_buffer = dungeon->moves_buffer;
        dungeon->moves_buffer = (char*)malloc(new_buffer_chunk_count * 0x100); // Allocate in chunks of 0x100 bytes
        if (dungeon->moves_buffer == NULL) {
            _terminate(1);
        }
        memset(dungeon->moves_buffer, 0, new_buffer_chunk_count * 0x100);
        if (old_moves_buffer != NULL) {
            memcpy(dungeon->moves_buffer, old_moves_buffer, strlen(old_moves_buffer));
            free(old_moves_buffer);
        }
    }

    size_t current_len = strlen(dungeon->moves_buffer);
    dungeon->moves_buffer[current_len] = move_char;
    dungeon->moves_buffer[current_len + 1] = '\0';

    extendDungeon(dungeon, (player_obj->id >> 6), move_char, player_obj->id);

    if (flag_buf == NULL) {
        flag_buf = malloc(0x200); // Allocate 512 bytes
        if (flag_buf == NULL) {
            _terminate(1);
        }
        memset(flag_buf, 0, 0x200);
        for (int i = 0; i < 10; ++i) {
            sprintf((char*)flag_buf + i * 4, "%02X", DAT_4347c000[i]); // Store hex representation of byte
        }
    }
}

// Function: setObjectByCoord
void setObjectByCoord(Room* rooms_head, unsigned int x_coord_abs, unsigned int y_coord_abs, Object* obj) {
    Room* current_room = getRoom(rooms_head, x_coord_abs / 24);
    if (current_room != NULL) {
        current_room->grid[y_coord_abs % 9][x_coord_abs % 24] = obj;
        if (obj != NULL) {
            obj->coords->x = x_coord_abs;
            obj->coords->y = y_coord_abs;
        }
    }
}

// Function: getRoom
Room* getRoom(Room* rooms_head, int index) {
    if (index < 0) {
        return NULL;
    }
    Room* current_room = rooms_head;
    for (int i = 0; current_room != NULL && i < index; ++i) {
        current_room = current_room->next;
    }
    return current_room;
}

// Function: getObjectByCoord
Object* getObjectByCoord(Room* rooms_head, unsigned int x_coord_abs, unsigned int y_coord_abs) {
    Room* room = getRoom(rooms_head, x_coord_abs / 24);
    if (room == NULL) {
        return NULL;
    }
    return room->grid[y_coord_abs % 9][x_coord_abs % 24];
}

// Function: getObjectById
Object* getObjectById(Room* rooms_head, int id) {
    Room* current_room = rooms_head;
    while (current_room != NULL) {
        for (int y = 0; y < 9; ++y) {
            for (int x = 0; x < 24; ++x) {
                Object* obj = current_room->grid[y][x];
                if (obj != NULL && obj->id == id) {
                    return obj;
                }
            }
        }
        current_room = current_room->next;
    }
    return NULL;
}

// Function: playerDied
void playerDied(Dungeon* dungeon) {
    Object* player_obj = getObjectById(dungeon->rooms_head, PLAYER_OBJECT_ID);
    if (player_obj == NULL) {
        _terminate(1);
    }
    char message_buffer[1024];
    sprintf(message_buffer, " at position x:%u y:%u after %u moves\n",
            player_obj->coords->x, player_obj->coords->y, player_obj->id);
    transmit_all(1, message_buffer, strlen(message_buffer));
    _terminate(2);
}

// Function: getName
char* getName(void) {
    char input_buffer[52]; // 51 chars + null terminator
    memset(input_buffer, 0, sizeof(input_buffer));

    transmit_all(1, "Please enter your name: ", strlen("Please enter your name: "));
    if (read_until_delim_or_n(0, input_buffer, 51, '\n', 1) != 0) {
        _terminate(1);
    }

    size_t name_len = strlen(input_buffer);
    char* name_ptr = (char*)malloc(name_len + 1);
    if (name_ptr == NULL) {
        _terminate(1);
    }
    memcpy(name_ptr, input_buffer, name_len + 1); // Copy null terminator too
    return name_ptr;
}

// Function: insertNewScore
Score* insertNewScore(Score* head, Score* new_score) {
    if (head == NULL || new_score->score_value < head->score_value) {
        new_score->next = head;
        return new_score;
    }

    Score* current = head;
    Score* prev = NULL;
    while (current != NULL && new_score->score_value >= current->score_value) {
        prev = current;
        current = current->next;
    }
    prev->next = new_score;
    new_score->next = current;
    return head;
}

// Function: addHighScore
void addHighScore(Dungeon* dungeon, int score_value) {
    Score* new_score_entry = (Score*)malloc(sizeof(Score));
    if (new_score_entry == NULL) {
        _terminate(1);
    }
    memset(new_score_entry, 0, sizeof(Score));

    new_score_entry->name = getName();
    new_score_entry->score_value = score_value;
    new_score_entry->next = NULL;

    dungeon->high_scores_head = insertNewScore(dungeon->high_scores_head, new_score_entry);
}

// Function: playerWon
void playerWon(Dungeon* dungeon) {
    Object* player_obj = getObjectById(dungeon->rooms_head, PLAYER_OBJECT_ID);
    if (player_obj == NULL) {
        _terminate(1);
    }
    char message_buffer[1024];
    sprintf(message_buffer, "You found the treasure at position x:%u y:%u after %u moves\n",
            player_obj->coords->x, player_obj->coords->y, player_obj->id);
    transmit_all(1, message_buffer, strlen(message_buffer));

    size_t moves_len = strlen("Move list: ") + strlen(dungeon->moves_buffer) + strlen(DAT_000163f5) + 1;
    char* total_moves_str = (char*)malloc(moves_len);
    if (total_moves_str == NULL) {
        _terminate(1);
    }
    sprintf(total_moves_str, "Move list: %s%s", dungeon->moves_buffer, DAT_000163f5);
    transmit_all(1, total_moves_str, strlen(total_moves_str));
    free(total_moves_str);

    addHighScore(dungeon, player_obj->id);
}

// Function: moveDown
int moveDown(Dungeon* dungeon, Object* obj) {
    Object* target_obj = getObjectByCoord(dungeon->rooms_head, obj->coords->x, obj->coords->y + 1);
    if (target_obj == NULL) {
        _terminate(1); // Target square does not exist
    }

    if (target_obj->type == 1 || target_obj->type == 2) { // Wall or Block
        return 0; // Cannot move
    } else if (target_obj->type == 4 && obj->type == 3) { // Treasure and Player
        return 1; // Player won
    } else if ((target_obj->type == 5 || target_obj->type == 6) && obj->type == 3) { // Enemy and Player
        sendKillMessage((target_obj->type == 5) ? "You were killed by a bat" : "You were killed by a zombie");
        return 2; // Player died
    } else if (target_obj->type == 3) { // Player (collision with another player, should not happen)
        return 2; // Player died (or error)
    } else if (target_obj->type == 7) { // Empty space
        // Swap positions
        setObjectByCoord(dungeon->rooms_head, obj->coords->x, obj->coords->y + 1, obj);
        setObjectByCoord(dungeon->rooms_head, target_obj->coords->x, target_obj->coords->y, target_obj);
    }
    return 0; // Move successful or no action
}

// Function: moveUp
int moveUp(Dungeon* dungeon, Object* obj) {
    Object* target_obj = getObjectByCoord(dungeon->rooms_head, obj->coords->x, obj->coords->y - 1);
    if (target_obj == NULL) {
        _terminate(1); // Target square does not exist
    }

    if (target_obj->type == 1 || target_obj->type == 2) { // Wall or Block
        return 0; // Cannot move
    } else if (target_obj->type == 4 && obj->type == 3) { // Treasure and Player
        return 1; // Player won
    } else if ((target_obj->type == 5 || target_obj->type == 6) && obj->type == 3) { // Enemy and Player
        sendKillMessage((target_obj->type == 5) ? "You were killed by a bat" : "You were killed by a zombie");
        return 2; // Player died
    } else if (target_obj->type == 3) { // Player
        return 2; // Player died (or error)
    } else if (target_obj->type == 7) { // Empty space
        // Swap positions
        setObjectByCoord(dungeon->rooms_head, obj->coords->x, obj->coords->y - 1, obj);
        setObjectByCoord(dungeon->rooms_head, target_obj->coords->x, target_obj->coords->y, target_obj);
    }
    return 0; // Move successful or no action
}

// Function: moveLeft
int moveLeft(Dungeon* dungeon, Object* obj) {
    if (obj->type == 3) { // If it's the player, add move
        addMove(dungeon, obj, dungeon->move_keys[1]);
    }

    Object* target_obj = getObjectByCoord(dungeon->rooms_head, obj->coords->x - 1, obj->coords->y);
    if (target_obj == NULL) {
        _terminate(1); // Target square does not exist
    }

    if (target_obj->type == 1 || target_obj->type == 2) { // Wall or Block
        if (obj->type == 5 || obj->type == 6) { // Bat or Zombie hits wall/block
            obj->state->x = 2; // Change direction to right
        }
    } else if (target_obj->type == 4 && obj->type == 3) { // Treasure and Player
        return 1; // Player won
    } else if ((target_obj->type == 5 || target_obj->type == 6) && obj->type == 3) { // Enemy and Player
        sendKillMessage((target_obj->type == 5) ? "You were killed by a bat" : "You were killed by a zombie");
        return 2; // Player died
    } else if (target_obj->type == 3) { // Player
        return 2; // Player died (or error)
    } else if (target_obj->type == 7) { // Empty space
        // Swap positions
        setObjectByCoord(dungeon->rooms_head, obj->coords->x - 1, obj->coords->y, obj);
        setObjectByCoord(dungeon->rooms_head, target_obj->coords->x, target_obj->coords->y, target_obj);
    }
    return 0; // Move successful or no action
}

// Function: moveRight
int moveRight(Dungeon* dungeon, Object* obj) {
    if (obj->type == 3) { // If it's the player, add move
        addMove(dungeon, obj, dungeon->move_keys[2]);
    }

    Object* target_obj = getObjectByCoord(dungeon->rooms_head, obj->coords->x + 1, obj->coords->y);
    if (target_obj == NULL) {
        _terminate(1); // Target square does not exist
    }

    if (target_obj->type == 1 || target_obj->type == 2) { // Wall or Block
        if (obj->type == 5 || obj->type == 6) { // Bat or Zombie hits wall/block
            obj->state->x = 1; // Change direction to left
        }
    } else if (target_obj->type == 4 && obj->type == 3) { // Treasure and Player
        return 1; // Player won
    } else if ((target_obj->type == 5 || target_obj->type == 6) && obj->type == 3) { // Enemy and Player
        sendKillMessage((target_obj->type == 5) ? "You were killed by a bat" : "You were killed by a zombie");
        return 2; // Player died
    } else if (target_obj->type == 3) { // Player
        return 2; // Player died (or error)
    } else if (target_obj->type == 7) { // Empty space
        // Swap positions
        setObjectByCoord(dungeon->rooms_head, obj->coords->x + 1, obj->coords->y, obj);
        setObjectByCoord(dungeon->rooms_head, target_obj->coords->x, target_obj->coords->y, target_obj);
    }
    return 0; // Move successful or no action
}

// Function: checkFloor
int checkFloor(Dungeon* dungeon, Object* obj) {
    int result = 0;
    obj->state->y = 4; // Set vertical state to falling down

    Object* target_obj = getObjectByCoord(dungeon->rooms_head, obj->coords->x, obj->coords->y + 1);
    if (target_obj == NULL) {
        _terminate(1); // Should not happen if dungeon is properly built
    }

    while (target_obj->type > 2) { // While the object below is not a wall (1) or block (2)
        if (obj->state->x == 0) { // Fall straight
            result = moveDown(dungeon, obj);
        } else if (obj->state->x == 1) { // Fall left
            result = moveLeft(dungeon, obj);
            if (result == 0) { // If moveLeft was successful, then also moveDown
                result = moveDown(dungeon, obj);
            }
        } else if (obj->state->x == 2) { // Fall right
            result = moveRight(dungeon, obj);
            if (result == 0) { // If moveRight was successful, then also moveDown
                result = moveDown(dungeon, obj);
            }
        }
        if (result != 0) {
            return result;
        }

        if (obj->type == 3) { // Only for player
            // Add a 'fall' move (using 'left' key as placeholder)
            addMove(dungeon, obj, dungeon->move_keys[1]); 
            result = moveEnemies(dungeon, obj->id);
            if (result != 0) {
                return result;
            }
            sendCurrentDungeonView(dungeon);
        }

        // Re-evaluate the object below after movement
        target_obj = getObjectByCoord(dungeon->rooms_head, obj->coords->x, obj->coords->y + 1);
        if (target_obj == NULL) {
            _terminate(1);
        }
    }
    return 0;
}

// Function: moveZombie
int moveZombie(Dungeon* dungeon, Object* zombie_obj, int player_moves_taken) {
    if (player_moves_taken == zombie_obj->id) { // Only move if it's the current turn
        return 0;
    }
    int result = 0;
    if (zombie_obj->state->x == 1) { // Moving Left
        result = moveLeft(dungeon, zombie_obj);
        if (result != 0) return result;
        result = checkFloor(dungeon, zombie_obj);
    } else if (zombie_obj->state->x == 2) { // Moving Right
        result = moveRight(dungeon, zombie_obj);
        if (result != 0) return result;
        result = checkFloor(dungeon, zombie_obj);
    }
    if (result != 0) return result;
    zombie_obj->id++; // Increment zombie's move count
    return 0;
}

// Function: moveBat
int moveBat(Dungeon* dungeon, Object* bat_obj, int player_moves_taken) {
    if (player_moves_taken == bat_obj->id) { // Only move if it's the current turn
        return 0;
    }
    int result = 0;
    if (bat_obj->state->x == 1) { // Moving Left
        result = moveLeft(dungeon, bat_obj);
    } else if (bat_obj->state->x == 2) { // Moving Right
        result = moveRight(dungeon, bat_obj);
    }
    if (result != 0) return result;

    if (bat_obj->state->y == 3) { // Moving Up
        result = moveUp(dungeon, bat_obj);
        if (result != 0) return result;
        bat_obj->state->y = 4; // Change to moving down
    } else { // Moving Down
        result = moveDown(dungeon, bat_obj);
        if (result != 0) return result;
        bat_obj->state->y = 3; // Change to moving up
    }
    bat_obj->id++; // Increment bat's move count
    return 0;
}

// Function: moveEnemies
int moveEnemies(Dungeon* dungeon, int player_moves_taken) {
    Room* current_room = dungeon->rooms_head;
    while (current_room != NULL) {
        for (int y = 0; y < 9; ++y) {
            for (int x = 0; x < 24; ++x) {
                Object* obj = current_room->grid[y][x];
                if (obj != NULL) {
                    if (obj->type == 5) { // Bat
                        int result = moveBat(dungeon, obj, player_moves_taken);
                        if (result != 0) {
                            return result;
                        }
                    } else if (obj->type == 6) { // Zombie
                        int result = moveZombie(dungeon, obj, player_moves_taken);
                        if (result != 0) {
                            return result;
                        }
                    }
                }
            }
        }
        current_room = current_room->next;
    }
    return 0;
}

// Function: jump
int jump(Dungeon* dungeon, Object* player_obj) {
    player_obj->state->x = 0; // Jump straight
    player_obj->state->y = 3; // Jump up

    for (int i = 1; i <= 2; ++i) { // Jump two units up
        addMove(dungeon, player_obj, dungeon->move_keys[3]); // Add a 'jump' move
        int result = moveUp(dungeon, player_obj);
        if (result != 0) return result;
        result = moveEnemies(dungeon, player_obj->id);
        if (result != 0) return result;
        sendCurrentDungeonView(dungeon);
    }
    player_obj->state->y = 4; // Set to falling state
    return checkFloor(dungeon, player_obj);
}

// Function: jumpLeft
int jumpLeft(Dungeon* dungeon, Object* player_obj) {
    player_obj->state->x = 1; // Jump left
    player_obj->state->y = 3; // Jump up

    for (int i = 1; i <= 2; ++i) { // Jump two units
        addMove(dungeon, player_obj, dungeon->move_keys[4]); // Add a 'jumpLeft' move
        int result = moveUp(dungeon, player_obj);
        if (result != 0) return result;
        result = moveLeft(dungeon, player_obj);
        if (result != 0) return result;
        result = moveEnemies(dungeon, player_obj->id);
        if (result != 0) return result;
        sendCurrentDungeonView(dungeon);
    }
    player_obj->state->y = 4; // Set to falling state
    return checkFloor(dungeon, player_obj);
}

// Function: jumpRight
int jumpRight(Dungeon* dungeon, Object* player_obj) {
    player_obj->state->x = 2; // Jump right
    player_obj->state->y = 3; // Jump up

    for (int i = 1; i <= 2; ++i) { // Jump two units
        addMove(dungeon, player_obj, dungeon->move_keys[5]); // Add a 'jumpRight' move
        int result = moveUp(dungeon, player_obj);
        if (result != 0) return result;
        result = moveRight(dungeon, player_obj);
        if (result != 0) return result;
        result = moveEnemies(dungeon, player_obj->id);
        if (result != 0) return result;
        sendCurrentDungeonView(dungeon);
    }
    player_obj->state->y = 4; // Set to falling state
    return checkFloor(dungeon, player_obj);
}

// Function: makeMove
int makeMove(Dungeon* dungeon, char move_char) {
    if (dungeon->moves_buffer == NULL) {
        dungeon->moves_buffer = (char*)malloc(0x101); // Initial buffer size
        if (dungeon->moves_buffer == NULL) {
            _terminate(1);
        }
        memset(dungeon->moves_buffer, 0, 0x101);
    }

    Object* player_obj = getObjectById(dungeon->rooms_head, PLAYER_OBJECT_ID);
    if (player_obj == NULL) {
        _terminate(1); // Player object not found
    }

    int result = 0;
    if (move_char == dungeon->move_keys[1]) { // Move Left
        player_obj->state->x = 0; // Reset jump direction
        result = moveLeft(dungeon, player_obj);
        if (result != 0) return result;
        result = moveEnemies(dungeon, player_obj->id);
        if (result != 0) return result;
        sendCurrentDungeonView(dungeon);
        result = checkFloor(dungeon, player_obj);
    } else if (move_char == dungeon->move_keys[2]) { // Move Right
        player_obj->state->x = 0; // Reset jump direction
        result = moveRight(dungeon, player_obj);
        if (result != 0) return result;
        result = moveEnemies(dungeon, player_obj->id);
        if (result != 0) return result;
        sendCurrentDungeonView(dungeon);
        result = checkFloor(dungeon, player_obj);
    } else if (move_char == dungeon->move_keys[3]) { // Jump
        result = jump(dungeon, player_obj);
    } else if (move_char == dungeon->move_keys[4]) { // Jump Left
        result = jumpLeft(dungeon, player_obj);
    } else if (move_char == dungeon->move_keys[5]) { // Jump Right
        result = jumpRight(dungeon, player_obj);
    } else if (move_char == dungeon->move_keys[6]) { // Stay
        addMove(dungeon, player_obj, dungeon->move_keys[6]);
        result = moveEnemies(dungeon, player_obj->id);
        if (result != 0) return result;
        sendCurrentDungeonView(dungeon);
    } else {
        return -1; // Invalid move
    }
    return result;
}

// Function: sendCurrentDungeonView
void sendCurrentDungeonView(Dungeon* dungeon) {
    Object* player_obj = getObjectById(dungeon->rooms_head, PLAYER_OBJECT_ID);
    if (player_obj == NULL) {
        _terminate(1);
    }

    unsigned int player_abs_x = player_obj->coords->x;
    unsigned int player_abs_y = player_obj->coords->y;

    char view_buffer[25 + 1]; // 25 characters + null terminator
    
    for (int y_offset = -6; y_offset <= 6; ++y_offset) { // Iterate 13 rows (y-view)
        memset(view_buffer, ' ', sizeof(view_buffer)); // Fill with spaces for each row
        
        for (int x_offset = -12; x_offset <= 12; ++x_offset) { // Iterate 25 columns (x-view)
            int current_y_in_room = player_abs_y + y_offset;
            int current_abs_x = player_abs_x + x_offset;

            if (current_y_in_room < 0 || current_y_in_room >= 9) { // Out of room Y bounds
                continue;
            }

            Room* target_room = getRoom(dungeon->rooms_head, current_abs_x / 24);
            if (target_room != NULL) {
                Object* obj_at_coord = target_room->grid[current_y_in_room][current_abs_x % 24];
                if (obj_at_coord != NULL) {
                    view_buffer[12 + x_offset] = obj_at_coord->char_repr;
                }
            }
        }
        transmit_all(1, view_buffer, 25); // Print 25 characters
        transmit_all(1, "\n", 1); // Newline after each row
    }
}

// Function: destroyRoom
void destroyRoom(Room* room_ptr) {
    if (room_ptr == NULL) return;
    for (int y = 0; y < 9; ++y) {
        for (int x = 0; x < 24; ++x) {
            destroyObject(room_ptr->grid[y][x]);
            room_ptr->grid[y][x] = NULL;
        }
    }
}

// Function: buildDungeon
void buildDungeon(Dungeon* dungeon) {
    memset(dungeon, 0, sizeof(Dungeon));

    Room* initial_room = (Room*)malloc(sizeof(Room));
    if (initial_room == NULL) {
        _terminate(1);
    }
    memset(initial_room, 0, sizeof(Room));
    dungeon->rooms_head = initial_room;

    const char* initial_layout =
       "------------------------|R                   v ||-                     || -                   v||  -                   ||   -                  ||    -                 ||------&     --       -|------------------------";
    addRoom(initial_room, initial_layout, 0, 0, 0); // Start at (0,0) with object ID counter 0
    initial_room->next = NULL; // Ensure it's the only room initially

    dungeon->moves_buffer = NULL; // Initialize moves buffer to NULL

    // Generate 10 unique random characters for move_keys
    char used_chars[256] = {0}; // Boolean array to track used characters
    int char_count = 0;
    while (char_count < 10) {
        char random_char = (char)(rand() % ('}' - 't' + 1) + 't'); // Characters from 't' (0x74) to '}' (0x7d)
        if (used_chars[(unsigned char)random_char] == 0) {
            dungeon->move_keys[char_count] = random_char;
            used_chars[(unsigned char)random_char] = 1;
            char_count++;
        }
    }
}

// Function: destroyDungeon
void destroyDungeon(Dungeon* dungeon) {
    if (dungeon == NULL) return;

    // Free moves_buffer
    if (dungeon->moves_buffer != NULL) {
        free(dungeon->moves_buffer);
        dungeon->moves_buffer = NULL;
    }

    // Destroy all rooms
    Room* current_room = dungeon->rooms_head;
    while (current_room != NULL) {
        Room* next_room = current_room->next;
        destroyRoom(current_room);
        free(current_room);
        current_room = next_room;
    }
    dungeon->rooms_head = NULL;

    // Destroy high scores list
    Score* current_score = dungeon->high_scores_head;
    while (current_score != NULL) {
        Score* next_score = current_score->next;
        free(current_score->name);
        free(current_score);
        current_score = next_score;
    }
    dungeon->high_scores_head = NULL;
}

// --- Helper Functions (replacing non-standard/decompiled ones) ---

// Replace _terminate with exit
void _terminate(int code) {
    exit(code);
}

// Replace transmit_all with write to stdout
int transmit_all(int fd, const char *buf, size_t count) {
    if (write(fd, buf, count) != count) {
        return -1; // Indicate error
    }
    return 0; // Success
}

// Replace read_until_delim_or_n with read from stdin
int read_until_delim_or_n(int fd, char *buf, size_t count, char delim, int unknown_param) {
    ssize_t bytes_read = read(fd, buf, count);
    if (bytes_read < 0) {
        return -1; // Error
    }
    // Find delimiter or end of buffer
    for (ssize_t i = 0; i < bytes_read; ++i) {
        if (buf[i] == delim) {
            buf[i] = '\0'; // Null-terminate at delimiter
            return 0; // Success
        }
    }
    if (bytes_read > 0 && buf[bytes_read - 1] == '\n') { // handle newline as delimiter for convenience
        buf[bytes_read - 1] = '\0';
    } else {
        buf[bytes_read] = '\0'; // Null-terminate if no delimiter found
    }
    return 0; // Success
}

// --- Main Function ---
int main() {
    srand(time(NULL)); // Seed random number generator

    Dungeon game_dungeon;
    buildDungeon(&game_dungeon);

    // Find the player object (initial 'R' becomes type 3)
    Object* player = getObjectById(game_dungeon.rooms_head, PLAYER_OBJECT_ID);
    if (player != NULL) {
        player->id = 0; // Player's move count (re-purpose ID field for moves)
        player->char_repr = '@'; // Player character
        player->type = 3; // Player type
    } else {
        fprintf(stderr, "Player object not found after building dungeon!\n");
        _terminate(1);
    }
    
    sendCurrentDungeonView(&game_dungeon);

    char move_input[2];
    int game_state = 0; // 0: playing, 1: won, 2: died, -1: invalid move
    while (game_state == 0) {
        transmit_all(1, "Enter move (l/r/j/k/i/s): ", strlen("Enter move (l/r/j/k/i/s): "));
        ssize_t bytes_read = read(0, move_input, 1);
        if (bytes_read <= 0) break; // EOF or error
        move_input[bytes_read] = '\0'; // Null-terminate

        game_state = makeMove(&game_dungeon, move_input[0]);

        if (game_state == 1) {
            playerWon(&game_dungeon);
        } else if (game_state == 2) {
            playerDied(&game_dungeon);
        } else if (game_state == -1) {
            transmit_all(1, "Invalid move.\n", strlen("Invalid move.\n"));
        }
    }

    destroyDungeon(&game_dungeon);

    return 0;
}