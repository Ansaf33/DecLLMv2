#include <stdlib.h> // For calloc, rand, srand
#include <stdint.h> // For uint32_t, uint8_t
#include <stdbool.h> // For bool
#include <time.h>   // For time (to seed PRNG, though not strictly part of snippet fix)

// --- Placeholder functions (not provided in original snippet, assuming their signatures and purpose) ---
// Assuming a card is represented as a 32-bit unsigned integer
// For example, higher bits for suit, lower bits for rank
uint32_t create_card(int suit, int rank) {
    // Example implementation: (suit << 8) | rank
    // Assuming suit is 1-4, rank is 1-13
    return (uint32_t)((suit << 8) | rank);
}

// Assuming a Pseudo-Random Number Generator function
// Returns an integer, likely used for modulo operations to get an index
int prng_get_next(void) {
    // For compilability and basic functionality, use rand().
    // In a real application, srand() should be called once, e.g., in main.
    return rand();
}
// --- End of placeholder functions ---

// Define the total number of cards in a standard deck
#define DECK_SIZE 52

// Function: create_deck
// Allocates memory for a deck of cards and initializes it.
// The first element (index 0) stores the current card count.
// Cards are stored from index 1 to DECK_SIZE.
uint32_t *create_deck(void) {
    // Allocate space for DECK_SIZE cards + 1 element for the card count
    uint32_t *deck = (uint32_t *)calloc(DECK_SIZE + 1, sizeof(uint32_t));
    if (deck == NULL) {
        return NULL; // Handle allocation failure
    }

    int card_index = 0; // Index for storing cards in the deck array (0 to DECK_SIZE - 1 for array access)
    for (int suit = 1; suit <= 4; ++suit) { // Suits (e.g., 1-4)
        for (int rank = 1; rank <= 13; ++rank) { // Ranks (e.g., 1-13)
            deck[1 + card_index] = create_card(suit, rank); // Store cards from deck[1] onwards
            card_index++;
        }
    }
    // Set the initial count of cards in the deck
    deck[0] = card_index; // Should be DECK_SIZE (52)

    return deck;
}

// Function: shuffle_deck
// Shuffles the cards in the deck using a simple swap method.
// deck[0] is assumed to be the card count. Cards are from deck[1] to deck[deck[0]].
// Returns 0 on success, -1 on error (e.g., null deck).
int shuffle_deck(uint32_t *deck, uint8_t seed_offset) {
    if (deck == NULL) {
        return -1; // Original 0xffffffe0 likely indicated an error
    }

    int current_card_count = deck[0];
    if (current_card_count == 0) {
        return 0; // Nothing to shuffle
    }

    int idx1 = seed_offset % current_card_count; // Initial index based on seed
    int idx2; // Second index for swapping

    for (int i = 0; i < current_card_count; ++i) {
        // Calculate the first index for swapping (0 to current_card_count - 1)
        idx1 = (idx1 + i) % current_card_count;

        // Get a random number and calculate the second index for swapping
        idx2 = (prng_get_next() + i) % current_card_count; // Use 'i' to introduce more variation

        // Ensure indices are different before swapping
        if (idx1 != idx2) {
            // Swap cards at deck[1 + idx1] and deck[1 + idx2]
            uint32_t temp_card = deck[1 + idx1];
            deck[1 + idx1] = deck[1 + idx2];
            deck[1 + idx2] = temp_card;
        }
    }
    return 0; // Success
}

// Function: get_shuffled_deck
// Creates a new deck and then shuffles it.
// Returns a pointer to the shuffled deck, or NULL if creation fails.
uint32_t *get_shuffled_deck(uint8_t seed_offset) {
    uint32_t *deck = create_deck();
    if (deck != NULL) {
        shuffle_deck(deck, seed_offset);
    }
    return deck;
}

// Function: pop
// Removes and returns the top card from the deck.
// deck[0] is assumed to be the current card count.
// Returns the card value, or 0 if the deck is null or empty.
uint32_t pop(uint32_t *deck) {
    if (deck == NULL || deck[0] == 0) {
        return 0; // Return 0 for null or empty deck
    }

    // Decrement the card count
    deck[0]--;
    // Get the card from the new top of the deck (which was the previous top)
    uint32_t card = deck[deck[0] + 1];
    // Zero out the slot where the card was
    deck[deck[0] + 1] = 0; // Mark the slot as empty (optional, but consistent with original)

    return card;
}

// Function: is_deck_empty
// Checks if the deck is empty.
// deck[0] is assumed to be the current card count.
// Returns true if empty or deck is NULL, false otherwise.
bool is_deck_empty(uint32_t *deck) {
    if (deck == NULL) {
        return true; // A null deck can be considered empty
    }
    return deck[0] == 0;
}