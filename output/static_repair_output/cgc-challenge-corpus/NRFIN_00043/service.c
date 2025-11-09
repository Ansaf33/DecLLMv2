#include <string.h> // For memset
#include <stdlib.h> // For exit (used by _terminate)

// Declare external functions based on analysis of the original snippet.
// These functions are assumed to be defined elsewhere and linked during compilation.
// The argument types and order are inferred from the stack manipulation in the original code.
extern int send_bytes(const char *data, int size, int type);
extern int recv_until_delim_n(char *buffer, int max_len, char delimiter, int flags);
extern int process(unsigned int *buffer_main, char *buffer_aux, int param);
extern void _terminate(int error_code);

// Global data inferred from DAT_00014000 and DAT_00014019 in the original snippet.
// These are placeholder values; the actual data would depend on the original program.
const char DAT_00014000[24] = "Initial data for send_bytes";
const char DAT_00014019[25] = "Reply data for send_bytes...";

// Function: main
int main(void) {
  char local_buffer_aux[24];        // Corresponds to local_43c
  unsigned int local_buffer_main[256]; // Corresponds to local_424 (256 * 4 = 1024 bytes)
  int bytes_transferred;            // Used for return values of send_bytes and recv_until_delim_n

  // Initialize local_buffer_main with zeros.
  // Original loop: for (iVar1 = 0x100; iVar1 != 0; iVar1 = iVar1 + -1) { *puVar4 = 0; puVar4 = puVar4 + 1; }
  // This initializes 0x100 (256) 'undefined4' (unsigned int) elements to 0, covering 1024 bytes.
  memset(local_buffer_main, 0, sizeof(local_buffer_main));

  do {
    // First call to send_bytes
    // Inferred arguments: data=&DAT_00014000, size=0x18 (24 bytes), type=1
    bytes_transferred = send_bytes(DAT_00014000, 0x18, 1);
    if (bytes_transferred != 0x18) {
      _terminate(10); // Terminate with error code 10
    }

    // Initialize local_buffer_main with zeros again.
    // Inferred arguments: dest=local_buffer_main, value=0, size=0x400 (1024 bytes)
    memset(local_buffer_main, 0, sizeof(local_buffer_main));

    // Initialize local_buffer_aux with zeros.
    // Inferred arguments: dest=local_buffer_aux, value=0, size=0x18 (24 bytes)
    memset(local_buffer_aux, 0, sizeof(local_buffer_aux));

    // Call recv_until_delim_n
    // Inferred arguments: buffer=local_buffer_main, max_len=0x418 (1048 bytes), delimiter=3, flags=0
    // Note: 0x418 (1048) is larger than sizeof(local_buffer_main) (1024).
    // The called function is expected to handle this, or it implies a larger conceptual buffer.
    bytes_transferred = recv_until_delim_n((char *)local_buffer_main, 0x418, 3, 0);
    if (bytes_transferred < 0) {
      _terminate(0x14); // Terminate with error code 20
    }

    // Call process
    // Inferred arguments: buffer_main=local_buffer_main, buffer_aux=local_buffer_aux, param=3
    bytes_transferred = process(local_buffer_main, local_buffer_aux, 3);
    if (bytes_transferred != 0) {
      _terminate(0xf); // Terminate with error code 15
    }

    // Second call to send_bytes
    // Inferred arguments: data=&DAT_00014019, size=0x19 (25 bytes), type=1
    bytes_transferred = send_bytes(DAT_00014019, 0x19, 1);
    if (bytes_transferred != 0x19) {
      _terminate(10); // Terminate with error code 10
    }

    // Third call to send_bytes
    // Inferred arguments: data=local_buffer_aux, size=0x18 (24 bytes), type=1
    bytes_transferred = send_bytes(local_buffer_aux, 0x18, 1);

  } while (bytes_transferred == 0x18); // Loop continues as long as the last send_bytes transferred 24 bytes

  // Final termination if the loop condition is not met
  _terminate(10); // Terminate with error code 10

  // The program exits via _terminate(), so this return is technically unreachable.
  return 0;
}