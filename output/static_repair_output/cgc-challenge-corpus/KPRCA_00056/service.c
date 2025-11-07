#include <stdio.h>    // For printf, fprintf, fwrite, fflush, stdin, stdout, stderr, FILE
#include <stdlib.h>   // For malloc, free
#include <string.h>   // For memcpy, memset, strchr
#include <unistd.h>   // For select (often indirectly via sys/select.h)
#include <sys/select.h> // For fd_set, FD_ZERO, FD_SET, FD_ISSET, select
#include <sys/time.h> // For struct timeval

// Define standard types for the "undefined" ones
typedef unsigned char byte;
typedef unsigned int uint;
typedef unsigned char undefined;
typedef unsigned int undefined4;

// Global variables for I/O buffers and counters
#define TX_BUFFER_SIZE 64 // Consistent with original 0x40
#define RX_BUFFER_SIZE 64 // Consistent with original 0x40
volatile int txcnt = 0;
volatile byte txbuf[TX_BUFFER_SIZE];
volatile int rxcnt = 0;
volatile int rxidx = 0;
volatile byte rxbuf[RX_BUFFER_SIZE];

// Global error stream, initialized in main
FILE *g_error_stream = NULL;

// Mock function for transmit
// In a real system, this would write to a device or file descriptor.
// Here, we simulate writing to stdout.
int transmit(int fd, const void *buf, size_t count, int *bytes_sent) {
    (void)fd; // Unused parameter, assuming fd 1 for stdout
    size_t written = fwrite(buf, 1, count, stdout);
    *bytes_sent = (int)written;
    return 0; // Assume success
}

// Mock function for receive
// In a real system, this would read from a device or file descriptor.
// Here, we simulate reading from stdin.
int receive(int fd, void *buf, size_t count, int *bytes_received) {
    (void)fd; // Unused parameter, assuming fd 0 for stdin
    if (count == 0) {
        *bytes_received = 0;
        return 0;
    }
    size_t read_count = fread(buf, 1, count, stdin);
    *bytes_received = (int)read_count;
    if (read_count == 0 && feof(stdin)) {
        return -1; // Indicate EOF or error
    }
    return 0; // Assume success
}

// Mock function for fdwait, using select
// param_1 (nfds), param_2 (readfds), param_3 (writefds), param_4 (exceptfds), param_5 (timeout), param_6 (ready_count)
int fdwait(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout, int *ready_count) {
    // select modifies the timeout structure, so we need to copy it if we want to preserve the original
    struct timeval tv_copy = *timeout;
    int ret = select(nfds, readfds, writefds, exceptfds, &tv_copy);
    if (ret > 0) {
        *ready_count = ret; // Number of ready descriptors
        return 0; // Success, with ready descriptors
    } else if (ret == 0) {
        *ready_count = 0; // Timeout
        return 0; // Success, but no descriptors ready
    }
    // ret < 0: Error
    *ready_count = 0;
    return -1; // Error
}

// Function: send_flush
void send_flush(void) {
  int bytes_transmitted_total = 0;
  int bytes_transmitted_current;

  while (txcnt > bytes_transmitted_total) {
    if (transmit(1, (const void*)(txbuf + bytes_transmitted_total), txcnt - bytes_transmitted_total, &bytes_transmitted_current) != 0) {
      break; // Error during transmit
    }
    if (bytes_transmitted_current == 0) {
      // transmit failed to send any bytes, avoid infinite loop
      break;
    }
    bytes_transmitted_total += bytes_transmitted_current;
  }
  txcnt = 0; // Reset buffer index after flushing
}

// Function: get_byte
uint get_byte(void) {
  if (rxcnt == rxidx) { // If internal receive buffer is empty
    int bytes_received;
    // Attempt to receive data into rxbuf
    if (receive(0, (void*)rxbuf, RX_BUFFER_SIZE, &bytes_received) != 0 || bytes_received == 0) {
      return 0xffffffff; // Indicate error or no data
    }
    rxcnt = bytes_received;
    rxidx = 0;
  }
  // Return the next byte from the internal receive buffer
  return (uint)rxbuf[rxidx++];
}

// Function: send_n_bytes
unsigned int send_n_bytes(unsigned int fd, const char *buf, int count_to_send) {
  int bytes_sent_total = 0;
  int bytes_sent_current;
  unsigned int chunk_size_limit;

  while (count_to_send > 0) {
    chunk_size_limit = count_to_send;
    if (TX_BUFFER_SIZE < chunk_size_limit) { // Original code uses 0x40 (64)
      chunk_size_limit = TX_BUFFER_SIZE; // Use defined buffer size
    }
    if (transmit(fd, buf + bytes_sent_total, chunk_size_limit, &bytes_sent_current) != 0) {
      return 0xffffffff; // Error during transmit
    }
    if (bytes_sent_current == 0) {
      // transmit failed to send any bytes, avoid infinite loop
      return 0xffffffff;
    }
    bytes_sent_total += bytes_sent_current;
    count_to_send -= bytes_sent_current;
  }
  return 0; // Success
}

// Function: read_until_or_timeout
uint read_until_or_timeout(int fd, char *buffer, uint max_len, char delimiter, int timeout_sec, int timeout_usec, unsigned int *timeout_flag) {
  uint bytes_read_total = 0;
  *timeout_flag = 0; // Initialize timeout flag

  while (bytes_read_total < max_len) {
    // If internal rx buffer is empty, wait for data on the file descriptor
    if (rxcnt == rxidx) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);

        struct timeval timeout_tv = { .tv_sec = timeout_sec, .tv_usec = timeout_usec };
        int ready_count;

        // fdwait expects nfds to be the highest-numbered file descriptor plus one.
        // Here, fd is 0 (stdin), so nfds should be 1.
        if (fdwait(fd + 1, &readfds, NULL, NULL, &timeout_tv, &ready_count) != 0) {
          *timeout_flag = 1; // System call error
          return 0;
        }

        if (ready_count == 0) { // Timeout occurred
          return bytes_read_total;
        }
    }

    // Read a byte (either from rxbuf or by calling receive if rxbuf was empty)
    int byte_value = get_byte();
    if (byte_value == 0xffffffff) { // Error or EOF from get_byte
      return bytes_read_total; // Return what was read so far
    }

    buffer[bytes_read_total] = (char)byte_value;
    // Null-terminate the string for strchr. Ensure bounds.
    if (bytes_read_total + 1 < max_len) {
        buffer[bytes_read_total + 1] = '\0';
    } else {
        buffer[max_len - 1] = '\0'; // Terminate at max_len if buffer is full
    }

    char *delimiter_ptr = strchr(buffer, delimiter);
    if (delimiter_ptr != NULL) {
      // Delimiter found
      bytes_read_total = delimiter_ptr - buffer; // Adjust total bytes read to the delimiter position
      // Original code had a problematic send_n_bytes call here, which has been removed.
      return bytes_read_total;
    }
    bytes_read_total++;
  }
  return bytes_read_total; // Reached max_len without delimiter
}

// Function: strip_program
// Filters out non-Brainfuck characters from the program.
int strip_program(char *program_buffer, uint buffer_len) {
  // Create a temporary buffer to hold the original program
  char *temp_buffer = (char*)malloc(buffer_len);
  if (temp_buffer == NULL) {
      fprintf(g_error_stream, "Error: Memory allocation failed in strip_program.\n");
      return 0; // Malloc failed
  }
  memcpy(temp_buffer, program_buffer, buffer_len);

  char *write_ptr = program_buffer;
  for (uint i = 0; i < buffer_len; i++) {
    char c = temp_buffer[i];
    if (c == '<' || c == '>' || c == '+' || c == '-' ||
        c == '.' || c == ',' || c == '[' || c == ']') {
      *write_ptr++ = c;
    }
  }
  free(temp_buffer);
  return (int)(write_ptr - program_buffer); // Return the new length
}

// Function: execute_program
// Interprets a Brainfuck-like program.
undefined4 execute_program(const char *program, int program_len) {
  // Brainfuck-like interpreter data tape
#define TAPE_SIZE 32768 // Roughly 0x8000 from original decompiler output
  char tape[TAPE_SIZE];
  char *data_ptr = tape + (TAPE_SIZE / 2); // Start in the middle of the tape

  // Initialize tape cells to 0
  memset(tape, 0, TAPE_SIZE);

  const char *program_ptr = program;
  int loop_depth = 0;
  int loop_direction = 1; // 1 for forward, -1 for backward during loop skipping

  while (program_ptr >= program && program_ptr < program + program_len) {
    if (loop_depth == 0) { // If not currently skipping a loop
      switch (*program_ptr) {
        case '>':
          if (data_ptr >= tape + TAPE_SIZE - 1) { // Check for tape overflow
            fprintf(g_error_stream, "Error: Data pointer moved right past tape end.\n");
            return 0xffffffff;
          }
          data_ptr++;
          break;
        case '<':
          if (data_ptr <= tape) { // Check for tape underflow
            fprintf(g_error_stream, "Error: Data pointer moved left past tape start.\n");
            return 0xffffffff;
          }
          data_ptr--;
          break;
        case '+':
          (*data_ptr)++;
          break;
        case '-':
          (*data_ptr)--;
          break;
        case '.':
          // Put character directly into txbuf. If full, flush and try again.
          if (txcnt < TX_BUFFER_SIZE) {
              txbuf[txcnt++] = *data_ptr;
          } else {
              send_flush(); // Flush txbuf
              if (txcnt < TX_BUFFER_SIZE) { // Should be true now
                  txbuf[txcnt++] = *data_ptr;
              } else {
                  fprintf(g_error_stream, "Error: txbuf full after flush, cannot write '.' output.\n");
                  return 0xffffffff;
              }
          }
          break;
        case ',':
          send_flush(); // Flush any pending output before input
          int input_byte = get_byte();
          if (input_byte == 0xffffffff) {
            fprintf(g_error_stream, "Error: Failed to get input byte.\n");
            return 0xffffffff;
          }
          *data_ptr = (char)input_byte;
          if (*data_ptr == '\0') {
            return 0; // Program terminates on null input
          }
          break;
        case '[':
          if (*data_ptr == '\0') {
            loop_depth = 1;
            loop_direction = 1; // Search forward for matching ']'
          }
          break;
        case ']':
          if (*data_ptr != '\0') {
            loop_depth = 1;
            loop_direction = -1; // Search backward for matching '['
          }
          break;
        default:
          // This case should ideally not be reached if strip_program works correctly
          fprintf(g_error_stream, "Error: Invalid character in program: %c\n", *program_ptr);
          return 0xffffffff;
      }
    } else { // Currently skipping a loop (finding matching bracket)
      if (*program_ptr == '[') {
        loop_depth += loop_direction;
      } else if (*program_ptr == ']') {
        loop_depth -= loop_direction;
      }

      if (loop_depth < 0) { // Found a ']' without a preceding '['
        fprintf(g_error_stream, "Error: Unmatched ']' bracket.\n");
        return 0xffffffff;
      }
      if (loop_depth == 0) { // Found the matching bracket, exit skip mode
        loop_direction = 1; // Reset direction to forward
      }
    }
    program_ptr += loop_direction; // Move program pointer
  }

  if (loop_depth != 0) { // End of program with unmatched '['
      fprintf(g_error_stream, "Error: Unmatched '[' bracket.\n");
      return 0xffffffff;
  }

  return 0; // Success
}

// Function: main
undefined4 main(void) {
  g_error_stream = stderr; // Initialize the global error stream to stderr

  char program_buffer[8192]; // Buffer for the user's program (0x2000 bytes)
  unsigned int timeout_occurred;
  int stripped_len;

  // Print welcome message and a magic number (original code used a XOR of two addresses)
  // Replaced with 0 for simplicity.
  printf("Welcome to the headscratch interpreter.\nEnter your program to have it executed!\n%u\n", 0);
  fflush(stdout); // Ensure the prompt is displayed to the user

  // Read program from stdin (fd 0), max 8191 chars + null terminator, delimiter '\n' (10)
  // 3 seconds timeout for input
  uint read_len = read_until_or_timeout(0, program_buffer, sizeof(program_buffer) - 1, '\n', 3, 0, &timeout_occurred);

  if (timeout_occurred != 0) {
    fprintf(g_error_stream, "Error: Timeout while reading program input.\n");
    return 0xffffffff;
  }
  program_buffer[read_len] = '\0'; // Ensure the read program is null-terminated

  stripped_len = strip_program(program_buffer, read_len); // Strip non-BF characters

  if (stripped_len == 0) {
    fprintf(g_error_stream, "Error: No valid program instructions found.\n");
    return 0xffffffff;
  } else {
    int exec_result = execute_program(program_buffer, stripped_len);
    if (exec_result < 0) {
      // Error message would have been printed by execute_program
      return 0xffffffff;
    } else {
      send_flush(); // Flush any remaining output from execute_program

      // Append magic values to txbuf and flush them.
      // Check for buffer space before appending.
      if (txcnt + 4 <= TX_BUFFER_SIZE) {
          txbuf[txcnt++] = 0x43; // 'C'
          txbuf[txcnt++] = 0x47; // 'G'
          txbuf[txcnt++] = 0xc0;
          txbuf[txcnt++] = 0x00;
          send_flush();
      } else {
          fprintf(g_error_stream, "Warning: Not enough space in txbuf for final magic sequence. Flushing existing output.\n");
          // If the magic sequence is critical, more robust error handling or a larger buffer is needed.
          // For now, if no space, existing output is flushed, and magic is skipped.
      }
      return 0; // Success
    }
  }
}