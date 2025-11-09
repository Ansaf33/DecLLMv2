#include <stdio.h>    // For snprintf
#include <unistd.h>   // For read, write
#include <string.h>   // For strlen, strncpy, strchr
#include <stdlib.h>   // For NULL

// Define the PlayerState structure based on memory access patterns
// The original code uses a 120-byte buffer for player state.
// 4 bytes of padding + 4 * 4 bytes for ship counts + 10 * 10 bytes for the board = 120 bytes.
typedef struct {
    char _padding[4]; // Unused by the provided code, but aligns offsets
    int ship2_count;  // Offset 4
    int ship3_count;  // Offset 8
    int ship4_count;  // Offset 12 (0xc)
    int ship5_count;  // Offset 16 (0x10)
    char board[10][10]; // Offset 20 (0x14) - 100 bytes
} PlayerState;

// Constants for messages
const char *P0_PROMPT = "Player 0 Command > ";
const char *P1_PROMPT = "Player 1 Command > ";
const char *EXIT_MSG = "Exiting game.\n";
const char *HELP_MSG = "Available commands: N(ew), P(lace), T(arget), M(ap), H(elp), E(xit)\n";
const char *GAME_ALREADY_STARTED_MSG = "Game already started, cannot place ships.\n";
const char *GAME_NOT_STARTED_MSG = "Game not started, cannot target ships.\n";
const char *BAD_COMMAND_MSG = "Bad Command.\n";
const char *GAME_ALREADY_ENDED_MSG = "Game already ended. Start a new game (N).\n";
const char *HIT_MSG = "Hit!\n";
const char *MISSED_MSG = "Missed.\n";
const char *SHIP2_SUNK_MSG = "You sunk a 2-unit ship!\n";
const char *SHIP3_SUNK_MSG = "You sunk a 3-unit ship!\n";
const char *SHIP4_SUNK_MSG = "You sunk a 4-unit ship!\n";
const char *SHIP5_SUNK_MSG = "You sunk a 5-unit ship!\n";
const char *YOU_WIN_MSG = "You win!\n";
const char *READ_ERROR_MSG = "Read error.\n";

// --- Custom readLine function ---
// This function mimics the behavior suggested by the original code.
// It reads from a file descriptor until a newline character is encountered,
// the buffer is full, or an error occurs.
// It returns the number of bytes read (excluding newline), or -1 on error.
int readLine(int fd, char *buf, size_t count, int flags) {
    size_t i = 0;
    char c;
    while (i < count - 1) { // Leave space for null terminator
        ssize_t bytes_read = read(fd, &c, 1);
        if (bytes_read < 0) {
            return -1; // Read error
        }
        if (bytes_read == 0) {
            // EOF, but no newline seen. Treat as success with current content.
            break;
        }
        if (c == '\n') {
            break; // Newline found
        }
        buf[i++] = c;
    }
    buf[i] = '\0'; // Null-terminate the string
    return i; // Return number of characters read (excluding newline)
}

// Function: getCommand
// param_1 (command_buffer) is expected to be a char array of at least 5 bytes.
// Returns 0 on success, -1 for bad command, -2 for read error.
int getCommand(char *command_buffer) {
    char line_buffer[512]; // Matches 0x200 (512) from original readLine call
    int bytes_read;

    if (command_buffer == NULL) {
        return -1;
    }

    bytes_read = readLine(0, line_buffer, sizeof(line_buffer), 0x1100f);

    if (bytes_read < 0) {
        return -2; // Indicates a read error from readLine
    } else if (bytes_read == 0) {
        // No input, treat as bad command or EOF. Original code returns -1 for bad command.
        command_buffer[0] = '\0'; // Ensure it's empty
        return -1;
    } else {
        // Copy the relevant parts. main passes a 5-byte buffer.
        // command_buffer[0] gets the first char
        // command_buffer[1] gets the second char, etc.
        strncpy(command_buffer, line_buffer, 5);
        command_buffer[4] = '\0'; // Ensure null termination for command_buffer (size 5)

        // Based on the original code, the first character determines the command.
        // If line_buffer[0] is not a recognized command, it's 'U' (Unknown)
        if (strchr("NPTMEHS", command_buffer[0]) == NULL) {
             command_buffer[0] = 'U';
             return -1; // Unknown command
        }
    }
    return 0; // Success
}

// Function: dumpBoard
int dumpBoard(PlayerState *player_state) {
    if (player_state == NULL) {
        return -1;
    }

    char output_buf[128]; // Buffer for formatted output

    // Print column headers: \tA B C D E F G H I J\n
    int offset = snprintf(output_buf, sizeof(output_buf), "\t");
    for (int col = 0; col < 10; ++col) {
        offset += snprintf(output_buf + offset, sizeof(output_buf) - offset, "%c ", (char)col + 'A');
    }
    snprintf(output_buf + offset -1, sizeof(output_buf) - offset -1, "\n"); // overwrite last space with newline
    write(1, output_buf, strlen(output_buf));

    // Print rows
    for (int row = 0; row < 10; ++row) {
        // Original logic for row header: write(1,&local_1c,6); which was "   %c:\t"
        offset = snprintf(output_buf, sizeof(output_buf), "   %c:\t", (char)row + '0');
        write(1, output_buf, offset); // Write the row header part

        for (int col = 0; col < 10; ++col) {
            write(1, &player_state->board[row][col], 1);
        }
        write(1, "\n", 1);
    }
    return 0;
}

// Function: clearPlayerState
int clearPlayerState(PlayerState *player_state) {
    if (player_state == NULL) {
        return -1;
    }

    for (int row = 0; row < 10; ++row) {
        for (int col = 0; col < 10; ++col) {
            player_state->board[row][col] = '.';
        }
    }
    player_state->ship2_count = 0;
    player_state->ship3_count = 0;
    player_state->ship4_count = 0;
    player_state->ship5_count = 0;
    return 0;
}

// Function: isGoodRow
int isGoodRow(char row_char) {
    return (row_char >= '0') && (row_char <= '9');
}

// Function: isGoodCol
int isGoodCol(char col_char) {
    return (col_char >= 'A') && (col_char <= 'J');
}

// Function: isOnBoard
int isOnBoard(char row_char, char col_char, char orientation, int ship_size) {
    int row = row_char - '0';
    int col = col_char - 'A';

    if (orientation == 'R') { // Right (horizontal)
        return (col >= 0) && (col + ship_size <= 10);
    } else if (orientation == 'D') { // Down (vertical)
        return (row >= 0) && (row + ship_size <= 10);
    }
    return 0; // Invalid orientation
}

// Function: processPlace
// param_2 (command_args) contains the command string, e.g., "P5RA"
// command_args[0] = 'P' (ignored here)
// command_args[1] = row_char ('0'-'9')
// command_args[2] = col_char ('A'-'J')
// command_args[3] = orientation ('R' or 'D')
// command_args[4] = ship_size_char ('2'-'5')
int processPlace(PlayerState *player_state, char *command_args) {
    if (player_state == NULL || command_args == NULL) {
        return -1;
    }

    char row_char = command_args[1];
    char col_char = command_args[2];
    char orientation = command_args[3];
    char ship_size_char = command_args[4];
    int ship_size = ship_size_char - '0';

    if (!isGoodRow(row_char) || !isGoodCol(col_char) || (ship_size < 2 || ship_size > 5) || (orientation != 'R' && orientation != 'D')) {
        return -1;
    }

    int row = row_char - '0';
    int col = col_char - 'A';

    if (!isOnBoard(row_char, col_char, orientation, ship_size)) {
        return -1;
    }

    // Check if ship of this size is already placed
    if ((ship_size == 2 && player_state->ship2_count > 0) ||
        (ship_size == 3 && player_state->ship3_count > 0) ||
        (ship_size == 4 && player_state->ship4_count > 0) ||
        (ship_size == 5 && player_state->ship5_count > 0)) {
        return -1;
    }

    // Check for overlaps and place ship
    if (orientation == 'R') { // Horizontal
        for (int i = 0; i < ship_size; ++i) {
            if (player_state->board[row][col + i] != '.') {
                return -1; // Overlap
            }
        }
        for (int i = 0; i < ship_size; ++i) {
            player_state->board[row][col + i] = ship_size_char;
        }
    } else { // Vertical (orientation == 'D')
        for (int i = 0; i < ship_size; ++i) {
            if (player_state->board[row + i][col] != '.') {
                return -1; // Overlap
            }
        }
        for (int i = 0; i < ship_size; ++i) {
            player_state->board[row + i][col] = ship_size_char;
        }
    }

    // Update ship count (number of hits remaining)
    switch (ship_size) {
        case 2: player_state->ship2_count = 2; break;
        case 3: player_state->ship3_count = 3; break;
        case 4: player_state->ship4_count = 4; break;
        case 5: player_state->ship5_count = 5; break;
    }

    return 0;
}

// Function: processTarget
// param_2 (command_args) contains the command string, e.g., "T5A"
// command_args[0] = 'T' (ignored here)
// command_args[1] = row_char ('0'-'9')
// command_args[2] = col_char ('A'-'J')
int processTarget(PlayerState *target_player_state, char *command_args) {
    if (target_player_state == NULL || command_args == NULL) {
        return -1;
    }

    char row_char = command_args[1];
    char col_char = command_args[2];

    if (!isGoodRow(row_char) || !isGoodCol(col_char)) {
        return -1;
    }

    int row = row_char - '0';
    int col = col_char - 'A';

    char target_cell = target_player_state->board[row][col];

    if (target_cell == '.') {
        write(1, MISSED_MSG, strlen(MISSED_MSG));
        target_player_state->board[row][col] = 'O'; // Mark as missed
    } else if (target_cell == 'X' || target_cell == 'O') {
        // Already hit or missed, original code implies it's a miss.
        write(1, MISSED_MSG, strlen(MISSED_MSG));
    } else { // It's a ship ('2', '3', '4', '5')
        write(1, HIT_MSG, strlen(HIT_MSG));
        target_player_state->board[row][col] = 'X'; // Mark as hit

        // Decrement ship part count
        switch (target_cell) {
            case '2':
                target_player_state->ship2_count--;
                if (target_player_state->ship2_count == 0) {
                    write(1, SHIP2_SUNK_MSG, strlen(SHIP2_SUNK_MSG));
                }
                break;
            case '3':
                target_player_state->ship3_count--;
                if (target_player_state->ship3_count == 0) {
                    write(1, SHIP3_SUNK_MSG, strlen(SHIP3_SUNK_MSG));
                }
                break;
            case '4':
                target_player_state->ship4_count--;
                if (target_player_state->ship4_count == 0) {
                    write(1, SHIP4_SUNK_MSG, strlen(SHIP4_SUNK_MSG));
                }
                break;
            case '5':
                target_player_state->ship5_count--;
                if (target_player_state->ship5_count == 0) {
                    write(1, SHIP5_SUNK_MSG, strlen(SHIP5_SUNK_MSG));
                }
                break;
        }
    }
    return 0;
}

// Function: allShipsSunk
int allShipsSunk(PlayerState *player_state) {
    if (player_state == NULL) {
        return 0; // Or -1 for error, but original returns 0 for NULL
    }
    return (player_state->ship2_count == 0 &&
            player_state->ship3_count == 0 &&
            player_state->ship4_count == 0 &&
            player_state->ship5_count == 0);
}

// Function: main
int main(void) {
    char command_buffer[5]; // Stores command and arguments (e.g., "P5RA\0")
    PlayerState player0_state;
    PlayerState player1_state;

    int get_command_result = 0; // Return value from getCommand
    unsigned int current_player = 0; // 0 for player 0, 1 for player 1
    int game_started = 0; // 0 not started, 1 started
    int game_ended = 0; // 0 not ended, 1 ended

    // Initialize command_buffer (ensure null termination for safety)
    command_buffer[0] = 'U'; // Default to unknown command
    command_buffer[4] = '\0';

    clearPlayerState(&player0_state);
    clearPlayerState(&player1_state);

    do {
        // Output player prompt
        if (current_player == 0) {
            write(1, P0_PROMPT, strlen(P0_PROMPT));
        } else {
            write(1, P1_PROMPT, strlen(P1_PROMPT));
        }

        get_command_result = getCommand(command_buffer);

        if (get_command_result == -2) { // Read error from readLine
            write(1, READ_ERROR_MSG, strlen(READ_ERROR_MSG));
            break; // Exit game loop
        }

        if (get_command_result != 0) { // Bad command or empty input
            write(1, BAD_COMMAND_MSG, strlen(BAD_COMMAND_MSG));
            continue; // Go to next loop iteration, ask for command again
        }

        char command_char = command_buffer[0];

        if (command_char == 'E') {
            write(1, EXIT_MSG, strlen(EXIT_MSG));
            break; // Exit game
        } else if (command_char == 'N') {
            clearPlayerState(&player0_state);
            clearPlayerState(&player1_state);
            current_player = 0;
            game_started = 0;
            game_ended = 0;
        } else if (command_char == 'H') {
            write(1, HELP_MSG, strlen(HELP_MSG));
        } else if (game_ended == 1) { // If game has already ended
            write(1, GAME_ALREADY_ENDED_MSG, strlen(GAME_ALREADY_ENDED_MSG));
        } else { // Game not ended, process other commands
            if (command_char == 'M') {
                if (current_player == 0) {
                    dumpBoard(&player0_state);
                } else {
                    dumpBoard(&player1_state);
                }
            } else if (command_char == 'S') {
                if (game_started == 0) {
                    game_started = 1;
                } else {
                    write(1, GAME_ALREADY_STARTED_MSG, strlen(GAME_ALREADY_STARTED_MSG));
                }
            } else if (command_char == 'P') {
                if (game_started == 0) {
                    int process_place_result;
                    if (current_player == 0) {
                        process_place_result = processPlace(&player0_state, command_buffer);
                    } else {
                        process_place_result = processPlace(&player1_state, command_buffer);
                    }

                    if (process_place_result == 0) {
                        current_player = (current_player == 0) ? 1 : 0; // Switch player
                    } else {
                        write(1, BAD_COMMAND_MSG, strlen(BAD_COMMAND_MSG));
                    }
                } else {
                    write(1, GAME_ALREADY_STARTED_MSG, strlen(GAME_ALREADY_STARTED_MSG));
                }
            } else if (command_char == 'T') {
                if (game_started == 0) {
                    write(1, GAME_NOT_STARTED_MSG, strlen(GAME_NOT_STARTED_MSG));
                } else {
                    int process_target_result;
                    // Target the opponent's board
                    PlayerState *target_board = (current_player == 0) ? &player1_state : &player0_state;
                    process_target_result = processTarget(target_board, command_buffer);

                    if (process_target_result == 0) {
                        if (allShipsSunk(target_board)) {
                            write(1, YOU_WIN_MSG, strlen(YOU_WIN_MSG));
                            game_started = 0; // Game ends
                            game_ended = 1;
                        }
                        current_player = (current_player == 0) ? 1 : 0; // Switch player
                    } else {
                        write(1, BAD_COMMAND_MSG, strlen(BAD_COMMAND_MSG));
                    }
                }
            } else { // 'U' for unknown command from getCommand
                 write(1, BAD_COMMAND_MSG, strlen(BAD_COMMAND_MSG));
            }
        }
    } while (1); // Loop indefinitely until 'E' command or read error breaks it

    return 0;
}