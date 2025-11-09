#include <stdlib.h> // For calloc, free
#include <string.h> // For memset
#include <unistd.h> // For ssize_t, read, write (assuming read_all/write_all wrap these)
#include <stdint.h> // For standard integer types like uint8_t, uint32_t
#include <stddef.h> // For size_t

// Type definitions based on disassembler output, mapped to standard C types
typedef uint8_t byte;
typedef uint32_t uint;       // Assuming uint is 4 bytes
typedef uint32_t undefined4; // Assuming undefined4 is 4 bytes
typedef uint8_t undefined;   // Assuming undefined is 1 byte

// Forward declarations for external functions.
// Signatures are inferred from usage in the provided snippet.
// Actual implementations of these functions would be linked externally.
ssize_t read_all(int fd, void *buf, size_t count, undefined4 magic_val);
undefined4 run_command(void *param_1, undefined4 **param_2);
ssize_t write_all(int fd, const void *buf, size_t count);

// Function: generate_csum
byte generate_csum(const byte *param_1, uint param_2) {
  byte bVar1 = 0;
  // This static array simulates the global memory region at 0x4347c000
  // accessed in the original snippet. In a real system, this might be
  // a memory-mapped peripheral or a specific data segment.
  static byte s_global_data_region[0x400 * 4];

  // Loop 1: XORing bytes from a simulated global data region
  for (uint local_8 = 0; local_8 < 0x400; ++local_8) {
    bVar1 ^= s_global_data_region[local_8 * 4];
  }

  // Loop 2: XORing bytes from the buffer pointed to by param_1
  for (uint local_8 = 0; local_8 < param_2; ++local_8) {
    bVar1 ^= param_1[local_8];
  }
  return bVar1;
}

// Function: main
undefined4 main(void) {
  // `local_14 = &stack0x00000004;` is a disassembler artifact and not standard C. It is removed.
  undefined4 magic_value = 0x12ecf; // Renamed uVar3 for clarity

  // `in_stack_ffffffc4` was an uninitialized variable used as a size argument to calloc.
  // This is a bug. Given `memset(local_24, 0, 0x10e);`, it's clear 0x10e is the total size.
  // `calloc(nmemb, size)` should be `calloc(1, 0x10e)` to allocate 0x10e bytes.
  void *local_24 = calloc(1, 0x10e);
  if (local_24 == NULL) { // Use NULL instead of (void *)0x0 for C standard compliance
    return 0xffffffff;
  }

  do {
    memset(local_24, 0, 0x10e);
    read_all(0, local_24, 0xd, magic_value);

    // Accessing data at offset 9 within local_24.
    // The original code used `(int)local_24 + 9` which is incorrect pointer arithmetic.
    // It should be `(byte *)local_24 + 9` to ensure correct byte-level offset.
    // Casting to `int *` or `uint *` directly from `void *` might violate strict aliasing rules.
    // We read the 4-byte value at offset 9 as both an int and a uint for the condition.
    int val_at_offset_9 = *(int *)((byte *)local_24 + 9);
    uint uval_at_offset_9 = *(uint *)((byte *)local_24 + 9);

    if ((val_at_offset_9 != 0) && (uval_at_offset_9 < 0x101)) {
      // Again, correct pointer arithmetic: `(byte *)local_24 + 0xd`
      read_all(0, (byte *)local_24 + 0xd, *(undefined4 *)((byte *)local_24 + 9), magic_value);
    }

    undefined4 *local_2c = NULL; // Initialize local_2c to NULL
    undefined4 local_28 = run_command(local_24, &local_2c);

    // Reduced intermediate variable `puVar1` by using `local_2c` directly.
    if (local_2c == NULL) {
      write_all(1, &local_28, sizeof(local_28));
    } else {
      *local_2c = local_28;
      // Reduced intermediate variable `uVar2` by direct assignment.
      // `local_2c + 3` performs pointer arithmetic on `undefined4 *`,
      // moving 3 * sizeof(undefined4) bytes forward, which is then cast to `byte *`
      // as required by `generate_csum`.
      local_2c[2] = generate_csum((const byte *)(local_2c + 3), local_2c[1]);
      write_all(1, local_2c, local_2c[1] + 0xc);
      free(local_2c);
    }
  } while (1); // Replaced `true` with `1` for standard C infinite loop.

  // The `free(local_24)` would typically be here, but it's unreachable due to the infinite loop.
  // This pattern is common in embedded systems or server daemons.
}