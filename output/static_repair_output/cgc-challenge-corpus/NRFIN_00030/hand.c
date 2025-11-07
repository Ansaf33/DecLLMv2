#include <stdlib.h> // For calloc, NULL
#include <stddef.h> // For size_t

// --- Type Definitions (to match original snippet's inferred types) ---
typedef unsigned char byte;
typedef unsigned int uint;
typedef int undefined4; // Assuming 4-byte integer
// 'undefined' is used for rank parameter in `qty_of_rank_in_hand` and `is_rank`.
// It is likely an unsigned char representing the rank.
typedef unsigned char undefined;

// --- Constants ---
// Error codes (derived from 0xffffffXX values in original snippet)
#define HAND_ERR_NULL_POINTER       -23 // 0xffffffe9
#define HAND_ERR_INVALID_CARD       -43 // 0xffffffd5
#define HAND_ERR_HAND_FULL          -22 // 0xffffffea
#define HAND_ERR_INVALID_INDEX      -42 // 0xffffffd6
#define HAND_ERR_HAND_EMPTY         -24 // 0xffffffe8
#define HAND_ERR_INVALID_CARDS_ARRAY -43 // Reused for param_2 == 0 in add_cards_to_hand or null output ptr

// Maximum number of cards in a hand (0x34 = 52)
#define MAX_CARDS 52
// Total size of hand structure in bytes (0xd4 = 212).
// Assuming 1 byte for count + MAX_CARDS * sizeof(int) = 1 + 52 * 4 = 209 bytes.
// 212 bytes might include some padding or additional metadata.
#define HAND_STRUCT_SIZE 212

// --- External Function Declarations (placeholders) ---
// These functions are called in the provided snippet but not defined.
// Assuming prng_get_next returns a byte (unsigned char) for random number generation.
byte prng_get_next(void);
// Assuming is_rank takes an int card value and an unsigned char rank,
// and returns 1 if the card's rank matches, 0 otherwise.
int is_rank(int card_value, unsigned char rank);


// --- Function: create_hand ---
// Allocates memory for a new hand structure.
// Returns a pointer to the newly created hand, or NULL if allocation fails.
void *create_hand(void) {
  // Allocate HAND_STRUCT_SIZE bytes and initialize to zero.
  return calloc(HAND_STRUCT_SIZE, 1);
}

// --- Function: consolidate_hand ---
// Consolidates the cards in the hand pointed to by hand_data.
// It removes any zero (empty) card slots by shifting valid cards to the left.
// It updates the card count stored in hand_data[0].
// Returns 0 on success.
int consolidate_hand(byte *hand_data) {
  unsigned char write_idx = 0; // Index for where the next non-zero card should be placed
  unsigned char read_idx = 0;  // Index for iterating through all possible card slots

  // Iterate through all possible card slots up to MAX_CARDS.
  while (read_idx < MAX_CARDS) {
    // Check if the card at the current read_idx is not zero (i.e., it's a valid card).
    if (*(int *)(hand_data + 4 + (uint)read_idx * 4) != 0) {
      // If the card is not already in its consolidated position (i.e., read_idx != write_idx),
      // then move it to the write_idx position.
      if (read_idx != write_idx) {
        *(int *)(hand_data + 4 + (uint)write_idx * 4) = *(int *)(hand_data + 4 + (uint)read_idx * 4);
        // Clear the original slot where the card was read from.
        *(int *)(hand_data + 4 + (uint)read_idx * 4) = 0;
      }
      // Advance the write_idx, as we've placed a valid card.
      write_idx++;
    }
    // Always advance the read_idx to check the next slot.
    read_idx++;
  }

  // After consolidation, update the hand's card count to the new number of valid cards.
  hand_data[0] = write_idx;

  return 0; // Indicate success.
}

// --- Function: add_card_to_hand ---
// Adds a single card to the hand.
// Returns 0 on success, or a negative error code.
int add_card_to_hand(byte *hand_data, int card_value) {
  if (hand_data == NULL) {
    return HAND_ERR_NULL_POINTER;
  }
  if (card_value == 0) { // Card value 0 is considered invalid/empty.
    return HAND_ERR_INVALID_CARD;
  }
  if (hand_data[0] < MAX_CARDS) { // Check if there's space in the hand.
    // Add card to the next available slot.
    *(int *)(hand_data + 4 + (uint)hand_data[0] * 4) = card_value;
    hand_data[0]++; // Increment card count.
    return 0;       // Success.
  }
  return HAND_ERR_HAND_FULL;
}

// --- Function: add_cards_to_hand ---
// Adds multiple cards from an array to the hand.
// Returns 0 on success, or a negative error code.
int add_cards_to_hand(byte *hand_data, int *cards_to_add, byte num_cards_to_add) {
  if (hand_data == NULL) {
    return HAND_ERR_NULL_POINTER;
  }
  if (cards_to_add == NULL) { // Check if the source array of cards is valid.
    return HAND_ERR_INVALID_CARDS_ARRAY;
  }
  // Check if adding cards would exceed hand capacity.
  if ((uint)hand_data[0] + (uint)num_cards_to_add > MAX_CARDS) {
    return HAND_ERR_HAND_FULL;
  }

  // First, validate all cards to be added.
  for (unsigned char i = 0; i < num_cards_to_add; i++) {
    if (cards_to_add[i] == 0) {
      return HAND_ERR_INVALID_CARD; // Return error if any card is invalid.
    }
  }

  // Add cards one by one.
  for (unsigned char i = 0; i < num_cards_to_add; i++) {
    int result = add_card_to_hand(hand_data, cards_to_add[i]);
    if (result != 0) {
      // If add_card_to_hand fails for any reason (e.g., hand becomes full unexpectedly),
      // return the error.
      return result;
    }
    cards_to_add[i] = 0; // Clear the card from the source array after adding.
  }
  return 0; // Success.
}

// --- Function: rm_card_from_hand ---
// Removes a card from the hand at a specified index.
// The removed card's value is stored in `removed_card_ptr` if provided.
// Returns 0 on success, or a negative error code.
int rm_card_from_hand(byte *hand_data, byte index_to_remove, int *removed_card_ptr) {
  if (hand_data == NULL) {
    return HAND_ERR_NULL_POINTER;
  }
  // Check if index is within bounds of current cards in hand.
  if (index_to_remove >= hand_data[0]) {
    return HAND_ERR_INVALID_INDEX;
  }

  int card_value = *(int *)(hand_data + 4 + (uint)index_to_remove * 4);
  if (card_value == 0) { // Card at index is already empty.
    return HAND_ERR_INVALID_CARD;
  }

  // Store the removed card's value if a pointer is provided.
  if (removed_card_ptr != NULL) {
    *removed_card_ptr = card_value;
  }
  
  // Mark the slot as empty.
  *(int *)(hand_data + 4 + (uint)index_to_remove * 4) = 0;
  hand_data[0]--; // Decrement card count.
  
  // Consolidate the hand to fill the gap created by removal.
  consolidate_hand(hand_data);
  return 0; // Success.
}

// --- Function: qty_of_rank_in_hand ---
// Counts the number of cards of a specific rank in the hand.
// Returns the count, or a negative error code.
int qty_of_rank_in_hand(byte *hand_data, undefined rank) {
  if (hand_data == NULL) {
    return HAND_ERR_NULL_POINTER;
  }

  int count = 0;
  // Iterate through currently held cards.
  for (unsigned char i = 0; i < hand_data[0]; i++) {
    int card_value = *(int *)(hand_data + 4 + (uint)i * 4);
    if (is_rank(card_value, rank) == 1) {
      count++;
    }
  }
  return count;
}

// --- Function: get_all_of_rank_from_hand ---
// Extracts all cards of a specific rank from the hand and stores them in `removed_cards_array`.
// Returns the number of cards removed, or a negative error code.
int get_all_of_rank_from_hand(byte *hand_data, unsigned char rank, int *removed_cards_array) {
  if (hand_data == NULL) {
    return HAND_ERR_NULL_POINTER;
  }
  if (removed_cards_array == NULL) {
    return HAND_ERR_INVALID_CARDS_ARRAY; // Using this error for a null output array pointer.
  }
  if (rank == 0) { // Rank 0 might be considered an invalid or "no rank" value.
    return 0; // No cards to remove for rank 0, or invalid rank.
  }

  unsigned char removed_count = 0;
  // Iterate through the hand. When a card is removed, the hand is consolidated (cards shift left).
  // Thus, we do NOT increment `i` when a card is removed, as the next card
  // will have shifted into the current `i` position.
  unsigned char i = 0;
  while (i < hand_data[0]) { // Loop while 'i' is less than the current card count.
    int card_value = *(int *)(hand_data + 4 + (uint)i * 4);
    if (is_rank(card_value, rank) == 1) {
      // If card matches rank, remove it. rm_card_from_hand decrements hand_data[0] and consolidates.
      int rm_result = rm_card_from_hand(hand_data, i, &removed_cards_array[removed_count]);
      if (rm_result < 0) {
        return rm_result; // Return error if removal fails.
      }
      removed_count++;
      // Do NOT increment 'i' here, as the next card has shifted into position 'i'.
    } else {
      // If card does not match, move to the next card.
      i++;
    }
  }
  return removed_count; // Return the total number of cards removed.
}

// --- Function: get_count_cards_in_hand ---
// Returns the number of cards currently in the hand.
// Returns the count, or a negative error code.
int get_count_cards_in_hand(byte *hand_data) {
  if (hand_data == NULL) {
    return HAND_ERR_NULL_POINTER;
  }
  return hand_data[0]; // The first byte stores the card count.
}

// --- Function: get_rank_of_random_card_in_hand ---
// Retrieves the rank of a randomly selected card from the hand.
// Returns the rank (0-255), or a negative error code.
int get_rank_of_random_card_in_hand(byte *hand_data) {
  if (hand_data == NULL) {
    return HAND_ERR_NULL_POINTER;
  }

  int card_count = get_count_cards_in_hand(hand_data);
  if (card_count < 0) { // Handle error from get_count_cards_in_hand.
    return card_count;
  }
  if (card_count == 0) {
    return HAND_ERR_HAND_EMPTY;
  }

  unsigned char random_index = 0;
  if (card_count > 1) {
    // Generate a random index within the bounds of current cards.
    random_index = prng_get_next() % card_count;
  }

  // Retrieve the card value at the random index.
  int card_value = *(int *)(hand_data + 4 + (uint)random_index * 4);
  
  // Assuming the rank is encoded as the second byte (offset 1) within the 4-byte integer card value.
  // This is a common way to pack rank/suit into an int.
  // The original decompiled code had a suspicious dereference:
  // `*(byte *)(*(int *)(param_1 + 4 + (uint)local_d * 4) + 1)`
  // which suggests reading a byte at address `(card_value + 1)`. If `card_value` is an `int`,
  // `card_value + 1` would be `card_value + sizeof(int)`. This is usually incorrect for rank extraction.
  // A more plausible interpretation is bit shifting to get the second byte.
  return (unsigned char)((card_value >> 8) & 0xFF); 
}

// --- Function: is_hand_empty ---
// Checks if the hand is empty.
// Returns 1 if empty, 0 if not empty, or a negative error code.
int is_hand_empty(byte *hand_data) {
  if (hand_data == NULL) {
    return HAND_ERR_NULL_POINTER;
  }
  int card_count = get_count_cards_in_hand(hand_data);
  if (card_count < 0) { // Handle error from get_count_cards_in_hand.
    return card_count;
  }
  return (card_count == 0) ? 1 : 0;
}

// --- Function: get_latest_card ---
// Retrieves the value of the last card added to the hand.
// Returns 0 on success, or a negative error code.
int get_latest_card(byte *hand_data, int *latest_card_ptr) {
  if (hand_data == NULL) {
    return HAND_ERR_NULL_POINTER;
  }
  if (latest_card_ptr == NULL) {
      return HAND_ERR_INVALID_CARDS_ARRAY; // Using this error for a null output pointer.
  }

  int is_empty_result = is_hand_empty(hand_data);
  if (is_empty_result < 0) { // Handle error from is_hand_empty.
    return is_empty_result;
  }
  if (is_empty_result == 1) {
    return HAND_ERR_HAND_EMPTY;
  }

  // The "latest" card is typically the one at the highest index (count - 1).
  *latest_card_ptr = *(int *)(hand_data + 4 + (uint)(hand_data[0] - 1) * 4);
  return 0; // Success.
}