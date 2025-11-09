#include <string.h>  // For memcpy, memset
#include <stdarg.h>  // For dprintf (though mock doesn't use it)
#include <unistd.h>  // For read, assuming receive is similar to read
#include <stdlib.h>  // For exit, assuming _terminate is similar to exit
#include <stdbool.h> // For bool type

// Type definitions commonly found in decompiled code
typedef unsigned char byte;
typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned char undefined;
typedef unsigned short undefined2;
typedef unsigned int undefined4;

// Mock dprintf function
int dprintf(int __fd, char *__fmt, ...) {
  // Original implementation just returns a constant. Keep it as is.
  return 0x16ff4;
}

// Stubs for functions called by execute
// These functions are assumed to operate on the provided context pointer and instruction.
void doQATB(unsigned char* param_1, undefined4 instruction) {}
void doAdd(unsigned char* param_1, undefined4 instruction) {}
void doAdc(unsigned char* param_1, undefined4 instruction) {}
void doSub(unsigned char* param_1, undefined4 instruction) {}
void doSuc(unsigned char* param_1, undefined4 instruction) {}
void doLsl(unsigned char* param_1, undefined4 instruction) {}
void doLsr(unsigned char* param_1, undefined4 instruction) {}
void doRsb(unsigned char* param_1, undefined4 instruction) {}
void doRsc(unsigned char* param_1, undefined4 instruction) {}
void doAnd(unsigned char* param_1, undefined4 instruction) {}
void doOr(unsigned char* param_1, undefined4 instruction) {}
void doXor(unsigned char* param_1, undefined4 instruction) {}
void doNot(unsigned char* param_1, undefined4 instruction) {}
void doMin(unsigned char* param_1, undefined4 instruction) {}
void doMax(unsigned char* param_1, undefined4 instruction) {}
void doClr(unsigned char* param_1, undefined4 instruction) {}
void doSet(unsigned char* param_1, undefined4 instruction) {}
void doBranch(unsigned char* param_1, undefined4 instruction) {}
void doLdi(unsigned char* param_1, undefined4 instruction) {}
void doLmbd(unsigned char* param_1, undefined4 instruction) {}
void doScan(unsigned char* param_1, undefined4 instruction) {}

// Global index for mock receive to simulate byte-by-byte reads
static int _global_recv_idx = 0;
// Example integer value for the mock receive function
static unsigned int _mock_recv_int_val = 0x12345678;

// Mock receive function
// This mock simulates reading 1 byte at a time to match the original loop's behavior
// of accumulating bytes.
int receive(int fd, void *buf, size_t count) {
    if (count == 0) return 0;
    // Simulate reading one byte from a mock integer value
    *(unsigned char*)buf = (_mock_recv_int_val >> (3 - _global_recv_idx) * 8) & 0xFF;
    _global_recv_idx++;
    if (_global_recv_idx >= 4) {
        _global_recv_idx = 0;    // Reset for the next full integer read
        _mock_recv_int_val++; // Change mock data for the next integer
    }
    return 1; // Always reads one byte
}

// Mock _terminate function (likely equivalent to exit)
void _terminate(void) {
  exit(1);
}

// Mock dumpState function
void dumpState(unsigned char* param_1) {
    // Placeholder for actual state dumping logic
    dprintf(1, "dumpState called with context pointer: %p\n", param_1);
}

// Function: execute
void execute(unsigned char* param_1) {
  undefined4 instruction;
  byte bVar1;
  bool branched_this_cycle;
  
  do {
    branched_this_cycle = false; // Reset branch flag for each instruction cycle

    // Read the instruction directly from memory using pointer arithmetic.
    // This reduces the need for the intermediate `local_14` and `memcpy`.
    instruction = *(undefined4*)(param_1 + (*(int*)(param_1 + 0x80) + 0x20) * 4 + 4);
    
    bVar1 = (byte)instruction & 7;

    if (bVar1 == 3) {
      doQATB(param_1, instruction); // Use instruction directly
      return;
    }
    if (3 < bVar1) { // If bVar1 is 4, 5, 6, or 7
      return;
    }
    
    // At this point, bVar1 can only be 0, 1, or 2 (from 0 to 7, excluding 3, and >3 cases).
    if (bVar1 == 0) {
      switch((byte)instruction >> 3 & 0xf) {
      default: doAdd(param_1, instruction); break;
      case 1: doAdc(param_1, instruction); break;
      case 2: doSub(param_1, instruction); break;
      case 3: doSuc(param_1, instruction); break;
      case 4: doLsl(param_1, instruction); break;
      case 5: doLsr(param_1, instruction); break;
      case 6: doRsb(param_1, instruction); break;
      case 7: doRsc(param_1, instruction); break;
      case 8: doAnd(param_1, instruction); break;
      case 9: doOr(param_1, instruction); break;
      case 10: doXor(param_1, instruction); break;
      case 0xb: doNot(param_1, instruction); break;
      case 0xc: doMin(param_1, instruction); break;
      case 0xd: doMax(param_1, instruction); break;
      case 0xe: doClr(param_1, instruction); break;
      case 0xf: doSet(param_1, instruction); break;
      }
    } else { // bVar1 must be 1 or 2
      if (bVar1 != 1) { // If bVar1 is 2, return (this covers the bVar1==2 case).
        return;
      }
      // If execution reaches here, bVar1 must be 1.
      switch(((byte)instruction) >> 3 & 0xf) { // Use instruction directly, reducing `local_15`
      default:
        doBranch(param_1, instruction); // Use instruction directly, reducing `local_1d`
        branched_this_cycle = true; // Mark that a branch occurred
        break;
      case 2: doLdi(param_1, instruction); break; // Use instruction directly, reducing `local_21`
      case 3: doLmbd(param_1, instruction); break; // Use instruction directly, reducing `local_25`
      case 4: doScan(param_1, instruction); break; // Use instruction directly, reducing `local_29`
      case 5: case 6: case 7: case 8: case 9: case 10: case 0xb: case 0xc: case 0xd: case 0xe: case 0xf:
        return; // Return for these specific cases
      }
    }

    if (!branched_this_cycle) { // Increment program counter only if no branch occurred
      *(int *)(param_1 + 0x80) = *(int *)(param_1 + 0x80) + 1;
    }
    // Increment a cycle counter
    *(ushort *)(param_1 + 0x4086) = *(ushort *)(param_1 + 0x4086) + 1;
  } while (*(ushort *)(param_1 + 0x4086) < 2000); // Loop until cycle counter reaches 2000
  return;
}

// Function: recvInt
undefined4 recvInt(void) {
  undefined local_14[4]; // Buffer to store the 4 bytes of the integer
  uint bytes_read_total = 0;
  int bytes_received_this_call;
  
  // Loop to read exactly 4 bytes into local_14
  while (bytes_read_total < 4) {
    // Call the mock receive function.
    // Assuming receive(fd, buffer_ptr, bytes_to_read) signature.
    // fd 0 is used in the original decompiled code.
    bytes_received_this_call = receive(0, local_14 + bytes_read_total, 4 - bytes_read_total);
    
    if (bytes_received_this_call == 0) {
      _terminate(); // If receive returns 0 bytes, terminate
    }
    bytes_read_total += bytes_received_this_call;
  }
  
  // Interpret the 4 bytes as an undefined4 (unsigned int) and return
  // This effectively converts the byte array to an integer, respecting native endianness.
  return *(undefined4*)local_14;
}

// Function: main
void main(void) {
  // Declare a large buffer to serve as the execution context.
  // The largest offset accessed in `execute` is 0x4086, which is read as a `ushort` (2 bytes).
  // Therefore, the buffer needs to be at least 0x4086 + 2 = 0x4088 bytes.
  // The `memset` operation in the original code covers 0x4000 bytes.
  // We'll allocate 0x4100 bytes to be safe and cover all observed offsets.
  unsigned char context_buffer[0x4100];
  
  undefined4 received_value;
  int num_instructions;
  int i;

  // Initialize the entire context_buffer region that was targeted by the original `memset`.
  // The original `memset` target was `auStack_401c` (a 12-byte array) with a size of `0x4000`.
  // This implies `auStack_401c` was likely the base of a larger structure,
  // or the `memset` would have caused a stack overflow.
  // We assume `context_buffer` is the intended large memory region.
  memset(context_buffer, 0xff, 0x4000);

  // Initialize a specific field within the context_buffer.
  // `local_1a` in the original code, `*(ushort *)(param_1 + 0x4086)` in `execute`,
  // is initialized to 0.
  *(ushort *)(context_buffer + 0x4086) = 0;

  // Read the number of instructions or data items to process
  num_instructions = recvInt();

  // Declare a local array for storing received data.
  // The original code used `undefined4 auStack_409c[32];` and indexed it as
  // `auStack_409c[local_14 + 0x20]`. This `+ 0x20` offset would cause an
  // out-of-bounds access if `auStack_409c` is truly a 32-element array.
  // Assuming `auStack_409c` is intended to be a local array and `+ 0x20`
  // was a decompiler artifact or a mistake, we'll index it directly with `i`.
  // A warning is added if `num_instructions` exceeds the array size.
  undefined4 auStack_409c[32]; // Max 32 instructions can be stored here

  for (i = 0; i < num_instructions; i = i + 1) {
    received_value = recvInt();
    if (i < 32) { // Ensure we don't write out of bounds for auStack_409c
        auStack_409c[i] = received_value;
    } else {
        dprintf(2, "Warning: num_instructions (%d) exceeded auStack_409c buffer size (32). Dropping value.\n", num_instructions);
    }
  }

  // Call the execute function, passing the context buffer as its parameter.
  execute(context_buffer);

  // Call dumpState function, passing the context buffer.
  dumpState(context_buffer);

  // The original code had some architecture-specific calls like
  // `*(undefined **)(puVar3 + -0x10a0) = 0;` and
  // `*(code **)(puVar3 + -0x10a4) = __x86_get_pc_thunk_bx;`
  // which are typically related to cleanup or obtaining the program counter.
  // For standard C compilation and termination, these are not necessary.
  _terminate();
}