#include <stdio.h>   // For printf, puts, fgets
#include <stdlib.h>  // For atoi, atof, malloc, free
#include <string.h>  // For strlen, strchr
#include <math.h>    // For logf
#include <limits.h>  // For UINT_MAX (used in RAND_MT_MAX_PLUS_ONE calculation)

// --- Global Variables (replacing decompiler's DAT_xxxx) ---
// These variables are shared across the functions.
double last_pkt_time = 0.0;
double gen_rate = 0.0;
unsigned int DAT_00018030 = 0; // Represents remaining packet length (bytes) or a flag
unsigned int DAT_00018034 = 0; // Represents packet priority
double NextPkt = 0.0;

// --- Global Constants (replacing decompiler's DAT_xxxx) ---
// These are assumed constants based on their usage in the original snippet.
const double CONST_BITRATE_REF = 1000000.0; // DAT_000151f8: e.g., 1 Mbps reference
// RAND_MT_MAX_PLUS_ONE is used to normalize random numbers from rand_mt()
// Assuming rand_mt() returns a value up to UINT_MAX.
const double RAND_MT_MAX_PLUS_ONE = (double)UINT_MAX + 1.0; // DAT_00015200, _DAT_00015208

// --- Manual Generator Specific Globals ---
// Structure for a single entry in the manual packet ring buffer.
// The decompiler's pointer arithmetic suggests this specific layout.
typedef struct ManualPacketEntry {
    double time_since_last;      // Time since the last packet (8 bytes)
    int length;                  // Packet length in bytes (4 bytes)
    int priority;                // Packet priority (4 bytes)
    struct ManualPacketEntry *next; // Pointer to the next entry in the ring (8 bytes on 64-bit)
} ManualPacketEntry;

ManualPacketEntry *ManualRingBuffer = NULL; // Start of the ring buffer
ManualPacketEntry *ManualCurrPkt = NULL;    // Current position in the ring buffer
int repeat = -1; // Flag for manual generator repeat mode (0=no, 1=yes, -1=uninitialized)

// --- Helper Functions (Dummy Implementations) ---
// These functions are called by the snippet but not provided.
// Dummy implementations are given to make the code compilable.
// In a real system, these would be provided by other modules.

/**
 * @brief Reads a line from stdin until a terminator or max_len characters.
 *
 * @param buffer The buffer to store the read characters.
 * @param max_len The maximum number of characters to read (excluding terminator and null).
 * @param terminator The character that signals the end of input (e.g., '\n').
 * @return The number of characters read (excluding terminator and null), or -1 on error/EOF.
 */
int readUntil(char *buffer, int max_len, char terminator) {
    if (fgets(buffer, max_len + 1, stdin) == NULL) { // max_len + 1 for null terminator
        return -1; // Error or EOF
    }
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == terminator) {
        buffer[len - 1] = '\0'; // Remove the terminator character
        return (int)len - 1;    // Return length without the terminator
    }
    return (int)len; // Return actual length if no terminator or partial read
}

/**
 * @brief Generates a random unsigned integer.
 * @note This is a dummy implementation using standard rand().
 *       A real system might use a Mersenne Twister (rand_mt) or similar.
 * @return A pseudo-random unsigned integer.
 */
unsigned int rand_mt(void) {
    return (unsigned int)rand();
}

/**
 * @brief Initializes a circular ring buffer for manual packet entries.
 *
 * @param count The number of entries in the ring buffer.
 * @param buffer_ptr A pointer to a ManualPacketEntry* which will store the allocated buffer.
 * @return 0 on success, -1 on failure.
 */
int InitRingBuffer(int count, ManualPacketEntry **buffer_ptr) {
    if (count <= 0) return -1;
    ManualPacketEntry *buffer = (ManualPacketEntry *)malloc(count * sizeof(ManualPacketEntry));
    if (buffer == NULL) {
        perror("Failed to allocate ring buffer");
        return -1;
    }
    // Link entries to form a circular buffer
    for (int i = 0; i < count; ++i) {
        buffer[i].next = &buffer[(i + 1) % count];
    }
    *buffer_ptr = buffer;
    return 0;
}

// --- Function: InitRandom ---
// Initializes the random packet generator.
// Returns 0 on success, 1 on error.
unsigned int InitRandom(void) {
  char input_buffer[10];
  int read_len;
  unsigned int bit_rate = 0;
  
  last_pkt_time = 0.0;
  // Loop until a valid bit rate (1 to 0x400000 bps) is entered.
  while (bit_rate == 0 || bit_rate > 0x400000) { // 0x400000 = 4,194,304
    printf("What average bit rate do you want the random generator to produce? (1 - %d bps): ",
           0x400000);
    read_len = readUntil(input_buffer, 9, '\n'); // Max 9 chars + null for input
    if (read_len == -1) {
      return 1; // Error reading input
    }
    bit_rate = atoi(input_buffer);
  }
  // Calculate gen_rate. Original snippet has 'X + X', which simplifies to '2 * X'.
  gen_rate = 2.0 * CONST_BITRATE_REF / (double)bit_rate;
  return 0; // Success
}

// --- Function: RandomGenerator ---
// Generates parameters for the next random packet.
// Returns 0 on success.
unsigned int RandomGenerator(void) {
  // Only generate a new packet if the previous one's bytes counter (DAT_00018030) is 0.
  if (DAT_00018030 == 0) {
    unsigned int rand_val1 = rand_mt();
    // Calculate time_delta based on a normalized random number and gen_rate.
    double time_delta = ((double)rand_val1 / RAND_MT_MAX_PLUS_ONE) * gen_rate;
    
    rand_val1 = rand_mt(); // Get another random value for packet length
    unsigned int rand_val2 = rand_mt(); // Get another random value for priority
    
    DAT_00018034 = rand_val2 & 0x3f; // Set packet priority (0-63)
    NextPkt = last_pkt_time + time_delta; // Calculate next packet arrival time
    // Set packet length (64-1500 bytes). 0x59c = 1436. 1436 + 64 = 1500.
    DAT_00018030 = rand_val1 % 0x59c + 0x40; 
    last_pkt_time = NextPkt; // Update last packet time
  }
  return 0; // Success
}

// --- Function: InitPoisson ---
// Initializes the Poisson packet generator.
// Returns 0 on success, 1 on error.
unsigned int InitPoisson(void) {
  char input_buffer[10];
  int read_len;
  unsigned int bit_rate = 0;
  
  last_pkt_time = 0.0;
  // Loop until a valid bit rate (1 to 0x400000 bps) is entered.
  while (bit_rate == 0 || bit_rate > 0x400000) { // 0x400000 = 4,194,304
    printf("What average bit rate do you want the poisson generator to produce? (1 - %d bps): ",
           0x400000);
    read_len = readUntil(input_buffer, 9, '\n'); // Max 9 chars + null for input
    if (read_len == -1) {
      return 1; // Error reading input
    }
    bit_rate = atoi(input_buffer);
  }
  // Calculate gen_rate.
  gen_rate = (double)bit_rate / CONST_BITRATE_REF;
  return 0; // Success
}

// --- Function: CalcDelta ---
// Calculates the time delta for Poisson distribution.
// This function's parameters param_2 and param_3 are used to reconstruct 'gen_rate'.
// Returns a long double value representing the time delta.
long double CalcDelta(unsigned int param_1, unsigned int param_2, unsigned int param_3) {
  // Calculate log term. (float)param_1 / (float)RAND_MT_MAX_PLUS_ONE normalizes param_1 to [0, 1).
  float log_val = logf(1.0f - (float)param_1 / (float)RAND_MT_MAX_PLUS_ONE);
  
  // Reconstruct the double value (gen_rate) from two unsigned int parts.
  // This uses type punning via a union, which is a common pattern for decompilers
  // when a double is implicitly split into two 32-bit values.
  union {
      unsigned long long ull;
      double d;
  } gen_rate_reconstructed;
  // param_3 is assumed to hold the higher 32 bits, param_2 the lower 32 bits.
  gen_rate_reconstructed.ull = ((unsigned long long)param_3 << 32) | param_2;

  // Return the calculated delta, reducing intermediate variables by direct return.
  return -(long double)log_val / (long double)gen_rate_reconstructed.d;
}

// --- Function: PoissonGenerator ---
// Generates parameters for the next Poisson packet.
// Returns 0 on success.
unsigned int PoissonGenerator(void) {
  // Only generate a new packet if the previous one's bytes counter (DAT_00018030) is 0.
  if (DAT_00018030 == 0) {
    // Reinterpret gen_rate (double) as unsigned long long for splitting into two unsigned ints.
    // This is done to match the decompiler's logic for passing gen_rate to CalcDelta.
    union {
        double d;
        unsigned long long ull;
    } gen_rate_u;
    gen_rate_u.d = gen_rate;

    unsigned int gen_rate_high = (unsigned int)(gen_rate_u.ull >> 32);
    unsigned int gen_rate_low = (unsigned int)gen_rate_u.ull;
    
    unsigned int rand_val1 = rand_mt();
    // Calculate time_delta using CalcDelta, passing the split gen_rate.
    long double delta_time = CalcDelta(rand_val1, gen_rate_low, gen_rate_high);
    
    unsigned int rand_val2 = rand_mt(); // For packet length
    unsigned int rand_val3 = rand_mt(); // For packet priority
    
    DAT_00018034 = rand_val3 & 0x3f; // Set packet priority (0-63)
    NextPkt = last_pkt_time + (double)delta_time; // Calculate next packet arrival time
    // Set packet length (64-1500 bytes). 0x59c = 1436. 1436 + 64 = 1500.
    DAT_00018030 = rand_val2 % 0x59c + 0x40; 
    last_pkt_time = NextPkt; // Update last packet time
  }
  return 0; // Success
}

// --- Function: InitManual ---
// Initializes the manual packet generator, prompting user for packet details.
// Returns 0 on success, 1 on error.
unsigned int InitManual(void) {
  char input_buffer[21]; // Max 20 chars for input (18 data + comma + comma + null)
  int read_len;
  
  last_pkt_time = 0.0;
  ManualCurrPkt = NULL; // Initialize to NULL, will point to ManualRingBuffer on first use
  repeat = -1;

  // Prompt user for repeat mode (y/n)
  while (repeat == -1) {
    printf("Should the packet generator repeat the manually entered packets? (y,n): ");
    read_len = readUntil(input_buffer, 2, '\n'); // Read 'y' or 'n' (max 1 char + null)
    if (read_len == -1) {
      return 1; // Error reading input
    }
    if (input_buffer[0] == 'y') {
      repeat = 1;
    } else if (input_buffer[0] == 'n') {
      repeat = 0;
    }
  }

  // Prompt user for number of packets
  int num_packets_to_enter = -1;
  while (num_packets_to_enter < 1 || num_packets_to_enter > 1000) {
    printf("How many packets would you like to enter? (%d - %d): ", 1, 1000);
    read_len = readUntil(input_buffer, 10, '\n'); // Max 10 chars for number + null
    if (read_len == -1) {
      return 1; // Error reading input
    }
    num_packets_to_enter = atoi(input_buffer);
  }

  // Initialize the ring buffer
  if (InitRingBuffer(num_packets_to_enter, &ManualRingBuffer) != 0) {
    return 1; // Error during buffer allocation
  }
  ManualPacketEntry *current_entry = ManualRingBuffer; // Pointer to fill the buffer

  printf("Enter the packets, one per line in this format:\n");
  printf("time since last packet (s, ##.######),length (bytes 64-1500),priority (0-63)\n");

  // Loop to read and parse each packet entry
  for (int i = 0; i < num_packets_to_enter; ++i) {
    read_len = readUntil(input_buffer, 20, '\n'); // Max 20 chars + null (e.g., 99.999999,1500,63)
    if (read_len == -1) {
      return 1; // Error reading input
    }

    char *timestamp_str = input_buffer;
    char *length_str = strchr(input_buffer, ',');
    if (length_str == NULL) {
      puts("Invalid format: missing length or priority separator.");
      return 1; // Error
    }
    *length_str = '\0'; // Null-terminate timestamp string
    length_str++;       // Move pointer past the comma

    char *priority_str = strchr(length_str, ',');
    if (priority_str == NULL) {
      puts("Invalid format: missing priority separator.");
      return 1; // Error
    }
    *priority_str = '\0'; // Null-terminate length string
    priority_str++;       // Move pointer past the comma

    // Validate and parse timestamp
    for (unsigned int char_idx = 0; timestamp_str[char_idx] != '\0'; char_idx++) {
      if (!strchr("0123456789.", timestamp_str[char_idx])) {
        puts("Invalid timestamp: contains non-numeric/non-dot characters.");
        return 1; // Error
      }
    }
    double timestamp = atof(timestamp_str);
    if (timestamp < 0.0) {
      puts("Invalid timestamp: cannot be negative.");
      return 1; // Error
    }

    // Validate and parse length
    for (unsigned int char_idx = 0; length_str[char_idx] != '\0'; char_idx++) {
      if (!strchr("0123456789", length_str[char_idx])) {
        puts("Invalid bytes: contains non-numeric characters.");
        return 1; // Error
      }
    }
    int length = atoi(length_str);
    if (length < 64 || length > 1500) {
      puts("Invalid bytes: must be between 64 and 1500.");
      return 1; // Error
    }

    // Validate and parse priority
    for (unsigned int char_idx = 0; priority_str[char_idx] != '\0'; char_idx++) {
      if (!strchr("0123456789", priority_str[char_idx])) {
        puts("Invalid priority: contains non-numeric characters.");
        return 1; // Error
      }
    }
    int priority = atoi(priority_str);
    if (priority < 0 || priority > 63) { // 0x3f = 63
      puts("Invalid priority: must be between 0 and 63.");
      return 1; // Error
    }

    // Store parsed values into the current ring buffer entry
    current_entry->time_since_last = timestamp;
    current_entry->length = length;
    current_entry->priority = priority;
    current_entry = current_entry->next; // Move to the next entry in the ring
  }
  return 0; // Success
}

// --- Function: ManualGenerator ---
// Generates parameters for the next packet based on manually entered data.
// Returns 1 if a packet was generated, 0 if no more packets are available (and not repeating).
unsigned int ManualGenerator(void) {
  // Only generate a new packet if the previous one's bytes counter (DAT_00018030) is 0.
  if (DAT_00018030 == 0) { 
    if (ManualCurrPkt == NULL) {
      ManualCurrPkt = ManualRingBuffer; // First time, start at the beginning of the buffer
    } else if (ManualCurrPkt == ManualRingBuffer && repeat == 0) {
      // If we've looped back to the start and not repeating, signal no more packets.
      return 0; // No more packets
    }
    
    // Generate packet details from the current entry in the ring buffer
    NextPkt = last_pkt_time + ManualCurrPkt->time_since_last;
    DAT_00018030 = ManualCurrPkt->length;
    DAT_00018034 = ManualCurrPkt->priority;
    
    last_pkt_time = NextPkt;
    ManualCurrPkt = ManualCurrPkt->next; // Advance to the next entry for the next generation
  }
  return 1; // Packet generated successfully
}

// Example main function (not part of the original snippet, but useful for testing)
/*
int main() {
    srand(time(NULL)); // Seed random generator
    printf("--- Initializing Random Generator ---\n");
    if (InitRandom() == 0) {
        printf("Random Generator Initialized. Gen Rate: %f\n", gen_rate);
        for (int i = 0; i < 5; ++i) {
            DAT_00018030 = 0; // Reset packet length to force new packet generation
            RandomGenerator();
            printf("Random Pkt %d: Time=%f, Length=%u, Priority=%u\n", i + 1, NextPkt, DAT_00018030, DAT_00018034);
        }
    } else {
        printf("Failed to initialize Random Generator.\n");
    }

    printf("\n--- Initializing Poisson Generator ---\n");
    if (InitPoisson() == 0) {
        printf("Poisson Generator Initialized. Gen Rate: %f\n", gen_rate);
        for (int i = 0; i < 5; ++i) {
            DAT_00018030 = 0; // Reset packet length to force new packet generation
            PoissonGenerator();
            printf("Poisson Pkt %d: Time=%f, Length=%u, Priority=%u\n", i + 1, NextPkt, DAT_00018030, DAT_00018034);
        }
    } else {
        printf("Failed to initialize Poisson Generator.\n");
    }

    printf("\n--- Initializing Manual Generator ---\n");
    if (InitManual() == 0) {
        printf("Manual Generator Initialized.\n");
        int pkt_count = 0;
        while (1) {
            DAT_00018030 = 0; // Reset packet length to force new packet generation
            unsigned int result = ManualGenerator();
            if (result == 0) {
                printf("No more manual packets.\n");
                break;
            }
            pkt_count++;
            printf("Manual Pkt %d: Time=%f, Length=%u, Priority=%u\n", pkt_count, NextPkt, DAT_00018030, DAT_00018034);
        }
    } else {
        printf("Failed to initialize Manual Generator.\n");
    }
    
    // Clean up allocated memory for ManualRingBuffer
    if (ManualRingBuffer != NULL) {
        free(ManualRingBuffer);
        ManualRingBuffer = NULL;
    }

    return 0;
}
*/