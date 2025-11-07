#include <stdio.h>    // For fprintf, stderr
#include <stdlib.h>   // For exit, random, atoi
#include <string.h>   // For strlen, strcpy, strcmp, strncmp
#include <stdint.h>   // For uint32_t, uint64_t
#include <time.h>     // For time (if srandom is called, which is usually in main)

// --- Global variables and forward declarations for external functions ---

// Assuming curpkt is a pointer to a buffer/packet structure.
// It seems to be used as a char array base for packet data.
char *curpkt;

// Assuming qss is a 2D array for storing key-value pairs.
// 0x800 (2048) entries, each 0x180 (384) bytes long.
// Within each entry: 0x80 (128) for key, 0x100 (256) for value.
#define MAX_QSS_ENTRIES 2048
#define QSS_ENTRY_SIZE  384 // 0x180 bytes per entry
#define QSS_KEY_MAX_LEN 128 // 0x80 bytes for key (including null terminator space)
#define QSS_VAL_MAX_LEN 256 // 0x100 bytes for value (including null terminator space)
char qss[MAX_QSS_ENTRIES][QSS_ENTRY_SIZE];
unsigned int numentries = 0;

// Assuming DAT_00013000 is a constant string for error messages.
const char *DAT_00013000 = "Error: Datagram reception failed.";

// Forward declarations for functions not provided in the snippet.
// These are placeholder declarations to allow compilation.
// Actual implementations would be system-specific (e.g., network I/O).
int recvdgram(void);
void create_resp_pkt(char *pkt, const char *msg);
int senddgram(void);
int sendline(void); // Custom function, likely related to network I/O
int streq(const char *s1, const char *s2); // Custom string equality check
unsigned int str2uint(const char *s); // Custom string to unsigned int conversion
int startswith(const char *str, const char *prefix); // Custom string prefix check

// --- Helper function for termination ---
// Replaces the decompiler's `_terminate()` artifact with a standard exit.
void _terminate(void) {
    fprintf(stderr, "Program terminated due to an unrecoverable error.\n");
    exit(EXIT_FAILURE);
}

// Function: easteregg
int easteregg(void) {
  // Assuming curpkt points to a buffer where relevant data starts at offset 7.
  char *ptr = (char *)(curpkt + 7); 
  
  // Find the first occurrence of 'B'.
  // Added null check to prevent reading past the end of a string.
  while (*ptr != 'B' && *ptr != '\0') { 
    ptr++;
  }
  
  // If 'B' was found, skip over all subsequent 'B's.
  // This positions ptr to the character immediately after a block of 'B's.
  if (*ptr == 'B') {
    while (*ptr == 'B' && *ptr != '\0') {
      ptr++;
    }
  }
  
  // Copy the easter egg message to the determined location.
  // Ensure the destination buffer (`ptr`) has sufficient space.
  strcpy(ptr, "If you do things right, people won\'t be sure that you\'ve done anything at all...");
  
  return 1; // Indicate success
}

// Function: getrand
// This function's original decompiler output was heavily mangled,
// with uninitialized variables and illogical control flow.
// It's re-implemented here to return a 64-bit pseudo-random number,
// as implied by its usage in `do_rand` expecting a `uint64_t` comparison.
// The `param_1` is unused in this simplified, corrected version.
uint64_t getrand(uint32_t param_1) { 
  // Combine two `random()` calls to generate a 64-bit number.
  // `random()` typically returns a `long`.
  uint64_t high_bits = (uint64_t)random();
  uint64_t low_bits = (uint64_t)random();
  
  // Combine into a 64-bit value. Mask low_bits to ensure it fits into 32 bits.
  return (high_bits << 32) | (low_bits & 0xFFFFFFFFUL);
}

// Function: do_rand
int do_rand(void) {
  unsigned int seed_val = 4; // Default seed value
  
  if (numentries == 0) {
    // If no entries, there's nothing to randomize/select from in the qss array.
    return 0; // Failure
  }
  
  // Extract a potential seed value from the packet (offset 8).
  char *param_str = (char *)(curpkt + 8);
  if (*param_str != '\0') {
    seed_val = str2uint(param_str);
  }
  
  // Check for a valid seed_val range.
  if (seed_val >= 9) {
    return 0; // Failure: Invalid seed_val
  }
  
  // Get a 64-bit random value.
  uint64_t rand_val = getrand(seed_val); // `param_1` is passed but unused in current `getrand`.
  
  // Check for a specific "magic" random value and prefix in the packet.
  if ((rand_val == 0x3127a1b73ULL) && 
      (startswith((char *)(curpkt + 10), "0101010101010101") != 0)) {
    // If conditions met, trigger the easter egg.
    return easteregg(); // Returns 1 on success.
  } else {
    // Otherwise, create a standard response.
    // (0U % numentries) is always 0. This means it refers to the first entry in qss.
    // Assuming qss[0] points to the key part of the first entry.
    create_resp_pkt(curpkt, qss[0]); 
    return 1; // Indicate success
  }
}

// Function: do_put
int do_put(void) {
  // The original line `qss[curpkt + 0x27e7] = 0;` was a decompiler artifact and has been removed.
  
  // Check if maximum number of entries has been reached.
  if (numentries >= MAX_QSS_ENTRIES) { 
    return 0; // Failure: Max entries reached
  }
  
  // Extract key string from the packet (offset 8).
  char *key = (char *)(curpkt + 8);
  size_t key_len = strlen(key);
  
  // Validate key length.
  // QSS_KEY_MAX_LEN includes space for the null terminator.
  if (key_len >= QSS_KEY_MAX_LEN) { 
    return 0; // Failure: Key too long
  }
  
  // Extract value string, which is immediately after the key (plus null terminator).
  char *value = key + key_len + 1;
  size_t value_len = strlen(value);
  
  // Validate value length.
  // QSS_VAL_MAX_LEN includes space for the null terminator.
  if (value_len >= QSS_VAL_MAX_LEN) { 
    return 0; // Failure: Value too long
  }
  
  // Check if a key with the same name already exists.
  for (unsigned int i = 0; i < numentries; i++) {
    // An entry is considered "existing" if its first byte is not null.
    // Then compare the incoming key with the stored key.
    if ((qss[i][0] != '\0') && (streq(key, qss[i]) != 0)) {
      return 0; // Failure: Key already exists
    }
  }
  
  // Copy the new key and value into the next available slot in qss.
  // Key is at the start of the entry.
  strcpy(qss[numentries], key);
  // Value is stored after the key part (at offset QSS_KEY_MAX_LEN within the entry).
  strcpy(qss[numentries] + QSS_KEY_MAX_LEN, value); 
  
  numentries++; // Increment the count of stored entries.
  
  create_resp_pkt(curpkt, "Information upload complete.");
  return 1; // Indicate success
}

// Function: do_get
int do_get(void) {
  // Extract the key to search for from the packet (offset 8).
  char *key = (char *)(curpkt + 8);
  
  // Iterate through existing entries to find a matching key.
  for (unsigned int i = 0; i < numentries; i++) {
    if (streq(key, qss[i]) != 0) {
      // Key found, create a response packet with the associated value.
      // The value is stored at offset QSS_KEY_MAX_LEN within the entry.
      create_resp_pkt(curpkt, qss[i] + QSS_KEY_MAX_LEN); 
      return 1; // Indicate success
    }
  }
  
  // Key not found after checking all entries.
  create_resp_pkt(curpkt, "Not found.");
  return 0; // Indicate failure
}

// Function: process_next_dgram
// This function orchestrates the processing of an incoming datagram.
// Returns 0 on success, or a negative integer on various failure conditions.
int process_next_dgram(void) {
  int action_result; // Stores the result of do_rand/do_put/do_get
  
  // Attempt to receive a datagram.
  if (recvdgram() < 0) {
    // If reception fails, prepare and send an error response.
    create_resp_pkt(curpkt, DAT_00013000);
    if (senddgram() < 0) {
      _terminate(); // Critical error: cannot send error response, terminate program.
    }
    return -1; // Return specific error code for reception failure.
  }
  
  // Determine the action to perform based on the command byte (curpkt[1]).
  // Casting to unsigned char handles potential negative char values and ensures switch works as expected.
  switch ((unsigned char)curpkt[1]) {
    case 2: // Command for do_rand
      action_result = do_rand();
      break;
    case 0: // Command for do_put
      action_result = do_put();
      break;
    case 1: // Command for do_get
      action_result = do_get();
      break;
    default:
      // Unknown or unsupported command byte.
      create_resp_pkt(curpkt, "Error: Unknown command."); // Optional: send error response
      if (senddgram() < 0) { // Optional: handle send error for custom response
          _terminate();
      }
      return -1; // Return specific error code for invalid command.
  }
  
  // Check if the specific action (do_rand/do_put/do_get) was successful.
  // These functions return 1 for success, 0 for failure.
  if (action_result == 0) { 
    return -2; // Return specific error code for action failure.
  }
  
  // If the action was successful, send the response datagram prepared by the action function.
  if (senddgram() < 0) {
    _terminate(); // Critical error: cannot send final response, terminate program.
  }
  
  return 0; // Indicate overall success.
}