#include <stdio.h>    // For puts
#include <stdlib.h>   // For exit, EXIT_FAILURE
#include <stdint.h>   // For uintptr_t, uint8_t, uint32_t

// Function: validatePtr
// Original 'uint' typically maps to 'unsigned int' in C.
int validatePtr(unsigned int param_1) {
  // Check if param_1 falls within either of the valid pointer ranges.
  // If it does not, print an error and terminate the program.
  if (!(((param_1 > 0xbaa8b000) && (param_1 < 0xbaaab000)) ||
        ((param_1 > 0x8048000) && (param_1 < 0x804c000)))) {
    puts("Invalid pointer detected!");
    // _terminate(0xffffffff) is replaced by exit(0xffffffff).
    // 0xffffffff as a signed int is -1, commonly used for error exit codes.
    exit(0xffffffff);
  }
  // If no termination, the pointer is considered valid.
  return 1;
}

// Dummy types for Ghidra's 'undefined' and 'undefined4'.
// These are approximations; 'undefined' often means a byte or void*,
// 'undefined4' means a 4-byte integer.
typedef uint8_t undefined;
typedef uint32_t undefined4;

// Function: validateRet
void validateRet(void) {
  // The original function attempts to inspect the caller's stack frame
  // (e.g., via 'unaff_EBP + 4' to get the return address) and verify
  // specific opcodes preceding the call instruction.
  // This is highly architecture-dependent, non-portable, and unsafe in standard C.
  //
  // This implementation uses GCC's __builtin_return_address(0) extension
  // to retrieve the return address, which is common on Linux.
  // However, dereferencing memory at 'ret_addr - 5' is still inherently
  // unsafe and fragile, as there's no guarantee the memory is readable
  // or contains the expected opcode. This is a best-effort translation
  // of the original assembly-level intent.

  // Get the current function's return address. (GCC extension)
  uintptr_t ret_addr = (uintptr_t)__builtin_return_address(0);

  // Check if the return address is outside the expected code segment range.
  // These addresses (0x8048000-0x804c000) are typical for 32-bit x86 executables.
  if ((ret_addr < 0x8048000) || (ret_addr > 0x804c000)) {
    puts("***** STACK SMASHING DETECTED! *****");
    // _terminate() without argument, typically indicates an error.
    exit(EXIT_FAILURE);
  }

  // Adjust the return address to point 5 bytes before it.
  // This is likely to check the opcode of the 'CALL' instruction that
  // pushed the return address onto the stack (0xE8 for x86).
  // This operation is highly unsafe as it reads arbitrary memory potentially
  // outside the executable code segment or in unmapped regions.
  uint8_t *instruction_ptr = (uint8_t *)(ret_addr - 5);

  // Check if the byte at the adjusted address is the expected 'CALL' opcode (0xE8).
  // This assumes the memory is readable and contains valid instruction data.
  if ((*instruction_ptr & 0xff) != 0xe8) {
    puts("***** STACK SMASHING DETECTED! *****");
    // _terminate() without argument, typically indicates an error.
    exit(EXIT_FAILURE);
  }

  return;
}