#include <stddef.h> // For NULL
#include <string.h> // For strlen (though mostly replaced by direct char checks)
#include <ctype.h>  // For tolower
#include <math.h>   // For pow

// Function: atof (simplified implementation, no leading whitespace handling as per original snippet)
double atof(char *nptr) {
  double result = 0.0;
  double exponent_value = 0.0;
  double decimal_divisor = 1.0; // Used for fractional part
  int sign = 1;
  int decimal_point_count = 0;
  int exponent_char_count = 0;

  // Flag to indicate an error state, allowing early exit or conditional processing
  int has_error = 0;

  // Initial checks for NULL or empty string
  if (nptr == NULL || *nptr == '\0') {
    has_error = 1;
  }

  if (has_error) {
    return 0.0;
  }

  // Handle optional sign
  if (*nptr == '-') {
    sign = -1;
    nptr++;
    if (*nptr == '\0') { // String ends after sign, e.g., "-"
      has_error = 1;
    }
  } else if (*nptr == '+') { // Standard atof also handles '+'
    nptr++;
    if (*nptr == '\0') { // String ends after sign, e.g., "+"
      has_error = 1;
    }
  }

  if (has_error) {
    return 0.0;
  }

  // Main parsing loop
  for (; *nptr != '\0'; nptr++) {
    if (*nptr >= '0' && *nptr <= '9') { // It's a digit
      if (exponent_char_count == 0) { // Before 'e' or 'E'
        if (decimal_point_count == 0) { // Before '.'
          result = result * 10.0 + (*nptr - '0');
        } else { // After '.'
          decimal_divisor *= 10.0;
          result += (double)(*nptr - '0') / decimal_divisor;
          // The original code's arbitrary limit of 4 fractional digits (local_34)
          // has been removed for standard atof behavior.
        }
      } else { // After 'e' or 'E', parsing exponent value
        exponent_value = exponent_value * 10.0 + (*nptr - '0');
      }
    } else { // Not a digit
      if (*nptr == '.') {
        decimal_point_count++;
      } else if (tolower((unsigned char)*nptr) == 'e') { // Check for 'e' or 'E'
        exponent_char_count++;
      } else {
        // Invalid character, stop parsing here.
        // The current 'result' is valid up to this point, similar to strtod behavior.
        break;
      }
    }

    // Error conditions during parsing (multiple '.' or 'e')
    if (decimal_point_count > 1 || exponent_char_count > 1) {
      has_error = 1;
      break;
    }
  }

  if (has_error) {
    return 0.0;
  }

  // Apply exponent if present
  if (exponent_char_count != 0) {
    result *= pow(10.0, exponent_value);
  }

  // Apply overall sign
  result *= sign;

  return result;
}