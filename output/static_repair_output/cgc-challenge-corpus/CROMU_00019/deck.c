#include <stdio.h>   // For printf
#include <stdlib.h>  // For rand, srand, exit
#include <time.h>    // For time
#include <stdbool.h> // For bool

// Forward declarations of functions
char draw(char* deck);
bool is_empty(char* deck);

// Global variable for card names
// Index 0 is typically unused or represents an invalid card.
// Indices 1-52 correspond to specific cards in a standard deck.
const char* card_names[] = {
    "N/A", // Index 0: Not a valid card value for a standard deck
    // Clubs (1-13)
    "C-A", "C-2", "C-3", "C-4", "C-5", "C-6", "C-7", "C-8", "C-9", "C-10", "C-J", "C-Q", "C-K",
    // Diamonds (14-26)
    "D-A", "D-2", "D-3", "D-4", "D-5", "D-6", "D-7", "D-8", "D-9", "D-10", "D-J", "D-Q", "D-K",
    // Hearts (27-39)
    "H-A", "H-2", "H-3", "H-4", "H-5", "H-6", "H-7", "H-8", "H-9", "H-10", "H-J", "H-Q", "H-K",
    // Spades (40-52)
    "S-A", "S-2", "S-3", "S-4", "S-5", "S-6", "S-7", "S-8", "S-9", "S-10", "S-J", "S-Q", "S-K"
};

// Helper function: random_in_range
// Generates a random integer in the range [min, max] (inclusive).
int random_in_range(int min, int max) {
    return min + rand() % (max - min + 1);
}

// Helper function: _terminate
// Replaces the original _terminate function, using standard exit.
void _terminate(int status) {
    exit(status);
}

// Function: shuffle
// Shuffles a deck of 52 cards and initializes the 'top' index.
// param_1 is a pointer to the deck array.
void shuffle(char* deck) {
    // The deck array is assumed to be `char deck[53]`,
    // where deck[0...51] store card values (1-52) and deck[52] stores the 'top' index.
    for (int i = 0; i < 52; ++i) {
        int j = random_in_range(0, i); // Get a random index from 0 to i (inclusive)
        
        // This implements an inside-out Fisher-Yates shuffle algorithm.
        // It initializes the deck with unique values (1 to 52) in a random order.
        deck[i] = deck[j];     // Copy the card from the random position j to the current position i
        deck[j] = (char)(i + 1); // Place the new card value (i+1) at the random position j
    }
    deck[52] = 0; // Initialize the 'top' index to 0, meaning the first card to draw is at deck[0].
}

// Function: print_deck
// Prints the current state of the deck.
// param_1 is a pointer to the deck array.
void print_deck(char* deck) {
    printf("deck: ");
    for (int i = 0; i < 52; ++i) { // Iterate through the 52 card slots
        printf("%s ", card_names[deck[i]]); // Print card name using the value as index into card_names
    }
    printf("\ntop: %d\n", (int)deck[52]); // Print the current 'top' index of the deck
}

// Function: print_hand
// Prints the cards in a player's hand.
// param_1 is a pointer to the hand array.
// param_2 is the number of cards in the hand.
void print_hand(char* hand, unsigned int count) {
    for (unsigned int i = 0; i < count; ++i) {
        // Check if the card value is within the valid range (1 to 52)
        if ((hand[i] > 0) && (hand[i] <= 52)) { 
            printf("%s ", card_names[hand[i]]);
        } else {
            printf("[INVALID] "); // Indicate an invalid card value in the hand
        }
    }
    printf("\n");
}

// Function: draw
// Draws a card from the deck.
// param_1 is a pointer to the deck array.
// Returns the value of the drawn card.
char draw(char* deck) {
    if (is_empty(deck)) {
        printf("Error: Drawing from an empty deck!\n");
        _terminate(1); // Exit the program if trying to draw from an empty deck
    }
    // Increment the 'top' index first. It now points to the *next* card to be drawn.
    // The card just drawn was at the *previous* 'top' index.
    deck[52]++; 
    return deck[deck[52] - 1]; // Return the card at the position that was just drawn
}

// Function: is_empty
// Checks if the deck is empty.
// param_1 is a pointer to the deck array.
// Returns true if the deck is empty, false otherwise.
bool is_empty(char* deck) {
    // The deck is considered empty if the 'top' index reaches 52,
    // meaning all 52 cards (indices 0-51) have been drawn.
    return deck[52] == 52; 
}

// Function: get_suit
// Determines the suit of a given card value.
// param_1 is the card value (1-52).
// Returns an integer representing the suit (0 for invalid, 1-4 for suits).
int get_suit(char card_value) {
    if ((card_value < 1) || (card_value > 52)) {
        return 0; // Invalid card value
    } else if (card_value < 14) { // Card values 1-13 (Clubs)
        return 1;
    } else if (card_value < 27) { // Card values 14-26 (Diamonds)
        return 2;
    } else if (card_value < 40) { // Card values 27-39 (Hearts)
        return 3;
    } else { // Card values 40-52 (Spades)
        return 4;
    }
}

// Main function to demonstrate the usage of the card game functions
int main() {
    srand((unsigned int)time(NULL)); // Seed the random number generator once

    char deck[53]; // Declare a deck array: 52 slots for cards + 1 for the 'top' index

    printf("--- Initializing and Shuffling Deck ---\n");
    shuffle(deck);
    print_deck(deck);

    printf("\n--- Drawing a hand of 5 cards ---\n");
    char player_hand[5]; // Array to hold player's hand
    for (int i = 0; i < 5; ++i) {
        player_hand[i] = draw(deck); // Draw 5 cards
    }
    printf("Player's hand: ");
    print_hand(player_hand, 5); // Print the player's hand
    print_deck(deck); // Show the deck's state after drawing

    printf("\n--- Drawing another card ---\n");
    char another_card = draw(deck); // Draw one more card
    printf("Another card drawn: %s\n", card_names[another_card]);
    
    // Create an extended hand for printing purposes
    char player_hand_extended[6];
    for (int i = 0; i < 5; ++i) {
        player_hand_extended[i] = player_hand[i];
    }
    player_hand_extended[5] = another_card;
    printf("Player's hand (with another card): ");
    print_hand(player_hand_extended, 6);
    print_deck(deck); // Show the deck's state after drawing again

    printf("\n--- Checking card suits for drawn cards ---\n");
    for (int i = 0; i < 6; ++i) {
        printf("Card %s has suit %d\n", card_names[player_hand_extended[i]], get_suit(player_hand_extended[i]));
    }
    printf("Suit for invalid card value (0): %d\n", get_suit(0));
    printf("Suit for invalid card value (53): %d\n", get_suit(53));


    printf("\n--- Drawing all remaining cards to empty the deck ---\n");
    char remaining_cards[52]; // Buffer to hold all remaining cards
    int drawn_count = 0;
    while (!is_empty(deck)) { // Keep drawing until the deck is empty
        remaining_cards[drawn_count++] = draw(deck);
    }
    printf("All remaining cards drawn: ");
    print_hand(remaining_cards, drawn_count); // Print all drawn cards
    print_deck(deck); // Deck should now show as empty

    printf("\n--- Trying to draw from an empty deck (this should cause program termination) ---\n");
    // Uncomment the line below to test the error handling for drawing from an empty deck.
    // This call will print an error message and terminate the program.
    // draw(deck); 

    return 0; // Successful program execution
}