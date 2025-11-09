#include <stdio.h>   // For printf, puts, fgets
#include <stdlib.h>  // For atoi, malloc, free, calloc
#include <string.h>  // For strncpy, strlen, strcspn
#include <stddef.h>  // For size_t

// Define custom types based on Ghidra's output for portability
typedef unsigned int uint;
typedef unsigned char byte;
typedef unsigned int undefined4;     // Assuming 32-bit unsigned integer
typedef unsigned long long ulonglong; // Assuming 64-bit unsigned integer
typedef unsigned long long undefined8; // Assuming 64-bit unsigned integer

// Forward declarations for external functions (dummy implementations provided below)
void init_mt(uint);
int readUntil(char *, int, int);
void RX(void);

// Structure for a packet based on memory access patterns
// Offsets derived from TransmitPktFromQueue function logic
struct Packet {
    double arrival_time;            // Offset 0
    unsigned int size_bytes;        // Offset 8 (pdVar1 + 1, assuming double is 8 bytes)
    unsigned int unknown_packet_field_12; // Offset 12 (0xc)
    struct Packet *next_packet;     // Offset 16 (0x10, pdVar1 + 2, assuming pointer is 4 bytes)
};

// Structure for a queue based on memory access patterns
// Offsets derived from usage in various functions
struct Queue {
    int unknown_field_0;            // Offset 0 (placeholder for possible data)
    int num_enqueued_pkts;          // Offset 4
    unsigned int current_tokens;    // Offset 8
    unsigned int max_tokens;        // Offset 12 (0xc)
    unsigned int pkts_transmitted;  // Offset 16 (0x10)
    unsigned int bytes_transmitted; // Offset 20 (0x14)
    unsigned int pkts_dropped;      // Offset 24 (0x18)
    double total_latency;           // Offset 28 (0x1c)
    // Padding or another field to align first_packet_in_queue to offset 0x28
    // If double is 8 bytes, total_latency ends at 0x1c+8 = 0x24.
    // So 4 bytes of padding are needed before 0x28 for a 4-byte pointer.
    undefined4 padding_0x24;
    struct Packet *first_packet_in_queue; // Offset 40 (0x28)
};

// Global variables (initialized with example values for a compilable program)
uint iface;
double DAT_00015820 = 1000.0; // Example constant value
double DAT_00015828 = 1.0;    // Example constant value
double DAT_00018a30;          // Calculated interface speed factor
double wall_clock = 0.0;      // Current simulation time
double max_wall_clock = 100.0; // Example maximum simulation time for TX loop
unsigned int DAT_00018a24 = 0; // Total packets transmitted across all queues
unsigned int DAT_00018a28 = 0; // Total bytes transmitted across all queues
unsigned int DAT_00018a2c = 2; // Number of active queues (e.g., 2 for demonstration)
char DAT_00018a38 = 0;        // Flag for special queue 0 handling (0 = disabled, non-0 = enabled)

// DAT_00018a3c is an array of pointers to Queue structs
// Example: array can hold up to 10 queue pointers. DAT_00018a2c determines active count.
struct Queue *DAT_00018a3c[10];

// Dummy implementation for init_mt: Initializes a "Mersenne Twister" or similar PRNG
void init_mt(uint speed) {
    printf("DEBUG: init_mt called with speed: %u\n", speed);
    // In a real scenario, this would seed a PRNG or initialize hardware.
}

// Dummy implementation for readUntil: Reads input from stdin until newline or max_len
int readUntil(char *buffer, int max_len, int timeout) {
    printf("DEBUG: Enter value (max %d chars): ", max_len);
    if (fgets(buffer, max_len + 1, stdin) != NULL) { // +1 for null terminator
        // Remove trailing newline character if present
        buffer[strcspn(buffer, "\n")] = 0;
        return 0; // Success
    }
    return -1; // Failure
}

// Dummy implementation for RX: Represents receiving data
void RX(void) {
    // printf("DEBUG: RX called\n"); // Uncomment for verbose debug
}

// Function: InitInterface
undefined4 InitInterface(void) {
  char input_buffer[12]; // Buffer for user input (e.g., "4000000\n" is 8 chars + newline + null)
  unsigned int interface_speed;
  
  interface_speed = 0;
  while (interface_speed == 0 || interface_speed > 0x400000) {
    printf("What's the interface speed (in bps up to %u bps): ", 0x400000);
    int read_result = readUntil(input_buffer, sizeof(input_buffer) - 1, 10);
    if (read_result == -1) {
      return 0xffffffff; // Error reading input
    }
    interface_speed = atoi(input_buffer);
  }
  iface = interface_speed;
  init_mt(iface);
  DAT_00018a24 = 0; // Reset total packets transmitted
  DAT_00018a28 = 0; // Reset total bytes transmitted
  // Calculate interface speed factor (e.g., time per bit)
  DAT_00018a30 = DAT_00015820 / (double)iface;
  wall_clock = 0.0; // Reset simulation clock
  return 0; // Success
}

// Function: TransmitPktFromQueue
undefined4 TransmitPktFromQueue(byte queue_idx) {
  if (queue_idx < DAT_00018a2c) {
    struct Queue *pQueue = DAT_00018a3c[queue_idx];
    if (pQueue == NULL || pQueue->first_packet_in_queue == NULL) {
        // No packet to transmit or queue is invalid
        puts("No packet in queue or invalid queue pointer.");
        return 0xffffffff;
    }
    struct Packet *pkt = pQueue->first_packet_in_queue;

    // Update total latency for this queue
    pQueue->total_latency = (wall_clock - pkt->arrival_time) + pQueue->total_latency;
    // Advance wall clock based on transmission time
    wall_clock = wall_clock + DAT_00018a30 +
                 (DAT_00015828 * (double)pkt->size_bytes) / (double)iface;
    
    pQueue->num_enqueued_pkts--;
    if (DAT_00018a38 == 0 || queue_idx != 0) { // If queue 0 is not special OR it's not queue 0
      pQueue->current_tokens -= pkt->size_bytes; // Consume tokens
    }
    pQueue->pkts_transmitted++;
    pQueue->bytes_transmitted += pkt->size_bytes;
    
    DAT_00018a24++; // Increment global packet count
    DAT_00018a28 += pkt->size_bytes; // Increment global byte count
    
    // In a real system, the packet would be freed or moved to a transmitted list.
    // For this example, we clear its data and free it.
    pkt->size_bytes = 0;
    pkt->unknown_packet_field_12 = 0;
    pkt->arrival_time = 0.0;
    
    // Remove packet from the head of the queue
    pQueue->first_packet_in_queue = pkt->next_packet;
    
    free(pkt); // Free the memory of the transmitted packet
    
    return 0; // Success
  }
  else {
    puts("Invalid queue number");
    return 0xffffffff; // Error
  }
}

// Function: ReplinishTokens
void ReplinishTokens(void) {
  for (int i = 0; i < DAT_00018a2c; i++) {
    struct Queue *pQueue = DAT_00018a3c[i];
    if (pQueue != NULL) {
        pQueue->current_tokens = pQueue->max_tokens; // Reset current tokens to max
    }
  }
}

// Function: uint_to_str - Converts an unsigned integer to its string representation
void uint_to_str(unsigned int value, char *buffer) {
  char temp_buffer[32]; // Max 10 digits for uint + null terminator for largest uint
  char *current_pos = temp_buffer;
  int digit_count = 0;
  
  if (buffer == NULL) {
    return;
  }

  if (value == 0) { // Special case for value 0
      *buffer = '0';
      *(buffer + 1) = '\0';
      return;
  }

  // Convert number to string in reverse order (e.g., 123 -> '3', '2', '1')
  while (value != 0) {
    *current_pos = (value % 10) + '0';
    value /= 10;
    current_pos++;
    digit_count++;
  }

  // Reverse the string and copy to the output buffer
  while (digit_count > 0) {
    current_pos--; // Move back to the last written digit in temp_buffer
    *buffer = *current_pos; // Copy digit to output buffer
    buffer++; // Move output buffer pointer
    digit_count--; // Decrement count
  }
  *buffer = '\0'; // Null-terminate the string
}

// Function: print_uint - Prints a prefix string followed by an unsigned integer
void print_uint(char *prefix, unsigned int value) {
  char output_buffer[64]; // Sufficient space for prefix and number string
  
  if (prefix == NULL) {
      prefix = ""; // Handle NULL prefix gracefully
  }

  // Copy prefix to output buffer
  strncpy(output_buffer, prefix, sizeof(output_buffer) - 1);
  output_buffer[sizeof(output_buffer) - 1] = '\0'; // Ensure null termination
  
  size_t prefix_len = strlen(output_buffer);
  
  // Append the string representation of the unsigned int to the buffer
  uint_to_str(value, output_buffer + prefix_len);
  
  printf("%s\n", output_buffer);
}

// Function: PrintStats
void PrintStats(void) {
  printf("wall_clock: %f (s)\n", wall_clock);
  
  for (int i = 0; i < DAT_00018a2c; i++) {
    struct Queue *pQueue = DAT_00018a3c[i];
    if (pQueue == NULL) {
        printf("Queue %d: (NULL - not initialized or freed)\n", i);
        continue;
    }
    printf("Queue %d\n", i);
    printf("  Pkts Transmitted:       %u\n", pQueue->pkts_transmitted);
    printf("  Pkts Dropped:           %u\n", pQueue->pkts_dropped);
    
    if (pQueue->pkts_transmitted == 0) {
      printf("  Average Latency (s):    %f\n", 0.0);
    }
    else {
      double avg_latency = pQueue->total_latency / (double)pQueue->pkts_transmitted;
      printf("  Average Latency (s):    %f\n", avg_latency);
    }
    printf("  Enqueued Pkts:          %d\n", pQueue->num_enqueued_pkts);
    
    int queue_depth_bytes = 0;
    struct Packet *current_pkt = pQueue->first_packet_in_queue;
    // Iterate through enqueued packets to calculate total bytes
    for (uint j = 0; j < pQueue->num_enqueued_pkts && current_pkt != NULL; j++) {
      queue_depth_bytes += current_pkt->size_bytes;
      current_pkt = current_pkt->next_packet;
    }
    printf("  Queue Depth (bytes):    %d\n", queue_depth_bytes);
  }
  printf("Interface Stats\n");
  print_uint("  Pkts Transmitted:       ", DAT_00018a24);
  // Additional global stats could be printed here (e.g., total bytes transmitted, etc.)
}

// Function: TX - Main transmission loop
undefined4 TX(void) {
  unsigned int transmission_result;
  
  ReplinishTokens(); // Replenish tokens for all queues
  
  // Special handling for queue 0 if DAT_00018a38 is set (non-zero)
  if (DAT_00018a38 != 0 && DAT_00018a2c > 0 && DAT_00018a3c[0] != NULL) {
    while (DAT_00018a3c[0]->num_enqueued_pkts != 0) {
      transmission_result = TransmitPktFromQueue(0);
      if (transmission_result != 0) {
        puts("Packet transmission failed for queue 0");
        return 0xffffffff;
      }
      if (wall_clock > max_wall_clock) { // Exit if simulation time limit exceeded
        return 0;
      }
      RX(); // Simulate receiving packets
    }
  }
  
  // Process other queues (or all queues if DAT_00018a38 is 0)
  for (unsigned int queue_idx = 0; queue_idx < DAT_00018a2c; queue_idx++) {
    // Skip queue 0 if it was handled specially (DAT_00018a38 != 0)
    if (DAT_00018a38 == 0 || queue_idx != 0) {
      struct Queue *pQueue = DAT_00018a3c[queue_idx];
      if (pQueue == NULL) {
          continue; // Skip uninitialized queues
      }
      // Transmit packets as long as there are packets and enough tokens
      while (pQueue->num_enqueued_pkts != 0 &&
             pQueue->first_packet_in_queue != NULL && // Ensure there's a packet
             pQueue->first_packet_in_queue->size_bytes <= pQueue->current_tokens) {
        
        transmission_result = TransmitPktFromQueue(queue_idx);
        if (transmission_result != 0) {
          puts("Packet transmission failed");
          return 0xffffffff;
        }
        if (wall_clock > max_wall_clock) { // Exit if simulation time limit exceeded
          return 0;
        }
        RX(); // Simulate receiving packets
      }
    }
  }
  return 0; // Success
}

// Main function to demonstrate and test the functionality
int main() {
    // Initialize queues and packets for demonstration
    printf("Initializing %u queues...\n", DAT_00018a2c);
    for (int i = 0; i < DAT_00018a2c; i++) {
        DAT_00018a3c[i] = (struct Queue*) calloc(1, sizeof(struct Queue));
        if (DAT_00018a3c[i] == NULL) {
            fprintf(stderr, "Failed to allocate memory for queue %d\n", i);
            // Clean up previously allocated queues
            for (int j = 0; j < i; j++) free(DAT_00018a3c[j]);
            return 1;
        }
        // Set initial queue parameters
        DAT_00018a3c[i]->max_tokens = 1000;
        DAT_00018a3c[i]->current_tokens = 1000;
        // Other fields are 0 due to calloc
        
        // Add some dummy packets to queue 0 for testing
        if (i == 0) {
            for (int p = 0; p < 3; p++) {
                struct Packet *new_pkt = (struct Packet*) malloc(sizeof(struct Packet));
                if (new_pkt == NULL) {
                    fprintf(stderr, "Failed to allocate memory for packet\n");
                    // Proper cleanup is needed here in a real app
                    return 1;
                }
                new_pkt->arrival_time = wall_clock + p * 0.05; // Example arrival time
                new_pkt->size_bytes = 100 + p * 10; // Example packet size
                new_pkt->unknown_packet_field_12 = 0;
                new_pkt->next_packet = NULL;

                // Add packet to the end of the queue (simple linked list append)
                if (DAT_00018a3c[i]->first_packet_in_queue == NULL) {
                    DAT_00018a3c[i]->first_packet_in_queue = new_pkt;
                } else {
                    struct Packet *last_pkt = DAT_00018a3c[i]->first_packet_in_queue;
                    while (last_pkt->next_packet != NULL) {
                        last_pkt = last_pkt->next_packet;
                    }
                    last_pkt->next_packet = new_pkt;
                }
                DAT_00018a3c[i]->num_enqueued_pkts++;
            }
            printf("Queue 0 initialized with %d packets.\n", DAT_00018a3c[0]->num_enqueued_pkts);
        }
    }

    printf("\n--- Initializing Interface ---\n");
    if (InitInterface() != 0) {
        fprintf(stderr, "Interface initialization failed.\n");
        // Clean up allocated memory
        for (int i = 0; i < DAT_00018a2c; i++) {
            if (DAT_00018a3c[i] != NULL) {
                struct Packet *current = DAT_00018a3c[i]->first_packet_in_queue;
                while (current != NULL) {
                    struct Packet *next = current->next_packet;
                    free(current);
                    current = next;
                }
                free(DAT_00018a3c[i]);
            }
        }
        return 1;
    }
    printf("Interface speed set to: %u bps\n", iface);
    
    printf("\n--- Running TX Loop ---\n");
    unsigned int tx_status = TX();
    if (tx_status != 0) {
        printf("TX loop ended with error: %u\n", tx_status);
    } else {
        printf("TX loop completed (wall_clock: %f).\n", wall_clock);
    }

    printf("\n--- Printing Final Statistics ---\n");
    PrintStats();

    // Clean up all dynamically allocated memory
    for (int i = 0; i < DAT_00018a2c; i++) {
        if (DAT_00018a3c[i] != NULL) {
            // Free any remaining packets in the queue
            struct Packet *current = DAT_00018a3c[i]->first_packet_in_queue;
            while (current != NULL) {
                struct Packet *next = current->next_packet;
                free(current);
                current = next;
            }
            free(DAT_00018a3c[i]); // Free the queue structure itself
        }
    }

    return 0;
}