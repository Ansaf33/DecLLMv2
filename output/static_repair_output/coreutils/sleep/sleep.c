#include <stdio.h>    // For printf, fprintf, FILE
#include <stdlib.h>   // For exit, atexit
#include <libintl.h>  // For gettext, bindtextdomain, textdomain
#include <locale.h>   // For setlocale, LC_ALL
#include <errno.h>    // For errno, __errno_location
#include <stdbool.h>  // For bool
#include <unistd.h>   // For fputs_unlocked (or fputs)

// --- External declarations (assuming these are provided by the GNU coreutils build environment) ---
// These declarations are crucial for the code to compile, as they refer to functions
// and variables typically provided by the GNU coreutils library or glibc.
extern const char *_program_name;
extern FILE *_stdout;
extern FILE *_stderr;
extern void emit_ancillary_info(const char *program_name);
extern void set_program_name(const char *name);
extern void close_stdout(void);
extern const char *proper_name_lite(const char *name1, const char *name2);
// The last argument of parse_gnu_standard_options_only is usually a va_list or NULL
// to indicate the end of author names. Using char* for simplicity.
extern void parse_gnu_standard_options_only(int argc, char *argv[], const char *package, const char *program_name, const char *version, int flags, void (*usage_func)(int), const char *author1, const char *author2, const char *author3);
extern const char *_Version; // Version string
extern void error(int status, int errnum, const char *format, ...);
extern const char *quote(const char *str);
extern double cl_strtod(const char *nptr, char **endptr);
extern double dtimespec_bound(double value, int error_code);
extern int _optind; // From getopt.h, tracks next argument index
extern int xnanosleep(double seconds);
extern int *__errno_location(void); // Standard glibc function
extern void __stack_chk_fail(void); // Stack smashing protection (if compiler-inserted check fails)

// Function: usage
void usage(int param_1) {
  if (param_1 == 0) {
    printf((char *)gettext(
                            "Usage: %s NUMBER[SUFFIX]...\n  or:  %s OPTION\nPause for NUMBER seconds, where NUMBER is an integer or floating-point.\nSUFFIX may be \'s\',\'m\',\'h\', or \'d\', for seconds, minutes, hours, days.\nWith multiple arguments, pause for the sum of their values.\n\n"
                            ), _program_name, _program_name);
    fputs_unlocked((char *)gettext("      --help        display this help and exit\n"), _stdout);
    fputs_unlocked((char *)gettext("      --version     output version information and exit\n"), _stdout);
    emit_ancillary_info("sleep");
  } else {
    fprintf(_stderr, (char *)gettext("Try \'%s --help\' for more information.\n"), _program_name);
  }
  exit(param_1);
}

// Function: apply_suffix (changed return type from undefined8 to int)
int apply_suffix(double *param_1, char param_2) {
  int multiplier;
  
  switch (param_2) {
    case '\0': // No suffix, treated as seconds
    case 's':
      multiplier = 1;
      break;
    case 'm':
      multiplier = 60;
      break;
    case 'h':
      multiplier = 3600; // 0xe10
      break;
    case 'd':
      multiplier = 86400; // 0x15180
      break;
    default:
      return 0; // Unknown suffix
  }
  
  *param_1 = dtimespec_bound(*param_1 * (double)multiplier, 0);
  return 1; // Success
}

// Function: main (changed return type from undefined8 to int, param_2 from undefined8* to char*[])
int main(int argc, char *argv[]) {
  double total_sleep_time = 0.0;
  bool all_intervals_valid = true;
  
  set_program_name(argv[0]);
  setlocale(LC_ALL, "");
  bindtextdomain("coreutils", "/usr/local/share/locale");
  textdomain("coreutils");
  atexit(close_stdout);

  // Calls to proper_name_lite are inlined, and their results passed directly.
  // The original code passed uVar5 then uVar4, which corresponds to Jim Meyering then Paul Eggert.
  parse_gnu_standard_options_only(
    argc, argv, "sleep", "GNU coreutils", _Version, 1, usage,
    proper_name_lite("Jim Meyering", "Jim Meyering"),
    proper_name_lite("Paul Eggert", "Paul Eggert"),
    (char *)0 // End of author list
  );
  
  if (argc == 1) {
    error(0, 0, (char *)gettext("missing operand"));
    usage(1);
  }
  
  // _optind is typically initialized by parse_gnu_standard_options_only or getopt_long
  for (int i = _optind; i < argc; ++i) {
    char *end_ptr;
    *__errno_location() = 0; // Clear errno before cl_strtod
    
    double current_value = cl_strtod(argv[i], &end_ptr);
    // Use the value from cl_strtod and current errno for dtimespec_bound
    double bounded_value = dtimespec_bound(current_value, *__errno_location());
    
    bool interval_error = false;

    if (argv[i] == end_ptr) { // No number was parsed
      interval_error = true;
    } else if (bounded_value < 0.0) { // Negative time
      interval_error = true;
    } else if (*end_ptr != '\0' && end_ptr[1] != '\0') { // Suffix exists and is longer than one character (e.g., "10ss")
      interval_error = true;
    } else if (*end_ptr != '\0') { // A single-character suffix exists, try to apply it
      if (!apply_suffix(&bounded_value, *end_ptr)) {
        interval_error = true; // Suffix was invalid
      }
    }

    if (interval_error) {
      error(0, 0, (char *)gettext("invalid time interval %s"), quote(argv[i]));
      all_intervals_valid = false;
    } else {
      // Accumulate valid sleep times
      total_sleep_time = dtimespec_bound(bounded_value + total_sleep_time, 0);
    }
  }
  
  if (!all_intervals_valid) {
    usage(1); // Exit with error if any interval was invalid
  }
  
  // Perform sleep with the accumulated total time
  if (xnanosleep(total_sleep_time) != 0) {
    // If xnanosleep fails, report error and exit with status 1
    error(1, *__errno_location(), (char *)gettext("cannot read realtime clock"));
  }
  
  // The original code included a manual stack canary check using platform-specific memory access.
  // This is non-portable C, and compilers automatically insert stack protection.
  // The __stack_chk_fail() function is called by the runtime if a compiler-inserted
  // canary check fails. Therefore, the explicit manual check is removed.
  
  return 0;
}