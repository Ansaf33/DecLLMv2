#include <stdint.h> // Required for uint16_t and uint32_t

// Function: motorola_swap_short
uint16_t motorola_swap_short(uint16_t param_1) {
  return (param_1 << 8) | (param_1 >> 8);
}

// Function: motorola_swap_word
uint32_t motorola_swap_word(uint32_t param_1) {
  return (param_1 << 24) |            // Move byte 0 (LSB) to byte 3 (MSB)
         (param_1 >> 24) |            // Move byte 3 (MSB) to byte 0 (LSB)
         ((param_1 & 0x0000FF00) << 8) | // Move byte 1 to byte 2
         ((param_1 & 0x00FF0000) >> 8);  // Move byte 2 to byte 1
}

// Function: intel_swap_short
uint16_t intel_swap_short(uint16_t param_1) {
  return param_1;
}

// Function: intel_swap_word
uint32_t intel_swap_word(uint32_t param_1) {
  return param_1;
}