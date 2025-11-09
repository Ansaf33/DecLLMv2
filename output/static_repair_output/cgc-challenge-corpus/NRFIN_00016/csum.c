#include <stdint.h>   // For uint32_t, uint64_t, uint8_t
#include <stdbool.h>  // For bool
#include <math.h>     // For sin, fabs, pow, round
#include <string.h>   // For memset, memcpy
#include <stdio.h>    // For fprintf
#include <stdlib.h>   // For malloc, free, exit

// Custom macros from original snippet
#define CARRY4(a, b) ((uint32_t)(a) > (UINT32_MAX - (uint32_t)(b)))
#define CONCAT44(high, low) (((uint64_t)(high) << 32) | (uint32_t)(low))
#define ROUND(x) round(x) // Assuming ROUND maps to standard round()

// Global variables (from snippet context, declared static to avoid linker issues)
static uint32_t crctable[256];
static uint8_t calcd = 0; // '\0' is 0
static uint32_t md5table[64];
static double DAT_00013010 = 0.0; // Initial value for fpadditive
static const double DAT_MD5_POW_BASE = 2.0; // Likely 2.0 for MD5 T-table generation
static const double DAT_MD5_POW_EXP = 32.0; // Likely 32.0 for MD5 T-table generation

// Function: additive
uint64_t additive(const uint8_t *param_1, uint32_t param_2) {
  uint32_t local_14 = 0xefc0ffee;
  int32_t local_10 = 0xdeadbe;

  if ((param_2 & 7) == 0) {
    for (uint32_t local_8 = 0; local_8 < param_2; local_8 += 8) {
      uint32_t uVar1_val = *(const uint32_t *)(param_1 + local_8);
      bool bVar2 = CARRY4(local_14, uVar1_val);
      local_14 += uVar1_val;
      local_10 += *(const uint32_t *)(param_1 + local_8 + 4) + (uint32_t)bVar2;
    }
  }
  return CONCAT44(local_10, local_14);
}

// Function: addxoradd
uint64_t addxoradd(const uint8_t *param_1, uint32_t param_2) {
  uint32_t local_14 = 0x57c0ffee;
  int32_t local_10 = 0x313371;

  if ((param_2 & 7) == 0) {
    for (uint32_t local_8 = 0; local_8 < param_2; local_8 += 8) {
      uint32_t val0 = *(const uint32_t *)(param_1 + local_8);
      uint32_t val1 = *(const uint32_t *)(param_1 + local_8 + 4);

      bool bVar3 = CARRY4(local_14, val0);
      uint32_t uVar1_temp = (local_14 + val0) ^ val0;
      uint32_t uVar2_temp = val0; // Original uVar2 was *(uint *)(local_8 + param_1), which is val0

      local_14 = uVar1_temp + uVar2_temp;
      local_10 = (local_10 + val1 + (uint32_t)bVar3 ^ val1) + val1 + (uint32_t)CARRY4(uVar1_temp, uVar2_temp);
    }
  }
  return CONCAT44(local_10, local_14);
}

// Function: compute_me_crc
void compute_me_crc(void) {
  for (uint32_t local_8 = 0; local_8 < 0x100; local_8++) {
    uint32_t local_10 = local_8;
    for (uint32_t local_c = 0; local_c < 8; local_c++) {
      if ((local_10 & 1) == 0) {
        local_10 >>= 1;
      } else {
        local_10 = (local_10 >> 1) ^ 0xedb88320;
      }
    }
    crctable[local_8] = local_10; // *(uint *)(crctable + local_8 * 4)
  }
  calcd = 1;
}

// Function: updatecrc
uint32_t updatecrc(const uint8_t *param_1, uint32_t param_2, uint32_t param_3) {
  uint32_t local_10 = param_3;
  if (calcd == 0) { // '\0' is 0
    compute_me_crc();
  }
  for (uint32_t local_c = 0; local_c < param_2; local_c++) {
    local_10 = crctable[(*(const uint8_t *)(param_1 + local_c) ^ (local_10 & 0xff))] ^ (local_10 >> 8);
  }
  return local_10;
}

// Function: crc32
uint32_t crc32(const uint8_t *param_1, uint32_t param_2) {
  uint32_t uVar1 = updatecrc(param_1, param_2, 0xaf3f0011);
  return ~uVar1;
}

// Function: fpadditive
long double fpadditive(const uint8_t *param_1, uint32_t param_2) {
  double local_14 = DAT_00013010;
  if ((param_2 & 3) == 0) { // If param_2 is a multiple of 4
    for (uint32_t local_8 = 0; local_8 < param_2; local_8 += 8) { // processes in 8-byte chunks (double)
      local_14 += *(const double *)(param_1 + local_8);
    }
  }
  return (long double)local_14;
}

// Function: F
uint32_t F(uint32_t param_1, uint32_t param_2, uint32_t param_3) {
  return (~param_1 & param_3) | (param_1 & param_2);
}

// Function: G
uint32_t G(uint32_t param_1, uint32_t param_2, uint32_t param_3) {
  return (~param_3 & param_2) | (param_1 & param_3);
}

// Function: H
uint32_t H(uint32_t param_1, uint32_t param_2, uint32_t param_3) {
  return param_1 ^ param_2 ^ param_3;
}

// Function: I
uint32_t I(uint32_t param_1, uint32_t param_2, uint32_t param_3) {
  return (~param_3 | param_1) ^ param_2;
}

// Function: calc_me_md5
void calc_me_md5(void) {
  for (uint32_t local_10 = 0; local_10 < 0x40; local_10++) {
    double d_val = sin((double)(local_10 + 1));
    d_val = fabs(d_val);
    double pow_val = pow(DAT_MD5_POW_BASE, DAT_MD5_POW_EXP); // 2^32
    md5table[local_10] = (uint32_t)ROUND(pow_val * d_val);
  }
}

// Constants for MD5 shifts
static const uint8_t SHIFT_AMOUNTS[4][4] = {
    {7, 12, 17, 22}, // Round 1 shifts
    {5, 9, 14, 20},  // Round 2 shifts
    {4, 11, 16, 23}, // Round 3 shifts
    {6, 10, 15, 21}  // Round 4 shifts
};

// Message block access schedule for MD5 rounds
static const uint8_t M_INDICES[4][16] = {
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}, // Round 1
    {1, 6, 11, 0, 5, 10, 15, 4, 9, 14, 3, 8, 13, 2, 7, 12}, // Round 2
    {5, 8, 11, 14, 1, 4, 7, 10, 13, 0, 3, 6, 9, 12, 15, 2}, // Round 3
    {0, 7, 14, 5, 12, 3, 10, 1, 8, 15, 6, 13, 4, 11, 2, 9}  // Round 4
};

// Function: md5
void md5(const uint8_t *param_1, uint32_t param_2, uint32_t *param_3) {
  uint32_t H_registers[4]; // Corresponds to local_9c
  uint32_t A, B, C, D;     // Corresponds to local_8c
  uint32_t M[16];          // Corresponds to local_7c, current 64-byte message block

  // Initialize H registers (MD5 IV)
  H_registers[0] = 0x67452301;
  H_registers[1] = 0xefcdab89; // -0x10325477
  H_registers[2] = 0x98badcfe; // -0x67452302
  H_registers[3] = 0x10325476;

  // Function pointers for F, G, H, I (corresponds to local_ac)
  uint32_t (*md5_funcs[4])(uint32_t, uint32_t, uint32_t) = {F, G, H, I};

  // Calculate T table (md5table)
  calc_me_md5();

  // MD5 Padding
  uint32_t original_len_bytes = param_2;
  // Calculate padded message size (multiple of 64 bytes)
  // Need space for original data, '1' bit, zeros, and 8 bytes for length
  uint32_t padded_len_bytes = ((original_len_bytes + 8 + 63) / 64) * 64;
  
  uint8_t *message_buffer = (uint8_t *)malloc(padded_len_bytes);
  if (message_buffer == NULL) {
    fprintf(stderr, "Error: Failed to allocate memory for MD5 message buffer.\n");
    exit(1);
  }

  memset(message_buffer, 0, padded_len_bytes);
  memcpy(message_buffer, param_1, original_len_bytes);

  // Append '1' bit (0x80 byte)
  message_buffer[original_len_bytes] = 0x80;

  // Append original length in bits (little-endian)
  uint64_t bit_len = (uint64_t)original_len_bytes << 3;
  memcpy(message_buffer + padded_len_bytes - 8, &bit_len, 8);

  // Process message in 64-byte chunks
  for (uint32_t block_idx = 0; block_idx < padded_len_bytes / 64; block_idx++) {
    // Copy 64-byte chunk into M (local_7c)
    memcpy(M, message_buffer + block_idx * 64, 64);

    // Initialize A, B, C, D for this block with current H_registers
    A = H_registers[0];
    B = H_registers[1];
    C = H_registers[2];
    D = H_registers[3];

    // Main MD5 loop (4 rounds of 16 operations each)
    for (uint32_t round = 0; round < 4; round++) {
      uint32_t (*func_ptr)(uint32_t, uint32_t, uint32_t) = md5_funcs[round];
      for (uint32_t step = 0; step < 16; step++) {
        uint32_t F_result = func_ptr(B, C, D);
        uint32_t M_k_index = M_INDICES[round][step];
        uint32_t shift_amount = SHIFT_AMOUNTS[round][step % 4]; // Shift amounts repeat every 4 steps in a round

        uint32_t temp = A + F_result + M[M_k_index] + md5table[round * 16 + step];
        
        // Rotate Left (ROL) and add B
        uint32_t new_A_val = B + ((temp << shift_amount) | (temp >> (32 - shift_amount)));

        // Rotate A, B, C, D registers
        A = D;
        D = C;
        C = B;
        B = new_A_val;
      }
    }

    // Add this block's results to H_registers
    H_registers[0] += A;
    H_registers[1] += B;
    H_registers[2] += C;
    H_registers[3] += D;
  }

  // Copy final hash to param_3
  memcpy(param_3, H_registers, 16);
  free(message_buffer);
}