#include <stdint.h> // For uint32_t

// Constants for Mersenne Twister
#define MT_N 624 // 0x270
#define MT_M 397 // 0x18d (used in gen_mt_state)

// Global state variables
static uint32_t mt_state[MT_N];
static int idx = 0;

// Function: init_mt
void init_mt(uint32_t seed) {
  mt_state[0] = seed;
  for (int i = 1; i < MT_N; ++i) {
    mt_state[i] = (mt_state[i - 1] >> 30 ^ mt_state[i - 1]) * 0x6c078965 + i;
  }
}

// Function: gen_mt_state
void gen_mt_state(void) {
  for (int i = 0; i < MT_N; ++i) {
    uint32_t y = (mt_state[i] & 0x80000000) | (mt_state[(i + 1) % MT_N] & 0x7fffffff);
    mt_state[i] = mt_state[(i + MT_M) % MT_N] ^ (y >> 1);
    if (y & 1) { // If y is odd
      mt_state[i] ^= 0x9908b0df;
    }
  }
}

// Function: rand_mt
uint32_t rand_mt(void) {
  if (idx == 0) {
    gen_mt_state();
  }
  
  uint32_t y = mt_state[idx];
  y ^= (y >> 11);
  y ^= (y & 0x13a58ad) << 7;
  y ^= (y & 0x1df8c) << 15;
  
  idx = (idx + 1) % MT_N;
  
  return y ^ (y >> 18);
}