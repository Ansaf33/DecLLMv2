#include <stdint.h>
#include <stdio.h>
#include <stdlib.h> // For NULL

static uint32_t last_seed;
static char initialized = 0;

#define TABLE_SIZE 1024
static uint32_t data_table[TABLE_SIZE];

#define INITIAL_SEED_VALUE 0x4347c000U

uint32_t randint(void) {
  if (!initialized) {
    last_seed = INITIAL_SEED_VALUE;
    // Initialize data_table with some values. In a real scenario, this might
    // be loaded from a specific source or generated more robustly.
    for (int i = 0; i < TABLE_SIZE; ++i) {
        data_table[i] = (uint32_t)(i * 0x9e3779b9U + 0xc141f129U);
    }
    initialized = 1;
  }

  uint32_t index_base = last_seed & 0x3ff;
  
  uint32_t term1_index = data_table[index_base] & 0x3ff;
  
  // Note the explicit cast to int for multiplication, as implied in the original snippet.
  uint32_t term2_index = (last_seed * (int)data_table[index_base]) & 0x3ff;

  last_seed = last_seed ^ data_table[term1_index] ^ data_table[term2_index];
  
  return last_seed;
}

// Renamed from 'rand' to avoid conflict with standard library rand() and to clarify its parameters.
// The original 'rand' function snippet implied these arguments from stack offsets in decompiled code.
int custom_rand_fill(uint32_t *buffer, uint32_t count, uint32_t *written_count_ptr) {
  if (buffer == NULL) {
    if (written_count_ptr != NULL) {
      *written_count_ptr = 0;
    }
    return -1; // Indicate error
  }

  for (uint32_t i = 0; i < count; ++i) {
    buffer[i] = randint();
  }
  
  if (written_count_ptr != NULL) {
    *written_count_ptr = count;
  }
  
  return 0; // Indicate success
}

int main() {
  const int num_values = 5;
  uint32_t random_buffer[num_values];
  uint32_t actual_count_written;

  printf("Generating %d random numbers:\n", num_values);
  int result = custom_rand_fill(random_buffer, num_values, &actual_count_written);

  if (result == 0) {
    for (int i = 0; i < actual_count_written; ++i) {
      printf("Value %d: %u\n", i + 1, random_buffer[i]);
    }
  } else {
    printf("Error generating random numbers.\n");
  }

  return 0;
}