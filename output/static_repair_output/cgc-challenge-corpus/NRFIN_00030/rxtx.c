#include <stdlib.h> // For calloc, free
#include <string.h> // For memset
#include <sys/socket.h> // For send, ssize_t
#include <sys/types.h> // For size_t (often included by other headers)
#include <unistd.h> // For close (though not directly in these snippets), generally useful for socket ops

// --- Forward Declarations of External Functions ---
// These functions are assumed to be defined elsewhere in the project.
// The types are inferred from their usage in the provided snippets.
// Error codes like 0xfffffff7 (-9) and 0xffffffc2 (-62) imply 'int' return type for status.
// Lengths and counts are typically 'unsigned int' or 'size_t'.

// Receives data until a delimiter is found or max_len is reached.
// Returns the number of bytes received, or a negative error code.
unsigned int recv_until_delim(int fd, void *buf, size_t len, char delimiter);

// Parses XML for a player name.
// Returns an integer status code.
int parse_xml_player_name(void *xml_buf, void *player_name_out, unsigned int xml_len);

// Generates XML for cards into xml_buf_out.
// Returns the length of the generated XML, or a negative error code.
int gen_xml_cards(void *xml_buf_out, void *game_data_in);

// Parses XML for cards.
// Returns the number of cards parsed, or a negative error code.
int parse_xml_cards(void *xml_buf, void *card_data_out, unsigned char expected_count);

// Parses XML for a draw request.
// Returns an integer status code or value.
int parse_xml_draw(void *xml_buf);

// Generates XML for a "go fish" notice.
// Returns the length of the generated XML, or a negative error code.
int gen_xml_go_fish(void *xml_buf_out, char player_id);

// Parses XML for a "go fish" notice.
// Returns an integer status code or value.
int parse_xml_go_fish(void *xml_buf);

// Parses XML for a fishing request.
// Returns an integer status code or value.
int parse_xml_fishing(void *xml_buf);

// Compares two sets of cards for equality.
// Returns 1 if equal, 0 if not, or a negative error code.
int is_set_equal(void *set1, void *set2, unsigned char count, int param_4);

// Parses XML for an "ask" request.
// Returns an integer status code or value (e.g., the rank asked for).
int parse_xml_ask(void *xml_buf);

// Generates XML for an "ask" request.
// Returns the length of the generated XML, or a negative error code.
int gen_xml_ask(void *xml_buf_out, char rank_to_ask_for);

// Parses XML for the count of books played.
// Returns the number of books parsed, or a negative error code.
int parse_xml_books(void *xml_buf);

// Generates XML for the count of books played.
// Returns the length of the generated XML, or a negative error code.
int gen_xml_books(void *xml_buf_out, char books_count);

// Generates XML for a "turn" notice.
// Returns the length of the generated XML, or a negative error code.
int gen_xml_turn(void *xml_buf_out, char player_id);

// Generates XML for final game results.
// Returns the length of the generated XML, or a negative error code.
int gen_xml_final_results(void *xml_buf_out, char player_data_byte);

// Generates XML for an error message.
// Returns the length of the generated XML, or a negative error code.
int gen_xml_error(void *xml_buf_out, int error_code);

// Standard socket send function.
ssize_t send(int sockfd, const void *buf, size_t len, int flags);

// Function: recv_player_name
// Original: undefined4 recv_player_name(undefined4 param_1)
// Fix: Replaced undefined4 with int, uint with unsigned int.
//      Added socket_fd parameter.
//      local_7c (25 undefined4) is 100 bytes. local_18 is 100.
//      Replaced initialization loop with memset.
//      Reduced intermediate variables.
int recv_player_name(int socket_fd, void *player_name_out) {
    char name_buffer[100]; // Buffer for player name XML (25 * sizeof(int) = 100 bytes)
    memset(name_buffer, 0, sizeof(name_buffer));

    unsigned int bytes_received = recv_until_delim(socket_fd, name_buffer, sizeof(name_buffer), ')');

    if ((int)bytes_received < 1) {
        return -9; // 0xfffffff7
    } else {
        // `bytes_received & 0xff` is kept as it was in the original snippet,
        // assuming it's a specific protocol requirement to use only the lower byte of length.
        return parse_xml_player_name(name_buffer, player_name_out, bytes_received & 0xff);
    }
}

// Function: send_cards
// Original: void * send_cards(undefined4 param_1,byte param_2)
// Fix: Replaced undefined4 with int, byte with unsigned char.
//      Added socket_fd parameter.
//      Corrected `calloc` second argument from `in_stack_ffffffc8` to `1`.
//      Corrected `send` arguments. `__ptr` is buffer, `__buf` is length.
//      Corrected comparison `__buf == (void *)(uint)bVar1` to `generated_len == xml_buffer_size`.
//      Mapped return values (0x0 to NULL, 0xffffffc2 to (void*)-62).
void *send_cards(int socket_fd, void *game_data, unsigned char card_count) {
    int flags = 0x136ce;
    size_t xml_buffer_size = (size_t)card_count * 8 + 4; // param_2 * '\b' + 4
    void *xml_buffer = calloc(xml_buffer_size, 1);

    if (xml_buffer == NULL) {
        return (void *)-1; // Allocation failed
    }

    int generated_len = gen_xml_cards(xml_buffer, game_data);
    void *result = NULL; // Success by default

    if (generated_len > 0) {
        if ((size_t)generated_len == xml_buffer_size) {
            if (send(socket_fd, xml_buffer, generated_len, flags) == -1) {
                result = (void *)-2; // Indicate send error
            }
        } else {
            result = (void *)-62; // 0xffffffc2 - Mismatch in generated length
        }
    } else {
        result = (void *)-3; // gen_xml_cards failed or returned 0
    }

    free(xml_buffer);
    return result;
}

// Function: recv_cards
// Original: uint recv_cards(undefined4 param_1,byte param_2)
// Fix: Replaced undefined4 with int, byte with unsigned char, uint with unsigned int.
//      Added socket_fd parameter.
//      Corrected `calloc` second argument from `in_stack_ffffffc8` to `1`.
//      Simplified conditional logic and reduced intermediate variables.
unsigned int recv_cards(int socket_fd, void *card_data_out, unsigned char expected_card_count) {
    const size_t buffer_size = 0x1400; // 5120 bytes
    void *xml_buffer = calloc(buffer_size, 1);

    if (xml_buffer == NULL) {
        return (unsigned int)-1; // Indicate allocation failure
    }

    unsigned int bytes_received = recv_until_delim(socket_fd, xml_buffer, buffer_size, ')');
    unsigned int result = bytes_received; // Default to bytes received, which can be an error code

    if ((int)bytes_received >= 0) {
        int parsed_count = parse_xml_cards(xml_buffer, card_data_out, expected_card_count);
        if (parsed_count >= 0) {
            if ((unsigned char)parsed_count != expected_card_count) {
                result = (unsigned int)-62; // 0xffffffc2 - Mismatch in parsed card count
            } else {
                result = (unsigned int)parsed_count; // Success, return parsed count
            }
        } else {
            result = (unsigned int)parsed_count; // Error from parse_xml_cards
        }
    }
    // If bytes_received was negative, result already holds the error code.

    free(xml_buffer);
    return result;
}

// Function: recv_draw_request
// Original: undefined4 recv_draw_request(void)
// Fix: Replaced undefined4 with int.
//      Added socket_fd parameter.
//      Initialized local buffer. Reduced intermediate variables.
int recv_draw_request(int socket_fd) {
    int draw_request_buffer = 0; // undefined4, 4 bytes, initialized to 0
    const int expected_len = 4;

    unsigned int bytes_received = recv_until_delim(socket_fd, &draw_request_buffer, sizeof(draw_request_buffer), ')');

    if (expected_len == (int)bytes_received) {
        return parse_xml_draw(&draw_request_buffer);
    } else {
        return -9; // 0xfffffff7
    }
}

// Function: send_hand
// Original: void send_hand(undefined *param_1)
// Fix: Replaced undefined* with unsigned char*.
//      Added socket_fd parameter.
//      Passed the correct arguments to send_cards.
void send_hand(int socket_fd, unsigned char *hand_data) {
    // Assuming hand_data points to a structure where the first byte is card_count
    // and cards data starts at an offset of 4 bytes (due to padding or other fields).
    send_cards(socket_fd, hand_data + 4, *hand_data);
}

// Function: send_go_fish_notice
// Original: void send_go_fish_notice(undefined param_1)
// Fix: Replaced undefined with char.
//      Added socket_fd parameter.
//      Combined `local_15` and `local_11` into a single `char` array buffer.
//      Corrected `send` arguments. `local_10` was holding generated length, not buffer.
void send_go_fish_notice(int socket_fd, char player_id) {
    int flags = 0x138ce;
    char xml_buffer[5] = {0}; // undefined4 (4 bytes) + undefined (1 byte)

    int generated_len = gen_xml_go_fish(xml_buffer, player_id);

    if (generated_len > 0) {
        send(socket_fd, xml_buffer, generated_len, flags);
    }
}

// Function: recv_go_fish_notice
// Original: undefined4 recv_go_fish_notice(void)
// Fix: Replaced undefined4 with int, undefined with char.
//      Added socket_fd parameter.
//      Combined `local_1d` and `local_19` into a single `char` array buffer.
//      Reduced intermediate variables.
int recv_go_fish_notice(int socket_fd) {
    char xml_buffer[5] = {0}; // undefined4 (4 bytes) + undefined (1 byte)
    const int expected_len = 5;

    unsigned int bytes_received = recv_until_delim(socket_fd, xml_buffer, sizeof(xml_buffer), ')');

    if (expected_len == (int)bytes_received) {
        return parse_xml_go_fish(xml_buffer);
    } else {
        return -9; // 0xfffffff7
    }
}

// Function: recv_fish_request
// Original: undefined4 recv_fish_request(void)
// Fix: Replaced undefined4 with int.
//      Added socket_fd parameter.
//      Initialized local buffer. Reduced intermediate variables.
int recv_fish_request(int socket_fd) {
    int xml_buffer = 0; // undefined4, 4 bytes, initialized to 0
    const int expected_len = 4;

    unsigned int bytes_received = recv_until_delim(socket_fd, &xml_buffer, sizeof(xml_buffer), ')');

    if (expected_len == (int)bytes_received) {
        return parse_xml_fishing(&xml_buffer);
    } else {
        return -9; // 0xfffffff7
    }
}

// Function: recv_and_match_cards
// Original: undefined4 recv_and_match_cards(undefined4 param_1,byte param_2)
// Fix: Replaced undefined4 with int, byte with unsigned char, uint with unsigned int.
//      Added socket_fd parameter.
//      Corrected `calloc` second argument from `in_stack_ffffffc8` to `1`.
//      Simplified conditional logic and reduced intermediate variables.
//      Assumes `param_1` is `void *expected_cards_set`.
int recv_and_match_cards(int socket_fd, void *expected_cards_set, unsigned char card_count) {
    const int comparison_param = 0x139f6;
    size_t buffer_size;

    if (card_count == 0) {
        buffer_size = 4; // Minimum size, or a specific protocol value for 0 cards
    } else {
        buffer_size = (size_t)card_count * 4; // (uint)param_2 << 2 is param_2 * 4
    }

    void *received_cards_buffer = calloc(buffer_size, 1);
    if (received_cards_buffer == NULL) {
        return -1; // Allocation failed
    }

    unsigned int recv_result = recv_cards(socket_fd, received_cards_buffer, card_count);
    int match_result = 0; // Default to no match or error

    // recv_cards returns the parsed count on success if it matches expected_card_count,
    // or an error code otherwise.
    if (recv_result == card_count) {
        match_result = is_set_equal(received_cards_buffer, expected_cards_set, card_count, comparison_param);
    }

    free(received_cards_buffer);
    return match_result;
}

// Function: recv_ask_and_get_rank
// Original: undefined4 recv_ask_and_get_rank(void)
// Fix: Replaced undefined4 with int, undefined3 with char[3].
//      Added socket_fd parameter.
//      Combined `local_1b` and `uStack_17` into a single `char` array buffer.
//      Reduced intermediate variables.
int recv_ask_and_get_rank(int socket_fd) {
    char xml_buffer[7] = {0}; // undefined4 (4 bytes) + undefined3 (3 bytes)
    const int expected_len = 7;

    unsigned int bytes_received = recv_until_delim(socket_fd, xml_buffer, sizeof(xml_buffer), ')');

    if (expected_len == (int)bytes_received) {
        return parse_xml_ask(xml_buffer);
    } else {
        return -9; // 0xfffffff7
    }
}

// Function: send_ask
// Original: void send_ask(undefined param_1)
// Fix: Replaced undefined with char.
//      Added socket_fd parameter.
//      Combined `local_17` and `uStack_13` into a single `char` array buffer.
//      Corrected `send` arguments. `local_10` was holding generated length, not buffer.
void send_ask(int socket_fd, char rank_to_ask_for) {
    int flags = 0x13b0e;
    char xml_buffer[7] = {0}; // undefined4 (4 bytes) + undefined3 (3 bytes)

    int generated_len = gen_xml_ask(xml_buffer, rank_to_ask_for);

    if (generated_len > 0) {
        send(socket_fd, xml_buffer, generated_len, flags);
    }
}

// Function: recv_and_match_count_books_played
// Original: uint recv_and_match_count_books_played(uint param_1)
// Fix: Replaced uint with unsigned int, undefined4 with int, undefined with char.
//      Added socket_fd parameter.
//      Combined `local_1d` and `local_19` into a single `char` array buffer.
//      Simplified conditional logic and reduced intermediate variables.
//      `param_1 & 0xff` implies only the lowest byte of expected_books_count_byte is used.
unsigned int recv_and_match_count_books_played(int socket_fd, unsigned int expected_books_count_byte) {
    char xml_buffer[5] = {0}; // undefined4 (4 bytes) + undefined (1 byte)
    const int expected_xml_len = 5;

    unsigned int bytes_received = recv_until_delim(socket_fd, xml_buffer, sizeof(xml_buffer), ')');

    if (expected_xml_len == (int)bytes_received) {
        int parsed_books_count = parse_xml_books(xml_buffer);
        if (parsed_books_count >= 0) {
            if ((unsigned int)parsed_books_count == (expected_books_count_byte & 0xff)) {
                return 1; // Match
            } else {
                return 0; // Mismatch
            }
        } else {
            return (unsigned int)parsed_books_count; // Error from parser
        }
    } else {
        return (unsigned int)-9; // 0xfffffff7 - Recv error or length mismatch
    }
}

// Function: send_count_books_played
// Original: void send_count_books_played(undefined param_1)
// Fix: Replaced undefined with char.
//      Added socket_fd parameter.
//      Combined `local_17` and `uStack_13` into a single `char` array buffer.
//      Corrected `send` arguments. `local_10` was holding generated length, not buffer.
void send_count_books_played(int socket_fd, char books_count) {
    int flags = 0x13c00;
    char xml_buffer[7] = {0}; // undefined4 (4 bytes) + undefined3 (3 bytes)

    int generated_len = gen_xml_books(xml_buffer, books_count);

    if (generated_len > 0) {
        send(socket_fd, xml_buffer, generated_len, flags);
    }
}

// Function: send_turn_notice
// Original: void send_turn_notice(undefined param_1)
// Fix: Replaced undefined with char.
//      Added socket_fd parameter.
//      Combined `local_19`, `local_15`, `local_11` into a single `char` array buffer.
//      Corrected `send` arguments. `local_10` was holding generated length, not buffer.
void send_turn_notice(int socket_fd, char player_id) {
    int flags = 0x13c5d;
    char xml_buffer[9] = {0}; // undefined4 (4) + undefined4 (4) + undefined (1)

    int generated_len = gen_xml_turn(xml_buffer, player_id);

    if (generated_len > 0) {
        send(socket_fd, xml_buffer, generated_len, flags);
    }
}

// Function: send_initial_hand_to_player
// Original: void send_initial_hand_to_player(int param_1)
// Fix: Added socket_fd parameter.
//      Retained complex pointer arithmetic, assuming it correctly accesses data structures.
//      Casted to appropriate C types.
void send_initial_hand_to_player(int socket_fd, int player_struct_ptr) {
    // This pointer arithmetic is specific to the game's data structures.
    // It is assumed to resolve to a pointer to card data and the card count.
    void *cards_data_ptr = (void *)(*(int *)(*(int *)(player_struct_ptr + 8) + 8) + 4);
    unsigned char card_count = **(unsigned char **)(*(int *)(player_struct_ptr + 8) + 8);

    send_cards(socket_fd, cards_data_ptr, card_count);
}

// Function: send_final_results
// Original: void send_final_results(int param_1)
// Fix: Added socket_fd parameter.
//      Combined local_24 to local_14 into a single `char` array buffer.
//      Retained complex pointer arithmetic, assuming it correctly accesses data structures.
//      Corrected `send` arguments to use the length returned by `gen_xml_final_results`.
void send_final_results(int socket_fd, int game_state_ptr) {
    int flags = 0x13cf8;
    char xml_buffer[20] = {0}; // 5 x undefined4

    // This pointer arithmetic is specific to the game's data structures.
    // It is assumed to extract a byte representing player data (e.g., player ID).
    char player_data_byte = *(unsigned char *)(*(int *)(game_state_ptr + 8) + 1);

    int generated_len = gen_xml_final_results(xml_buffer, player_data_byte);

    if (generated_len > 0) {
        // The original code used a separate calculated length for `send`,
        // but `gen_xml_final_results` should return the actual length.
        // Using `generated_len` for consistency.
        send(socket_fd, xml_buffer, generated_len, flags);
    }
}

// Function: send_error
// Original: void send_error(char param_1)
// Fix: Added socket_fd parameter.
//      Combined `local_15` and `local_11` into a single `char` array buffer.
//      Corrected `send` arguments. `local_10` was holding generated length, not buffer.
void send_error(int socket_fd, char error_code) {
    int flags = 0x13d7b;
    char xml_buffer[5] = {0}; // undefined4 (4 bytes) + undefined (1 byte)

    int generated_len = gen_xml_error(xml_buffer, (int)error_code);

    if (generated_len > 0) {
        send(socket_fd, xml_buffer, generated_len, flags);
    }
}