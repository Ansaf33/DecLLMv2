#include <stdbool.h> // For bool
#include <time.h>    // For time_t, struct tm, localtime_r, time, strftime, localtime
#include <stdio.h>   // For FILE, stdout, stderr, fprintf, vfprintf, snprintf, printf
#include <string.h>  // For strcpy
#include <stdarg.h>  // For va_list in mock error
#include <stdlib.h>  // For exit

// Define _stdout if it's not simply stdout
#ifndef _stdout
#define _stdout stdout
#endif

// --- Mock/Helper Functions (to make the provided snippet compilable) ---
// In a real project, these would be provided by specific libraries.

// Mock error function: Prints an error message and optionally exits.
void error(int status, int errnum, const char *format, ...) {
    va_list args;
    fprintf(stderr, "Error (errno: %d): ", errnum);
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
    if (status != 0) {
        exit(status);
    }
}

// Mock timetostr function: Converts time_t to a string.
// Assumes buf is large enough (e.g., 24 bytes for "YYYY-MM-DD HH:MM:SS\0").
char *timetostr(time_t t, char *buf) {
    // Using localtime (non-reentrant) for simplicity in this mock.
    // In a production environment, localtime_r or gmtime_r would be preferred.
    struct tm *tm_info = localtime(&t);
    if (tm_info) {
        strftime(buf, 24, "%Y-%m-%d %H:%M:%S", tm_info);
    } else {
        strcpy(buf, "Invalid Time"); // Fallback for error
    }
    return buf;
}

// Mock quote function: Adds single quotes around a string.
// Uses a static buffer, so not thread-safe.
char *quote(const char *s) {
    static char quoted_buf[256]; // Buffer for the quoted string
    snprintf(quoted_buf, sizeof(quoted_buf), "'%s'", s);
    return quoted_buf;
}

// Mock gettext function: A placeholder for internationalization.
// If libintl is available, link against it. Otherwise, it just returns the input string.
#ifdef __GNUC__
#define gettext(msgid) (msgid) // Simple passthrough for compilation
#else
const char *gettext(const char *msgid) {
    return msgid;
}
#endif

// Mock fprintftime function: Formats and prints time to a FILE stream.
// This is a simplified version; a real one would have extensive formatting options.
void fprintftime(FILE *stream, const char *format, const struct tm *tm_info, time_t raw_time, unsigned long flags) {
    char time_str[128]; // Buffer for formatted time string
    if (tm_info) {
        // Use the provided format string, or a default if not specified
        if (format && *format) {
            strftime(time_str, sizeof(time_str), format, tm_info);
        } else {
            strftime(time_str, sizeof(time_str), "%c", tm_info); // Default format
        }
        fprintf(stream, "%s (flags: %lu)\n", time_str, flags);
    } else {
        fprintf(stream, "Failed to format time (raw: %ld, flags: %lu)\n", (long)raw_time, flags);
    }
}

// --- Original Function (refactored) ---
bool show_date(const char *format_str, time_t time_for_error_msg, unsigned long flags, const time_t *time_val_ptr) {
  struct tm tm_buf; // Buffer to store the result of localtime_r
  struct tm *tm_result_ptr; // Pointer to the result (same as &tm_buf on success, NULL on failure)
  char time_str_buf[24]; // Buffer for timetostr

  // Attempt to convert the time_t value pointed to by time_val_ptr into a struct tm.
  // localtime_r is a reentrant version, storing the result in tm_buf.
  tm_result_ptr = localtime_r(time_val_ptr, &tm_buf);

  if (tm_result_ptr == NULL) {
    // If conversion fails, report an error.
    // Use time_for_error_msg (param_2 in original) for the error message.
    error(0, 0, gettext("time %s is out of range"), quote(timetostr(time_for_error_msg, time_str_buf)));
  } else {
    // If conversion succeeds, format and print the date/time.
    // _stdout is defined as stdout.
    // format_str is param_1.
    // &tm_buf is the struct tm result.
    // *time_val_ptr is the raw time_t value (dereferenced param_4).
    // flags is param_3.
    fprintftime(_stdout, format_str, &tm_buf, *time_val_ptr, flags);
  }
  
  // Return true if the time conversion was successful, false otherwise.
  return tm_result_ptr != NULL;
}

// --- Main function for testing ---
int main() {
    time_t now = time(NULL); // Get current time
    time_t invalid_time = -1; // Example of an invalid time_t value for localtime_r

    printf("--- Test Case 1: Valid current time ---\n");
    bool success1 = show_date("%A, %B %d, %Y %H:%M:%S", now, 0, &now);
    printf("Operation successful: %s\n\n", success1 ? "true" : "false");

    printf("--- Test Case 2: Invalid time value ---\n");
    // For the error path, 'invalid_time' is passed as time_for_error_msg and also used for time_val_ptr.
    bool success2 = show_date("%c", invalid_time, 0, &invalid_time);
    printf("Operation successful: %s\n\n", success2 ? "true" : "false");

    // Another valid time example with different flags
    time_t future_time = now + 31536000; // Approximately one year from now
    printf("--- Test Case 3: Valid future time with flags ---\n");
    bool success3 = show_date("Future: %x %X", future_time, 123, &future_time);
    printf("Operation successful: %s\n\n", success3 ? "true" : "false");

    return 0;
}