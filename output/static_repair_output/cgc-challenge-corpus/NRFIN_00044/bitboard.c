#include <stdio.h>    // For printf
#include <string.h>   // For memset, strcmp
#include <stdbool.h>  // For bool type
#include <stdlib.h>   // For abs

// Define standard C types for clarity and portability
typedef unsigned int uint;
typedef unsigned long long ulonglong;

// --- Constants for Piece Types and Colors ---
#define PAWN   0
#define ROOK   1
#define KNIGHT 2
#define BISHOP 3
#define QUEEN  4
#define KING   5

#define WHITE  0
#define BLACK  1

// --- Bitboard Structure ---
// Offsets from the original code:
// 0x60: white_queenside_castle_rights
// 0x64: black_kingside_castle_rights (from 100 in init)
// 0x68: white_kingside_castle_rights
// 0x6c: black_queenside_castle_rights
// 0x70: en_passant_row
// 0x74: en_passant_col
// 0x78: halfmove_clock
typedef struct {
    ulonglong boards[12]; // 6 piece types * 2 colors (type*2 + color)
    uint white_kingside_castle_rights;  // 0x68
    uint white_queenside_castle_rights; // 0x60
    uint black_kingside_castle_rights;  // 0x64 (originally 100)
    uint black_queenside_castle_rights; // 0x6c
    int en_passant_row; // 0x70, -1 if no en passant target
    int en_passant_col; // 0x74, -1 if no en passant target
    int halfmove_clock; // 0x78, for 50-move rule
    int fullmove_number; // Not in snippet, but common.
} Bitboard;

// --- Move Info Structure ---
// Based on usage in parse_san, validate_move, make_move
typedef struct {
    int color; // param_2[0]
    int piece_type; // param_2[1]
    int promotion_type; // param_2[2] (0=none, 1=R, 2=N, 3=B, 4=Q)
    int is_kingside_castle_flag; // param_2[3]
    int is_queenside_castle_flag; // param_2[4]
    int is_capture; // param_2[5]
    int is_promotion; // param_2[6]
    int src_row; // param_2[9]
    int src_col; // param_2[10]
    int dst_row; // param_2[0xb]
    int dst_col; // param_2[0xc]
} MoveInfo;

// --- Global Constants for print_bitboard ---
// These are inferred from DAT_00016000 etc.
// Assuming ANSI escape codes for colors
const char* const SQUARE_BG_COLORS[4] = {
    "\x1b[47m", // White square, color 0, index 0 (light square)
    "\x1b[40m", // Black square, color 0, index 1 (dark square)
    "\x1b[47m", // White square, color 1, index 2 (light square)
    "\x1b[40m"  // Black square, color 1, index 3 (dark square)
};

const char* const PIECE_FG_COLORS[4] = {
    "\x1b[30m", // Black piece, color 0 (WHITE)
    "\x1b[37m", // White piece, color 1 (BLACK)
    "\x1b[30m", // Black piece, color 2 (WHITE)
    "\x1b[37m"  // White piece, color 3 (BLACK)
};

const char* const PIECE_SYMBOLS[6] = {
    "P", "R", "N", "B", "Q", "K"
};

const char* const RESET_COLOR = "\x1b[0m";

// --- Function Prototypes ---
// CONCAT44 is a Ghidra macro, replaced with standard C bitwise operations
#define CONCAT44(high, low) (((ulonglong)(high)) << 32 | (low))

// piece_moves is an array of function pointers. Declared globally.
typedef ulonglong (*PieceMoveFunc)(const Bitboard*, int, int, int);
PieceMoveFunc piece_moves[6]; // One for each piece type (PAWN, ROOK, KNIGHT, BISHOP, QUEEN, KING)

// Helper for bit manipulation
static inline int get_bit(ulonglong board, int row, int col) {
    if (row < 0 || row >= 8 || col < 0 || col >= 8) return 0;
    return (int)((board >> (ulonglong)(row * 8 + col)) & 1ULL);
}

static inline void set_bit(ulonglong *board, int row, int col) {
    if (row < 0 || row >= 8 || col < 0 || col >= 8) return;
    *board |= (1ULL << (ulonglong)(row * 8 + col));
}

static inline void clear_bit(ulonglong *board, int row, int col) {
    if (row < 0 || row >= 8 || col < 0 || col >= 8) return;
    *board &= ~(1ULL << (ulonglong)(row * 8 + col));
}

// Function: make_piece
// Stores color and type into a 2-element int array/struct
int * make_piece(int *piece_info, int color, int type) {
  piece_info[0] = color;
  piece_info[1] = type;
  return piece_info;
}

// Function: set_piece
int set_piece(Bitboard *board, int row, int col, int color, int type) {
  if (row < 0 || row >= 8 || col < 0 || col >= 8) {
    return -1; // Out of bounds
  }
  uint board_idx = (uint)type * 2 + (uint)color;
  set_bit(&board->boards[board_idx], row, col);
  return 0;
}

// Function: clear_piece
int clear_piece(Bitboard *board, int row, int col) {
  if (row < 0 || row >= 8 || col < 0 || col >= 8) {
    return -1; // Out of bounds
  }
  for (int i = 0; i < 12; ++i) { // Clear from all bitboards
    clear_bit(&board->boards[i], row, col);
  }
  return 0;
}

// Function: get_piece
// Stores color in piece_info[0] and type in piece_info[1]
// Returns 1 if a piece is found, 0 if not, -1 if out of bounds.
int get_piece(const Bitboard *board, int row, int col, int *piece_info) {
  if (row < 0 || row >= 8 || col < 0 || col >= 8) {
    return -1; // Out of bounds
  }
  for (int type = 0; type < 6; ++type) {
    for (int color = 0; color < 2; ++color) {
      uint board_idx = (uint)type * 2 + (uint)color;
      if (get_bit(board->boards[board_idx], row, col)) {
        if (piece_info) {
          piece_info[0] = color;
          piece_info[1] = type;
        }
        return 1; // Piece found
      }
    }
  }
  return 0; // No piece found
}

// Function: init_bitboard
void init_bitboard(Bitboard *board) {
  memset(board, 0, sizeof(Bitboard));

  // Initialize castling rights, en passant, halfmove clock
  board->white_kingside_castle_rights = 1;  // 0x68
  board->white_queenside_castle_rights = 1; // 0x60
  board->black_kingside_castle_rights = 1;  // 0x64 (originally 100)
  board->black_queenside_castle_rights = 1; // 0x6c
  board->en_passant_row = -1; // 0x70
  board->en_passant_col = -1; // 0x74
  board->halfmove_clock = 0;  // 0x78
  board->fullmove_number = 1;

  for (int color = 0; color < 2; ++color) {
    int row = (color == WHITE) ? 0 : 7;
    int pawn_row = (color == WHITE) ? 1 : 6;

    // Rooks
    set_piece(board, row, 0, color, ROOK);
    set_piece(board, row, 7, color, ROOK);
    // Knights
    set_piece(board, row, 1, color, KNIGHT);
    set_piece(board, row, 6, color, KNIGHT);
    // Bishops
    set_piece(board, row, 2, color, BISHOP);
    set_piece(board, row, 5, color, BISHOP);
    // Queen
    set_piece(board, row, 3, color, QUEEN);
    // King
    set_piece(board, row, 4, color, KING);

    // Pawns
    for (int col = 0; col < 8; ++col) {
      set_piece(board, pawn_row, col, color, PAWN);
    }
  }
  return;
}

// Function: print_bitboard
void print_bitboard(const Bitboard *board, int inverted_view) {
  int piece_info[2]; // piece_info[0]=color, piece_info[1]=type

  for (int row_idx = 0; row_idx < 8; ++row_idx) {
    int current_row = inverted_view ? (7 - row_idx) : row_idx;
    printf("%d ", 8 - current_row); // Print row number

    for (int col_idx = 0; col_idx < 8; ++col_idx) {
      int current_col = inverted_view ? (7 - col_idx) : col_idx;

      // Determine square color for background
      int square_color_idx = ((current_row % 2 + current_col % 2) % 2); // 0 for light, 1 for dark
      const char* bg_color = SQUARE_BG_COLORS[square_color_idx * 2]; // Always use index 0 or 2 for BG color

      int piece_found = get_piece(board, current_row, current_col, piece_info);
      if (piece_found == 1) {
        int piece_color = piece_info[0]; // WHITE or BLACK
        int piece_type = piece_info[1];
        // Index for PIECE_FG_COLORS: (piece_color * 2) + square_color_idx (0 for light, 1 for dark)
        // 0: White piece on Light square, 1: White piece on Dark square
        // 2: Black piece on Light square, 3: Black piece on Dark square
        const char* fg_color = PIECE_FG_COLORS[piece_color * 2 + square_color_idx];
        printf("%s%s%s %s", bg_color, fg_color, PIECE_SYMBOLS[piece_type], RESET_COLOR);
      } else {
        printf("%s  %s", bg_color, RESET_COLOR); // Empty square
      }
    }
    printf("\n");
  }

  printf("  "); // Spacer for column labels
  if (inverted_view == 0) {
    for (char c = 'a'; c <= 'h'; ++c) {
      printf("%c ", c);
    }
  } else {
    for (char c = 'h'; c >= 'a'; --c) {
      printf("%c ", c);
    }
  }
  printf("\n");
  return;
}

// Function: pawn_moves
ulonglong pawn_moves(const Bitboard *board, int piece_color, int row, int col) {
  ulonglong moves = 0ULL;
  int piece_info[2];
  int direction = (piece_color == WHITE) ? 1 : -1;

  // Single push
  if (get_piece(board, row + direction, col, NULL) == 0) {
    set_bit(&moves, row + direction, col);
    // Double push from starting rank
    int start_rank = (piece_color == WHITE) ? 1 : 6;
    if (row == start_rank && get_piece(board, row + 2 * direction, col, NULL) == 0) {
      set_bit(&moves, row + 2 * direction, col);
    }
  }

  // Captures (diagonal)
  // Left diagonal
  if (col > 0 && get_piece(board, row + direction, col - 1, piece_info) == 1 && piece_info[0] != piece_color) {
    set_bit(&moves, row + direction, col - 1);
  }
  // Right diagonal
  if (col < 7 && get_piece(board, row + direction, col + 1, piece_info) == 1 && piece_info[0] != piece_color) {
    set_bit(&moves, row + direction, col + 1);
  }

  // En passant
  if (board->en_passant_row != -1 && board->en_passant_col != -1) {
    if ((row + direction == board->en_passant_row) &&
        (col - 1 == board->en_passant_col || col + 1 == board->en_passant_col)) {
      set_bit(&moves, board->en_passant_row, board->en_passant_col);
    }
  }

  return moves;
}

// Function: rook_moves
ulonglong rook_moves(const Bitboard *board, int piece_color, int row, int col) {
  ulonglong moves = 0ULL;
  int piece_info[2];
  int directions[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}}; // N, S, E, W

  for (int i = 0; i < 4; ++i) {
    int dr = directions[i][0];
    int dc = directions[i][1];
    int curr_row = row + dr;
    int curr_col = col + dc;

    while (curr_row >= 0 && curr_row < 8 && curr_col >= 0 && curr_col < 8) {
      int piece_found = get_piece(board, curr_row, curr_col, piece_info);
      if (piece_found == 0) { // Empty square
        set_bit(&moves, curr_row, curr_col);
      } else if (piece_found == 1) { // Piece found
        if (piece_info[0] != piece_color) { // Opponent's piece (capture)
          set_bit(&moves, curr_row, curr_col);
        }
        break; // Blocked by own or opponent's piece
      } else { // Out of bounds
        break;
      }
      curr_row += dr;
      curr_col += dc;
    }
  }
  return moves;
}

// Function: knight_moves
ulonglong knight_moves(const Bitboard *board, int piece_color, int row, int col) {
  ulonglong moves = 0ULL;
  int piece_info[2];
  int knight_moves_offsets[8][2] = {
    {2, 1}, {1, 2}, {-1, 2}, {-2, 1},
    {-2, -1}, {-1, -2}, {1, -2}, {2, -1}
  };

  for (int i = 0; i < 8; ++i) {
    int target_row = row + knight_moves_offsets[i][0];
    int target_col = col + knight_moves_offsets[i][1];

    int piece_found = get_piece(board, target_row, target_col, piece_info);
    if (piece_found == 0) { // Empty square
      set_bit(&moves, target_row, target_col);
    } else if (piece_found == 1) { // Piece found
      if (piece_info[0] != piece_color) { // Opponent's piece (capture)
        set_bit(&moves, target_row, target_col);
      }
    }
  }
  return moves;
}

// Function: bishop_moves
ulonglong bishop_moves(const Bitboard *board, int piece_color, int row, int col) {
  ulonglong moves = 0ULL;
  int piece_info[2];
  int directions[4][2] = {{1, 1}, {-1, 1}, {1, -1}, {-1, -1}}; // NE, NW, SE, SW

  for (int i = 0; i < 4; ++i) {
    int dr = directions[i][0];
    int dc = directions[i][1];
    int curr_row = row + dr;
    int curr_col = col + dc;

    while (curr_row >= 0 && curr_row < 8 && curr_col >= 0 && curr_col < 8) {
      int piece_found = get_piece(board, curr_row, curr_col, piece_info);
      if (piece_found == 0) { // Empty square
        set_bit(&moves, curr_row, curr_col);
      } else if (piece_found == 1) { // Piece found
        if (piece_info[0] != piece_color) { // Opponent's piece (capture)
          set_bit(&moves, curr_row, curr_col);
        }
        break; // Blocked by own or opponent's piece
      } else { // Out of bounds
        break;
      }
      curr_row += dr;
      curr_col += dc;
    }
  }
  return moves;
}

// Function: queen_moves
ulonglong queen_moves(const Bitboard *board, int piece_color, int row, int col) {
  ulonglong bishop_m = bishop_moves(board, piece_color, row, col);
  ulonglong rook_m = rook_moves(board, piece_color, row, col);
  return bishop_m | rook_m;
}

// Function: is_at_risk
// Checks if a square (king_row, king_col) is attacked by opponent's pieces
// check_king_moves_only: 1 if only checking squares for king movement, 0 for general risk
bool is_at_risk(const Bitboard *board, int king_color, int king_row, int king_col, int check_king_moves_only) {
  int opponent_color = 1 - king_color;

  // Iterate through all opponent's pieces
  // If check_king_moves_only is true, we don't need to consider opponent kings for attack
  // (as kings cannot attack adjacent squares if it puts themselves in check)
  int max_piece_type = check_king_moves_only ? KING : KING; // All piece types can attack
  
  for (int type = 0; type <= max_piece_type; ++type) {
    uint board_idx = (uint)type * 2 + (uint)opponent_color;
    ulonglong opponent_board = board->boards[board_idx];

    for (int r = 0; r < 8; ++r) {
      for (int c = 0; c < 8; ++c) {
        if (get_bit(opponent_board, r, c)) { // If opponent has a piece at (r, c)
          ulonglong opponent_piece_moves = piece_moves[type](board, opponent_color, r, c);
          
          // Check if any of these moves target the king's square
          if (get_bit(opponent_piece_moves, king_row, king_col)) {
            return true; // King is at risk
          }
        }
      }
    }
  }
  return false; // King is not at risk
}

// Function: king_moves
ulonglong king_moves(const Bitboard *board, int piece_color, int row, int col) {
  ulonglong moves = 0ULL;
  int piece_info[2];
  int king_moves_offsets[8][2] = {
    {1, 1}, {0, 1}, {-1, 1}, {-1, 0},
    {-1, -1}, {0, -1}, {1, -1}, {1, 0}
  };

  for (int i = 0; i < 8; ++i) {
    int target_row = row + king_moves_offsets[i][0];
    int target_col = col + king_moves_offsets[i][1];

    if (target_row >= 0 && target_row < 8 && target_col >= 0 && target_col < 8) {
      int piece_found = get_piece(board, target_row, target_col, piece_info);
      if (piece_found == 0) { // Empty square
        // Check if moving to this square puts the king in check
        if (!is_at_risk(board, piece_color, target_row, target_col, 1)) {
          set_bit(&moves, target_row, target_col);
        }
      } else if (piece_found == 1) { // Piece found
        if (piece_info[0] != piece_color) { // Opponent's piece (capture)
          if (!is_at_risk(board, piece_color, target_row, target_col, 1)) {
            set_bit(&moves, target_row, target_col);
          }
        }
      }
    }
  }
  return moves;
}

// Function: is_checkmate
// king_color: color of the king to check
// king_row, king_col: position of the king
bool is_checkmate(const Bitboard *board, int king_color, int king_row, int king_col) {
  // Check if king is currently at risk
  if (is_at_risk(board, king_color, king_row, king_col, 0)) {
    // If king has no valid moves, it's checkmate
    if (king_moves(board, king_color, king_row, king_col) == 0ULL) {
        // Also need to check if any other piece can block the check or capture the attacker.
        // This is a complex check, but the original code only checks king moves.
        // For now, we'll stick to the original logic which only checks if the king can move.
        // A full checkmate detection involves iterating through all possible moves of all pieces
        // to see if any move can resolve the check.
        
        // This simplified check for checkmate (only checking king moves) is generally insufficient.
        // However, replicating the given snippet's logic:
        return true;
    }
  }
  return false;
}

// Function: is_stalemate
bool is_stalemate(const Bitboard *board, int current_turn_color) {
  // 50-move rule check
  if (board->halfmove_clock >= 100) { // 100 half-moves = 50 full moves
    return true;
  }

  // Find the king's position for current_turn_color
  int king_row = -1, king_col = -1;
  ulonglong king_board = board->boards[KING * 2 + current_turn_color];
  for (int r = 0; r < 8; ++r) {
    for (int c = 0; c < 8; ++c) {
      if (get_bit(king_board, r, c)) {
        king_row = r;
        king_col = c;
        break;
      }
    }
    if (king_row != -1) break;
  }

  // If king is not found (shouldn't happen in a valid game state), return false
  if (king_row == -1) return false;

  // Check if current player is in check
  bool in_check = is_at_risk(board, current_turn_color, king_row, king_col, 0);

  // If in check, it's not stalemate (could be checkmate, handled by is_checkmate)
  if (in_check) return false;

  // If not in check, check if there are ANY legal moves for current player
  for (int type = 0; type < 6; ++type) {
    uint board_idx = (uint)type * 2 + (uint)current_turn_color;
    ulonglong player_piece_board = board->boards[board_idx];

    for (int r = 0; r < 8; ++r) {
      for (int c = 0; c < 8; ++c) {
        if (get_bit(player_piece_board, r, c)) { // If player has a piece at (r, c)
          ulonglong possible_moves = piece_moves[type](board, current_turn_color, r, c);

          // For each possible move, check if it leaves the king in check
          for (int tr = 0; tr < 8; ++tr) {
            for (int tc = 0; tc < 8; ++tc) {
              if (get_bit(possible_moves, tr, tc)) {
                // Simulate the move
                Bitboard temp_board = *board; // Copy current board state
                int piece_info[2];
                get_piece(&temp_board, r, c, piece_info); // Get piece to move
                
                clear_piece(&temp_board, r, c); // Clear source
                int captured_piece_row = -1, captured_piece_col = -1;
                // Check if it's an en passant capture
                if (type == PAWN && c != tc && get_piece(board, tr, tc, NULL) == 0) {
                    captured_piece_row = r;
                    captured_piece_col = tc;
                    clear_piece(&temp_board, captured_piece_row, captured_piece_col);
                } else {
                    clear_piece(&temp_board, tr, tc); // Clear destination (for captures)
                }
                
                set_piece(&temp_board, tr, tc, piece_info[0], piece_info[1]); // Set piece at destination

                // Find king's new position (it might have moved)
                int new_king_row = king_row, new_king_col = king_col;
                if (type == KING) {
                    new_king_row = tr;
                    new_king_col = tc;
                }
                
                // If this move does NOT result in the king being at risk, then it's a legal move
                if (!is_at_risk(&temp_board, current_turn_color, new_king_row, new_king_col, 0)) {
                  return false; // Found a legal move, not stalemate
                }
              }
            }
          }
        }
      }
    }
  }
  return true; // No legal moves found, and not in check = stalemate
}

// Function: infer_src
// Tries to infer the source square (src_row, src_col) given a destination and piece type/color
// Returns 0 on success, -1 on failure
int infer_src(const Bitboard *board, MoveInfo *move_info) {
  // Iterate through all possible source squares for the given piece type and color
  ulonglong candidate_src_board = board->boards[move_info->piece_type * 2 + move_info->color];

  for (int r = 0; r < 8; ++r) {
    for (int c = 0; c < 8; ++c) {
      if (get_bit(candidate_src_board, r, c)) { // If there's a piece of the correct type/color at (r,c)
        ulonglong possible_moves = piece_moves[move_info->piece_type](board, move_info->color, r, c);

        // Check if this piece can move to the destination (dst_row, dst_col)
        if (get_bit(possible_moves, move_info->dst_row, move_info->dst_col)) {
          // If disambiguation is provided, check if it matches
          if ((move_info->src_row == -1 || move_info->src_row == r) &&
              (move_info->src_col == -1 || move_info->src_col == c)) {

            // Simulate the move to check if it leaves the king in check
            Bitboard temp_board = *board;
            clear_piece(&temp_board, r, c);
            int captured_piece_row = -1, captured_piece_col = -1;
            // Check if it's an en passant capture
            if (move_info->piece_type == PAWN && c != move_info->dst_col && get_piece(board, move_info->dst_row, move_info->dst_col, NULL) == 0) {
                captured_piece_row = r;
                captured_piece_col = move_info->dst_col;
                clear_piece(&temp_board, captured_piece_row, captured_piece_col);
            } else {
                clear_piece(&temp_board, move_info->dst_row, move_info->dst_col); // Clear potential captured piece
            }
            
            set_piece(&temp_board, move_info->dst_row, move_info->dst_col, move_info->color, move_info->piece_type);

            // Find king's position
            int king_r = -1, king_c = -1;
            ulonglong king_b = temp_board.boards[KING * 2 + move_info->color];
            for (int kr = 0; kr < 8; ++kr) {
                for (int kc = 0; kc < 8; ++kc) {
                    if (get_bit(king_b, kr, kc)) {
                        king_r = kr; king_c = kc; break;
                    }
                }
                if (king_r != -1) break;
            }

            if (!is_at_risk(&temp_board, move_info->color, king_r, king_c, 0)) {
                // This is a legal move
                move_info->src_row = r;
                move_info->src_col = c;
                return 0; // Source inferred successfully
            }
          }
        }
      }
    }
  }
  return -1; // Could not infer source
}

// Function: can_castle
// is_queenside: 0 for kingside, 1 for queenside
int can_castle(const Bitboard *board, int color, int is_queenside) {
  int king_row = (color == WHITE) ? 0 : 7;
  int king_col = 4;
  int rook_col = is_queenside ? 0 : 7;

  // Check if castling rights exist
  if (color == WHITE) {
    if (is_queenside && !board->white_queenside_castle_rights) return 0;
    if (!is_queenside && !board->white_kingside_castle_rights) return 0;
  } else { // BLACK
    if (is_queenside && !board->black_queenside_castle_rights) return 0;
    if (!is_queenside && !board->black_kingside_castle_rights) return 0;
  }

  // Check if king and rook are in original positions
  int piece_info[2];
  if (get_piece(board, king_row, king_col, piece_info) != 1 || piece_info[0] != color || piece_info[1] != KING) return 0;
  if (get_piece(board, king_row, rook_col, piece_info) != 1 || piece_info[0] != color || piece_info[1] != ROOK) return 0;

  // Check if squares between king and rook are empty and not under attack
  int start_path_col = is_queenside ? 1 : 5;
  int end_path_col = is_queenside ? 3 : 6;

  for (int col = start_path_col; col <= end_path_col; ++col) {
    if (get_piece(board, king_row, col, NULL) != 0) { // Square is not empty
      return 0;
    }
    // King cannot pass through or land on an attacked square
    if (col == king_col || col == king_col + (is_queenside ? -1 : 1) || col == king_col + (is_queenside ? -2 : 2)) {
        if (is_at_risk(board, color, king_row, col, 0)) {
            return 0;
        }
    }
  }
  
  // Also check the king's start position for attack
  if (is_at_risk(board, color, king_row, king_col, 0)) return 0;

  return 1; // Can castle
}

// Function: validate_move
// Returns 1 on valid, 0 on invalid
int validate_move(const Bitboard *board, MoveInfo *move_info) {
  if (move_info->src_row == -1 || move_info->src_col == -1 ||
      move_info->dst_row == -1 || move_info->dst_col == -1) {
    return 0; // Incomplete move info
  }

  int piece_info[2]; // piece_info[0]=color, piece_info[1]=type
  int src_piece_found = get_piece(board, move_info->src_row, move_info->src_col, piece_info);

  // Check if source square has a piece of the correct color and type
  if (src_piece_found != 1 || piece_info[0] != move_info->color || piece_info[1] != move_info->piece_type) {
    return 0;
  }

  // Handle castling as a special case
  if (move_info->is_kingside_castle_flag || move_info->is_queenside_castle_flag) {
    return can_castle(board, move_info->color, move_info->is_queenside_castle_flag);
  }

  // Get all possible moves for the piece at src_row, src_col
  ulonglong possible_moves = piece_moves[move_info->piece_type](board, move_info->color, move_info->src_row, move_info->src_col);

  // Check if destination square is in the set of possible moves
  if (!get_bit(possible_moves, move_info->dst_row, move_info->dst_col)) {
    return 0; // Destination is not a valid move for this piece
  }

  // Check if destination has a piece of the same color (cannot capture own piece)
  int dst_piece_found = get_piece(board, move_info->dst_row, move_info->dst_col, piece_info);
  if (dst_piece_found == 1 && piece_info[0] == move_info->color) {
    return 0; // Cannot capture own piece
  }

  // En passant specific check:
  // If pawn moves diagonally to an empty square, it must be an en passant capture
  if (move_info->piece_type == PAWN && move_info->src_col != move_info->dst_col && dst_piece_found == 0) {
      if (!(move_info->dst_row == board->en_passant_row && move_info->dst_col == board->en_passant_col)) {
          return 0; // Invalid pawn diagonal move to empty square (not en passant)
      }
      move_info->is_capture = 1; // Mark as capture for en passant
  }
  
  // Simulate the move to check for leaving king in check
  Bitboard temp_board = *board;
  int src_piece_color = move_info->color;
  int src_piece_type = move_info->piece_type;

  clear_piece(&temp_board, move_info->src_row, move_info->src_col);
  
  // If en passant, clear the captured pawn
  if (move_info->piece_type == PAWN && move_info->is_capture &&
      move_info->dst_row == board->en_passant_row && move_info->dst_col == board->en_passant_col) {
      int captured_pawn_row = move_info->src_row; // Pawn always captured on its rank
      int captured_pawn_col = move_info->dst_col;
      clear_piece(&temp_board, captured_pawn_row, captured_pawn_col);
  } else {
      clear_piece(&temp_board, move_info->dst_row, move_info->dst_col); // Clear potential captured piece
  }
  
  // For promotion, set the promoted piece type
  int final_piece_type = src_piece_type;
  if (move_info->is_promotion) {
      final_piece_type = move_info->promotion_type;
  }

  set_piece(&temp_board, move_info->dst_row, move_info->dst_col, src_piece_color, final_piece_type);

  // Find king's current position for the player making the move
  int king_r = -1, king_c = -1;
  ulonglong king_board = temp_board.boards[KING * 2 + src_piece_color];
  for (int r = 0; r < 8; ++r) {
      for (int c = 0; c < 8; ++c) {
          if (get_bit(king_board, r, c)) {
              king_r = r; king_c = c; break;
          }
      }
      if (king_r != -1) break;
  }
  
  // If the king is at risk after the move, it's an invalid move
  if (is_at_risk(&temp_board, src_piece_color, king_r, king_c, 0)) {
    return 0;
  }

  return 1; // Move is valid
}

// Function: make_move
// Returns 0: success, 1: check, 2: checkmate, 3: stalemate, -1: invalid move
int make_move(Bitboard *board, MoveInfo *move_info) {
  if (!validate_move(board, move_info)) {
    return -1; // Invalid move
  }

  // Reset halfmove clock if it's a pawn move or a capture
  if (move_info->piece_type == PAWN || move_info->is_capture) {
    board->halfmove_clock = 0;
  } else {
    board->halfmove_clock++;
  }

  // Clear en passant target from previous turn
  board->en_passant_row = -1;
  board->en_passant_col = -1;

  // Handle castling
  if (move_info->is_kingside_castle_flag || move_info->is_queenside_castle_flag) {
    int king_row = (move_info->color == WHITE) ? 0 : 7;
    int king_src_col = 4;
    int king_dst_col;
    int rook_src_col;
    int rook_dst_col;

    if (move_info->is_kingside_castle_flag) {
      king_dst_col = 6;
      rook_src_col = 7;
      rook_dst_col = 5;
    } else { // Queenside
      king_dst_col = 2;
      rook_src_col = 0;
      rook_dst_col = 3;
    }

    clear_piece(board, king_row, king_src_col);
    clear_piece(board, king_row, rook_src_col);
    set_piece(board, king_row, king_dst_col, move_info->color, KING);
    set_piece(board, king_row, rook_dst_col, move_info->color, ROOK);

    // Revoke castling rights for this color
    if (move_info->color == WHITE) {
      board->white_kingside_castle_rights = 0;
      board->white_queenside_castle_rights = 0;
    } else {
      board->black_kingside_castle_rights = 0;
      board->black_queenside_castle_rights = 0;
    }
  } else { // Regular move
    // Handle en passant capture (if pawn moved diagonally to an empty en passant target square)
    if (move_info->piece_type == PAWN && move_info->is_capture &&
        move_info->dst_row == board->en_passant_row && move_info->dst_col == board->en_passant_col) {
        int captured_pawn_row = move_info->src_row; // Pawn always captured on its rank
        int captured_pawn_col = move_info->dst_col;
        clear_piece(board, captured_pawn_row, captured_pawn_col);
    } else {
        // Clear captured piece at destination if not en passant
        clear_piece(board, move_info->dst_row, move_info->dst_col);
    }
    
    clear_piece(board, move_info->src_row, move_info->src_col);

    int final_piece_type = move_info->piece_type;
    if (move_info->is_promotion) {
      final_piece_type = move_info->promotion_type;
    }
    set_piece(board, move_info->dst_row, move_info->dst_col, move_info->color, final_piece_type);

    // Set new en passant target if pawn moved two squares
    if (move_info->piece_type == PAWN && abs(move_info->src_row - move_info->dst_row) == 2) {
      board->en_passant_row = (move_info->src_row + move_info->dst_row) / 2;
      board->en_passant_col = move_info->dst_col;
    }

    // Revoke castling rights if king or rook moved
    if (move_info->piece_type == KING) {
      if (move_info->color == WHITE) {
        board->white_kingside_castle_rights = 0;
        board->white_queenside_castle_rights = 0;
      } else {
        board->black_kingside_castle_rights = 0;
        board->black_queenside_castle_rights = 0;
      }
    } else if (move_info->piece_type == ROOK) {
      if (move_info->color == WHITE) {
        if (move_info->src_row == 0 && move_info->src_col == 0) board->white_queenside_castle_rights = 0;
        if (move_info->src_row == 0 && move_info->src_col == 7) board->white_kingside_castle_rights = 0;
      } else { // BLACK
        if (move_info->src_row == 7 && move_info->src_col == 0) board->black_queenside_castle_rights = 0;
        if (move_info->src_row == 7 && move_info->src_col == 7) board->black_kingside_castle_rights = 0;
      }
    }
  }

  // After move, check game state for the OPPONENT
  int opponent_color = 1 - move_info->color;
  int king_r = -1, king_c = -1;
  ulonglong king_b = board->boards[KING * 2 + opponent_color];
  for (int r = 0; r < 8; ++r) {
      for (int c = 0; c < 8; ++c) {
          if (get_bit(king_b, r, c)) {
              king_r = r; king_c = c; break;
          }
      }
      if (king_r != -1) break;
  }

  // If opponent's king not found (error state), return 0
  if (king_r == -1) return 0;

  if (is_stalemate(board, opponent_color)) {
    return 3; // Stalemate
  } else if (is_checkmate(board, opponent_color, king_r, king_c)) {
    return 2; // Checkmate
  } else if (is_at_risk(board, opponent_color, king_r, king_c, 0)) {
    return 1; // Check
  }

  return 0; // Move successful
}

// Function: parse_san
// board: current board state
// current_player_color: color of the player making the move
// san_string: SAN string of the move
// move_info: struct to populate with parsed move details
// This is a simplified SAN parser. It handles "O-O", "O-O-O", and basic moves like "e2e4" (pawn), "Nb1c3" (knight).
// It does not handle full SAN grammar (disambiguation, checks, mates, etc.) beyond basic source/destination.
// Returns 0 on success, -1 on failure
int parse_san(const Bitboard *board, int current_player_color, const char *san_string, MoveInfo *move_info) {
  memset(move_info, 0, sizeof(MoveInfo));
  move_info->color = current_player_color;
  move_info->src_row = -1;
  move_info->src_col = -1;
  move_info->dst_row = -1;
  move_info->dst_col = -1;
  move_info->promotion_type = -1; // Default to no promotion

  // Handle castling
  if (strcmp(san_string, "O-O") == 0 || strcmp(san_string, "0-0") == 0) {
    move_info->piece_type = KING;
    move_info->is_kingside_castle_flag = 1;
    move_info->src_row = (current_player_color == WHITE) ? 0 : 7;
    move_info->src_col = 4;
    move_info->dst_row = move_info->src_row;
    move_info->dst_col = 6; // Kingside castle destination
    return 0;
  }
  if (strcmp(san_string, "O-O-O") == 0 || strcmp(san_string, "0-0-0") == 0) {
    move_info->piece_type = KING;
    move_info->is_queenside_castle_flag = 1;
    move_info->src_row = (current_player_color == WHITE) ? 0 : 7;
    move_info->src_col = 4;
    move_info->dst_row = move_info->src_row;
    move_info->dst_col = 2; // Queenside castle destination
    return 0;
  }

  // Basic parsing for "e2e4", "Nb1c3", etc.
  int len = strlen(san_string);
  if (len < 2) return -1; // Minimum length for a destination like "e4"

  int current_char_idx = 0;
  char piece_char = san_string[0];

  // Determine piece type (default to pawn if no piece character)
  if (piece_char == 'N') move_info->piece_type = KNIGHT;
  else if (piece_char == 'B') move_info->piece_type = BISHOP;
  else if (piece_char == 'R') move_info->piece_type = ROOK;
  else if (piece_char == 'Q') move_info->piece_type = QUEEN;
  else if (piece_char == 'K') move_info->piece_type = KING;
  else {
    move_info->piece_type = PAWN;
    current_char_idx = 0; // No piece char, first char is part of source/destination
  }
  
  if (move_info->piece_type != PAWN) {
      current_char_idx = 1; // Piece char was present, move to next char
  } else {
      current_char_idx = 0; // No piece char, start parsing from the beginning
  }

  // Check for capture indicator 'x' (e.g., "Nxf3", "exd5")
  if (san_string[current_char_idx] == 'x') {
      move_info->is_capture = 1;
      current_char_idx++;
  }

  // Parse destination square (e.g., "e4")
  if (current_char_idx + 1 < len &&
      san_string[current_char_idx] >= 'a' && san_string[current_char_idx] <= 'h' &&
      san_string[current_char_idx+1] >= '1' && san_string[current_char_idx+1] <= '8') {
    move_info->dst_col = san_string[current_char_idx] - 'a';
    move_info->dst_row = san_string[current_char_idx+1] - '1';
    current_char_idx += 2;
  } else {
    return -1; // Invalid destination format
  }

  // Check for promotion (e.g., "e8=Q")
  if (current_char_idx < len && san_string[current_char_idx] == '=') {
      move_info->is_promotion = 1;
      current_char_idx++;
      if (current_char_idx < len) {
          char promo_char = san_string[current_char_idx];
          if (promo_char == 'R') move_info->promotion_type = ROOK;
          else if (promo_char == 'N') move_info->promotion_type = KNIGHT;
          else if (promo_char == 'B') move_info->promotion_type = BISHOP;
          else if (promo_char == 'Q') move_info->promotion_type = QUEEN;
          else return -1; // Invalid promotion piece
      } else return -1; // Missing promotion piece
  }

  // The original Ghidra code inferred source if not provided in the move string.
  // This is a minimal implementation, a full SAN parser is complex.
  if (infer_src(board, move_info) != 0) {
    return -1; // Failed to infer source
  }

  return 0; // Success
}

// --- Main function to initialize piece_moves array ---
void init_piece_moves_array() {
    piece_moves[PAWN] = pawn_moves;
    piece_moves[ROOK] = rook_moves;
    piece_moves[KNIGHT] = knight_moves;
    piece_moves[BISHOP] = bishop_moves;
    piece_moves[QUEEN] = queen_moves;
    piece_moves[KING] = king_moves;
}