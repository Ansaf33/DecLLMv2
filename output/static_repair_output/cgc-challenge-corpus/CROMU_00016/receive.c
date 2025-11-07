#include <stdio.h>   // For puts, printf
#include <stdlib.h>  // For atoi, atof
#include <string.h>  // For strlen, strcspn, bzero
#include <stdbool.h> // For true
#include <stdint.h>  // For uintptr_t

// Type aliases from decompiled code
typedef int undefined4;
typedef unsigned int uint;
typedef unsigned char byte;

// Forward declarations for function pointers
static int (*generator)(void);

// Global variables (with dummy initializations for compilation)
static double max_wall_clock;
static double NextPkt;
static double wall_clock;
static int DAT_00018030;
static uint DAT_00018a2c;
static uintptr_t* DAT_00018a3c;
static uint DAT_00018034;

// Stub implementations for undefined functions
// These are minimal implementations to allow the provided snippet to compile.
int readUntil(char *buffer, int max_len, int timeout_seconds) {
    if (fgets(buffer, max_len + 1, stdin) != NULL) {
        buffer[strcspn(buffer, "\n")] = 0;
        return strlen(buffer);
    }
    return -1;
}

int InitRandom(void) {
    puts("Initializing Random Generator...");
    return 0;
}

int InitPoisson(void) {
    puts("Initializing Poisson Generator...");
    return 0;
}

int InitManual(void) {
    puts("Initializing Manual Generator...");
    return 0;
}

int RandomGenerator(void) {
    // Placeholder for random packet generation logic
    NextPkt = wall_clock + 0.5;
    DAT_00018030 = 1; // Example packet ID
    DAT_00018034 = 5; // Example priority
    return 1; // Indicate a packet was generated
}

int PoissonGenerator(void) {
    // Placeholder for Poisson packet generation logic
    NextPkt = wall_clock + 1.0;
    DAT_00018030 = 2;
    DAT_00018034 = 3;
    return 1;
}

int ManualGenerator(void) {
    // Placeholder for manual packet generation logic
    NextPkt = wall_clock + 2.0;
    DAT_00018030 = 3;
    DAT_00018034 = 7;
    return 1;
}

// Function: InitReceive
undefined4 InitReceive(void) {
  char input_buffer[10];
  int bytes_read;
  uint generator_choice = 0;
  
  puts("The following packet generators are available:");
  puts("  1 - Random");
  puts("  2 - Poisson");
  puts("  3 - Manual");

  while (generator_choice < 1 || generator_choice > 3) {
    printf("Which generator would you like to use? (1-3): ");
    bytes_read = readUntil(input_buffer, 9, 10);
    if (bytes_read == -1) {
      return 0xffffffff; // Error
    }
    generator_choice = atoi(input_buffer);
  }

  if (generator_choice == 1) {
    if (InitRandom() != 0) {
      return 0xffffffff;
    }
    generator = RandomGenerator;
  } else if (generator_choice == 2) {
    if (InitPoisson() != 0) {
      return 0xffffffff;
    }
    generator = PoissonGenerator;
  } else { // generator_choice must be 3
    if (InitManual() != 0) {
      return 0xffffffff;
    }
    generator = ManualGenerator;
  }

  max_wall_clock = -1.0;
  while(true) {
    if (max_wall_clock >= 0.0 && max_wall_clock <= 10.0) { // Assuming 0-10s range based on prompt
      // bzero(&NextPkt,0x14); // Removed: NextPkt is a double, bzero is inappropriate.
      return 0;
    }
    printf("For how long would you like the simulation to run? (1 - 10s): ");
    bytes_read = readUntil(input_buffer, 9, 10);
    if (bytes_read == -1) {
      return 0xffffffff;
    }
    max_wall_clock = atof(input_buffer);
    if (max_wall_clock < 0.0 || max_wall_clock > 10.0) {
        puts("Invalid duration. Please enter a value between 0 and 10 seconds.");
    }
  }
}

// Function: RX
undefined4 RX(void) {
  unsigned int *current_queue_ptr;
  uint queue_index;
  
  while(true) {
    if (((*generator)() == 0) || (DAT_00018030 == 0) || (wall_clock < NextPkt)) {
      return 0;
    }
    
    queue_index = 0;
    while (queue_index < DAT_00018a2c) {
      current_queue_ptr = (unsigned int *)DAT_00018a3c[queue_index];

      // Accesses based on decompiled offsets and types
      // *(byte *)(current_queue_ptr + 9) refers to byte at offset 9 * sizeof(unsigned int) = 36
      // *(byte *)((int)current_queue_ptr + 0x25) refers to byte at offset 37
      if (DAT_00018034 < *(((byte *)current_queue_ptr) + 36) ||
          *(((byte *)current_queue_ptr) + 37) < DAT_00018034) {
        queue_index = queue_index + 1;
      } else {
        break; // Found a matching queue
      }
    }

    if (queue_index == DAT_00018a2c) {
      break; // No target queue found
    }

    // current_queue_ptr[10] is an unsigned int holding a memory address (uintptr_t)
    uintptr_t pkt_data_addr = current_queue_ptr[10];

    if (current_queue_ptr[1] == 0) {
      *(double *)pkt_data_addr = NextPkt;
      *(int *)(pkt_data_addr + 8) = DAT_00018030;
      *(uint *)(pkt_data_addr + 0xc) = DAT_00018034;
      current_queue_ptr[0xb] = *(uint *)(pkt_data_addr + 0x10);
      current_queue_ptr[1] = current_queue_ptr[1] + 1;
    } else if (current_queue_ptr[1] < current_queue_ptr[0]) {
      *(double *)current_queue_ptr[0xb] = NextPkt; // current_queue_ptr[0xb] is also an address
      *(int *)(current_queue_ptr[0xb] + 8) = DAT_00018030;
      *(uint *)(current_queue_ptr[0xb] + 0xc) = DAT_00018034;
      current_queue_ptr[0xb] = *(uint *)(current_queue_ptr[0xb] + 0x10);
      current_queue_ptr[1] = current_queue_ptr[1] + 1;
    } else {
      current_queue_ptr[6] = current_queue_ptr[6] + 1;
    }
    DAT_00018030 = 0;
  }
  printf("Unable to find target queue for packet with priority %u\n", DAT_00018034);
  return 0xffffffff;
}