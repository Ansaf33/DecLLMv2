#include <stdio.h>  // For printf
#include <stdlib.h> // For exit

// Function: PrintErrorAndTerminate
void PrintErrorAndTerminate(int error_code) {
  // The original code had an unconditional _terminate(1) before an if-block,
  // making the if-block unreachable. It also used an uninitialized variable
  // uStack_14 and an incorrect format specifier '@s'.
  // This revised version assumes the intent was to print an error based on the code
  // and then terminate.
  if (error_code == 0) {
    printf("ERROR: Allocation failed.\n");
  } else {
    printf("ERROR: An unexpected error occurred (code: %d).\n", error_code);
  }
  exit(1); // Terminate the program with an error status
}

// Function: VerifyPointerOrTerminate
void VerifyPointerOrTerminate(int pointer_status, const char* error_message) {
  // Replaced 'undefined4' with 'const char*' as it's used with '%s'.
  // Replaced '@s' with standard C format specifier '%s'.
  // Replaced '_terminate(1)' with standard C 'exit(1)'.
  if (pointer_status == 0) {
    printf("ERROR: Allocation failed: %s\n", error_message);
    exit(1); // Terminate the program with an error status
  }
  return;
}