#include <stdbool.h> // For 'true' in the main loop
#include <stdint.h>  // For standard integer types, though 'unsigned int' is used for undefined4/uint

// --- Placeholder function declarations for external dependencies ---
// These declarations assume the types inferred from the provided snippet.
// 'undefined4' is mapped to 'unsigned int'.
// 'uint' is mapped to 'unsigned int'.
// 'undefined *' is mapped to 'void *' or implicitly handled by pointer casts.

void send_empty_frame(int arg);
void expect_empty_frame(int arg);
unsigned int expect_frame(int arg); // Returns undefined4
void handle_seed(unsigned int arg); // Takes undefined4
void handle_guess(unsigned int arg); // Takes undefined4
void free_frame(unsigned int arg); // Takes undefined4
void transmit(void);
unsigned int *receive_frame(void); // Returns uint *
void handle_echo(void);
void handle_rand(void);
void _terminate(void);

// --- Fixed functions ---

// Function: main
int main(void) { // Changed return type to int for standard C compliance
  send_empty_frame(0);
  expect_empty_frame(0);
  seed(); // Removed argument as the 'seed' function definition takes void
  gauntlet();
  while (true) { // Replaced 'do { ... } while( true );' with 'while (true)'
    operate();
  }
  // This loop never terminates, so a return statement is unreachable.
  // For standard main, `return 0;` would be here if it could exit.
}

// Function: seed
void seed(void) {
  unsigned int frame_handle; // Renamed uVar1, reduced intermediate variable

  frame_handle = expect_frame(1);
  handle_seed(frame_handle);
  free_frame(frame_handle);
  return;
}

// Function: gauntlet
void gauntlet(void) {
  unsigned int frame_handle; // Renamed uVar1, reduced intermediate variable

  // Renamed local_10 to i for standard loop variable naming, simplified loop condition
  for (int i = 0; i < 32; ++i) { // 0x20 is 32 in decimal
    frame_handle = expect_frame(20); // 0x14 is 20 in decimal
    handle_guess(frame_handle);
    free_frame(frame_handle);
  }
  return;
}

// Function: operate
void operate(void) {
  // Removed intermediate variables: ppuVar2, puStack_2c, puStack_28, uStack_24, uStack_20, auStack_1c
  // and their assignments, as they appear to be decompiler artifacts not essential to C logic.

  transmit();
  unsigned int *frame_data_ptr = receive_frame(); // Renamed local_10, reduced intermediate variable
  unsigned int frame_type = *frame_data_ptr;     // Renamed uVar1, reduced intermediate variable

  // Replaced 'goto' statements with a 'switch' statement for structured control flow.
  switch (frame_type) {
    case 0x1e: // Decimal 30
      handle_echo();
      break;
    case 0x14: // Decimal 20
      handle_guess(frame_type); // Assuming handle_guess takes the frame type as argument
      break;
    case 1:
      handle_seed(frame_type); // Assuming handle_seed takes the frame type as argument
      break;
    case 10:
      handle_rand();
      break;
    default:
      // This path corresponds to the original code's _terminate() call for unmatched uVar1 values.
      _terminate();
      break;
  }

  // The original code called free_frame() after the goto label, implying it's always executed.
  // Based on other functions, free_frame takes an unsigned int (frame handle/ID).
  // Assuming frame_type (which is *frame_data_ptr) acts as the handle for freeing.
  free_frame(frame_type);
  return;
}