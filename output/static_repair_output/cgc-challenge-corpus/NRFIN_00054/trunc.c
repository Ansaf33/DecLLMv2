#include <stdint.h> // For uint32_t, uint64_t
#include <string.h> // For memcpy
#include <stdio.h>  // For printf in main (optional, but useful for demonstration)

// Type aliases from Ghidra decompiler output
typedef uint32_t undefined4;
typedef uint64_t undefined8;

// Function: float_trunc
// Interprets two 32-bit unsigned integers as the low and high parts of a 64-bit double's bit pattern.
// Applies a bitmask and then reinterprets the masked bits as a double.
// Returns the result as a long double.
long double float_trunc(undefined4 param_1, undefined4 param_2) {
  // Reconstruct the 64-bit value where param_2 is the high 32 bits and param_1 is the low 32 bits.
  uint64_t combined_bits = ((uint64_t)param_2 << 32) | param_1;

  // Apply the mask 0xffffffffffff0000 to clear the lower 16 bits of the 64-bit representation.
  uint64_t masked_bits = combined_bits & 0xffffffffffff0000ULL;

  double result_double;
  // Reinterpret the masked_bits as a double using memcpy for type-safe punning.
  memcpy(&result_double, &masked_bits, sizeof(double));
  
  return (long double)result_double;
}

// Dummy make_vector function to allow compilation.
// Its arguments are inferred from the call in vector_trunc.
void make_vector(undefined4 arg1, double arg2, int arg3, int arg4, double arg5) {
    // Placeholder implementation, prints arguments for demonstration purposes.
    printf("make_vector called with: %u, %f, %d, %d, %f\n", arg1, arg2, arg3, arg4, arg5);
}

// Function: vector_trunc
// Processes several undefined8 parameters (interpreted as bit patterns of doubles)
// using float_trunc and then calls make_vector.
undefined4 vector_trunc(undefined4 param_1, undefined8 param_2, undefined8 param_3, undefined8 param_4) {
  // Calculate lVar1 by splitting param_4 (undefined8) into two undefined4 arguments for float_trunc.
  long double lVar1_val = float_trunc((uint32_t)param_4, (uint32_t)(param_4 >> 32));

  // Calculate dVar3 directly from param_3, splitting param_3 into two undefined4 arguments.
  // The result of float_trunc (long double) is cast to double.
  double dVar3_val = (double)float_trunc((uint32_t)param_3, (uint32_t)(param_3 >> 32));

  // Calculate the second lVar2 (reassigning, as in the original snippet).
  // This call (float_trunc(param_2, dVar3_val)) is problematic given float_trunc's signature (undefined4, undefined4).
  // Param_2 is undefined8, and dVar3_val is double.
  // We assume a direct cast to undefined4 (uint32_t) for both, which is likely semantically incorrect
  // in terms of the original intent but allows for compilation with the given signature.
  long double lVar2_val = float_trunc((uint32_t)param_2, (uint32_t)dVar3_val);

  // Extract bit components of dVar3_val for make_vector arguments.
  // This interprets the double's bit pattern as a uint64_t to extract its lower and upper 32-bit halves.
  uint64_t dVar3_bits;
  memcpy(&dVar3_bits, &dVar3_val, sizeof(double));

  make_vector(
      param_1,
      (double)lVar2_val,
      (int)(dVar3_bits & 0xFFFFFFFFULL), // Interpreted as extracting the lower 32 bits (SUB84(dVar3,0))
      (int)(dVar3_bits >> 32),          // Interpreted as extracting the upper 32 bits ((ulonglong)dVar3 >> 0x20)
      (double)lVar1_val
  );
  return param_1;
}

// Minimal main function for compilation and demonstration.
int main() {
    // Example usage:
    // param_1: an arbitrary identifier (e.g., 10)
    // param_2, param_3, param_4: bit patterns of double values
    
    // Create some example double values
    double example_dval2 = 123.456;
    double example_dval3 = 789.012;
    double example_dval4 = 3.14159;

    // Convert double values to their uint64_t bit representations for passing to vector_trunc
    uint64_t p2_bits, p3_bits, p4_bits;
    memcpy(&p2_bits, &example_dval2, sizeof(double));
    memcpy(&p3_bits, &example_dval3, sizeof(double));
    memcpy(&p4_bits, &example_dval4, sizeof(double));

    printf("Calling vector_trunc with example values:\n");
    printf("  param_2 (double): %f (bit pattern: 0x%llx)\n", example_dval2, p2_bits);
    printf("  param_3 (double): %f (bit pattern: 0x%llx)\n", example_dval3, p3_bits);
    printf("  param_4 (double): %f (bit pattern: 0x%llx)\n", example_dval4, p4_bits);

    // Call vector_trunc with the example data
    undefined4 result = vector_trunc(10, p2_bits, p3_bits, p4_bits);
    printf("vector_trunc returned: %u\n", result);

    return 0;
}