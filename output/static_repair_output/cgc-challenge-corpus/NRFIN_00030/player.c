#include <stdlib.h> // For calloc, free, NULL
#include <stdio.h>  // For printf (in main and mocks, for demonstration)
#include <stdint.h> // For int32_t, uint32_t if precise types are needed, but int is usually fine for undefined4

// Define Player structure based on memory access patterns
typedef struct Player {
    char id;            // param_1 in create_player, accessed as *puVar1
    char num_books;     // puVar1[1], incremented in play_books
    char _padding[2];   // Padding to align 'score' to a 4-byte boundary on most systems
    int score;          // param_2 in create_player, accessed as *(puVar1 + 4)
    int hand;           // result of create_hand(), accessed as *(puVar1 + 8)
} Player;

// --- Mock external functions to make the code compilable ---
// In a real application, these would be defined elsewhere.
// They return placeholder values or perform minimal actions.

int create_hand() {
    // Mock: Returns a placeholder integer representing a hand object/ID.
    // In a real system, this might allocate a Hand struct and return a pointer.
    return 100; // Example hand ID
}

int pop(int deck_ptr) {
    // Mock: Simulates taking the top card from a deck.
    // Returns a placeholder card ID.
    (void)deck_ptr; // Suppress unused parameter warning
    return 1; // Example card ID
}

int add_card_to_hand(int hand_ptr, int card_val) {
    // Mock: Simulates adding a card to a hand.
    // Returns 0 on success, negative on error.
    (void)hand_ptr;
    (void)card_val;
    return 0; // Success
}

int get_all_of_rank_from_hand(int hand_ptr, char rank, int *card_list_ptr, int magic_num) {
    // Mock: Retrieves all cards of a specific rank from a hand.
    // Fills 'card_list_ptr' with up to 4 card IDs.
    (void)hand_ptr;
    (void)rank;
    (void)magic_num;
    if (card_list_ptr) {
        card_list_ptr[0] = 10;
        card_list_ptr[1] = 11;
        card_list_ptr[2] = 12;
        card_list_ptr[3] = 13;
    }
    return 0; // Success
}

void destroy_card(int card_val) {
    // Mock: Simulates destroying a card object (e.g., freeing memory).
    (void)card_val;
}

int get_count_cards_in_hand(int hand_ptr) {
    // Mock: Returns the number of cards in a hand.
    (void)hand_ptr;
    return 5; // Example count
}

int qty_of_rank_in_hand(int hand_ptr, char rank) {
    // Mock: Returns the quantity of cards of a specific rank in a hand.
    (void)hand_ptr;
    // Example: Return 4 if rank is 7, otherwise 0
    return (rank == 7) ? 4 : 0;
}

int is_hand_empty(int hand_ptr) {
    // Mock: Checks if a hand is empty.
    // Returns 1 if empty, 0 if not.
    (void)hand_ptr;
    return 0; // Example: Hand is never empty
}

int get_rank_of_random_card_in_hand(int hand_ptr) {
    // Mock: Returns the rank of a random card in the hand.
    (void)hand_ptr;
    return 5; // Example rank
}

int add_cards_to_hand(int hand_ptr, int cards_ptr, char count, int magic_num) {
    // Mock: Adds multiple cards to a hand.
    (void)hand_ptr;
    (void)cards_ptr;
    (void)count;
    (void)magic_num;
    return 0; // Success
}

int get_latest_card(int hand_ptr, int *card_output_ptr) {
    // Mock: Retrieves the latest card added to a hand.
    (void)hand_ptr;
    if (card_output_ptr) {
        *card_output_ptr = 20; // Example card ID
    }
    return 0; // Success
}

// --- Fixed and Refactored Functions ---

// Function: create_player
Player *create_player(char player_id, int player_score) {
    Player *player = (Player *)calloc(1, sizeof(Player));
    if (player == NULL) {
        return NULL; // Return NULL on allocation failure
    }
    player->id = player_id;
    player->num_books = 0; // Initialize num_books
    player->score = player_score;
    player->hand = create_hand(); // Assign result directly
    return player;
}

// Function: take_top_card
int take_top_card(Player *player, int deck_ptr) {
    if (player == NULL) {
        return -15; // 0xfffffff1
    }
    if (deck_ptr == 0) {
        return -32; // 0xffffffe0
    }
    // Reduce intermediate variable: pass pop result directly to add_card_to_hand
    return add_card_to_hand(player->hand, pop(deck_ptr));
}

// Function: have_cards_of_rank
int have_cards_of_rank(Player *player, char rank, int *card_list_ptr) {
    if (player == NULL) {
        return -15; // 0xfffffff1
    }
    // Reduce intermediate variable: return result directly
    return get_all_of_rank_from_hand(player->hand, rank, card_list_ptr, 0x1330b);
}

// Function: play_books
int play_books(Player *player) {
    if (player == NULL) {
        return -15; // -0xf
    }

    int card_count = get_count_cards_in_hand(player->hand);
    if (card_count <= 3) {
        return 0; // No books possible with less than 4 cards
    }

    int books_played_count = 0;
    int cards_of_rank[4]; // Array to store card pointers
    const int magic_num_get_all = 0x13350;

    // Loop through all possible ranks (1 to 13)
    for (unsigned int rank_idx = 1; rank_idx <= 13; ++rank_idx) {
        int qty = qty_of_rank_in_hand(player->hand, (char)rank_idx);
        if (qty < 0) {
            return qty; // Return error if qty_of_rank_in_hand failed
        }
        if (qty == 4) {
            // Found a book (4 cards of the same rank)
            int result_get_all = get_all_of_rank_from_hand(
                player->hand, (char)rank_idx, cards_of_rank, magic_num_get_all);
            if (result_get_all < 0) {
                return result_get_all; // Return error if get_all_of_rank_from_hand failed
            }
            // Destroy the four cards
            destroy_card(cards_of_rank[0]);
            destroy_card(cards_of_rank[1]);
            destroy_card(cards_of_rank[2]);
            destroy_card(cards_of_rank[3]);

            player->num_books++; // Increment player's book count
            books_played_count++;
        }
    }
    return books_played_count;
}

// Function: is_player_hand_empty
int is_player_hand_empty(Player *player) {
    if (player == NULL) {
        return -15; // 0xfffffff1
    }
    // Reduce intermediate variable: return result directly
    return is_hand_empty(player->hand);
}

// Function: draw_new_hand
int draw_new_hand(Player *player, int deck_ptr, unsigned char num_cards) {
    if (player == NULL) {
        return -15; // -0xf
    }
    if (deck_ptr == 0) {
        return -32; // -0x20
    }
    if (num_cards == 0 || num_cards > 7) {
        return -52; // -0x34
    }

    int cards_drawn_count = 0;
    for (int i = 0; i < num_cards; ++i) {
        // Call take_top_card, passing player pointer directly
        int result = take_top_card(player, deck_ptr);
        if (result < 0) {
            return result; // Return error code if card taking failed
        }
        cards_drawn_count++;
    }
    return cards_drawn_count; // Return number of cards successfully drawn
}

// Function: select_random_card
int select_random_card(Player *player) {
    if (player == NULL) {
        return -15; // 0xfffffff1
    }
    // Reduce intermediate variable: check count directly
    if (get_count_cards_in_hand(player->hand) == 0) {
        return -24; // 0xffffffe8
    }
    // Reduce intermediate variable: return result directly
    return get_rank_of_random_card_in_hand(player->hand);
}

// Function: accept_cards
int accept_cards(Player *player, int cards_ptr, char count) {
    if (player == NULL) {
        return -15; // 0xfffffff1
    }
    if (cards_ptr == 0) { // Assuming cards_ptr cannot be NULL if count > 0
        return -53; // 0xffffffcb
    }
    // Reduce intermediate variable: return result directly
    return add_cards_to_hand(player->hand, cards_ptr, count, 0x135b0);
}

// Function: get_players_newest_card
int get_players_newest_card(Player *player, int *card_output_ptr) {
    if (player == NULL) {
        return -15; // 0xfffffff1
    }
    if (card_output_ptr == 0) {
        return -53; // 0xffffffcb
    }
    // Reduce intermediate variable: return result directly
    return get_latest_card(player->hand, card_output_ptr);
}

// --- Main function to demonstrate usage ---
int main() {
    printf("Starting player game simulation...\n");

    // Create a player
    Player *p1 = create_player(1, 100);
    if (p1 == NULL) {
        fprintf(stderr, "Failed to create player 1.\n");
        return 1;
    }
    printf("Player 1 created. ID: %d, Score: %d, Hand: %d, Books: %d\n",
           p1->id, p1->score, p1->hand, p1->num_books);

    // Mock deck pointer
    int mock_deck_ptr = 200;

    // Take top card
    int take_card_res = take_top_card(p1, mock_deck_ptr);
    printf("take_top_card result: %d\n", take_card_res);

    // Have cards of rank
    int cards_arr[4];
    int have_cards_res = have_cards_of_rank(p1, 7, cards_arr);
    printf("have_cards_of_rank for rank 7 result: %d\n", have_cards_res);

    // Play books
    int initial_books = p1->num_books;
    int books_played = play_books(p1);
    printf("play_books result: %d books played. Player books now: %d (was %d)\n",
           books_played, p1->num_books, initial_books);

    // Check if hand is empty
    int hand_empty_res = is_player_hand_empty(p1);
    printf("is_player_hand_empty result: %d\n", hand_empty_res);

    // Draw new hand
    int cards_to_draw = 3;
    int draw_hand_res = draw_new_hand(p1, mock_deck_ptr, cards_to_draw);
    printf("draw_new_hand result (drawn %d cards): %d\n", cards_to_draw, draw_hand_res);

    // Select random card
    int random_card_rank = select_random_card(p1);
    printf("select_random_card result (rank): %d\n", random_card_rank);

    // Accept cards
    int mock_cards_ptr = 300;
    char num_cards_to_accept = 2;
    int accept_cards_res = accept_cards(p1, mock_cards_ptr, num_cards_to_accept);
    printf("accept_cards result: %d\n", accept_cards_res);

    // Get player's newest card
    int newest_card_id;
    int get_newest_res = get_players_newest_card(p1, &newest_card_id);
    printf("get_players_newest_card result: %d, Newest card ID: %d\n", get_newest_res, newest_card_id);

    // Clean up player memory
    free(p1);
    printf("Player 1 memory freed.\n");

    printf("Simulation finished.\n");
    return 0;
}