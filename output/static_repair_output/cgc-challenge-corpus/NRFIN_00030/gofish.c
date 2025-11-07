#include <stdlib.h> // For calloc, free
#include <string.h> // For strncpy
#include <stddef.h> // For size_t (though often included by stdlib.h)

// Type definitions for clarity, mimicking the decompiler's `undefined` and `byte`
typedef unsigned char byte;

// Define the Game struct based on memory access patterns observed in the original code
struct Game {
    byte current_turn_player; // Offset 0x00, used by is_player_turn, turn_complete
    char _padding0[3];        // Padding to align next int to 4 bytes
    int deck_id;              // Offset 0x04, used by get_shuffled_deck, take_top_card, is_deck_empty
    int player1_id;           // Offset 0x08, used by create_player, take_top_card, is_player_hand_empty
    int player2_id;           // Offset 0x0c, used by create_player, take_top_card, is_player_hand_empty
}; // Total size 16 bytes (1 + 3 + 4 + 4 + 4 = 16)

// External function declarations (assumed signatures based on usage)
// These functions are called but not defined in the provided snippet.
// They would typically be found in a separate header file.
int get_shuffled_deck(char game_type);
int create_player(int player_id, char *name_or_null);
int take_top_card(int player_id, int deck_id);
int is_deck_empty(int deck_id);
int is_player_hand_empty(int player_id);


// Function: create_game
struct Game *create_game(char *param_1) {
    struct Game *game = (struct Game *)calloc(1, sizeof(struct Game));
    if (!game) {
        return NULL; // Handle allocation failure
    }

    // The first byte (current_turn_player) is already 0 due to calloc.

    char game_type = '\0';
    if (param_1 && *param_1 != '\0') {
        game_type = *param_1;
    }

    game->deck_id = get_shuffled_deck(game_type);
    game->player1_id = create_player(0, NULL); // Player 0, no specific name

    char *bot_name = (char *)calloc(1, 4); // Allocate 4 bytes for "Bot\0"
    if (!bot_name) {
        free(game); // Clean up previously allocated memory
        return NULL; // Handle allocation failure
    }
    strncpy(bot_name, "Bot", 3); // Copies "Bot", calloc ensures null termination

    game->player2_id = create_player(1, bot_name);
    free(bot_name); // Free the temporary buffer for "Bot"

    return game;
}

// Function: deal
int deal(struct Game *game) {
    if (game == NULL) {
        return -54; // 0xffffffca in signed 32-bit int is -54
    }

    for (byte i = 0; i < get_hand_size(); i++) {
        int result = take_top_card(game->player1_id, game->deck_id);
        if (result < 0) {
            return result;
        }
        result = take_top_card(game->player2_id, game->deck_id);
        if (result < 0) {
            return result;
        }
    }
    return 0;
}

// Function: get_hand_size
int get_hand_size(void) {
    return 7;
}

// Function: turn_complete
int turn_complete(byte *current_player_turn_ptr) {
    if (current_player_turn_ptr == NULL) {
        return -54; // 0xffffffca in signed 32-bit int is -54
    }
    *current_player_turn_ptr = (*current_player_turn_ptr + 1) & 1;
    return 0;
}

// Function: is_player_turn
int is_player_turn(byte *current_player_turn_ptr) {
    if (current_player_turn_ptr == NULL) {
        return -54; // 0xffffffca in signed 32-bit int is -54
    }
    // If *current_player_turn_ptr is 0, it means player 0's turn. Return 1.
    // If *current_player_turn_ptr is 1, it means player 1's turn. Return 0.
    return !(*current_player_turn_ptr);
}

// Function: is_game_over
int is_game_over(struct Game *game) {
    if (game == NULL) {
        return -54; // 0xffffffca in signed 32-bit int is -54
    }

    // Check if deck is empty AND player1's hand is empty AND player2's hand is empty
    if (is_deck_empty(game->deck_id) == 1 &&
        is_player_hand_empty(game->player1_id) == 1 &&
        is_player_hand_empty(game->player2_id) == 1) {
        return 1; // Game is over
    }
    return 0; // Game is not over
}