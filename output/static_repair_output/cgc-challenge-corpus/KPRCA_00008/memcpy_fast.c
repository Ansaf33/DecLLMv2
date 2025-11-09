#include <string.h> // Required for memcpy
#include <stdint.h> // Required for uint32_t

// Function: memcpy_fast
void * memcpy_fast(void *param_1, void *param_2, unsigned int param_3) {
  // Check for 16-byte alignment of destination, source, and size being a multiple of 16
  // Using unsigned long for pointer address comparison for portability on 32-bit/64-bit systems
  if ((param_3 % 16 == 0) &&
      (((unsigned long)param_1 % 16) == 0) &&
      (((unsigned long)param_2 % 16) == 0)) {

    // Cast void pointers to uint32_t pointers for 4-byte aligned access
    uint32_t *dest_u32 = (uint32_t *)param_1;
    const uint32_t *src_u32 = (const uint32_t *)param_2; // Use const for source data

    // Loop to copy data in 16-byte (4 x uint32_t) blocks
    // param_3 >> 4 is an efficient way to calculate param_3 / 16
    for (unsigned int i = 0; i < (param_3 >> 4); ++i) {
      // Copy 4 uint32_t values (16 bytes) from source to destination
      // This reduces intermediate variables uVar1, uVar2, uVar3
      dest_u32[0] = src_u32[0];
      dest_u32[1] = src_u32[1];
      dest_u32[2] = src_u32[2];
      dest_u32[3] = src_u32[3];

      // Advance both pointers by 4 uint32_t (16 bytes) to the next block
      // This replaces recalculating pointers (puVar4, puVar5) and local_20 in each iteration
      dest_u32 += 4;
      src_u32 += 4;
    }
  } else {
    // If conditions for fast copy are not met, fall back to standard memcpy
    // memcpy returns the destination pointer (param_1)
    return memcpy(param_1, param_2, param_3);
  }

  // Return the original destination pointer (param_1), consistent with memcpy behavior
  return param_1;
}