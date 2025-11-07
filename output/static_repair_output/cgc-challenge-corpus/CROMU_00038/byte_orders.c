#include <stdint.h> // For uint16_t, uint32_t
#include <stdio.h>  // For printf in main

// Function: motorola_swap_short
uint16_t motorola_swap_short(uint16_t param_1) {
  return (param_1 << 8) | (param_1 >> 8);
}

// Function: motorola_swap_word
uint32_t motorola_swap_word(uint32_t param_1) {
  return param_1 << 0x18 | param_1 >> 0x18 | (param_1 & 0xff00) << 8 | (param_1 >> 8 & 0xff00);
}

// Function: intel_swap_short
uint16_t intel_swap_short(uint16_t param_1) {
  return param_1;
}

// Function: intel_swap_word
uint32_t intel_swap_word(uint32_t param_1) {
  return param_1;
}

int main() {
    uint16_t short_val = 0x1234;
    uint32_t word_val = 0x12345678;

    printf("Original short: 0x%04X\n", short_val);
    printf("Motorola swapped short: 0x%04X\n", motorola_swap_short(short_val));
    printf("Intel swapped short (no-op): 0x%04X\n", intel_swap_short(short_val));

    printf("\nOriginal word: 0x%08X\n", word_val);
    printf("Motorola swapped word: 0x%08X\n", motorola_swap_word(word_val));
    printf("Intel swapped word (no-op): 0x%08X\n", intel_swap_word(word_val));

    // Test with specific values to clearly demonstrate the swap
    uint16_t test_short = 0xAABB;
    uint32_t test_word = 0xCCDDEEFF;

    printf("\n--- Detailed Tests ---\n");
    printf("Test short 0x%04X -> 0x%04X (Expected: 0xBB AA)\n", test_short, motorola_swap_short(test_short));
    printf("Test word 0x%08X -> 0x%08X (Expected: 0xFF EE DD CC)\n", test_word, motorola_swap_word(test_word));

    return 0;
}