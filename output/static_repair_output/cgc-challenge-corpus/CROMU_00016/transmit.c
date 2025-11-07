#include <stdio.h>
#include <stdlib.h> // For atoi
#include <string.h> // For strncpy, strlen
#include <stdint.h> // For explicit integer types if needed, but standard pointers are usually fine

// Type definitions based on typical decompiler output and context
typedef int undefined4;
typedef unsigned int uint;
typedef unsigned char byte;
// undefined8 is not used in the final reduced code, but if it were, long long would be appropriate.

// Forward declarations for functions not provided in the snippet
// Assuming common signatures based on usage. These would typically be in other source files or libraries.
// For compilation, simple declarations are sufficient, and mock implementations are provided below.
int readUntil(char* buffer, size_t max_len, int base);
void init_mt(unsigned int seed);
void RX(void);

// Structure definitions inferred from pointer arithmetic and field access
// These are designed to match the offsets and types implied by the original code
// assuming a 64-bit system where pointers are 8 bytes.
typedef struct Packet {
    double arrival_time; // offset 0x0
    unsigned int size;   // offset 0x8 (assuming 8-byte double, then 4-byte uint)
    int field_0xc;       // offset 0xc (4 bytes, possibly padding or another field)
    struct Packet* next; // offset 0x10 (8 bytes for 64-bit pointer)
} Packet;

typedef struct Queue {
    // Placeholder for unknown fields if any, before 0x4
    int field_0x0_0x3;      // 4 bytes, placeholder
    int pkts_in_queue;      // offset 0x4
    int tokens;             // offset 0x8
    int max_tokens;         // offset 0xc
    int pkts_transmitted;   // offset 0x10
    int bytes_transmitted;  // offset 0x14
    int pkts_dropped;       // offset 0x18
    double total_latency;   // offset 0x1c (8 bytes)
    // Placeholder for unknown fields if any, before 0x28
    int field_0x24_0x27;    // 4 bytes, placeholder
    Packet* head_packet;    // offset 0x28 (8 bytes for 64-bit pointer)
} Queue;

// Global variables inferred from usage
// Initialized to prevent linker errors if not defined elsewhere.
unsigned int iface = 0;
int DAT_00018a24 = 0; // Pkts Transmitted (total)
int DAT_00018a28 = 0; // Bytes Transmitted (total)
double DAT_00018a30 = 0.0;
double wall_clock = 0.0;
double DAT_00015820 = 1.0; // Placeholder, assumed to be some constant (e.g., simulation constant)
double DAT_00015828 = 1.0; // Placeholder, assumed to be some constant (e.g., bytes_to_time_factor)
unsigned int DAT_00018a2c = 1; // Number of queues, initialized to 1 for example
Queue** DAT_00018a3c = NULL; // Array of pointers to Queue structures, needs allocation
char DAT_00018a38 = 0; // Boolean flag, e.g., '0' for false
double max_wall_clock = 1000.0; // Placeholder, maximum simulation time

// Mock implementations for compilation purposes
// In a real application, these would be defined elsewhere.
int readUntil(char* buffer, size_t max_len, int base) {
    if (fgets(buffer, max_len, stdin) != NULL) {
        buffer[strcspn(buffer, "\n")] = 0; // Remove newline character
        return 0; // Success
    }
    return -1; // Failure
}

void init_mt(unsigned int seed) {
    // Placeholder for initialization of a random number generator or similar.
    // printf("init_mt called with seed %u\n", seed);
}

void RX(void) {
    // Placeholder for receive logic.
    // printf("RX called\n");
}

// Function: InitInterface
undefined4 InitInterface(void) {
  char local_20 [12];
  int local_14;
  uint local_10;
  
  local_10 = 0;
  while (local_10 == 0 || local_10 > 0x400000) {
    printf("What's the interface speed (in bps up to %u bps): ", 0x400000);
    local_14 = readUntil(local_20, sizeof(local_20) - 1, 10);
    if (local_14 == -1) {
      return 0xffffffff;
    }
    local_10 = atoi(local_20);
  }
  iface = local_10;
  init_mt(local_10);
  DAT_00018a24 = 0;
  DAT_00018a28 = 0;
  DAT_00018a30 = DAT_00015820 / (double)local_10;
  wall_clock = 0;
  return 0;
}

// Function: TransmitPktFromQueue
undefined4 TransmitPktFromQueue(byte param_1) {
  if (param_1 < DAT_00018a2c) {
    Queue* q = DAT_00018a3c[param_1];
    if (q == NULL || q->head_packet == NULL) { // Safety check for null queue or empty queue
        puts("Attempted to transmit from an empty or uninitialized queue slot.");
        return 0xffffffff;
    }
    Packet* pkt = q->head_packet;

    q->total_latency += (wall_clock - pkt->arrival_time);
    wall_clock += DAT_00018a30 + (DAT_00015828 * (double)pkt->size) / (double)iface;
    
    q->pkts_in_queue--;
    
    if (!DAT_00018a38 || param_1 != 0) {
      q->tokens -= pkt->size;
    }
    q->pkts_transmitted++;
    q->bytes_transmitted += pkt->size;
    
    DAT_00018a24++;
    DAT_00018a28 += pkt->size;
    
    // Reset packet fields (or free the packet if it's dynamically allocated)
    pkt->size = 0;
    pkt->field_0xc = 0;
    pkt->arrival_time = 0.0;
    
    q->head_packet = pkt->next;
    // In a real system, 'pkt' would likely be freed or moved to a free list here.
    return 0;
  }
  else {
    puts("Invalid queue number");
    return 0xffffffff;
  }
}

// Function: ReplinishTokens
void ReplinishTokens(void) {
  for (unsigned int i = 0; i < DAT_00018a2c; ++i) {
    Queue* q = DAT_00018a3c[i];
    if (q != NULL) { // Safety check
        q->tokens = q->max_tokens;
    }
  }
}

// Function: uint_to_str
void uint_to_str(uint param_1, char *param_2) {
  char local_2c [32];
  int local_c = 0;
  char *local_8 = local_2c;
  
  if (param_2 != NULL) {
    if (param_1 == 0) { // Handle case for 0 explicitly
        *local_8 = '0';
        local_8++;
        local_c++;
    } else {
        do {
            *local_8 = (param_1 % 10) + '0';
            param_1 /= 10;
            local_8++;
            local_c++;
        } while (param_1 != 0);
    }

    // Reverse the string of digits
    while (local_c > 0) {
      local_8--;
      *param_2 = *local_8;
      param_2++;
      local_c--;
    }
    *param_2 = '\0';
  }
}

// Function: print_uint
void print_uint(char *param_1, unsigned int param_2) {
  size_t sVar1;
  char local_30 [32];
  
  strncpy(local_30, param_1, sizeof(local_30) - 1);
  local_30[sizeof(local_30) - 1] = '\0'; // Ensure null termination
  sVar1 = strlen(local_30);
  uint_to_str(param_2, local_30 + sVar1);
  printf("%s\n", local_30);
  return;
}

// Function: PrintStats
void PrintStats(void) {
  double dVar1;
  int local_18; // Total bytes in queue for current queue
  
  printf("wall_clock: %f (s)\n", wall_clock);
  for (unsigned int i = 0; i < DAT_00018a2c; ++i) {
    Queue* q = DAT_00018a3c[i];
    if (q == NULL) { // Safety check
        printf("Queue %u: (NULL)\n", i);
        continue;
    }

    printf("Queue %u\n", i);
    printf("  Pkts Transmitted:       %d\n", q->pkts_transmitted);
    printf("  Pkts Dropped:           %d\n", q->pkts_dropped);
    
    if (q->pkts_transmitted == 0) {
      printf("  Average Latency (s):    %f\n", 0.0);
    } else {
      dVar1 = q->total_latency / (double)q->pkts_transmitted;
      printf("  Average Latency (s):    %f\n", dVar1);
    }
    printf("  Enqueued Pkts:          %d\n", q->pkts_in_queue);
    
    local_18 = 0;
    Packet* current_pkt = q->head_packet;
    for (unsigned int j = 0; j < q->pkts_in_queue; ++j) {
      if (current_pkt == NULL) { // Safety check for corrupted linked list
          printf("  Warning: Queue %u has less packets than expected, list ended prematurely.\n", i);
          break;
      }
      local_18 += current_pkt->size;
      current_pkt = current_pkt->next;
    }
    printf("  Queue Depth (bytes):    %d\n", local_18);
  }
  printf("Interface Stats\n");
  print_uint("  Pkts Transmitted:       ", DAT_00018a24);
  return;
}

// Function: TX
undefined4 TX(void) {
  ReplinishTokens();
  if (DAT_00018a38) { // If DAT_00018a38 is true (non-zero)
    Queue* q0 = DAT_00018a3c[0];
    if (q0 != NULL) { // Safety check for queue 0
        while (q0->pkts_in_queue != 0 && q0->head_packet != NULL) {
            if (TransmitPktFromQueue(0) != 0) {
                puts("Packet transmission failed");
                return 0xffffffff;
            }
            if (wall_clock > max_wall_clock) {
                return 0;
            }
            RX();
        }
    }
  }
  unsigned int i = 0;
  do {
    if (i >= DAT_00018a2c) { // Exit condition for the do-while loop
      return 0;
    }
    if (!DAT_00018a38 || i != 0) { // If DAT_00018a38 is false OR it's not queue 0
      Queue* q_i = DAT_00018a3c[i];
      if (q_i != NULL) { // Safety check for current queue
          while (q_i->pkts_in_queue != 0 && q_i->head_packet != NULL && q_i->head_packet->size <= q_i->tokens) {
            if (TransmitPktFromQueue(i) != 0) {
              puts("Packet transmission failed");
              return 0xffffffff;
            }
            if (wall_clock > max_wall_clock) {
              return 0;
            }
            RX();
          }
      }
    }
    i++;
  } while(1); // Loop indefinitely until an explicit return statement is hit
}