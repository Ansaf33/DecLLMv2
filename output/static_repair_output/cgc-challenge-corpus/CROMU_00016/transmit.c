#include <stdio.h>   // For printf, puts, fgets
#include <stdlib.h>  // For atoi, calloc, free
#include <string.h>  // For strncpy, strlen, strcspn
#include <stdbool.h> // For bool type if needed, though not strictly required here

// Define types from decompiler output to standard C types
// Assuming a 32-bit environment where pointers are 4 bytes, matching `undefined4` usage.
typedef unsigned char byte;
typedef unsigned int uint;
typedef int undefined4; // 32-bit integer, used for function return types

// --- Structure Definitions ---

// Packet structure (nodes in a queue's linked list)
// Offsets inferred from decompiler output:
// - arrival_time at 0x0
// - size at 0x8
// - reserved at 0xc (a 4-byte field, set to 0 in TransmitPktFromQueue)
// - next pointer at 0x10 (a 4-byte pointer for 32-bit architecture)
typedef struct Packet {
    double arrival_time; // Offset 0x0 (8 bytes)
    unsigned int size;   // Offset 0x8 (4 bytes)
    unsigned int reserved; // Offset 0xc (4 bytes, e.g., status flags or padding)
    struct Packet *next; // Offset 0x10 (4 bytes, assuming 32-bit pointers)
} Packet;

// Queue structure
// `DAT_00018a3c` is an array of `QueueInfo *`
// Offsets inferred from decompiler output for fields within this structure:
typedef struct QueueInfo {
    // There might be fields before 0x4, but the code only shows access from 0x4 onwards.
    // For simplicity, we assume relevant fields start here.
    int pkts_in_queue;     // Offset 0x4
    int tokens;            // Offset 0x8
    int max_tokens;        // Offset 0xc
    int pkts_transmitted;  // Offset 0x10
    int bytes_transmitted; // Offset 0x14
    int pkts_dropped;      // Offset 0x18
    double total_latency;  // Offset 0x1c (8 bytes)
    Packet *head_packet;   // Offset 0x28 (pointer to the first Packet in the queue)
} QueueInfo;

// --- Global Variables (with dummy initializations for compilation) ---
unsigned int iface = 0; // Interface speed in bps
double wall_clock = 0.0; // Current simulation time
double DAT_00015820 = 1000.0; // A constant, possibly base time for a unit of data
double DAT_00018a30 = 0.0;    // Calculated based on DAT_00015820 / iface
int DAT_00018a24 = 0;         // Global packets transmitted counter
int DAT_00018a28 = 0;         // Global total bytes transmitted
int DAT_00018a2c = 2;         // Number of queues (dummy: 2 queues)
double DAT_00015828 = 1.0;    // A constant, possibly per-byte transmission time factor
char DAT_00018a38 = 1;        // A flag (e.g., 1 for enabled, 0 for disabled)
double max_wall_clock = 1000.0; // Simulation end time

// Global array of queue pointers. DAT_00018a2c determines its effective size.
QueueInfo *DAT_00018a3c[2] = {NULL, NULL}; // Initialized to NULL, will be allocated in InitInterface

// --- Forward Declarations for Functions ---
void init_mt(unsigned int seed);
int readUntil(char *buffer, int max_len, int base);
void RX(void);

// --- Dummy Function Implementations (to make the code compilable) ---

// Dummy init_mt function
void init_mt(unsigned int seed) {
    // In a real scenario, this would initialize a Mersenne Twister or similar PRNG.
    // printf("init_mt called with seed: %u\n", seed);
}

// Dummy readUntil implementation using fgets, replacing decompiler's unknown function.
// It reads a line from stdin, up to max_len characters, and removes the newline.
int readUntil(char *buffer, int max_len, int base) {
    if (fgets(buffer, max_len + 1, stdin) == NULL) { // +1 for null terminator
        return -1; // Error reading input
    }
    // Remove newline character if present
    buffer[strcspn(buffer, "\n")] = '\0';
    return 0; // Success
}

// Dummy RX function
void RX(void) {
    // In a real system, this would handle receiving packets.
    // For compilation, it does nothing.
}

// --- Original Functions (Refactored) ---

// Function: InitInterface
int InitInterface(void) { // Changed return type to int
    char input_buffer[12]; // Buffer for user input
    int read_status;       // Status from readUntil
    uint speed_bps;        // Interface speed

    speed_bps = 0;
    // Loop until speed_bps is valid (non-zero and <= 0x400000)
    while (speed_bps == 0 || speed_bps > 0x400000) {
        printf("What's the interface speed (in bps up to %u bps): ", 0x400000);
        read_status = readUntil(input_buffer, sizeof(input_buffer) - 1, 10); // -1 for null terminator
        if (read_status == -1) {
            return -1; // Indicate error
        }
        speed_bps = atoi(input_buffer);
    }
    iface = speed_bps;
    init_mt(speed_bps);

    // Allocate and initialize queues based on DAT_00018a2c
    for (int i = 0; i < DAT_00018a2c; ++i) {
        DAT_00018a3c[i] = (QueueInfo *)calloc(1, sizeof(QueueInfo));
        if (DAT_00018a3c[i] == NULL) {
            fprintf(stderr, "Failed to allocate memory for queue %d\n", i);
            // Clean up previously allocated queues to prevent memory leaks
            for (int j = 0; j < i; ++j) {
                free(DAT_00018a3c[j]);
                DAT_00018a3c[j] = NULL;
            }
            return -1; // Indicate error
        }
        // Initialize default values for the queue
        DAT_00018a3c[i]->max_tokens = 1000; // Dummy value for max tokens
    }

    DAT_00018a24 = 0; // Global packet counter
    DAT_00018a28 = 0; // Global byte counter
    DAT_00018a30 = DAT_00015820 / (double)iface; // Calculate based on interface speed
    wall_clock = 0;
    return 0;
}

// Function: TransmitPktFromQueue
int TransmitPktFromQueue(byte param_1) { // Changed return type to int
    if (param_1 < DAT_00018a2c) {
        QueueInfo *current_queue = DAT_00018a3c[param_1];
        
        // Basic checks for queue and packet existence
        if (current_queue == NULL || current_queue->head_packet == NULL) {
            // This case might be reached if pkts_in_queue > 0 but head_packet is NULL,
            // or if the queue itself was not initialized.
            return -1; // Indicate an error or nothing to transmit
        }

        Packet *pkt_to_transmit = current_queue->head_packet;

        // Update total latency for the queue
        current_queue->total_latency += (wall_clock - pkt_to_transmit->arrival_time);

        // Update wall_clock based on transmission time
        wall_clock += DAT_00018a30 + (DAT_00015828 * (double)pkt_to_transmit->size) / (double)iface;

        // Decrement packet count in queue
        current_queue->pkts_in_queue--;

        // Decrement tokens for the queue if specific conditions are met
        if (DAT_00018a38 != '\0' || param_1 != 0) {
            current_queue->tokens -= pkt_to_transmit->size;
        }

        // Increment queue-specific stats
        current_queue->pkts_transmitted++;
        current_queue->bytes_transmitted += pkt_to_transmit->size;

        // Increment global stats
        DAT_00018a24++;
        DAT_00018a28 += pkt_to_transmit->size;

        // Remove packet from queue: advance head_packet and free the transmitted packet
        current_queue->head_packet = pkt_to_transmit->next;
        
        // Clear the packet's contents (as per original decompiler output)
        // This is done before freeing, so it's writing to valid memory.
        pkt_to_transmit->size = 0;
        pkt_to_transmit->reserved = 0;
        pkt_to_transmit->arrival_time = 0.0;

        free(pkt_to_transmit); // Free the transmitted packet

        return 0; // Success
    } else {
        puts("Invalid queue number");
        return -1; // Indicate error
    }
}

// Function: ReplinishTokens
void ReplinishTokens(void) {
    for (int i = 0; i < DAT_00018a2c; ++i) {
        if (DAT_00018a3c[i]) { // Check if queue pointer is valid
            DAT_00018a3c[i]->tokens = DAT_00018a3c[i]->max_tokens;
        }
    }
}

// Function: uint_to_str
void uint_to_str(uint param_1, char *param_2) {
    char buffer[32]; // Temporary buffer for string conversion
    int i = 0;       // Index for buffer
    
    if (param_2 != NULL) {
        // Handle zero explicitly, as the loop won't run for 0
        if (param_1 == 0) {
            buffer[i++] = '0';
        } else {
            // Convert digits in reverse order
            while (param_1 != 0) {
                buffer[i] = (char)(param_1 % 10) + '0'; // (char)param_1 + ((char)(param_1 / 10 << 2) + (char)(param_1 / 10)) * -2 + '0' is equivalent to param_1 % 10 + '0'
                param_1 /= 10;
                i++;
            }
        }
        // Copy digits in correct order to destination
        while (i > 0) {
            i--;
            *param_2 = buffer[i];
            param_2++;
        }
        *param_2 = '\0'; // Null-terminate the string
    }
}

// Function: print_uint
void print_uint(char *param_1, uint param_2) {
    char combined_buffer[64]; // Buffer large enough for prefix and number

    // Copy param_1 into combined_buffer, ensuring null termination
    strncpy(combined_buffer, param_1, sizeof(combined_buffer) - 1);
    combined_buffer[sizeof(combined_buffer) - 1] = '\0';

    // Append the uint to the string
    uint_to_str(param_2, combined_buffer + strlen(combined_buffer));
    printf("%s\n", combined_buffer); // Use %s for string
}

// Function: PrintStats
void PrintStats(void) {
    printf("wall_clock: %.6f (s)\n", wall_clock); // Use %.6f for double precision

    for (int i = 0; i < DAT_00018a2c; ++i) {
        QueueInfo *current_queue = DAT_00018a3c[i];
        if (current_queue == NULL) {
            printf("Queue %d (not initialized)\n", i);
            continue;
        }

        printf("Queue %d\n", i);
        printf("  Pkts Transmitted:       %d\n", current_queue->pkts_transmitted);
        printf("  Pkts Dropped:           %d\n", current_queue->pkts_dropped);

        if (current_queue->pkts_transmitted == 0) {
            printf("  Average Latency (s):    %.6f\n", 0.0);
        } else {
            double avg_latency = current_queue->total_latency / (double)current_queue->pkts_transmitted;
            printf("  Average Latency (s):    %.6f\n", avg_latency);
        }

        printf("  Enqueued Pkts:          %d\n", current_queue->pkts_in_queue);

        int queue_depth_bytes = 0;
        Packet *current_packet = current_queue->head_packet;
        while (current_packet != NULL) {
            queue_depth_bytes += current_packet->size;
            current_packet = current_packet->next;
        }
        printf("  Queue Depth (bytes):    %d\n", queue_depth_bytes);
    }
    printf("Interface Stats\n");
    print_uint("  Pkts Transmitted:       ", DAT_00018a24);
}

// Function: TX
int TX(void) { // Changed return type to int
    ReplinishTokens();

    // Special handling for queue 0 if DAT_00018a38 is not null ('\0')
    if (DAT_00018a38 != '\0') {
        QueueInfo *queue0 = DAT_00018a3c[0];
        if (queue0) { // Ensure queue0 is initialized
            while (queue0->pkts_in_queue != 0) {
                if (TransmitPktFromQueue(0) != 0) {
                    puts("Packet transmission failed");
                    return -1;
                }
                if (wall_clock > max_wall_clock) { // Check for simulation end
                    return 0;
                }
                RX(); // Simulate receiving packets
            }
        }
    }

    // General handling for all queues
    // This loop replaces the original `do...while(true)` structure with an explicit for loop.
    for (uint current_queue_idx = 0; current_queue_idx < DAT_00018a2c; ++current_queue_idx) {
        // If DAT_00018a38 is null ('\0'), or it's not queue 0 (queue 0 was handled above if DAT_00018a38 is not null)
        if (DAT_00018a38 == '\0' || current_queue_idx != 0) {
            QueueInfo *current_queue = DAT_00018a3c[current_queue_idx];
            if (current_queue == NULL) {
                continue; // Skip uninitialized queues
            }

            // Transmit packets from the current queue as long as there are packets
            // and tokens are sufficient for the head packet's size.
            while (current_queue->pkts_in_queue != 0 &&
                   current_queue->head_packet != NULL &&
                   current_queue->head_packet->size <= current_queue->tokens) {

                if (TransmitPktFromQueue((byte)current_queue_idx) != 0) {
                    puts("Packet transmission failed");
                    return -1;
                }
                if (wall_clock > max_wall_clock) { // Check for simulation end
                    return 0;
                }
                RX(); // Simulate receiving packets
            }
        }
    }
    return 0; // All transmissions handled or simulation ended
}