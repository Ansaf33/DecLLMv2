#include <stdlib.h> // For calloc, rand
#include <stdint.h> // For uint32_t

// Define custom types to match the decompiled output's likely intent
typedef unsigned char byte;
typedef uint32_t undefined4;

// --- Helper/Stub Functions (not part of the original snippet, but needed for compilation) ---

// Function: create_card
// Assumed to encode suit and rank into a single 4-byte value (uint32_t)
static inline undefined4 create_card(byte suit, byte rank) {
    // Simple encoding: (suit << 8) | rank
    // Assuming suit is 1-4 and rank is 1-13
    return (undefined4)((suit << 8) | rank);
}

// Function: prng_get_next
// Assumed to return a pseudo-random integer
static inline int prng_get_next(void) {
    return rand(); // Using rand() from stdlib.h
}

// --- Fixed Functions from the Snippet ---

// Function: create_deck
char * create_deck(void) {
  char *deck; // Renamed pcVar1 to deck
  
  // Allocate memory for the deck.
  // The original calloc(0xd4, in_stack_ffffffd8) with uninitialized size is a bug.
  // 0xd4 (212 decimal) is likely the total size in bytes.
  // Structure: 1 byte for card count (deck[0]), followed by 52 uint32_t card values.
  // This implies 3 bytes of padding after the count before cards start,
  // to align card values on a 4-byte boundary, as indicated by `+ 4`.
  deck = (char *)calloc(1, 0xd4); // Allocate 212 bytes
  if (deck == NULL) {
      return NULL;
  }

  // Card count is implicitly 0 due to calloc, but explicit assignment for clarity.
  // deck[0] = 0; 
  
  unsigned char card_idx = 0; // Renamed local_d
  // Outer loop for suits (1 to 4)
  for (unsigned char suit = 1; suit < 5; suit++) { // Renamed local_e
    // Inner loop for ranks (1 to 13)
    for (unsigned char rank = 1; rank < 14; rank++) { // Renamed local_f
      undefined4 card_value = create_card(suit, rank); // Renamed uVar2
      
      // Store the card value at the correct offset.
      // Offset 4 bytes for the card count and padding, then card_idx * sizeof(undefined4).
      *((undefined4 *)(deck + 4 + card_idx * sizeof(undefined4))) = card_value;
      
      deck[0]++; // Increment card count stored at the first byte
      card_idx++; // Increment the index for the next card
    }
  }
  return deck;
}

// Function: shuffle_deck
// param_1 is char* deck, param_2 is unsigned char seed_val
undefined4 shuffle_deck(char *deck, byte seed_val) { // Renamed param_1, param_2 and types
  if (deck == NULL) {
    return 0xffffffe0; // Error: NULL deck pointer
  }
  
  const int NUM_CARDS = 52; // 0x34 is 52 decimal

  unsigned char idx1 = seed_val % NUM_CARDS; // Renamed local_d
  unsigned char idx2 = 0; // Renamed local_e, initialized to 0

  for (int i = 0; i < NUM_CARDS; i++) { // Renamed local_14 to i
    // Original complex modulo expression simplified to (idx1 + i) % NUM_CARDS
    idx1 = (idx1 + i) % NUM_CARDS;

    int rand_val = prng_get_next(); // Renamed iVar2
    
    // Original complex modulo expression simplified to (idx2 + rand_val) % NUM_CARDS
    idx2 = (idx2 + rand_val) % NUM_CARDS;
    
    if (idx1 != idx2) {
      // Perform the card swap
      // Cards start at offset 4, each card is sizeof(undefined4) bytes
      undefined4 *card_ptr1 = (undefined4 *)(deck + 4 + idx1 * sizeof(undefined4));
      undefined4 *card_ptr2 = (undefined4 *)(deck + 4 + idx2 * sizeof(undefined4));
      
      undefined4 temp_card = *card_ptr1; // Renamed uVar1
      *card_ptr1 = *card_ptr2;
      *card_ptr2 = temp_card;
    }
  }
  return 0; // Success
}

// Function: get_shuffled_deck
// param_1 is unsigned char seed_val
char * get_shuffled_deck(byte seed_val) { // Return type changed to char*, param_1 type changed
  char *deck = create_deck(); // Renamed uVar1
  if (deck != NULL) { // Ensure deck was created successfully before shuffling
    shuffle_deck(deck, seed_val);
  }
  return deck;
}

// Function: pop
// param_1 is char* deck
undefined4 pop(char *deck) { // param_1 type changed to char*
  if (deck == NULL) {
    return 0; // Error: NULL deck pointer
  }
  else if (deck[0] == 0) { // Check if deck is empty (card count is 0)
    return 0; // Deck is empty, no card to pop
  }
  else {
    deck[0]--; // Decrement card count
    
    // Get the value of the card that was at the top (now at the new count index)
    undefined4 card_value = *((undefined4 *)(deck + 4 + deck[0] * sizeof(undefined4)));
    
    // Mark the popped card's slot as empty (set to 0)
    *((undefined4 *)(deck + 4 + deck[0] * sizeof(undefined4))) = 0;
    
    return card_value;
  }
}

// Function: is_deck_empty
undefined4 is_deck_empty(char *deck) { // param_1 type changed to char*
  if (deck == NULL) {
    return 0xffffffe0; // Error: NULL deck pointer
  }
  else if (deck[0] == 0) { // Check if card count is 0
    return 1; // Deck is empty
  }
  else {
    return 0; // Deck is not empty
  }
}