#include <stdint.h> // For uint32_t, int32_t, uint64_t
#include <stdio.h>  // For printf
#include <time.h>   // For time(NULL)

// Global state variables for the custom PRNG.
// These variables are typically initialized once at program start.
static uint32_t seed_0;
static int32_t DAT_00018004;

// Function: randint
int randint(int param_1, int param_2) {
  uint32_t old_seed_0 = seed_0; // One intermediate variable to capture the state before modification

  // Update seed_0 using the old value of seed_0.
  // The expression (uint64_t)old_seed_0 * 0x972ad4c5 is computed here.
  seed_0 = (uint32_t)((uint64_t)old_seed_0 * 0x972ad4c5);

  // Update DAT_00018004 using the old_seed_0 and the same 64-bit product expression.
  // The value of 'iVar2' (old_seed_0 * 0x2d17c92c) is inlined.
  // The value of '(uVar1 * 0x972ad4c5 >> 0x20)' is also inlined.
  DAT_00018004 = (int32_t)(old_seed_0 * 0x2d17c92c) + DAT_00018004 * -0x68d52b3b + (int32_t)(((uint64_t)old_seed_0 * 0x972ad4c5) >> 0x20);

  // Calculate and return the random number in the specified range.
  // Handle cases where param_2 - param_1 results in a non-positive range to avoid modulo by zero.
  int32_t range = param_2 - param_1;
  if (range <= 0) {
      // If the range is invalid or zero, return param_1.
      return param_1;
  }
  return param_1 + (int32_t)(seed_0 % (uint32_t)range);
}

// Function to initialize the PRNG state
void init_prng(uint32_t initial_seed) {
    seed_0 = initial_seed;
    DAT_00018004 = (int32_t)initial_seed; // A simple initialization for DAT_00018004
}

int main() {
    // Initialize the PRNG with a time-based seed for varied results on each run.
    init_prng((uint32_t)time(NULL));

    printf("Generating 10 random numbers between 1 and 100 (inclusive):\n");
    for (int i = 0; i < 10; ++i) {
        printf("%d ", randint(1, 101)); // param_2 is an exclusive upper bound
    }
    printf("\n\n");

    printf("Generating 5 random numbers between -50 and 50 (inclusive):\n");
    for (int i = 0; i < 5; ++i) {
        printf("%d ", randint(-50, 51)); // param_2 is an exclusive upper bound
    }
    printf("\n\n");

    printf("Generating 3 random numbers for an edge case (10 to 10):\n");
    for (int i = 0; i < 3; ++i) {
        printf("%d ", randint(10, 10)); // Should return 10 as range is 0
    }
    printf("\n\n");
    
    printf("Generating 3 random numbers for another edge case (10 to 9):\n");
    for (int i = 0; i < 3; ++i) {
        printf("%d ", randint(10, 9)); // Should return 10 as range is negative
    }
    printf("\n");

    return 0;
}