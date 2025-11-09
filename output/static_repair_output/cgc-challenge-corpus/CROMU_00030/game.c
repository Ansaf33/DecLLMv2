#include <stdio.h>   // For printf
#include <stdlib.h>  // For rand, srand
#include <time.h>    // For time

// Custom type definitions to match the snippet's decompiled output
typedef char undefined;
typedef int undefined4;

// Global game state structure
// 0x80 is derived from the check `(int)game_stack._1152_4_ < 0x80`
struct GameStack {
    char pieces[0x80 * 9]; // Array to store piece data (each piece is 9 bytes)
    int stack_ptr;         // Acts as a stack pointer or current top index
};

// Global instance of the game stack
struct GameStack game_stack;

// Global variable for current maximum road length
int current_max_road_len;

// Helper function for random number generation within a range
// (Not provided in snippet, but used by create_random_piece)
int random_in_range(int min, int max) {
    return min + rand() % (max - min + 1);
}

// Function: getNextPieceNum
int getNextPieceNum(void) {
    if (game_stack.stack_ptr == -1) {
        return 0;
    }
    return game_stack.pieces[game_stack.stack_ptr * 9] + 1;
}

// Function: isTopPiecePlaced
undefined4 isTopPiecePlaced(void) {
    if (game_stack.stack_ptr < 0) {
        return 1; // If no pieces, it's considered "placed" (no unplaced top piece)
    }
    for (int i = 0; i < 4; i++) {
        // -4 is 0xFC, -3 is 0xFD when interpreted as signed char
        if ((game_stack.pieces[game_stack.stack_ptr * 9 + i * 2 + 1] != (char)0xfc) &&
            (game_stack.pieces[game_stack.stack_ptr * 9 + i * 2 + 1] != (char)0xfd)) {
            return 1; // Found a connection that is not 0xFC or 0xFD, meaning it's placed.
        }
    }
    return 0; // All connections are 0xFC or 0xFD, meaning it's not placed.
}

// Function: push_piece
void push_piece(undefined *param_1) {
    if (game_stack.stack_ptr < 0x80) {
        game_stack.stack_ptr++;
        // Copy 9 bytes of piece data
        game_stack.pieces[game_stack.stack_ptr * 9] = param_1[0];
        game_stack.pieces[game_stack.stack_ptr * 9 + 1] = param_1[1];
        game_stack.pieces[game_stack.stack_ptr * 9 + 2] = param_1[2];
        game_stack.pieces[game_stack.stack_ptr * 9 + 3] = param_1[3];
        game_stack.pieces[game_stack.stack_ptr * 9 + 4] = param_1[4];
        game_stack.pieces[game_stack.stack_ptr * 9 + 5] = param_1[5];
        game_stack.pieces[game_stack.stack_ptr * 9 + 6] = param_1[6];
        game_stack.pieces[game_stack.stack_ptr * 9 + 7] = param_1[7];
        game_stack.pieces[game_stack.stack_ptr * 9 + 8] = param_1[8];
    }
}

// Function: piece_to_pkt
void piece_to_pkt(char *param_1, char *param_2) {
    *param_2 = *param_1 + '0';
    // -3 is 0xFD
    param_2[1] = '0'; param_2[2] = (param_1[1] == (char)0xfd) ? 'y' : 'n';
    param_2[3] = '1'; param_2[4] = (param_1[3] == (char)0xfd) ? 'y' : 'n';
    param_2[5] = '2'; param_2[6] = (param_1[5] == (char)0xfd) ? 'y' : 'n';
    param_2[7] = '3'; param_2[8] = (param_1[7] == (char)0xfd) ? 'y' : 'n';
    param_2[9] = '\0'; // Null-terminate for safe printf %s
}

// Function: create_random_piece
undefined4 create_random_piece(undefined *param_1) {
    if (game_stack.stack_ptr < 0x80) {
        if (isTopPiecePlaced() == 0 && game_stack.stack_ptr > 0) {
            return 0x16;
        } else {
            *param_1 = getNextPieceNum();
            if (game_stack.stack_ptr == -1) {
                int r_val = random_in_range(1, 3);
                for (int i = 0; i < 4; i++) {
                    param_1[i * 2 + 1] = (i == r_val) ? (char)0xfd : (char)0xfc; // -3 (0xFD) or -4 (0xFC)
                }
            } else {
                int r_val1 = random_in_range(0, 3);
                int r_val2 = random_in_range(0, 3);
                while (r_val2 == r_val1) {
                    r_val2 = random_in_range(0, 3);
                }
                for (int i = 0; i < 4; i++) {
                    param_1[i * 2 + 1] = ((i == r_val1) || (i == r_val2)) ? (char)0xfd : (char)0xfc;
                }
            }
            if (game_stack.stack_ptr < 0) { // This condition is true only if it's the very first piece
                current_max_road_len++;
            }
            push_piece(param_1);
            return 1;
        }
    } else {
        return 0x21;
    }
}

// Function: discard_piece
undefined4 discard_piece(void) {
    if (game_stack.stack_ptr < 0) {
        return 0;
    }
    if (isTopPiecePlaced() == 1) {
        return 0;
    }
    game_stack.stack_ptr--;
    return 1;
}

// Function: connect_pieces
undefined4 connect_pieces(undefined *param_1, char param_2, undefined *param_3, char param_4) {
    char piece_pkt_buf[10]; // Buffer for piece_to_pkt

    // -3 is 0xFD
    if ((param_1[param_2 * 2 + 1] == (char)0xfd) && (param_3[param_4 * 2 + 1] == (char)0xfd)) {
        param_1[param_2 * 2 + 1] = *param_3;
        param_1[param_2 * 2 + 2] = param_4;
        param_3[param_4 * 2 + 1] = *param_1;
        param_3[param_4 * 2 + 2] = param_2;
        current_max_road_len++;
        return 1;
    } else {
        // Fixed printf format string from @d to %d and added newline
        printf("a: %d, b: %d...\n", (int)param_1[param_2 * 2 + 1], (int)param_3[param_4 * 2 + 1]);
        for (int i = 0; i <= game_stack.stack_ptr; i++) {
            piece_to_pkt(&game_stack.pieces[i * 9], piece_pkt_buf); // Pass address of piece
            // Fixed printf format string and added newline
            printf("piece %d: %s\n", i, piece_pkt_buf);
        }
        return 0;
    }
}

// Function: get_max_road_len
int get_max_road_len(void) {
    return current_max_road_len;
}

// Function: get_piece
int get_piece(char param_1) {
    for (int i = 0; i <= game_stack.stack_ptr; i++) {
        if (param_1 == game_stack.pieces[i * 9]) {
            return i;
        }
    }
    return -1;
}

// Main function (example usage for compilation)
int main() {
    // Initialize random seed
    srand(time(NULL));

    // Initialize game state
    game_stack.stack_ptr = -1;
    current_max_road_len = 0;

    printf("Game initialized.\n");

    // Example usage:
    // Create a piece
    undefined new_piece[9];
    undefined4 status = create_random_piece(new_piece);
    printf("Create random piece status: %d (Piece 0: %d)\n", status, new_piece[0]);

    // Create another piece
    undefined new_piece2[9];
    status = create_random_piece(new_piece2);
    printf("Create random piece status: %d (Piece 1: %d)\n", status, new_piece2[0]);

    // Try to connect pieces (assuming valid indices and connectable ports)
    // This example will likely fail as piece ports are random 0xFC/0xFD
    printf("Attempting to connect piece 0, port 0 to piece 1, port 0...\n");
    status = connect_pieces(&game_stack.pieces[0 * 9], 0, &game_stack.pieces[1 * 9], 0);
    printf("Connect pieces status: %d\n", status);

    printf("Max road length: %d\n", get_max_road_len());

    // Discard a piece
    status = discard_piece();
    printf("Discard piece status: %d\n", status);
    printf("Game stack pointer: %d\n", game_stack.stack_ptr);

    return 0;
}