#include <stdbool.h> // For bool type
#include <stdint.h>  // For uint8_t, uint32_t
#include <string.h>  // For memchr

// --- Dummy definitions for compilation ---
// Assuming 'byte' is uint8_t and 'uint' is uint32_t for Linux compilation
typedef uint8_t byte;
typedef uint32_t uint;

// Dummy game_state structure and instance
struct GameState {
    uint32_t _65528_4_;
} game_state;

// Dummy DAT_00018134 - assuming it's a 2-byte array as used by memchr
const uint8_t DAT_00018134_bytes[] = {0x41, 0x42}; // Example values: 'A', 'B'

// Dummy get_flag_byte function
byte get_flag_byte(uint index) {
    // Placeholder implementation for demonstration purposes.
    // In a real scenario, this would read from a specific flag storage.
    // Example values to make the cookie predictable for testing:
    switch (index) {
        case 0x3fd: return 0x11;
        case 0x3fe: return 0x22;
        case 0x3ff: return 0x33;
        case 0x400: return 0x44;
        default: return (byte)(index % 256); // Fallback for other indices
    }
}
// --- End of dummy definitions ---


// Function: init_cookie
void init_cookie(void) {
  uint32_t cookie_value = 0; // Accumulator for the cookie value
  for (uint32_t i = 0; i < 4; ++i) { // Loop counter
    uint8_t flag_byte = get_flag_byte(i + 0x3fd);
    
    // Reduce intermediate variables by directly using a ternary operator
    // The shift amount (3 - i) * 8 produces 24, 16, 8, 0 for i = 0, 1, 2, 3 respectively.
    cookie_value |= (memchr(DAT_00018134_bytes, flag_byte, 2) == NULL ? flag_byte : 1) 
                  << ((3 - i) * 8);
  }
  game_state._65528_4_ = cookie_value;
}

// Function: check_cookie
bool check_cookie(uint32_t param_1) {
  uint32_t calculated_cookie = 0; // Accumulator for the calculated cookie value
  for (uint32_t i = 0; i < 4; ++i) { // Loop counter
    uint8_t flag_byte = get_flag_byte(i + 0x3fd);
    
    // Reduce intermediate variables by directly using a ternary operator
    // The shift amount (3 - i) * 8 produces 24, 16, 8, 0 for i = 0, 1, 2, 3 respectively.
    calculated_cookie |= (memchr(DAT_00018134_bytes, flag_byte, 2) == NULL ? flag_byte : 1) 
                       << ((3 - i) * 8);
  }
  return param_1 == calculated_cookie;
}