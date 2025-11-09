#include <ctype.h>   // For isspace
#include <stdbool.h> // For bool
#include <stddef.h>  // For NULL

// Function: strtod
double strtod(char *__nptr, char **__endptr) {
  bool has_decimal_point = false;
  bool is_negative = false;
  char *start_of_parsing = __nptr; // Stores the original __nptr for error case reset
  char *start_of_digits_or_dot;     // Stores the pointer after sign processing

  double fractional_multiplier = 1.0;
  double result = 0.0;

  // Skip leading whitespace
  while (*__nptr != '\0' && isspace((int)*__nptr)) {
    __nptr++;
  }

  // Handle sign
  if (*__nptr == '-') {
    is_negative = true;
    __nptr++;
    start_of_digits_or_dot = __nptr;
  } else {
    start_of_digits_or_dot = __nptr; // This assignment matches the original's `pcVar4 = __nptr;` before checking for '+'
    if (*__nptr == '+') {
      __nptr++;
      start_of_digits_or_dot = __nptr;
    }
  }

  // Process digits and decimal point
  while (*__nptr != '\0') {
    if (*__nptr == '.') {
      // If a decimal point was already found, treat subsequent '.' as non-digit
      // and stop parsing the number.
      if (has_decimal_point) {
          break;
      }
      has_decimal_point = true;
      __nptr++;
    } else {
      // Check if it's a digit
      if (*__nptr < '0' || '9' < *__nptr) {
          break; // Not a digit, stop processing
      }

      if (has_decimal_point) {
        fractional_multiplier /= 10.0; // Assuming DAT_000153b8 is 10.0
        result += (double)(*__nptr - '0') * fractional_multiplier;
      } else {
        result = result * 10.0 + (double)(*__nptr - '0'); // Assuming DAT_000153b8 is 10.0
      }
      __nptr++;
    }
  }

  // Error handling: if no digits were processed after sign (e.g., "  ", "  -", "  +")
  // The original code's `if (pcVar4 == __nptr)` implies checking if `__nptr` never moved
  // from its position after initial sign processing.
  if (start_of_digits_or_dot == __nptr) {
    result = 0.0;
    __nptr = start_of_parsing; // Reset __nptr to original start for __endptr
  }

  // Set end pointer
  if (__endptr != NULL) {
    *__endptr = __nptr;
  }

  // Apply sign
  if (is_negative) {
    result = -result;
  }

  // The original line `local_1c = DAT_000153c0 + local_1c;` is removed.
  // Assuming DAT_000153c0 was 0.0, this line is redundant.
  // If it was any other value, it would apply an arbitrary offset not
  // consistent with standard strtod behavior.

  return result;
}