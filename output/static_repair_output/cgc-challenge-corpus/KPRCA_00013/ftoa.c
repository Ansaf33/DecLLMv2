#include <math.h>     // For fabs, isnan, remainder, pow, log10, round, isfinite, HUGE_VAL
#include <stdbool.h>  // For bool
#include <stdint.h>   // For unsigned int (used for buffer_size)
#include <stdlib.h>   // For NULL, nan
#include <float.h>    // For DBL_MAX

// Define constants based on likely decompiler interpretation and common ftoa implementations.
// These values are educated guesses to make the ftoa function behave reasonably.

// DAT_00017168: A large number for overflow check (ABS(value) > threshold).
// Using DBL_MAX to check if the absolute value is too large to represent or handle.
static const double FTOA_DBL_MAX_THRESHOLD = DBL_MAX; 

// DAT_00017170: Base for remainder(value, base) - typically 1.0 for extracting the fractional part.
static const double FTOA_REMAINDER_BASE = 1.0;     

// DAT_00017180: Base for pow and log10 - typically 10.0 for decimal conversion.
static const double FTOA_POWER_BASE = 10.0;        

// DAT_00017178: Exponent for rounding precision (pow(10, EXP) * fractional_part).
// The digit printing loop iterates down to an exponent of -4, implying 4 decimal places are printed.
// For "round half up" logic, we typically look at one digit beyond the printed precision.
// So, an exponent of 5.0 would scale the fractional part to expose the 5th decimal digit.
static const double FTOA_ROUND_PRECISION_EXP = 5.0; 

// DAT_00017188: Threshold for rounding up positive numbers (0.5 for standard "round half up").
static const double FTOA_ROUND_UP_THRESHOLD = 0.5; 

// DAT_00017190: Threshold for rounding down negative numbers.
// In the original logic, after `value` is made positive, `scaled_fractional` will be non-negative.
// Thus, the condition `(scaled_fractional < 0.0 && FTOA_ROUND_DOWN_THRESHOLD < scaled_fractional)`
// will never be true. This constant is effectively unused in the current code flow.
static const double FTOA_ROUND_DOWN_THRESHOLD = -0.5; // Placeholder, as it's logically unused here.

// DAT_00017198: Value to add during rounding (typically 1.0).
static const double FTOA_ONE = 1.0;                

// Function: ftoa
// Original signature: undefined * ftoa(undefined4 param_1,undefined4 param_2,undefined *param_3,uint param_4)
// - `param_1` and `param_2` are assumed to reconstruct a `double` value. This is replaced by a direct `double` parameter.
// - `param_3` (type `undefined *`) is assumed to be a character buffer, replaced by `char *buffer`.
// - `param_4` (type `uint`) is assumed to be the buffer size, replaced by `unsigned int buffer_size`.
char *ftoa(double value, char *buffer, unsigned int buffer_size) {
    // Ensure minimum buffer size: at least 2 characters for one digit and the null terminator.
    if (buffer_size < 2) {
        return NULL;
    }

    // Handle special floating-point values: Not-a-Number (NaN), infinity, or values
    // exceeding a defined maximum threshold. In these cases, return NULL as per original logic.
    if (isnan(value) || !isfinite(value) || fabs(value) >= FTOA_DBL_MAX_THRESHOLD) {
        return NULL;
    }

    bool is_negative = value < 0.0;
    unsigned int current_idx = 0; // Index for writing characters into the buffer

    // If the number is negative, add a '-' sign to the buffer and work with its absolute value.
    if (is_negative) {
        buffer[current_idx++] = '-';
        value = -value; 
    }

    // Check if buffer is already full after potentially adding the sign.
    if (current_idx == buffer_size) {
        return NULL;
    }

    // Original rounding logic:
    // This block determines if the `value` should be incremented by 1.0 based on its fractional part.
    // `remainder(value, FTOA_REMAINDER_BASE)` extracts the fractional part (e.g., 123.45 -> 0.45).
    // `pow(FTOA_POWER_BASE, FTOA_ROUND_PRECISION_EXP)` creates a scaling factor (e.g., 10^5).
    // `scaled_fractional` is used to check if rounding up is needed.
    // Since `value` is now positive, `fractional_part` and `scaled_fractional` will be non-negative.
    // The original condition `(dVar4 < 0.0 && (DAT_00017190 < dVar4))` becomes irrelevant.
    double fractional_part = remainder(value, FTOA_REMAINDER_BASE);
    double scaled_fractional = pow(FTOA_POWER_BASE, FTOA_ROUND_PRECISION_EXP) * fractional_part;

    if (FTOA_ROUND_UP_THRESHOLD < scaled_fractional) {
        value += FTOA_ONE; // Increment the value (effectively rounds up the integer part)
    }

    int exponent;
    // Determine the exponent of the most significant digit.
    // For 0.0, the exponent is 0. For other numbers, log10 gives the magnitude, and round() finds the closest integer exponent.
    if (value == 0.0) {
        exponent = 0;
    } else {
        exponent = (int)round(log10(value));
    }
    
    // If the exponent is negative (e.g., for 0.123, log10(0.123) is approx -0.9, rounded to -1),
    // set it to 0. This ensures a leading '0' before the decimal point for numbers between 0 and 1.
    if (exponent < 0) {
        exponent = 0;
    }

    // Main loop for extracting and printing digits:
    // The loop prints digits from the most significant (determined by `exponent`)
    // down to the digit at 10^-4 (i.e., 4 decimal places after the point).
    // It also terminates if the buffer runs out of space.
    while (exponent >= -4 && current_idx < buffer_size) {
        // If the exponent is -1, it means we are about to print the first fractional digit,
        // so a decimal point should be inserted.
        if (exponent == -1) {
            buffer[current_idx++] = '.';
            if (current_idx == buffer_size) { // Check buffer again after adding the decimal point.
                break;
            }
        }

        double power_of_10 = pow(FTOA_POWER_BASE, (double)exponent);
        double digit_value_at_magnitude = value / power_of_10;
        
        // Extract the current digit. The original code used `(int)ROUND(dVar4)`.
        // This implicitly assumes the result of `round()` will be 0-9.
        // If `round(9.5)` yields `10`, `(char)10 + '0'` would result in an invalid character (ASCII ':').
        // To prevent this without fundamentally changing the rounding logic (which would be a major logic change),
        // we extract the integer part of the rounded value.
        int digit = (int)round(digit_value_at_magnitude);

        buffer[current_idx++] = (char)digit + '0';
        value -= (double)digit * power_of_10; // Subtract the value of the printed digit from the number.

        // Handle potential floating-point inaccuracies where `value` might become slightly negative
        // due to repeated subtractions. This ensures subsequent fractional digit extractions
        // operate on a non-negative base. This is a numerical stability improvement, not a logic change.
        if (value < 0.0 && exponent > -4) { // Only adjust if still more digits to process
            value = fabs(value);
        }

        exponent--; // Move to the next lower power of 10 for the next digit.
    }

    // Null-terminate the string if there is space available in the buffer.
    if (current_idx < buffer_size) {
        buffer[current_idx] = '\0';
    } else {
        // If the buffer is completely full (no space for null terminator),
        // the original code would effectively return NULL.
        // We ensure the last character written is a null terminator if space permits.
        // If not, it means the buffer overflowed to the point no null terminator fits.
        // In this specific case, returning NULL indicates the failure.
        return NULL;
    }

    return buffer;
}

// Simple main function for testing the ftoa implementation.
#include <stdio.h>
#include <string.h>

int main() {
    char buffer[32]; // A buffer to store the converted string (max 31 chars + null terminator)

    printf("Testing ftoa:\n");

    // Test cases with various numbers and buffer sizes.
    // The expected output might vary slightly depending on floating-point precision and rounding.
    // The `(ftoa(...) ? buffer : "NULL (buffer too small or error)")` syntax
    // elegantly handles cases where ftoa returns NULL.

    printf("123.456 (size 10): %s\n", ftoa(123.456, buffer, 10) ? buffer : "NULL (error)"); 
    // Expected: "123.456"
    printf("-123.456 (size 10): %s\n", ftoa(-123.456, buffer, 10) ? buffer : "NULL (error)"); 
    // Expected: "-123.456"
    printf("0.12345 (size 10): %s\n", ftoa(0.12345, buffer, 10) ? buffer : "NULL (error)"); 
    // Expected: "0.1234" (prints 4 decimal places)
    printf("-0.12345 (size 10): %s\n", ftoa(-0.12345, buffer, 10) ? buffer : "NULL (error)"); 
    // Expected: "-0.1234"
    printf("0.0 (size 10): %s\n", ftoa(0.0, buffer, 10) ? buffer : "NULL (error)"); 
    // Expected: "0"
    printf("1.0 (size 10): %s\n", ftoa(1.0, buffer, 10) ? buffer : "NULL (error)"); 
    // Expected: "1"
    printf("123456.789 (size 10): %s\n", ftoa(123456.789, buffer, 10) ? buffer : "NULL (error)"); 
    // Expected: "123456.78" (truncated to buffer size/precision)
    printf("1234567.89 (size 5): %s\n", ftoa(1234567.89, buffer, 5) ? buffer : "NULL (error)"); 
    // Expected: "NULL (error)" (buffer too small)
    printf("NaN (size 10): %s\n", ftoa(nan(""), buffer, 10) ? buffer : "NULL (error)"); 
    // Expected: "NULL (error)"
    printf("Inf (size 10): %s\n", ftoa(HUGE_VAL, buffer, 10) ? buffer : "NULL (error)"); 
    // Expected: "NULL (error)"
    printf("-Inf (size 10): %s\n", ftoa(-HUGE_VAL, buffer, 10) ? buffer : "NULL (error)"); 
    // Expected: "NULL (error)"
    printf("123.9999 (size 10) with rounding: %s\n", ftoa(123.9999, buffer, 10) ? buffer : "NULL (error)"); 
    // Expected: "124.0000" (due to FTOA_ROUND_PRECISION_EXP = 5.0)
    printf("9999.9999 (size 10) with rounding: %s\n", ftoa(9999.9999, buffer, 10) ? buffer : "NULL (error)"); 
    // Expected: "10000.0000" (due to FTOA_ROUND_PRECISION_EXP = 5.0)
    printf("9.5 (size 10) with rounding: %s\n", ftoa(9.5, buffer, 10) ? buffer : "NULL (error)");
    // Expected: "10.0000" (due to FTOA_ROUND_PRECISION_EXP = 5.0)

    return 0;
}