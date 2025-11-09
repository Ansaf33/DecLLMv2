#include <unistd.h> // For STDIN_FILENO, STDOUT_FILENO, ssize_t

// Function prototypes for external functions
// Assuming rpncalc_init initializes an 8-byte buffer
void rpncalc_init(char *state_buffer);

// Assuming read_all reads 'count' bytes into 'buf' from 'fd'.
// The 'flags' argument (0x13d0f) is kept as per original.
// Returns the number of bytes read, or -1 on error.
ssize_t read_all(int fd, void *buf, size_t count, unsigned int flags);

// Assuming perform_rpncalc_op takes the state buffer and an integer operation code.
// Returns a 32-bit unsigned integer result.
unsigned int perform_rpncalc_op(char *state_buffer, int op_code);

// Assuming write_all writes 'count' bytes from 'buf' to 'fd'.
// Returns the number of bytes written, or -1 on error.
ssize_t write_all(int fd, const void *buf, size_t count);

// Assuming rpncalc_destroy cleans up the 8-byte state buffer
void rpncalc_destroy(char *state_buffer);

int main(void) {
  ssize_t bytes_read;
  char rpn_state_buffer[8]; // Calculator state buffer
  int input_value;          // Value read from input
  unsigned int operation_result; // Result of RPN operation

  rpncalc_init(rpn_state_buffer);

  while (1) {
    do {
      bytes_read = read_all(STDIN_FILENO, &input_value, sizeof(input_value), 0x13d0f);
    } while (bytes_read != sizeof(input_value));

    if (input_value == -1) {
      break; // Exit loop if termination value is read
    }

    operation_result = perform_rpncalc_op(rpn_state_buffer, input_value);
    write_all(STDOUT_FILENO, &operation_result, sizeof(operation_result));
  }

  rpncalc_destroy(rpn_state_buffer);
  return 0;
}