#include <stdio.h>
#include <stdlib.h>
#include <libintl.h>
#include <locale.h>
#include <unistd.h>
#include <sys/resource.h> // For getpriority, setpriority, PRIO_PROCESS
#include <errno.h>      // For errno, EACCES, EPERM, ENOENT
#include <getopt.h>     // For getopt_long, struct option, optarg, optind
#include <ctype.h>      // For isdigit
#include <string.h>     // For strerror
#include <stdarg.h>     // For va_list

// Global variables (simulating coreutils context)
char *program_name = NULL;
const char *Version = "1.0"; // Placeholder, actual coreutils has a specific version string

// --- Stub Functions (to make the code self-contained and compilable) ---
// In a real coreutils environment, these would be linked from lib/ or other modules.

// Coreutils error function. Exits if status is non-zero.
void error(int status, int errnum, const char *format, ...) {
    va_list args;
    fprintf(stderr, "%s: ", program_name ? program_name : "UNKNOWN");
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    if (errnum) {
        fprintf(stderr, ": %s", strerror(errnum));
    }
    fprintf(stderr, "\n");
    if (status) {
        exit(status);
    }
}

// Coreutils function for robust string to long conversion.
// Returns 0 on success, 1 on partial conversion, 2 on overflow/underflow, 3 on invalid input.
unsigned long xstrtol(const char *s, char **ptr, int base, long *val, const char *err_msg) {
    errno = 0;
    char *endptr;
    long result = strtol(s, &endptr, base);

    if (errno == ERANGE) {
        *val = result; // Store the clamped value
        return 2; // Overflow or underflow
    }
    if (endptr == s) { // No digits found
        return 3; // Invalid input
    }
    if (ptr) {
        *ptr = endptr;
    }
    if (*endptr != '\0') {
        *val = result;
        return 1; // Partial conversion (non-numeric characters after number)
    }
    *val = result;
    return 0; // Success
}

// Other coreutils stub functions
void emit_mandatory_arg_note(void) { /* Impl. would print a note about mandatory arguments. */ }
void emit_exec_status(const char *s) { /* Impl. would print execution status. */ }
void emit_ancillary_info(const char *s) { /* Impl. would print ancillary information. */ }
void close_stdout(void) { /* Impl. would ensure stdout is flushed and closed. */ }
void initialize_exit_failure(int status) { /* Impl. would set the default exit status for failures. */ }
void set_program_name(char *name) { program_name = name; }
char *proper_name_lite(const char *a, const char *b) { return (char *)a; } // Simplified
void version_etc(FILE *fp, const char *pkg, const char *prog, const char *version, const char *authors, int flags) {
    fprintf(fp, "%s %s\n", prog, version);
    if (authors) fprintf(fp, "Copyright (C) ... Authors: %s\n", authors);
    // Real coreutils version_etc prints more detailed info.
}
char *quote(const char *s) { return (char *)s; } // Simplified for compilation

// --- Global Data (simulating coreutils context) ---
// longopts definition (from coreutils nice.c)
static const struct option longopts[] = {
    {"adjustment", required_argument, NULL, 'n'},
    {"help", no_argument, NULL, 0x82}, // Using arbitrary values > 255 for custom options
    {"version", no_argument, NULL, 0x83},
    {NULL, 0, NULL, 0}
};

static const char *PACKAGE_NAME_FOR_USAGE = "nice";
static const char *XSTRTOL_INVALID_CHARS_MSG = "invalid numeric value";
static const char *EXECVP_ERROR_FORMAT_MSG = "%s: %s";

// Function: usage
void usage(int status) {
  if (status == 0) {
    printf(gettext("Usage: %s [OPTION] [COMMAND [ARG]...]\n"), program_name);
    printf(gettext(
                            "Run COMMAND with an adjusted niceness, which affects process scheduling.\nWith no COMMAND, print the current niceness.  Niceness values range from\n%d (most favorable to the process) to %d (least favorable to the process).\n"
                            ), -20, 19);
    emit_mandatory_arg_note();
    fputs_unlocked(gettext("  -n, --adjustment=N   add integer N to the niceness (default 10)\n"), stdout);
    fputs_unlocked(gettext("      --help        display this help and exit\n"), stdout);
    fputs_unlocked(gettext("      --version     output version information and exit\n"), stdout);
    printf(gettext(
                            "\nYour shell may have its own version of %s, which usually supersedes\nthe version described here.  Please refer to your shell\'s documentation\nfor details about the options it supports.\n"
                            ), PACKAGE_NAME_FOR_USAGE);
    emit_exec_status(PACKAGE_NAME_FOR_USAGE);
    emit_ancillary_info(PACKAGE_NAME_FOR_USAGE);
  } else {
    fprintf(stderr, gettext("Try \'%s --help\' for more information.\n"), program_name);
  }
  exit(status);
}

// Function: perm_related_errno
int perm_related_errno(int err_code) {
  return (err_code == EACCES) || (err_code == EPERM);
}

// Function: main
int main(int argc, char **argv) {
  int adjustment_value = 10;
  char *adjustment_str = NULL;
  int arg_idx = 1; // Current argument index
  int exit_status = 0; // Final exit status
  long long_val; // For xstrtol

  set_program_name(argv[0]);
  setlocale(LC_ALL, "");
  bindtextdomain("coreutils","/usr/local/share/locale");
  textdomain("coreutils");
  initialize_exit_failure(EXIT_FAILURE);
  atexit(close_stdout);

  // Option parsing loop
  int opt_char;
  int options_done = 0; // Flag to exit the loop
  while (arg_idx < argc && !options_done) {
    char *current_arg = argv[arg_idx];
    char *numeric_arg_start = NULL;

    // Check for explicit numeric adjustment like -10 or +5
    if (current_arg[0] == '-') {
      int offset = 0;
      if (current_arg[1] == '-' || current_arg[1] == '+') {
        offset = 1;
      }
      if (isdigit((unsigned char)current_arg[offset + 1])) {
        numeric_arg_start = current_arg + offset; // Include the +/- sign
      }
    }
    // If it was an explicit numeric adjustment, handle it and continue
    if (numeric_arg_start) {
      adjustment_str = numeric_arg_start;
      arg_idx++; // Move to next argument
      continue; // Skip getopt_long for this argument and go to next loop iteration
    }

    // Prepare arguments for getopt_long.
    // The original code uses a common coreutils trick: it makes a temporary argv
    // starting from argv[arg_idx - 1] and puts argv[0] into argv[arg_idx - 1].
    // This allows getopt_long to parse options from arg_idx onwards,
    // as if it's a new program invocation.
    char **temp_argv = argv + arg_idx - 1;
    temp_argv[0] = argv[0]; // Temporarily overwrite argv[arg_idx-1] with argv[0]

    optind = 0; // Reset optind for each call to getopt_long
    // getopt_long processes arguments from temp_argv[1] onwards
    opt_char = getopt_long(argc - (arg_idx - 1), temp_argv, "n:", longopts, NULL);

    arg_idx += optind - 1; // Adjust arg_idx based on how many args getopt_long consumed

    switch (opt_char) {
      case 'n':
        adjustment_str = optarg;
        break;
      case 0x82: // --help
        usage(0);
        // usage calls exit, so no need for further action here.
      case 0x83: // --version
        version_etc(stdout, PACKAGE_NAME_FOR_USAGE, "nice", Version, proper_name_lite("David MacKenzie", "David MacKenzie"), 0);
        exit(0);
      case -1: // End of options or non-option argument
        options_done = 1; // Set flag to exit the while loop
        break;
      default: // Unknown option or error
        usage(EXIT_FAILURE);
    }
  }

  if (adjustment_str != NULL) {
    unsigned long strtol_status = xstrtol(adjustment_str, NULL, 10, &long_val, XSTRTOL_INVALID_CHARS_MSG);
    if (strtol_status != 0) { // xstrtol returns 0 on success, >0 on error
      error(EXIT_FAILURE, 0, gettext("invalid adjustment %s"), quote(adjustment_str));
    }
    // Clamp adjustment value between -20 and 19
    if (long_val < -20) {
      adjustment_value = -20;
    } else if (long_val > 19) {
      adjustment_value = 19;
    } else {
      adjustment_value = (int)long_val;
    }
  }

  if (arg_idx == argc) { // No command specified
    if (adjustment_str != NULL) {
      error(0, 0, gettext("a command must be given with an adjustment"));
      usage(EXIT_FAILURE);
    }
    errno = 0;
    int current_niceness = getpriority(PRIO_PROCESS, 0);
    if (current_niceness == -1 && errno != 0) {
      error(EXIT_FAILURE, errno, gettext("cannot get niceness"));
    }
    printf("%d\n", current_niceness);
    exit_status = 0;
  } else { // Command specified
    errno = 0;
    int current_niceness = getpriority(PRIO_PROCESS, 0);
    if (current_niceness == -1 && errno != 0) {
      error(EXIT_FAILURE, errno, gettext("cannot get niceness"));
    }

    int new_niceness = adjustment_value + current_niceness;
    int set_priority_result = setpriority(PRIO_PROCESS, 0, new_niceness);

    if (set_priority_result != 0) {
      int err = errno;
      exit_status = perm_related_errno(err) ? 0 : EXIT_FAILURE; // If permission related, exit 0, else 125
      error(exit_status, err, gettext("cannot set niceness"));
      if (ferror_unlocked(stderr) != 0) { // Check if stderr itself had an error
        exit_status = EXIT_FAILURE;
      }
    }

    // Execute the command
    execvp(argv[arg_idx], &argv[arg_idx]);

    // If execvp returns, it failed
    int err = errno;
    if (err == ENOENT) {
      exit_status = 127; // Command not found
    } else {
      exit_status = 126; // Command invoked cannot execute
    }
    error(0, err, EXECVP_ERROR_FORMAT_MSG, quote(argv[arg_idx]));
  }

  return exit_status;
}