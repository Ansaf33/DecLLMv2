#include <stdio.h> // Required for printf in main function

// Declare a global array to simulate the memory region.
// The original code implies an access pattern that, when interpreted as
// an array of integers, would access 0x400 (1024) elements.
// To make the code compilable and runnable on a standard Linux system
// without causing a segmentation fault, we use a static array.
static int s_simulated_memory[0x400];

// Function: calc_version
int calc_version(void) {
  int local_c = 0; // Initialize local_c when declared
  unsigned int local_8; // Changed 'uint' to standard 'unsigned int'
  
  for (local_8 = 0; local_8 < 0x400; ++local_8) { // Use prefix increment for local_8
    // The original expression '*(int *)((local_8 + 0x10d1f000) * 4)'
    // suggests accessing an array of integers where the base address is
    // effectively (0x10d1f000 * 4) and 'local_8' is an index.
    // For a runnable Linux program, we replace this specific memory address
    // access with an index into our globally declared simulated array.
    local_c += s_simulated_memory[local_8];
  }
  return local_c;
}

// Main function to make the code a complete, compilable, and runnable program.
int main(void) {
    // Initialize the simulated memory array for predictable results.
    // Static storage duration variables are zero-initialized by default,
    // but explicit initialization here makes the output of calc_version consistent.
    for (unsigned int i = 0; i < 0x400; ++i) {
        s_simulated_memory[i] = 1; // Assign a value to each element
    }

    int result = calc_version();
    printf("Calculated version: %d\n", result);
    
    return 0;
}