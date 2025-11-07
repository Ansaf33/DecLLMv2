#include <stdlib.h> // For calloc, free
#include <string.h> // For memset
#include <unistd.h> // For ssize_t, common for read/write functions

// Define types based on common decompiler output conventions
typedef unsigned char byte;
typedef unsigned int uint;
typedef unsigned int undefined4;
typedef unsigned char undefined;

// Declare external functions. Their actual implementation is not provided.
// Assumptions about signatures are made based on usage in the main function.
ssize_t read_all(int fd, void *buf, size_t count, unsigned int param_4);
ssize_t write_all(int fd, const void *buf, size_t count);
unsigned int run_command(void *param_1, unsigned int **param_2);

// Global pointer for the fixed memory region used by generate_csum.
// This address is a decompiler artifact. In a real system, it would likely be
// a memory-mapped region or a pointer to a specific global data structure.
// For compilation, declaring it as a const unsigned char* is sufficient.
const unsigned char *GLOBAL_DATA_BASE = (const unsigned char *)0x4347c000;

// Function: generate_csum
byte generate_csum(const byte *data_ptr, uint data_len) {
  byte checksum = 0;
  uint i;

  // First loop: XORing bytes from a fixed memory region
  // The original *(byte *)(local_8 * 4 + 0x4347c000) implies accessing
  // every 4th byte from the GLOBAL_DATA_BASE address.
  for (i = 0; i < 0x400; i++) {
    checksum ^= *(GLOBAL_DATA_BASE + i * 4);
  }

  // Second loop: XORing bytes from the provided data_ptr
  for (i = 0; i < data_len; i++) {
    checksum ^= data_ptr[i];
  }
  return checksum;
}

// Function: main
undefined4 main(void) {
  // The value 0x12ecf is used as a constant parameter to read_all.
  const unsigned int READ_ALL_PARAM = 0x12ecf;

  // Allocate command_buffer. The original calloc(0x10e, in_stack_ffffffc4)
  // and subsequent memset(local_24, 0, 0x10e) suggest 0x10e is the total size.
  // Using unsigned char* for command_buffer for byte-level arithmetic.
  unsigned char *command_buffer = (unsigned char *)calloc(1, 0x10e);
  if (command_buffer == NULL) {
    return 0xffffffff; // Indicate allocation failure
  }

  // Variables for the command response and result.
  unsigned int *response_buffer_ptr; // Pointer to a dynamically allocated response buffer
  unsigned int command_result;        // The return value from run_command

  // Infinite loop as indicated by `do { ... } while( true );`
  do {
    memset(command_buffer, 0, 0x10e);
    read_all(0, command_buffer, 0xd, READ_ALL_PARAM);

    // Read a 4-byte length value starting at offset 9 within command_buffer.
    // The cast `(int)local_24 + 9` implies byte offset.
    unsigned int data_len_from_buffer = *(unsigned int *)(command_buffer + 9);

    // Conditional read based on the length value
    if ((data_len_from_buffer != 0) && (data_len_from_buffer < 0x101)) {
      read_all(0, command_buffer + 0xd, data_len_from_buffer, READ_ALL_PARAM);
    }

    // Execute the command and get its result and a pointer to a response buffer.
    command_result = run_command(command_buffer, &response_buffer_ptr);

    if (response_buffer_ptr == NULL) {
      // If no response buffer is returned, just write the command result.
      write_all(1, &command_result, sizeof(command_result)); // sizeof(undefined4) is 4
    } else {
      // If a response buffer is returned:
      // 1. Store the command result at the beginning of the response buffer.
      *response_buffer_ptr = command_result;

      // 2. Calculate checksum and store it at offset 2 (third unsigned int).
      //    The data for checksum starts at offset 3 (fourth unsigned int)
      //    and its length is at offset 1 (second unsigned int).
      response_buffer_ptr[2] = generate_csum((byte *)(response_buffer_ptr + 3), response_buffer_ptr[1]);

      // 3. Write the response buffer to stdout. The total length is
      //    the data length (response_buffer_ptr[1]) plus 0xc (12 bytes for
      //    the first three undefined4 fields: result, data_len, checksum).
      write_all(1, response_buffer_ptr, response_buffer_ptr[1] + 0xc);

      // 4. Free the dynamically allocated response buffer.
      free(response_buffer_ptr);
    }
  } while(1); // Loop indefinitely

  // The following code is unreachable due to the infinite loop.
  // If there were a break condition, memory would be freed here.
  // free(command_buffer);
  // return 0; // Standard success return
}