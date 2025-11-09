#include <stdbool.h> // For bool
#include <stdint.h>  // For uint8_t, uint32_t
#include <stdlib.h>  // For malloc, NULL

// Forward declarations for external functions and structs
// These functions are called in the snippet but not defined within it.
// Dummy implementations are provided for compilation.
struct Card;
struct CardList;

extern void discard_hand(struct CardList* hand_cards_ptr);
extern void shuffle_deck_if_needed(void* deck_ptr);
extern void init_deck(void* deck_ptr, uint32_t param_1);
extern void shuffle_deck(void* deck_ptr);
extern uint32_t draw_card(void* deck_ptr);
extern int get_card(struct CardList* card_list_ptr, uint32_t card_val);
extern int remove_card(struct CardList* card_list_ptr, uint32_t card_val);

// Basic Card structure based on usage in calc_score and dealer_has_duck
typedef struct Card {
    int value;
    struct Card* next;
} Card;

// Basic CardList structure
typedef struct CardList {
    Card* head;
    // Additional fields like count, tail, etc., could exist but are not strictly
    // necessary for the provided snippet's logic.
} CardList;

// Structure representing a "squarerabbit" hand or game state.
// This structure is deduced from memory access patterns (offsets) in the original code.
// It assumes a 32-bit environment where sizeof(void*) and sizeof(int) are 4 bytes,
// and that the struct is packed to ensure 24 bytes (0x18) total size.
#pragma pack(push, 1) // Ensure byte-level packing
typedef struct SrabbitHand {
    void* deck_ptr;         // Offset 0x00: Pointer to deck data (main deck for g_srabbit)
    CardList* dealer_cards; // Offset 0x04: Pointer to dealer's cards (only for g_srabbit)
    CardList* player_cards; // Offset 0x08: Pointer to player's cards for this hand
    int bet_amount;         // Offset 0x0c: Bet placed on this hand
    int insurance_amount;   // Offset 0x10: Insurance bet for this hand
    uint8_t player_finished_turn; // Offset 0x14: Flag, 1 if player finished turn for this hand
    uint8_t can_act;        // Offset 0x15: Flag, 1 if player can hit/double/split (initial turn)
    uint8_t busted;         // Offset 0x16: Flag, 1 if hand busted
    uint8_t split_count;    // Offset 0x17: For g_srabbit, tracks total split hands in game
} SrabbitHand;
#pragma pack(pop)

// Global variables deduced from the snippet
SrabbitHand* g_srabbit; // Pointer to the main game state/hand
#define MAX_SPLIT_HANDS 2 // Based on max_split()
SrabbitHand split_hand_array[MAX_SPLIT_HANDS]; // Array for split hands
SrabbitHand* split_hand = split_hand_array; // Pointer to the start of the split hands array

// --- Dummy Implementations for External Functions (for compilation only) ---
void discard_hand(struct CardList* hand_cards_ptr) {
    // Placeholder: In a real game, this would free card nodes.
    if (hand_cards_ptr) {
        Card* current = hand_cards_ptr->head;
        while (current) {
            Card* next = current->next;
            free(current);
            current = next;
        }
        hand_cards_ptr->head = NULL;
    }
}
void shuffle_deck_if_needed(void* deck_ptr) { /* Placeholder */ }
void init_deck(void* deck_ptr, uint32_t param_1) { /* Placeholder */ }
void shuffle_deck(void* deck_ptr) { /* Placeholder */ }
uint32_t draw_card(void* deck_ptr) { return 0; /* Placeholder: returns a card value */ }
int get_card(struct CardList* card_list_ptr, uint32_t card_val) {
    // Placeholder: Adds a card to the list. Returns 0 on success, -1 on error.
    if (card_list_ptr == NULL) {
        // Assume card_list_ptr needs to be allocated if it's NULL
        card_list_ptr = (CardList*)malloc(sizeof(CardList));
        if (card_list_ptr == NULL) return -1;
        card_list_ptr->head = NULL;
    }
    Card* new_card = (Card*)malloc(sizeof(Card));
    if (new_card == NULL) return -1;
    new_card->value = card_val;
    new_card->next = card_list_ptr->head;
    card_list_ptr->head = new_card;
    return 0;
}
int remove_card(struct CardList* card_list_ptr, uint32_t card_val) {
    // Placeholder: Removes a card from the list. Returns 0 on success, -1 on error.
    if (card_list_ptr == NULL || card_list_ptr->head == NULL) return -1;
    Card* current = card_list_ptr->head;
    Card* prev = NULL;
    while (current != NULL && current->value != card_val) {
        prev = current;
        current = current->next;
    }
    if (current == NULL) return -1; // Card not found

    if (prev == NULL) { // Card is head
        card_list_ptr->head = current->next;
    } else {
        prev->next = current->next;
    }
    free(current);
    return 0;
}
// --- End Dummy Implementations ---


// Function: clear_squarerabbit
void clear_squarerabbit(SrabbitHand* hand) {
  discard_hand(hand->dealer_cards);
  discard_hand(hand->player_cards);
  hand->bet_amount = 0;
  hand->insurance_amount = 0;
  hand->player_finished_turn = 0;
  hand->can_act = 1;
  hand->busted = 0;
  hand->split_count = 0; // This field is primarily for g_srabbit, but reset for all hands.
}

// Function: split_len
int split_len(void) {
  uint8_t count = g_srabbit->split_count;
  return (count > MAX_SPLIT_HANDS) ? MAX_SPLIT_HANDS : count;
}

// Function: clear_split_squarerabbit
void clear_split_squarerabbit(void) {
  for (int i = 0; i < split_len(); ++i) {
    split_hand[i].dealer_cards = NULL; // Split hands don't have their own dealer cards
    clear_squarerabbit(&split_hand[i]);
  }
}

// Function: new_srabbit_game
void new_srabbit_game(uint32_t bet_amount_param) {
  clear_split_squarerabbit();
  clear_squarerabbit(g_srabbit);
  g_srabbit->bet_amount = bet_amount_param;
  shuffle_deck_if_needed(g_srabbit->deck_ptr);
}

// Function: dealer_has_duck
bool dealer_has_duck(void) {
  // Checks if the dealer's first card is an Ace (value 1)
  return g_srabbit->dealer_cards != NULL &&
         g_srabbit->dealer_cards->head != NULL &&
         g_srabbit->dealer_cards->head->value == 1;
}

// Function: init_squarerabbit
void init_squarerabbit(uint32_t param_1) {
  g_srabbit = (SrabbitHand*)malloc(sizeof(SrabbitHand));
  if (g_srabbit == NULL) {
    // Handle allocation error
    return;
  }

  g_srabbit->deck_ptr = malloc(0x1a4); // Allocate memory for the deck data
  if (g_srabbit->deck_ptr == NULL) {
    free(g_srabbit);
    g_srabbit = NULL;
    // Handle allocation error
    return;
  }

  g_srabbit->dealer_cards = NULL; // Will be allocated by get_card
  g_srabbit->player_cards = NULL; // Will be allocated by get_card
  g_srabbit->bet_amount = 0;
  g_srabbit->insurance_amount = 0;

  g_srabbit->player_finished_turn = 0;
  g_srabbit->can_act = 1;
  g_srabbit->busted = 0;
  g_srabbit->split_count = 0;

  init_deck(g_srabbit->deck_ptr, param_1);
  shuffle_deck(g_srabbit->deck_ptr);
}

// Function: get_srabbit_game
SrabbitHand* get_srabbit_game(void) {
  return g_srabbit;
}

// Function: get_split_hand
SrabbitHand* get_split_hand(void) {
  return split_hand;
}

// Function: max_split
uint32_t max_split(void) {
  return MAX_SPLIT_HANDS;
}

// Function: all_hands_busted
uint32_t all_hands_busted(void) {
  if (!g_srabbit->busted) {
    return 0;
  }
  for (int i = 0; i < split_len(); ++i) {
    if (!split_hand[i].busted) {
      return 0;
    }
  }
  return 1;
}

// Function: is_player_finished
uint32_t is_player_finished(void) {
  if (!g_srabbit->player_finished_turn) {
    return 0;
  }
  for (int i = 0; i < split_len(); ++i) {
    if (!split_hand[i].player_finished_turn) {
      return 0;
    }
  }
  return 1;
}

// Function: calc_score
int calc_score(CardList* card_list, uint32_t* has_convertible_ace) {
  int score = 0;
  int ace_count = 0;

  if (card_list == NULL) {
      if (has_convertible_ace != NULL) *has_convertible_ace = 0;
      return 0;
  }

  for (Card* current_card = card_list->head; current_card != NULL; current_card = current_card->next) {
    int card_val = current_card->value;
    if (card_val == 1) { // Ace
      ace_count++;
      score += 1;
    } else if (card_val >= 10 && card_val <= 13) { // 10, Jack, Queen, King
      score += 10;
    } else { // 2 through 9 (and potentially 0 or >13 if those are valid card values)
      score += card_val;
    }
  }

  // Handle Ace as 11 if it improves score without busting (soft Ace)
  if ((score <= 11) && (ace_count > 0)) {
    score += 10; // Convert one Ace from 1 to 11
    if (has_convertible_ace != NULL) {
      *has_convertible_ace = 1; // Indicate that an Ace was converted
    }
  } else if (has_convertible_ace != NULL) {
    *has_convertible_ace = 0; // No Ace was converted
  }
  return score;
}

// Function: check_player_squarerabbit
uint32_t check_player_squarerabbit(SrabbitHand* hand) {
  int player_score = calc_score(hand->player_cards, NULL);
  // Original code checks if player has 21 AND the second card's value is 0.
  // The '0' value for a card is unusual for Blackjack and might signify a specific game rule
  // or a decompiler interpretation of a 10-value card. Adhering to the snippet.
  if (player_score == 21 &&
      hand->player_cards != NULL &&
      hand->player_cards->head != NULL &&
      hand->player_cards->head->next != NULL &&
      hand->player_cards->head->next->value == 0)
  {
    hand->player_finished_turn = 1;
    return 1;
  }
  return 0;
}

// Function: check_dealer_squarerabbit
uint32_t check_dealer_squarerabbit(void) {
  // Checks for a standard Blackjack: first card Ace, second card a 10-value card.
  if (g_srabbit->dealer_cards != NULL &&
      g_srabbit->dealer_cards->head != NULL &&
      g_srabbit->dealer_cards->head->next != NULL &&
      g_srabbit->dealer_cards->head->value == 1) // First card is Ace
  {
    int second_card_value = g_srabbit->dealer_cards->head->next->value;
    if (second_card_value >= 10 && second_card_value <= 13) { // Second card is 10, J, Q, K
      g_srabbit->player_finished_turn = 1; // Dealer BJ means player turn ends
      return 1;
    }
  }
  return 0;
}

// Function: check_dealer_squarerabbit2
uint32_t check_dealer_squarerabbit2(void) {
  int dealer_score = calc_score(g_srabbit->dealer_cards, NULL);
  // Similar to check_player_squarerabbit but for dealer, using the '0' card value rule.
  if (dealer_score == 21 &&
      g_srabbit->dealer_cards != NULL &&
      g_srabbit->dealer_cards->head != NULL &&
      g_srabbit->dealer_cards->head->next != NULL &&
      g_srabbit->dealer_cards->head->next->value == 0)
  {
    return 1;
  }
  return 0;
}

// Function: srabbit_payout
int srabbit_payout(void) {
  int dealer_score = calc_score(g_srabbit->dealer_cards, NULL);
  bool dealer_blackjack = (check_dealer_squarerabbit() == 1);
  int payout = 0;

  if (dealer_blackjack) {
    payout = (g_srabbit->insurance_amount * 2) - g_srabbit->bet_amount;
  } else {
    payout = -g_srabbit->insurance_amount;

    // Process main player hand
    int player_main_score = calc_score(g_srabbit->player_cards, NULL);
    bool player_main_blackjack = (check_player_squarerabbit(g_srabbit) == 1);

    if (player_main_blackjack) { // Player has Blackjack (and dealer does not)
      payout += (g_srabbit->bet_amount * 3) / 2;
    } else if (g_srabbit->busted) { // Player busted
      payout -= g_srabbit->bet_amount;
    } else if (dealer_score > 21) { // Dealer busted (player not busted)
      payout += g_srabbit->bet_amount;
    } else if (player_main_score > dealer_score) { // Player has higher score
      payout += g_srabbit->bet_amount;
    } else if (player_main_score < dealer_score) { // Dealer has higher score
      payout -= g_srabbit->bet_amount;
    }
    // Else (player_main_score == dealer_score), it's a push, payout remains unchanged for this bet.

    // Process split hands
    for (int i = 0; i < split_len(); ++i) {
      SrabbitHand* current_split_hand = &split_hand[i];
      int split_hand_score = calc_score(current_split_hand->player_cards, NULL);
      bool split_hand_blackjack = (check_player_squarerabbit(current_split_hand) == 1);

      if (split_hand_blackjack) { // Split hand has Blackjack (and dealer does not)
        payout += (current_split_hand->bet_amount * 3) / 2;
      } else if (current_split_hand->busted) { // Split hand busted
        payout -= current_split_hand->bet_amount;
      } else if (dealer_score > 21) { // Dealer busted (split hand not busted)
        payout += current_split_hand->bet_amount;
      } else if (split_hand_score > dealer_score) { // Split hand has higher score
        payout += current_split_hand->bet_amount;
      } else if (split_hand_score < dealer_score) { // Dealer has higher score
        payout -= current_split_hand->bet_amount;
      }
      // Else (split_hand_score == dealer_score), push, payout remains unchanged for this bet.
    }
  }
  return payout;
}

// Function: deal_srabbit
uint32_t deal_srabbit(uint32_t bet_amount_param) {
  new_srabbit_game(bet_amount_param);

  // Deal two cards to player, two to dealer (one face down for dealer)
  for (int i = 0; i < 2; ++i) {
    uint32_t card_val;
    
    card_val = draw_card(g_srabbit->deck_ptr);
    if (get_card(g_srabbit->player_cards, card_val) != 0) return 0xFFFFFFFF; // Error

    card_val = draw_card(g_srabbit->deck_ptr);
    if (get_card(g_srabbit->dealer_cards, card_val) != 0) return 0xFFFFFFFF; // Error
  }

  int player_score = calc_score(g_srabbit->player_cards, NULL);
  int dealer_score = calc_score(g_srabbit->dealer_cards, NULL);

  if (player_score == 21 && dealer_score == 21) {
    g_srabbit->player_finished_turn = 1; // Both have 21, push
    return 1;
  }

  if (dealer_has_duck()) { // Dealer's up-card is Ace, offer insurance
    return 2;
  }

  if (player_score == 21) { // Player has Blackjack (and dealer does not)
    g_srabbit->player_finished_turn = 1;
    return 0; // Player wins 3:2
  }
  
  if (dealer_score == 21) { // Dealer has Blackjack (and player doesn't have 21, or both had 21 already handled)
      g_srabbit->player_finished_turn = 1;
      return 0; // Dealer wins (player loses bet)
  }

  return 3; // Game continues, player can act
}

// Function: pay_insurance
void pay_insurance(void) {
  g_srabbit->insurance_amount = g_srabbit->bet_amount / 2;
}

// Helper function to find the current active hand (not finished)
SrabbitHand* get_current_active_hand(void) {
    if (!g_srabbit->player_finished_turn) {
        return g_srabbit;
    }
    for (int i = 0; i < split_len(); ++i) {
        if (!split_hand[i].player_finished_turn) {
            return &split_hand[i];
        }
    }
    return NULL; // All hands are finished
}

// Function: hit
int hit(void) {
  SrabbitHand* current_hand = get_current_active_hand();
  if (current_hand == NULL) {
    return -1; // No active hand
  }

  uint32_t card_val = draw_card(g_srabbit->deck_ptr); // All hands draw from the main deck
  if (get_card(current_hand->player_cards, card_val) != 0) {
    return -1; // Error drawing/getting card
  }

  int score = calc_score(current_hand->player_cards, NULL);
  if (score > 21) {
    current_hand->player_finished_turn = 1;
    current_hand->busted = 1;
  }
  current_hand->can_act = 0; // After hitting, can no longer double down/split

  return 0; // Success
}

// Function: stand
void stand(void) {
  SrabbitHand* current_hand = get_current_active_hand();
  if (current_hand == NULL) {
    return; // No active hand
  }
  current_hand->player_finished_turn = 1;
}

// Function: double_down
uint32_t double_down(void) {
  SrabbitHand* current_hand = get_current_active_hand();
  if (current_hand == NULL) {
    return 0xFFFFFFFF; // No active hand
  }

  if (!current_hand->can_act) { // Cannot double down if already hit/split
    return 0xFFFFFFFF;
  }

  uint32_t hit_result = hit(); // Hit once
  if (hit_result == 0) { // If hit was successful
    current_hand->player_finished_turn = 1; // Double down ends turn
    current_hand->bet_amount *= 2; // Double the bet
  }
  return hit_result; // Return hit result (0 for success, -1 for error)
}

// Function: can_split
uint32_t can_split(SrabbitHand* hand) {
  if (hand == NULL || hand->player_cards == NULL || hand->player_cards->head == NULL || hand->player_cards->head->next == NULL) {
    return 0; // Not enough cards to split
  }

  // Check if exactly two cards are present.
  if (hand->player_cards->head->next->next != NULL) {
      return 0; // More than two cards, cannot split
  }

  int card1_val = hand->player_cards->head->value;
  int card2_val = hand->player_cards->head->next->value;

  // Can split if:
  // 1. Both cards have the same rank (e.g., two 8s)
  // 2. Both cards are 10-value cards (e.g., King and Queen, or two 10s)
  if ((card1_val == card2_val) ||
      ((card1_val >= 10 && card1_val <= 13) && (card2_val >= 10 && card2_val <= 13))) {
    return 1;
  }
  return 0;
}

// Function: split
uint32_t split(void) {
  SrabbitHand* current_hand = get_current_active_hand();
  if (current_hand == NULL) {
    return 0xFFFFFFFF; // No active hand
  }

  if (!current_hand->can_act) { // Can only split on the first move
    return 0xFFFFFFFF;
  }

  if (can_split(current_hand) == 0) {
    return 0xFFFFFFFF; // Cannot split based on card values or count
  }

  if (g_srabbit->split_count >= MAX_SPLIT_HANDS) { // Max splits reached
    return 0xFFFFFFFF;
  }

  // Increment split count and get index for the new split hand
  g_srabbit->split_count++;
  int new_split_idx = g_srabbit->split_count - 1;
  SrabbitHand* new_split_hand = &split_hand[new_split_idx];

  // Initialize the new split hand (copy relevant fields from current_hand)
  new_split_hand->deck_ptr = g_srabbit->deck_ptr; // All hands share the main deck
  new_split_hand->dealer_cards = NULL; // Split hands don't have their own dealer cards
  new_split_hand->player_cards = NULL; // Will be initialized with one card
  new_split_hand->bet_amount = current_hand->bet_amount; // Same bet as original hand
  new_split_hand->insurance_amount = 0;
  new_split_hand->player_finished_turn = 0;
  new_split_hand->can_act = 1; // New split hand can act
  new_split_hand->busted = 0;
  new_split_hand->split_count = 0; // Split count is only tracked in g_srabbit

  // Move one card from current_hand to new_split_hand
  Card* card_to_move = current_hand->player_cards->head->next; // The second card
  if (card_to_move == NULL) return 0xFFFFFFFF; // Should not happen if can_split was true

  // Remove card from current_hand's player_cards
  if (remove_card(current_hand->player_cards, card_to_move->value) != 0) return 0xFFFFFFFF;
  
  // Add card to new_split_hand's player_cards
  if (get_card(new_split_hand->player_cards, card_to_move->value) != 0) return 0xFFFFFFFF;

  // Draw a new card for the original hand
  uint32_t new_card_val_orig_hand = draw_card(g_srabbit->deck_ptr);
  if (get_card(current_hand->player_cards, new_card_val_orig_hand) != 0) return 0xFFFFFFFF;

  // Draw a new card for the new split hand
  uint32_t new_card_val_split_hand = draw_card(g_srabbit->deck_ptr);
  if (get_card(new_split_hand->player_cards, new_card_val_split_hand) != 0) return 0xFFFFFFFF;

  current_hand->can_act = 0; // Original hand can no longer double down/split after splitting
  
  return 0; // Success
}

// Function: dealer_hit
int dealer_hit(void) {
  if (is_player_finished() == 0) {
    return -1; // Not dealer's turn yet, players haven't finished
  }

  int dealer_score;
  uint32_t has_convertible_ace;

  while (true) {
    dealer_score = calc_score(g_srabbit->dealer_cards, &has_convertible_ace);

    if (dealer_score > 21) { // Dealer busted
        return dealer_score;
    }
    if (dealer_score > 17) { // Dealer stands on 18 or more
      return dealer_score;
    }
    if (dealer_score == 17 && has_convertible_ace == 0) { // Dealer stands on hard 17
      return 17;
    }
    // Dealer hits on 16 or less, and on soft 17 (score 17, has_convertible_ace is 1)

    uint32_t card_val = draw_card(g_srabbit->deck_ptr);
    if (get_card(g_srabbit->dealer_cards, card_val) != 0) {
      return -1; // Error drawing card
    }
  }
}