#include <stdio.h>   // For printf, puts
#include <stdlib.h>  // For atoi, atof, malloc, rand
#include <string.h>  // For strchr, strlen
#include <math.h>    // For logf
#include <stddef.h>  // For NULL

// --- Global variables and constants ---
// Placeholder values for constants. In a real application, these would be defined elsewhere
// or have specific numerical values.
static const double DAT_000151f8 = 1000.0; 
static const double DAT_00015200 = 4294967295.0; // Max value for unsigned int (2^32 - 1)
static const float _DAT_00015208 = 4294967295.0f; // Max value for unsigned int (2^32 - 1)

// Global variables based on usage in the snippet
static unsigned int DAT_00018030 = 0; // Represents packet length or a counter
static unsigned int DAT_00018034 = 0; // Represents packet priority

static double gen_rate = 0.0;
static double last_pkt_time = 0.0;
static double NextPkt = 0.0;

// Structure for manual packet information, inferred from InitManual
typedef struct PacketInfo {
    double time_delta;
    int length;
    int priority;
    struct PacketInfo *next; // Pointer for ring buffer
} PacketInfo;

static PacketInfo *ManualRingBuffer = NULL;
static PacketInfo *ManualCurrPkt = NULL;
static int repeat = -1; // -1 for unset, 0 for no, 1 for yes

// --- Forward declarations for external functions ---
// These functions are not provided in the snippet, so dummy implementations are included
// for compilation purposes.
extern unsigned int rand_mt(void);
extern int readUntil(char *buffer, int max_len, char term_char);
extern int InitRingBuffer(int num_packets, PacketInfo **buffer_ptr);

// --- Dummy implementations for external functions (for compilation) ---
unsigned int rand_mt(void) {
    // Placeholder for a Mersenne Twister or similar random number generator.
    // Using stdlib's rand() for a minimal compilable example.
    return (unsigned int)rand();
}

int readUntil(char *buffer, int max_len, char term_char) {
    int count = 0;
    int c;

    // Read characters until max_len is reached, EOF, or term_char is found.
    while (count < max_len && (c = getchar()) != EOF && c != term_char) {
        buffer[count++] = (char)c;
    }
    buffer[count] = '\0'; // Null-terminate the string

    // If term_char was not consumed (e.g., max_len was reached first, or EOF)
    // and input remains in the buffer (not EOF), clear the rest of the line.
    if (c != term_char && c != EOF) {
        while ((c = getchar()) != '\n' && c != EOF);
    }
    
    // Return -1 if no characters were read and EOF was encountered immediately.
    if (count == 0 && c == EOF) {
        return -1;
    }
    return count;
}

int InitRingBuffer(int num_packets, PacketInfo **buffer_ptr) {
    if (num_packets <= 0) {
        return -1;
    }
    *buffer_ptr = (PacketInfo *)malloc(num_packets * sizeof(PacketInfo));
    if (*buffer_ptr == NULL) {
        return -1;
    }

    // Link packets in a ring
    for (int i = 0; i < num_packets; ++i) {
        (*buffer_ptr)[i].next = &(*buffer_ptr)[(i + 1) % num_packets];
    }
    return 0;
}

// --- Fixed Functions ---

// Function: InitRandom
unsigned int InitRandom(void) {
  char buffer[10];
  int read_len;
  unsigned int rate_input;
  
  last_pkt_time = 0.0;
  rate_input = 0;
  while (rate_input == 0 || rate_input > 0x400000) {
    printf("What average bit rate do you want the random generator to produce? (1 - %u bps): ",
           0x400000);
    read_len = readUntil(buffer, 9, '\n'); 
    if (read_len == -1) {
      return 0xffffffff;
    }
    rate_input = atoi(buffer);
  }
  gen_rate = 2.0 * DAT_000151f8 / (double)rate_input;
  return 0;
}

// Function: RandomGenerator
unsigned int RandomGenerator(void) {
  if (DAT_00018030 == 0) {
    unsigned int rand_val_time = rand_mt();
    double time_delta = ((double)rand_val_time / DAT_00015200) * gen_rate;
    
    DAT_00018034 = rand_mt() & 0x3f; // Priority (0-63)
    NextPkt = last_pkt_time + time_delta;
    DAT_00018030 = rand_mt() % 0x59c + 0x40; // Length (64-1500) or counter
    last_pkt_time = NextPkt;
  }
  return 1;
}

// Function: InitPoisson
unsigned int InitPoisson(void) {
  char buffer[10];
  int read_len;
  unsigned int rate_input;
  
  last_pkt_time = 0.0;
  rate_input = 0;
  while (rate_input == 0 || rate_input > 0x400000) {
    printf("What average bit rate do you want the poisson generator to produce? (1 - %u bps): ",
           0x400000);
    read_len = readUntil(buffer, 9, '\n');
    if (read_len == -1) {
      return 0xffffffff;
    }
    rate_input = atoi(buffer);
  }
  gen_rate = (double)rate_input / DAT_000151f8;
  return 0;
}

// Function: CalcDelta
// Assumes param_2 and param_3 in the original snippet were parts of a double 'rate_val'.
// Passing 'rate_val' directly simplifies the function call and logic.
long double CalcDelta(unsigned int rand_val, double rate_val) {
  float log_result = logf(1.0f - (float)rand_val / _DAT_00015208);
  return -(long double)log_result / (long double)rate_val;
}

// Function: PoissonGenerator
unsigned int PoissonGenerator(void) {
  if (DAT_00018030 == 0) {
    unsigned int rand_val_for_delta = rand_mt();
    long double time_delta_ld = CalcDelta(rand_val_for_delta, gen_rate);
    
    DAT_00018034 = rand_mt() & 0x3f; // Priority (0-63)
    NextPkt = last_pkt_time + (double)time_delta_ld;
    DAT_00018030 = rand_mt() % 0x59c + 0x40; // Length (64-1500) or counter
    last_pkt_time = NextPkt;
  }
  return 1;
}

// Function: InitManual
unsigned int InitManual(void) {
  char buffer[21]; // Buffer for user input: 20 chars + null terminator
  int read_len;
  int num_packets = -1;
  
  last_pkt_time = 0.0;
  ManualCurrPkt = NULL; // Initialize to NULL
  repeat = -1;

  while (repeat == -1) {
    printf("Should the packet generator repeat the manually entered packets? (y,n): ");
    // Reads into buffer[1], max 2 chars ('y', 'n', or '\n')
    read_len = readUntil(buffer + 1, 1, '\n'); 
    if (read_len == -1) {
      return 0xffffffff;
    }
    if (buffer[1] == 'y') {
      repeat = 1;
    } else if (buffer[1] == 'n') {
      repeat = 0;
    }
  }

  while (num_packets < 1 || num_packets > 1000) {
    printf("How many packets would you like to enter? (%u - %u): ", 1, 1000);
    // Reads into buffer[1], max 10 chars (9 digits + '\n')
    read_len = readUntil(buffer + 1, 9, '\n'); 
    if (read_len == -1) {
      return 0xffffffff;
    }
    num_packets = atoi(buffer + 1);
  }

  if (InitRingBuffer(num_packets, &ManualRingBuffer) != 0) {
    return 0xffffffff;
  }
  PacketInfo *current_pkt_node = ManualRingBuffer;

  printf("Enter the packets, one per line in this format:\n");
  printf("time since last packet (s, ##.######),length (bytes 64-1500),priority (0-63)\n");

  for (int p_idx = 0; p_idx < num_packets; ++p_idx) {
    // Reads into buffer[1], max 20 chars (19 data + '\n')
    read_len = readUntil(buffer + 1, 19, '\n'); 
    if (read_len == -1) {
      return 0xffffffff;
    }

    char *time_str = buffer + 1;
    char *length_str = strchr(time_str, ',');
    if (length_str == NULL) {
      puts("Invalid format: missing comma after time");
      return 0xffffffff;
    }
    *length_str = '\0'; // Null-terminate time_str
    length_str++;       // Point to start of length string

    char *priority_str = strchr(length_str, ',');
    if (priority_str == NULL) {
      puts("Invalid format: missing comma after length");
      return 0xffffffff;
    }
    *priority_str = '\0'; // Null-terminate length_str
    priority_str++;       // Point to start of priority string

    // Validate and parse time since last packet
    for (size_t i = 0; time_str[i] != '\0'; ++i) {
      if (strchr("0123456789.", time_str[i]) == NULL) {
        puts("Invalid timestamp: contains non-numeric or non-decimal characters");
        return 0xffffffff;
      }
    }
    double time_since_last = atof(time_str);
    if (time_since_last < 0.0) {
      puts("Invalid timestamp: negative value");
      return 0xffffffff;
    }

    // Validate and parse packet length
    for (size_t i = 0; length_str[i] != '\0'; ++i) {
      if (strchr("0123456789", length_str[i]) == NULL) {
        puts("Invalid bytes: contains non-numeric characters");
        return 0xffffffff;
      }
    }
    int length_bytes = atoi(length_str);
    if (length_bytes < 64 || length_bytes > 1500) {
      puts("Invalid bytes: out of range (64-1500)");
      return 0xffffffff;
    }

    // Validate and parse packet priority
    for (size_t i = 0; priority_str[i] != '\0'; ++i) {
      if (strchr("0123456789", priority_str[i]) == NULL) {
        puts("Invalid priority: contains non-numeric characters");
        return 0xffffffff;
      }
    }
    int priority = atoi(priority_str);
    if (priority < 0 || priority > 63) {
      puts("Invalid priority: out of range (0-63)");
      return 0xffffffff;
    }
    
    // Assign values to the current packet node in the ring buffer
    current_pkt_node->time_delta = time_since_last;
    current_pkt_node->length = length_bytes;
    current_pkt_node->priority = priority;
    current_pkt_node = current_pkt_node->next; // Move to the next node
  }
  return 0; // Successfully entered all packets
}

// Function: ManualGenerator
unsigned int ManualGenerator(void) {
  if (DAT_00018030 == 0) {
    if (ManualCurrPkt == NULL) {
      ManualCurrPkt = ManualRingBuffer;
    } else if (ManualCurrPkt == ManualRingBuffer && repeat == 0) {
      return 0; // No more packets and not repeating
    }
    
    NextPkt = last_pkt_time + ManualCurrPkt->time_delta;
    DAT_00018030 = ManualCurrPkt->length;
    DAT_00018034 = ManualCurrPkt->priority;
    ManualCurrPkt = ManualCurrPkt->next; // Move to the next packet in the ring
    last_pkt_time = NextPkt;
  }
  return 1; // Packet generated (or will be when DAT_00018030 counts down)
}