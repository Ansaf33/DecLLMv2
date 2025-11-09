#include <stdint.h> // For uint32_t
#include <stdio.h>  // For printf in main (optional, but good for demonstration)

// Define the constant value, assuming 0x4347c000 is the intended hexadecimal value
const uint32_t _DAT_4347c000 = 0x4347c000U;

// Function: seed_ai
// Renamed undefined4 to uint32_t for standard C compatibility
void seed_ai(uint32_t *param_1) {
  *param_1 = _DAT_4347c000;
  return;
}

// Main function to demonstrate usage
int main() {
  uint32_t ai_seed_value; // Declare the variable directly

  seed_ai(&ai_seed_value); // Pass its address to seed_ai

  printf("AI seed initialized to: 0x%08x\n", ai_seed_value);

  return 0;
}