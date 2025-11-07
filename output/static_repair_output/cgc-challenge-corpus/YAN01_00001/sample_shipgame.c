#include <stdio.h>    // For standard I/O (printf, but using write as in original)
#include <unistd.h>   // For read, write, STDIN_FILENO, STDOUT_FILENO
#include <string.h>   // For strlen
#include <stdlib.h>   // For size_t

// Replace undefined types with standard C types
// Assuming 'undefined4' is a 4-byte integer (int)
// Assuming 'undefined' is a 1-byte character (char)
typedef char BYTE;
typedef int DWORD; // Or unsigned int, depending on context. Given comparison with -1, int is fine.

// PlayerState structure based on memory offsets observed in original code
// 0x00: _unused_pad (or potentially a player ID/status, not explicitly used as such in the provided snippets)
// 0x04: ship2_count (remaining segments for 2-unit ship)
// 0x08: ship3_count (remaining segments for 3-unit ship)
// 0x0C: ship4_count (remaining segments for 4-unit ship)
// 0x10: ship5_count (remaining segments for 5-unit ship)
// 0x14: board[10][10] (10x10 grid of characters)
typedef struct {
    DWORD _unused_pad; // Placeholder for the first 4 bytes, if any, before ship counts
    DWORD ship2_count;
    DWORD ship3_count;
    DWORD ship4_count;
    DWORD ship5_count;
    BYTE board[10][10]; // 10x10 grid
} PlayerState;

// Global messages (defined as const char* for efficiency and read-only access)
const char* P0_PROMPT = "Player 0 command: ";
const char* P1_PROMPT = "Player 1 command: ";
const char* EXIT_MSG = "Exiting game.\n";
const char* HELP_MSG = "Commands:\n N - New Game\n P <row><col><D/R><size> - Place Ship (e.g., P1AR2)\n T <row><col> - Target Cell (e.g., T1A)\n M - My Board\n H - Help\n E - Exit\n S - Start Game\n";
const char* GAME_ALREADY_STARTED_MSG = "Cannot place ships after game started. Use T to target.\n";
const char* GAME_NOT_STARTED_MSG = "Game has not started. Place ships first.\n";
const char* BAD_COMMAND_MSG = "Bad command or arguments.\n";
const char* GAME_ALREADY_ENDED_MSG = "Game has already ended. Start a new game with N.\n";
const char* HIT_MSG = "Hit!\n";
const char* MISSED_MSG = "Missed!\n";
const char* SHIP2_SUNK_MSG = "You sunk a 2-unit ship!\n";
const char* SHIP3_SUNK_MSG = "You sunk a 3-unit ship!\n";
const char* SHIP4_SUNK_MSG = "You sunk a 4-unit ship!\n";
const char* SHIP5_SUNK_MSG = "You sunk a 5-unit ship!\n";
const char* YOU_WIN_MSG = "You win!\n";
const char* READ_ERROR_MSG = "Read error or EOF.\n";
const char* ALREADY_TARGETED_MSG = "Already targeted this spot.\n";

// Stub for readLine - reads a line from stdin
// Returns number of bytes read, 0 for EOF, or negative on error.
// The original code uses readLine(0, &local_210, 0x200, 0x1100f);
// fd=0 (stdin), buf=&local_210 (char buffer), count=0x200 (max size), param4 (unknown, ignored for stub)
ssize_t readLine(int fd, char *buf, size_t count, int param4_ignored) {
    ssize_t bytes_read = 0;
    char c;
    while (bytes_read < count - 1) { // Leave space for null terminator
        ssize_t r = read(fd, &c, 1);
        if (r <= 0) { // EOF or error
            if (bytes_read > 0) { // Return partial line if any characters were read
                buf[bytes_read] = '\0';
                return bytes_read;
            }
            return r; // Return 0 for EOF, -1 for read error
        }
        if (c == '\n') {
            buf[bytes_read] = '\0';
            return bytes_read;
        }
        buf[bytes_read++] = c;
    }
    buf[bytes_read] = '\0'; // Ensure null termination if buffer full
    // Consume rest of the line if buffer was full
    while (c != '\n') {
        if (read(fd, &c, 1) <= 0) break; // Stop if EOF or error while consuming
    }
    return bytes_read;
}

// Function: getCommand
// command_buffer: char* to store the parsed command (e.g., char command_buffer[5])
// Returns 0 on success, -1 on invalid command, -2 on read error.
DWORD getCommand(BYTE *command_buffer) {
    char input_buffer[0x200]; // Buffer for readLine, 512 bytes
    ssize_t bytes_read;

    if (command_buffer == NULL) {
        return -1;
    }

    bytes_read = readLine(STDIN_FILENO, input_buffer, sizeof(input_buffer), 0x1100f);

    if (bytes_read < 0) {
        return -2; // Read error
    }

    // Initialize command_buffer[0] to 'U' (Unknown)
    command_buffer[0] = 'U';

    if (bytes_read > 0) {
        // Copy the first character as the command type
        command_buffer[0] = input_buffer[0];

        // Based on the original code's variable access,
        // local_20f is input_buffer[1], local_20e is input_buffer[2], etc.
        switch (input_buffer[0]) {
            case 'N': // New Game
            case 'M': // My Board
            case 'E': // Exit
            case 'H': // Help
            case 'S': // Start Game
                // These commands are single characters. No further arguments copied.
                break;
            case 'P': // Place Ship: P<row><col><D/R><size> (e.g., P1AR2)
                if (bytes_read >= 5) {
                    command_buffer[1] = input_buffer[1]; // row
                    command_buffer[2] = input_buffer[2]; // col
                    command_buffer[3] = input_buffer[3]; // direction
                    command_buffer[4] = input_buffer[4]; // size
                } else {
                    command_buffer[0] = 'U'; // Invalid P command format
                    return -1;
                }
                break;
            case 'T': // Target: T<row><col> (e.g., T1A)
                if (bytes_read >= 3) {
                    command_buffer[1] = input_buffer[1]; // row
                    command_buffer[2] = input_buffer[2]; // col
                } else {
                    command_buffer[0] = 'U'; // Invalid T command format
                    return -1;
                }
                break;
            default:
                // Unknown command
                return -1;
        }
    } else {
        // If bytes_read is 0 (empty line or EOF on first read), treat as unknown/error
        return -1;
    }

    return 0;
}

// Function: dumpBoard
// player_state: PlayerState* to the player's board
DWORD dumpBoard(PlayerState *player_state) {
    if (player_state == NULL) {
        return -1;
    }

    // Print column headers: "\tA B C D E F G H I J\n"
    write(STDOUT_FILENO, "\t", 1);
    for (int col = 0; col < 10; ++col) {
        char col_char = (char)col + 'A';
        write(STDOUT_FILENO, &col_char, 1);
        if (col < 9) { // Add space between columns, but not after the last one
            write(STDOUT_FILENO, " ", 1);
        }
    }
    write(STDOUT_FILENO, "\n", 1);

    // Print rows with row numbers
    for (int row = 0; row < 10; ++row) {
        char row_char = (char)row + '0';
        char row_prefix[6] = {' ', ' ', ' ', row_char, ':', '\t'}; // "   <row>:\t"
        write(STDOUT_FILENO, row_prefix, 6);

        for (int col = 0; col < 10; ++col) {
            write(STDOUT_FILENO, &player_state->board[row][col], 1);
            if (col < 9) { // Add space between cells
                write(STDOUT_FILENO, " ", 1);
            }
        }
        write(STDOUT_FILENO, "\n", 1);
    }
    return 0;
}

// Function: clearPlayerState
// player_state: PlayerState* to clear
DWORD clearPlayerState(PlayerState *player_state) {
    if (player_state == NULL) {
        return -1;
    }

    for (int row = 0; row < 10; ++row) {
        for (int col = 0; col < 10; ++col) {
            player_state->board[row][col] = '.'; // Initialize board cells to '.' (0x2e)
        }
    }

    // Reset ship counts
    player_state->ship2_count = 0;
    player_state->ship3_count = 0;
    player_state->ship4_count = 0;
    player_state->ship5_count = 0;

    return 0;
}

// Function: isGoodRow
// row_char: character representing the row ('0'-'9')
// Returns 1 if valid, 0 otherwise
DWORD isGoodRow(char row_char) {
    return (row_char >= '0') && (row_char <= '9');
}

// Function: isGoodCol
// col_char: character representing the column ('A'-'J')
// Returns 1 if valid, 0 otherwise
DWORD isGoodCol(char col_char) {
    return (col_char >= 'A') && (col_char <= 'J');
}

// Function: isOnBoard
// row_char: character representing the starting row
// col_char: character representing the starting column
// direction: 'R' for right (horizontal), 'D' for down (vertical)
// ship_size: integer size of the ship (2-5)
// Returns 1 if the ship fits on the board from the given starting point and direction, 0 otherwise
DWORD isOnBoard(char row_char, char col_char, char direction, int ship_size) {
    int row = row_char - '0';
    int col = col_char - 'A';

    if (direction == 'R') { // Right (horizontal)
        return (col + ship_size <= 10); // Check if end of ship (col + size - 1) is within board (index 0-9)
    } else if (direction == 'D') { // Down (vertical)
        return (row + ship_size <= 10); // Check if end of ship (row + size - 1) is within board (index 0-9)
    }
    return 0; // Invalid direction
}

// Function: processPlace
// player_state: PlayerState* to place ship on
// command_buffer: char* containing the P command and arguments (e.g., P1AR2)
// Returns 0 on success, -1 on error
DWORD processPlace(PlayerState *player_state, BYTE *command_buffer) {
    if (player_state == NULL || command_buffer == NULL) {
        return -1;
    }

    char row_char = command_buffer[1];
    char col_char = command_buffer[2];
    char direction = command_buffer[3];
    char ship_char = command_buffer[4]; // '2', '3', '4', '5'
    int ship_size = ship_char - '0';

    if (!isGoodRow(row_char) || !isGoodCol(col_char)) {
        return -1; // Invalid row or column character
    }

    if (!isOnBoard(row_char, col_char, direction, ship_size)) {
        return -1; // Ship does not fit on board
    }

    int start_row = row_char - '0';
    int start_col = col_char - 'A';

    DWORD *ship_count_ptr = NULL;
    switch (ship_size) {
        case 2: ship_count_ptr = &player_state->ship2_count; break;
        case 3: ship_count_ptr = &player_state->ship3_count; break;
        case 4: ship_count_ptr = &player_state->ship4_count; break;
        case 5: ship_count_ptr = &player_state->ship5_count; break;
        default: return -1; // Invalid ship size character
    }

    if (*ship_count_ptr != 0) { // If ship of this size already placed
        return -1;
    }

    // First, check if all cells for the ship are empty ('.')
    if (direction == 'R') { // Horizontal
        for (int i = 0; i < ship_size; ++i) {
            if (player_state->board[start_row][start_col + i] != '.') {
                return -1; // Cell not empty
            }
        }
    } else if (direction == 'D') { // Vertical
        for (int i = 0; i < ship_size; ++i) {
            if (player_state->board[start_row + i][start_col] != '.') {
                return -1; // Cell not empty
            }
        }
    } else {
        return -1; // Invalid direction character
    }

    // If all cells are empty, place the ship
    if (direction == 'R') { // Horizontal
        for (int i = 0; i < ship_size; ++i) {
            player_state->board[start_row][start_col + i] = ship_char;
        }
    } else { // Vertical (direction must be 'D' by this point)
        for (int i = 0; i < ship_size; ++i) {
            player_state->board[start_row + i][start_col] = ship_char;
        }
    }

    *ship_count_ptr = ship_size; // Store the ship's length (initial health)
    return 0;
}

// Function: processTarget
// opponent_state: PlayerState* of the opponent to target
// command_buffer: char* containing the T command and arguments (e.g., T1A)
// Returns 0 on success, -1 on error
DWORD processTarget(PlayerState *opponent_state, BYTE *command_buffer) {
    if (opponent_state == NULL || command_buffer == NULL) {
        return -1;
    }

    char row_char = command_buffer[1];
    char col_char = command_buffer[2];

    if (!isGoodRow(row_char) || !isGoodCol(col_char)) {
        return -1; // Invalid row or column character
    }

    int row = row_char - '0';
    int col = col_char - 'A';

    char target_cell_content = opponent_state->board[row][col];

    if (target_cell_content == '.') {
        write(STDOUT_FILENO, MISSED_MSG, strlen(MISSED_MSG));
        opponent_state->board[row][col] = 'O'; // Mark as missed
    } else if (target_cell_content == 'X' || target_cell_content == 'O') {
        write(STDOUT_FILENO, ALREADY_TARGETED_MSG, strlen(ALREADY_TARGETED_MSG));
        // No change to board state or ship counts for already targeted spots
    } else { // It's a ship ('2', '3', '4', '5')
        write(STDOUT_FILENO, HIT_MSG, strlen(HIT_MSG));
        opponent_state->board[row][col] = 'X'; // Mark as hit

        DWORD *ship_count_ptr = NULL;
        const char *sunk_msg = NULL;

        switch (target_cell_content) {
            case '2': ship_count_ptr = &opponent_state->ship2_count; sunk_msg = SHIP2_SUNK_MSG; break;
            case '3': ship_count_ptr = &opponent_state->ship3_count; sunk_msg = SHIP3_SUNK_MSG; break;
            case '4': ship_count_ptr = &opponent_state->ship4_count; sunk_msg = SHIP4_SUNK_MSG; break;
            case '5': ship_count_ptr = &opponent_state->ship5_count; sunk_msg = SHIP5_SUNK_MSG; break;
            default: return -1; // Should not happen if only valid ship chars are placed
        }

        if (ship_count_ptr != NULL) {
            (*ship_count_ptr)--; // Decrement remaining segments for the hit ship
            if (*ship_count_ptr == 0) {
                write(STDOUT_FILENO, sunk_msg, strlen(sunk_msg));
            }
        }
    }
    return 0;
}

// Function: allShipsSunk
// player_state: PlayerState* to check if all ships are sunk
// Returns 1 if all ships are sunk, 0 otherwise
DWORD allShipsSunk(PlayerState *player_state) {
    if (player_state == NULL) {
        return 0; // Invalid player state, assume no ships sunk (or error)
    }
    return (player_state->ship2_count == 0 &&
            player_state->ship3_count == 0 &&
            player_state->ship4_count == 0 &&
            player_state->ship5_count == 0);
}

// Function: main
DWORD main(void) {
    PlayerState player0_state;
    PlayerState player1_state;

    char command_buffer[5]; // Stores the parsed command, e.g., {'P', '1', 'A', 'R', '2'}
    DWORD read_status;      // Stores return value from getCommand, processPlace, processTarget
    unsigned int current_player = 0; // 0 for Player 0, 1 for Player 1
    int game_started = 0; // 0 = not started (placing ships), 1 = started (targeting)
    int game_ended = 0;   // 0 = not ended, 1 = ended (someone won)

    // Initialize command_buffer[0] to 'U' (Unknown)
    command_buffer[0] = 'U';

    clearPlayerState(&player0_state);
    clearPlayerState(&player1_state);

    while (1) { // Main game loop
        // Print prompt for current player
        if (current_player == 0) {
            write(STDOUT_FILENO, P0_PROMPT, strlen(P0_PROMPT));
        } else {
            write(STDOUT_FILENO, P1_PROMPT, strlen(P1_PROMPT));
        }

        read_status = getCommand(command_buffer);

        if (read_status == -2) { // readLine error or EOF
            write(STDOUT_FILENO, READ_ERROR_MSG, strlen(READ_ERROR_MSG));
            break; // Exit main loop, terminate program
        }

        if (read_status != 0) { // Bad command format from getCommand
            write(STDOUT_FILENO, BAD_COMMAND_MSG, strlen(BAD_COMMAND_MSG));
            continue; // Skip to next loop iteration
        }

        // Process command
        char cmd = command_buffer[0];

        if (cmd == 'E') { // Exit command
            write(STDOUT_FILENO, EXIT_MSG, strlen(EXIT_MSG));
            break; // Exit main loop, terminate program
        } else if (cmd == 'N') { // New Game command
            command_buffer[0] = 'U'; // Reset command
            clearPlayerState(&player0_state);
            clearPlayerState(&player1_state);
            current_player = 0;
            game_started = 0;
            game_ended = 0;
        } else if (cmd == 'H') { // Help command
            write(STDOUT_FILENO, HELP_MSG, strlen(HELP_MSG));
        } else if (game_ended != 0) { // If game has already ended
            write(STDOUT_FILENO, GAME_ALREADY_ENDED_MSG, strlen(GAME_ALREADY_ENDED_MSG));
        } else { // Game is ongoing or in setup phase
            if (cmd == 'M') { // My Board command
                if (current_player == 0) {
                    dumpBoard(&player0_state);
                } else {
                    dumpBoard(&player1_state);
                }
            } else if (cmd == 'S') { // Start Game command
                if (game_started == 0) {
                    game_started = 1;
                    // Note: 'S' just marks the game as started, it does not implicitly change turns.
                    // Turn changes are handled by 'P' and 'T' commands.
                } else {
                    write(STDOUT_FILENO, GAME_ALREADY_STARTED_MSG, strlen(GAME_ALREADY_STARTED_MSG));
                }
            } else if (cmd == 'P') { // Place Ship command
                if (game_started == 0) {
                    DWORD place_result;
                    if (current_player == 0) {
                        place_result = processPlace(&player0_state, command_buffer);
                    } else {
                        place_result = processPlace(&player1_state, command_buffer);
                    }

                    if (place_result == 0) {
                        current_player = (current_player == 0) ? 1 : 0; // Switch player after successful placement
                    } else {
                        write(STDOUT_FILENO, BAD_COMMAND_MSG, strlen(BAD_COMMAND_MSG));
                    }
                } else {
                    write(STDOUT_FILENO, GAME_ALREADY_STARTED_MSG, strlen(GAME_ALREADY_STARTED_MSG));
                }
            } else if (cmd == 'T') { // Target Cell command
                if (game_started == 0) {
                    write(STDOUT_FILENO, GAME_NOT_STARTED_MSG, strlen(GAME_NOT_STARTED_MSG));
                } else {
                    DWORD target_result;
                    // Determine which player's board is being targeted (the opponent's)
                    PlayerState *target_player_state = (current_player == 0) ? &player1_state : &player0_state;
                    target_result = processTarget(target_player_state, command_buffer);

                    if (target_result == 0) {
                        // Check if the current player won by sinking all opponent's ships
                        if (allShipsSunk(target_player_state)) {
                            write(STDOUT_FILENO, YOU_WIN_MSG, strlen(YOU_WIN_MSG));
                            game_started = 0; // Game setup phase again
                            game_ended = 1;   // Mark game as ended
                        }
                        current_player = (current_player == 0) ? 1 : 0; // Switch player after a valid target
                    } else {
                        write(STDOUT_FILENO, BAD_COMMAND_MSG, strlen(BAD_COMMAND_MSG));
                    }
                }
            } else { // Any other unrecognized command
                write(STDOUT_FILENO, BAD_COMMAND_MSG, strlen(BAD_COMMAND_MSG));
            }
        }
    } // End of main game loop

    return 0; // Program exits successfully
}