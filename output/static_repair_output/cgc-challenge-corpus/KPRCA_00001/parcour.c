#include <stddef.h> // For size_t, though not strictly needed here
#include <stdint.h> // For fixed-width integer types if desired, but unsigned char/int are sufficient

// Global variables
unsigned char g_state;
unsigned char DAT_000160a1;
unsigned char DAT_000160a2[256]; // Array size 0x100

// Function: parcour_byte
unsigned char parcour_byte(void) {
  unsigned char bVar2_val = g_state + 1;
  unsigned char bVar3_val = DAT_000160a2[bVar2_val] + DAT_000160a1;
  unsigned char uVar1_val = DAT_000160a2[bVar2_val];

  g_state = bVar2_val;
  DAT_000160a1 = bVar3_val;

  DAT_000160a2[bVar2_val] = DAT_000160a2[bVar3_val];
  DAT_000160a2[bVar3_val] = uVar1_val;

  return DAT_000160a2[DAT_000160a2[bVar3_val] + DAT_000160a2[bVar2_val]];
}

// Function: parcour_init
void parcour_init(const char *param_1, unsigned int param_2) {
  unsigned char uVar1;
  int local_10;
  unsigned int local_c;

  for (local_c = 0; local_c < 256; local_c++) {
    DAT_000160a2[local_c] = (unsigned char)local_c;
  }

  local_10 = 0;
  for (local_c = 0; local_c < 256; local_c++) {
    // Original calculation:
    // local_10 = (int)(((local_10 + 0x100) - (uint)(byte)(&DAT_000160a2)[local_c]) +
    //                 (int)*(char *)(param_1 + local_c % param_2)) % 0x100;
    //
    // Simplified to ensure positive modulo result for (A - B + C) % N:
    local_10 = (local_10 - (int)DAT_000160a2[local_c] + (int)param_1[local_c % param_2]);
    local_10 = (local_10 % 256 + 256) % 256; // Ensure result is in [0, 255]

    uVar1 = DAT_000160a2[local_c];
    DAT_000160a2[local_c] = DAT_000160a2[local_10];
    DAT_000160a2[local_10] = uVar1;
  }

  g_state = 0;
  DAT_000160a1 = 0;
  for (local_c = 0; local_c < 5000; local_c++) {
    parcour_byte();
  }
}