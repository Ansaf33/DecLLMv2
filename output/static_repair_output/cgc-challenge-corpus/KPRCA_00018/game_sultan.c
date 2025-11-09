#include <stdio.h>    // For printf
#include <stdlib.h>   // For strtol
#include <string.h>   // For strcmp
#include <unistd.h>   // For read
#include <sys/types.h> // For ssize_t
#include <stdbool.h>  // For bool

// Type definitions based on typical decompiler output and C equivalents
typedef unsigned int uint;
typedef unsigned int undefined4; // Often used for 4-byte values of unknown type
// `undefined` is used as a return type for `suittos`, which returns `char*`.
// So it implies `void*` or `char*` in C. Using `char*` for direct string return.

// --- Global Variables ---
int g_player;
int DAT_000180a4;
int DAT_000180a8;
char DAT_0001509c[] = "S"; // Assuming "S" for a generic suit, as `suittos` always returns this.
char DAT_000151fa[] = "QUIT"; // For strcmp comparison for the "QUIT" command.

// --- Structure Definitions (Inferred from usage) ---
// A card node in a linked list. Used for player and dealer hands.
typedef struct CardNode {
    unsigned int card_value;
    struct CardNode *next;
} CardNode;

// Game state structure (simplified based on offsets in print_cards)
typedef struct GameState {
    // ... other game state members ...
    CardNode *dealer_hand; // Offset +4 from GameState*
    CardNode *player_hand; // Offset +8 from GameState*
    // ...
    char is_main_hand_finished; // Offset +0x14
    char can_double_down;      // Offset +0x15
    // ... more members for split hands, etc.
} GameState;

// --- Stub Function Declarations (to allow compilation) ---
// These functions represent the game logic not provided in the snippet.
// Their types are inferred from their usage in the provided code.
void *get_srabbit_game(void) {
    // Mock game state for compilation and basic execution
    static CardNode mock_dealer_card = {1 /*D*/, NULL};
    static CardNode mock_player_card1 = {7 /*7*/, NULL};
    static CardNode mock_player_card2 = {10 /*10*/, NULL};
    mock_player_card1.next = &mock_player_card2;
    static GameState mock_game_state = {
        .dealer_hand = &mock_dealer_card,
        .player_hand = &mock_player_card1,
        .is_main_hand_finished = 0, // Not finished
        .can_double_down = 1        // Can double down
    };
    return &mock_game_state;
}
void *get_split_hand(void) { return NULL; } // No split hands in this stub
unsigned int calc_score(CardNode *hand, int param_2) {
    if (hand == NULL) return 0;
    unsigned int score = 0;
    CardNode *current = hand;
    while(current != NULL) {
        // Basic score calculation stub
        if (current->card_value >= 2 && current->card_value <= 9) {
            score += current->card_value;
        } else if (current->card_value == 10 || current->card_value == 0xB || current->card_value == 0xC || current->card_value == 0xD) {
            score += 10; // 10, R, B, L
        } else if (current->card_value == 0 || current->card_value == 1) { // W (Ace), D (Duck/Ace)
            score += 11; // Assume 11 for now, real game would handle 1 or 11
        }
        current = current->next;
    }
    return score;
}
int is_player_finished(void) { return 0; } // Default to not finished
void dealer_hit(void) { printf("STUB: dealer_hit\n"); }
int can_split(void *game_state_ptr) { printf("STUB: can_split\n"); return 0; }
int split_len(void) { return 0; } // No splits for this stub
int max_split(void) { return 0; } // No splits for this stub
int deal_srabbit(int bet) { printf("STUB: deal_srabbit (bet %d)\n", bet); return 1; } // Default to normal deal
void pay_insurance(void) { printf("STUB: pay_insurance\n"); }
void check_dealer_squarerabbit(void) { printf("STUB: check_dealer_squarerabbit\n"); }
int check_dealer_squarerabbit2(void) { return 0; } // Default: dealer does not have squarerabbit
int check_player_squarerabbit(void *game_ptr) { return 0; } // Default: player does not have squarerabbit
int hit(void) { printf("STUB: hit\n"); return 0; }
int stand(void) { printf("STUB: stand\n"); return 0; }
int double_down(void) { printf("STUB: double_down\n"); return 0; }
int split(void) { printf("STUB: split\n"); return 0; }
int srabbit_payout(void) { printf("STUB: srabbit_payout\n"); return 0; }
void init_squarerabbit(long *seed) { printf("STUB: init_squarerabbit (seed %ld)\n", *seed); }


// --- Original Functions (fixed) ---

// Function: readchar
unsigned int readchar(int fd) {
    char c1, c2;
    ssize_t bytes_read;

    bytes_read = read(fd, &c1, 1);
    if (bytes_read == 1) {
        bytes_read = read(fd, &c2, 1);
        if (bytes_read == 1) {
            if (c2 == '\n') {
                return (unsigned int)c1;
            }
        }
    }
    return 0xFFFFFFFF; // Error or not newline terminated
}

// Function: readline
unsigned int readline(int fd, char *buffer, unsigned int max_len) {
    unsigned int count;
    ssize_t bytes_read;
    char *current_pos = buffer;

    for (count = 0; count < max_len; ++count) {
        bytes_read = read(fd, current_pos, 1);
        if (bytes_read == 0) { // EOF
            if (count == 0) return 1; // Nothing read, partial
            *current_pos = '\0';
            return 1; // Partial line
        }
        if (bytes_read == -1) { // Read error
            return 0xFFFFFFFF;
        }
        // bytes_read == 1
        if (*current_pos == '\n') {
            *current_pos = '\0';
            return 0; // Success, newline found
        }
        current_pos++;
    }
    // If loop finishes, max_len characters were read without a newline
    return 0xFFFFFFFF;
}

// Function: init_player
void init_player(void) {
  g_player = 100000;
  DAT_000180a4 = 0;
  DAT_000180a8 = 0;
  return;
}

// Function: print_game_menu
void print_game_menu(void) {
  printf("Game Sultan: Main Menu\n");
  printf("1. Square Rabbit\n");
  printf("2. Quit\n\n");
  printf("Select:-@ ");
  return;
}

// Function: cardtos
char * cardtos(unsigned int card_value) {
  switch(card_value) {
  case 0: return "W";
  case 1: return "D";
  case 2: return "2";
  case 3: return "3";
  case 4: return "4";
  case 5: return "5";
  case 6: return "6";
  case 7: return "7";
  case 8: return "8";
  case 9: return "9";
  case 10: return "10";
  case 0xb: return "R"; // 11
  case 0xc: return "B"; // 12
  case 0xd: return "L"; // 13
  default: return "ERROR";
  }
}

// Function: suittos
char * suittos(void) {
  return DAT_0001509c; // Return pointer to global suit string
}

// Function: print_winner
void print_winner(int player_score, int dealer_score) {
  if (player_score < 0x16) { // 0x16 is 22 (bust threshold)
    if ((dealer_score < player_score) || (0x15 < dealer_score)) { // 0x15 is 21 (blackjack)
      printf("  Winner!\n");
    }
    else if (player_score == dealer_score) {
      printf("  Pushed.\n");
    }
    else {
      printf("  Dealer Wins.\n");
    }
  }
  else {
    printf("  Busted! Better luck next time\n");
  }
  return;
}

// Function: print_cards
unsigned int print_cards(void) {
    GameState *game_state = get_srabbit_game();
    void *split_hand_base = get_split_hand(); // Base pointer for split hands

    // Dereference pointers within the GameState struct
    CardNode *dealer_hand = game_state->dealer_hand;
    CardNode *player_hand = game_state->player_hand;

    unsigned int player_score = calc_score(player_hand, 0);
    int player_finished = is_player_finished();

    unsigned int actual_dealer_score = 0; // Will be calculated if dealer's turn

    if (!player_finished) {
        unsigned int dealer_first_card_value = dealer_hand->card_value;
        printf("Dealer: |%s%s|?|\n", cardtos(dealer_first_card_value), suittos());
        printf("----------------\n");
    } else {
        dealer_hit();
        actual_dealer_score = calc_score(dealer_hand, 0);
        printf("Dealer: |");
        for (CardNode *current_card = dealer_hand; current_card != NULL; current_card = current_card->next) {
            printf("%s%s|", cardtos(current_card->card_value), suittos());
        }
        printf("    Score: %d\n", actual_dealer_score);
        printf("-------------------------------\n");
    }

    printf("Player: |");
    for (CardNode *current_card = player_hand; current_card != NULL; current_card = current_card->next) {
        printf("%s%s|", cardtos(current_card->card_value), suittos());
    }
    printf("    Score: %d\n", player_score);

    // Main hand actions
    if (game_state->is_main_hand_finished == '\0') {
        printf("Select (H/S");
        if (game_state->can_double_down != '\0') {
            printf("/D");
            if (can_split(game_state) != 0) {
                if (split_len() < max_split()) {
                    printf("/X");
                }
            }
        }
        printf("): ");
        return 1; // Player turn active for main hand
    } else {
        if (player_finished != 0) {
            print_winner(player_score, actual_dealer_score);
        }

        // Split hands actions (if any)
        for (int split_idx = 0; split_idx < split_len(); ++split_idx) {
            void *current_split_game_state_raw = (char *)split_hand_base + split_idx * 0x18;
            // Assuming split_hand_base points to a GameState-like structure or an array of them
            GameState *current_split_game_state = (GameState *)current_split_game_state_raw;

            CardNode *split_player_hand = current_split_game_state->player_hand;
            unsigned int split_player_score = calc_score(split_player_hand, 0);

            printf("Split %d: |", split_idx + 1);
            for (CardNode *current_card = split_player_hand; current_card != NULL; current_card = current_card->next) {
                printf("%s%s|", cardtos(current_card->card_value), suittos());
            }
            printf("    Score: %d\n", split_player_score);

            if (current_split_game_state->is_main_hand_finished == '\0') {
                printf("Select (H/S");
                if (current_split_game_state->can_double_down != '\0') {
                    printf("/D");
                    if (can_split(current_split_game_state) != 0) {
                        if (split_len() < max_split()) {
                            printf("/X");
                        }
                    }
                }
                printf("): ");
                return 1; // Player turn active for a split hand
            }
            if (player_finished != 0) {
                print_winner(split_player_score, actual_dealer_score);
            }
        }
    }
    return 0; // All hands finished
}

// Function: play_squarerabbit
void play_squarerabbit(void) {
    char input_buffer[512];
    char player_action_char;
    int current_bet;
    long bet_value;

    printf("******************\n");
    printf("* Square Rabbit! *\n");
    printf("******************\n");

    while (true) { // Main game loop
        while (true) { // Bet placement loop
            printf("Total Money: $%d\n", g_player);
            printf("\nType QUIT to exit to main menu\n");
            printf("Place Bet (100, 200, 500, 1000): ");
            if (readline(0, input_buffer, sizeof(input_buffer)) != 0) { // If readline fails or partial
                printf("Bad input for bet.\n\n");
                continue;
            }

            if (strcmp(input_buffer, DAT_000151fa) == 0) { // QUIT check
                return;
            }

            bet_value = strtol(input_buffer, NULL, 10);
            if (bet_value == 100 || bet_value == 200 || bet_value == 500 || bet_value == 1000) {
                current_bet = (int)bet_value;
            } else {
                current_bet = -1; // Invalid bet
            }
            
            DAT_000180a8 = current_bet; // Store the attempted bet

            if (current_bet == -1) {
                printf("Bad Bet\n\n");
            } else if (DAT_000180a8 <= g_player) {
                break; // Valid bet, enough money
            } else {
                printf("Not enough money, bet with a lower amount\n");
            }
        }

        int deal_result = deal_srabbit(DAT_000180a8);
        bool game_over_early = false; // Flag to skip player turn and go to payout

        if (deal_result == 0) { // Player Square Rabbit
            print_cards();
            printf("Square Rabbit!\n");
            game_over_early = true;
        } else if (deal_result == 1) { // Normal deal
            print_cards();
            // Player turn follows
        } else if (deal_result == 2) { // Dealer has a Duck (Ace showing)
            printf("Dealer has a Duck... Insurance? (y\\n): ");
            player_action_char = '\0';
            while (player_action_char != 'y' && player_action_char != 'n') {
                player_action_char = readchar(0);
            }
            if (player_action_char == 'y') {
                pay_insurance();
            }
            check_dealer_squarerabbit(); 
            if (check_dealer_squarerabbit2() == 0) { // If dealer does NOT have Square Rabbit
                if (check_player_squarerabbit(get_srabbit_game()) != 0) { // If player has Square Rabbit
                    print_cards();
                    printf("Square Rabbit!\n");
                    game_over_early = true;
                }
            } else { // Dealer HAS Square Rabbit
                game_over_early = true; // Game ends, payout handled
            }
        } else if (deal_result == 3) { // Player turn starts immediately (no special print)
            // player_turn_needed remains true
        }

        if (!game_over_early) {
            // Player action loop
            while (print_cards() != 0) { // print_cards returns 1 if player turn is still active, 0 if finished
                player_action_char = readchar(0);
                if (player_action_char == 'X') {
                    if ((g_player < DAT_000180a8) || (split() < 0)) {
                        printf("CAN\'T SPLIT");
                    }
                } else if (player_action_char == 'S') {
                    if (stand() < 0) {
                        printf("INVALID CHOICE");
                    }
                } else if (player_action_char == 'D') {
                    if ((g_player < DAT_000180a8) || (double_down() < 0)) {
                        printf("CAN\'T DOUBLE DOWN\'");
                    }
                } else if (player_action_char == 'H') {
                    if (hit() < 0) {
                        printf("INVALID CHOICE");
                    }
                } else {
                    printf("INVALID CHOICE");
                }
                printf("\n");
            }
        }
        
        // Payout
        g_player += srabbit_payout();
    }
}

// Function: play_poker
void play_poker(void) {
  printf("STUB: play_poker (not implemented)\n");
  return;
}

// Function: main
undefined4 main(void) {
  long initial_seed_val;
  char input_buffer[512];
  char choice_char;
  
  printf("Enter a number greater than 0: ");
  if (readline(0, input_buffer, sizeof(input_buffer)) == 0) {
    initial_seed_val = strtol(input_buffer, NULL, 10);
    if (initial_seed_val == 0) {
      printf("INVALID INPUT\n");
      printf("Exiting...\n");
    }
    else {
      printf("\n\n");
      printf("*************************\n");
      printf("*Welcome to Game Sultan!*\n");
      printf("*************************\n\n");
      init_player();
      init_squarerabbit(&initial_seed_val);
      choice_char = '0';
      while (choice_char != '2') {
        print_game_menu();
        choice_char = readchar(0);
        if (choice_char == '1') {
          play_squarerabbit();
        }
        else if (choice_char != '2') {
          printf("Bad Choice. Select another option\n\n");
        }
      }
      printf("Thanks for playing!\n");
    }
  }
  else {
    printf("INVALID INPUT\n");
    printf("Exiting...\n");
  }
  return 0;
}