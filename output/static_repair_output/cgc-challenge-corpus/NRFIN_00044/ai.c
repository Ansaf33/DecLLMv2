#include <stdio.h>
#include <stdint.h>

// Function: seed_ai
// Interprets 'undefined4' as uint32_t and '_DAT_4347c000' as a hexadecimal constant.
void seed_ai(uint32_t *param_1) {
  *param_1 = 0x4347c000;
}

int main() {
  uint32_t ai_seed_value;
  seed_ai(&ai_seed_value);
  printf("AI Seed Value: 0x%08X\n", ai_seed_value);
  return 0;
}