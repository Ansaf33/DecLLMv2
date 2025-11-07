#include <stdio.h>   // For printf (in main)
#include <stdlib.h>  // For calloc, free
#include <string.h>  // For strncpy, strlen
#include <stdbool.h> // For bool

// Define common error codes
#define PARSE_ERROR_GENERIC -62 // 0xffffffc2
#define PARSE_ERROR_CARD_CREATE_FAIL -42 // 0xffffffd6
#define PARSE_ERROR_INVALID_RANK -44 // 0xffffffd4 (original was -0x2c)

// --- Dummy functions for compilation ---
// Assumes card_id encodes suit in lower 8 bits, rank in next 8 bits.
// For example, 0x5448 could be 'H' (suit) and 'T' (rank)
int create_card(char suit, char rank) {
    if (suit == '\0' || rank == '\0') {
        return 0; // Invalid card
    }
    // Store rank in higher byte, suit in lower byte
    return ((int)rank << 8) | (unsigned char)suit;
}

// Dummy function for is_valid_rank
int is_valid_rank(char rank) {
    // Example valid ranks: '2'-'9', 'T', 'J', 'Q', 'K', 'A'
    if ((rank >= '2' && rank <= '9') || rank == 'T' || rank == 'J' ||
        rank == 'Q' || rank == 'K' || rank == 'A') {
        return 1; // Valid
    }
    return 0; // Invalid
}
// --- End Dummy functions ---


// Function: parse_xml_card
// Parses a card XML snippet like "EGX'F Y''" or "EFX'G Y''".
// Returns: length of parsed XML snippet (8) on success, or a negative error code.
int parse_xml_card(const char *param_1, int *param_2) {
    char rank_char = '\0';
    char suit_char = '\0';
    const char *current_ptr = param_1;

    if (*current_ptr != 'E') {
        return PARSE_ERROR_GENERIC;
    }
    current_ptr++;

    // First part: E[G|F][char]'
    if (*current_ptr == 'G') { // Expecting Rank
        rank_char = current_ptr[1];
        if (current_ptr[2] != '\'') {
            return PARSE_ERROR_GENERIC;
        }
        current_ptr += 3; // Advance past G[char]'
    } else if (*current_ptr == 'F') { // Expecting Suit
        suit_char = current_ptr[1];
        if (current_ptr[2] != '\'') {
            return PARSE_ERROR_GENERIC;
        }
        current_ptr += 3; // Advance past F[char]'
    } else {
        return PARSE_ERROR_GENERIC;
    }

    // Second part: [G|F][char]'
    if (rank_char != '\0') { // If rank was set, expect Suit
        if (*current_ptr != 'F') {
            return PARSE_ERROR_GENERIC;
        }
        suit_char = current_ptr[1];
    } else { // If suit was set, expect Rank
        if (*current_ptr != 'G') {
            return PARSE_ERROR_GENERIC;
        }
        rank_char = current_ptr[1];
    }
    if (current_ptr[2] != '\'') {
        return PARSE_ERROR_GENERIC;
    }
    current_ptr += 3; // Advance past [G|F][char]'

    // Check for final quote
    if (*current_ptr == '\'') {
        int card_id = create_card(suit_char, rank_char);
        *param_2 = card_id;
        if (card_id == 0) {
            return PARSE_ERROR_CARD_CREATE_FAIL; // Card creation failed
        }
        return 8; // Length of "E[X][Y]'[Z][W]''" is 8 characters.
    }
    return PARSE_ERROR_GENERIC; // Missing final quote
}

// Function: gen_xml_card
// Generates a card XML snippet from a card ID.
// Assumes card_id encodes suit in lower 8 bits, rank in next 8 bits.
// Returns: length of generated XML snippet (8).
int gen_xml_card(char *buffer, int card_id) {
    char suit_char = (char)(card_id & 0xFF);
    char rank_char = (char)((card_id >> 8) & 0xFF);

    buffer[0] = 'E'; // 0x45
    buffer[1] = 'F'; // 0x46 (Suit first)
    buffer[2] = suit_char;
    buffer[3] = '\''; // 0x27
    buffer[4] = 'G'; // 0x47 (Rank second)
    buffer[5] = rank_char;
    buffer[6] = '\''; // 0x27
    buffer[7] = '\''; // 0x27
    return 8; // Total length
}

// Function: parse_xml_player_name
// Parses a player name XML snippet like "(BCVPLAYER1''')".
// Returns: 0 on success, or a negative error code.
int parse_xml_player_name(const char *param_1, char **param_2, unsigned char param_3) {
    if (param_3 < 9) { // Minimum length for "(BCVname''')" is 9
        return PARSE_ERROR_GENERIC;
    }
    if (!((*param_1 == '(') && (param_1[1] == 'B') && (param_1[2] == 'C') && (param_1[3] == 'V'))) {
        return PARSE_ERROR_GENERIC;
    }

    // Allocate memory for the player name (param_3 - 8 characters + null terminator)
    // param_3 is total length, 4 for "(BCV", 4 for "''')" = 8 chars to remove
    char *name_dest = (char *)calloc(param_3 - 8 + 1, sizeof(char)); // +1 for null terminator
    if (name_dest == NULL) {
        return -1; // Allocation failed
    }

    strncpy(name_dest, param_1 + 4, param_3 - 8);
    name_dest[param_3 - 8] = '\0'; // Ensure null termination
    *param_2 = name_dest;

    if (!((param_1[param_3 - 4] == '\'') && (param_1[param_3 - 3] == '\'') &&
          (param_1[param_3 - 2] == '\'') && (param_1[param_3 - 1] == ')'))) {
        free(name_dest); // Free memory on error
        *param_2 = NULL;
        return PARSE_ERROR_GENERIC;
    }
    return 0; // Success
}

// Function: parse_xml_cards
// Parses a list of card XML snippets like "(DCARD1CARD2')".
// Returns: number of cards parsed on success, or a negative error code.
int parse_xml_cards(const char *param_1, int *card_ids_array) {
    int num_cards_parsed = 0;
    const char *current_ptr = param_1;

    if (*current_ptr != '(') return PARSE_ERROR_GENERIC;
    current_ptr++; // Consume '('

    if (*current_ptr != 'D') return PARSE_ERROR_GENERIC;
    current_ptr++; // Consume 'D'

    while (*current_ptr != '\'') {
        int card_id;
        int parse_result = parse_xml_card(current_ptr, &card_id);
        if (parse_result < 0) {
            return parse_result;
        }
        current_ptr += parse_result; // Advance by the length of the parsed card XML
        card_ids_array[num_cards_parsed] = card_id;
        num_cards_parsed++;
    }

    // After the loop, current_ptr points to the first quote.
    if (*current_ptr == '\'') {
        if (current_ptr[1] != ')') {
            return PARSE_ERROR_GENERIC;
        }
    } else { // Should not happen if loop condition is *current_ptr != '\''
        return PARSE_ERROR_GENERIC;
    }
    return num_cards_parsed;
}

// Function: gen_xml_cards
// Generates an XML snippet for a list of cards.
// Returns: total length of generated XML snippet.
char gen_xml_cards(char *buffer, const int *card_ids, unsigned char num_cards_to_gen) {
    unsigned char current_length = 0;

    buffer[current_length++] = '('; // 0x28
    buffer[current_length++] = 'D'; // 0x44

    for (unsigned char i = 0; i < num_cards_to_gen; ++i) {
        current_length += gen_xml_card(buffer + current_length, card_ids[i]);
    }

    buffer[current_length++] = '\''; // 0x27
    buffer[current_length++] = ')'; // 0x29
    return current_length; // Total length
}

// Function: parse_xml_draw
// Parses a draw action XML snippet like "(P')".
// Returns: 0 on success, or a negative error code.
int parse_xml_draw(const char *param_1) {
    if (!((*param_1 == '(') && (param_1[1] == 'P') && (param_1[2] == '\'') && (param_1[3] == ')'))) {
        return PARSE_ERROR_GENERIC;
    }
    return 0;
}

// Function: parse_xml_go_fish
// Parses a Go Fish action XML snippet like "(QX')".
// Returns: integer value of char at param_1[2] on success, or a negative error code.
int parse_xml_go_fish(const char *param_1) {
    if (!((*param_1 == '(') && (param_1[1] == 'Q') && (param_1[3] == '\'') && (param_1[4] == ')'))) {
        return PARSE_ERROR_GENERIC;
    }
    return (int)param_1[2];
}

// Function: gen_xml_go_fish
// Generates a Go Fish action XML snippet.
// Returns: length of generated XML snippet (5).
int gen_xml_go_fish(char *buffer, char value) {
    buffer[0] = '('; // 0x28
    buffer[1] = 'Q'; // 0x51
    buffer[2] = value;
    buffer[3] = '\''; // 0x27
    buffer[4] = ')'; // 0x29
    return 5;
}

// Function: parse_xml_ask
// Parses an Ask action XML snippet like "(SGK''')".
// Returns: integer value of char at param_1[3] (rank) on success, or a negative error code.
int parse_xml_ask(const char *param_1) {
    if (!((*param_1 == '(') && (param_1[1] == 'S') && (param_1[2] == 'G'))) {
        return PARSE_ERROR_GENERIC;
    }
    char rank_char = param_1[3];
    if (is_valid_rank(rank_char) == 0) {
        return PARSE_ERROR_INVALID_RANK;
    }
    if (!((param_1[4] == '\'') && (param_1[5] == '\'') && (param_1[6] == ')'))) {
        return PARSE_ERROR_GENERIC;
    }
    return (int)rank_char;
}

// Function: gen_xml_ask
// Generates an Ask action XML snippet.
// Returns: length of generated XML snippet (7).
int gen_xml_ask(char *buffer, char rank_char) {
    buffer[0] = '('; // 0x28
    buffer[1] = 'S'; // 0x53
    buffer[2] = 'G'; // 0x47
    buffer[3] = rank_char;
    buffer[4] = '\''; // 0x27
    buffer[5] = '\''; // 0x27
    buffer[6] = ')'; // 0x29
    return 7;
}

// Function: parse_xml_fishing
// Parses a Fishing action XML snippet like "(R')".
// Returns: 0 on success, or a negative error code.
int parse_xml_fishing(const char *param_1) {
    if (!((*param_1 == '(') && (param_1[1] == 'R') && (param_1[2] == '\'') && (param_1[3] == ')'))) {
        return PARSE_ERROR_GENERIC;
    }
    return 0;
}

// Function: parse_xml_books
// Parses a books count XML snippet like "(TX')".
// Returns: integer value of char at param_1[2] on success, or a negative error code.
unsigned int parse_xml_books(const char *param_1) {
    if (!((*param_1 == '(') && (param_1[1] == 'T'))) {
        return PARSE_ERROR_GENERIC;
    }
    if (!((param_1[3] == '\'') && (param_1[4] == ')'))) {
        return PARSE_ERROR_GENERIC;
    }
    return (unsigned int)(unsigned char)param_1[2];
}

// Function: gen_xml_books
// Generates a books count XML snippet.
// Returns: length of generated XML snippet (5).
int gen_xml_books(char *buffer, char value) {
    buffer[0] = '('; // 0x28
    buffer[1] = 'T'; // 0x54
    buffer[2] = value;
    buffer[3] = '\''; // 0x27
    buffer[4] = ')'; // 0x29
    return 5;
}

// Function: gen_xml_turn
// Generates a turn indicator XML snippet.
// Returns: length of generated XML snippet (9).
int gen_xml_turn(char *buffer, char player_id) {
    buffer[0] = '('; // 0x28
    buffer[1] = 'U'; // 0x55
    buffer[2] = 'C'; // 0x43
    buffer[3] = 'I'; // 0x49
    buffer[4] = player_id;
    buffer[5] = '\''; // 0x27
    buffer[6] = '\''; // 0x27
    buffer[7] = '\''; // 0x27
    buffer[8] = ')'; // 0x29
    return 9;
}

// Function: gen_xml_final_results
// Generates final results XML snippet.
// Returns: length of generated XML snippet (20).
int gen_xml_final_results(char *buffer, char player0_books, char player1_books) {
    buffer[0] = '('; // 0x28
    buffer[1] = 'B'; // 0x42
    buffer[2] = 'C'; // 0x43
    buffer[3] = 'I'; // 0x49
    buffer[4] = '0'; // Player 0 ID
    buffer[5] = '\''; // 0x27
    buffer[6] = 'T'; // 0x54
    buffer[7] = player0_books;
    buffer[8] = '\''; // 0x27
    buffer[9] = '\''; // 0x27
    buffer[10] = 'C'; // 0x43
    buffer[11] = 'I'; // 0x49
    buffer[12] = '1'; // Player 1 ID
    buffer[13] = '\''; // 0x27
    buffer[14] = 'T'; // 0x54
    buffer[15] = player1_books;
    buffer[16] = '\''; // 0x27
    buffer[17] = '\''; // 0x27
    buffer[18] = '\''; // 0x27
    buffer[19] = ')'; // 0x29
    return 20;
}

// Function: gen_xml_error
// Generates an error XML snippet.
// Returns: length of generated XML snippet (5).
int gen_xml_error(char *buffer, char error_code) {
    buffer[0] = '('; // 0x28
    buffer[1] = 'H'; // 0x48
    buffer[2] = error_code;
    buffer[3] = '\''; // 0x27
    buffer[4] = ')'; // 0x29
    return 5;
}

// --- Main function for testing and compilation ---
int main() {
    char test_buffer[256];
    int card_ids[5];
    char *player_name = NULL;
    int result;
    int i;

    printf("--- Testing parse_xml_card ---\n");
    // Valid card: EGX'F Y'' (Rank X, Suit Y)
    result = parse_xml_card("EGX'F Y''", &card_ids[0]);
    if (result == 8) {
        printf("parse_xml_card(\"EGX'F Y''\"): SUCCESS, card_id = 0x%x (Suit: %c, Rank: %c)\n",
               card_ids[0], (char)(card_ids[0] & 0xFF), (char)((card_ids[0] >> 8) & 0xFF));
    } else {
        printf("parse_xml_card(\"EGX'F Y''\"): FAILED, result = %d\n", result);
    }
    // Valid card: EFX'G Y'' (Suit X, Rank Y)
    result = parse_xml_card("EFX'G Y''", &card_ids[1]);
    if (result == 8) {
        printf("parse_xml_card(\"EFX'G Y''\"): SUCCESS, card_id = 0x%x (Suit: %c, Rank: %c)\n",
               card_ids[1], (char)(card_ids[1] & 0xFF), (char)((card_ids[1] >> 8) & 0xFF));
    } else {
        printf("parse_xml_card(\"EFX'G Y''\"): FAILED, result = %d\n", result);
    }
    // Invalid card
    result = parse_xml_card("EGXF Y''", &card_ids[0]);
    printf("parse_xml_card(\"EGXF Y''\"): %s, result = %d\n", result < 0 ? "FAILED" : "SUCCESS", result);

    printf("\n--- Testing gen_xml_card ---\n");
    int card_to_gen = create_card('S', 'K'); // Suit 'S', Rank 'K'
    result = gen_xml_card(test_buffer, card_to_gen);
    test_buffer[result] = '\0';
    printf("gen_xml_card(0x%x): '%s' (Length: %d)\n", card_to_gen, test_buffer, result);

    printf("\n--- Testing parse_xml_player_name ---\n");
    result = parse_xml_player_name("(BCVPLAYER1''')", &player_name, strlen("(BCVPLAYER1''')"));
    if (result == 0) {
        printf("parse_xml_player_name(\"(BCVPLAYER1''')\"): SUCCESS, name = '%s'\n", player_name);
        free(player_name);
        player_name = NULL;
    } else {
        printf("parse_xml_player_name(\"(BCVPLAYER1''')\"): FAILED, result = %d\n", result);
    }
    result = parse_xml_player_name("(BCVSHORT''", &player_name, strlen("(BCVSHORT''"));
    printf("parse_xml_player_name(\"(BCVSHORT''\"): %s, result = %d\n", result < 0 ? "FAILED" : "SUCCESS", result);

    printf("\n--- Testing parse_xml_cards & gen_xml_cards ---\n");
    int cards_to_parse[2];
    char card_xml_string[] = "(DEGX'F S''EHA'G 2'')"; // 2 cards: (X,S) and (A,2)
    result = parse_xml_cards(card_xml_string, cards_to_parse);
    if (result > 0) {
        printf("parse_xml_cards(\"%s\"): SUCCESS, parsed %d cards.\n", card_xml_string, result);
        for (i = 0; i < result; ++i) {
            printf("  Card %d: 0x%x (Suit: %c, Rank: %c)\n", i, cards_to_parse[i],
                   (char)(cards_to_parse[i] & 0xFF), (char)((cards_to_parse[i] >> 8) & 0xFF));
        }
    } else {
        printf("parse_xml_cards(\"%s\"): FAILED, result = %d\n", card_xml_string, result);
    }

    int cards_to_generate[] = {
        create_card('D', 'Q'), // Queen of Diamonds
        create_card('C', 'A')  // Ace of Clubs
    };
    result = gen_xml_cards(test_buffer, cards_to_generate, 2);
    test_buffer[result] = '\0';
    printf("gen_xml_cards(2 cards): '%s' (Length: %d)\n", test_buffer, result);


    printf("\n--- Testing parse_xml_draw ---\n");
    result = parse_xml_draw("(P')");
    printf("parse_xml_draw(\"(P')\"): %s, result = %d\n", result == 0 ? "SUCCESS" : "FAILED", result);
    result = parse_xml_draw("(P)");
    printf("parse_xml_draw(\"(P)\"): %s, result = %d\n", result == 0 ? "SUCCESS" : "FAILED", result);

    printf("\n--- Testing gen_xml_go_fish ---\n");
    result = gen_xml_go_fish(test_buffer, '5');
    test_buffer[result] = '\0';
    printf("gen_xml_go_fish('5'): '%s' (Length: %d)\n", test_buffer, result);

    printf("\n--- Testing parse_xml_ask ---\n");
    result = parse_xml_ask("(SGK''')");
    printf("parse_xml_ask(\"(SGK''')\"): %s, result = %d\n", result > 0 ? "SUCCESS" : "FAILED", result);
    result = parse_xml_ask("(SGX''')"); // X is not a valid rank in dummy is_valid_rank
    printf("parse_xml_ask(\"(SGX''')\"): %s, result = %d\n", result > 0 ? "SUCCESS" : "FAILED", result);

    printf("\n--- Testing gen_xml_turn ---\n");
    result = gen_xml_turn(test_buffer, '0');
    test_buffer[result] = '\0';
    printf("gen_xml_turn('0'): '%s' (Length: %d)\n", test_buffer, result);

    printf("\n--- Testing gen_xml_final_results ---\n");
    result = gen_xml_final_results(test_buffer, '3', '2'); // Player 0 has 3 books, Player 1 has 2 books
    test_buffer[result] = '\0';
    printf("gen_xml_final_results(3, 2): '%s' (Length: %d)\n", test_buffer, result);

    printf("\n--- Testing gen_xml_error ---\n");
    result = gen_xml_error(test_buffer, '1');
    test_buffer[result] = '\0';
    printf("gen_xml_error('1'): '%s' (Length: %d)\n", test_buffer, result);

    return 0;
}