#include <stdint.h> // For uint32_t

// Constants for Mersenne Twister
#define MT_N 624 // 0x270
#define MT_M 397 // 0x18d
#define MT_A 0x9908b0dfUL

// Global state for Mersenne Twister
static uint32_t mt_state[MT_N];
static int idx = 0; // Current index in the state array

// Function: init_mt
void init_mt(uint32_t seed) {
  mt_state[0] = seed;
  for (int i = 1; i < MT_N; ++i) {
    mt_state[i] = (mt_state[i - 1] >> 30 ^ mt_state[i - 1]) * 0x6c078965UL + i;
  }
  idx = 0; // Reset index after initialization
}

// Function: gen_mt_state
void gen_mt_state(void) {
  for (int i = 0; i < MT_N; ++i) {
    uint32_t y = (mt_state[i] & 0x80000000UL) | (mt_state[(i + 1) % MT_N] & 0x7fffffffUL);
    mt_state[i] = mt_state[(i + MT_M) % MT_N] ^ (y >> 1);
    if (y & 1) { // If y is odd
      mt_state[i] ^= MT_A;
    }
  }
  idx = 0; // Reset index after generating new state
}

// Function: rand_mt
uint32_t rand_mt(void) {
  if (idx == 0) {
    gen_mt_state();
  }

  uint32_t temp = mt_state[idx];
  temp ^= (temp >> 11);
  temp ^= (temp & 0x13a58adUL) << 7;
  temp ^= (temp & 0x1df8cUL) << 15;
  temp ^= (temp >> 18);

  idx = (idx + 1) % MT_N;
  return temp;
}