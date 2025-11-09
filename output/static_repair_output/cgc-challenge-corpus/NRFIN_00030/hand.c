#include <stdlib.h> // For calloc
#include <stdint.h> // For uint32_t
#include <stddef.h> // For size_t

// Custom types from decompilation
typedef unsigned char byte;
typedef unsigned char undefined; // Used for single byte values, like rank
typedef uint32_t undefined4; // Used for 4-byte values, like card IDs or return codes

// Constants
#define MAX_HAND_SIZE 52
#define HAND_STRUCT_SIZE (sizeof(byte) + 3 * sizeof(byte) + MAX_HAND_SIZE * sizeof(int)) // 1 (count) + 3 (padding) + 52 * 4 (cards) = 212 bytes

// Error codes (derived from original negative return values)
#define ERR_NULL_HAND_PTR       -23 // 0xffffffe9
#define ERR_INVALID_CARD_VALUE  -43 // 0xffffffd5 (e.g., card value 0 or NULL output buffer)
#define ERR_HAND_FULL           -22 // 0xffffffea (specific to add_card_to_hand)
#define ERR_HAND_FULL_BATCH     -52 // 0xffffffcc (specific to add_cards_to_hand for multiple cards)
#define ERR_HAND_EMPTY          -24 // 0xffffffe8
#define ERR_INDEX_OUT_OF_BOUNDS -26 // 0xffffffd6

// Dummy declarations for external functions (replace with actual implementations if available)
int is_rank(undefined4 card_value, undefined rank_value);
byte prng_get_next(void);

// Function: create_hand
void *create_hand(void) {
  // Original: pvVar1 = calloc(0xd4,in_stack_ffffffd8);
  // 0xd4 is 212 decimal. `in_stack_ffffffd8` is an uninitialized decompiler artifact.
  // This likely means `calloc(1, 212)` to allocate a single hand structure of 212 bytes.
  return calloc(1, HAND_STRUCT_SIZE);
}

// Function: consolidate_hand
// The original return type 'undefined **' and variable 'ppuVar1' are decompiler artifacts.
// This function modifies the hand in place to remove empty slots.
void consolidate_hand(byte *hand_ptr) {
  int *cards = (int *)(hand_ptr + 4); // Pointer to the array of card integers
  byte read_idx = 0;  // Index for reading cards
  byte write_idx = 0; // Index for writing consolidated cards

  // Iterate through all possible card slots up to MAX_HAND_SIZE
  while (read_idx < MAX_HAND_SIZE) {
    if (cards[read_idx] == 0) {
      // If the card at read_idx is empty, just advance the read pointer.
      read_idx++;
    } else if (cards[write_idx] != 0) {
      // If the card at read_idx is valid, AND the slot at write_idx is also valid,
      // then both pointers advance without moving a card.
      read_idx++;
      write_idx++;
    } else {
      // If the card at read_idx is valid, BUT the slot at write_idx is empty,
      // move the card from read_idx to write_idx.
      cards[write_idx] = cards[read_idx];
      cards[read_idx] = 0; // Clear the source slot
      read_idx++;
      write_idx++;
    }
  }

  // After consolidation, 'write_idx' holds the new count of valid cards.
  // Zero out any remaining slots from 'write_idx' to 'MAX_HAND_SIZE - 1'.
  for (byte i = write_idx; i < MAX_HAND_SIZE; ++i) {
    cards[i] = 0;
  }

  // Update the hand's card count.
  *hand_ptr = write_idx;
}

// Function: add_card_to_hand
undefined4 add_card_to_hand(byte *hand_ptr, int card_value) {
  if (hand_ptr == NULL) {
    return ERR_NULL_HAND_PTR;
  }
  if (card_value == 0) { // Card value 0 is considered invalid/empty
    return ERR_INVALID_CARD_VALUE;
  }
  // Check if there is space in the hand
  if (*hand_ptr < MAX_HAND_SIZE) {
    int *cards = (int *)(hand_ptr + 4); // Pointer to the card array
    cards[*hand_ptr] = card_value;      // Add card to the next available slot
    *hand_ptr = *hand_ptr + 1;          // Increment card count
    return 0;                           // Success
  }
  return ERR_HAND_FULL; // Hand is full
}

// Function: add_cards_to_hand
int add_cards_to_hand(byte *hand_ptr, int *card_values, byte num_cards_to_add) {
  if (hand_ptr == NULL) {
    return ERR_NULL_HAND_PTR;
  }
  // Check if there's enough space in the hand for all new cards
  if ((uint)*hand_ptr + (uint)num_cards_to_add > MAX_HAND_SIZE) {
    return ERR_HAND_FULL_BATCH;
  }
  if (card_values == NULL) {
    return ERR_INVALID_CARD_VALUE; // Source array of cards is NULL
  }

  // Verify all cards in the batch are valid (non-zero) before adding any
  for (byte i = 0; i < num_cards_to_add; ++i) {
    if (card_values[i] == 0) {
      return ERR_INVALID_CARD_VALUE;
    }
  }

  // Add cards one by one
  for (byte i = 0; i < num_cards_to_add; ++i) {
    int result = add_card_to_hand(hand_ptr, card_values[i]);
    if (result != 0) {
      return result; // Return error if any card fails to add
    }
    card_values[i] = 0; // Clear the card from the source array after adding
  }
  return 0; // Success
}

// Function: rm_card_from_hand
undefined4 rm_card_from_hand(byte *hand_ptr, byte card_idx, int *removed_card_value_out) {
  if (hand_ptr == NULL) {
    return ERR_NULL_HAND_PTR;
  }
  if (removed_card_value_out == NULL) { // Added check for output pointer
    return ERR_INVALID_CARD_VALUE;
  }
  // Check if the card_idx is within the bounds of currently active cards
  if (card_idx >= *hand_ptr) {
    return ERR_INDEX_OUT_OF_BOUNDS;
  }

  int *cards = (int *)(hand_ptr + 4);
  *removed_card_value_out = cards[card_idx]; // Store the removed card's value

  if (*removed_card_value_out == 0) { // Should not happen if hand is consolidated and index is valid
    return ERR_INVALID_CARD_VALUE;
  }

  cards[card_idx] = 0;       // Mark the slot as empty
  *hand_ptr = *hand_ptr - 1; // Decrement card count

  consolidate_hand(hand_ptr); // Consolidate the hand to remove the empty slot

  return 0; // Success
}

// Function: qty_of_rank_in_hand
int qty_of_rank_in_hand(byte *hand_ptr, undefined rank_value) {
  if (hand_ptr == NULL) {
    return ERR_NULL_HAND_PTR;
  }

  int count = 0;
  int *cards = (int *)(hand_ptr + 4);

  // Iterate through currently active cards (up to *hand_ptr)
  for (byte i = 0; i < *hand_ptr; ++i) {
    if (is_rank(cards[i], rank_value) == 1) {
      count++;
    }
  }
  return count;
}

// Function: get_all_of_rank_from_hand
uint get_all_of_rank_from_hand(byte *hand_ptr, char rank_to_remove, int *removed_cards_array) {
  if (hand_ptr == NULL) {
    return ERR_NULL_HAND_PTR;
  }
  if (removed_cards_array == NULL) { // Output array for removed cards is NULL
    return ERR_INVALID_CARD_VALUE;
  }
  if (rank_to_remove == '\0') { // If rank is 0, nothing to remove
    return 0;
  }

  uint num_removed_cards = 0;
  uint initial_card_count = *hand_ptr; // Store initial count for loop iterations

  int *cards = (int *)(hand_ptr + 4);
  byte current_card_slot_idx = 0; // This index tracks the current position in the hand.
                                  // It is not incremented if a card is removed, as consolidation
                                  // will shift the next card into this position.

  for (uint i = 0; i < initial_card_count; ++i) { // Loop 'initial_card_count' times
    if (is_rank(cards[current_card_slot_idx], rank_to_remove) == 1) {
      // If card matches rank, remove it and store its value
      undefined4 result = rm_card_from_hand(hand_ptr, current_card_slot_idx, &removed_cards_array[num_removed_cards]);
      if ((int)result < 0) {
        return result; // Return error if removal fails
      }
      num_removed_cards++;
      // Do NOT increment current_card_slot_idx here; consolidate_hand has shifted
      // the next card into this position, which needs to be checked.
    } else {
      // If card does not match, move to the next slot
      current_card_slot_idx++;
    }
  }

  return num_removed_cards; // Return count of cards removed
}

// Function: get_count_cards_in_hand
uint get_count_cards_in_hand(byte *hand_ptr) {
  if (hand_ptr == NULL) {
    return ERR_NULL_HAND_PTR;
  }
  return (uint)*hand_ptr;
}

// Function: get_rank_of_random_card_in_hand
uint get_rank_of_random_card_in_hand(byte *hand_ptr) {
  if (hand_ptr == NULL) {
    return ERR_NULL_HAND_PTR;
  }

  uint card_count = get_count_cards_in_hand(hand_ptr);
  if (card_count == 0) {
    return ERR_HAND_EMPTY;
  }

  byte random_idx = 0;
  if (card_count > 1) { // Only generate random number if more than one card
    random_idx = prng_get_next() % card_count;
  }

  // Cards are stored as `int`. Assuming rank is the second byte of the 4-byte card value.
  int *cards = (int *)(hand_ptr + 4);
  return (uint)*((byte *)&cards[random_idx] + 1); // Extract the second byte
}

// Function: is_hand_empty
undefined4 is_hand_empty(byte *hand_ptr) {
  if (hand_ptr == NULL) {
    return ERR_NULL_HAND_PTR;
  }
  return (get_count_cards_in_hand(hand_ptr) == 0) ? 1 : 0;
}

// Function: get_latest_card
undefined4 get_latest_card(byte *hand_ptr, undefined4 *latest_card_value_out) {
  if (hand_ptr == NULL) {
    return ERR_NULL_HAND_PTR;
  }
  if (latest_card_value_out == NULL) { // Added check for output pointer
    return ERR_INVALID_CARD_VALUE;
  }

  if (is_hand_empty(hand_ptr) == 1) {
    return ERR_HAND_EMPTY;
  }

  // The latest card is at index (*hand_ptr - 1)
  int *cards = (int *)(hand_ptr + 4);
  *latest_card_value_out = cards[*hand_ptr - 1];

  return 0; // Success
}