#include <stdlib.h> // For calloc, free
#include <string.h> // For strncpy
#include <stdint.h> // For fixed-width integer types if needed

// --- Placeholder declarations for external functions ---
// These functions are assumed to be defined elsewhere and are necessary
// for the provided snippets to compile and link. Their signatures are
// inferred from their usage in the given code.
//
// get_shuffled_deck: Takes a character representing a game type, returns
//                    an integer handle to a shuffled deck.
int get_shuffled_deck(char game_type);

// create_player: Takes a player ID (e.g., 0 for human),
//                returns an integer handle to the player.
int create_player(int player_id);

// create_player_with_name: An alternative create_player that takes a name.
//                          Used for the bot player.
int create_player_with_name(int player_id, const char* name);

// take_top_card: Deals a card from the deck to a player.
//                Returns 0 on success, or a negative error code.
int take_top_card(int player_handle, int deck_handle);

// is_deck_empty: Checks if the deck has no more cards.
//                Returns 1 if empty, 0 otherwise.
int is_deck_empty(int deck_handle);

// is_player_hand_empty: Checks if a player's hand has no cards.
//                       Returns 1 if empty, 0 otherwise.
int is_player_hand_empty(int player_handle);

// get_hand_size: Returns the number of cards to deal initially.
//                Defined below in the snippet, but declared here for order.
int get_hand_size(void);

// --- Error codes inferred from the original snippet ---
#define ERROR_INVALID_GAME_POINTER -54 // Corresponds to -0x36 and 0xffffffca

// --- Game Structure Definition ---
// Based on the memory access patterns (offsets 0, 4, 8, 12 from base pointer)
// and the size (0x10 = 16 bytes) in calloc, this structure is inferred.
// Assuming 'int' is 4 bytes on the target system.
typedef struct Game {
    int current_player_turn; // Offset 0: Indicates whose turn it is (e.g., 0 for player, 1 for bot)
    int deck_handle;         // Offset 4: Handle to the game deck
    int player_human_handle; // Offset 8: Handle to the human player
    int player_bot_handle;   // Offset 12: Handle to the bot player
} Game;

// Function: create_game
Game *create_game(char *game_type_param) {
    // Allocate memory for one Game structure. The original calloc(0x10, ...) implies 16 bytes total.
    Game *game = (Game *)calloc(1, sizeof(Game));
    if (!game) {
        return NULL; // Handle memory allocation failure
    }

    // Initialize current player turn to 0 (e.g., human player's turn)
    game->current_player_turn = 0;

    // Determine the game type character. If game_type_param is NULL or empty, use '\0'.
    char game_type_char = (game_type_param && *game_type_param != '\0') ? *game_type_param : '\0';
    game->deck_handle = get_shuffled_deck(game_type_char);
    game->player_human_handle = create_player(0); // Player 0 (human)

    // Create a temporary string for the bot's name.
    char *bot_name_str = (char *)calloc(1, 4); // Allocate space for "Bot\0"
    if (!bot_name_str) {
        free(game); // Clean up previously allocated game structure
        return NULL; // Handle memory allocation failure
    }
    strncpy(bot_name_str, "Bot", 3);
    bot_name_str[3] = '\0'; // Ensure null-termination

    game->player_bot_handle = create_player_with_name(1, bot_name_str); // Player 1 (bot)
    free(bot_name_str); // Free the temporary string, as create_player_with_name likely copies it

    return game;
}

// Function: deal
int deal(Game *game) {
    if (!game) {
        return ERROR_INVALID_GAME_POINTER; // Return error for NULL game pointer
    }

    int hand_size = get_hand_size(); // Get hand size once before the loop
    for (int i = 0; i < hand_size; ++i) {
        // Deal a card to the human player
        int result = take_top_card(game->player_human_handle, game->deck_handle);
        if (result < 0) {
            return result; // Propagate error if dealing fails
        }

        // Deal a card to the bot player
        result = take_top_card(game->player_bot_handle, game->deck_handle);
        if (result < 0) {
            return result; // Propagate error if dealing fails
        }
    }
    return 0; // Success
}

// Function: get_hand_size
int get_hand_size(void) {
    return 7;
}

// Function: turn_complete
int turn_complete(int *current_player_turn_ptr) {
    if (!current_player_turn_ptr) {
        return ERROR_INVALID_GAME_POINTER; // Return error for NULL pointer
    }
    // Toggle the turn between 0 and 1 using bitwise AND
    *current_player_turn_ptr = (*current_player_turn_ptr + 1) & 1;
    return 0; // Success
}

// Function: is_player_turn
int is_player_turn(int *current_player_turn_ptr) {
    if (!current_player_turn_ptr) {
        return ERROR_INVALID_GAME_POINTER; // Return error for NULL pointer
    }
    // Returns 1 if it's player 0's turn, 0 otherwise.
    // Assuming '0' means player's turn (e.g., human) and '1' means bot's turn.
    return (*current_player_turn_ptr == 0) ? 1 : 0;
}

// Function: is_game_over
int is_game_over(Game *game) {
    if (!game) {
        return ERROR_INVALID_GAME_POINTER; // Return error for NULL game pointer
    }

    // The game is over if the deck is empty AND both players' hands are empty.
    if (is_deck_empty(game->deck_handle) == 1 &&
        is_player_hand_empty(game->player_human_handle) == 1 &&
        is_player_hand_empty(game->player_bot_handle) == 1) {
        return 1; // Game is over
    }
    return 0; // Game is not over
}