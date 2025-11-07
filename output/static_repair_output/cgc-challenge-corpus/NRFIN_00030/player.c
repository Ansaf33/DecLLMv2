#include <stdlib.h> // For calloc, free
#include <stddef.h> // For size_t

// --- Mock External Functions ---
// These are placeholder implementations for functions called in the snippet
// but not defined within it. They allow the provided code to compile.
// In a real application, these would be provided by other modules.

// Returns an integer handle for a newly created hand. 0 for error.
int create_hand() {
    static int next_hand_id = 1000;
    return next_hand_id++;
}

// Pops the top card from a deck identified by deck_handle. Returns card value or error.
int pop(int deck_handle) {
    if (deck_handle == 0) return -1; // Invalid deck handle
    static int next_card_val = 1;
    return next_card_val++; // Dummy card value
}

// Adds a card to the hand identified by hand_handle. Returns 0 on success, <0 on error.
int add_card_to_hand(int hand_handle, int card_val) {
    if (hand_handle == 0) return -1; // Invalid hand handle
    // In a real system, this would modify the hand data structure
    (void)card_val; // Suppress unused parameter warning
    return 0; // Success
}

// Retrieves all cards of a specific rank from a hand.
// Stores card values in card_list_ptr (assumed to be an array of int).
// Returns the number of cards found, or <0 on error.
int get_all_of_rank_from_hand(int hand_handle, unsigned char rank, int *card_list_ptr, int some_magic_val) {
    if (hand_handle == 0) return -1;
    if (card_list_ptr == NULL) return -2;
    // For mocking, let's just fill it with dummy values if rank is 1
    if (rank == 1) { // Assume rank 1 always has 4 cards for testing play_books
        if (some_magic_val == 0x13350) { // Specific magic value for play_books
            card_list_ptr[0] = 101; card_list_ptr[1] = 102; card_list_ptr[2] = 103; card_list_ptr[3] = 104;
            return 4;
        }
    }
    return 0; // No cards found for other ranks
}

// Returns the number of cards in the hand.
int get_count_cards_in_hand(int hand_handle) {
    if (hand_handle == 0) return -1;
    // Dummy count for testing
    return 5; // Assume hand has some cards
}

// Returns the quantity of cards of a specific rank in the hand.
int qty_of_rank_in_hand(int hand_handle, unsigned char rank) {
    if (hand_handle == 0) return -1;
    // Dummy quantity for testing play_books
    if (rank == 1) return 4; // Assume 4 cards of rank 1 for testing
    return 0;
}

// Destroys a card (e.g., frees its memory).
void destroy_card(int card_handle) {
    // In a real system, this would free card resources.
    (void)card_handle; // Suppress unused parameter warning
}

// Checks if the hand is empty. Returns 1 if empty, 0 if not, <0 on error.
int is_hand_empty(int hand_handle) {
    if (hand_handle == 0) return -1;
    return 0; // Dummy: Hand is not empty
}

// Returns the rank of a random card in the hand.
int get_rank_of_random_card_in_hand(int hand_handle) {
    if (hand_handle == 0) return -1;
    return 7; // Dummy rank
}

// Adds a list of cards to the hand.
int add_cards_to_hand(int hand_handle, int cards_list_handle, unsigned char num_cards, int some_magic_val) {
    if (hand_handle == 0 || cards_list_handle == 0) return -1;
    (void)num_cards; // Suppress unused parameter warning
    (void)some_magic_val; // Suppress unused parameter warning
    return 0; // Success
}

// Gets the latest card from the hand into a container.
int get_latest_card(int hand_handle, int card_container_handle) {
    if (hand_handle == 0 || card_container_handle == 0) return -1;
    return 0; // Dummy card value
}

// --- Player Structure Definition ---
// Based on the memory access patterns (offsets 0, 1, 4, 8) and total size (0xc = 12 bytes).
// This implies a 32-bit system where pointers/handles are 4 bytes, or a packed struct.
typedef struct Player {
    char player_id_char; // Offset 0: First byte for a char ID
    char books_count;    // Offset 1: Second byte for number of books
    // (Implicit 2 bytes of padding here to align `player_id_int` to offset 4)
    int player_id_int;   // Offset 4: An integer ID for the player
    int hand_handle;     // Offset 8: An integer handle/ID to the player's hand
} Player; // Total size should be 12 bytes (1+1+2+4+4)

// --- Function Implementations ---

// Function: create_player
Player *create_player(char player_id_char, int player_id_int) {
    Player *player = (Player *)calloc(1, sizeof(Player)); // Allocate 1 Player struct (12 bytes)
    if (player == NULL) {
        return NULL; // Allocation failed
    }
    player->player_id_char = player_id_char;
    player->books_count = 0;
    player->player_id_int = player_id_int;
    player->hand_handle = create_hand(); // Assign a new hand handle
    return player;
}

// Function: take_top_card
int take_top_card(Player *player, int deck_handle) {
    if (player == NULL) {
        return -15; // Error: Invalid player pointer (0xfffffff1)
    }
    if (deck_handle == 0) {
        return -32; // Error: Invalid deck handle (0xffffffe0)
    }
    int card_val = pop(deck_handle);
    if (card_val < 0) { // Check if pop returned an error
        return card_val;
    }
    return add_card_to_hand(player->hand_handle, card_val);
}

// Function: have_cards_of_rank
int have_cards_of_rank(Player *player, unsigned char rank, int *card_list_ptr) {
    if (player == NULL) {
        return -15; // Error: Invalid player pointer (0xfffffff1)
    }
    // 0x1330b is a magic number from the original snippet
    return get_all_of_rank_from_hand(player->hand_handle, rank, card_list_ptr, 0x1330b);
}

// Function: play_books
int play_books(Player *player) {
    if (player == NULL) {
        return -15; // Error: Invalid player pointer (0xfffffff1)
    }

    const int MAGIC_VAL_PLAY_BOOKS = 0x13350;
    int hand_card_count = get_count_cards_in_hand(player->hand_handle);

    if (hand_card_count <= 3) {
        return 0; // Not enough cards to form a book (need 4 of a kind)
    }

    int total_books_played = 0;
    int cards_of_rank[4]; // Array to hold 4 cards of the same rank

    // Iterate through all possible ranks (1 to 13, typically Ace to King)
    for (unsigned int rank = 1; rank <= 13; ++rank) {
        int qty = qty_of_rank_in_hand(player->hand_handle, (unsigned char)rank);
        if (qty < 0) {
            return qty; // Propagate error from qty_of_rank_in_hand
        }
        if (qty == 4) { // Found a book (4 cards of this rank)
            int result = get_all_of_rank_from_hand(player->hand_handle, (unsigned char)rank, cards_of_rank, MAGIC_VAL_PLAY_BOOKS);
            if (result < 0) {
                return result; // Propagate error
            }
            // Destroy the cards that formed the book
            for (int i = 0; i < 4; ++i) {
                destroy_card(cards_of_rank[i]);
            }
            player->books_count++; // Increment player's book count
            total_books_played++;
        }
    }
    return total_books_played;
}

// Function: is_player_hand_empty
int is_player_hand_empty(Player *player) {
    if (player == NULL) {
        return -15; // Error: Invalid player pointer (0xfffffff1)
    }
    return is_hand_empty(player->hand_handle); // Returns 1 if empty, 0 if not, <0 on error
}

// Function: draw_new_hand
int draw_new_hand(Player *player, int deck_handle, unsigned char num_cards_to_draw) {
    if (player == NULL) {
        return -15; // Error: Invalid player pointer (-0xf)
    }
    if (deck_handle == 0) {
        return -32; // Error: Invalid deck handle (-0x20)
    }
    if (num_cards_to_draw == 0 || num_cards_to_draw > 7) {
        return -52; // Error: Invalid number of cards to draw (0x34)
    }

    int last_card_result = 0; // Stores the result of the last take_top_card call
    for (unsigned char i = 0; i < num_cards_to_draw; ++i) {
        last_card_result = take_top_card(player, deck_handle);
        if (last_card_result < 0) {
            return last_card_result; // Return error immediately
        }
    }
    return last_card_result; // Return the result of the last successful card addition
}

// Function: select_random_card
int select_random_card(Player *player) {
    if (player == NULL) {
        return -15; // Error: Invalid player pointer (0xfffffff1)
    }
    if (get_count_cards_in_hand(player->hand_handle) == 0) {
        return -24; // Error: Hand is empty (0xffffffe8)
    }
    return get_rank_of_random_card_in_hand(player->hand_handle);
}

// Function: accept_cards
int accept_cards(Player *player, int cards_list_handle, unsigned char num_cards) {
    if (player == NULL) {
        return -15; // Error: Invalid player pointer (0xfffffff1)
    }
    if (cards_list_handle == 0) {
        return -53; // Error: Invalid cards list handle (0xffffffcb)
    }
    // 0x135b0 is a magic number from the original snippet
    return add_cards_to_hand(player->hand_handle, cards_list_handle, num_cards, 0x135b0);
}

// Function: get_players_newest_card
int get_players_newest_card(Player *player, int card_container_handle) {
    if (player == NULL) {
        return -15; // Error: Invalid player pointer (0xfffffff1)
    }
    if (card_container_handle == 0) {
        return -53; // Error: Invalid card container handle (0xffffffcb)
    }
    return get_latest_card(player->hand_handle, card_container_handle);
}

// --- Main function for compilation and basic testing ---
int main() {
    // Example usage of the functions
    Player *player1 = create_player('A', 101);
    if (player1 == NULL) {
        // Handle player creation error
        return 1;
    }

    int deck_handle = 2000; // Dummy deck handle

    // Draw a new hand of 5 cards
    int draw_result = draw_new_hand(player1, deck_handle, 5);
    if (draw_result < 0) {
        // Handle draw error
    }

    // Play books (if any)
    int books_played = play_books(player1);
    if (books_played < 0) {
        // Handle play_books error
    }

    // Check if hand is empty
    int hand_empty = is_player_hand_empty(player1);
    if (hand_empty < 0) {
        // Handle error
    }

    // Select a random card
    int random_rank = select_random_card(player1);
    if (random_rank < 0) {
        // Handle error
    }

    // Free player memory
    free(player1);

    return 0;
}