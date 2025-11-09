#include <stdio.h>    // For fprintf, stdout, stderr
#include <stdlib.h>   // For exit
#include <unistd.h>   // For potential system calls like close (if client_socket_fd were real)
#include <sys/socket.h> // For send (if it's a real socket function)
#include <string.h>   // For memcpy, if needed by low-level functions

// Decompiler often uses 'undefined' types. Using standard C types.
typedef unsigned int uint;

// --- Function Declarations (placeholders) ---
// These functions are assumed to be defined elsewhere in the actual program.
// Their signatures are inferred from the usage in the main function.

// recv_all is assumed to read 4 bytes (sizeof(uint)) into the provided pointer.
// It returns the number of bytes read, or -1 on error.
int recv_all(uint *buffer_ptr) {
    // Dummy implementation: In a real system, this would read from a socket.
    // Example: `return recv(global_socket_fd, buffer_ptr, sizeof(uint), 0);`
    static uint test_commands[] = {
        0x127690, // get_haiku_count
        1999,     // get_haiku_by_id
        0x5d4,    // add_haiku
        0x30d6a,  // get_haiku_random
        0x845fed, // get_haiku_ids
        0xBADBEEF // Unknown command, will trigger default case and exit
    };
    static int cmd_idx = 0;

    if (cmd_idx < sizeof(test_commands) / sizeof(test_commands[0])) {
        *buffer_ptr = test_commands[cmd_idx++];
        fprintf(stderr, "recv_all: Received command 0x%x\n", *buffer_ptr);
        return sizeof(uint); // Simulate successful reception of 4 bytes
    }
    return 0; // Simulate no more data to process, or connection closed
}

// _terminate is assumed to exit the program due to a fatal error.
void _terminate(void) {
    fprintf(stderr, "FATAL ERROR: Program termination requested.\n");
    exit(1);
}

// Placeholder functions for various operations, returning an int result.
int get_haiku_ids(void) {
    fprintf(stdout, "Action: get_haiku_ids\n");
    return 100; // Example success code
}

int get_haiku_count(void) {
    fprintf(stdout, "Action: get_haiku_count\n");
    return 5; // Example success code
}

int get_haiku_random(void) {
    fprintf(stdout, "Action: get_haiku_random\n");
    return 1; // Example success code
}

int add_haiku(void) {
    fprintf(stdout, "Action: add_haiku\n");
    return 0; // Example success code
}

int get_haiku_by_id(void) {
    fprintf(stdout, "Action: get_haiku_by_id\n");
    return 200; // Example success code
}

// Global or context variable for the socket file descriptor.
// In a real application, this would be properly managed (e.g., passed as an argument).
// Using stdout (FD 1) for demonstration purposes.
int client_socket_fd = 1;

// This `send_response` function is a wrapper for a socket `send`.
// It sends an integer result back to the client.
int send_response(int fd, const void *buf, size_t len, int flags) {
    // Dummy implementation: print the integer value being "sent"
    if (len == sizeof(int)) {
        fprintf(stderr, "Sending response to FD %d: %d (len %zu)\n", fd, *(const int*)buf, len);
    } else {
        fprintf(stderr, "Sending %zu bytes to FD %d.\n", len, fd);
    }
    // In a real application, this would be:
    // return send(fd, buf, len, flags);
    return (int)len; // Simulate successful send
}

// Function: main
int main(void) {
  int bytes_received;
  int result;       // Corresponds to local_18 in the original snippet
  uint command_id;  // Corresponds to local_14 in the original snippet

  // The original code contained decompiler artifacts like
  // `undefined *puVar2;`, `uint *apuStack_34 [5];`, `undefined auStack_20 [8];`,
  // `undefined *local_10;`, and stack manipulations like `*(undefined4 *)(puVar2 + -0xc) = 4;`.
  // These are removed as they represent low-level stack frame management, not
  // the logical intent of the C code.

  // The `do-while(true)` loop indicates a continuous processing loop, typical for servers.
  do {
    command_id = 0; // Reset command_id for each iteration

    // Attempt to receive a command ID. Assume 4 bytes (sizeof(uint)) are expected.
    bytes_received = recv_all(&command_id);

    // If recv_all didn't return the expected number of bytes, it's an error.
    if (bytes_received != sizeof(uint)) {
      fprintf(stderr, "Error: Expected %zu bytes for command ID, received %d.\n", sizeof(uint), bytes_received);
      _terminate(); // Terminate the program on critical reception error
    }

    // Use a switch statement to handle different command IDs.
    // This replaces the complex nested if-else structure and `goto LAB_00011d17;`.
    switch (command_id) {
      case 0x845fed: // Decimal 8675309
        result = get_haiku_ids();
        break;
      case 0x127690: // Decimal 1209990
        result = get_haiku_count();
        break;
      case 0x30d6a:  // Decimal 200042
        result = get_haiku_random();
        break;
      case 0x5d4:    // Decimal 1492
        result = add_haiku();
        break;
      case 1999:
        result = get_haiku_by_id();
        break;
      default:
        fprintf(stderr, "Unknown command ID received: 0x%x\n", command_id);
        result = -999; // Default error code, matching the LAB_00011d17 path
        break;
    }

    // If any operation resulted in an error (negative return value),
    // send the error code back and terminate the program.
    if (result < 0) {
      // The original `send` call's arguments were highly obfuscated by the decompiler.
      // This interpretation assumes it sends the `result` integer as a 4-byte response.
      send_response(client_socket_fd, &result, sizeof(result), 0);
      return result; // Exit main with the error code
    }
    // If `result` is non-negative, the loop continues to process the next command.
  } while(1); // The `while( true )` implies an infinite loop for continuous operation.
}