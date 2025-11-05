#include <stdio.h>    // For puts, printf
#include <stdlib.h>   // For atoi, atof, malloc, free
#include <string.h>   // For memset, strcspn
#include <stdbool.h>  // For true
#include <stdint.h>   // For uintptr_t

// --- Global Type Definitions ---

// PacketSlot structure (20 bytes)
typedef struct {
    double time;
    int id;
    unsigned int priority;
    unsigned int next_slot_ptr_val; // Stores the address of the next PacketSlot
} PacketSlot;

// PacketInfo structure (20 bytes), used for NextPkt
// This matches PacketSlot structure, implying they might be interchangeable or related.
// The bzero(&NextPkt, 0x14) and wall_clock < NextPkt.time usage supports this.
typedef struct {
    double time;
    int id;
    unsigned int priority;
    unsigned int other; // Generic field, could be anything
} PacketInfo;

// QueueEntry structure (using an array of unsigned int and then casting for specific byte access)
// Accesses:
// - current_queue_ptr->data[0] (max_packets)
// - current_queue_ptr->data[1] (current_packets)
// - current_queue_ptr->data[6] (dropped_packets)
// - current_queue_ptr->data[10] (packet_slots_base_ptr_val)
// - current_queue_ptr->data[11] (current_packet_slot_ptr_val)
// - *(unsigned char*)((char*)current_queue_ptr + 36) (min_priority)
// - *(unsigned char*)((char*)current_queue_ptr + 37) (max_priority)
// The `data` array covers all `unsigned int` fields. Byte accesses use `char*` cast.
typedef struct {
    unsigned int data[12]; // Array to hold all unsigned int fields and provide raw access
} RawQueueEntry;

// --- Global Variables ---

double wall_clock = 0.0;
PacketInfo NextPkt; // Initialized by InitReceive
double max_wall_clock; // Initialized by InitReceive
int DAT_00018030 = 0; // Packet ID / status
unsigned int DAT_00018034 = 0; // Packet priority
unsigned int DAT_00018a2c = 0; // Number of queues
RawQueueEntry **DAT_00018a3c = NULL; // Array of pointers to RawQueueEntry

// Function pointer for packet generator
int (*generator)(void);

// --- Helper Functions ---

// Dummy readUntil implementation
// Reads up to `max_len` characters into `buf`, or until newline.
// `buf_size` is the total buffer size, `max_len` is the maximum characters to read (excluding null terminator).
// Returns number of bytes read, or -1 on error.
int readUntil(char *buf, int max_len, int buf_size) {
    if (fgets(buf, buf_size, stdin) == NULL) {
        return -1;
    }
    // Remove trailing newline if present
    buf[strcspn(buf, "\n")] = 0;
    return strlen(buf);
}

// Dummy generator functions
int InitRandom(void) { /* puts("Random generator initialized."); */ return 0; }
int RandomGenerator(void) { return 1; /* Simulate packet generation */ }

int InitPoisson(void) { /* puts("Poisson generator initialized."); */ return 0; }
int PoissonGenerator(void) { return 1; /* Simulate packet generation */ }

int InitManual(void) { /* puts("Manual generator initialized."); */ return 0; }
int ManualGenerator(void) { return 1; /* Simulate packet generation */ }

// --- Main Functions ---

// Function: InitReceive
int InitReceive(void) {
  unsigned int generator_choice;
  char buffer[10];
  int bytes_read;
  
  puts("The following packet generators are available:");
  puts("  1 - Random");
  puts("  2 - Poisson");
  puts("  3 - Manual");

  do {
    printf("Which generator would you like to use? (1-3): ");
    bytes_read = readUntil(buffer, sizeof(buffer) - 1, sizeof(buffer));
    if (bytes_read == -1) {
      return -1; // Use -1 for error return, consistent with original 0xffffffff
    }
    generator_choice = atoi(buffer);
  } while (generator_choice == 0 || generator_choice > 3);

  int init_result;
  switch (generator_choice) {
    case 1:
      init_result = InitRandom();
      generator = RandomGenerator;
      break;
    case 2:
      init_result = InitPoisson();
      generator = PoissonGenerator;
      break;
    case 3:
      init_result = InitManual();
      generator = ManualGenerator;
      break;
    // The do-while loop ensures generator_choice is 1-3, so default is unreachable.
  }
  if (init_result != 0) {
    return -1;
  }

  max_wall_clock = -1.0; // Initialize to an invalid value
  do {
    printf("For how long would you like the simulation to run? (1 - 10s): ");
    bytes_read = readUntil(buffer, sizeof(buffer) - 1, sizeof(buffer));
    if (bytes_read == -1) {
      return -1;
    }
    max_wall_clock = atof(buffer);
  } while (max_wall_clock < 0.0); // Continue as long as it's invalid (less than 0.0)

  // Initialize NextPkt struct (20 bytes) to zeros.
  // This matches the original `bzero(&NextPkt,0x14);` behavior.
  memset(&NextPkt, 0, sizeof(NextPkt));
  return 0;
}

// Function: RX
int RX(void) {
  int generator_status;
  unsigned int queue_idx;
  RawQueueEntry *current_queue_ptr;
  
  while( true ) {
    generator_status = (*generator)();
    // Check if packet generation failed, or no packet ID, or simulation time exceeded next packet time.
    if (generator_status == 0 || DAT_00018030 == 0 || wall_clock < NextPkt.time) {
      return 0;
    }

    queue_idx = 0;
    // Iterate through queues to find a suitable one
    while (queue_idx < DAT_00018a2c) {
      current_queue_ptr = DAT_00018a3c[queue_idx]; // Get pointer to current queue entry

      // Access min_priority at byte offset 36, max_priority at byte offset 37
      // These are accessed by casting the RawQueueEntry pointer to char* and indexing.
      unsigned char min_priority = ((unsigned char*)current_queue_ptr)[36];
      unsigned char max_priority = ((unsigned char*)current_queue_ptr)[37];

      // Check if packet priority is outside the queue's acceptable range
      if (DAT_00018034 < min_priority || max_priority < DAT_00018034) {
        queue_idx++; // Try next queue
      } else {
        break; // Found a suitable queue
      }
    }

    // If no suitable queue was found
    if (queue_idx == DAT_00018a2c) {
      printf("Unable to find target queue for packet with priority %u\n", DAT_00018034);
      return -1; // Use -1 for error
    }

    // At this point, current_queue_ptr points to the selected queue.
    // Access members using current_queue_ptr->data[index]
    unsigned int current_packets = current_queue_ptr->data[1];
    unsigned int max_packets = current_queue_ptr->data[0];

    // Case 1: Queue is empty or has space
    if (current_packets < max_packets) {
      PacketSlot *slot;
      if (current_packets == 0) {
          // Get pointer to the first packet slot.
          // current_queue_ptr->data[10] holds an address which is a PacketSlot*
          slot = (PacketSlot *)(uintptr_t)current_queue_ptr->data[10]; 
      } else {
          // current_queue_ptr->data[11] holds the address of the current slot to use
          slot = (PacketSlot *)(uintptr_t)current_queue_ptr->data[11];
      }
      
      slot->time = NextPkt.time;
      slot->id = DAT_00018030;
      slot->priority = DAT_00018034;
      
      // Update the current_packet_slot_ptr_val to the next slot's address
      current_queue_ptr->data[11] = slot->next_slot_ptr_val;
      current_queue_ptr->data[1]++; // Increment current_packets
    }
    // Case 2: Queue is full, drop packet
    else {
      current_queue_ptr->data[6]++; // Increment dropped_packets
    }
    DAT_00018030 = 0; // Reset packet ID/status
  }
}