#include <stdint.h> // For uint32_t, though unsigned int is used directly for portability

// Define constants for Mersenne Twister
#define MT_N 624    // 0x270
#define MT_M 397    // 0x18d
#define MT_MATRIX_A 0x9908b0dfU // Constant for gen_mt_state
#define MT_UPPER_MASK 0x80000000U // Most significant bit (MSB)
#define MT_LOWER_MASK 0x7FFFFFFFU // 31 least significant bits

// Tempering constants for rand_mt
#define MT_TEMPER_SHIFT_1 11
#define MT_TEMPER_MASK_2 0x13a58adU
#define MT_TEMPER_SHIFT_2 7
#define MT_TEMPER_MASK_3 0x1df8cU
#define MT_TEMPER_SHIFT_3 15
#define MT_TEMPER_SHIFT_4 18

// Global state for the Mersenne Twister
static unsigned int mt_state[MT_N];
static int idx = 0; // Current index in the state array

// Function: init_mt
void init_mt(unsigned int seed) {
  mt_state[0] = seed;
  for (int i = 1; i < MT_N; i++) {
    // Original: ((uint)(&mt_state)[i + -1] >> 0x1e ^ (&mt_state)[i + -1]) * 0x6c078965 + i;
    // 0x1e is 30.
    mt_state[i] = ((mt_state[i - 1] >> 30) ^ mt_state[i - 1]) * 0x6c078965U + (unsigned int)i;
  }
  idx = 0; // Reset index after initialization
}

// Function: gen_mt_state
void gen_mt_state(void) {
  for (int i = 0; i < MT_N; i++) {
    // y combines the MSB of current state and LSBs of next state
    unsigned int y = (mt_state[i] & MT_UPPER_MASK) | (mt_state[(i + 1) % MT_N] & MT_LOWER_MASK);

    mt_state[i] = mt_state[(i + MT_M) % MT_N] ^ (y >> 1);

    if ((y & 1U) != 0) { // If y is odd (last bit is 1)
      mt_state[i] ^= MT_MATRIX_A;
    }
  }
}

// Function: rand_mt
unsigned int rand_mt(void) {
  if (idx == 0) {
    gen_mt_state();
  }

  unsigned int tempered_value = mt_state[idx];

  // Tempering transformations
  tempered_value ^= (tempered_value >> MT_TEMPER_SHIFT_1);
  tempered_value ^= (tempered_value & MT_TEMPER_MASK_2) << MT_TEMPER_SHIFT_2;
  tempered_value ^= (tempered_value & MT_TEMPER_MASK_3) << MT_TEMPER_SHIFT_3;

  idx = (idx + 1) % MT_N;

  return tempered_value ^ (tempered_value >> MT_TEMPER_SHIFT_4);
}