#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Global constants for points
const int ROYAL_FLUSH_POINTS = 800;
const int STRAIGHT_FLUSH_POINTS = 500;
const int FLUSH_POINTS = 200;
const int FOUR_OF_A_KIND_POINTS = 100;
const int FULL_HOUSE_POINTS = 50;
const int TWO_PAIR_POINTS = 20;
const int THREE_OF_A_KIND_POINTS = 15;
const int JACKS_OR_BETTER_POINTS = 10;
const int STRAIGHT_POINTS = 5;

// poker_value maps card IDs (0-51) to their rank values (e.g., 2-14 for Ace)
// Assuming card IDs are 0-51.
// Rank values: 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 (J), 12 (Q), 13 (K), 14 (A)
// poker_ranks_map: 0-Ace, 1-2, 2-3, 3-4, 4-5, 5-6, 6-7, 7-8, 8-9, 9-T, 10-J, 11-Q, 12-K
int poker_ranks_map[] = {14, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13}; 
int poker_value[52]; // Array to store poker values for each card ID (0-51)

// Global hand variable used by helper functions
char current_hand[5];

// --- Helper Functions (Placeholders for compilation) ---

// Initializes the poker_value array. Should be called once at program start.
void init_poker_value() {
    for (int i = 0; i < 52; ++i) {
        poker_value[i] = poker_ranks_map[i % 13];
    }
}

void shuffle() { 
    // Placeholder: Shuffles the deck.
}

char draw() { 
    // Placeholder: Draws a single card. Returns card ID (0-51).
    static char next_card_id = 0;
    char card = next_card_id;
    next_card_id = (next_card_id + 1) % 52;
    return card;
}

// Placeholder: Reads user input into a buffer.
void receive_fixed_input(char* buffer, int size) {
    if (fgets(buffer, size, stdin) == NULL) {
        buffer[0] = 'q'; // Default to quit on error
    }
    buffer[strcspn(buffer, "\n")] = 0; // Remove newline character
}

// Placeholder: Prints the current hand.
void print_hand_impl(char* hand) {
    printf("Hand: ");
    for (int i = 0; i < 5; ++i) {
        char rank_char;
        int rank = hand[i] % 13;
        if (rank == 0) rank_char = 'A';
        else if (rank == 9) rank_char = 'T';
        else if (rank == 10) rank_char = 'J';
        else if (rank == 11) rank_char = 'Q';
        else if (rank == 12) rank_char = 'K';
        else rank_char = (char)('0' + poker_ranks_map[rank]);
        
        char suit_char;
        int suit = hand[i] / 13;
        if (suit == 0) suit_char = 'C'; // Clubs
        else if (suit == 1) suit_char = 'D'; // Diamonds
        else if (suit == 2) suit_char = 'H'; // Hearts
        else suit_char = 'S'; // Spades

        printf("%c%c ", rank_char, suit_char); 
    }
    printf("\n");
}

// Placeholder: Retrieves the suit of the next card in sequence.
static int get_suit_card_idx = 0;
int get_suit_impl() {
    if (get_suit_card_idx < 5) {
        int suit = current_hand[get_suit_card_idx] / 13; // Assuming suit is card_id / 13
        get_suit_card_idx++;
        return suit;
    }
    return -1; // Indicate end of hand or error
}

// Resets the index for get_suit_impl.
void reset_get_suit_idx() {
    get_suit_card_idx = 0;
}

// Redefine the function calls to use the explicit versions with `current_hand`
#define print_hand() print_hand_impl(current_hand)
#define get_suit() get_suit_impl()

// Struct for player data, based on memory offsets in the original snippet.
typedef struct {
    unsigned char _padding_0_27[40]; // Filler to reach offset 0x28 (40 bytes)
    int credits;                    // At offset 0x28
    unsigned char _padding_2c_2f[4]; // Filler to reach offset 0x30 (4 bytes)
    int score;                      // At offset 0x30
} PlayerData;

// Function: sort_by_suit
void sort_by_suit(char* hand, unsigned int size) {
  unsigned char key_card;
  int j;
  
  for (unsigned int i = 1; i < size; i++) {
    key_card = hand[i];
    j = i - 1;
    
    while (j >= 0 && hand[j] > key_card) {
      hand[j + 1] = hand[j];
      j = j - 1;
    }
    hand[j + 1] = key_card;
  }
}

// Function: sort_by_value
void sort_by_value(char* hand, unsigned int size) {
  unsigned char key_card;
  int j;
  
  for (unsigned int i = 1; i < size; i++) {
    key_card = hand[i];
    j = i - 1;
    
    while (j >= 0 && poker_value[(unsigned char)hand[j]] > poker_value[(unsigned char)key_card]) {
      hand[j + 1] = hand[j];
      j = j - 1;
    }
    hand[j + 1] = key_card;
  }
}

// Function: count_cards
int count_cards(char* hand, unsigned int size, char card_value) {
  int count = 0;
  for (unsigned int i = 0; i < size; i++) {
    if (poker_value[(unsigned char)hand[i]] == poker_value[(unsigned char)card_value]) {
      count++;
    }
  }
  return count;
}

// Function: poker
void poker(PlayerData* player_data_ptr) {
  char input_buffer[5];
  char hold_mask[5];
  
  PlayerData local_player_data_storage;
  if (player_data_ptr == NULL) {
    memset(&local_player_data_storage, 0, sizeof(PlayerData));
    player_data_ptr = &local_player_data_storage;
  }

  while (1) {
    shuffle();
    
    for (int i = 0; i < 5; ++i) {
        current_hand[i] = draw();
    }
    
    print_hand();
    
    memset(input_buffer, 0, sizeof(input_buffer));
    printf("Enter cards to hold by number (ex: 13 or 12345). Others will be replaced.\n");
    receive_fixed_input(input_buffer, sizeof(input_buffer));
    
    if (input_buffer[0] == 'q') {
      return;
    }
    
    player_data_ptr->credits += 100;
    if (4 < player_data_ptr->score) {
      player_data_ptr->score -= 5;
    }
    
    memset(hold_mask, 0, sizeof(hold_mask));
    for (int i = 0; i < 5; i++) {
      if (input_buffer[i] >= '1' && input_buffer[i] <= '5') {
        hold_mask[input_buffer[i] - '1'] = 1;
      }
    }
    
    for (int i = 0; i < 5; i++) {
      if (hold_mask[i] == 0) {
        current_hand[i] = draw();
      }
    }
    
    print_hand();
    
    // --- Hand Evaluation ---
    sort_by_suit(current_hand, 5);
    reset_get_suit_idx();
    int suit1 = get_suit_impl();
    int suit2 = get_suit_impl();
    int suit3 = get_suit_impl();
    int suit4 = get_suit_impl();
    int suit5 = get_suit_impl();

    if (suit1 == suit2 && suit2 == suit3 && suit3 == suit4 && suit4 == suit5) {
        sort_by_value(current_hand, 5); // Sort by value for straight checks

        int is_royal_flush_candidate = 0;
        // Card IDs for Ten of each suit: 9, 22, 35, 48.
        if (current_hand[0] == 9 || current_hand[0] == 22 || current_hand[0] == 35 || current_hand[0] == 48) {
            is_royal_flush_candidate = 1;
        }

        int is_straight_value = (poker_value[(unsigned char)current_hand[1]] == poker_value[(unsigned char)current_hand[0]] + 1 &&
                                 poker_value[(unsigned char)current_hand[2]] == poker_value[(unsigned char)current_hand[0]] + 2 &&
                                 poker_value[(unsigned char)current_hand[3]] == poker_value[(unsigned char)current_hand[0]] + 3 &&
                                 poker_value[(unsigned char)current_hand[4]] == poker_value[(unsigned char)current_hand[0]] + 4);
        
        int is_ace_low_straight = (poker_value[(unsigned char)current_hand[0]] == 2 && // 2,3,4,5,A
                                   poker_value[(unsigned char)current_hand[1]] == 3 &&
                                   poker_value[(unsigned char)current_hand[2]] == 4 &&
                                   poker_value[(unsigned char)current_hand[3]] == 5 &&
                                   poker_value[(unsigned char)current_hand[4]] == 14);

        if (is_royal_flush_candidate && (is_straight_value || is_ace_low_straight)) {
              printf("Royal Flush!\n");
              player_data_ptr->score += ROYAL_FLUSH_POINTS;
        } else if (is_straight_value || is_ace_low_straight) {
              printf("Straight Flush!\n");
              player_data_ptr->score += STRAIGHT_FLUSH_POINTS;
        } else {
              printf("Flush!\n");
              player_data_ptr->score += FLUSH_POINTS;
        }
        continue;
    }
    
    sort_by_value(current_hand, 5);
    
    int three_of_a_kind_flag = 0;
    int pair_flag = 0;
    int jacks_or_better_pair_flag = 0;
    int current_card_count;
    
    for (unsigned int i = 0; i < 5; i += current_card_count) {
      current_card_count = count_cards(current_hand, 5, current_hand[i]);
      
      if (current_card_count == 4) {
        printf("Four of a kind!\n");
        player_data_ptr->score += FOUR_OF_A_KIND_POINTS;
        continue;
      }
      
      if (current_card_count == 3) {
        if (pair_flag == 1) {
          printf("Full House!\n");
          player_data_ptr->score += FULL_HOUSE_POINTS;
          continue;
        }
        three_of_a_kind_flag = 1;
      }
      
      if (current_card_count == 2) {
        if (three_of_a_kind_flag == 1) {
          printf("Full House!\n");
          player_data_ptr->score += FULL_HOUSE_POINTS;
          continue;
        }
        if (poker_value[(unsigned char)current_hand[i]] >= 11) { // J=11, Q=12, K=13, A=14
          jacks_or_better_pair_flag = 1;
        }
        
        if (pair_flag == 1) {
          printf("Two Pair!\n");
          player_data_ptr->score += TWO_PAIR_POINTS;
          continue;
        }
        pair_flag = 1;
      }
    }
    
    if (three_of_a_kind_flag == 1) {
      printf("Three of a kind!\n");
      player_data_ptr->score += THREE_OF_A_KIND_POINTS;
    } else if (jacks_or_better_pair_flag == 1) {
      printf("Jacks or better!\n");
      player_data_ptr->score += JACKS_OR_BETTER_POINTS;
    } else {
        int is_straight_value = (poker_value[(unsigned char)current_hand[1]] == poker_value[(unsigned char)current_hand[0]] + 1 &&
                                 poker_value[(unsigned char)current_hand[2]] == poker_value[(unsigned char)current_hand[0]] + 2 &&
                                 poker_value[(unsigned char)current_hand[3]] == poker_value[(unsigned char)current_hand[0]] + 3 &&
                                 poker_value[(unsigned char)current_hand[4]] == poker_value[(unsigned char)current_hand[0]] + 4);
        
        int is_ace_low_straight = (poker_value[(unsigned char)current_hand[0]] == 2 && // 2,3,4,5,A
                                   poker_value[(unsigned char)current_hand[1]] == 3 &&
                                   poker_value[(unsigned char)current_hand[2]] == 4 &&
                                   poker_value[(unsigned char)current_hand[3]] == 5 &&
                                   poker_value[(unsigned char)current_hand[4]] == 14);

        if (is_straight_value || is_ace_low_straight) {
            printf("Straight!\n");
            player_data_ptr->score += STRAIGHT_POINTS;
        } else {
            printf("You Lose!\n");
        }
    }
  }
}