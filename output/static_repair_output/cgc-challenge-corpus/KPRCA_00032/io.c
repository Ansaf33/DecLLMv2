#include <stdbool.h> // For bool
#include <stdint.h>  // For fixed-width types if needed, otherwise unsigned int/char are fine
#include <stdio.h>   // For printf, fprintf
#include <stdlib.h>  // For exit, malloc, free
#include <string.h>  // For strlen, memcpy, memset

// --- Type Definitions (based on interpretation of original snippet) ---
typedef void *undefined4;       // Assuming undefined4 is a generic pointer to a buffer
typedef unsigned char undefined; // Generic byte type
typedef unsigned char byte;     // Explicit byte type

// --- Mock I/O functions for compilation and testing ---
// These replace external `transmit` and `receive` functions.
// They use an in-memory buffer to simulate I/O operations.
#define MAX_IO_BUFFER_SIZE 1024
static unsigned char io_buffer[MAX_IO_BUFFER_SIZE];
static int io_buffer_write_idx = 0;
static int io_buffer_read_idx = 0;
static int io_buffer_data_size = 0;

// Resets the mock I/O buffer for a new test sequence
void reset_io_buffer() {
  io_buffer_write_idx = 0;
  io_buffer_read_idx = 0;
  io_buffer_data_size = 0;
  memset(io_buffer, 0, MAX_IO_BUFFER_SIZE);
}

// Mock `transmit` function: simulates writing data to the I/O channel
// mode: 1 for write
// buffer: pointer to data to transmit
// size: number of bytes to transmit
// bytes_transferred_out: array to store the actual number of bytes transferred
// Returns 0 on success, non-zero on error.
int transmit(int mode, undefined4 buffer, int size, int *bytes_transferred_out) {
  if (mode != 1) {
    fprintf(stderr, "Mock transmit: Invalid mode %d\n", mode);
    return -1;
  }
  if (io_buffer_write_idx + size > MAX_IO_BUFFER_SIZE) {
    fprintf(stderr, "Mock transmit: Buffer overflow (size %d, current_idx %d)\n", size,
            io_buffer_write_idx);
    return -1;
  }
  memcpy(&io_buffer[io_buffer_write_idx], buffer, size);
  io_buffer_write_idx += size;
  io_buffer_data_size = io_buffer_write_idx; // Update total data size
  if (bytes_transferred_out) {
    bytes_transferred_out[0] = size;
  }
  return 0; // Success
}

// Mock `receive` function: simulates reading data from the I/O channel
// mode: 0 for read
// buffer: pointer to buffer to store received data
// max_size: maximum number of bytes to receive
// bytes_received_out: array to store the actual number of bytes received
// Returns 0 on success, non-zero on error.
int receive(int mode, undefined4 buffer, int max_size, int *bytes_received_out) {
  if (mode != 0) {
    fprintf(stderr, "Mock receive: Invalid mode %d\n", mode);
    return -1;
  }
  int bytes_to_read = max_size;
  if (io_buffer_read_idx + bytes_to_read > io_buffer_data_size) {
    bytes_to_read = io_buffer_data_size - io_buffer_read_idx; // Read only available bytes
  }
  if (bytes_to_read < 0)
    bytes_to_read = 0;

  memcpy(buffer, &io_buffer[io_buffer_read_idx], bytes_to_read);
  io_buffer_read_idx += bytes_to_read;
  if (bytes_received_out) {
    bytes_received_out[0] = bytes_to_read;
  }
  return 0; // Success
}

// Function: write_bytes
void write_bytes(undefined4 buffer, int size) {
  int transferred_count[1]; // Reduced intermediate variable
  if (transmit(1, buffer, size, transferred_count) != 0 || size != transferred_count[0]) {
    exit(1); // Error: subroutine does not return
  }
}

// Function: read_bytes
void read_bytes(undefined4 buffer, int size) {
  int received_count[1]; // Reduced intermediate variable
  if (receive(0, buffer, size, received_count) != 0 || size != received_count[0]) {
    exit(1); // Error: subroutine does not return
  }
}

// Function: write_byte
void write_byte(undefined val) {
  unsigned char buffer[1]; // Reduced intermediate variable
  buffer[0] = val;
  write_bytes(buffer, 1);
}

// Function: read_byte
undefined read_byte(void) {
  unsigned char buffer[1]; // Reduced intermediate variable
  read_bytes(buffer, 1);
  return buffer[0];
}

// Function: write_int
void write_int(int value) { // Changed param_1 type from uint to int to handle negative numbers
  unsigned int abs_val = (value < 0) ? (unsigned int)-value : (unsigned int)value;
  int num_additional_bytes = 0;
  unsigned int current_chunk_val_for_loop = abs_val;

  // This loop determines `num_additional_bytes` (similar to original `local_10`)
  // and sets `current_chunk_val_for_loop` (similar to original `local_14` before `do-while`).
  while (current_chunk_val_for_loop > 0x3f) { // Check if more than 6 bits for the first byte
    num_additional_bytes++;
    current_chunk_val_for_loop >>= 7; // Subsequent shifts are 7 bits
  }

  // Apply sign bit (0x40) to the most significant chunk if the original value was negative.
  if (value < 0) {
    current_chunk_val_for_loop |= 0x40;
  }

  // Loop to write bytes from MSB to LSB
  int loop_counter = num_additional_bytes; // Corresponds to `local_10` in the original `do-while`
  do {
    unsigned char continuation_bit = (loop_counter > 0) ? 0x80 : 0;
    write_byte((unsigned char)current_chunk_val_for_loop | continuation_bit);

    // Calculate the next 7-bit chunk to be written
    if (loop_counter > 0) {
      current_chunk_val_for_loop = (abs_val >> ((loop_counter - 1) * 7)) & 0x7f;
    }
    loop_counter--;
  } while (loop_counter >= 0);
}

// Function: read_int
int read_int(void) { // Changed return type from uint to int
  unsigned char current_byte;
  bool is_negative;
  int result_val; // Reduced intermediate variable

  current_byte = read_byte();
  is_negative = (current_byte & 0x40) != 0; // Extract sign bit
  result_val = current_byte & 0x3f;       // Extract first 6 data bits

  // Loop while the continuation bit (MSB) is set
  while ((current_byte & 0x80) != 0) { // Equivalent to `(char)current_byte < '\0'`
    current_byte = read_byte();
    result_val |= (current_byte & 0x7f) << 7; // Accumulate next 7 data bits
  }

  if (is_negative) {
    result_val = -result_val;
  }
  return result_val;
}

// Function: write_string
void write_string(char *str) {
  size_t len = strlen(str); // Reduced intermediate variable
  write_int(len);           // Write string length as an int
  write_bytes(str, len);    // Write string data
}

// Function: read_string
char *read_string(void) { // Changed return type to char *
  int len = read_int();   // Reduced intermediate variable
  if (len == -1) {        // Original code checks for -1 for null/error
    return NULL;
  }

  char *str_buffer = (char *)malloc(len + 1); // Reduced intermediate variable
  if (str_buffer == NULL) {
    return NULL;
  }

  read_bytes(str_buffer, len);
  str_buffer[len] = '\0'; // Null-terminate the string
  return str_buffer;
}

// --- Main function for testing and demonstration ---
int main() {
  printf("--- Testing write_int and read_int ---\n");

  int test_values[] = {0,        1,        63,       64,       100,      127,
                       128,      16383,    16384,    -1,       -63,      -64,
                       -100,     -127,     -128,     -16383,   -16384,   INT32_MAX,
                       INT32_MIN}; // Using INT32_MAX/MIN for clear range

  for (size_t i = 0; i < sizeof(test_values) / sizeof(test_values[0]); ++i) {
    int original_val = test_values[i];
    reset_io_buffer(); // Clear buffer for each test
    write_int(original_val);
    io_buffer_data_size = io_buffer_write_idx; // Finalize data size for reading
    io_buffer_read_idx = 0;                     // Reset read index

    // Print bytes written
    printf("Value: %d (0x%X), Written bytes (%d): ", original_val, original_val,
           io_buffer_data_size);
    for (int j = 0; j < io_buffer_data_size; ++j) {
      printf("%02X ", io_buffer[j]);
    }
    printf(" -> ");

    int read_val = read_int();
    printf("Read: %d (0x%X)", read_val, read_val);

    if (original_val == read_val) {
      printf(" [OK]\n");
    } else {
      printf(" [FAIL]\n");
    }
  }

  printf("\n--- Testing write_string and read_string ---\n");
  char *test_strings[] = {
      "Hello, world!",
      "",
      "Short",
      "A much longer string to test buffer capacity and multi-byte length encoding if "
      "applicable for write_int. This string is intentionally made long enough to potentially "
      "exceed a single byte for its length when encoded. Let's make sure it's really long, "
      "maybe a few hundred characters. The quick brown fox jumps over the lazy dog. "
      "The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog. "
      "The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog. "
      "The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog. "
      "The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog.",
      NULL // Test for NULL string read (if write_int returns -1)
  };

  for (size_t i = 0; i < sizeof(test_strings) / sizeof(test_strings[0]); ++i) {
    char *original_str = test_strings[i];
    reset_io_buffer(); // Clear buffer for each test

    if (original_str == NULL) {
      // Simulate writing -1 for length to trigger NULL return in read_string
      write_int(-1);
    } else {
      write_string(original_str);
    }

    io_buffer_data_size = io_buffer_write_idx; // Finalize data size for reading
    io_buffer_read_idx = 0;                     // Reset read index

    printf("Original string: \"%s\" (length %zu)\n",
           original_str ? original_str : "NULL (simulated)",
           original_str ? strlen(original_str) : 0);
    printf("Written bytes for string (%d): ", io_buffer_data_size);
    for (int j = 0; j < io_buffer_data_size; ++j) {
      printf("%02X ", io_buffer[j]);
    }
    printf("\n");

    char *read_str = read_string();
    if (read_str == NULL) {
      printf("Read string: [NULL] ");
      if (original_str == NULL) {
        printf("[OK]\n");
      } else {
        printf("[FAIL] (Expected \"%s\")\n", original_str);
      }
    } else {
      printf("Read string: \"%s\"\n", read_str);
      if (original_str && strcmp(original_str, read_str) == 0) {
        printf("Result: [OK]\n");
      } else {
        printf("Result: [FAIL]\n");
      }
      free(read_str);
    }
    printf("\n");
  }

  return 0;
}