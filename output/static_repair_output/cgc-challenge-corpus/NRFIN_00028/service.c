#include <stdio.h>    // For printf, fprintf, stderr
#include <stdlib.h>   // For calloc, free, exit, size_t
#include <string.h>   // For strcpy, strlen, memset, strncmp, memcpy
#include <unistd.h>   // For ssize_t, recv
#include <sys/socket.h> // For recv (standard for socket operations)

// Define custom types
typedef unsigned char byte;
typedef unsigned int uint;
typedef unsigned char undefined;
typedef unsigned int undefined4; // Assuming 4-byte undefined is unsigned int
typedef void (*code)(void); // Function pointer type for segment code

// Mock/External function declarations
// These functions are not provided in the snippet, so they are declared as extern
// with best-guess signatures for compilation. In a real project, their
// implementations or correct headers would be provided.

// sendline: Sends a message to a file descriptor. Returns < 0 on error.
extern int sendline(int fd, const char *msg, size_t len);

// allocate/deallocate: Memory management for executable segments.
// 'ptr' is a pointer to the memory address to be allocated/deallocated.
// 'protection_flags' likely relates to mmap permissions (e.g., PROT_READ | PROT_EXEC).
extern int allocate(void **ptr, size_t size, int protection_flags);
extern int deallocate(void *ptr, size_t size);

// _terminate: Exits the program due to a fatal error.
void _terminate() {
    fprintf(stderr, "Program terminated due to an unhandled error.\n");
    exit(EXIT_FAILURE);
}

// Global data (placeholders for addresses found in the decompiled code)
extern char *responses[]; // Array of string pointers for responses
extern char *trusted[];   // Array of string pointers for trusted segments

// Data segments from main - assumed to be unsigned int for XOR operations
extern unsigned int DAT_4347c000;
extern unsigned int DAT_4347c00c;
extern unsigned int DAT_4347c004;
extern unsigned int DAT_4347c008;
extern char DAT_00013127[4]; // Assumed to be a 4-byte data block

// Helper function to send an error message and terminate the program
void handle_error_and_terminate(int fd, const char *msg, int exit_code) {
    if (msg) {
        sendline(fd, msg, strlen(msg));
    }
    fprintf(stderr, "Fatal Error: %s\n", msg ? msg : "Unknown error");
    exit(exit_code);
}

// Function: scramble
// Implements a RC4-like cipher/hashing scheme.
// param_1: Input data buffer (key stream source / data to be scrambled).
// param_2: Output buffer (where the scrambled data is stored).
// param_3: Length of the input data (param_1).
void scramble(const byte *param_1, byte *param_2, uint param_3) {
  byte S[256]; // S-box
  uint i;
  byte j; // j is always & 0xff, so byte type is appropriate
  byte temp_byte;

  // Initialize S-box with values 255 down to 0
  byte cVar2_init = 255;
  for (i = 0; i < 256; i++) {
      S[i] = cVar2_init--;
  }

  // Key-scheduling algorithm (KSA) - uses param_1 as key material
  j = 0;
  for (i = 0; i < 256; i++) {
    j = (param_1[i % param_3] + S[i] + j) & 0xff;

    // Swap S[i] and S[j]
    temp_byte = S[i];
    S[i] = S[j];
    S[j] = temp_byte;
  }

  // Pseudo-random generation algorithm (PRGA) and XORing with input data
  j = 0; // Index j for PRGA
  byte k = 0; // Index k for PRGA (byte because it's always & 0xff)
  for (i = 0; i < param_3; i++) { // Loop param_3 times for each byte of input data
    k = (k + 1) & 0xff;
    j = (j + S[k]) & 0xff;

    // Swap S[k] and S[j]
    temp_byte = S[k];
    S[k] = S[j];
    S[j] = temp_byte;

    // Generate keystream byte
    byte keystream_byte = S[(S[k] + S[j]) & 0xff];
    byte input_data_byte = param_1[i]; // Get byte from input data stream

    // XOR keystream and input data byte into the output buffer at index (k & 0xf)
    // This effectively produces a 16-byte rolling signature/hash.
    param_2[k & 0xf] = param_2[k & 0xf] ^ keystream_byte ^ input_data_byte;
  }
}

// Struct to represent a segment in a linked list
// Based on how param_1 is accessed in run_seg, load_seg, validate_seg, recv_seg, main.
typedef struct Segment {
    struct Segment *next;   // param_1[0] -> next segment
    void *data;             // param_1[1] -> pointer to segment's actual data
    void *code_ptr;         // param_1[2] -> function pointer or allocated memory address for code
} Segment;

// Function: run_seg
// Executes the code segments in the linked list.
// param_1: Pointer to the head of the Segment linked list.
undefined4 run_seg(Segment *param_1) {
  void *buf1 = NULL;
  void *buf2 = NULL;
  int result_code = 0;

  // Allocate 0x10 bytes for buf1
  buf1 = calloc(1, 0x10);
  if (buf1 == NULL) {
    handle_error_and_terminate(1, "Memory allocation failed for buf1.", EXIT_FAILURE);
  }

  // Allocate 0x70 bytes for buf2
  buf2 = calloc(1, 0x70);
  if (buf2 == NULL) {
    free(buf1); // Clean up buf1 before terminating
    handle_error_and_terminate(1, "Only the sigs deal in absolutes.", EXIT_FAILURE);
  }

  Segment *current_seg = param_1;
  while (current_seg != NULL) {
    // Copy data from segment->data + 4 into buf1
    strcpy((char *)buf1, (char *)current_seg->data + 4);

    // Copy data from segment->data + 0x14 into buf2
    strcpy((char *)buf2, (char *)current_seg->data + 0x14);

    // Send contents of buf1
    if (sendline(1, (char *)buf1, strlen((char *)buf1)) < 0) {
      free(buf1); free(buf2);
      handle_error_and_terminate(1, "Sendline error for buf1.", EXIT_FAILURE);
    }

    // Send contents of buf2
    if (sendline(1, (char *)buf2, strlen((char *)buf2)) < 0) {
      free(buf1); free(buf2);
      handle_error_and_terminate(1, "Sendline error for buf2.", EXIT_FAILURE);
    }

    // Call the function pointer stored in current_seg->code_ptr
    code seg_func = (code)current_seg->code_ptr;
    result_code = ((int (*)())seg_func)(); // Cast and call as a function returning int

    // Send the appropriate response string based on the result_code
    if (sendline(1, responses[result_code - 1], strlen(responses[result_code - 1])) < 0) {
      free(buf1); free(buf2);
      handle_error_and_terminate(1, "Sendline error for response.", EXIT_FAILURE);
    }

    current_seg = current_seg->next; // Move to the next segment
  }

  free(buf1);
  free(buf2);
  return 0; // Success
}

// Function: load_seg
// Allocates executable memory for each segment's code and copies the code into it.
// param_1: Pointer to the head of the Segment linked list.
undefined4 load_seg(Segment *param_1) {
  Segment *current_seg = param_1;
  while (current_seg != NULL) {
    // Allocate 0x1000 bytes of executable memory for code_ptr
    if (allocate(&current_seg->code_ptr, 0x1000, 1) != 0) { // 1 likely refers to protection flags (e.g., PROT_EXEC)
      handle_error_and_terminate(1, "Clever girl.", EXIT_FAILURE);
    }

    // Copy segment data from offset 0x84 into the newly allocated code memory
    memcpy(current_seg->code_ptr, (char *)current_seg->data + 0x84, 0x1000);

    current_seg = current_seg->next; // Move to the next segment
  }
  return 0; // Success
}

// Function: sanitycheck
// Performs a basic check on strings within the segment data.
// param_1: Pointer to the segment's data buffer (current_seg->data).
undefined4 sanitycheck(void *param_1) {
  // Check if strings at offset +4 and +0x14 are non-empty
  size_t len1 = strlen((char *)param_1 + 4);
  size_t len2 = strlen((char *)param_1 + 0x14);

  if ((len1 != 0) && (len2 != 0)) {
    return 0; // Sanity check passed
  }
  return 1; // Sanity check failed
}

// Function: validate_seg
// Validates segments by scrambling data and comparing against trusted signatures,
// and conditionally performing a sanity check.
// param_1: Pointer to the head of the Segment linked list.
int validate_seg(Segment *param_1) {
  char scramble_output[17]; // Buffer for the 16-byte scramble output (+ null terminator)
  int cmp_result;
  int local_18_counter = 0; // Counter replicating the original's local_18 logic

  Segment *current_seg = param_1;
  while (1) { // Replicates the original's 'while(true)' loop
    // This condition means sanitycheck is only performed on the 3rd segment or later
    if (1 < local_18_counter) {
      local_18_counter++; // Increment to 3, 4, ...
      return sanitycheck(current_seg->data); // Return sanitycheck result immediately
    }

    local_18_counter++; // Increment counter (0 -> 1, 1 -> 2)

    memset(scramble_output, 0, sizeof(scramble_output));
    // Scramble the segment's data starting from offset 0x84, outputting to scramble_output
    scramble((byte *)current_seg->data + 0x84, (byte *)scramble_output, 0x1000);

    int found_trusted_match = 0;
    for (int i = 0; i < 6; i++) { // Iterate through the list of trusted signatures
      cmp_result = strncmp(scramble_output, trusted[i], 0x10);
      if (cmp_result == 0) {
        found_trusted_match = 1;
        break;
      }
    }

    if (!found_trusted_match) {
      return cmp_result; // If no trusted match, return the last non-zero comparison result
    }

    current_seg = current_seg->next; // Move to the next segment
    if (current_seg == NULL) {
      return 0; // All segments processed and validated (signature check passed)
    }
  }
  // This part of the code should not be reached.
}

// Function: recv_seg
// Receives segments from a network connection (or stdin) and builds a linked list of Segment structures.
// Returns a pointer to the head of the linked list.
Segment *recv_seg(void) {
  uint num_segments;
  ssize_t bytes_received;
  Segment *head = NULL;
  Segment *tail = NULL;

  // Receive the number of segments (4 bytes)
  bytes_received = recv(0, &num_segments, sizeof(num_segments), 0);
  if (bytes_received < (ssize_t)sizeof(num_segments)) {
    handle_error_and_terminate(1, "Failed to receive segment count.", EXIT_FAILURE);
  }

  // Validate number of segments
  if ((num_segments == 0) || (0x1000 < num_segments)) {
    handle_error_and_terminate(1, "Only the sigs deal in absolutes.", EXIT_FAILURE);
  }

  for (uint i = 0; i < num_segments; i++) {
    // Allocate memory for segment data (0x1000 bytes for code + 0x84 bytes for header/metadata)
    void *segment_data = calloc(1, 0x1084);
    if (segment_data == NULL) {
      handle_error_and_terminate(1, "Memory allocation failed for segment data.", EXIT_FAILURE);
    }

    // Allocate memory for the Segment structure itself (3 pointers)
    Segment *new_seg = (Segment *)calloc(1, sizeof(Segment));
    if (new_seg == NULL) {
      free(segment_data); // Clean up data buffer
      handle_error_and_terminate(1, "Memory allocation failed for segment structure.", EXIT_FAILURE);
    }

    new_seg->data = segment_data; // Link data buffer to the segment structure

    // Receive the segment's data (0x1084 bytes)
    bytes_received = recv(0, new_seg->data, 0x1084, 0);
    if (bytes_received < 0) {
        free(segment_data); free(new_seg);
        handle_error_and_terminate(1, "You didn't say the magic word.", EXIT_FAILURE);
    }
    if (bytes_received < 0x1084) { // Check if the full expected data was received
        free(segment_data); free(new_seg);
        handle_error_and_terminate(1, "Incomplete segment data received.", EXIT_FAILURE);
    }

    // Validate the first 4 bytes of the received data (expected to be 0x1000)
    if (*(uint *)new_seg->data != 0x1000) {
      free(segment_data); free(new_seg);
      handle_error_and_terminate(1, "Only the sigs deal in absolutes.", EXIT_FAILURE);
    }

    // Null terminate specific offsets within the data, likely for string fields
    ((char *)new_seg->data)[0x13] = '\0';
    ((char *)new_seg->data)[0x83] = '\0';

    // Link the new segment into the linked list
    if (head == NULL) {
      head = new_seg;
    } else {
      tail->next = new_seg;
    }
    tail = new_seg;
  }

  if (head == NULL) {
    handle_error_and_terminate(1, "Only the sigs deal in absolutes.", EXIT_FAILURE);
  }
  return head; // Return pointer to the head of the linked list
}

// Function: main
undefined4 main(void) {
  Segment *segments_head = NULL;
  Segment *current_seg = NULL;
  Segment *prev_seg = NULL;
  undefined4 return_code = 0; // Stores the final program exit code

  // Calculate an initial checksum and send it.
  // This replicates the `local_24` calculation and `sendall()` call from the original.
  uint initial_checksum = DAT_4347c00c ^ DAT_4347c000 ^ DAT_4347c004 ^ DAT_4347c008;
  if (sendline(1, (char *)&initial_checksum, sizeof(initial_checksum)) < 0) {
    handle_error_and_terminate(1, "Initial checksum send failed.", EXIT_FAILURE);
  }

  segments_head = recv_seg(); // Receive all segments

  if (validate_seg(segments_head) != 0) {
    return_code = 1; // Validation failed
  } else {
    if (load_seg(segments_head) != 0) {
      return_code = 2; // Loading failed
    } else {
      run_seg(segments_head); // Run the segments
      // run_seg returns 0 on completion, so no direct error code from it is propagated here.
    }
  }

  // Cleanup: Iterate through the linked list to free all allocated resources
  current_seg = segments_head;
  while (current_seg != NULL) {
    prev_seg = current_seg;      // Store current segment for freeing
    current_seg = current_seg->next; // Move to the next segment before freeing current

    if (prev_seg->data != NULL) {
      free(prev_seg->data); // Free the segment's data buffer
    }
    if (prev_seg->code_ptr != NULL) {
      // Deallocate the executable memory allocated for the segment's code
      if (deallocate(prev_seg->code_ptr, 0x1000) != 0) {
        handle_error_and_terminate(1, "Clever girl.", EXIT_FAILURE);
      }
    }
    free(prev_seg); // Free the Segment structure itself
  }

  // Send a final data block (DAT_00013127)
  if (sendline(1, DAT_00013127, 4) < 0) { // Assuming DAT_00013127 is 4 bytes
    handle_error_and_terminate(1, "Final sendline failed.", EXIT_FAILURE);
  }

  return return_code; // Return the determined exit code
}