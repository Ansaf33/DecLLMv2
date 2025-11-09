#include <stdio.h>

void PrintBoard(const char* board) {
  char line_buffer[9];
  
  for (int row = 7; row >= 0; row--) {
    for (int col = 0; col < 8; col++) {
      char piece = board[col * 8 + row];
      if (piece == '\0') {
        line_buffer[col] = '.';
      } else {
        line_buffer[col] = piece;
      }
    }
    line_buffer[8] = '\0';
    printf("%s\n", line_buffer);
  }
}

void InitBoard(char *board) {
  board[56] = 'd';
  board[0] = board[56];
  board[48] = 'b';
  board[8] = board[48];
  board[40] = 'c';
  board[16] = board[40];
  board[24] = 'f';
  board[32] = 'e';

  for (int col = 0; col < 8; col++) {
    board[col * 8 + 1] = 'a';
  }

  for (int row = 2; row < 6; row++) {
    for (int col = 0; col < 8; col++) {
      board[col * 8 + row] = '\0';
    }
  }

  for (int col = 0; col < 8; col++) {
    board[col * 8 + 6] = 'g';
  }

  board[63] = 'j';
  board[7] = board[63];
  board[55] = 'h';
  board[15] = board[55];
  board[47] = 'i';
  board[23] = board[47];
  board[31] = 'l';
  board[39] = 'k';
}

int main() {
  char board[64];

  InitBoard(board);
  PrintBoard(board);

  return 0;
}