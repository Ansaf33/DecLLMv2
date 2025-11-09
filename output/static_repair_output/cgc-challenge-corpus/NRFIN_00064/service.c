#include <stdio.h>   // For sprintf
#include <stdlib.h>  // For malloc
#include <string.h>  // For bzero, strlen
#include <unistd.h>  // For read (if read_n_bytes wraps it)
#include <stdint.h>  // For uint32_t

// --- Custom types from disassembler output ---
// Assuming 'byte' is unsigned char
typedef unsigned char byte;
// Assuming 'undefined' is a generic void type or char for single bytes
// We'll use specific types (char, void*, uint32_t) where appropriate
// Assuming 'undefined4' is a 4-byte unsigned integer
typedef uint32_t undefined4;

// --- Global variables ---
// Placeholder for DAT_4347c000, assuming it's an array of chars/bytes
// and large enough for index + 10 up to 100
static unsigned char DAT_4347c000[100] = {
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, // Example values
    0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14,
    0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E,
    0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
    0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32,
    0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C,
    0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
    0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50,
    0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A,
    0x5B, 0x5C, 0x5D, 0x5E, 0x5F, 0x60, 0x61, 0x62, 0x63, 0x64
};
static int flag_index = 0; // Global index used by getRandomName

// --- Forward declarations for external/custom functions ---
// (Signatures adjusted based on usage in the provided snippet)
extern int read_n_bytes(int fd, void *buf, size_t count, int *bytes_read);
extern void _terminate(int status);
extern int transmit_all(int fd, const void *buf, size_t count);

// Define a struct to represent the dungeon information,
// which seems to be passed around as a block of uint32_t values.
// The member names reflect their apparent usage in main and other functions.
typedef struct {
    uint32_t menu_quit_key_game_description_moves_packed1; // local_30
    uint32_t menu_play_key_game_description_moves_packed2; // local_2c
    uint32_t menu_inst_hs_keys;                            // local_28
    uint32_t current_dungeon_view_id;                      // local_24 (param_1[3] in playGame)
    uint32_t player_data_1;                                // local_20 (param_1[4] in playerDied)
    uint32_t high_score_list_head;                         // local_1c (param_1[5] in playerDied, head of HS list)
} DungeonInfo;

extern void sendCurrentDungeonView(uint32_t dungeon_view_id);
extern int makeMove(DungeonInfo *info, char move_char);
extern void playerDied(uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4, uint32_t p5, uint32_t p6);
extern void playerWon(DungeonInfo *info);
extern void buildDungeon(DungeonInfo *info);
extern void destroyDungeon(DungeonInfo *info);
extern uint32_t insertNewScore(uint32_t *new_score_entry, uint32_t old_head);


// Function: getRandomName
void *getRandomName(void) {
    char *s = (char *)malloc(11); // Allocate 11 bytes for a 10-char string + null terminator
    if (s == NULL) {
        return NULL;
    }

    bzero(s, 11);
    for (int i = 0; i < 10; i++) {
        unsigned char data_val = DAT_4347c000[i + flag_index];
        if ((data_val & 1) == 0) { // Check least significant bit for case
            s[i] = (data_val % 26) + 'A'; // Uppercase 'A' (0x41)
        } else {
            s[i] = (data_val % 26) + 'a'; // Lowercase 'a' (0x61)
        }
    }
    flag_index = (flag_index + 10) % 100;
    return s;
}

// Function: getMove
char getMove(void) { // Changed return type to char
    char input_buffer[8];
    int bytes_read_count = 0;

    bzero(input_buffer, 4); // Clear first 4 bytes
    int read_status = read_n_bytes(0, input_buffer, 2, &bytes_read_count);
    if (read_status != 0) {
        _terminate(1);
    }
    if (bytes_read_count == 0) {
        input_buffer[0] = 0; // If no bytes read, set to null
    }
    return input_buffer[0];
}

// Function: sendGameDescription
// Parameters adjusted to reflect byte-packed characters in main's DungeonInfo
void sendGameDescription(uint32_t packed_moves1, uint32_t packed_moves2) {
    char description_buffer[1024];
    size_t len = 0;
    int transmit_status = 0;

    bzero(description_buffer, sizeof(description_buffer));

    // Extract characters from packed uint32_t parameters
    char left_key = (char)(packed_moves1 & 0xFF);
    char right_key = (char)((packed_moves1 >> 8) & 0xFF);
    char jump_key = (char)((packed_moves1 >> 16) & 0xFF);
    char quit_key = (char)((packed_moves1 >> 24) & 0xFF);

    char jump_left_key = (char)(packed_moves2 & 0xFF);
    char jump_right_key = (char)((packed_moves2 >> 8) & 0xFF);
    char wait_key = (char)((packed_moves2 >> 16) & 0xFF);

    sprintf(description_buffer,
            "Game moves\n----------\nLeft: %c\nRight: %c\nJump: %c\nJump Left: %c\nJump Right: %c\nWait: %c\nQuit game: %c\n",
            left_key, right_key, jump_key, jump_left_key, jump_right_key, wait_key, quit_key);

    len = strlen(description_buffer);
    transmit_status = transmit_all(1, description_buffer, len);
    if (transmit_status != 0) {
        _terminate(2);
    }
}

// Function: sendMenuInstruction
// Parameters adjusted to reflect byte-packed characters in main's DungeonInfo
void sendMenuInstruction(uint32_t quit_packed, uint32_t play_packed, uint32_t inst_hs_packed) {
    char menu_buffer[1024];
    size_t len = 0;
    int transmit_status = 0;

    bzero(menu_buffer, sizeof(menu_buffer));

    // Extract characters from packed uint32_t parameters
    char play_game_key = (char)((play_packed >> 16) & 0xFF);
    char get_instructions_key = (char)(inst_hs_packed & 0xFF);
    char high_scores_key = (char)((inst_hs_packed >> 8) & 0xFF);
    char quit_game_key = (char)(quit_packed & 0xFF);

    sprintf(menu_buffer,
            "Menu\n-----\nPlay game: %c\nGet instructions: %c\nHigh Scores: %c\nQuit game: %c\n",
            play_game_key, get_instructions_key, high_scores_key, quit_game_key);

    len = strlen(menu_buffer);
    transmit_status = transmit_all(1, menu_buffer, len);
    if (transmit_status != 0) {
        _terminate(2);
    }
}

// Function: playGame
uint32_t playGame(DungeonInfo *dungeon_info) { // Changed param type to DungeonInfo* and return type to uint32_t
    char move_char;
    int move_result;

    sendCurrentDungeonView(dungeon_info->current_dungeon_view_id);
    while (1) {
        move_char = getMove();
        // Compare with the quit key stored in dungeon_info
        if (move_char == (char)(dungeon_info->menu_quit_key_game_description_moves_packed1 & 0xFF)) {
            return 3; // Game quit
        }
        move_result = makeMove(dungeon_info, move_char);
        if (move_result == 1) {
            break; // Player won
        }
        if (move_result == 2) {
            playerDied(dungeon_info->menu_quit_key_game_description_moves_packed1,
                       dungeon_info->menu_play_key_game_description_moves_packed2,
                       dungeon_info->menu_inst_hs_keys,
                       dungeon_info->current_dungeon_view_id,
                       dungeon_info->player_data_1,
                       dungeon_info->high_score_list_head);
            return 2; // Player died
        }
    }
    playerWon(dungeon_info);
    return 1; // Player won
}

// Function: sendHighScores
void sendHighScores(uint32_t *high_score_list_head) { // Renamed param for clarity, kept uint32_t*
    char buffer[61]; // local_55 was 0x3d = 61 bytes
    int score_rank = 2; // local_10 initialized to 2

    if (high_score_list_head == NULL) {
        size_t len = strlen("NO HIGH SCORES!\n");
        int transmit_status = transmit_all(1, "NO HIGH SCORES!\n", len);
        if (transmit_status != 0) {
            _terminate(0); // Status 0 for this case, not 2
        }
        return;
    }

    // First score entry (which is the param_1 itself)
    bzero(buffer, sizeof(buffer));
    // Assuming *high_score_list_head is char* name, high_score_list_head[1] is score
    sprintf(buffer, "Dungeon conquered in %u moves %s\n", high_score_list_head[1], (char *)*high_score_list_head);
    size_t len = strlen(buffer);
    int transmit_status = transmit_all(1, buffer, len);
    if (transmit_status != 0) {
        _terminate(2);
    }

    // Separator
    const char *separator = "\n-------------------\n moves   |   name  \n-------------------\n";
    len = strlen(separator);
    transmit_status = transmit_all(1, separator, len);
    if (transmit_status != 0) {
        _terminate(2);
    }

    // Iterate through the rest of the high scores linked list
    // high_score_list_head[2] is the pointer to the next score entry
    for (uint32_t *current_score_entry = (uint32_t *)high_score_list_head[2];
         current_score_entry != NULL;
         current_score_entry = (uint32_t *)current_score_entry[2]) {

        bzero(buffer, sizeof(buffer));
        // Assuming *current_score_entry is char* name, current_score_entry[1] is score
        sprintf(buffer, "%u. %u  %s\n", score_rank, current_score_entry[1], (char *)*current_score_entry);
        score_rank++;

        len = strlen(buffer);
        transmit_status = transmit_all(1, buffer, len);
        if (transmit_status != 0) {
            _terminate(2);
        }
    }
}

// Function: initScoreboard
void initScoreboard(DungeonInfo *dungeon_info) { // Changed param type to DungeonInfo*
    uint32_t *new_score_entry;
    uint32_t old_head;

    // First score entry
    new_score_entry = (uint32_t *)malloc(0xc); // Allocate 12 bytes for score entry (name_ptr, score, next_ptr)
    if (new_score_entry == NULL) {
        _terminate(1);
    }
    bzero(new_score_entry, 0xc);
    *new_score_entry = (uint32_t)getRandomName(); // Name pointer
    new_score_entry[1] = 600;                     // Score
    new_score_entry[2] = 0;                       // Next pointer (initially NULL)
    
    old_head = dungeon_info->high_score_list_head; // Get current head of list
    dungeon_info->high_score_list_head = insertNewScore(new_score_entry, old_head); // Insert and update head

    // Second score entry
    new_score_entry = (uint32_t *)malloc(0xc);
    if (new_score_entry == NULL) {
        _terminate(1);
    }
    bzero(new_score_entry, 0xc);
    *new_score_entry = (uint32_t)getRandomName();
    new_score_entry[1] = 0x259; // 601 in decimal
    new_score_entry[2] = 0;
    
    old_head = dungeon_info->high_score_list_head;
    dungeon_info->high_score_list_head = insertNewScore(new_score_entry, old_head);

    // Third score entry
    new_score_entry = (uint32_t *)malloc(0xc);
    if (new_score_entry == NULL) {
        _terminate(1);
    }
    bzero(new_score_entry, 0xc);
    *new_score_entry = (uint32_t)getRandomName();
    new_score_entry[1] = 999999;
    new_score_entry[2] = 0;
    
    old_head = dungeon_info->high_score_list_head;
    dungeon_info->high_score_list_head = insertNewScore(new_score_entry, old_head);
}

// Function: main
uint32_t main(void) { // Changed return type to uint32_t
    DungeonInfo dungeon_info; // Use the struct for dungeon information
    char menu_choice_char;
    int game_result = 0;

    // Initialize the struct to zero
    bzero(&dungeon_info, sizeof(DungeonInfo));

    buildDungeon(&dungeon_info);
    initScoreboard(&dungeon_info);

    while (1) {
        while (1) {
            sendMenuInstruction(dungeon_info.menu_quit_key_game_description_moves_packed1,
                                dungeon_info.menu_play_key_game_description_moves_packed2,
                                dungeon_info.menu_inst_hs_keys);
            menu_choice_char = getMove();

            // Compare menu_choice_char with keys stored in dungeon_info
            char play_game_key = (char)((dungeon_info.menu_play_key_game_description_moves_packed2 >> 16) & 0xFF);
            char get_instructions_key = (char)(dungeon_info.menu_inst_hs_keys & 0xFF);
            char high_scores_key = (char)((dungeon_info.menu_inst_hs_keys >> 8) & 0xFF);
            char quit_game_key = (char)(dungeon_info.menu_quit_key_game_description_moves_packed1 & 0xFF);

            if (menu_choice_char == play_game_key) {
                break; // Exit inner loop to play game
            } else if (menu_choice_char == get_instructions_key) {
                sendGameDescription(dungeon_info.menu_quit_key_game_description_moves_packed1,
                                    dungeon_info.menu_play_key_game_description_moves_packed2);
            } else if (menu_choice_char == high_scores_key) {
                sendHighScores((uint32_t *)dungeon_info.high_score_list_head);
            } else if (menu_choice_char == quit_game_key) {
                return 0; // Quit application
            }
        }
        game_result = playGame(&dungeon_info);
        if (game_result == 3) { // Player explicitly quit during game
            break;
        }
        destroyDungeon(&dungeon_info);
        buildDungeon(&dungeon_info);
    }
    return 0;
}