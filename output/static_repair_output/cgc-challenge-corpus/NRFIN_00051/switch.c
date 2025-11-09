#include <stdint.h> // For uint32_t
#include <stdio.h>  // For printf

// Define the type for functions in our jump table
typedef uint32_t (*func_ptr_t)(void);

// Placeholder functions that the jump table might point to
uint32_t func_0(void) {
    printf("  Calling func_0\n");
    return 0x100;
}

uint32_t func_1(void) {
    printf("  Calling func_1\n");
    return 0x200;
}

uint32_t func_2(void) {
    printf("  Calling func_2\n");
    return 0x300;
}

// Default error function for unhandled indices
uint32_t func_error(void) {
    printf("  Error: Unhandled jump table index!\n");
    return 0xdeadc0de; // A specific error code
}

// This array represents the "jump table" (conceptually DAT_0003b000).
// The original code implies an array of 0xfffd elements (indices 0 to 0xfffc).
#define JUMP_TABLE_SIZE 0xfffd

// Global array of function pointers.
// In a real scenario, this might be statically initialized or populated by the linker.
func_ptr_t jump_table[JUMP_TABLE_SIZE];

// Function: static_switch
uint32_t static_switch(uint32_t param_1) {
    if (param_1 < JUMP_TABLE_SIZE) {
        // The original assembly pattern involving _GLOBAL_OFFSET_TABLE_ and DAT_0003b000
        // is a low-level way of resolving a function address via a jump table.
        // In standard C, this is directly translated to an array of function pointers.
        // The intermediate variable 'uVar1' has been removed as requested.
        return jump_table[param_1]();
    }
    return 0xdeadbeef; // Original return for out-of-bounds
}

// Main function to test static_switch
int main() {
    // Initialize all entries in the jump table to the error handler.
    for (uint32_t i = 0; i < JUMP_TABLE_SIZE; ++i) {
        jump_table[i] = func_error;
    }

    // Assign specific functions to their intended indices.
    jump_table[0] = func_0;
    jump_table[1] = func_1;
    jump_table[2] = func_2;
    // Add more assignments here for all valid indices that have specific functions.
    // For example, if there were a func_0xfffc:
    // jump_table[0xfffc] = func_0xfffc;

    printf("Testing static_switch:\n");

    uint32_t result = static_switch(0);
    printf("Result for param_1=0: 0x%x\n", result);

    result = static_switch(1);
    printf("Result for param_1=1: 0x%x\n", result);

    result = static_switch(2);
    printf("Result for param_1=2: 0x%x\n", result);

    result = static_switch(3); // Unhandled index, will call func_error
    printf("Result for param_1=3 (unhandled): 0x%x\n", result);

    result = static_switch(0xfffc); // Last valid index, but unassigned to a specific function, calls func_error
    printf("Result for param_1=0xfffc (unhandled): 0x%x\n", result);

    result = static_switch(0xfffd); // Out of bounds, returns 0xdeadbeef
    printf("Result for param_1=0xfffd (out of bounds): 0x%x\n", result);

    result = static_switch(0x10000); // Definitely out of bounds, returns 0xdeadbeef
    printf("Result for param_1=0x10000 (out of bounds): 0x%x\n", result);

    return 0;
}