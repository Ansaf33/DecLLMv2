#include <stdint.h> // For uint8_t, uint32_t, uint64_t
#include <math.h>   // For sin, cos, tan, fabs, log2

// Type definitions based on the snippet's "undefined" types
typedef uint8_t byte;
typedef uint32_t undefined4;
typedef uint64_t undefined8;
typedef unsigned int uint; // Assuming uint is typically 32-bit
typedef unsigned long long ulonglong; // For clarity, though 'unsigned long long' is standard
// longdouble is standard C 'long double'

// Helper macros for bit manipulation
// CONCAT44 combines two 32-bit values into a 64-bit value
#define CONCAT44(high, low) (((uint64_t)(high)) << 32 | (low))

// Union for accessing double's internal representation
// This is used by SUB84 and local_28._0_4_ etc.
// Assumes little-endian for u32[0] being the "low" part, which is common on Linux (x86_64).
typedef union {
    double d;
    uint32_t u32[2];
    uint64_t u64; // For convenience in assignments
} double_union;

// SUB84(dval, 0) extracts the low 32 bits of a double's internal representation.
static inline uint32_t get_low_32_from_double(double dval) {
    double_union du;
    du.d = dval;
    return du.u32[0];
}

// Global variables
// gyre is an array of 64 8-byte elements. It's accessed as uint32_t*, double*, int*.
// Declared as uint64_t array and cast pointers for assignments.
uint64_t gyre[64];
unsigned int gyre_pos = 0;
// Placeholder for DAT_00013000. Its actual value is not provided in the snippet.
const double DAT_00013000 = 1.0;

// Function prototypes
undefined8 init_one(uint param_1, uint param_2, uint param_3, uint param_4);
long double init_two(undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4 param_4);
undefined8 init_thr(uint param_1, uint param_2, uint param_3, uint param_4);
long double init_fou(undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4 param_4);
undefined8 churn_rand_uint64(void);
undefined4 * churn_rand_item(undefined4 *param_1);


// Function: churn_initialize
void churn_initialize(undefined4 *param_1) {
  double_union local_28_u;
  local_28_u.d = 3.883147281147429e+281;

  for (uint local_10 = 0; local_10 < 0x10; local_10++) {
    uint uVar2 = local_10 * 4; // This is the index into the 8-byte gyre elements

    // First 8 bytes (undefined8)
    undefined8 uVar5_init_one_res = init_one(uVar2 ^ local_28_u.u32[0], (uint)(local_28_u.u64 >> 32), param_1[0], param_1[1]);
    ((uint64_t*)gyre)[uVar2] = uVar5_init_one_res;

    // Second 8 bytes (double)
    uint64_t init_two_arg1 = CONCAT44((uint32_t)(uVar5_init_one_res >> 32), (uint32_t)uVar5_init_one_res ^ uVar2);
    uint64_t init_two_arg2 = CONCAT44(param_1[3], param_1[2]); // Read 8 bytes from &param_1[2]
    long double lVar4_init_two_res = init_two(
        (uint32_t)init_two_arg1, (uint32_t)(init_two_arg1 >> 32),
        (uint32_t)init_two_arg2, (uint32_t)(init_two_arg2 >> 32)
    );
    double dVar1_init_two_res = (double)lVar4_init_two_res;
    ((double*)gyre)[uVar2 + 1] = dVar1_init_two_res;
    local_28_u.u32[0] = get_low_32_from_double(dVar1_init_two_res);

    // Third 8 bytes (undefined8)
    undefined8 uVar5_init_thr_res = init_thr(uVar2 ^ local_28_u.u32[0], (uint)((uint64_t)dVar1_init_two_res >> 32), param_1[4], param_1[5]);
    ((uint64_t*)gyre)[uVar2 + 2] = uVar5_init_thr_res;

    // Fourth 8 bytes (double)
    // The original snippet had init_fou() with no arguments, which is a mismatch with its definition.
    // Assuming it takes arguments derived from uVar5_init_thr_res and local_28_u.u64,
    // following the pattern seen in churn_rand_item.
    long double lVar4_init_fou_res = init_fou(
        (uint32_t)uVar5_init_thr_res, (uint32_t)(uVar5_init_thr_res >> 32),
        local_28_u.u32[0], local_28_u.u32[1]
    );
    local_28_u.d = (double)lVar4_init_fou_res;
    ((double*)gyre)[uVar2 + 3] = local_28_u.d;
  }
  gyre_pos = 0;
}

// Function: init_one
undefined8 init_one(uint param_1, uint param_2, uint param_3, uint param_4) {
  return CONCAT44(param_2 ^ (param_4 >> 0x15) ^ ((param_3 & 0x3234b29) << 5) ^ ((param_4 >> 9) & 0x6372616e) ^ (param_3 << 0x12),
                  param_1 ^ ((param_3 >> 0x15) | (param_4 << 0xb)) ^ (((param_3 >> 9) | (param_4 << 0x17)) & 0x6b74776f));
}

// Function: init_two
long double init_two(undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4 param_4) {
  double val_param12 = (double)CONCAT44(param_2, param_1);
  double val_param34 = (double)CONCAT44(param_4, param_3);

  double abs_sin_val12 = fabs(sin(val_param12));
  double abs_cos_val34 = fabs(cos(val_param34));
  double sum_abs_vals = abs_cos_val34 + abs_sin_val12;

  double term1_inner_cos = cos(sum_abs_vals);
  double term1_outer_product = sum_abs_vals * (DAT_00013000 + term1_inner_cos);

  double term2_cos_val34 = cos(val_param34);
  double term3_cos_term1 = cos(term1_outer_product);
  double tan_val34 = tan(val_param34);

  return (long double)((term1_outer_product + term3_cos_term1 + term2_cos_val34) * tan_val34);
}

// Function: init_thr
undefined8 init_thr(uint param_1, uint param_2, uint param_3, uint param_4) {
  return CONCAT44(param_2 ^ ((param_4 << 0x10) | (param_3 >> 0x10)) ^ ((param_3 & 0x636f6d6) << 4) ^ (param_4 >> 10),
                  param_1 ^ (param_3 << 0x10) ^ ((param_4 >> 0xc) & 0x64626c73) ^ ((param_3 >> 10) | (param_4 << 0x16)));
}

// Function: init_fou
long double init_fou(undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4 param_4) {
  double local_24_val = (double)CONCAT44(param_2, param_1);
  double local_2c_val = (double)CONCAT44(param_4, param_3);

  double c_val = cos(local_2c_val);
  double s_val = sin(local_24_val);

  double dVar1_abs_ratio = fabs(c_val / s_val);
  double dVar2_sin_dVar1 = sin(dVar1_abs_ratio);
  double dVar3_log2_abs_dVar1 = log2(fabs(dVar1_abs_ratio));

  double dVar1_sum_abs = fabs(dVar1_abs_ratio + local_24_val + dVar3_log2_abs_dVar1 + (local_2c_val - dVar2_sin_dVar1));
  double dVar1_sin_sum = sin(dVar1_sum_abs);

  return (long double)((local_24_val + dVar3_log2_abs_dVar1) - dVar1_sin_sum);
}

// Function: churn_rand_uint8
undefined8 churn_rand_uint8(void) {
  undefined8 uVar7 = churn_rand_uint64();
  uint32_t high = (uint32_t)(uVar7 >> 32);
  uint32_t low = (uint32_t)uVar7;

  uint32_t shift_amount = (low & 7) << 3;
  uint8_t actual_shift = (uint8_t)shift_amount & 0x1f;

  uint32_t uVar5_val = high >> actual_shift;
  uint32_t uVar3_val = low >> actual_shift | high << (32 - actual_shift);

  uint32_t uVar6_val;
  if ((shift_amount & 0x20) != 0) { // if shift_amount >= 32
    uVar6_val = 0;
    uVar3_val = uVar5_val;
  } else {
    uVar6_val = uVar5_val;
  }
  return CONCAT44(uVar6_val, uVar3_val);
}

// Function: churn_rand_uint32
ulonglong churn_rand_uint32(void) {
  ulonglong uVar2 = churn_rand_uint64();
  return (uVar2 & 1) ? (uVar2 >> 0x1f) : (uVar2 & 0xffffffffULL);
}

// Function: churn_rand_uint64
undefined8 churn_rand_uint64(void) {
  uint32_t temp_arr[2];
  churn_rand_item(temp_arr);
  return CONCAT44(temp_arr[1], temp_arr[0]); // CONCAT44(high, low)
}

// Function: churn_rand_float64
long double churn_rand_float64(void) {
  double_union temp_union;
  churn_rand_item(temp_union.u32);
  return (long double)temp_union.d;
}

// Function: churn_rand_item
undefined4 * churn_rand_item(undefined4 *param_1) {
  uint32_t u1 = ((uint32_t*)gyre)[gyre_pos * 2];
  uint32_t u2 = ((uint32_t*)gyre)[gyre_pos * 2 + 1];
  uint64_t u3 = ((uint64_t*)gyre)[gyre_pos];

  uint64_t init_one_res = init_one(u1, u2, gyre_pos, 0);
  
  long double init_fou_res = init_fou(
      (uint32_t)u3, (uint32_t)(u3 >> 32), // param_1, param_2 from u3 (low, high)
      (uint32_t)init_one_res, (uint32_t)(init_one_res >> 32) // param_3, param_4 from init_one_res (low, high)
  );

  double_union temp_double_union;
  temp_double_union.d = (double)init_fou_res;

  ((uint64_t*)gyre)[gyre_pos] = temp_double_union.u64;

  gyre_pos = (gyre_pos + 1) & 0x3f;
  
  param_1[0] = u1;
  param_1[1] = u2;
  return param_1;
}