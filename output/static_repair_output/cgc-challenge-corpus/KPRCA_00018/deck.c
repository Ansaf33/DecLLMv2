#include <stdio.h>   // For printf in main
#include <stdlib.h>  // For NULL
#include <stdint.h>  // For standard integer types like uint32_t, though we'll use a typedef for 'uint'
#include <time.h>    // For time(NULL) for RNG seeding

// Define 'uint' as unsigned int, matching typical 4-byte unsigned integer behavior
typedef unsigned int uint;

// Global state for the RNG, initialized with arbitrary values
static uint a = 0x12345678;
static uint b = 0x90ABCDEF;
static uint c = 0xFEDCBA90;
static uint d = 0x87654321;

// Card structure: 2 integers, total 8 bytes
typedef struct {
    int rank; // e.g., 1-13 (Ace-King)
    int suit; // e.g., 0-3 (Clubs, Diamonds, Hearts, Spades)
} Card;

// Deck structure, inferring layout from memory offsets used in original code
// 52 cards * sizeof(Card) = 52 * 8 = 416 bytes
// The current_card_idx is accessed at an offset of 0x1a0 (672 decimal) from the base.
// This implies padding between the card array and the index variable.
typedef struct {
    Card cards[52];
    char padding[672 - (52 * sizeof(Card))]; // 672 - 416 = 256 bytes of padding
    int current_card_idx;
} Deck;

// Function: rng
// Generates a pseudo-random unsigned integer using a custom algorithm.
uint rng(void) {
  uint uVar1 = a ^ (a << 7);
  a = b;
  b = c;
  c = d;
  d = (uVar1 >> 0x10) ^ uVar1 ^ (d >> 0x16) ^ d;
  return d;
}

// Function: init_deck
// Initializes a deck of 52 cards and resets the draw index.
// Allows for initial seeding of the RNG if a non-NULL seed_ptr is provided.
void init_deck(Deck *deck_ptr, unsigned int *seed_ptr) {
  if (seed_ptr != NULL) {
    a = *seed_ptr;
  } else {
    // If no seed provided, use time to initialize RNG state
    a = (uint)time(NULL);
    b = a + 1;
    c = a + 2;
    d = a + 3;
  }

  // Populate the deck with standard cards (4 suits, 13 ranks each)
  for (int suit_idx = 0; suit_idx < 4; ++suit_idx) {
    for (int rank_idx = 0; rank_idx < 13; ++rank_idx) {
      deck_ptr->cards[suit_idx * 13 + rank_idx].rank = rank_idx + 1;
      deck_ptr->cards[suit_idx * 13 + rank_idx].suit = suit_idx;
    }
  }
  deck_ptr->current_card_idx = 0; // Reset card draw index
}

// Function: shuffle_deck
// Shuffles the deck by performing 10,000 random card swaps.
// Resets the card draw index after shuffling.
unsigned int shuffle_deck(Deck *deck_ptr) {
  for (uint i = 0; i < 10000; ++i) {
    uint idx1 = rng() % 52; // Get a random index for the first card
    uint idx2 = rng() % 52; // Get a random index for the second card

    // Swap the two cards
    Card temp_card = deck_ptr->cards[idx1];
    deck_ptr->cards[idx1] = deck_ptr->cards[idx2];
    deck_ptr->cards[idx2] = temp_card;
  }
  deck_ptr->current_card_idx = 0; // Reset card draw index after shuffling
  return 1; // Indicates success
}

// Function: shuffle_deck_if_needed
// Shuffles the deck if the current card draw index is 34 or higher.
unsigned int shuffle_deck_if_needed(Deck *deck_ptr) {
  if (deck_ptr->current_card_idx < 34) {
    return 0; // No shuffle needed
  } else {
    // Perform shuffle and return its result.
    // The original logic would return 0xFFFFFFFF if shuffle_deck returned 0,
    // but shuffle_deck currently always returns 1.
    int shuffle_result = shuffle_deck(deck_ptr);
    return (shuffle_result == 0) ? 0xFFFFFFFF : 1;
  }
}

// Function: draw_card
// Draws the next card from the deck and increments the draw index.
// Returns a pointer to the drawn Card.
Card* draw_card(Deck *deck_ptr) {
  // Return a pointer to the card at the current index, then increment the index.
  return &(deck_ptr->cards[deck_ptr->current_card_idx++]);
}

// Main function to demonstrate the usage of the deck and RNG functions.
int main() {
    Deck my_deck;
    unsigned int seed = (unsigned int)time(NULL); // Use current time for a different seed each run

    printf("Initializing deck...\n");
    init_deck(&my_deck, &seed);

    printf("Deck initialized. First card: Rank %d, Suit %d\n",
           my_deck.cards[0].rank, my_deck.cards[0].suit);
    printf("Current card index: %d\n", my_deck.current_card_idx);

    printf("\nShuffling deck if needed (current_card_idx < 34, so shouldn't shuffle yet)...\n");
    unsigned int shuffle_needed_result = shuffle_deck_if_needed(&my_deck);
    printf("shuffle_deck_if_needed returned: %u\n", shuffle_needed_result);
    printf("Current card index after check: %d\n", my_deck.current_card_idx);

    printf("\nDrawing 34 cards to trigger a shuffle on the next check...\n");
    for (int i = 0; i < 34; ++i) {
        Card* drawn = draw_card(&my_deck);
        // Uncomment below to print each drawn card
        // printf("Drawn card %d: Rank %d, Suit %d\n", i + 1, drawn->rank, drawn->suit);
    }
    printf("Current card index after drawing 34 cards: %d\n", my_deck.current_card_idx);
    printf("First card after drawing 34 (should be original first card): Rank %d, Suit %d\n",
           my_deck.cards[0].rank, my_deck.cards[0].suit);

    printf("\nShuffling deck if needed (current_card_idx >= 34, should shuffle)...\n");
    shuffle_needed_result = shuffle_deck_if_needed(&my_deck);
    printf("shuffle_deck_if_needed returned: %u\n", shuffle_needed_result);
    printf("Current card index after shuffle: %d\n", my_deck.current_card_idx); // Should be 0
    printf("First card after shuffle (should be different): Rank %d, Suit %d\n",
           my_deck.cards[0].rank, my_deck.cards[0].suit);

    printf("\nDrawing a card after shuffle:\n");
    Card* drawn_after_shuffle = draw_card(&my_deck);
    printf("Drawn card: Rank %d, Suit %d\n", drawn_after_shuffle->rank, drawn_after_shuffle->suit);
    printf("Current card index: %d\n", my_deck.current_card_idx);

    return 0;
}