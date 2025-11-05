#include <stdio.h>  // For fprintf
#include <stdlib.h> // For exit

// Function: PrintErrorAndTerminate
// Fixes: Removed unreachable code, replaced undefined4 with a generic message,
//        changed _terminate to exit, fixed printf format specifier,
//        uses stderr for error output.
void PrintErrorAndTerminate(int param_1) {
  if (param_1 == 0) {
    fprintf(stderr, "ERROR: Allocation failed: Unknown reason.\n");
  } else {
    // This branch handles a generic error when param_1 is not 0.
    fprintf(stderr, "ERROR: An unspecified error occurred (code %d).\n", param_1);
  }
  exit(1);
}

// Function: VerifyPointerOrTerminate
// Fixes: Corrected parameter types for pointer and error message string,
//        changed _terminate to exit, fixed printf format specifier,
//        uses stderr for error output.
void VerifyPointerOrTerminate(void* ptr, const char* errorMessage) {
  if (ptr == NULL) {
    fprintf(stderr, "ERROR: Allocation failed: %s\n", errorMessage);
    exit(1);
  }
}