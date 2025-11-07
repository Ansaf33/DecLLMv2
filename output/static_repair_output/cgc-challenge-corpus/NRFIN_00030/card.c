#include <stdlib.h>  // For calloc, free, NULL
#include <stdint.h>  // For uint8_t
#include <stdbool.h> // For bool

// Define card structure
typedef struct {
    uint8_t suit;
    uint8_t rank;
} Card_t;

// Function prototypes to ensure correct compilation order
bool is_valid_suit(uint8_t suit);
bool is_valid_rank(uint8_t rank);
bool is_equal(const Card_t *card1, const Card_t *card2);

Card_t *create_card(uint8_t suit, uint8_t rank) {
    if (is_valid_suit(suit) && is_valid_rank(rank)) {
        Card_t *new_card = (Card_t *)calloc(1, sizeof(Card_t));
        if (new_card != NULL) {
            new_card->suit = suit;
            new_card->rank = rank;
        }
        return new_card;
    }
    return NULL;
}

// Renamed from is_rank for clarity, as it checks if a specific card has a given rank
bool card_has_rank(const Card_t *card, uint8_t rank_val) {
    // The original code returned 0xffffffd3 for NULL card, which is -45.
    // Assuming 'false' is the intended boolean equivalent for a NULL card in a rank check.
    if (card == NULL) {
        return false;
    }
    return card->rank == rank_val;
}

void destroy_card(void *card_ptr) {
    free(card_ptr);
}

bool is_valid_suit(uint8_t suit) {
    return (suit >= 1 && suit <= 4);
}

bool is_valid_rank(uint8_t rank) {
    return (rank >= 1 && rank <= 13);
}

bool is_equal(const Card_t *card1, const Card_t *card2) {
    if (card1 == NULL && card2 == NULL) {
        return true;
    }
    if (card1 == NULL || card2 == NULL) {
        return false;
    }
    return (card1->suit == card2->suit) && (card1->rank == card2->rank);
}

bool is_set_equal(Card_t *const *set1, Card_t *const *set2, size_t count) {
    if (count == 0) {
        // Two empty sets are considered equal.
        // The original code called is_equal(*param_1, *param_2) if count was 0,
        // which would be problematic if param_1 or param_2 were NULL.
        return true;
    }

    // If count > 0, sets must not be NULL
    if (set1 == NULL || set2 == NULL) {
        return false;
    }

    size_t total_matches = 0;
    for (size_t i = 0; i < count; ++i) {
        size_t current_card_matches = 0;
        for (size_t j = 0; j < count; ++j) {
            if (is_equal(set1[i], set2[j])) {
                current_card_matches++;
                total_matches++;
            }
        }
        // If a card from set1 matches more than one card in set2, sets are not equal.
        if (current_card_matches > 1) {
            return false;
        }
    }
    // Sets are considered equal if the total number of matches equals the count
    // (given the constraint that each card in set1 matches at most one in set2).
    return total_matches == count;
}