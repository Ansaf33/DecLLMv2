#include <stdio.h>   // For printf
#include <stdlib.h>  // For NULL
#include <stdint.h>  // For explicit integer types if desired, but unsigned int is fine

// --- Global variables ---
// 'last' stores the previous random value, initialized on first call to randint
static unsigned int last;
// 'initialized' flag for one-time initialization of randint state
static char initialized = 0; // '\0' is 0

// 'DAT_ARRAY' corresponds to DAT_4347c000.
// It's a lookup table used by randint. Size 1024 implied by 0x3ff mask.
// Initialize with some arbitrary values for a compilable example.
// In a real scenario, this data would likely come from a specific source (e.g., a ROM or pre-calculated table).
static unsigned int DAT_ARRAY[1024];

// '_DAT_4347c000' is an initial seed value.
// Using the literal value from the snippet.
#define INITIAL_SEED 0x4347c000U // Use U suffix for unsigned int literal

// --- Function: randint ---
// Generates a pseudo-random unsigned integer.
// Based on the provided snippet, using a custom XOR-shift-like logic with a lookup table.
unsigned int randint(void) {
  if (initialized == 0) { // Check if it's the first call
    last = INITIAL_SEED;  // Initialize 'last' with the seed value
    
    // Initialize DAT_ARRAY. This is an assumption as the original snippet
    // does not show how DAT_4347c000 is populated.
    // For a functional example, we need some data here.
    // A simple deterministic fill for reproducibility.
    for (int i = 0; i < 1024; ++i) {
      DAT_ARRAY[i] = (unsigned int)(i * 0x9e3779b9U + 0xDEADBEEFU); // Arbitrary pattern
    }
    initialized = 1; // Mark as initialized
  }

  // Calculate the next random value using the complex XOR logic and lookup table.
  // The original expression involved pointer arithmetic with *(uint *)(&DAT_4347c000 + offset)
  // which is translated to DAT_ARRAY[index].
  // Intermediate variables are reduced by directly using array indexing and inline calculations.
  last = last ^ DAT_ARRAY[(DAT_ARRAY[last & 0x3ffU] & 0x3ffU)] ^
               DAT_ARRAY[(last * (int)DAT_ARRAY[last & 0x3ffU] & 0x3ffU)];
  
  return last;
}

// --- Function: my_rand (renamed from rand to avoid conflict with standard library) ---
// Fills a buffer with pseudo-random bytes.
// 'buffer': Pointer to the buffer to fill. Assumed to be unsigned char* based on usage.
// 'count': Number of bytes to fill.
// 'out_count': Optional pointer to store the actual number of bytes filled.
// Returns 0 on success.
int my_rand(unsigned char *buffer, unsigned int count, unsigned int *out_count) {
  unsigned int i; // Loop counter

  for (i = 0; i < count; i++) {
    // Call randint to get a random unsigned int, then cast to unsigned char.
    // This directly assigns to the buffer without an explicit intermediate variable 'uVar1'.
    buffer[i] = (unsigned char)randint();
  }
  
  // If an output count pointer is provided, store the final count.
  if (out_count != NULL) {
    *out_count = i;
  }
  
  return 0;
}

// --- main function (for demonstration and compilability) ---
int main(void) {
  printf("Demonstrating randint:\n");
  for (int i = 0; i < 5; ++i) {
    printf("Random int %d: %u\n", i + 1, randint());
  }

  printf("\nDemonstrating my_rand:\n");
  unsigned char random_bytes[20];
  unsigned int actual_count;

  // Fill 10 bytes
  my_rand(random_bytes, 10, &actual_count);
  printf("Filled %u bytes: ", actual_count);
  for (int i = 0; i < 10; ++i) {
    printf("%02x ", random_bytes[i]);
  }
  printf("\n");

  // Fill another 10 bytes into the same buffer, starting at offset 10
  my_rand(random_bytes + 10, 10, NULL); // No out_count needed here
  printf("Filled another 10 bytes: ");
  for (int i = 10; i < 20; ++i) {
    printf("%02x ", random_bytes[i]);
  }
  printf("\n");

  return 0;
}