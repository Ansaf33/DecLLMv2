#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h> // For uint32_t, uint64_t, int32_t
#include <math.h>   // For abs in pawn_moves

// Constants for piece types and colors
#define WHITE 0
#define BLACK 1

#define PAWN   0
#define ROOK   1
#define KNIGHT 2
#define BISHOP 3
#define QUEEN  4
#define KING   5

// A structure to hold piece information (color and type)
typedef struct {
    uint32_t color;
    uint32_t type;
} PieceInfo;

// A structure to represent the chess board state
typedef struct {
    uint64_t piece_bitboards[12]; // 6 piece types * 2 colors
                                  // [0]: White Pawns, [1]: Black Pawns
                                  // [2]: White Rooks, [3]: Black Rooks
                                  // ...
                                  // [10]: White King, [11]: Black King
    uint32_t white_queenside_castle; // Offset 0x60 (96 bytes)
    uint32_t black_queenside_castle; // Offset 0x64 (100 bytes)
    uint32_t white_kingside_castle;  // Offset 0x68 (104 bytes)
    uint32_t black_kingside_castle;  // Offset 0x6c (108 bytes)
    int32_t en_passant_row;         // Offset 0x70 (112 bytes)
    int32_t en_passant_col;         // Offset 0x74 (116 bytes)
    uint32_t halfmove_clock;        // Offset 0x78 (120 bytes)
} Board; // Total size: 12*8 + 6*4 = 96 + 24 = 120 bytes (0x78)

// MoveInfo struct to hold parsed move details
typedef struct {
    uint32_t color;
    uint32_t type;
    uint32_t promo_type;
    uint32_t kingside_castle;
    uint32_t queenside_castle;
    uint32_t is_capture;
    uint32_t is_en_passant;
    uint32_t is_check;
    uint32_t is_checkmate;
    int32_t src_row;
    int32_t src_col;
    int32_t dest_row;
    int32_t dest_col;
} MoveInfo; // Total size 13 * sizeof(uint32_t) = 52 bytes (0x34)

// Function prototypes (to allow global array of function pointers)
uint64_t pawn_moves(Board *board, int color, int row, int col);
uint64_t rook_moves(Board *board, int color, int row, int col);
uint64_t knight_moves(Board *board, int color, int row, int col);
uint64_t bishop_moves(Board *board, int color, int row, int col);
uint64_t queen_moves(Board *board, int color, int row, int col);
uint64_t king_moves(Board *board, int color, int row, int col);
uint32_t is_at_risk(Board *board, int king_color, int king_row, int king_col, int is_king_move);
uint32_t infer_src(Board *board, MoveInfo *move_info);
uint32_t can_castle(Board *board, int color, int is_kingside);
uint32_t set_piece(Board *board, int row, int col, uint32_t color, uint32_t type);
uint32_t clear_piece(Board *board, int row, int col);
int get_piece(Board *board, int row, int col, PieceInfo *piece_out);


// Global array of function pointers for piece moves
typedef uint64_t (*PieceMoveFunc)(Board *, int, int, int);
PieceMoveFunc piece_moves[6]; // Index by piece type (PAWN to KING)

// String literals for printing
const char *piece_symbols[2][6] = { // [color][type]
    {"P", "R", "N", "B", "Q", "K"}, // White
    {"p", "r", "n", "b", "q", "k"}  // Black
};
const char *bg_white_square = "\033[47m";
const char *bg_black_square = "\033[40m";
const char *fg_white_piece = "\033[37m";
const char *fg_black_piece = "\033[30m";
const char *reset_color = "\033[0m";

// Function: make_piece
void make_piece(PieceInfo *piece, uint32_t color, uint32_t type) {
  piece->color = color;
  piece->type = type;
}

// Helper for bitboard manipulation
// Sets a bit at (row, col) in a 64-bit bitboard
static inline void set_bit(uint64_t *bitboard, int row, int col) {
    if (row >= 0 && row < 8 && col >= 0 && col < 8) {
        int pos = row * 8 + col;
        *bitboard |= (1ULL << pos);
    }
}

// Clears a bit at (row, col) in a 64-bit bitboard
static inline void clear_bit(uint64_t *bitboard, int row, int col) {
    if (row >= 0 && row < 8 && col >= 0 && col < 8) {
        int pos = row * 8 + col;
        *bitboard &= ~(1ULL << pos);
    }
}

// Checks if a bit is set at (row, col) in a 64-bit bitboard
static inline bool get_bit(uint64_t bitboard, int row, int col) {
    if (row < 0 || row > 7 || col < 0 || col > 7) {
        return false;
    }
    int pos = row * 8 + col;
    return (bitboard >> pos) & 1ULL;
}

// Function: init_bitboard
void init_bitboard(Board *board) {
  // Initialize all bitboards to 0 and other state variables
  memset(board, 0, sizeof(Board)); // Use sizeof(Board) for robustness

  // Initialize castling rights (original offsets: 0x60, 0x64, 0x68, 0x6c)
  board->white_kingside_castle = 1;  // At 0x68
  board->white_queenside_castle = 1; // At 0x60
  board->black_kingside_castle = 1;  // At 0x6c
  board->black_queenside_castle = 1; // At 0x64
  board->en_passant_row = -1; // -1 for no en passant target
  board->en_passant_col = -1; // -1 for no en passant target
  board->halfmove_clock = 0;

  PieceInfo piece;
  uint32_t color_idx;
  int row_offset;

  for (color_idx = 0; color_idx < 2; ++color_idx) {
    row_offset = (color_idx == WHITE) ? 0 : 7; // Row 0 for WHITE, Row 7 for BLACK

    // Rooks
    make_piece(&piece, color_idx, ROOK);
    set_piece(board, row_offset, 0, piece.color, piece.type);
    set_piece(board, row_offset, 7, piece.color, piece.type);

    // Knights
    make_piece(&piece, color_idx, KNIGHT);
    set_piece(board, row_offset, 1, piece.color, piece.type);
    set_piece(board, row_offset, 6, piece.color, piece.type);

    // Bishops
    make_piece(&piece, color_idx, BISHOP);
    set_piece(board, row_offset, 2, piece.color, piece.type);
    set_piece(board, row_offset, 5, piece.color, piece.type);

    // Queen
    make_piece(&piece, color_idx, QUEEN);
    set_piece(board, row_offset, 3, piece.color, piece.type);

    // King
    make_piece(&piece, color_idx, KING);
    set_piece(board, row_offset, 4, piece.color, piece.type);

    // Pawns
    uint32_t pawn_row = (color_idx == WHITE) ? 1 : 6;
    make_piece(&piece, color_idx, PAWN);
    for (int col_idx = 0; col_idx < 8; ++col_idx) {
      set_piece(board, pawn_row, col_idx, piece.color, piece.type);
    }
  }
}

// Function: print_bitboard
void print_bitboard(Board *board, int player_color) {
  PieceInfo piece_info;
  int display_row, display_col;
  const char *current_bg;
  const char *current_fg;
  const char *piece_char;

  for (int r = 0; r < 8; ++r) {
    display_row = (player_color == WHITE) ? (7 - r) : r; // Invert rows for black player
    printf("%d ", display_row + 1); // Print rank number (1-8)

    for (int c = 0; c < 8; ++c) {
      display_col = (player_color == WHITE) ? c : (7 - c); // Invert columns for black player

      // Determine square background color
      current_bg = ((display_row + display_col) % 2 == 0) ? bg_white_square : bg_black_square;
      printf("%s", current_bg);

      // Get piece information
      int result = get_piece(board, display_row, display_col, &piece_info);
      if (result == 1) { // Piece found
        current_fg = (piece_info.color == WHITE) ? fg_white_piece : fg_black_piece;
        piece_char = piece_symbols[piece_info.color][piece_info.type];
        printf("%s %s %s", current_fg, piece_char, reset_color); // Use reset after piece char
      } else { // Empty square
        printf("   %s", reset_color); // Print empty space with reset
      }
    }
    printf("%s\n", reset_color); // Reset color at end of line
  }

  // Print file labels (a-h)
  printf("  ");
  if (player_color == WHITE) {
    for (char file = 'a'; file <= 'h'; ++file) {
      printf("%c  ", file);
    }
  } else {
    for (char file = 'h'; file >= 'a'; --file) {
      printf("%c  ", file);
    }
  }
  printf("\n");
}

// Function: set_piece
uint32_t set_piece(Board *board, int row, int col, uint32_t color, uint32_t type) {
  if (row < 0 || col < 0 || row > 7 || col > 7) {
    return 0xFFFFFFFF; // Out of bounds
  }
  uint32_t piece_idx = type * 2 + color;
  set_bit(&board->piece_bitboards[piece_idx], row, col);
  return 0;
}

// Function: clear_piece
uint32_t clear_piece(Board *board, int row, int col) {
  if (row < 0 || col < 0 || row > 7 || col > 7) {
    return 0xFFFFFFFF; // Out of bounds
  }
  for (int i = 0; i < 12; ++i) {
    clear_bit(&board->piece_bitboards[i], row, col);
  }
  return 0;
}

// Function: get_piece
int get_piece(Board *board, int row, int col, PieceInfo *piece_out) {
  if (row < 0 || col < 0 || row > 7 || col > 7) {
    return -1; // Out of bounds
  }
  for (int i = 0; i < 12; ++i) {
    if (get_bit(board->piece_bitboards[i], row, col)) {
      if (piece_out) {
        make_piece(piece_out, i & 1, i >> 1); // i&1 is color, i>>1 is type
      }
      return 1; // Piece found
    }
  }
  return 0; // No piece found
}

// Function: pawn_moves
uint64_t pawn_moves(Board *board, int color, int row, int col) {
  uint64_t moves = 0ULL;
  PieceInfo target_piece;
  int dir = (color == WHITE) ? 1 : -1; // Direction for pawn movement
  int target_row;
  int result;

  // Single step forward
  target_row = row + dir;
  result = get_piece(board, target_row, col, NULL); // Check if square is empty
  if (result == 0) { // Empty square
    set_bit(&moves, target_row, col);
    // Double step from starting rank
    if ((color == WHITE && row == 1) || (color == BLACK && row == 6)) {
      target_row = row + 2 * dir;
      result = get_piece(board, target_row, col, NULL); // Check if square is empty
      if (result == 0) { // Empty square
        set_bit(&moves, target_row, col);
      }
    }
  }

  // Captures
  // Left diagonal capture
  result = get_piece(board, row + dir, col - 1, &target_piece);
  if (result == 1 && target_piece.color != color) {
    set_bit(&moves, row + dir, col - 1);
  }
  // Right diagonal capture
  result = get_piece(board, row + dir, col + 1, &target_piece);
  if (result == 1 && target_piece.color != color) {
    set_bit(&moves, row + dir, col + 1);
  }

  // En passant
  if (board->en_passant_row != -1 && board->en_passant_col != -1) {
    if (board->en_passant_row == row + dir && board->en_passant_col == col - 1) {
      set_bit(&moves, row + dir, col - 1);
    }
    if (board->en_passant_row == row + dir && board->en_passant_col == col + 1) {
      set_bit(&moves, row + dir, col + 1);
    }
  }

  return moves;
}

// Function: rook_moves
uint64_t rook_moves(Board *board, int color, int row, int col) {
  uint64_t moves = 0ULL;
  PieceInfo target_piece;
  int directions[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}}; // N, S, E, W

  for (int i = 0; i < 4; ++i) {
    int dr = directions[i][0];
    int dc = directions[i][1];
    int r = row + dr;
    int c = col + dc;
    while (true) {
      int result = get_piece(board, r, c, &target_piece);
      if (result == -1) break; // Out of bounds
      if (result == 0) { // Empty square
        set_bit(&moves, r, c);
      } else { // Piece found
        if (target_piece.color != color) { // Opponent piece
          set_bit(&moves, r, c);
        }
        break; // Stop sliding in this direction
      }
      r += dr;
      c += dc;
    }
  }
  return moves;
}

// Function: knight_moves
uint64_t knight_moves(Board *board, int color, int row, int col) {
  uint64_t moves = 0ULL;
  PieceInfo target_piece;
  int offsets[8][2] = {
      {2, 1}, {1, 2}, {1, -2}, {-2, 1},
      {-1, 2}, {-2, -1}, {-1, -2}, {2, -1}
  };

  for (int i = 0; i < 8; ++i) {
    int r = row + offsets[i][0];
    int c = col + offsets[i][1];
    int result = get_piece(board, r, c, &target_piece);
    if (result == 0) { // Empty square
      set_bit(&moves, r, c);
    } else if (result == 1 && target_piece.color != color) { // Opponent piece
      set_bit(&moves, r, c);
    }
  }
  return moves;
}

// Function: bishop_moves
uint64_t bishop_moves(Board *board, int color, int row, int col) {
  uint64_t moves = 0ULL;
  PieceInfo target_piece;
  int directions[4][2] = {{1, 1}, {-1, 1}, {1, -1}, {-1, -1}}; // NE, NW, SE, SW

  for (int i = 0; i < 4; ++i) {
    int dr = directions[i][0];
    int dc = directions[i][1];
    int r = row + dr;
    int c = col + dc;
    while (true) {
      int result = get_piece(board, r, c, &target_piece);
      if (result == -1) break; // Out of bounds
      if (result == 0) { // Empty square
        set_bit(&moves, r, c);
      } else { // Piece found
        if (target_piece.color != color) { // Opponent piece
          set_bit(&moves, r, c);
        }
        break; // Stop sliding in this direction
      }
      r += dr;
      c += dc;
    }
  }
  return moves;
}

// Function: queen_moves
uint64_t queen_moves(Board *board, int color, int row, int col) {
  uint64_t bishop_m = bishop_moves(board, color, row, col);
  uint64_t rook_m = rook_moves(board, color, row, col);
  return bishop_m | rook_m;
}

// Function: is_at_risk
uint32_t is_at_risk(Board *board, int king_color, int king_row, int king_col, int is_king_move) {
  int opponent_color = (king_color == WHITE) ? BLACK : WHITE;

  // Iterate through all piece types
  for (int piece_type = PAWN; piece_type <= KING; ++piece_type) {
    // If checking for king moves, we only need to check up to QUEEN attacks (excluding king vs king)
    if (is_king_move && piece_type == KING) {
        continue;
    }

    uint64_t opponent_bitboard = board->piece_bitboards[piece_type * 2 + opponent_color];

    // Iterate through all squares to find opponent pieces of current type
    for (int r = 0; r < 8; ++r) {
      for (int c = 0; c < 8; ++c) {
        if (get_bit(opponent_bitboard, r, c)) {
          // Original decompiler condition: (piece_type != PAWN || (c != king_col))
          // This prevents checking for pawn attacks directly in front, which are not attacks.
          if (piece_type != PAWN || (c != king_col)) {
            uint64_t piece_attacks = piece_moves[piece_type](board, opponent_color, r, c);
            if (get_bit(piece_attacks, king_row, king_col)) {
              return 1; // King is at risk
            }
          }
        }
      }
    }
  }
  return 0; // King is not at risk
}

// Function: king_moves
uint64_t king_moves(Board *board, int color, int row, int col) {
  uint64_t moves = 0ULL;
  PieceInfo target_piece;
  int offsets[8][2] = {
      {1, 1}, {0, 1}, {-1, 1}, {-1, 0},
      {-1, -1}, {0, -1}, {1, -1}, {1, 0}
  };

  for (int i = 0; i < 8; ++i) {
    int r = row + offsets[i][0];
    int c = col + offsets[i][1];
    // Check if target square is on board
    if (r >= 0 && r <= 7 && c >= 0 && c <= 7) {
      int result = get_piece(board, r, c, &target_piece);
      // Check if square is empty or has opponent piece
      if (result == 0 || (result == 1 && target_piece.color != color)) {
        // Crucially, a king cannot move into a square that is attacked by an opponent piece.
        if (is_at_risk(board, color, r, c, 1) == 0) { // Check if new square is safe for king
          set_bit(&moves, r, c);
        }
      }
    }
  }
  return moves;
}

// Function: is_checkmate
uint32_t is_checkmate(Board *board, int king_color, int king_row, int king_col) {
  // Simplified checkmate: King is at risk AND King has no safe moves
  if (is_at_risk(board, king_color, king_row, king_col, 0) == 1) {
    if (king_moves(board, king_color, king_row, king_col) == 0ULL) {
      return 1; // Checkmate
    }
  }
  return 0; // Not checkmate (or not in check)
}

// Function: is_stalemate
bool is_stalemate(Board *board, int color) {
  uint64_t all_possible_moves = 0ULL;

  if (board->halfmove_clock >= 100) { // 50-move rule (50 full moves)
    return true;
  }

  for (int piece_type = PAWN; piece_type <= KING; ++piece_type) {
    uint64_t player_bitboard = board->piece_bitboards[piece_type * 2 + color];

    for (int r = 0; r < 8; ++r) {
      for (int c = 0; c < 8; ++c) {
        if (get_bit(player_bitboard, r, c)) {
          // Get moves for this piece
          all_possible_moves |= piece_moves[piece_type](board, color, r, c);
        }
      }
    }
  }
  return (all_possible_moves == 0ULL); // If no possible moves, it's stalemate
}

// Function: infer_src
uint32_t infer_src(Board *board, MoveInfo *move_info) {
  // If destination is not set, cannot infer source
  if (move_info->dest_row == -1 || move_info->dest_col == -1) {
    return 0xFFFFFFFF; // Error
  }

  uint32_t player_color = move_info->color;
  uint32_t piece_type = move_info->type;
  uint64_t piece_bb = board->piece_bitboards[piece_type * 2 + player_color];

  int found_src_row = -1;
  int found_src_col = -1;
  int num_possible_src = 0;

  for (int r = 0; r < 8; ++r) {
    for (int c = 0; c < 8; ++c) {
      if (get_bit(piece_bb, r, c)) { // If there's a piece of the correct type and color
        uint64_t possible_moves = piece_moves[piece_type](board, player_color, r, c);
        if (get_bit(possible_moves, move_info->dest_row, move_info->dest_col)) {
          // This piece at (r,c) can move to the destination
          // Check for disambiguation (if src_row/col is specified)
          if ((move_info->src_row == -1 || move_info->src_row == r) &&
              (move_info->src_col == -1 || move_info->src_col == c)) {
            found_src_row = r;
            found_src_col = c;
            num_possible_src++;
          }
        }
      }
    }
  }

  // If exactly one possible source found, set it in move_info
  if (num_possible_src == 1) {
    move_info->src_row = found_src_row;
    move_info->src_col = found_src_col;
    return 0; // Success
  }
  // If no source or multiple sources, it's an error or requires more disambiguation
  return 0xFFFFFFFF; // Error or ambiguous
}

// Function: can_castle
uint32_t can_castle(Board *board, int color, int is_kingside) {
  uint32_t castling_right;
  int king_row = (color == WHITE) ? 0 : 7;
  int king_col = 4; // King's starting column

  // Check castling rights based on board state
  if (color == WHITE) {
    castling_right = is_kingside ? board->white_kingside_castle : board->white_queenside_castle;
  } else {
    castling_right = is_kingside ? board->black_kingside_castle : board->black_queenside_castle;
  }
  if (castling_right == 0) { // Castling not allowed due to previous move
    return 0;
  }

  // Check if King is on its starting square and is a King piece
  PieceInfo king_piece;
  int result = get_piece(board, king_row, king_col, &king_piece);
  if (result != 1 || king_piece.color != color || king_piece.type != KING) {
    return 0;
  }

  // Check if Rook is on its starting square and is a Rook piece
  int rook_col = is_kingside ? 7 : 0;
  PieceInfo rook_piece;
  result = get_piece(board, king_row, rook_col, &rook_piece);
  if (result != 1 || rook_piece.color != color || rook_piece.type != ROOK) {
    return 0;
  }

  // Check squares between King and Rook are empty and not under attack
  int start_path_col = is_kingside ? king_col + 1 : rook_col + 1;
  int end_path_col = is_kingside ? rook_col : king_col;
  
  // Ensure start_path_col is always less than end_path_col for the loop
  if (start_path_col > end_path_col) {
      int temp = start_path_col;
      start_path_col = end_path_col;
      end_path_col = temp;
  }

  for (int c = start_path_col; c < end_path_col; ++c) {
    if (get_piece(board, king_row, c, NULL) != 0) { // Square not empty
      return 0;
    }
    if (is_at_risk(board, color, king_row, c, 0) != 0) { // Square under attack
      return 0;
    }
  }

  // Also check the king's current square and destination square for attacks
  if (is_at_risk(board, color, king_row, king_col, 0) != 0) { // King's current square under attack
      return 0;
  }
  int king_dest_col = is_kingside ? 6 : 2;
  if (is_at_risk(board, color, king_row, king_dest_col, 0) != 0) { // King's destination square under attack
      return 0;
  }

  return 1; // Can castle
}

// Function: validate_move
uint32_t validate_move(Board *board, MoveInfo *move_info) {
  // 1. Basic checks: source and destination must be set
  if (move_info->src_row == -1 || move_info->src_col == -1 ||
      move_info->dest_row == -1 || move_info->dest_col == -1) {
    return 0; // Invalid move: source or destination not specified
  }

  // 2. Check if there's a piece of the correct type and color at the source
  PieceInfo src_piece;
  int result = get_piece(board, move_info->src_row, move_info->src_col, &src_piece);
  if (result != 1 || src_piece.color != move_info->color || src_piece.type != move_info->type) {
    return 0; // Invalid move: no matching piece at source
  }

  // 3. Handle special moves: Castling
  if (move_info->kingside_castle || move_info->queenside_castle) {
    return can_castle(board, move_info->color, move_info->kingside_castle);
  }

  // 4. Check if destination square is occupied by own piece (unless it's an en passant capture)
  // The original code's logic here for en passant is slightly convoluted. Let's simplify:
  // If it's an en passant move, the destination square is logically "empty" for the capture,
  // but there's an opponent pawn one square behind it.
  // The `pawn_moves` function already handles en passant captures.
  // So, we only need to check for regular captures/moves.
  if (!move_info->is_en_passant) {
      PieceInfo dest_piece;
      result = get_piece(board, move_info->dest_row, move_info->dest_col, &dest_piece);
      if (result == 1 && dest_piece.color == move_info->color) {
          return 0; // Invalid move: destination occupied by own piece
      }
  }

  // 5. Calculate possible moves for the piece at source and check if destination is among them
  uint64_t possible_moves = piece_moves[move_info->type](board, move_info->color, move_info->src_row, move_info->src_col);
  if (!get_bit(possible_moves, move_info->dest_row, move_info->dest_col)) {
    return 0; // Invalid move: destination not reachable by this piece
  }

  // 6. Check for leaving own king in check (or moving into check)
  // This requires a temporary board or undo mechanism for a full implementation,
  // which is not present in the original decompiled code.
  // For now, we will assume the decompiled code's simplified validation.

  return 1; // Valid move
}

// Function: make_move
uint32_t make_move(Board *board, MoveInfo *move_info) {
  if (validate_move(board, move_info) == 0) {
    return 0xFFFFFFFF; // Invalid move
  }

  // Update halfmove clock
  PieceInfo dest_piece;
  int dest_has_piece = get_piece(board, move_info->dest_row, move_info->dest_col, &dest_piece);
  if (move_info->type == PAWN || (dest_has_piece == 1 && dest_piece.color != move_info->color)) {
    board->halfmove_clock = 0; // Reset for pawn move or capture
  } else {
    board->halfmove_clock++;
  }

  // Clear en passant target squares
  board->en_passant_row = -1;
  board->en_passant_col = -1;

  // Handle special moves
  if (move_info->kingside_castle) {
    int king_row = (move_info->color == WHITE) ? 0 : 7;
    clear_piece(board, king_row, 4); // Clear king
    clear_piece(board, king_row, 7); // Clear rook
    set_piece(board, king_row, 6, move_info->color, KING);   // Place king
    set_piece(board, king_row, 5, move_info->color, ROOK);   // Place rook
    // Revoke castling rights for this color
    if (move_info->color == WHITE) {
      board->white_kingside_castle = 0;
      board->white_queenside_castle = 0;
    } else {
      board->black_kingside_castle = 0;
      board->black_queenside_castle = 0;
    }
  } else if (move_info->queenside_castle) {
    int king_row = (move_info->color == WHITE) ? 0 : 7;
    clear_piece(board, king_row, 4); // Clear king
    clear_piece(board, king_row, 0); // Clear rook
    set_piece(board, king_row, 2, move_info->color, KING);   // Place king
    set_piece(board, king_row, 3, move_info->color, ROOK);   // Place rook
    // Revoke castling rights for this color
    if (move_info->color == WHITE) {
      board->white_kingside_castle = 0;
      board->white_queenside_castle = 0;
    } else {
      board->black_kingside_castle = 0;
      board->black_queenside_castle = 0;
    }
  } else if (move_info->is_en_passant) {
    clear_piece(board, move_info->src_row, move_info->src_col); // Clear attacking pawn
    int captured_pawn_row = (move_info->color == WHITE) ? move_info->dest_row - 1 : move_info->dest_row + 1;
    clear_piece(board, captured_pawn_row, move_info->dest_col); // Clear captured pawn
    set_piece(board, move_info->dest_row, move_info->dest_col, move_info->color, PAWN); // Place attacking pawn
  } else { // Regular Move / Capture / Promotion
    clear_piece(board, move_info->src_row, move_info->src_col); // Clear source square
    clear_piece(board, move_info->dest_row, move_info->dest_col); // Clear destination square (if piece captured)

    // Handle Pawn Promotion
    if (move_info->type == PAWN &&
        ((move_info->color == WHITE && move_info->dest_row == 7) ||
         (move_info->color == BLACK && move_info->dest_row == 0))) {
      // Promote pawn to specified promotion type, default to Queen if not specified
      uint32_t promo_type = (move_info->promo_type != 0 && move_info->promo_type <= QUEEN) ? move_info->promo_type : QUEEN;
      set_piece(board, move_info->dest_row, move_info->dest_col, move_info->color, promo_type);
    } else {
      set_piece(board, move_info->dest_row, move_info->dest_col, move_info->color, move_info->type);
    }

    // Set new en passant target if double pawn push
    if (move_info->type == PAWN && abs(move_info->src_row - move_info->dest_row) == 2) {
      board->en_passant_row = (move_info->color == WHITE) ? move_info->src_row + 1 : move_info->src_row - 1;
      board->en_passant_col = move_info->src_col;
    }
  }

  // Determine game state for the OPPONENT
  uint32_t current_king_row, current_king_col;
  int opponent_color = (move_info->color == WHITE) ? BLACK : WHITE;
  uint64_t king_bb = board->piece_bitboards[KING * 2 + opponent_color];
  bool found_king = false;
  for (int r = 0; r < 8; ++r) {
    for (int c = 0; c < 8; ++c) {
      if (get_bit(king_bb, r, c)) {
        current_king_row = r;
        current_king_col = c;
        found_king = true;
        break;
      }
    }
    if (found_king) break;
  }
  if (!found_king) {
      return 2; // King captured, implies checkmate (shouldn't happen in valid games)
  }

  uint32_t result_status = 0; // 0: success, 1: check, 2: checkmate, 3: stalemate
  if (is_stalemate(board, opponent_color)) {
      result_status = 3;
  } else if (is_checkmate(board, opponent_color, current_king_row, current_king_col)) {
      result_status = 2;
  } else if (is_at_risk(board, opponent_color, current_king_row, current_king_col, 0)) {
      result_status = 1;
  }

  return result_status;
}

// Function: parse_san
uint32_t parse_san(Board *board, int color, const char *san_str, MoveInfo *move_info) {
  memset(move_info, 0, sizeof(MoveInfo));
  move_info->color = color;
  move_info->src_row = -1;
  move_info->src_col = -1;
  move_info->dest_row = -1;
  move_info->dest_col = -1;

  if (strcmp(san_str, "O-O") == 0) { // Kingside castle
    move_info->type = KING;
    move_info->kingside_castle = 1;
    move_info->src_row = (color == WHITE) ? 0 : 7;
    move_info->src_col = 4;
    move_info->dest_row = move_info->src_row;
    move_info->dest_col = 6;
    return 0;
  } else if (strcmp(san_str, "O-O-O") == 0) { // Queenside castle
    move_info->type = KING;
    move_info->queenside_castle = 1;
    move_info->src_row = (color == WHITE) ? 0 : 7;
    move_info->src_col = 4;
    move_info->dest_row = move_info->src_row;
    move_info->dest_col = 2;
    return 0;
  } else {
    // This branch corresponds to the complex SAN parsing logic that was missing from the decompiler output.
    // As per instructions, I will not implement new logic, but will provide a minimal
    // interpretation based on the `infer_src` call hint and direct coordinate input.
    // This is NOT a full SAN parser.

    // A very basic attempt to parse "colrowcolrow" (e.g., "e2e4") for testing.
    if (strlen(san_str) == 4 &&
        san_str[0] >= 'a' && san_str[0] <= 'h' && san_str[1] >= '1' && san_str[1] <= '8' &&
        san_str[2] >= 'a' && san_str[2] <= 'h' && san_str[3] >= '1' && san_str[3] <= '8') {
        move_info->src_col = san_str[0] - 'a';
        move_info->src_row = san_str[1] - '1';
        move_info->dest_col = san_str[2] - 'a';
        move_info->dest_row = san_str[3] - '1';

        // Try to infer piece type based on source square
        PieceInfo src_piece;
        if (get_piece(board, move_info->src_row, move_info->src_col, &src_piece) == 1) {
            move_info->type = src_piece.type;
            move_info->color = src_piece.color;
            return 0; // Successfully parsed basic coordinate move
        } else {
            return 0xFFFFFFFF; // No piece at source
        }
    }

    // If dest_row/col are already set (e.g., from a more complex parser not provided)
    // and src_row/col are not, try to infer.
    // This aligns with the decompiler's output:
    // `if ((param_4[0xb] == -1) || (param_4[0xc] == -1))` -> checks if dest is NOT set
    // `else if (((param_4[9] == -1) || (param_4[0xc] == -1)) && (iVar1 = infer_src(param_1,param_4), iVar1 != 0))` ->
    // if dest IS set, but src is NOT set, then call infer_src.
    if (move_info->dest_row != -1 && move_info->dest_col != -1 &&
        (move_info->src_row == -1 || move_info->src_col == -1)) {
        if (infer_src(board, move_info) != 0) {
            return 0xFFFFFFFF; // Could not infer unique source
        }
        return 0;
    }
  }

  // If none of the above, it's an unhandled SAN format
  return 0xFFFFFFFF; // Error: Unrecognized SAN format
}

// Function to initialize the global piece_moves array
void setup_piece_move_functions() {
    piece_moves[PAWN]   = pawn_moves;
    piece_moves[ROOK]   = rook_moves;
    piece_moves[KNIGHT] = knight_moves;
    piece_moves[BISHOP] = bishop_moves;
    piece_moves[QUEEN]  = queen_moves;
    piece_moves[KING]   = king_moves;
}

// Main function for demonstration
int main() {
    setup_piece_move_functions(); // Initialize function pointers

    Board board;
    init_bitboard(&board);

    printf("Initial board:\n");
    print_bitboard(&board, WHITE);

    MoveInfo move;
    uint32_t move_status;

    // Example moves:
    // 1. White pawn e2-e4
    printf("\nAttempting move: e2e4 (White)\n");
    move_status = parse_san(&board, WHITE, "e2e4", &move);
    if (move_status == 0) {
        move_status = make_move(&board, &move);
        if (move_status == 0) {
            printf("Move successful.\n");
        } else if (move_status == 1) {
            printf("Move successful. Check!\n");
        } else if (move_status == 2) {
            printf("Move successful. Checkmate!\n");
        } else if (move_status == 3) {
            printf("Move successful. Stalemate!\n");
        } else {
            printf("Move failed: %u\n", move_status);
        }
    } else {
        printf("SAN parsing failed for 'e2e4'.\n");
    }
    print_bitboard(&board, WHITE);

    // 2. Black pawn d7-d5
    printf("\nAttempting move: d7d5 (Black)\n");
    move_status = parse_san(&board, BLACK, "d7d5", &move);
    if (move_status == 0) {
        move_status = make_move(&board, &move);
        if (move_status == 0) {
            printf("Move successful.\n");
        } else {
            printf("Move failed: %u\n", move_status);
        }
    } else {
        printf("SAN parsing failed for 'd7d5'.\n");
    }
    print_bitboard(&board, WHITE);

    // 3. White kingside castle (O-O) - requires clearing pieces in between
    // For demonstration, let's manually clear pieces for castling
    printf("\nAttempting to setup for White O-O\n");
    clear_piece(&board, 0, 1); // clear Knight
    clear_piece(&board, 0, 2); // clear Bishop
    clear_piece(&board, 0, 3); // clear Queen
    clear_piece(&board, 0, 5); // clear Bishop
    clear_piece(&board, 0, 6); // clear Knight
    print_bitboard(&board, WHITE);

    printf("\nAttempting move: O-O (White)\n");
    move_status = parse_san(&board, WHITE, "O-O", &move);
    if (move_status == 0) {
        move_status = make_move(&board, &move);
        if (move_status == 0) {
            printf("Move successful.\n");
        } else {
            printf("Move failed: %u\n", move_status);
        }
    } else {
        printf("SAN parsing failed for 'O-O'.\n");
    }
    print_bitboard(&board, WHITE);

    return 0;
}