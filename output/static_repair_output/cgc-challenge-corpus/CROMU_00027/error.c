#include <stdio.h> // For printf
#include <stdlib.h> // For exit

// Function: PrintErrorAndTerminate
void PrintErrorAndTerminate(int param_1) {
  // The original code had an uninitialized variable 'uStack_14' and
  // immediate termination before checking 'param_1'.
  // This version fixes the logic to make 'param_1' meaningful,
  // removes the intermediate variable, and ensures proper termination.
  if (param_1 == 0) {
    printf("ERROR: Allocation failed.\n");
  } else {
    printf("ERROR: An unspecified error occurred (code %d).\n", param_1);
  }
  exit(1); // Use exit(1) for standard program termination
}

// Function: VerifyPointerOrTerminate
void VerifyPointerOrTerminate(int param_1, const char* param_2) {
  // 'param_2' was 'undefined4' but used with '@s' (which should be '%s'),
  // implying it was meant to be a string. Changed to 'const char*'.
  if (param_1 == 0) {
    printf("ERROR: Allocation failed: %s\n", param_2); // Fixed format specifier
    exit(1); // Use exit(1) for standard program termination
  }
  return;
}