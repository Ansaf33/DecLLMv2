#include <stdio.h>   // For printf in example main and dummy functions
#include <stdint.h>  // For fixed-width integer types like uint32_t and uint64_t

// Global variable definition.
// The original snippet uses '_ls_mode' without declaration, implying it's global.
int _ls_mode;

// Dummy implementation for 'single_binary_main_ls' to make the code compilable.
// The parameters 'param_1' and 'param_2' are assumed to be 4-byte and 8-byte
// unsigned integers based on the 'undefined4' and 'undefined8' in the original snippet.
void single_binary_main_ls(uint32_t param_1, uint64_t param_2) {
    printf("DEBUG: single_binary_main_ls called with param_1=%u, param_2=%llu\n", param_1, param_2);
    printf("DEBUG: _ls_mode inside single_binary_main_ls: %d\n", _ls_mode);
}

// Original function with fixed types and no intermediate variables to reduce.
void single_binary_main_dir(uint32_t param_1, uint64_t param_2) {
  _ls_mode = 2; // Directly assigns to global variable
  single_binary_main_ls(param_1, param_2); // Directly calls function with passed parameters
  return; // Explicit return, though optional for void functions at the end
}

// Main function to demonstrate the usage of single_binary_main_dir.
int main() {
    printf("--- Starting program ---\n");

    // Initialize _ls_mode to a default value for clarity before the first call
    _ls_mode = 0;
    printf("Initial _ls_mode: %d\n", _ls_mode);

    // Call single_binary_main_dir with example values
    printf("\nCalling single_binary_main_dir for the first time...\n");
    single_binary_main_dir(123, 456789012345ULL);
    printf("After first call, _ls_mode is: %d\n", _ls_mode);

    // Another demonstration call
    printf("\nCalling single_binary_main_dir for the second time...\n");
    single_binary_main_dir(999, 8888888888ULL);
    printf("After second call, _ls_mode is: %d\n", _ls_mode);

    printf("\n--- Program finished ---\n");
    return 0;
}