#include <stdio.h>    // For printf, fprintf
#include <stdlib.h>   // For calloc, free
#include <string.h>   // For memcpy, memset, strchr
#include <unistd.h>   // For read, write (if using actual FDs)
#include <sys/select.h> // For fd_set, FD_SET, FD_ISSET, select
#include <sys/time.h> // For struct timeval

// Define custom types from the snippet
typedef unsigned int uint;
typedef unsigned char byte;

// Global variables from the snippet
#define BUFFER_SIZE 64 // Common buffer size seen in receive and send_n_bytes
byte txbuf[BUFFER_SIZE];
int txcnt = 0;

byte rxbuf[BUFFER_SIZE];
int rxcnt = 0;
int rxidx = 0;

// Global data from main, replaced with simple variables
// Assuming these are just placeholder values for printf, not actual addresses.
unsigned int DAT_4347c004 = 0x12345678;
unsigned int DAT_4347c008 = 0x87654321;

// Global context for interpreter output and error stream
struct InterpreterContext {
    char output_buffer[BUFFER_SIZE];
    int output_buffer_idx;
    FILE* error_stream;
};
struct InterpreterContext g_ctx = { .output_buffer_idx = 0, .error_stream = NULL };

// Mock functions for compilation - replace with actual implementations as needed
// int transmit(int fd, const byte* buf, int count, int* bytes_written_ptr);
// Returns 0 on success, non-zero on error.
int transmit(int fd, const byte* buf, int count, int* bytes_written_ptr) {
    if (fd == 1) { // Assuming fd 1 is stdout or a transmit channel
        // For demonstration, print to stderr and report all bytes written
        // fwrite(buf, 1, count, stderr); // Uncomment for debugging output
        *bytes_written_ptr = count;
        return 0; // Success
    }
    *bytes_written_ptr = 0;
    return -1; // Error
}

// int receive(int fd, byte* buf, int max_count, int* bytes_read_ptr);
// Returns 0 on success, non-zero on error.
int receive(int fd, byte* buf, int max_count, int* bytes_read_ptr) {
    if (fd == 0) { // Assuming fd 0 is stdin or a receive channel
        // Simulate reading a single character 'A' for testing, or actual stdin
        // *bytes_read_ptr = read(fd, buf, max_count); // Use this for actual stdin
        if (max_count > 0) {
            buf[0] = 'A'; // Example byte
            *bytes_read_ptr = 1;
            return 0;
        }
        *bytes_read_ptr = 0;
        return 0; // No data, not an error
    }
    *bytes_read_ptr = 0;
    return -1; // Error
}

// Mock fdwait function acting as a wrapper for select.
// It simulates immediate readiness for fd 0 (stdin) if it's in read_fds.
int mock_fdwait(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout) {
    if (readfds != NULL && FD_ISSET(0, readfds)) {
        return 1; // Simulate stdin being ready
    }
    return 0; // No descriptor ready (or not monitoring stdin)
}

// Function: send_flush
void send_flush(void) {
  int bytes_transmitted;
  int current_pos = 0;
  
  while (txcnt != current_pos &&
         (transmit(1, txbuf + current_pos, txcnt - current_pos, &bytes_transmitted) == 0)) {
    current_pos += bytes_transmitted;
  }
  txcnt = 0;
}

// Function: get_byte
uint get_byte(void) {
  int bytes_read;
  
  if (rxcnt == rxidx) {
    if (receive(0, rxbuf, BUFFER_SIZE, &bytes_read) != 0 || bytes_read == 0) {
      return 0xffffffff; // Error or no data
    }
    rxcnt = bytes_read;
    rxidx = 0;
  }
  return (uint)rxbuf[rxidx++];
}

// Function: send_n_bytes
int send_n_bytes(int fd, const char* data, int num_bytes) {
  int bytes_sent;
  int total_sent = 0;
  
  while (num_bytes > 0) {
    int chunk_size = num_bytes;
    if (BUFFER_SIZE < chunk_size) {
      chunk_size = BUFFER_SIZE;
    }
    
    if (transmit(fd, (const byte*)(data + total_sent), chunk_size, &bytes_sent) != 0) {
      return -1; // Error
    }
    num_bytes -= bytes_sent;
    total_sent += bytes_sent;
  }
  return 0; // Success
}

// Function: read_until_or_timeout
uint read_until_or_timeout(int fd_to_wait_on, char* buffer, uint max_len, char delimiter, struct timeval* timeout, int* timeout_flag) {
  uint current_len = 0;
  *timeout_flag = 0;
  
  fd_set read_fds;
  int select_ret_val;
  
  do {
    if (max_len <= current_len) {
      return current_len; // Buffer full
    }
    
    FD_ZERO(&read_fds);
    FD_SET(fd_to_wait_on, &read_fds);
    
    select_ret_val = mock_fdwait(fd_to_wait_on + 1, &read_fds, NULL, NULL, timeout);
    
    if (select_ret_val == -1) { // Error in select
      *timeout_flag = 1;
      return current_len; // Return bytes read so far on error
    }
    if (select_ret_val == 0) { // Timeout
      return current_len;
    }
    
    // Data is available on fd_to_wait_on (stdin)
    do {
      int received_byte = get_byte();
      if (received_byte == -1) { // Error or EOF from get_byte
        return current_len; // Return what was read so far
      }
      
      if (current_len < max_len) { // Ensure buffer bounds
          buffer[current_len++] = (char)received_byte;
          buffer[current_len] = '\0'; // Null-terminate the string
      } else {
          // Buffer full, discard byte and return current_len
          return current_len;
      }
      
      char* found_char = strchr(buffer, delimiter);
      if (found_char != NULL) {
        // Return the length up to and including the delimiter.
        // The original `send_n_bytes` call after finding the delimiter was problematic and removed.
        return (uint)((found_char - buffer) + 1);
      }
      
      if (current_len >= max_len) {
          return max_len;
      }

    } while (rxcnt != rxidx); // Continue reading if more bytes are in rxbuf
  } while (1); // Loop until max_len, timeout, or delimiter found
}

// Function: strip_program
int strip_program(char* program_buffer, uint buffer_size) {
  char* temp_buffer = (char*)calloc(1, buffer_size);
  if (temp_buffer == NULL) {
      return 0; // Allocation error
  }
  
  memcpy(temp_buffer, program_buffer, buffer_size);
  memset(program_buffer, 0, buffer_size); // Clear original buffer
  
  char* write_ptr = program_buffer;
  for (uint loop_idx = 0; loop_idx < buffer_size; ++loop_idx) {
    char c = temp_buffer[loop_idx];
    if (c == '<' || c == '>' || c == '+' || c == '-' ||
        c == '.' || c == ',' || c == '[' || c == ']') {
      *write_ptr++ = c;
    }
  }
  
  free(temp_buffer);
  return (int)(write_ptr - program_buffer); // Return new length
}

// Function: execute_program
int execute_program(char* program, int program_len) {
  #define DATA_TAPE_SIZE 32768
  char data_tape[DATA_TAPE_SIZE];

  char* program_ptr = program;
  char* data_ptr = data_tape + (DATA_TAPE_SIZE / 2); // Start data pointer in the middle
  
  int bracket_depth = 0;
  int direction = 1; // 1 for forward, -1 for backward during bracket skipping

  memset(data_tape, 0, DATA_TAPE_SIZE);

  while (program_ptr >= program && program_ptr < program + program_len) {
    char current_char = *program_ptr;

    if (bracket_depth == 0) {
      if (current_char == '>') {
        data_ptr++;
        if (data_ptr >= data_tape + DATA_TAPE_SIZE) {
          return -1; // Data pointer out of bounds
        }
      } else if (current_char == '<') {
        data_ptr--;
        if (data_ptr < data_tape) {
          return -1; // Data pointer out of bounds
        }
      } else if (current_char == '+') {
        (*data_ptr)++;
      } else if (current_char == '-') {
        (*data_ptr)--;
      } else if (current_char == '.') {
        g_ctx.output_buffer[g_ctx.output_buffer_idx++] = *data_ptr;
        if (g_ctx.output_buffer_idx == BUFFER_SIZE) {
          send_flush();
          g_ctx.output_buffer_idx = 0;
        }
      } else if (current_char == ',') {
        send_flush();
        int input_byte = get_byte();
        if (input_byte == -1) {
          if (g_ctx.error_stream) {
              fprintf(g_ctx.error_stream, "Error: Input failed.\n");
          }
          return -1;
        }
        *data_ptr = (char)input_byte;
        if (*data_ptr == '\0') {
          return 0; // Program termination on null input
        }
      } else if (current_char == '[') {
        if (*data_ptr == '\0') {
          bracket_depth = 1;
          direction = 1;
        }
      } else if (current_char == ']') {
        if (*data_ptr != '\0') {
          bracket_depth = 1;
          direction = -1;
        }
      } else {
        return -1; // Unknown character
      }
    } else { // In a bracket skipping state
      if (current_char == '[') {
        bracket_depth += direction;
      } else if (current_char == ']') {
        bracket_depth -= direction;
      }
      
      if (bracket_depth < 0) { // Mismatched brackets
        return -1;
      }
      if (bracket_depth == 0) { // Found matching bracket, stop skipping
        direction = 1; // Reset direction for normal execution
      }
    }
    
    program_ptr += direction;
  }
  
  return 0; // Program finished
}

// Function: main
int main(void) {
  char program_buffer[0x2000]; // 8192 bytes
  int timeout_flag;
  int stripped_len;
  
  g_ctx.error_stream = stderr; // Initialize global error stream

  printf("Welcome to the headscratch interpreter.\nEnter your program to have it executed!\n%u\n",
         DAT_4347c008 ^ DAT_4347c004);
  
  struct timeval timeout_val = { .tv_sec = 3, .tv_usec = 0 };

  // Read program until newline or timeout
  read_until_or_timeout(0, program_buffer, sizeof(program_buffer), '\n', &timeout_val, &timeout_flag);
  
  if (timeout_flag != 0) {
    return -1; // Timeout or error during read
  }
  
  stripped_len = strip_program(program_buffer, sizeof(program_buffer));
  if (stripped_len == 0) {
    return -1; // No program left after stripping
  }
  
  int exec_result = execute_program(program_buffer, stripped_len);
  if (exec_result != 0) {
    return -1; // Execution error
  }
  
  send_flush(); // Flush any remaining output from execute_program
  
  // Append specific bytes to txbuf and flush
  txbuf[txcnt++] = 'C';
  txbuf[txcnt++] = 'G';
  txbuf[txcnt++] = 0xC0;
  txbuf[txcnt++] = 0;
  
  send_flush();
  
  return 0; // Success
}