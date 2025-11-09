#include <stdio.h>
#include <string.h> // For bzero, memset
#include <stdint.h> // For fixed-width types if preferred, but int is fine

// --- Mock/External Declarations for Compilation ---
// These would typically be in other source files or headers.

// Assuming 'bj_value' is an array mapping card char values to their integer values.
// The size 256 covers all possible char values (0-255).
// Ace values are handled specially in hand_total, so their value here might be 0 or 1.
// For simplicity, let's just make sure all card chars used have a value.
const int bj_value[256] = {
    // These specific chars are identified as aces
    ['\r'] = 1, // Ace (value 1 here, 11/1 logic is in hand_total)
    ['\x1a'] = 1, // Ace
    ['\''] = 1, // Ace
    ['4'] = 1, // Ace
    ['K'] = 10, ['Q'] = 10, ['J'] = 10, ['T'] = 10, // Face cards and Ten
    ['9'] = 9, ['8'] = 8, ['7'] = 7, ['6'] = 6, ['5'] = 5,
    ['3'] = 3, ['2'] = 2, // Number cards
    [0 ... 255] = 0 // Default value for other chars
};

// Assuming 'bj_card_names' is an array mapping card char values to their string representations.
// 0x18000 offset is removed by direct array indexing.
const char *bj_card_names[256] = {
    ['\r'] = "Ace",
    ['\x1a'] = "Ace",
    ['\''] = "Ace",
    ['4'] = "Ace",
    ['K'] = "King",
    ['Q'] = "Queen",
    ['J'] = "Jack",
    ['T'] = "Ten",
    ['9'] = "Nine",
    ['8'] = "Eight",
    ['7'] = "Seven",
    ['6'] = "Six",
    ['5'] = "Five",
    ['3'] = "Three",
    ['2'] = "Two",
    [0 ... 255] = "Unknown Card" // Default for unknown cards
};

// Mock function prototypes for external functions
void shuffle(char *deck) { (void)deck; /* Mock implementation */ }
char draw(char *deck) { (void)deck; return 'T'; /* Mock: always draw a Ten */ }
void receive_fixed_input(char *buffer, int max_len, int flags) {
    (void)max_len; (void)flags;
    static int hit_count = 0;
    if (hit_count < 1) { // Allow one hit for player to demonstrate
        buffer[0] = 'h';
    } else {
        buffer[0] = 's';
    }
    hit_count++;
    buffer[1] = '\0';
    printf("%c\n", buffer[0]); // Echo input for debugging mock
}
void print_hand(char *hand, int num_cards) {
    printf("[");
    for (int i = 0; i < num_cards && hand[i] != '\0'; ++i) {
        printf("%s%s", bj_card_names[(unsigned char)hand[i]], (i < num_cards - 1 && hand[i+1] != '\0') ? ", " : "");
    }
    printf("]\n");
}

// Mock PlayerData structure based on observed offsets
typedef struct PlayerData {
    char _pad_0_23[0x24];
    int some_score_at_24; // Used at offset 0x24
    char _pad_28_2F[0x30 - 0x24 - sizeof(int)];
    int current_balance_at_30; // Used at offset 0x30
} PlayerData;

// Mock game outcome values
const int _DAT_0001810c = 0;   // Tie: no change in balance
const int _DAT_00018110 = 100; // Player Blackjack Win: gain 100
const int _DAT_00018114 = 50;  // Player Win: gain 50

// --- End Mock/External Declarations ---

// Function: is_an_ace
int is_an_ace(char card_char) {
    return (card_char == '\r' || card_char == '\x1a' || card_char == '\'' || card_char == '4');
}

// Function: hand_total
// hand_cards: Pointer to an array of card characters
// num_cards: The number of cards currently in the hand (actual count, not max capacity)
// current_total: The running total of card values (used in recursion for aces)
int hand_total(const char *hand_cards, unsigned int num_cards, int current_total) {
    for (unsigned int i = 0; i < num_cards; ++i) {
        if (is_an_ace(hand_cards[i])) {
            // Ace found, calculate two possibilities recursively.
            // The ace itself is "removed" from the current iteration and its value (11 or 1)
            // is added to the running total for the recursive calls.
            unsigned int remaining_cards_count = num_cards - (i + 1);
            const char *remaining_cards_ptr = hand_cards + (i + 1);

            int total_if_ace_is_11 = hand_total(remaining_cards_ptr, remaining_cards_count, current_total + 11);
            if (total_if_ace_is_11 < 22) { // If 11 doesn't bust, prefer it
                return total_if_ace_is_11;
            }
            // Otherwise, calculate with ace as 1
            return hand_total(remaining_cards_ptr, remaining_cards_count, current_total + 1);
        }
        // If not an ace, add its value from the bj_value table
        current_total += bj_value[(unsigned char)hand_cards[i]];
    }
    return current_total;
}

// Function: blackjack
void blackjack(PlayerData *player_data_ptr) {
    char player_hand[11]; // Max 11 cards (e.g., A,A,A,A,A,A,A,A,A,A,A would be 11 aces for 11 or 21)
    char dealer_hand[11];
    char deck[53]; // Standard deck size
    char input_buffer[10]; // For user input 'h' or 's'

    PlayerData local_player_data_storage; // Equivalent to local_60
    if (player_data_ptr == NULL) {
        memset(&local_player_data_storage, 0, sizeof(PlayerData));
        player_data_ptr = &local_player_data_storage;
    }

    bzero(player_hand, sizeof(player_hand));
    bzero(dealer_hand, sizeof(dealer_hand));

    shuffle(deck);

    player_data_ptr->some_score_at_24 += 50; // 0x32 is 50
    if (9 < player_data_ptr->current_balance_at_30) {
        player_data_ptr->current_balance_at_30 -= 10;
    }

    // Initial deal: 2 cards each
    player_hand[0] = draw(deck);
    dealer_hand[0] = draw(deck);
    player_hand[1] = draw(deck);
    dealer_hand[1] = draw(deck);

    printf("D: * %s \n", bj_card_names[(unsigned char)dealer_hand[1]]);
    printf("P: %s %s \n", bj_card_names[(unsigned char)player_hand[0]],
                          bj_card_names[(unsigned char)player_hand[1]]);

    int dealer_initial_total = hand_total(dealer_hand, 2, 0); // Only first two cards
    int player_initial_total = hand_total(player_hand, 2, 0);

    if (dealer_initial_total == 21) {
        if (player_initial_total == 21) {
            printf("Dealer Blackjack! Player Blackjack! You Tie :/\n");
            player_data_ptr->current_balance_at_30 += _DAT_0001810c;
        } else {
            printf("Dealer Blackjack! You Lose!\n");
        }
    } else {
        if (player_initial_total == 21) {
            printf("Blackjack! You Win!\n");
            player_data_ptr->current_balance_at_30 += _DAT_00018110;
        } else {
            int player_card_count = 2; // Keep track of how many cards player has
            int current_player_total;
            do {
                printf("Enter h for hit or s for stand:");
                receive_fixed_input(input_buffer, sizeof(input_buffer) - 1, 1);

                if (input_buffer[0] == 'h') {
                    player_hand[player_card_count] = draw(deck);
                    player_card_count++;
                    printf("P: ");
                    print_hand(player_hand, player_card_count);
                    current_player_total = hand_total(player_hand, player_card_count, 0);
                    if (current_player_total > 21) {
                        printf("Bust\n");
                        break; // Player busts, end player's turn
                    }
                }
            } while (input_buffer[0] != 's');

            // Dealer's turn
            int dealer_card_count = 2; // Keep track of how many cards dealer has
            printf("D: ");
            print_hand(dealer_hand, dealer_card_count); // Show dealer's full hand now

            int current_dealer_total;
            while ( (current_dealer_total = hand_total(dealer_hand, dealer_card_count, 0)) < 17) {
                dealer_hand[dealer_card_count] = draw(deck);
                dealer_card_count++;
                printf("D: ");
                print_hand(dealer_hand, dealer_card_count);
            }

            // Final totals for comparison
            current_player_total = hand_total(player_hand, player_card_count, 0);
            current_dealer_total = hand_total(dealer_hand, dealer_card_count, 0);

            printf("P:%d D:%d\n", current_player_total, current_dealer_total);

            if (current_player_total < 22) { // Player did not bust
                if (current_dealer_total < 22) { // Dealer did not bust
                    if (current_dealer_total < current_player_total) {
                        printf("You Win.\n");
                        player_data_ptr->current_balance_at_30 += _DAT_00018114;
                    } else if (current_player_total == current_dealer_total) {
                        printf("Push.\n");
                        player_data_ptr->current_balance_at_30 += _DAT_0001810c;
                    } else {
                        printf("You Lose.\n");
                    }
                } else { // Dealer Busts
                    printf("Dealer Bust. You Win.\n");
                    player_data_ptr->current_balance_at_30 += _DAT_00018114;
                }
            } else { // Player Busts
                printf("Player Bust. You Lose.\n");
            }
        }
    }
}