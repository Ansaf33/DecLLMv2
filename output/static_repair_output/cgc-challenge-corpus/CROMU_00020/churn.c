#include <stdint.h> // For uint32_t, uint64_t, int32_t, uint8_t
#include <math.h>   // For sin, cos, tan, fabs, log2

// Define types based on the snippet
typedef uint32_t undefined4;
typedef uint64_t undefined8;
typedef uint8_t byte;

// CONCAT44 macro definition
// Combines two 32-bit values into a 64-bit value, high part first.
#define CONCAT44(high, low) (((uint64_t)(high) << 32) | (low))

// Global variables
// 'gyre' is an array of 64 64-bit values, sometimes treated as uint64_t, sometimes as double.
uint64_t gyre[64];
uint32_t gyre_pos; // Index into gyre, modulo 64

// From init_two, DAT_00013000 is a double constant.
// Using 1.0 as a placeholder for compilation; the actual value would be from the original binary.
const double DAT_00013000 = 1.0;

// Helper for extracting the lower 32 bits of a double's bit representation.
// This is endianness-dependent (assumes little-endian, e.g., x86_64).
static inline uint32_t get_double_low32(double d) {
    return ((uint32_t*)&d)[0];
}

// Function declarations to handle potential forward-referencing issues
undefined8 init_one(uint32_t param_1, uint32_t param_2, uint32_t param_3, uint32_t param_4);
long double init_two(undefined4 param_1_low, undefined4 param_1_high, undefined4 param_2_low, undefined4 param_2_high);
undefined8 init_thr(uint32_t param_1, uint32_t param_2, uint32_t param_3, uint32_t param_4);
long double init_fou(uint64_t val1, uint64_t val2);
undefined4 * churn_rand_item(undefined4 *param_1);
undefined8 churn_rand_uint64(void);

// Function: init_one
undefined8 init_one(uint32_t param_1, uint32_t param_2, uint32_t param_3, uint32_t param_4) {
  return CONCAT44(param_2 ^ (param_4 >> 0x15) ^ ((param_3 & 0x3234b29) << 5) ^ ((param_4 >> 9) & 0x6372616e) ^ (param_3 << 0x12),
                  param_1 ^ ((param_3 >> 0x15) | (param_4 << 0xb)) ^ (((param_3 >> 9) | (param_4 << 0x17)) & 0x6b74776f));
}

// Function: init_two
long double init_two(undefined4 param_1_low, undefined4 param_1_high, undefined4 param_2_low, undefined4 param_2_high) {
  double d_val1_64 = (double)CONCAT44(param_1_high, param_1_low);
  double d_val2_64 = (double)CONCAT44(param_2_high, param_2_low);

  double fabs_sin_d1 = fabs(sin(d_val1_64));
  double fabs_cos_d2 = fabs(cos(d_val2_64));
  
  double sum_fabs = fabs_cos_d2 + fabs_sin_d1;
  double term1_intermediate = sum_fabs * (DAT_00013000 + cos(sum_fabs));

  return (long double)((term1_intermediate + cos(term1_intermediate) + cos(d_val2_64)) * tan(d_val2_64));
}

// Function: init_thr
undefined8 init_thr(uint32_t param_1, uint32_t param_2, uint32_t param_3, uint32_t param_4) {
  return CONCAT44(param_2 ^ ((param_4 << 0x10) | (param_3 >> 0x10)) ^ ((param_3 & 0x636f6d6) << 4) ^ (param_4 >> 10),
                  param_1 ^ (param_3 << 0x10) ^ ((param_4 >> 0xc) & 0x64626c73) ^ ((param_3 >> 10) | (param_4 << 0x16)));
}

// Function: init_fou
// Corrected signature to take two uint64_t arguments, as implied by usage.
long double init_fou(uint64_t val1, uint64_t val2) {
  double d_val1 = (double)val1;
  double d_val2 = (double)val2;

  double term_div_fabs = fabs(cos(d_val2) / sin(d_val1));
  double term_log2 = log2(term_div_fabs); // fabs(term_div_fabs) is just term_div_fabs
  double sin_term_div_fabs = sin(term_div_fabs);

  double final_sin_result = sin(fabs(term_div_fabs + d_val1 + term_log2 + (d_val2 - sin_term_div_fabs)));

  return (long double)((d_val1 + term_log2) - final_sin_result);
}

// Function: churn_rand_uint64
undefined8 churn_rand_uint64(void) {
  undefined4 temp_u32[2];
  churn_rand_item(temp_u32);
  // temp_u32[0] contains the low 32 bits, temp_u32[1] contains the high 32 bits from before the gyre update.
  return CONCAT44(temp_u32[1], temp_u32[0]);
}

// Function: churn_rand_float64
long double churn_rand_float64(void) {
  undefined4 temp_u32[2];
  churn_rand_item(temp_u32);
  // Reconstruct the 64-bit value from the two 32-bit parts.
  uint64_t val64_bits = CONCAT44(temp_u32[1], temp_u32[0]);
  return (long double)*(double*)&val64_bits; // Interpret the 64-bit integer as a double
}

// Function: churn_rand_item
undefined4 * churn_rand_item(undefined4 *param_1) {
  uint64_t current_gyre_val_before_update = gyre[gyre_pos];
  
  // Store original low and high 32-bit parts to be returned later
  *param_1 = (uint32_t)current_gyre_val_before_update;
  param_1[1] = (uint32_t)(current_gyre_val_before_update >> 32);

  // Calculate new gyre value
  uint64_t init_one_result = init_one(*param_1, param_1[1], gyre_pos, 0);
  long double init_fou_result = init_fou(current_gyre_val_before_update, init_one_result);

  // Update gyre[gyre_pos] with the bit representation of the new double value
  double d_init_fou_result = (double)init_fou_result;
  gyre[gyre_pos] = *(uint64_t*)&d_init_fou_result;

  gyre_pos = (gyre_pos + 1) & 0x3f; // Increment gyre_pos modulo 64

  return param_1; // Returns pointer to original values
}

// Function: churn_rand_uint32
uint64_t churn_rand_uint32(void) {
  uint64_t uVar_full_val = churn_rand_uint64();
  // If the least significant bit is set, shift by 31 bits, otherwise take lower 32 bits.
  return (uVar_full_val & 1) ? (uVar_full_val >> 0x1f) : (uVar_full_val & 0xffffffff);
}

// Function: churn_rand_uint8
undefined8 churn_rand_uint8(void) {
  uint64_t uVar_full_val = churn_rand_uint64();

  uint32_t high_part_of_full_val = (uint32_t)(uVar_full_val >> 32);
  uint32_t low_part_of_full_val = (uint32_t)uVar_full_val;

  uint32_t shift_amount_base = (low_part_of_full_val & 7) << 3; // (low_32 & 7) * 8
  uint8_t effective_shift_mask = shift_amount_base & 0x1f; // Shift amount capped at 31

  // Perform 64-bit right logical shift
  if ((shift_amount_base & 0x20) != 0) { // If actual shift amount is >= 32
    // High part of result is 0, low part is original high part shifted
    return CONCAT44(0, high_part_of_full_val >> effective_shift_mask);
  } else { // Actual shift amount is < 32
    // Standard 64-bit shift operation
    return CONCAT44(high_part_of_full_val >> effective_shift_mask,
                    (low_part_of_full_val >> effective_shift_mask) | (high_part_of_full_val << (32 - effective_shift_mask)));
  }
}

// Function: churn_initialize
void churn_initialize(undefined4 *param_1) {
  double current_local_28_double = 3.883147281147429e+281;

  for (uint32_t local_10 = 0; local_10 < 0x10; local_10++) {
    uint32_t uVar2_idx_base = local_10 * 4;

    uint64_t local_28_bits = *(uint64_t*)&current_local_28_double;
    
    // Call init_one and store result in gyre
    gyre[uVar2_idx_base] = init_one(uVar2_idx_base ^ (uint32_t)local_28_bits,
                                    (uint32_t)(local_28_bits >> 32),
                                    param_1[0], param_1[1]);

    // Call init_two and store result in gyre
    uint64_t param_1_2_3_val = *(uint64_t*)&param_1[2]; // Read param_1[2] and param_1[3] as a uint64_t
    double d_init_two_res = (double)init_two( (uint32_t)gyre[uVar2_idx_base] ^ uVar2_idx_base,
                                              (uint32_t)(gyre[uVar2_idx_base] >> 32),
                                              (uint32_t)param_1_2_3_val,
                                              (uint32_t)(param_1_2_3_val >> 32) );
    gyre[uVar2_idx_base + 1] = *(uint64_t*)&d_init_two_res;

    // Update current_local_28_double by replacing its low 32 bits with low 32 bits of d_init_two_res
    *(uint64_t*)&current_local_28_double = CONCAT44((uint32_t)(*(uint64_t*)&current_local_28_double >> 32),
                                                    get_double_low32(d_init_two_res));

    // Call init_thr and store result in gyre
    gyre[uVar2_idx_base + 2] = init_thr(uVar2_idx_base ^ get_double_low32(current_local_28_double),
                                        (uint32_t)(*(uint64_t*)&d_init_two_res >> 32),
                                        param_1[4], param_1[5]);

    // Call init_fou with default arguments (0,0) as implied by empty call in original snippet
    current_local_28_double = (double)init_fou(0, 0);
    gyre[uVar2_idx_base + 3] = *(uint64_t*)&current_local_28_double;
  }
  gyre_pos = 0;
}