#include <stdbool.h> // For bool
#include <stdlib.h>  // For calloc, free
#include <string.h>  // For strncpy
#include <stdio.h>   // For printf (in main for testing)
#include <stdint.h>  // For uint8_t, uint32_t

// --- Dummy helper functions for compilation ---
// In a real application, these would be implemented elsewhere.
// Assuming card_id = (suit_char << 8) | rank_char for simple encoding
int create_card(char rank, char suit) {
    if (rank == '\0' || suit == '\0') {
        return 0; // Invalid card
    }
    // Encodes suit in the higher byte and rank in the lower byte
    return (int)((uint8_t)suit << 8 | (uint8_t)rank);
}

int is_valid_rank(char rank) {
    return (rank >= '2' && rank <= '9') ||
           (rank == 'T' || rank == 'J' || rank == 'Q' || rank == 'K' || rank == 'A');
}

// --- Fixed functions ---

// Function: parse_xml_card
// Parses an XML card string (e.g., "EGQ'FA''") into a card ID.
// param_1: Input string
// param_2: Pointer to store the parsed card ID
// Returns: Length of the parsed card string (8) on success, or a negative error code on failure.
int parse_xml_card(char *param_1, int *param_2) {
    char rank = '\0';
    char suit = '\0';

    // Check fixed characters and overall format.
    // Expected format: E<Ind1><Val1>'<Ind2><Val2>''
    // Length must be at least 8 characters.
    if (param_1[0] != 'E' ||
        param_1[3] != '\'' ||
        param_1[6] != '\'' ||
        param_1[7] != '\'') {
        return -62; // 0xffffffc2 (Error: Malformed card string)
    }

    // Determine which character is rank and which is suit based on indicators
    if (param_1[1] == 'G' && param_1[4] == 'F') {
        // Format: EG<suit>'F<rank>''
        suit = param_1[2];
        rank = param_1[5];
    } else if (param_1[1] == 'F' && param_1[4] == 'G') {
        // Format: EF<rank>'G<suit>''
        rank = param_1[2];
        suit = param_1[5];
    } else {
        // Invalid indicators or unexpected combination
        return -62; // 0xffffffc2 (Error: Invalid card indicators)
    }

    int card_id = create_card(rank, suit);
    *param_2 = card_id;

    if (*param_2 == 0) {
        return -42; // 0xffffffd6 (Error: Invalid card created)
    }
    return 8; // Length of the parsed card string
}

// Function: gen_xml_card
// Generates an XML card string from a card ID.
// param_1: Output buffer to write the XML string
// param_2: The card ID (int where (suit_char << 8) | rank_char)
// Returns: Length of the generated XML string (8).
int gen_xml_card(uint8_t *param_1, int param_2) {
    uint8_t rank = (uint8_t)(param_2 & 0xFF);
    uint8_t suit = (uint8_t)((param_2 >> 8) & 0xFF);

    param_1[0] = 'E';
    param_1[1] = 'G'; // Indicator for suit
    param_1[2] = suit;
    param_1[3] = '\'';
    param_1[4] = 'F'; // Indicator for rank
    param_1[5] = rank;
    param_1[6] = '\'';
    param_1[7] = '\'';
    return 8;
}

// Function: parse_xml_player_name
// Parses an XML player name string (e.g., "(BCVAlice'''')").
// param_1: Input string
// param_2: Pointer to store a dynamically allocated string with the player's name
// param_3: Total length of the input string
// Returns: 0 on success, or a negative error code on failure.
int parse_xml_player_name(char *param_1, char **param_2, uint8_t param_3) {
    // Minimum length for "(BCVname'''')" is 9 chars (4 prefix + 1 name + 4 suffix)
    if (param_3 < 9) {
        return -62; // 0xffffffc2
    }
    
    if (param_1[0] == '(' && param_1[1] == 'B' && param_1[2] == 'C' && param_1[3] == 'V') {
        size_t name_len = param_3 - 8; // (BCV (4) + '''' (4))
        char *name_dest = (char *)calloc(name_len + 1, sizeof(char)); // +1 for null terminator
        if (name_dest == NULL) {
            return -1; // Allocation failure
        }
        strncpy(name_dest, param_1 + 4, name_len);
        name_dest[name_len] = '\0'; // Ensure null termination
        *param_2 = name_dest;

        if (param_1[param_3 - 4] == '\'' && param_1[param_3 - 3] == '\'' &&
            param_1[param_3 - 2] == '\'' && param_1[param_3 - 1] == ')') {
            return 0; // Success
        } else {
            free(name_dest); // Free allocated memory on parse error
            *param_2 = NULL;
            return -62; // 0xffffffc2
        }
    } else {
        return -62; // 0xffffffc2
    }
}

// Function: parse_xml_cards
// Parses an XML string containing multiple cards (e.g., "(DEGA'F2''EGS'F3''')").
// param_1: Input string
// param_2: Pointer to an array of integers to store the parsed card IDs
// Returns: Number of cards parsed on success, or a negative error code on failure.
int parse_xml_cards(char *param_1, int *param_2) {
    int card_id = 0;
    int parse_result;
    int card_count = 0;
    char *current_pos = param_1;

    if (*current_pos == '(') {
        if (current_pos[1] == 'D') {
            current_pos += 2; // Advance past "(D"
            while (*current_pos != '\'') { // Loop until the closing single quote for cards
                parse_result = parse_xml_card(current_pos, &card_id);
                if (parse_result < 0) {
                    return parse_result; // Error from parse_xml_card
                }
                current_pos += parse_result; // Advance by the length of the parsed card
                param_2[card_count] = card_id;
                card_count++;
            }
            // After loop, current_pos points to the first '\''
            if (*current_pos == '\'') {
                if (current_pos[1] != ')') {
                    return -62; // 0xffffffc2 - Expecting ')' after cards
                }
            } else {
                return -62; // 0xffffffc2 - Missing closing '\''
            }
        } else {
            return -62; // 0xffffffc2 - Expected 'D'
        }
    } else {
        return -62; // 0xffffffc2 - Expected '('
    }
    return card_count; // Return number of cards parsed
}

// Function: gen_xml_cards
// Generates an XML string representing multiple cards.
// param_1: Output buffer to write the XML string
// param_2: Array of card IDs (int where (suit_char << 8) | rank_char)
// param_3: Number of cards in the array
// Returns: Total length of the generated XML string.
int gen_xml_cards(uint8_t *param_1, int *param_2, uint8_t param_3) {
    int current_offset = 2; // Start after "(D"
    param_1[0] = '(';
    param_1[1] = 'D';

    for (uint8_t i = 0; i < param_3; ++i) {
        int card_len = gen_xml_card(param_1 + current_offset, param_2[i]);
        current_offset += card_len;
    }
    param_1[current_offset] = '\'';
    param_1[current_offset + 1] = ')';
    return current_offset + 2; // Total length
}

// Function: parse_xml_draw
// Parses an XML "draw" message (e.g., "(P'')").
// param_1: Input string
// Returns: 0 on success, or a negative error code on failure.
int parse_xml_draw(char *param_1) {
    if (param_1[0] == '(' && param_1[1] == 'P' && param_1[2] == '\'' && param_1[3] == ')') {
        return 0; // Success
    }
    return -62; // 0xffffffc2
}

// Function: parse_xml_go_fish
// Parses an XML "go fish" message (e.g., "(Q<rank>'')").
// param_1: Input string
// Returns: The rank character on success, or a negative error code on failure.
int parse_xml_go_fish(char *param_1) {
    if (param_1[0] == '(' && param_1[1] == 'Q' && param_1[3] == '\'' && param_1[4] == ')') {
        return (int)param_1[2]; // Return the rank character
    }
    return -62; // 0xffffffc2
}

// Function: gen_xml_go_fish
// Generates an XML "go fish" message.
// param_1: Output buffer to write the XML string
// param_2: The rank character for the go fish request
// Returns: Length of the generated XML string (5).
int gen_xml_go_fish(uint8_t *param_1, uint8_t param_2) {
    param_1[0] = '(';
    param_1[1] = 'Q';
    param_1[2] = param_2; // Rank
    param_1[3] = '\'';
    param_1[4] = ')';
    return 5;
}

// Function: parse_xml_ask
// Parses an XML "ask" message (e.g., "(SG<rank>'''')").
// param_1: Input string
// Returns: The rank character on success, or a negative error code on failure.
int parse_xml_ask(char *param_1) {
    if (param_1[0] == '(' && param_1[1] == 'S' && param_1[2] == 'G') {
        char rank_char = param_1[3];
        if (is_valid_rank(rank_char) == 0) {
            return -44; // 0xffffffd4 (Error: Invalid rank)
        }
        if (param_1[4] == '\'' && param_1[5] == '\'' && param_1[6] == ')') {
            return (int)rank_char;
        }
    }
    return -62; // 0xffffffc2
}

// Function: gen_xml_ask
// Generates an XML "ask" message.
// param_1: Output buffer to write the XML string
// param_2: The rank character being asked for
// Returns: Length of the generated XML string (7).
int gen_xml_ask(uint8_t *param_1, uint8_t param_2) {
    param_1[0] = '(';
    param_1[1] = 'S';
    param_1[2] = 'G';
    param_1[3] = param_2; // Rank
    param_1[4] = '\'';
    param_1[5] = '\'';
    param_1[6] = ')';
    return 7;
}

// Function: parse_xml_fishing
// Parses an XML "fishing" message (e.g., "(R'')").
// param_1: Input string
// Returns: 0 on success, or a negative error code on failure.
int parse_xml_fishing(char *param_1) {
    if (param_1[0] == '(' && param_1[1] == 'R' && param_1[2] == '\'' && param_1[3] == ')') {
        return 0; // Success
    }
    return -62; // 0xffffffc2
}

// Function: parse_xml_books
// Parses an XML "books" message (e.g., "(T<count>'')").
// param_1: Input string
// Returns: The book count character on success, or a negative error code on failure.
int parse_xml_books(char *param_1) {
    if (param_1[0] == '(' && param_1[1] == 'T') {
        if (param_1[3] == '\'' && param_1[4] == ')') {
            return (int)param_1[2]; // Return the book count character
        }
    }
    return -62; // 0xffffffc2
}

// Function: gen_xml_books
// Generates an XML "books" message.
// param_1: Output buffer to write the XML string
// param_2: The book count character
// Returns: Length of the generated XML string (5).
int gen_xml_books(uint8_t *param_1, uint8_t param_2) {
    param_1[0] = '(';
    param_1[1] = 'T';
    param_1[2] = param_2; // Book count or rank
    param_1[3] = '\'';
    param_1[4] = ')';
    return 5;
}

// Function: gen_xml_turn
// Generates an XML "turn" message.
// param_1: Output buffer to write the XML string
// param_2: Player ID character whose turn it is
// Returns: Length of the generated XML string (9).
int gen_xml_turn(uint8_t *param_1, uint8_t param_2) {
    param_1[0] = '(';
    param_1[1] = 'U'; // 'U' as an indicator for Turn
    param_1[2] = 'C';
    param_1[3] = 'I';
    param_1[4] = param_2; // Player ID
    param_1[5] = '\'';
    param_1[6] = '\'';
    param_1[7] = '\'';
    param_1[8] = ')';
    return 9;
}

// Function: gen_xml_final_results
// Generates an XML "final results" message for two players.
// param_1: Output buffer to write the XML string
// param_2: Books for player 0
// param_3: Books for player 1
// Returns: Length of the generated XML string (20).
int gen_xml_final_results(uint8_t *param_1, uint8_t param_2, uint8_t param_3) {
    param_1[0] = '(';
    param_1[1] = 'B';
    param_1[2] = 'C';
    param_1[3] = 'I';
    param_1[4] = '0'; // Player 0 ID
    param_1[5] = '\'';
    param_1[6] = 'T';
    param_1[7] = param_2; // Player 0 books
    param_1[8] = '\'';
    param_1[9] = '\'';
    param_1[10] = 'C';
    param_1[11] = 'I';
    param_1[12] = '1'; // Player 1 ID
    param_1[13] = '\'';
    param_1[14] = 'T';
    param_1[15] = param_3; // Player 1 books
    param_1[16] = '\'';
    param_1[17] = '\'';
    param_1[18] = '\'';
    param_1[19] = ')';
    return 20; // 0x14
}

// Function: gen_xml_error
// Generates an XML "error" message.
// param_1: Output buffer to write the XML string
// param_2: Error code character
// Returns: Length of the generated XML string (5).
int gen_xml_error(uint8_t *param_1, uint8_t param_2) {
    param_1[0] = '(';
    param_1[1] = 'H';
    param_1[2] = param_2; // Error code
    param_1[3] = '\'';
    param_1[4] = ')';
    return 5;
}

// --- Main function for testing ---
int main() {
    printf("--- XML Parser/Generator Test ---\n\n");

    // Test parse_xml_card
    char card_str_1[] = "EGQ'FA''"; // Suit Q, Rank A
    char card_str_2[] = "EF2'GT''"; // Rank 2, Suit T (Ten)
    int card_id;
    int parse_len;

    printf("Testing parse_xml_card:\n");
    parse_len = parse_xml_card(card_str_1, &card_id);
    if (parse_len > 0) {
        printf("Parsed '%s': Card ID = 0x%X (Rank: %c, Suit: %c), Length = %d\n", card_str_1, card_id, (char)(card_id & 0xFF), (char)((card_id >> 8) & 0xFF), parse_len);
    } else {
        printf("Failed to parse '%s', Error: %d\n", card_str_1, parse_len);
    }

    parse_len = parse_xml_card(card_str_2, &card_id);
    if (parse_len > 0) {
        printf("Parsed '%s': Card ID = 0x%X (Rank: %c, Suit: %c), Length = %d\n", card_str_2, (char)(card_id & 0xFF), (char)((card_id >> 8) & 0xFF), card_id, parse_len);
    } else {
        printf("Failed to parse '%s', Error: %d\n", card_str_2, parse_len);
    }

    // Test gen_xml_card
    uint8_t buffer_gen_card[9];
    int generated_len;
    int card_id_to_gen_1 = create_card('A', 'Q'); // Rank A, Suit Q
    int card_id_to_gen_2 = create_card('2', 'T'); // Rank 2, Suit T

    printf("\nTesting gen_xml_card:\n");
    generated_len = gen_xml_card(buffer_gen_card, card_id_to_gen_1);
    buffer_gen_card[generated_len] = '\0';
    printf("Generated card from ID 0x%X: '%s', Length = %d\n", card_id_to_gen_1, buffer_gen_card, generated_len);

    generated_len = gen_xml_card(buffer_gen_card, card_id_to_gen_2);
    buffer_gen_card[generated_len] = '\0';
    printf("Generated card from ID 0x%X: '%s', Length = %d\n", card_id_to_gen_2, buffer_gen_card, generated_len);

    // Test parse_xml_player_name
    char name_xml[] = "(BCVAlice''''"; // Length 13, name_len = 5
    char *player_name = NULL;
    printf("\nTesting parse_xml_player_name:\n");
    int name_parse_result = parse_xml_player_name(name_xml, &player_name, sizeof(name_xml) - 1);
    if (name_parse_result == 0) {
        printf("Parsed player name: '%s'\n", player_name);
        free(player_name);
    } else {
        printf("Failed to parse player name, Error: %d\n", name_parse_result);
    }

    // Test gen_xml_cards and parse_xml_cards
    printf("\nTesting gen_xml_cards and parse_xml_cards:\n");
    int cards_to_gen[] = {create_card('K', 'H'), create_card('Q', 'D'), create_card('J', 'C')};
    uint8_t num_cards = sizeof(cards_to_gen) / sizeof(cards_to_gen[0]);
    uint8_t cards_buffer[100]; // Sufficiently large buffer

    generated_len = gen_xml_cards(cards_buffer, cards_to_gen, num_cards);
    cards_buffer[generated_len] = '\0';
    printf("Generated cards XML: '%s', Length = %d\n", cards_buffer, generated_len);

    int parsed_card_ids[10];
    int parsed_count = parse_xml_cards((char*)cards_buffer, parsed_card_ids);
    if (parsed_count > 0) {
        printf("Parsed %d cards:\n", parsed_count);
        for (int i = 0; i < parsed_count; ++i) {
            printf("  Card %d: ID = 0x%X (Rank: %c, Suit: %c)\n", i, parsed_card_ids[i], (char)(parsed_card_ids[i] & 0xFF), (char)((parsed_card_ids[i] >> 8) & 0xFF));
        }
    } else {
        printf("Failed to parse cards, Error: %d\n", parsed_count);
    }

    // Test gen_xml_error
    uint8_t error_buffer[10];
    generated_len = gen_xml_error(error_buffer, 'E'); // Using 'E' as an example error code
    error_buffer[generated_len] = '\0';
    printf("\nGenerated error XML for code 'E': '%s', Length = %d\n", error_buffer, generated_len);

    // Test gen_xml_turn
    uint8_t turn_buffer[10];
    generated_len = gen_xml_turn(turn_buffer, '0'); // Player 0's turn
    turn_buffer[generated_len] = '\0';
    printf("Generated turn XML for player '0': '%s', Length = %d\n", turn_buffer, generated_len);

    // Test gen_xml_final_results
    uint8_t final_results_buffer[30];
    generated_len = gen_xml_final_results(final_results_buffer, '5', '3'); // Player 0 has 5 books, Player 1 has 3 books
    final_results_buffer[generated_len] = '\0';
    printf("Generated final results XML (P0:5, P1:3): '%s', Length = %d\n", final_results_buffer, generated_len);

    return 0;
}