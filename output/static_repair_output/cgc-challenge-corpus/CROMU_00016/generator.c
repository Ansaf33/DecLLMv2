#include <stdio.h>   // For printf, puts
#include <stdlib.h>  // For atoi, atof
#include <string.h>  // For strlen, strchr
#include <math.h>    // For logf

// --- Global Variables ---
// These are assumed to be defined externally and linked,
// or declared here with initial values if they are constants.
// Placeholder values are used for constants DAT_000151f8, etc.
// In a real scenario, these would likely be `const double` if fixed.

double last_pkt_time = 0.0;
double gen_rate = 0.0;
int DAT_00018030 = 0;   // Represents packet length or remaining packets counter
int DAT_00018034 = 0;   // Represents packet priority/flags
double NextPkt = 0.0;
double *ManualCurrPkt = NULL;
int repeat = -1;
double *ManualRingBuffer = NULL;

// Constants from data segment (placeholder values)
// Adjust these values to match your application's actual constants.
double DAT_000151f8 = 1.0;  // Example: 1.0, often represents a scaling factor or base rate
double DAT_00015200 = 1.0;  // Example: 1.0, often represents a maximum random value or scaling
double _DAT_00015208 = 1.0; // Example: 1.0, often represents a maximum random value or scaling

// --- External Function Prototypes ---
// These functions are assumed to be defined elsewhere in your project.
// Their signatures are inferred from usage in the provided snippet.

// Reads input from stdin until a delimiter or max_len is reached.
// Returns the number of characters read (excluding delimiter) or -1 on error.
// The delimiter character is replaced by a null terminator in the buffer.
int readUntil(char *buffer, int max_len, char delimiter);

// A Mersenne Twister or similar pseudo-random number generator.
// Returns an unsigned integer random value.
unsigned int rand_mt(void);

// Initializes a ring buffer for manual packet entries.
// num_packets: The number of packet entries to allocate.
// buffer_ptr: A pointer to a double* which will be updated to point to the allocated buffer.
// Returns 0 on success, non-zero on error.
int InitRingBuffer(int num_packets, double **buffer_ptr);

// --- Function: InitRandom ---
int InitRandom(void) {
  char buffer[10];
  int chars_read;
  unsigned int rate_bps = 0;

  last_pkt_time = 0.0;
  while ((rate_bps == 0 || (0x400000 < rate_bps))) {
    printf("What average bit rate do you want the random generator to produce? (1 - %u bps): ",
           0x400000);
    chars_read = readUntil(buffer, 9, '\n'); // Max 9 chars + null terminator
    if (chars_read == -1) {
      return -1; // Use -1 for error as per typical UNIX functions
    }
    buffer[chars_read] = '\0'; // Ensure null termination
    rate_bps = (unsigned int)atoi(buffer);
  }
  // Simplified expression: DAT_000151f8 / rate_bps + DAT_000151f8 / rate_bps
  gen_rate = 2.0 * DAT_000151f8 / (double)rate_bps;
  return 0;
}

// --- Function: RandomGenerator ---
int RandomGenerator(void) {
  if (DAT_00018030 == 0) {
    double time_increment = ((double)rand_mt() / DAT_00015200) * gen_rate;
    DAT_00018034 = rand_mt() & 0x3f; // Priority/flags
    NextPkt = last_pkt_time + time_increment;
    DAT_00018030 = rand_mt() % 0x59c + 0x40; // Packet length
    last_pkt_time = NextPkt;
  }
  return 1;
}

// --- Function: InitPoisson ---
int InitPoisson(void) {
  char buffer[10];
  int chars_read;
  unsigned int rate_bps = 0;

  last_pkt_time = 0.0;
  while ((rate_bps == 0 || (0x400000 < rate_bps))) {
    printf("What average bit rate do you want the poisson generator to produce? (1 - %u bps): ",
           0x400000);
    chars_read = readUntil(buffer, 9, '\n'); // Max 9 chars + null terminator
    if (chars_read == -1) {
      return -1;
    }
    buffer[chars_read] = '\0'; // Ensure null termination
    rate_bps = (unsigned int)atoi(buffer);
  }
  gen_rate = (double)rate_bps / DAT_000151f8;
  return 0;
}

// --- Function: CalcDelta ---
// param_2 and param_3 are the lower and upper 32-bit parts of a double value (gen_rate).
long double CalcDelta(unsigned int param_1, unsigned int param_2, unsigned int param_3) {
  // Reconstruct the double value from its 32-bit parts.
  // Assuming param_2 is the lower 32 bits and param_3 is the upper 32 bits.
  double reconstructed_gen_rate = (double)(((unsigned long long)param_3 << 32) | param_2);

  // Use 1.0f for float literal consistency with logf
  float f_val = logf(1.0f - (float)param_1 / (float)_DAT_00015208);
  return -(long double)f_val / (long double)reconstructed_gen_rate;
}

// --- Function: PoissonGenerator ---
int PoissonGenerator(void) {
  if (DAT_00018030 == 0) {
    // Pass gen_rate as two unsigned int parameters to CalcDelta
    unsigned int low_part_gen_rate = (unsigned int)gen_rate;
    unsigned int high_part_gen_rate = (unsigned int)(((unsigned long long)gen_rate) >> 32);

    long double delta_time = CalcDelta(rand_mt(), low_part_gen_rate, high_part_gen_rate);
    DAT_00018034 = rand_mt() & 0x3f; // Priority/flags
    NextPkt = last_pkt_time + (double)delta_time;
    DAT_00018030 = rand_mt() % 0x59c + 0x40; // Packet length
    last_pkt_time = NextPkt;
  }
  return 1;
}

// --- Function: InitManual ---
int InitManual(void) {
  char input_buffer[21]; // Buffer for 20 chars + null terminator
  int chars_read;
  int num_packets_to_enter;
  double current_timestamp;
  int current_length;
  int current_priority;
  
  last_pkt_time = 0.0;
  ManualCurrPkt = NULL; // Reset current packet pointer
  repeat = -1;

  while (repeat == -1) {
    printf("Should the packet generator repeat the manually entered packets? (y,n): ");
    chars_read = readUntil(input_buffer, 2, '\n'); // Read 'y' or 'n'
    if (chars_read == -1) {
      return -1;
    }
    if (chars_read > 0 && input_buffer[0] == 'y') { // Check first char
      repeat = 1;
    } else if (chars_read > 0 && input_buffer[0] == 'n') { // Check first char
      repeat = 0;
    }
  }

  num_packets_to_enter = -1;
  while ((num_packets_to_enter < 1 || (1000 < num_packets_to_enter))) {
    printf("How many packets would you like to enter? (%d - %d): ", 1, 1000);
    chars_read = readUntil(input_buffer, 10, '\n'); // Max 10 chars + null
    if (chars_read == -1) {
      return -1;
    }
    input_buffer[chars_read] = '\0'; // Ensure null termination
    num_packets_to_enter = atoi(input_buffer);
  }

  if (InitRingBuffer(num_packets_to_enter, &ManualRingBuffer) != 0) {
    return -1;
  }

  double *current_ring_buffer_ptr = ManualRingBuffer; // Pointer for filling the buffer
  printf("Enter the packets, one per line in this format:\n");
  printf("time since last packet (s, ##.######),length (bytes 64-1500),priority (0-63)\n");

  for (int i = 0; i < num_packets_to_enter; ++i) {
    chars_read = readUntil(input_buffer, 20, '\n'); // Max 20 chars + null
    if (chars_read == -1) {
      return -1;
    }
    // readUntil might return length including newline, or replace it with null.
    // Ensure null termination at the end of the actual input.
    if (chars_read > 0 && input_buffer[chars_read - 1] == '\n') {
      input_buffer[chars_read - 1] = '\0';
    } else {
      input_buffer[chars_read] = '\0';
    }

    char *token_time = input_buffer;
    char *token_length = strchr(token_time, ',');
    if (token_length == NULL) {
      puts("Invalid format: missing length field.");
      return -1;
    }
    *token_length = '\0'; // Null-terminate time string
    token_length++;       // Move past the comma

    char *token_priority = strchr(token_length, ',');
    if (token_priority == NULL) {
      puts("Invalid format: missing priority field.");
      return -1;
    }
    *token_priority = '\0'; // Null-terminate length string
    token_priority++;       // Move past the comma

    // Validate and convert timestamp
    for (size_t k = 0; k < strlen(token_time); ++k) {
      if (!strchr("0123456789.", token_time[k])) {
        puts("Invalid timestamp: contains non-numeric or non-decimal characters.");
        return -1;
      }
    }
    current_timestamp = atof(token_time);
    if (current_timestamp < 0.0) {
      puts("Invalid timestamp: cannot be negative.");
      return -1;
    }

    // Validate and convert length
    for (size_t k = 0; k < strlen(token_length); ++k) {
      if (!strchr("0123456789", token_length[k])) {
        puts("Invalid bytes: contains non-numeric characters.");
        return -1;
      }
    }
    current_length = atoi(token_length);
    if ((current_length < 64) || (1500 < current_length)) {
      puts("Invalid bytes: must be between 64 and 1500.");
      return -1;
    }

    // Validate and convert priority
    for (size_t k = 0; k < strlen(token_priority); ++k) {
      if (!strchr("0123456789", token_priority[k])) {
        puts("Invalid priority: contains non-numeric characters.");
        return -1;
      }
    }
    current_priority = atoi(token_priority);
    if ((current_priority < 0) || (0x3f < current_priority)) { // 0x3f is 63
      puts("Invalid priority: must be between 0 and 63.");
      return -1;
    }

    // Store the parsed values into the ring buffer structure
    // Each entry is assumed to be 24 bytes: double (8), int (4), int (4), double* (8)
    *current_ring_buffer_ptr = current_timestamp;
    *(int *)((char *)current_ring_buffer_ptr + 8) = current_length;
    *(int *)((char *)current_ring_buffer_ptr + 12) = current_priority;
    
    // Move to the next entry in the ring buffer
    // The 'next' pointer is stored at an offset of 16 bytes
    current_ring_buffer_ptr = *(double **)((char *)current_ring_buffer_ptr + 16);
  }
  return 0;
}

// --- Function: ManualGenerator ---
int ManualGenerator(void) {
  if (DAT_00018030 == 0) {
    if (ManualCurrPkt == NULL) {
      ManualCurrPkt = ManualRingBuffer; // Start from the beginning
    } else if ((ManualCurrPkt == ManualRingBuffer) && (repeat == 0)) {
      return 0; // No more packets and not repeating
    }
    
    // Retrieve values from the current packet entry
    // Each entry is assumed to be 24 bytes: double (8), int (4), int (4), double* (8)
    NextPkt = last_pkt_time + *ManualCurrPkt;
    DAT_00018030 = *(int *)((char *)ManualCurrPkt + 8);  // Packet length
    DAT_00018034 = *(int *)((char *)ManualCurrPkt + 12); // Packet priority
    
    // Advance to the next packet in the ring buffer
    ManualCurrPkt = *(double **)((char *)ManualCurrPkt + 16);
    last_pkt_time = NextPkt;
  }
  return 1;
}