#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

// Dummy declarations for missing functions/types
typedef uint8_t byte;
typedef uint32_t undefined4;
typedef unsigned int uint;

// Assuming a structure for player data based on array indexing
// Player number: 0
// Player name: +1 (char array)
// Blackjack Score: +9
// Poker Score: +10
// Slots Score: +0xb
// Player Wallet Balance: +0xc
// Next player in list: +0xd
typedef struct Player {
    int number;       // Offset 0
    char name[32];    // Offset 1 (char name[32] for 0x1f length + null)
    // ... other fields to fill up to offset 9
    int dummy_1[7]; // (9 - 1 - sizeof(name)/sizeof(int))
    int blackjack_score; // Offset 9
    int poker_score;     // Offset 10
    int slots_score;     // Offset 0xb (11)
    int wallet_balance;  // Offset 0xc (12)
    struct Player *next; // Offset 0xd (13)
} Player;

// Forward declarations
void receive_fixed_input(void *buffer, size_t max_len, size_t actual_len);
int allocate(size_t size, int param_2, void **out_ptr);
uint prng();
void sprng();
int receive();
void _terminate(int status);
void blackjack(Player *p);
void poker(Player *p);
void slots(Player *p);

// Global variable (assuming it's defined elsewhere if not provided)
// Based on usage in programming_interface, payouts seems to be a fixed-size buffer
undefined4 payouts[14]; // Assuming 0x38 bytes / 4 bytes per undefined4 = 14 undefined4s

// Function: lookup_by_number
int *lookup_by_number(int param_1, int *param_2) {
    int *current_player_ptr = param_2;
    while (current_player_ptr != NULL && param_1 != *current_player_ptr) {
        // Assuming current_player_ptr[0xd] is the 'next' pointer
        current_player_ptr = (int *)current_player_ptr[13];
    }
    return current_player_ptr;
}

// Function: login_player_by_number
int login_player_by_number(Player *player_list_head) {
    char input_buffer[16]; // Sufficient for 10 chars + null
    printf("Enter Player Number:");
    receive_fixed_input(input_buffer, sizeof(input_buffer), 10);
    int player_number = atoi(input_buffer);
    int *found_player_ptr = lookup_by_number(player_number, (int *)player_list_head);

    if (found_player_ptr == NULL) {
        printf("Player not found\n");
        return 0; // Indicate no player found or error
    } else {
        // Assuming player name starts at offset +4 bytes from the player struct
        printf("Hello, %s!\n", (char *)(found_player_ptr + 1)); // (found_player_ptr + 1) is equivalent to address +4 bytes
        return (int)found_player_ptr; // Return the address of the found player
    }
}

// Function: print_player_info
void print_player_info(Player *param_1) {
    if (param_1 == NULL) {
        printf("Player does not exist\n");
    } else {
        printf("Player Name: %s\n", param_1->name);
        printf("Player Number: %d\n", param_1->number);
        printf("Blackjack Score: %d\n", param_1->blackjack_score);
        printf("Poker Score: %d\n", param_1->poker_score);
        printf("Slots Score: %d\n", param_1->slots_score);
        printf("Player Wallet Balance: %d\n", param_1->wallet_balance);
        printf("\n");
    }
}

// Function: register_player
Player *register_player(Player *param_1) {
    Player *new_player = NULL;
    if (allocate(sizeof(Player), 0, (void **)&new_player) == 0) {
        memset(new_player, 0, sizeof(Player));
        int iVar2;
        uint uVar3;
        do {
            uVar3 = prng();
            new_player->number = uVar3 & 0x7fffffff;
            iVar2 = (int)lookup_by_number(new_player->number, (int *)param_1);
        } while (iVar2 != 0);

        printf("Enter your name:");
        receive_fixed_input(new_player->name, sizeof(new_player->name), 0x1f); // 0x1f = 31 chars + null

        printf("How much cash do you want to spend?:");
        char input_buffer[16]; // Sufficient for 10 chars + null
        receive_fixed_input(input_buffer, sizeof(input_buffer), 10);
        new_player->wallet_balance = atoi(input_buffer);

        if (new_player->wallet_balance > 1000 || new_player->wallet_balance < 0) {
            new_player->wallet_balance = 1000;
        }

        print_player_info(new_player);
        new_player->next = param_1; // Add to the beginning of the list
        return new_player;
    } else {
        printf("Error allocating memory for new player\n");
        return param_1; // Return original head if allocation fails
    }
}

// Function: print_all_players
Player *print_all_players(Player *param_1) {
    Player *current_player = param_1;
    while (current_player != NULL) {
        print_player_info(current_player);
        current_player = current_player->next;
    }
    return current_player; // Returns NULL if list is empty or reaches end
}

// Function: lookup
undefined4 lookup(char param_1) {
    switch (param_1) {
        case '!': return 0;
        case '#': return 2;
        case '$': return 3;
        case '%': return 4;
        case '&': return 6;
        case '*': return 0xf;
        case '+': return 7;
        case ':': return 9;
        case '<': return 0xb;
        case '=': return 8;
        case '>': return 10;
        case '?': return 0xd;
        case '@': return 1;
        case '^': return 5;
        case '|': return 0xc;
        case '~': return 0xe;
        default:
            _terminate(1);
            return 0; // Should not be reached
    }
}

// Function: decode
void decode(char *param_1, byte *param_2, int param_3) {
    char *input_ptr = param_1;
    byte *output_ptr = param_2;
    char *end_ptr = param_1 + param_3;

    while (input_ptr < end_ptr) {
        byte decoded_byte;
        if (*input_ptr == '`') {
            input_ptr++;
            decoded_byte = 0;
        } else {
            decoded_byte = lookup(*input_ptr);
            input_ptr++;
            decoded_byte |= (byte)(lookup(*input_ptr) << 4);
            input_ptr++;
        }
        *output_ptr = decoded_byte;
        output_ptr++;
    }
}

// Function: programming_interface
void programming_interface(void) {
    char local_buffer[56]; // Corresponds to local_5c
    int input_length;      // Corresponds to local_20
    int received_bytes;    // Corresponds to local_1c

    // Simplified stack operations, focusing on logic
    // The original code uses stack manipulation for arguments and return addresses,
    // which is typical of decompiled code. We'll use direct variable access.

    // receive input_length
    if (receive(&input_length, 4, 0) != 0 || input_length == 0) {
        _terminate(1);
    }

    if (input_length < 1 || input_length > 0x70) {
        _terminate(1);
    }

    char *allocated_buffer = NULL;
    if (allocate(input_length, 0, (void **)&allocated_buffer) != 0 || allocated_buffer == NULL) {
        _terminate(1);
    }

    int current_offset = 0;
    while (current_offset < input_length) {
        if (receive(&received_bytes, 4, 0) != 0) {
            _terminate(1);
        }
        memcpy(allocated_buffer + current_offset, &received_bytes, sizeof(int)); // Assuming receive gets an int part of the data
        current_offset += received_bytes; // Assuming received_bytes indicates the actual amount read
    }

    decode(allocated_buffer, (byte *)local_buffer, input_length);
    memcpy(payouts, local_buffer, sizeof(payouts)); // 0x38 bytes

    // Deallocate the buffer
    // Assuming deallocate takes the pointer and size
    // The original code has local_20 for size, local_24 for pointer
    // deallocate(allocated_buffer, input_length); // Placeholder, actual deallocate might be different
    free(allocated_buffer); // Use free for dynamically allocated memory
}

// Function: main
int main(void) {
    char player_name[64]; // Corresponds to local_78
    char menu_choice;     // Corresponds to local_7d
    Player *current_player = NULL; // Corresponds to local_30, points to the currently logged in player
    Player *player_list_head = NULL; // Corresponds to local_34, head of the linked list of players

    printf("Enter your name:");
    receive_fixed_input(player_name, sizeof(player_name), 0x53); // 0x53 is likely max_len, not actual_len

    // Remove trailing newline if present
    size_t name_len = strlen(player_name);
    if (name_len > 0 && player_name[name_len - 1] == '\n') {
        player_name[name_len - 1] = '\0';
    }

    printf("Hello, %s!\n", player_name);

    // Calculate initial seed values
    uint seed_val1 = 0; // Corresponds to local_28
    uint seed_val2 = 0; // Corresponds to local_24
    for (size_t i = 0; i < strlen(player_name); ++i) {
        uint temp_val1 = seed_val2 << 8 | seed_val1 >> 0x18;
        uint temp_val2 = seed_val1 << 8;
        uint temp_val3 = seed_val2 >> 0x18;
        seed_val1 = temp_val3 ^ (uint)player_name[i] | temp_val2;
        seed_val2 = ((uint)player_name[i] >> 0x1f) | temp_val1;
    }
    sprng(); // Assuming sprng uses some global state, not passed arguments

    // Check for "Access Granted" condition
    if ((seed_val1 ^ 0x32360652 | seed_val2 ^ 0x64623130) == 0) {
        printf("Access Granted\n");
        programming_interface();
    }

    // Main menu loop
    while (true) {
        // Check for "You broke the bank!" condition
        if (current_player != NULL && current_player->wallet_balance < 0x1389) { // 0x1389 = 5001
            printf("You broke the bank!\n");
            break; // Exit loop to "Goodbye"
        }

        printf("\nWelcome to the Casino!\n");
        printf("1. Play Blackjack\n");
        printf("2. Play Poker\n");
        printf("3. Play Slots\n");
        printf("4. Enter a Player's Club Number\n");
        printf("5. Register a new Player\n");
        printf("6. Show Player Status\n");
        printf("7. Show All Players\n");
        printf("8. Exit\n");
        printf("Select a number:");

        receive_fixed_input(&menu_choice, 1, 1); // Read a single character

        switch (menu_choice) {
            case '1':
                printf("Play Blackjack\n");
                blackjack(current_player);
                break;
            case '2':
                printf("Play Poker\n");
                poker(current_player);
                break;
            case '3':
                printf("Play Slots\n");
                slots(current_player);
                break;
            case '4':
                current_player = (Player *)login_player_by_number(player_list_head);
                break;
            case '5':
                printf("Register a new Player\n");
                player_list_head = register_player(player_list_head);
                // If this is the first player, make them the current player
                if (current_player == NULL && player_list_head != NULL) {
                    current_player = player_list_head;
                }
                break;
            case '6':
                print_player_info(current_player);
                break;
            case '7':
                printf("Show All Players\n");
                print_all_players(player_list_head);
                break;
            case '8':
                printf("You don't really want to leave, do you?\n");
                receive_fixed_input(&menu_choice, 1, 1);
                if (menu_choice == 'y') {
                    if (current_player == NULL) {
                        printf("Goodbye, stranger\n");
                        _terminate(0);
                    }
                    if (current_player->blackjack_score == 0) {
                        printf("But you didn't even play blackjack :(\n");
                        _terminate(0);
                    }
                    if (current_player->poker_score == 0) {
                        printf("But you didn't even play poker :(\n");
                        _terminate(0);
                    }
                    if (current_player->slots_score == 0) {
                        printf("But you didn't even play slots :(\n");
                        _terminate(0);
                    }
                    printf("Okay, you've had enough ;)\n");
                    goto exit_program; // Use goto for the single exit point as requested by the original code's structure
                }
                break;
            default:
                printf("Invalid choice. Please try again.\n");
                break;
        }
    }

exit_program:
    printf("Goodbye\n");
    return 0;
}

// --- Dummy implementations for external functions ---
void receive_fixed_input(void *buffer, size_t max_len, size_t actual_len) {
    if (fgets(buffer, max_len, stdin) == NULL) {
        fprintf(stderr, "Error reading input.\n");
        exit(1);
    }
    // Remove trailing newline if present, based on actual_len
    char *newline_pos = strchr(buffer, '\n');
    if (newline_pos != NULL) {
        *newline_pos = '\0';
    } else {
        // If no newline, input might have exceeded buffer, clear stdin
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
    }
}

int allocate(size_t size, int param_2, void **out_ptr) {
    *out_ptr = malloc(size);
    return (*out_ptr == NULL) ? 1 : 0;
}

void deallocate(void *ptr) {
    free(ptr);
}

uint prng() {
    return rand(); // Simple PRNG
}

void sprng() {
    // Dummy, in a real scenario it would seed the PRNG
    srand(time(NULL));
}

int receive(void *buffer, size_t size, int flags) {
    // Dummy implementation for receive, assuming it reads from stdin for simplicity
    // In a real scenario, this might read from a network socket or specific device.
    size_t bytes_read = fread(buffer, 1, size, stdin);
    if (bytes_read == 0 && feof(stdin)) {
        return -1; // EOF
    }
    if (bytes_read != size) {
        // Handle partial read or error
        return 1; // Indicate error
    }
    return 0; // Success
}

void _terminate(int status) {
    fprintf(stderr, "Program terminated with status %d.\n", status);
    exit(status);
}

void blackjack(Player *p) {
    printf("Blackjack game logic goes here. Player: %s\n", p ? p->name : "None");
    if (p) p->blackjack_score += 10; // Dummy score update
}

void poker(Player *p) {
    printf("Poker game logic goes here. Player: %s\n", p ? p->name : "None");
    if (p) p->poker_score += 20; // Dummy score update
}

void slots(Player *p) {
    printf("Slots game logic goes here. Player: %s\n", p ? p->name : "None");
    if (p) p->slots_score += 5; // Dummy score update
}