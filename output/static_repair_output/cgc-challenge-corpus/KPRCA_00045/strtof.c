#include <ctype.h>   // For isspace
#include <stdbool.h> // For bool
#include <stddef.h>  // For NULL
#include <stdio.h>   // For main function to test

// Define the magic numbers as constants
// DAT_00015288 appears to be 10.0
#define DECIMAL_BASE 10.0
// DAT_00015290 appears to be 0.0f
#define ZERO_FLOAT 0.0f

// Function: strtof
float strtof(char *__nptr, char **__endptr) {
    char *start_nptr = __nptr;       // Keep track of the original pointer for __endptr
    char *num_start_ptr;             // Pointer to the start of the numeric part (after sign)
    bool has_decimal = false;
    double value = 0.0;
    double decimal_place_multiplier = 1.0; // Multiplier for fractional part
    int sign = 1;                        // Default sign is positive

    // 1. Skip leading whitespace
    while (*__nptr != '\0' && isspace((unsigned char)*__nptr)) {
        __nptr++;
    }

    // 2. Handle sign
    if (*__nptr == '-') {
        sign = -1;
        __nptr++;
    } else if (*__nptr == '+') {
        __nptr++;
    }
    num_start_ptr = __nptr; // Mark this as the start of the number part

    // 3. Parse digits and decimal point
    char current_char;
    while ((current_char = *__nptr) != '\0') {
        if (current_char == '.') {
            if (has_decimal) { // Already encountered a decimal point, stop
                break;
            }
            has_decimal = true;
        } else if (current_char >= '0' && current_char <= '9') {
            int digit = current_char - '0';
            if (has_decimal) {
                decimal_place_multiplier /= DECIMAL_BASE;
                value += (double)digit * decimal_place_multiplier;
            } else {
                value = value * DECIMAL_BASE + (double)digit;
            }
        } else {
            // Not a digit or a decimal point, stop parsing
            break;
        }
        __nptr++;
    }

    // 4. Set __endptr and return value
    if (num_start_ptr == __nptr) { // No digits were parsed
        if (__endptr != NULL) {
            *__endptr = start_nptr; // Point to the very beginning if no number found
        }
        return ZERO_FLOAT; // Return 0.0f as per standard strtof behavior
    } else {
        if (__endptr != NULL) {
            *__endptr = __nptr; // Point to the character that stopped parsing
        }
        return (float)(value * sign);
    }
}

// Main function to demonstrate and test strtof
int main() {
    char *endptr;
    float result;

    printf("--- Testing custom strtof ---\n");

    // Test cases
    char s1[] = "123.45";
    result = strtof(s1, &endptr);
    printf("Input: \"%s\", Result: %f, Endptr: \"%s\"\n", s1, result, endptr);

    char s2[] = "-12.3";
    result = strtof(s2, &endptr);
    printf("Input: \"%s\", Result: %f, Endptr: \"%s\"\n", s2, result, endptr);

    char s3[] = "  +0.5abc";
    result = strtof(s3, &endptr);
    printf("Input: \"%s\", Result: %f, Endptr: \"%s\"\n", s3, result, endptr);

    char s4[] = "42";
    result = strtof(s4, &endptr);
    printf("Input: \"%s\", Result: %f, Endptr: \"%s\"\n", s4, result, endptr);

    char s5[] = ".123";
    result = strtof(s5, &endptr);
    printf("Input: \"%s\", Result: %f, Endptr: \"%s\"\n", s5, result, endptr);

    char s6[] = "no number"; // Error case: no digits
    result = strtof(s6, &endptr);
    printf("Input: \"%s\", Result: %f, Endptr: \"%s\"\n", s6, result, endptr);

    char s7[] = ""; // Empty string
    result = strtof(s7, &endptr);
    printf("Input: \"%s\", Result: %f, Endptr: \"%s\"\n", s7, result, endptr);

    char s8[] = "   "; // Only whitespace
    result = strtof(s8, &endptr);
    printf("Input: \"%s\", Result: %f, Endptr: \"%s\"\n", s8, result, endptr);

    char s9[] = "123.45.67"; // Multiple decimal points
    result = strtof(s9, &endptr);
    printf("Input: \"%s\", Result: %f, Endptr: \"%s\"\n", s9, result, endptr);

    char s10[] = "0";
    result = strtof(s10, &endptr);
    printf("Input: \"%s\", Result: %f, Endptr: \"%s\"\n", s10, result, endptr);

    char s11[] = " - 1.23"; // Space after sign, not a valid number format for this strtof
    result = strtof(s11, &endptr);
    printf("Input: \"%s\", Result: %f, Endptr: \"%s\"\n", s11, result, endptr);

    return 0;
}