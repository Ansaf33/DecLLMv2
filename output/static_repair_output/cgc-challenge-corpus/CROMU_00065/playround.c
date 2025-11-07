#include <stdio.h>   // For printf, fgets
#include <stdlib.h>  // For atoi
#include <string.h>  // For memset, strncpy, strcspn
#include <stdint.h>  // For uint8_t, uint16_t

// Constants for card values and deck size
#define DECK_SIZE 2048 // Based on *param_2 < 0x7ff in original code
#define MAX_PLAYERS 8
#define MAX_HAND_CARDS 10

// Global card names array (replaces original 0x18000 address)
const char *CARD_NAMES[] = {
    "N/A", "Ace", "2", "3", "4", "5", "6", "7", "8", "9", "10", "Jack", "Queen", "King"
};

// Global deck data (replaces original 0x4347c000 address)
// This array is used to draw cards. Its values are transformed into card ranks (1-13).
// It's not 'const' because the index `deck_idx` is modified, implying a pseudo-random sequence.
uint16_t DECK_DATA[DECK_SIZE];

// Function pointer type for player strategy (for bots or hints)
typedef char (*StrategyFuncPtr)(uint8_t dealer_up_card, uint8_t *player_hand);

// Structure for a player based on original code's memory offsets (0x30 bytes total)
typedef struct {
    char name[12];             // 0x00 - 0x0B (used for player name)
    StrategyFuncPtr strategy_func; // 0x0C - 0x0F (4 bytes for function pointer)
    int money;                 // 0x10 - 0x13
    uint8_t cards[MAX_HAND_CARDS]; // 0x14 - 0x1D (player's hand, 0-terminated)
    uint8_t is_bot;            // 0x1E (1 for bot, 0 for human)
    uint8_t padding_1;         // 0x1F (padding for alignment)
    uint8_t hint_enabled;      // 0x20 (1 to enable hints, 0 otherwise)
    uint8_t padding_2[3];      // 0x21 - 0x23 (padding for alignment)
    int wins;                  // 0x24 - 0x27
    int losses;                // 0x28 - 0x2B
    int bet;                   // 0x2C - 0x2F (current round's bet)
} Player;

// Forward declarations
int score_cards(uint8_t *hand_cards);
void show_cards(uint8_t *dealer_cards, Player *players);
int play_round(Player *players, uint16_t *deck_idx);

// Helper function to initialize the DECK_DATA array with example values
void init_deck_data() {
    // Fill DECK_DATA with a simple sequence for demonstration.
    // In a real game, this would be a properly shuffled deck or a seed for a PRNG.
    for (int i = 0; i < DECK_SIZE; ++i) {
        DECK_DATA[i] = (uint16_t)(i % 52); // Example: 0-51, representing 4 suits * 13 ranks
    }
}

// Mock receive_until function
// Reads up to `max_len` characters into `buffer`, or until newline.
// Returns 0 on error/EOF, 1 on success.
int receive_until(char *buffer, int max_len, int timeout_seconds) {
    // timeout_seconds is ignored for this mock implementation
    if (fgets(buffer, max_len, stdin) == NULL) {
        return 0; // Error or EOF
    }
    // Remove trailing newline if present
    buffer[strcspn(buffer, "\n")] = 0;
    return 1;
}

// Helper function to get a card from the deck and advance the deck index
uint8_t get_next_card(uint16_t *deck_idx) {
    uint16_t raw_card_val = DECK_DATA[*deck_idx];
    // Card value is (raw_card_val % 13) + 1, so Ace=1, 2=2, ..., King=13
    uint8_t card_value = (raw_card_val % 13) + 1;
    
    // Advance deck index, wrapping around if necessary
    *deck_idx = (*deck_idx + 1) % DECK_SIZE;
    
    return card_value;
}

// Function: score_cards
// Calculates the score of a given hand of cards. Aces can be 1 or 11.
int score_cards(uint8_t *hand_cards) {
  int score = 0;
  int aces = 0; // Count of Aces (initial value 11)
  
  for (int i = 0; i < MAX_HAND_CARDS && hand_cards[i] != 0; ++i) {
    uint8_t card_val = hand_cards[i];
    if (card_val == 1) { // Ace
        aces++;
        score += 11;
    } else if (card_val >= 10 && card_val <= 13) { // 10, Jack, Queen, King
        score += 10;
    } else { // Cards 2-9
        score += card_val;
    }
  }
  
  // Adjust for Aces if score is over 21
  while (score > 21 && aces > 0) {
    score -= 10; // Change an Ace's value from 11 to 1
    aces--;
  }
  
  return score;
}

// Function: show_cards
// Displays the dealer's and all active players' hands and scores.
void show_cards(uint8_t *dealer_cards, Player *players) {
  printf("Dealer:");
  for (int i = 0; i < MAX_HAND_CARDS && dealer_cards[i] != 0; ++i) {
    printf(" %s", CARD_NAMES[dealer_cards[i]]);
  }
  int dealer_score = score_cards(dealer_cards);
  printf(" (%d)\n", dealer_score);
  
  for (int i = 0; i < MAX_PLAYERS; ++i) {
    if (players[i].name[0] != '\0') { // Check if player is active
      printf("%s:", players[i].name);
      for (int j = 0; j < MAX_HAND_CARDS && players[i].cards[j] != 0; ++j) {
        printf(" %s", CARD_NAMES[players[i].cards[j]]);
      }
      int player_score = score_cards(players[i].cards);
      printf(" (%d)\n", player_score);
    }
  }
  return;
}

// Function: play_round
// Manages a single round of Blackjack.
int play_round(Player *players, uint16_t *deck_idx) {
  uint8_t dealer_cards[MAX_HAND_CARDS];
  char input_buf[5];
  int num_active_players = 0;
  
  // Initialize dealer's hand
  memset(dealer_cards, 0, sizeof(dealer_cards));

  // Initialize player hands and count active players
  for (int i = 0; i < MAX_PLAYERS; ++i) {
    if (players[i].name[0] != '\0') {
      num_active_players++;
    }
    memset(players[i].cards, 0, sizeof(players[i].cards));
  }
  
  if (num_active_players == 0) {
    printf("No players\n");
    return -1; // Indicate error or no players
  }
  
  // Player betting phase
  for (int i = 0; i < MAX_PLAYERS; ++i) {
    if (players[i].name[0] != '\0') {
      printf("Player: %s\n", players[i].name);
      printf("Place your bet (1-%d)\n", players[i].money);
      
      if (players[i].is_bot == 1) {
        players[i].bet = 5; // Bot always bets 5
        printf("%s bets %d (bot)\n", players[i].name, players[i].bet);
      } else {
        if (!receive_until(input_buf, sizeof(input_buf), 5)) {
          return -1; // Input error
        }
        int bet_amount = atoi(input_buf);
        if (bet_amount <= 0 || bet_amount > players[i].money) {
            printf("Invalid bet. Betting 1.\n"); // Default to 1 if invalid
            bet_amount = 1;
        }
        players[i].bet = bet_amount;
      }
    }
  }

  // Deal initial cards: Dealer gets 1 up, players get 2 up, dealer gets 1 down (not shown yet)
  dealer_cards[0] = get_next_card(deck_idx); // Dealer's up card
  
  for (int i = 0; i < MAX_PLAYERS; ++i) {
    if (players[i].name[0] != '\0') {
      players[i].cards[0] = get_next_card(deck_idx); // Player's first card
    }
  }
  
  dealer_cards[1] = get_next_card(deck_idx); // Dealer's down card
  
  for (int i = 0; i < MAX_PLAYERS; ++i) {
    if (players[i].name[0] != '\0') {
      players[i].cards[1] = get_next_card(deck_idx); // Player's second card
    }
  }
  
  // Show initial hands (dealer's second card is effectively face down for player decisions)
  // For simplicity, we'll show all cards as the original code does.
  show_cards(dealer_cards, players); 

  // Player turns
  for (int i = 0; i < MAX_PLAYERS; ++i) {
    if (players[i].name[0] != '\0') {
      int player_stands = 0;
      printf("Player: %s\n", players[i].name);
      
      while (!player_stands) {
        // Display player's current hand
        for (int j = 0; j < MAX_HAND_CARDS && players[i].cards[j] != 0; ++j) {
          printf("%s ", CARD_NAMES[players[i].cards[j]]);
        }
        int current_player_score = score_cards(players[i].cards);
        printf("(%d)\n", current_player_score);

        if (players[i].hint_enabled == 1) {
          printf("Do you want a hint?\n");
          if (!receive_until(input_buf, sizeof(input_buf), 5)) return -1;
          if (input_buf[0] == 'y' || input_buf[0] == 'Y') {
            char hint = players[i].strategy_func(dealer_cards[1], players[i].cards);
            if (hint == 'H') {
              printf("You should Hit\n");
            } else if (hint == 'S') {
              printf("You should Stand\n");
            } else {
              printf("Uh.... it didn't say\n"); // Fallback for unexpected strategy output
            }
          }
        }
        
        printf("Do you wish to H)it or S)tand\n");
        if (players[i].is_bot == 1) {
          input_buf[0] = players[i].strategy_func(dealer_cards[1], players[i].cards);
          printf("%s %c (bot)\n", players[i].name, input_buf[0]);
        } else {
          if (!receive_until(input_buf, sizeof(input_buf), 5)) {
            return -1; // Input error
          }
        }
        
        if (input_buf[0] == 'S' || input_buf[0] == 's') {
          player_stands = 1;
        } else if (input_buf[0] == 'H' || input_buf[0] == 'h') {
          // Find next empty slot in player's hand
          int card_idx = 0;
          while (card_idx < MAX_HAND_CARDS && players[i].cards[card_idx] != 0) {
            card_idx++;
          }
          if (card_idx < MAX_HAND_CARDS) {
            players[i].cards[card_idx] = get_next_card(deck_idx);
            printf("%s draws %s\n", players[i].name, CARD_NAMES[players[i].cards[card_idx]]);
            current_player_score = score_cards(players[i].cards);
            if (current_player_score > 21) {
              player_stands = 1; // Player busts
              printf("%s busts!\n", players[i].name);
            }
          } else {
              printf("%s's hand is full, must stand.\n", players[i].name);
              player_stands = 1;
          }
        } else {
            printf("Invalid input. Please choose H or S.\n");
        }
      }
    }
  }

  // Dealer's turn
  int dealer_card_idx = 0;
  // Find next empty slot in dealer's hand
  while (dealer_card_idx < MAX_HAND_CARDS && dealer_cards[dealer_card_idx] != 0) {
    dealer_card_idx++;
  }

  while (score_cards(dealer_cards) <= 16) { // Dealer hits on 16 or less
    if (dealer_card_idx < MAX_HAND_CARDS) {
        dealer_cards[dealer_card_idx] = get_next_card(deck_idx);
        printf("Dealer draws %s\n", CARD_NAMES[dealer_cards[dealer_card_idx]]);
        dealer_card_idx++;
    } else {
        printf("Dealer's hand is full, must stand.\n");
        break;
    }
  }
  int final_dealer_score = score_cards(dealer_cards);
  printf("Dealer final hand: ");
  for (int i = 0; i < MAX_HAND_CARDS && dealer_cards[i] != 0; ++i) {
      printf("%s ", CARD_NAMES[dealer_cards[i]]);
  }
  printf("(%d)\n", final_dealer_score);


  // Determine winners
  for (int i = 0; i < MAX_PLAYERS; ++i) {
    if (players[i].name[0] != '\0') {
      printf("%s: ", players[i].name);
      int player_final_score = score_cards(players[i].cards);
      
      // Player busts
      if (player_final_score > 21) {
        printf("loses (busts)!\n");
        players[i].money -= players[i].bet;
        players[i].losses++;
      } 
      // Dealer busts, player doesn't
      else if (final_dealer_score > 21) {
        printf("wins (dealer busts)!\n");
        players[i].money += players[i].bet;
        players[i].wins++;
      }
      // Both valid scores
      else if (player_final_score > final_dealer_score) {
        printf("wins!\n");
        players[i].money += players[i].bet;
        players[i].wins++;
      } else if (player_final_score == final_dealer_score) {
        printf("pushes\n");
      } else { // player_final_score < final_dealer_score
        printf("loses\n");
        players[i].money -= players[i].bet;
        players[i].losses++;
      }

      // Check if player is out of money
      if (players[i].money <= 0) {
        printf("%s is out of money and out of the game!\n", players[i].name);
        players[i].name[0] = '\0'; // Mark player as inactive
        players[i].wins = 0;
        players[i].losses = 0;
        players[i].is_bot = 0; // Reset bot status
        players[i].hint_enabled = 0;
        players[i].strategy_func = NULL;
      }
    }
  }
  
  return 0; // Success
}

// Dummy strategy function for bots and hints
char bot_strategy(uint8_t dealer_up_card, uint8_t *player_hand) {
    int player_score = score_cards(player_hand);
    
    // Simple strategy:
    // Always hit on 11 or less
    if (player_score <= 11) {
        return 'H'; 
    } 
    // Always stand on 17 or more
    else if (player_score >= 17) {
        return 'S'; 
    } 
    // For hard 12-16, follow basic strategy against dealer's up card
    else { 
        // Hit if dealer has 7, 8, 9, 10/Face, or Ace
        if (dealer_up_card >= 7 || dealer_up_card == 1) { 
            return 'H';
        } 
        // Stand if dealer has 2-6
        else {
            return 'S';
        }
    }
}

// Minimal main function to demonstrate functionality
int main() {
    init_deck_data(); // Initialize the DECK_DATA array

    Player players[MAX_PLAYERS];
    uint16_t deck_index = 0; // Current index into DECK_DATA

    // Initialize all player slots to empty
    memset(players, 0, sizeof(players));

    // Setup Player 1 (human)
    strncpy(players[0].name, "Alice", sizeof(players[0].name) - 1);
    players[0].money = 100;
    players[0].is_bot = 0;
    players[0].hint_enabled = 1;
    players[0].strategy_func = bot_strategy; // Provide strategy for hint system

    // Setup Player 2 (bot)
    strncpy(players[1].name, "BotBob", sizeof(players[1].name) - 1);
    players[1].money = 100;
    players[1].is_bot = 1;
    players[1].hint_enabled = 0; // Bots don't need hints
    players[1].strategy_func = bot_strategy;

    printf("Welcome to Blackjack!\n");

    int round_num = 1;
    while (1) {
        printf("\n--- Round %d ---\n", round_num);
        if (play_round(players, &deck_index) == -1) {
            printf("Error during round or no players left. Exiting.\n");
            break;
        }

        int active_players_count = 0;
        for (int i = 0; i < MAX_PLAYERS; ++i) {
            if (players[i].name[0] != '\0') {
                active_players_count++;
                printf("%s: Money=%d, Wins=%d, Losses=%d\n", players[i].name, players[i].money, players[i].wins, players[i].losses);
            }
        }

        if (active_players_count == 0) {
            printf("No players left. Game over.\n");
            break;
        }
        
        printf("Play another round? (y/n): ");
        char choice[5];
        if (!receive_until(choice, sizeof(choice), 5) || (choice[0] != 'y' && choice[0] != 'Y')) {
            printf("Thanks for playing!\n");
            break;
        }
        round_num++;
    }

    return 0;
}