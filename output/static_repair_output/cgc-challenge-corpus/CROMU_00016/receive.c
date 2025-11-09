#include <stdio.h>    // For puts, printf
#include <stdlib.h>   // For atoi, atof
#include <string.h>   // For memset (bzero replacement)
#include <unistd.h>   // For read (if readUntil uses it, though it's a placeholder)
#include <stdbool.h>  // For true
#include <stdint.h>   // For uintptr_t

// Type definitions for decompiled types
typedef unsigned int uint;
typedef int undefined4; // Assuming 4-byte return value

// Function pointer type for generators
typedef int (*GeneratorFunc)(void);

// Global variables (placeholders, types inferred from usage)
GeneratorFunc generator;
double max_wall_clock;
double NextPkt;
int DAT_00018030;       // Used as int
unsigned int DAT_00018a2c; // Used as loop bound (number of queue items)
unsigned int **DAT_00018a3c; // Array of uint pointers (pointers to queue items)
unsigned int DAT_00018034; // Used as uint (packet priority)
double wall_clock;      // Current simulation time

// --- Placeholder functions for compilation ---

// Dummy readUntil implementation
// Reads up to max_len-1 characters or until newline, stores in buffer.
// Returns number of characters read (excluding newline) or -1 on error.
// Timeout is ignored for simplicity.
int readUntil(char *buffer, int max_len, int timeout) {
    if (fgets(buffer, max_len, stdin) == NULL) {
        return -1;
    }
    // Remove trailing newline if present
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len-1] == '\n') {
        buffer[len-1] = '\0';
    }
    return (int)len;
}

// Dummy Init functions
int InitRandom(void) {
    puts("Initializing Random generator...");
    return 0; // Success
}
int InitPoisson(void) {
    puts("Initializing Poisson generator...");
    return 0; // Success
}
int InitManual(void) {
    puts("Initializing Manual generator...");
    return 0; // Success
}

// Dummy Generator functions
int RandomGenerator(void) {
    // printf("Random packet generated.\n");
    return 1; // Indicate success (non-zero)
}
int PoissonGenerator(void) {
    // printf("Poisson packet generated.\n");
    return 1; // Indicate success (non-zero)
}
int ManualGenerator(void) {
    // printf("Manual packet generated.\n");
    return 1; // Indicate success (non-zero)
}

// --- Original functions, fixed and optimized ---

// Function: InitReceive
undefined4 InitReceive(void) {
  char input_buffer[10]; // Renamed local_1e for clarity
  uint generator_choice = 0; // Renamed local_10 for clarity and to reflect its purpose
  
  puts("The following packet generators are available:");
  puts("  1 - Random");
  puts("  2 - Poisson");
  puts("  3 - Manual");

  // Loop until a valid choice (1-3) is made
  while (generator_choice == 0 || generator_choice > 3) {
    printf("Which generator would you like to use? (1-3): ");
    // Reduced intermediate variable local_14 by direct check
    if (readUntil(input_buffer, sizeof(input_buffer), 10) == -1) {
      return 0xffffffff;
    }
    generator_choice = atoi(input_buffer);
  }

  if (generator_choice == 1) {
    // Reduced intermediate variable iVar1 by direct check
    if (InitRandom() != 0) {
      return 0xffffffff;
    }
    generator = RandomGenerator;
  } else if (generator_choice == 2) {
    // Reduced intermediate variable iVar1 by direct check
    if (InitPoisson() != 0) {
      return 0xffffffff;
    }
    generator = PoissonGenerator;
  } else { // generator_choice must be 3 due to loop condition
    // Reduced intermediate variable iVar1 by direct check
    if (InitManual() != 0) {
      return 0xffffffff;
    }
    generator = ManualGenerator;
  }

  max_wall_clock = -1.0;
  while (true) {
    if (max_wall_clock >= 0.0) { // Condition reversed for clarity
      memset(&NextPkt, 0, sizeof(NextPkt)); // Replaced bzero with memset
      return 0;
    }
    printf("For how long would you like the simulation to run? (1 - 10s): ");
    // Reduced intermediate variable local_14 by direct check
    if (readUntil(input_buffer, sizeof(input_buffer), 10) == -1) {
        break; // Exit loop on error
    }
    max_wall_clock = atof(input_buffer);
  }
  return 0xffffffff; // Error return if readUntil breaks the loop
}

// Function: RX
undefined4 RX(void) {
  unsigned int *current_queue_item; // Renamed local_14 for clarity
  uint local_loop_counter; // Renamed local_10 for clarity
  
  while (true) {
    // Reduced iVar1 by direct check
    if (((*generator)() == 0) || (DAT_00018030 == 0) || (wall_clock < NextPkt)) {
      return 0;
    }

    local_loop_counter = 0;
    // The original loop condition was complex. This re-structures it for readability
    // while maintaining the original logic: find the first queue item that meets the criteria.
    while (local_loop_counter < DAT_00018a2c) {
      // Access DAT_00018a3c as an array of unsigned int pointers
      current_queue_item = DAT_00018a3c[local_loop_counter];

      // Cast to unsigned char* for byte-level access using array indexing
      unsigned char *byte_ptr = (unsigned char *)current_queue_item;

      // Check the conditions that would cause the original loop to CONTINUE
      // If these conditions are met, increment counter and continue search
      if (DAT_00018034 < byte_ptr[9] || byte_ptr[0x25] < DAT_00018034) {
          local_loop_counter++;
      } else {
          // If conditions are NOT met, this is the item we want, so break the search loop
          break;
      }
    }

    if (local_loop_counter == DAT_00018a2c) {
        break; // No target queue found
    }

    // Now current_queue_item points to the selected item
    // Assuming current_queue_item[10] and current_queue_item[0xb] store memory addresses
    // Using uintptr_t for safe casting of unsigned int values to pointers.
    if (current_queue_item[1] == 0) {
      *(double *)(uintptr_t)current_queue_item[10] = NextPkt;
      *(int *)(uintptr_t)(current_queue_item[10] + 8) = DAT_00018030;
      *(uint *)(uintptr_t)(current_queue_item[10] + 0xc) = DAT_00018034;
      current_queue_item[0xb] = *(uint *)(uintptr_t)(current_queue_item[10] + 0x10);
      current_queue_item[1]++; // current_queue_item[1] = current_queue_item[1] + 1;
    } else if (current_queue_item[1] < *current_queue_item) {
      *(double *)(uintptr_t)current_queue_item[0xb] = NextPkt;
      *(int *)(uintptr_t)(current_queue_item[0xb] + 8) = DAT_00018030;
      *(uint *)(uintptr_t)(current_queue_item[0xb] + 0xc) = DAT_00018034;
      current_queue_item[0xb] = *(uint *)(uintptr_t)(current_queue_item[0xb] + 0x10);
      current_queue_item[1]++; // current_queue_item[1] = current_queue_item[1] + 1;
    } else {
      current_queue_item[6]++; // current_queue_item[6] = current_queue_item[6] + 1;
    }
    DAT_00018030 = 0;
  }
  printf("Unable to find target queue for packet with priority %u\n", DAT_00018034); // Fixed format specifier
  return 0xffffffff;
}

// Main function to demonstrate usage
int main() {
    // Initialize global variables with example values
    DAT_00018030 = 0;
    DAT_00018a2c = 5; // Example: array of 5 queue items
    DAT_00018034 = 5; // Example packet priority
    wall_clock = 0.0;
    NextPkt = 0.0;

    // Allocate memory for DAT_00018a3c (an array of pointers to queue item structures)
    DAT_00018a3c = (unsigned int **)calloc(DAT_00018a2c, sizeof(unsigned int *));
    if (DAT_00018a3c == NULL) {
        perror("Failed to allocate DAT_00018a3c");
        return 1;
    }

    // Allocate memory for each "queue item" and associated data
    for (uint i = 0; i < DAT_00018a2c; ++i) {
        // Assume a queue item struct/memory block size of at least 0x40 bytes (64 bytes)
        // to cover all direct accesses (e.g., byte_ptr[0x25], current_queue_item[10]).
        DAT_00018a3c[i] = (unsigned int *)calloc(1, 0x40); 
        if (DAT_00018a3c[i] == NULL) {
            perror("Failed to allocate queue item");
            // Basic cleanup for already allocated items
            for (uint j = 0; j < i; ++j) {
                free((void*)(uintptr_t)DAT_00018a3c[j][10]); // Free associated data buffer
                free(DAT_00018a3c[j]); // Free queue item
            }
            free(DAT_00018a3c);
            return 1;
        }

        // Initialize some values within the dummy queue item for RX logic
        DAT_00018a3c[i][0] = 10; // *current_queue_item
        DAT_00018a3c[i][1] = 0;  // current_queue_item[1]

        // Create a dummy buffer for the data pointed to by current_queue_item[10]
        // This buffer needs to be large enough to hold a double, int, uint, uint.
        void *data_buffer_for_ptrs = calloc(1, sizeof(double) + sizeof(int) + sizeof(uint) + sizeof(uint));
        if (data_buffer_for_ptrs == NULL) {
            perror("Failed to allocate data buffer for pointers");
            // Cleanup all allocated memory
            for (uint j = 0; j <= i; ++j) {
                free(DAT_00018a3c[j]);
            }
            free(DAT_00018a3c);
            return 1;
        }
        // Store the address of this buffer in current_queue_item[10]
        DAT_00018a3c[i][10] = (uintptr_t)data_buffer_for_ptrs;
        
        // Initialize byte_ptr[9] and byte_ptr[0x25] for RX logic
        unsigned char *byte_ptr = (unsigned char *)DAT_00018a3c[i];
        byte_ptr[9] = (unsigned char)(DAT_00018034 + i); // Example value
        byte_ptr[0x25] = (unsigned char)(DAT_00018034 + i); // Example value
    }


    printf("--- Initializing Receiver ---\n");
    if (InitReceive() != 0) {
        printf("InitReceive failed.\n");
        // Cleanup global allocations before exiting
        for (uint i = 0; i < DAT_00018a2c; ++i) {
            if (DAT_00018a3c[i]) {
                free((void*)(uintptr_t)DAT_00018a3c[i][10]);
                free(DAT_00018a3c[i]);
            }
        }
        free(DAT_00018a3c);
        return 1;
    }
    printf("InitReceive successful. Max wall clock: %f\n", max_wall_clock);

    printf("--- Running RX loop (example) ---\n");
    // Simulate some conditions for the RX function
    wall_clock = 1.0;
    NextPkt = 0.5; // Next packet due before wall_clock, so it will be processed
    DAT_00018030 = 1; // Packet size/data, non-zero to proceed
    DAT_00018034 = 5; // Example priority for packet

    undefined4 rx_result = RX();
    if (rx_result != 0) {
        printf("RX function encountered an error.\n");
    } else {
        printf("RX function completed successfully.\n");
    }

    // Final cleanup of global allocations
    for (uint i = 0; i < DAT_00018a2c; ++i) {
        if (DAT_00018a3c[i]) {
            free((void*)(uintptr_t)DAT_00018a3c[i][10]); // Free the data buffer pointed to by [10]
            free(DAT_00018a3c[i]); // Free the queue item itself
        }
    }
    free(DAT_00018a3c); // Free the array of queue item pointers

    return 0;
}