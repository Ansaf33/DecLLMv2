#include <stdlib.h>  // For calloc, free
#include <stdbool.h> // For bool, true, false
#include <stdint.h>  // For uint8_t

// Define a Card structure based on the usage in the original code
typedef struct Card {
    uint8_t suit;
    uint8_t rank;
} Card;

// Function: is_valid_suit
bool is_valid_suit(uint8_t suit) {
    return (suit > 0 && suit <= 4);
}

// Function: is_valid_rank
bool is_valid_rank(uint8_t rank) {
    return (rank > 0 && rank <= 13);
}

// Function: create_card
Card *create_card(uint8_t suit, uint8_t rank) {
    if (is_valid_suit(suit) && is_valid_rank(rank)) {
        // Allocate memory for one Card object and initialize to zero
        Card *new_card = (Card *)calloc(1, sizeof(Card));
        if (new_card != NULL) {
            new_card->suit = suit;
            new_card->rank = rank;
        }
        return new_card; // Returns NULL if allocation failed or inputs invalid
    }
    return NULL;
}

// Function: destroy_card
void destroy_card(Card *card) {
    free(card);
}

// Function: is_rank
// This function checks if a given card's rank matches a target rank.
bool is_rank(const Card *card, uint8_t target_rank) {
    // If card is NULL, it cannot have the target_rank.
    return (card != NULL && card->rank == target_rank);
}

// Function: is_equal
bool is_equal(const Card *card1, const Card *card2) {
    // Two NULL cards are considered equal (e.g., representing an empty slot)
    if (card1 == NULL && card2 == NULL) {
        return true;
    }
    // One NULL and one not NULL means they are not equal
    if (card1 == NULL || card2 == NULL) {
        return false;
    }
    // Compare the suit and rank of the cards
    return (card1->rank == card2->rank && card1->suit == card2->suit);
}

// Function: is_set_equal
// Checks if two sets of cards (represented as arrays of Card pointers) are equal,
// meaning they contain the same cards, possibly in a different order,
// and each card appears exactly the same number of times (multiset equality).
bool is_set_equal(Card **set1, Card **set2, unsigned char set_size) {
    if (set_size == 0) {
        return true; // Two empty sets are equal
    }

    // Allocate a boolean array to track which cards in set2 have been matched.
    // calloc initializes the memory to all zeros (false), which is suitable here.
    bool *matched_in_set2 = (bool *)calloc(set_size, sizeof(bool));
    if (matched_in_set2 == NULL) {
        // Failed to allocate memory, cannot perform comparison reliably.
        // Return false as a safe error indicator.
        return false;
    }

    bool all_cards_matched = true;
    // For each card in set1
    for (unsigned char i = 0; i < set_size; ++i) {
        bool current_card_found_match = false;
        // Try to find a unique, unmatched card in set2
        for (unsigned char j = 0; j < set_size; ++j) {
            if (!matched_in_set2[j] && is_equal(set1[i], set2[j])) {
                matched_in_set2[j] = true; // Mark set2[j] as used
                current_card_found_match = true;
                break; // Move to the next card in set1
            }
        }
        if (!current_card_found_match) {
            all_cards_matched = false;
            break; // A card in set1 did not find a unique match in set2
        }
    }

    free(matched_in_set2); // Clean up allocated memory
    return all_cards_matched;
}