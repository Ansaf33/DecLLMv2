#include <stdio.h>    // For standard I/O operations (though not directly used, good practice)
#include <stdlib.h>   // For exit() and EXIT_FAILURE
#include <unistd.h>   // For write() system call
#include <string.h>   // For memset, if buffer initialization is desired (not strictly required by original logic)

// Decompiler-specific types mapped to standard C types
// Assuming 'undefined4' is int, 'undefined' is char, 'ushort' is unsigned short, 'uint' is unsigned int.
typedef int undefined4;
typedef char undefined;
typedef unsigned short ushort;
typedef unsigned int uint;

// Struct to represent the command header received
typedef struct {
    undefined4 magic;         // Corresponds to local_41c in original snippet
    ushort payload_size;    // Corresponds to local_418 in original snippet
    ushort command_code;    // Corresponds to local_416 in original snippet
} CommandHeader;

// Struct to represent the response sent back
typedef struct {
    undefined4 magic;
    undefined4 response_code;
} ResponseHeader;

// --- External Function Declarations ---
// These functions are assumed to be defined elsewhere and linked during compilation.

// receive_bytes: Reads 'count' bytes into 'buf'. Returns 0 on success, -1 on error.
// The original snippet implies a function that fills specific stack locations.
// For compilable C, we assume a standard signature:
extern int receive_bytes(void* buf, size_t count);

// Command handler functions - signatures inferred from their usage in the original snippet.
// Most take a buffer (char*) and an integer value (from *param_1).
extern undefined4 update_sbi_description(undefined* buffer, undefined4 value);
extern undefined4 update_sbi_points(undefined* buffer, undefined4 value);
extern undefined4 update_sbi_status(undefined* buffer, undefined4 value);
extern undefined4 delete_sprint(undefined* buffer, undefined4 value);
extern undefined4 delete_pbi(undefined* buffer, undefined4 value);
extern undefined4 list_product(undefined* buffer, undefined4 value);
extern undefined4 move_sbi_to_pbl(undefined* buffer, undefined4 value);
extern undefined4 move_pbi_to_sprint(undefined* buffer, undefined4 value);
extern undefined4 create_sprint(undefined* buffer, undefined4 value);
extern undefined4 create_pbi(undefined* buffer, undefined4 value);

// These functions take a buffer (char*) and a pointer to an integer (param_1 itself).
extern undefined4 delete_product(undefined* buffer, undefined4* ptr_value);
extern undefined4 create_product(undefined* buffer, undefined4* ptr_value);

// This function takes only an integer value (from *param_1).
extern undefined4 list_all_products(undefined4 value);

// Function: send_response
// This function sends a fixed magic number followed by a response code.
undefined4 send_response(undefined4 response_code) {
  ResponseHeader response = {0x87654321, response_code};
  write(1, &response, sizeof(ResponseHeader));
  return 0;
}

// Function: receive_commands
undefined4 receive_commands(undefined4 *param_1) {
  CommandHeader header;
  undefined auStack_414[1024]; // Buffer for command payload, 0x400 (1024) bytes maximum

  while (1) { // Infinite loop until a termination command is received
    // 1. Read command header (magic, payload_size, command_code)
    if (receive_bytes(&header, sizeof(CommandHeader)) == -1) {
      exit(EXIT_FAILURE); // Equivalent to original _terminate() on read error
    }

    // 2. Validate magic number
    if (header.magic != 0x12345678) {
      exit(EXIT_FAILURE); // Equivalent to original _terminate() on invalid magic
    }

    // 3. Validate payload size against buffer capacity
    if (header.payload_size > sizeof(auStack_414)) {
      exit(EXIT_FAILURE); // Equivalent to original _terminate() on oversized payload
    }

    // 4. Read payload into the buffer
    if (receive_bytes(auStack_414, header.payload_size) == -1) {
      exit(EXIT_FAILURE); // Equivalent to original _terminate() on payload read error
    }

    // 5. Check for termination command (0xdead)
    if (header.command_code == 0xdead) {
      break; // Exit the command processing loop
    }

    undefined4 command_result = 0; // Variable to store the result of command execution

    // 6. Dispatch commands using a switch statement for clarity and efficiency
    switch (header.command_code) {
        case 0x503: command_result = update_sbi_description(auStack_414, *param_1); break;
        case 0x502: command_result = update_sbi_points(auStack_414, *param_1); break;
        case 0x501: command_result = update_sbi_status(auStack_414, *param_1); break;
        case 0x403: command_result = delete_product(auStack_414, param_1); break; // Passes param_1 (pointer)
        case 0x402: command_result = delete_sprint(auStack_414, *param_1); break;
        case 0x401: command_result = delete_pbi(auStack_414, *param_1); break;
        case 0x304: command_result = list_all_products(*param_1); break; // No buffer argument
        case 0x301: command_result = list_product(auStack_414, *param_1); break;
        case 0x202: command_result = move_sbi_to_pbl(auStack_414, *param_1); break;
        case 0x201: command_result = move_pbi_to_sprint(auStack_414, *param_1); break;
        case 0x103: command_result = create_sprint(auStack_414, *param_1); break;
        case 0x101: command_result = create_product(auStack_414, param_1); break; // Passes param_1 (pointer)
        case 0x102: command_result = create_pbi(auStack_414, *param_1); break;
        default:
            // Handle unknown commands. For robustness, an error response could be sent.
            // For now, it defaults to command_result = 0 (initialized value).
            break;
    }
    // Send the response back after processing the command
    send_response(command_result);
  }
  return 0; // Return 0 on successful termination of the command loop
}