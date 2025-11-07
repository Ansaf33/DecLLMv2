#include <stdint.h> // For uint32_t
#include <stdio.h>  // For NULL, EOF, printf (used in receive_all for error)
#include <unistd.h> // For write, read
#include <string.h> // For memset (to clear gBoard if needed)

// Type redefinitions based on common decompiled types
typedef uint32_t uint;
typedef uint32_t undefined4;
typedef char     undefined;

// Global variables (assuming these are global based on the snippet)
uint32_t gBoard[32];
uint32_t DAT_0001417c; // Global variable
uint32_t DAT_00014178; // Global variable
uint32_t mask[32];     // Global array for bit masks
uint32_t neg_mask[32]; // Global array for negative bit masks

// Function to initialize global masks
void initialize_global_masks() {
    for (int i = 0; i < 32; ++i) {
        mask[i] = 1U << i;
        neg_mask[i] = ~(1U << i);
    }
}

// Function: transmit_all
// param_1: stream_id (e.g., 1 for stdout)
// param_2: buffer (const char*)
// param_3: size (uint32_t)
uint32_t transmit_all(uint32_t stream_id, const char* buffer, uint32_t size) {
  if (buffer == NULL || size == 0) {
    return 0; // No data to transmit or invalid buffer
  }

  uint32_t total_transmitted = 0;
  while (total_transmitted < size) {
    ssize_t transmitted = write(stream_id, buffer + total_transmitted, size - total_transmitted);
    if (transmitted <= 0) { // Error or nothing written
      // If some bytes were already transmitted, return them. Otherwise, return 0.
      if (total_transmitted > 0) return total_transmitted;
      return 0;
    }
    total_transmitted += transmitted;
  }
  return total_transmitted; // Return total bytes transmitted (should be 'size' if successful)
}

// Function: receive_all
// param_1: stream_id (e.g., 0 for stdin)
// param_2: buffer (char*)
// param_3: size (uint32_t)
uint32_t receive_all(uint32_t stream_id, char* buffer, uint32_t size) {
  if (buffer == NULL || size == 0) {
    return 0; // No buffer or nothing to receive
  }

  uint32_t total_received = 0;
  while (total_received < size) {
    ssize_t received = read(stream_id, buffer + total_received, size - total_received);
    if (received <= 0) { // Error or EOF
      // If EOF and some bytes were already read, return what was read.
      // If EOF immediately, or error, return 0.
      if (total_received > 0) return total_received;
      return 0;
    }
    total_received += received;
  }
  return total_received; // Return total bytes received (should be 'size' if successful)
}

// Function: rotate_right
void rotate_right(int param_1) {
  // Reduces intermediate variables by directly using bitwise operations
  if ((param_1 >= 0) && (param_1 < 32)) { // 0x20 is 32
    gBoard[param_1] = (gBoard[param_1] << 1) | (gBoard[param_1] >> 31); // Rotate 32-bit left
  }
}

// Function: rotate_down
void rotate_down(int param_1) {
  uint32_t uVar1; // Kept as it holds DAT_0001417c for the loop's initial state
  int row_idx; // Renamed from local_c for clarity
  
  uVar1 = DAT_0001417c;
  if ((param_1 >= 0) && (param_1 < 32)) { // 0x20 is 32
    for (row_idx = 31; row_idx > 0; row_idx--) { // 0x1f is 31
      gBoard[row_idx] = (gBoard[row_idx] & neg_mask[param_1]) | (mask[param_1] & gBoard[row_idx - 1]);
    }
    // The original `gBoard` here refers to `gBoard[0]` due to array decay/decompiler interpretation
    gBoard[0] = (mask[param_1] & uVar1) | (gBoard[0] & neg_mask[param_1]);
  }
}

// Function: printBoard
void printBoard(void) {
  char board_buffer[1088]; // Buffer for board representation, local_458
  int buffer_idx = 0; // Renamed from local_18
  int col_idx; // Renamed from local_14
  int row_idx; // Renamed from local_10
  
  for (row_idx = 0; row_idx < 32; row_idx++) { // 0x20 is 32
    for (col_idx = 0; col_idx < 32; col_idx++) { // 0x20 is 32
      // Reduced puVar1 intermediate variable by direct array access
      if ((mask[col_idx] & gBoard[row_idx]) == 0) {
        board_buffer[buffer_idx++] = '.'; // 0x2e is '.'
      } else {
        board_buffer[buffer_idx++] = '1'; // 0x31 is '1'
      }
    }
    board_buffer[buffer_idx++] = '\n'; // 10 is '\n'
  }
  board_buffer[buffer_idx] = '\0'; // Null-terminate the string
  transmit_all(1, board_buffer, buffer_idx); // Transmit to stdout (fd 1)
}

// Function: initBoard
void initBoard(void) {
  DAT_0001417c = 0xffffffff;
  DAT_00014178 = 0xffffffff;
  // gBoard is implicitly zero-initialized as a global.
  // If the game requires a non-empty starting board, gBoard must be explicitly
  // initialized here or in main. Assuming current logic is intended.
}

// Function: remove_bits
void remove_bits(void) {
  if (((DAT_0001417c & 3) == 3) && ((DAT_00014178 & 3) == 3)) {
    DAT_0001417c &= ~3; // 0xfffffffc is ~3
    DAT_00014178 &= ~3;
  }
}

// Function: makeMove
void makeMove(uint32_t param_1) { // Changed param_1 to uint32_t to match usage
  if ((param_1 & 32) == 0) { // 0x20 is 32
    rotate_down(param_1 & 31); // 0x1f is 31
  } else {
    rotate_right(param_1 & 31); // 0x1f is 31
  }
  remove_bits();
}

// Function: isWinner
uint32_t isWinner(void) { // Changed return type to uint32_t
  int row_idx = 0; // Renamed from local_8
  
  while( row_idx < 32 ) { // 0x1f < local_8 means local_8 >= 0x20 (32)
    if (gBoard[row_idx] != 0) {
        return 0; // Not a winner if any row is non-zero
    }
    row_idx++;
  }
  return 1; // All rows are zero, so it's a winner
}

// Function: main
int main(void) { // Changed return type to int
  int status_code; // Renamed from iVar1
  char move_char; // Renamed from local_11
  // local_10 = &stack0x00000004; -- Removed assembly-like statement
  
  initialize_global_masks(); // Initialize mask arrays
  initBoard();

  while(1) { // Loop indefinitely until a break condition
    status_code = isWinner();
    if (status_code != 0) {
      // (*(code *)0x0)(); -- Removed call to address 0 (causes segmentation fault)
      transmit_all(1, "You Win\n", 8); // Transmit "You Win\n"
      return 0; // Exit on win
    }
    
    // Read one character (move) from stdin (fd 0)
    status_code = receive_all(0, &move_char, 1);
    if (status_code != 1) { // If receive_all did not read exactly 1 byte (e.g., EOF or error)
      break; // Exit loop
    }
    
    // Check for special quit character (-1, usually 0xFF if char is signed)
    if (move_char == (char)-1) {
      printBoard();
      return 0; // Exit on quit character
    }
    
    makeMove((uint32_t)move_char); // Cast char to uint32_t for makeMove
  }
  
  printBoard(); // Print final board state
  return 0; // Exit cleanly
}