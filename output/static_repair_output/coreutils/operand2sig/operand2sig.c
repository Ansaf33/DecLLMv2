#include <ctype.h>   // for isdigit
#include <stdio.h>   // for fprintf, snprintf, stderr
#include <stdlib.h>  // for strdup, free, strtol, abort
#include <string.h>  // for strchr, strcmp
#include <errno.h>   // for errno
#include <stdarg.h>  // for va_list

// Assuming 'uint' refers to 'unsigned int'
// If it's a custom type from a specific header, that header would be needed.
// For standard C, 'unsigned int' is the correct type.

// Dummy declarations for external functions to make the code compilable.
// In a real project, these would be provided by other modules or libraries.

// str2sig: Parses a string to a signal number. Returns 0 on success, non-zero on failure.
int str2sig(const char *s, unsigned int *out_sig);

// quote_func: Returns a quoted version of the input string, typically for error messages.
// The original `undefined8` suggests an 8-byte return, likely a pointer (char*).
char *quote_func(const char *s);

// gettext_func: Retrieves a translated string.
const char *gettext_func(const char *msgid);

// error_func: A GNU extension for error reporting.
// void error(int status, int errnum, const char *format, ...);
void error_func(int status, int errnum, const char *format, ...);

// __stack_chk_fail: Function called by the stack smashing protector when a canary is corrupted.
void __stack_chk_fail(void);

// Function: operand2sig
unsigned int operand2sig(char *param_1) {
  unsigned int result_sig; // Renamed from local_58 for clarity

  // Check if param_1 starts with a digit
  if (!isdigit((unsigned char)*param_1)) { // Use (unsigned char) for isdigit for safety
    char *s = strdup(param_1); // Duplicate the input string for modification
    if (s == NULL) {
      // strdup failed (out of memory).
      // Original xstrdup might have terminated or had other error handling.
      // For this context, treat as an invalid signal.
      result_sig = 0xFFFFFFFF;
    } else {
      // Convert string to uppercase (only 'a'-'z' to 'A'-'Z')
      for (char *p = s; *p != '\0'; ++p) {
        if (strchr("abcdefghijklmnopqrstuvwxyz", *p) != NULL) {
          *p -= 0x20; // Convert to uppercase
        }
      }

      // First attempt to parse the signal name
      int ret = str2sig(s, &result_sig);

      // If parsing failed, check for "SIG" prefix and try again
      if (ret != 0) {
        if (s[0] == 'S' && s[1] == 'I' && s[2] == 'G') {
          // If it has "SIG" prefix, try parsing the remainder
          if (str2sig(s + 3, &result_sig) != 0) {
            result_sig = 0xFFFFFFFF; // Still failed, mark as invalid
          }
        } else {
          result_sig = 0xFFFFFFFF; // No "SIG" prefix and initial parse failed, mark as invalid
        }
      }
      free(s); // Free the duplicated string
    }
  } else {
    // Param_1 starts with a digit, attempt to parse as a number
    errno = 0; // Clear errno before strtol call
    char *endptr;
    long val_l = strtol(param_1, &endptr, 10); // Parse as base 10 long integer

    // Validate strtol result:
    // 1. No characters were consumed (param_1 == endptr).
    // 2. Unparsed characters remain (*endptr != '\0').
    // 3. errno was set (conversion error or out of range).
    // 4. The long value doesn't fit into an unsigned int (implies overflow/underflow for unsigned int).
    if ((param_1 == endptr) || (*endptr != '\0') || (errno != 0) || (val_l != (long)(unsigned int)val_l)) {
      result_sig = 0xFFFFFFFF; // Mark as invalid
    } else {
      result_sig = (unsigned int)val_l; // Convert to unsigned int
    }

    // Apply specific masking logic if the signal is valid
    if (result_sig != 0xFFFFFFFF) {
      // If the signal value (interpreted as int) is less than 255, mask with 0x7F.
      // Otherwise, mask with 0xFF.
      result_sig &= ((int)result_sig < 0xFF) ? 0x7F : 0xFF;
    }
  }

  // Final validation: check if the signal number is within the valid range (0-64)
  // 0xFFFFFFFF will be interpreted as -1 when cast to int, triggering the first part of the condition.
  if (((int)result_sig < 0) || ((int)result_sig > 0x40)) { // 0x40 is 64
    error_func(0, 0, gettext_func("%s: invalid signal"), quote_func(param_1));
    result_sig = 0xFFFFFFFF; // Set to error value
  }

  // Stack canary check (local_20 and in_FS_OFFSET) has been removed.
  // This is a compiler/platform-specific security feature not part of standard C.
  // Including it would require specific compiler flags or system headers,
  // which might not be universally available or desired for a generic C solution.

  return result_sig;
}

// --- Dummy Implementations for compilation ---

// A basic dummy implementation for str2sig.
// Recognizes a few common signal names. Extend as needed for real functionality.
int str2sig(const char *s, unsigned int *out_sig) {
    if (s == NULL) return 1;
    if (strcmp(s, "HUP") == 0) { *out_sig = 1; return 0; }
    if (strcmp(s, "INT") == 0) { *out_sig = 2; return 0; }
    if (strcmp(s, "QUIT") == 0) { *out_sig = 3; return 0; }
    if (strcmp(s, "KILL") == 0) { *out_sig = 9; return 0; }
    if (strcmp(s, "TERM") == 0) { *out_sig = 15; return 0; }
    if (strcmp(s, "USR1") == 0) { *out_sig = 10; return 0; }
    if (strcmp(s, "USR2") == 0) { *out_sig = 12; return 0; }
    // Add more signal names as necessary for a complete implementation.
    return 1; // Indicate failure to parse
}

// Dummy implementation for quote_func.
// Returns a simple quoted string using a static buffer. Not thread-safe.
char *quote_func(const char *s) {
    static char quoted_buf[256]; // Static buffer, not thread-safe. For compilation only.
    if (s == NULL) {
        strncpy(quoted_buf, "NULL", sizeof(quoted_buf) - 1);
        quoted_buf[sizeof(quoted_buf) - 1] = '\0';
        return quoted_buf;
    }
    snprintf(quoted_buf, sizeof(quoted_buf), "'%s'", s);
    return quoted_buf;
}

// Dummy implementation for gettext_func.
// Simply returns the input string (no translation).
const char *gettext_func(const char *msgid) {
    return msgid;
}

// Dummy implementation for error_func.
// Prints an error message to stderr.
void error_func(int status, int errnum, const char *format, ...) {
    va_list args;
    fprintf(stderr, "Error (status=%d, errno=%d): ", status, errnum);
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
    // In a real GNU error() call, `status` might trigger an exit.
    // For this dummy, we just print.
}

// Dummy implementation for __stack_chk_fail.
// Prints a message and aborts the program.
void __stack_chk_fail(void) {
    fprintf(stderr, "Stack smashing detected! Program aborted.\n");
    abort();
}