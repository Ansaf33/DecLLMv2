#include <stdio.h>    // For printf
#include <stdlib.h>   // For atoi
#include <string.h>   // For memset, strcspn
#include <stdint.h>   // For uint8_t, uint16_t

// Placeholder for card names (0x18000 in original code)
// Card values are 1-13 (1=Ace, 2-10, 11=Jack, 12=Queen, 13=King)
// Index 0 is unused for values, but code might produce it, so include it.
const char* card_names[] = {
    "ERR", "A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"
};

// Placeholder for deck data (0x4347c000 in original code)
// Max index 0x7ff (2047), so 2048 elements.
// This would typically be initialized with shuffled card indices.
uint16_t deck_data[2048];

// Initialize deck_data with a repeating sequence of card indices (0-51)
void initialize_deck_data() {
    for (int i = 0; i < 2048; ++i) {
        deck_data[i] = i % 52; // Simulate a repeating sequence of 52 cards
    }
}

// Player structure based on offsets
typedef struct player_t player_t; // Forward declaration for function pointer
typedef char (*strategy_func_ptr)(uint8_t dealer_up_card, uint8_t* player_hand);

struct player_t {
    char name[16]; // 0x0 - 0xb, assuming name is null-terminated and fits
    strategy_func_ptr strategy_func; // offset 0xc
    int balance; // offset 0x10
    uint8_t hand[10]; // offset 0x14, max 10 cards
    uint8_t is_bot; // offset 0x1e
    uint8_t hint_enabled; // offset 0x20
    int wins; // offset 0x24
    int losses; // offset 0x28
    int current_bet; // offset 0x2c
    // Total size 0x30 (48 bytes)
};

// Placeholder for receive_until
int receive_until(char* buffer, int max_len, int timeout) {
    // For this example, we'll just read from stdin without timeout.
    if (fgets(buffer, max_len, stdin) != NULL) {
        // Remove newline character if present
        buffer[strcspn(buffer, "\n")] = 0;
        return 1; // Success
    }
    return 0; // Failure (e.g., EOF)
}

// Simple bot strategy: Hit on 16 or less, Stand on 17 or more
char simple_bot_strategy(uint8_t dealer_up_card, uint8_t* player_hand) {
    // Calculate player's current score
    int score = 0;
    int ace_count = 0;
    for (int i = 0; i < 10 && player_hand[i] != 0; ++i) {
        uint8_t card_value = player_hand[i];
        if (card_value == 1) { // Ace
            ace_count++;
            score += 11;
        } else if (card_value >= 10) { // 10, J, Q, K
            score += 10;
        } else { // 2-9
            score += card_value;
        }
    }
    while (score > 21 && ace_count > 0) {
        score -= 10;
        ace_count--;
    }

    if (score <= 16) {
        return 'H'; // Hit
    } else {
        return 'S'; // Stand
    }
}

// Function: score_cards
int score_cards(uint8_t* hand) {
  int score = 0;
  int ace_count = 0;
  
  for (int i = 0; i < 10 && hand[i] != 0; ++i) {
    uint8_t card_value = hand[i];
    if (card_value == 1) { // Ace
      ace_count++;
      score += 11;
    } else if (card_value >= 10) { // 10, J, Q, K
      score += 10;
    } else { // 2-9
      score += card_value;
    }
  }

  // Adjust for Aces if score is over 21
  while (score > 21 && ace_count > 0) {
    score -= 10;
    ace_count--;
  }
  return score;
}

// Function: show_cards
void show_cards(uint8_t* dealer_hand, player_t* players) {
  printf("Dealer:");
  for (int i = 0; i < 10 && dealer_hand[i] != 0; ++i) {
    printf(" %s", card_names[dealer_hand[i]]);
  }
  printf(" (%d)\n", score_cards(dealer_hand));

  for (int i = 0; i < 8; ++i) {
    if (players[i].name[0] != '\0') {
      printf("%s:", players[i].name);
      for (int j = 0; j < 10 && players[i].hand[j] != 0; ++j) {
        printf(" %s", card_names[players[i].hand[j]]);
      }
      printf(" (%d)\n", score_cards(players[i].hand));
    }
  }
}

// Function: play_round
int play_round(player_t* players, uint16_t* deck_idx) {
  uint8_t dealer_hand[10];
  char input_buffer[10]; // For receive_until
  int active_player_count = 0;
  
  // Initialize dealer's hand
  memset(dealer_hand, 0, sizeof(dealer_hand));

  // Count active players and clear their hands
  for (int i = 0; i < 8; ++i) {
    if (players[i].name[0] != '\0') {
      active_player_count++;
    }
    memset(players[i].hand, 0, sizeof(players[i].hand));
  }

  if (active_player_count == 0) {
    printf("No players\n");
    return -1; // Indicate error or no players
  }

  // Place bets
  for (int i = 0; i < 8; ++i) {
    if (players[i].name[0] != '\0') {
      printf("Player: %s\n", players[i].name);
      printf("Place your bet (1-%d)\n", players[i].balance);
      if (players[i].is_bot) {
        players[i].current_bet = 5; // Bot always bets 5
        printf("%s bets %d\n", players[i].name, players[i].current_bet);
      } else {
        if (receive_until(input_buffer, sizeof(input_buffer), 5) == 0) {
          return -1;
        }
        players[i].current_bet = atoi(input_buffer);
        // Basic validation for bet amount
        if (players[i].current_bet < 1 || players[i].current_bet > players[i].balance) {
            printf("Invalid bet. Setting to 1.\n");
            players[i].current_bet = 1;
        }
      }
    }
  }
  
  // Deal initial cards
  // Dealer's first card (face down for now, but original code shows it)
  dealer_hand[0] = (deck_data[*deck_idx] % 13) + 1;
  *deck_idx = (*deck_idx < 0x7ff) ? (*deck_idx + 1) : 0;

  // Players' first card
  for (int i = 0; i < 8; ++i) {
    if (players[i].name[0] != '\0') {
      players[i].hand[0] = (deck_data[*deck_idx] % 13) + 1;
      *deck_idx = (*deck_idx < 0x7ff) ? (*deck_idx + 1) : 0;
    }
  }

  // Dealer's second card (face up)
  dealer_hand[1] = (deck_data[*deck_idx] % 13) + 1;
  *deck_idx = (*deck_idx < 0x7ff) ? (*deck_idx + 1) : 0;

  // Players' second card
  for (int i = 0; i < 8; ++i) {
    if (players[i].name[0] != '\0') {
      players[i].hand[1] = (deck_data[*deck_idx] % 13) + 1;
      *deck_idx = (*deck_idx < 0x7ff) ? (*deck_idx + 1) : 0;
    }
  }

  // Show initial cards (dealer's second card is face up)
  show_cards(dealer_hand, players);

  // Player turns
  for (int i = 0; i < 8; ++i) {
    if (players[i].name[0] != '\0') {
      int player_is_done = 0; // 0 = not done, 1 = done
      printf("Player: %s\n", players[i].name);
      
      while (!player_is_done) {
        // Find next empty slot in player's hand
        int hand_idx = 0;
        while (hand_idx < 10 && players[i].hand[hand_idx] != 0) {
            hand_idx++;
        }

        printf("Your hand: ");
        for (int j = 0; j < 10 && players[i].hand[j] != 0; ++j) {
          printf("%s ", card_names[players[i].hand[j]]);
        }
        int player_score = score_cards(players[i].hand);
        printf("(%d)\n", player_score);

        if (players[i].hint_enabled) {
          printf("Do you want a hint? (y/n)\n");
          if (receive_until(input_buffer, sizeof(input_buffer), 5) == 0) {
            return -1;
          }
          if (input_buffer[0] == 'y' || input_buffer[0] == 'Y') {
            char hint = players[i].strategy_func(dealer_hand[1], players[i].hand);
            if (hint == 'H') {
              printf("You should Hit\n");
            } else if (hint == 'S') {
              printf("You should Stand\n");
            } else {
              printf("Uh.... it didn't say\n"); // Fallback for unexpected strategy output
            }
          }
        }

        printf("Do you wish to H)it or S)tand?\n");
        if (players[i].is_bot) {
          input_buffer[0] = players[i].strategy_func(dealer_hand[1], players[i].hand);
          printf("%s chooses to %s\n", players[i].name, (input_buffer[0] == 'H' ? "Hit" : "Stand"));
        } else {
          if (receive_until(input_buffer, sizeof(input_buffer), 5) == 0) {
            return -1;
          }
        }

        if (input_buffer[0] == 'S' || input_buffer[0] == 's') {
          player_is_done = 1;
        } else if (input_buffer[0] == 'H' || input_buffer[0] == 'h') {
          // Player draws a card
          // Check if hand is full
          if (hand_idx >= 10) {
              printf("Hand full, cannot draw more cards.\n");
              player_is_done = 1;
              continue; // Skip to score check
          }
          players[i].hand[hand_idx] = (deck_data[*deck_idx] % 13) + 1;
          *deck_idx = (*deck_idx < 0x7ff) ? (*deck_idx + 1) : 0;
          
          printf("%s draws %s\n", players[i].name, card_names[players[i].hand[hand_idx]]);
          
          player_score = score_cards(players[i].hand);
          if (player_score > 21) {
            printf("%s busts with %d!\n", players[i].name, player_score);
            player_is_done = 1; // Player busted
          }
        } else {
            printf("Invalid input. Please enter 'H' or 'S'.\n");
        }
      }
    }
  }

  // Dealer's turn
  int dealer_hand_idx = 0;
  while (dealer_hand_idx < 10 && dealer_hand[dealer_hand_idx] != 0) {
      dealer_hand_idx++;
  }

  int dealer_score = 0;
  printf("Dealer's turn...\n");
  while (1) { // Loop until dealer stands or busts
    dealer_score = score_cards(dealer_hand);
    if (dealer_score > 16) { // Dealer stands on 17 or more
      break;
    }
    // Dealer draws a card
    if (dealer_hand_idx >= 10) {
        printf("Dealer's hand full, cannot draw more cards.\n");
        break;
    }
    dealer_hand[dealer_hand_idx] = (deck_data[*deck_idx] % 13) + 1;
    printf("Dealer draws %s\n", card_names[dealer_hand[dealer_hand_idx]]);
    *deck_idx = (*deck_idx < 0x7ff) ? (*deck_idx + 1) : 0;
    dealer_hand_idx++;
  }
  printf("Dealer stands with %d.\n", dealer_score);


  // Determine winners
  for (int i = 0; i < 8; ++i) {
    if (players[i].name[0] != '\0') {
      printf("%s: ", players[i].name);
      int player_score = score_cards(players[i].hand);
      
      if (player_score > 21) { // Player busted
        printf("loses (busted)\n");
        players[i].balance -= players[i].current_bet;
        players[i].losses++;
      } else if (dealer_score > 21) { // Dealer busted, player didn't
        printf("wins (dealer busted)!\n");
        players[i].balance += players[i].current_bet;
        players[i].wins++;
      } else if (player_score > dealer_score) { // Player has higher score than dealer
        printf("wins!\n");
        players[i].balance += players[i].current_bet;
        players[i].wins++;
      } else if (player_score == dealer_score) { // Push
        printf("pushes\n");
      } else { // Dealer has higher score than player
        printf("loses\n");
        players[i].balance -= players[i].current_bet;
        players[i].losses++;
      }

      // Check if player is out of money
      if (players[i].balance <= 0) { // Changed to <= 0 to handle negative balance
        printf("%s is out of money and leaves the table.\n", players[i].name);
        players[i].name[0] = 0; // Mark player as inactive
        players[i].wins = 0;
        players[i].losses = 0;
        players[i].is_bot = 0; // No longer a bot if inactive
        players[i].hint_enabled = 0;
        players[i].balance = 0; // Ensure balance is 0
      }
    }
  }
  return 0; // Success
}

// Main function to run the game
int main() {
    initialize_deck_data(); // Initialize the deck data

    player_t players[8];
    memset(players, 0, sizeof(players)); // Clear all player data

    // Setup some players
    strcpy(players[0].name, "Human Player");
    players[0].balance = 100;
    players[0].is_bot = 0;
    players[0].hint_enabled = 1;
    players[0].strategy_func = simple_bot_strategy; // Even for human, for hint

    strcpy(players[1].name, "Bot Player 1");
    players[1].balance = 75;
    players[1].is_bot = 1;
    players[1].hint_enabled = 0;
    players[1].strategy_func = simple_bot_strategy;

    strcpy(players[2].name, "Bot Player 2");
    players[2].balance = 120;
    players[2].is_bot = 1;
    players[2].hint_enabled = 0;
    players[2].strategy_func = simple_bot_strategy;

    uint16_t current_deck_idx = 0; // Starting index in the deck

    printf("Welcome to Blackjack!\n");
    int round_num = 1;
    while (1) {
        printf("\n--- Round %d ---\n", round_num);
        int result = play_round(players, &current_deck_idx);
        if (result == -1) {
            printf("Game ended due to input error or no players.\n");
            break;
        }

        // Check if any players are left
        int players_left = 0;
        for (int i = 0; i < 8; ++i) {
            if (players[i].name[0] != '\0') {
                players_left = 1;
                break;
            }
        }
        if (!players_left) {
            printf("All players are out of the game.\n");
            break;
        }

        printf("\n--- Scores after Round %d ---\n", round_num);
        for (int i = 0; i < 8; ++i) {
            if (players[i].name[0] != '\0') {
                printf("%s: Balance %d, Wins %d, Losses %d\n", 
                       players[i].name, players[i].balance, players[i].wins, players[i].losses);
            }
        }
        
        // Ask human player if they want to play another round
        if (players[0].name[0] != '\0' && !players[0].is_bot) {
            char choice_buffer[10];
            printf("Play another round? (y/n): ");
            if (receive_until(choice_buffer, sizeof(choice_buffer), 5) == 0 || 
                (choice_buffer[0] != 'y' && choice_buffer[0] != 'Y')) {
                printf("Thanks for playing!\n");
                break;
            }
        } else if (!players_left) { // If human player is gone, and no other players, end game
            break;
        }
        round_num++;
    }

    return 0;
}