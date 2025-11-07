#include <stdint.h> // For uint32_t, uint64_t, UINT32_MAX
#include <stdbool.h> // For bool

// Assuming swap32 is a byte swap, often __builtin_bswap32 for GCC/Clang
// On systems where __builtin_bswap32 is not available, a manual implementation or ntohl/htonl would be needed.
// For Linux compilable C code, __builtin_bswap32 is common.
#define swap32(x) __builtin_bswap32(x)

// Global variables (assuming these are defined elsewhere or need to be defined here)
// Marked as static to restrict their scope to this compilation unit, as is common for decompiled globals.
static uint32_t current_time = 0;
static uint32_t DAT_0001600c = 0;

// CARRY4 macro equivalent for unsigned 32-bit addition
// Returns 1 if (a + b) results in a carry, 0 otherwise.
#define CARRY4_MACRO(a, b) ((uint32_t)(a) + (uint32_t)(b) < (uint32_t)(a))

// Helper function to encapsulate the complex fixed-point multiplication high-part calculation.
// This function directly translates the decompiled arithmetic pattern.
static inline uint32_t calculate_complex_high_part(uint32_t val) {
    // (uint)(val * 0x20 < val) checks for overflow of val * 32.
    // If val * 32 overflows, (val * 32 < val) is true (1), otherwise false (0).
    uint32_t overflow_check_32 = (val > UINT32_MAX / 32) ? 1 : 0;

    // CARRY4(val * 0x7c, val) checks for carry in (val * 124) + val = val * 125.
    uint32_t carry_check_125 = CARRY4_MACRO(val * 124, val);

    return ((((val >> 27) - overflow_check_32) * 4 | (val * 31) >> 30) + carry_check_125) * 8 | (val * 125) >> 29;
}

// Function: handle_msg_time
uint32_t handle_msg_time(uint32_t *param_1, uint32_t param_2) {
    if (param_2 < 8) {
        return 0; // Input too short
    }

    uint32_t input_low_ts = swap32(param_1[0]);
    uint32_t input_high_ts = swap32(param_1[1]);

    // Calculate the high 32-bit part of a custom multiplication of input_low_ts
    uint32_t uVar4 = calculate_complex_high_part(input_low_ts);

    // Perform 64-bit addition: (input_high_ts * 1000) + uVar4
    // The result is stored in `calculated_low_ts` and `calculated_high_ts`
    uint32_t calculated_low_ts = input_high_ts * 1000;
    uint32_t calculated_high_ts = calculate_complex_high_part(input_high_ts); // This is (input_high_ts * K) >> 32

    // Add uVar4 to the low part and propagate carry to the high part
    bool carry_from_uVar4_add = CARRY4_MACRO(calculated_low_ts, uVar4);
    calculated_low_ts += uVar4;
    calculated_high_ts += carry_from_uVar4_add;
    
    // The original code has an additional carry here which is `CARRY4(uVar4, uVar3 * 1000)`
    // This implies that the 'uVar3 * 1000' and 'uVar4' are being added to form the new low 32 bits,
    // and the carry from that addition is then added to the high 32 bits.
    // Let's re-interpret the original lines directly:
    // uVar2 = uVar4 + uVar3 * 1000;
    // uVar3 = calculate_complex_high_part(uVar3) + (uint)CARRY4(uVar4, uVar3 * 1000);
    //
    // Let's use temporary variables to represent the intermediate `uVar2` and `uVar3` values.
    uint32_t new_low_ts_val = uVar4 + (input_high_ts * 1000);
    uint32_t new_high_ts_val = calculate_complex_high_part(input_high_ts) + CARRY4_MACRO(uVar4, (input_high_ts * 1000));

    // Combine into 64-bit values for comparison
    uint64_t new_timestamp_64 = ((uint64_t)new_high_ts_val << 32) | new_low_ts_val;
    uint64_t current_timestamp_64 = ((uint64_t)DAT_0001600c << 32) | current_time;

    // Check if the new timestamp is outside the acceptable window
    // (new_timestamp < current_timestamp) OR (new_timestamp - current_timestamp > 240000)
    if (new_timestamp_64 < current_timestamp_64 || (new_timestamp_64 - current_timestamp_64) > 240000) {
        // New timestamp is too old or too far in the future, do not update globals.
        // The function still returns 1, indicating it processed the input.
    } else {
        // Timestamp is within the acceptable range, update global time.
        current_time = new_low_ts_val;
        DAT_0001600c = new_high_ts_val;
    }

    return 1; // Indicates successful processing (even if globals weren't updated)
}