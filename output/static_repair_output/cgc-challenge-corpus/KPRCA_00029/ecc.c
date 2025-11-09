#include <stdio.h>    // For fprintf, stderr
#include <string.h>   // For memset, memcpy
#include <stdbool.h>  // For bool

// Assuming 'byte' is unsigned char for byte-level operations
typedef unsigned char byte;
// 'undefined' is used for generic byte arrays, so unsigned char is appropriate
typedef unsigned char undefined;
// 'undefined4' is a return type, likely an integer. int is standard.
typedef int undefined4;

// Global arrays for Galois Field operations
// field: stores the actual GF elements (alpha^i)
// gf_index: stores the log of GF elements (log_alpha(x) = i)
// Max index for field is 0x7e (126), so size 0x7f (127)
unsigned char field[0x7f];
// Max index for gf_index is determined by values in 'field'.
// Values in 'field' can range from 0 to 255. So size 256.
unsigned char gf_index[256];

// Placeholder for external data referenced by `0x193df`
// This is a heuristic interpretation to make the code compilable on Linux.
// In a real embedded system, 0x193df would be a memory address pointing to
// a constant data table. Here, we assume it's effectively an offset of 0
// into a dummy table `ecc_rom_data`.
// The specific values in this array are crucial for ECC correctness.
// For demonstration, it's filled with dummy values.
static const unsigned char ecc_rom_data[0x7f] = {
    // Dummy values (0 to 126) for compilation. Replace with actual GF polynomial data.
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126
};

// Helper functions for Galois Field arithmetic
// GF multiplication: c = a * b
// Handles cases where a or b is zero.
static unsigned char gf_mul(unsigned char a, unsigned char b) {
    if (a == 0 || b == 0) return 0;
    return field[(gf_index[a] + gf_index[b]) % 0x7f];
}

// GF inverse: b = 1 / a
// Returns 0 if a is 0 (division by zero is undefined).
static unsigned char gf_inv(unsigned char a) {
    if (a == 0) return 0; // Error: GF inverse of 0 is undefined
    return field[(0x7f - gf_index[a]) % 0x7f];
}

// Function: ecc_init
void ecc_init(void) {
  int c; // Renamed from local_c

  for (c = 0; c < 7; c++) {
    field[c] = (unsigned char)(1 << (c & 0x1f));
  }
  field[7] = 9;
  for (c = 8; c < 0x7f; c++) {
    // Original: `*(char *)(local_c + 0x193df)`. Interpreted as `ecc_rom_data[c]`.
    unsigned char val = ecc_rom_data[c] * 2;
    field[c] = val;
    // Check if MSB is set (value > 127) by casting to signed char
    if ((char)val < '\0') {
      field[c] = field[c] ^ field[7] ^ 0x80;
    }
  }
  for (c = 0; c < 0x7f; c++) {
    gf_index[field[c]] = (unsigned char)c;
  }
}

// Function: ecc_encode
undefined4 ecc_encode(undefined *data) { // Renamed param_1 to data
  unsigned char v1, v2; // Renamed cVar1, cVar2
  int i, j; // Renamed local_10, local_c

  for (j = 0; j < 0x1c; j++) { // 0x1c = 28
    data[j] = 0;
  }
  for (j = 0x7e; j > 0x1b; j--) { // 0x7e = 126, 0x1b = 27
    v1 = data[0x1b]; // data[27]
    v2 = data[j];
    for (i = 0x1b; i > 0; i--) { // i from 27 down to 1
      data[i] = data[i - 1];
    }
    data[0] = 0; // *data = 0;
    if (v2 != v1) {
      for (i = 0; i < 0x1c; i++) { // 0x1c = 28
        data[i] ^= (unsigned char)((0x1c9c26b9 >> (i & 0x1f)) & 1);
      }
    }
  }
  return 0;
}

// Function: ecc_decode
bool ecc_decode(unsigned char *data) { // Renamed param_1 to data
  int error_pos; // Renamed iVar1
  unsigned char temp_poly[5]; // Renamed local_3d (temporary polynomial)
  unsigned char lambda_poly[5]; // Renamed local_38 (error locator polynomial)
  unsigned char sigma_poly[5]; // Renamed local_33 (working polynomial)
  unsigned char syndrome[8]; // Renamed local_2e
  unsigned char check_val; // Renamed local_26
  unsigned char discrepancy; // Renamed local_25
  int corrected_errors; // Renamed local_24
  unsigned int last_discrepancy_val; // Renamed local_20
  int last_discrepancy_len; // Renamed local_1c
  int lambda_deg; // Renamed local_18 (degree of lambda_poly)
  int k; // Renamed local_14
  int i; // Renamed local_10

  for (i = 0; i < 8; i++) {
    syndrome[i] = 0;
  }
  for (i = 0; i < 0x7f; i++) {
    if (data[i] != 0) {
      for (k = 0; k < 8; k++) {
        syndrome[k] ^= field[((k + 1) * i) % 0x7f];
      }
    }
  }

  memset(lambda_poly, 0, 5);
  memset(sigma_poly, 0, 5);
  lambda_poly[0] = 1;
  sigma_poly[0] = 1;
  lambda_deg = 0;
  last_discrepancy_len = 1;
  last_discrepancy_val = 1; // Corresponds to field[0] (alpha^0)

  // Berlekamp-Massey algorithm
  for (i = 0; i < 8; i++) {
    discrepancy = syndrome[i];
    for (k = 1; k <= lambda_deg; k++) {
      discrepancy ^= gf_mul(sigma_poly[k], syndrome[i - k]);
    }

    if (discrepancy != 0) {
      memcpy(temp_poly, sigma_poly, 5);
      for (k = 0; k < 4; k++) { // Loop up to 4 for array size 5
        if (lambda_poly[k] != 0) {
          sigma_poly[k + last_discrepancy_len] ^= gf_mul(discrepancy,
                                                          gf_mul(gf_inv(last_discrepancy_val), lambda_poly[k]));
        }
      }
      // Original condition: SBORROW4(i, lambda_deg * 2) == i + lambda_deg * -2 < 0
      // This simplifies to i < 2 * lambda_deg
      if (i < 2 * lambda_deg) {
        lambda_deg = (i + 1) - lambda_deg;
        last_discrepancy_val = discrepancy;
        last_discrepancy_len = 0;
        memcpy(lambda_poly, temp_poly, 5);
      }
    }
    last_discrepancy_len++;
  }

  corrected_errors = 0;
  for (i = 0; i < 0x7f; i++) {
    check_val = 0;
    for (k = 0; k <= lambda_deg; k++) {
      check_val ^= gf_mul(sigma_poly[k], field[(i * k) % 0x7f]);
    }
    if (check_val == 0) { // Found an error location
      corrected_errors++;
      error_pos = (0x7f - i) % 0x7f;
      data[error_pos] ^= 1;
    }
  }

  // Debugging output for errors
  if (2 < lambda_deg) { // If more than 2 errors were detected by Berlekamp-Massey
    fprintf(stderr, "\nFOUND ERRORS: %d %d\n", corrected_errors, lambda_deg);
  }
  return corrected_errors == lambda_deg;
}