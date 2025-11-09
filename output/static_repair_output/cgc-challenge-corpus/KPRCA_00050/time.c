#include <stdint.h> // For uint32_t
#include <stdio.h>  // For main function and potential testing

// Global variables (initialized for compilation)
uint32_t DAT_0001600c = 0;
uint32_t current_time = 0;

// Helper function for byte swapping
// Uses GCC/Clang built-in for efficiency if available, otherwise a portable manual swap.
#if defined(__GNUC__) || defined(__clang__)
#define swap32(val) __builtin_bswap32(val)
#else
static inline uint32_t swap32(uint32_t val) {
    return ((val << 24) & 0xFF000000) |
           ((val << 8) & 0x00FF0000) |
           ((val >> 8) & 0x0000FF00) |
           ((val >> 24) & 0x000000FF);
}
#endif

// CARRY4 macro - checks for carry when adding two unsigned 32-bit integers.
// Returns 1 if (a+b) overflows, 0 otherwise.
#define CARRY4(a, b) (((uint32_t)(a) + (uint32_t)(b)) < (uint32_t)(a))

// Function: handle_msg_time
uint32_t handle_msg_time(uint32_t *param_1, uint32_t param_2) {
    if (param_2 < 8) {
        return 0;
    }

    uint32_t val_param1_0 = swap32(param_1[0]);
    uint32_t val_param1_1 = swap32(param_1[1]);

    // Calculate uVar4 based on val_param1_0
    uint32_t uVar4 = ((((val_param1_0 >> 27) - (uint32_t)(val_param1_0 * 32 < val_param1_0)) * 4 | ((val_param1_0 * 31) >> 30)) +
                     CARRY4(val_param1_0 * 124, val_param1_0)) * 8 | ((val_param1_0 * 125) >> 29);

    uint32_t new_current_time_val = uVar4 + val_param1_1 * 1000;

    // Calculate new_DAT_0001600c based on val_param1_1 and uVar4
    uint32_t new_DAT_0001600c_val = (((((val_param1_1 >> 27) - (uint32_t)(val_param1_1 * 32 < val_param1_1)) * 4 | ((val_param1_1 * 31) >> 30)) +
                                      CARRY4(val_param1_1 * 124, val_param1_1)) * 8 | ((val_param1_1 * 125) >> 29)) +
                                     CARRY4(uVar4, val_param1_1 * 1000);

    // Comparison logic
    // The original code uses an `int` for `iVar5`, implying signed arithmetic for this part.
    int32_t iVar5_intermediate = (int32_t)(new_DAT_0001600c_val - DAT_0001600c) - (int32_t)(new_current_time_val < current_time);

    if ((new_DAT_0001600c_val < DAT_0001600c) ||
        ((new_DAT_0001600c_val - DAT_0001600c) < (uint32_t)(new_current_time_val < current_time)) ||
        (iVar5_intermediate != 0) ||
        (iVar5_intermediate == 0 && (new_current_time_val - current_time > 240000))) {
        // Condition met: return 1 without updating global times
        return 1;
    } else {
        // Condition not met: update global times and return 1
        current_time = new_current_time_val;
        DAT_0001600c = new_DAT_0001600c_val;
        return 1;
    }
}

// Main function for compilation and basic testing
int main() {
    uint32_t test_param_1[2];
    uint32_t test_param_2;

    // --- Test Case 1: param_2 < 8 ---
    test_param_2 = 4;
    uint32_t result1 = handle_msg_time(test_param_1, test_param_2);
    printf("Test Case 1 (param_2 < 8): Result = %u (Expected: 0)\n", result1);

    // --- Test Case 2: param_2 >= 8, initial state (should update and return 1) ---
    // Example input values (assuming network byte order for swap32 to convert)
    test_param_1[0] = swap32(100); // Represents a time component
    test_param_1[1] = swap32(20);  // Represents another time component
    test_param_2 = 8;

    DAT_0001600c = 0;
    current_time = 0;

    printf("\nTest Case 2 (param_2 >= 8, initial): DAT_0001600c=%u, current_time=%u\n", DAT_0001600c, current_time);
    uint32_t result2 = handle_msg_time(test_param_1, test_param_2);
    printf("Test Case 2: Result = %u\n", result2);
    printf("After Test Case 2: DAT_0001600c=%u, current_time=%u\n", DAT_0001600c, current_time);

    // --- Test Case 3: param_2 >= 8, values that might trigger the 'if' condition (no update) ---
    // This example aims to make new_DAT_0001600c_val less than current DAT_0001600c
    // (exact behavior depends on the complex arithmetic, this is illustrative)
    test_param_1[0] = swap32(1); // Smaller value
    test_param_1[1] = swap32(1); // Smaller value
    test_param_2 = 8;

    // Set current global times to high values to make new values seem "older"
    DAT_0001600c = 0xFFFFFFFF;
    current_time = 0xFFFFFFFF;

    printf("\nTest Case 3 (param_2 >= 8, 'older' values): DAT_0001600c=%u, current_time=%u\n", DAT_0001600c, current_time);
    uint32_t result3 = handle_msg_time(test_param_1, test_param_2);
    printf("Test Case 3: Result = %u\n", result3);
    printf("After Test Case 3: DAT_0001600c=%u, current_time=%u\n", DAT_0001600c, current_time);


    return 0;
}