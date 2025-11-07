#include <stdio.h>    // For printf, fprintf, fputs_unlocked, putchar_unlocked, FILE, stdout, stderr
#include <stdlib.h>   // For exit, atexit, free, strdup
#include <string.h>   // For strlen, strrchr
#include <stdbool.h>  // For bool
#include <locale.h>   // For setlocale, LC_ALL
#include <libintl.h>  // For gettext, bindtextdomain, textdomain
#include <unistd.h>   // For optind, optarg
#include <getopt.h>   // For getopt_long, struct option
#include <stdarg.h>   // For va_list in error and version_etc stubs

// --- Global variables (mimicking decompiled ones) ---
// These would typically be defined elsewhere or initialized directly.
// For compilation, we need to declare them.
char *_program_name = NULL;
const char *_Version = "1.0"; // Placeholder version string

// --- Forward declarations for functions defined later or stubs for coreutils functions ---

// Defined in this snippet
void remove_suffix(char *str, const char *suffix);
void perform_basename(const char *name, const char *suffix, bool zero_terminate);
void usage(int status);

// Stubs for coreutils functions to make the code compilable
// In a real coreutils environment, these would be provided by lib/
void set_program_name(const char *name_arg) {
    _program_name = (char *)name_arg; // Cast to non-const as _program_name is char*
}
void close_stdout(void) {
    // In a real scenario, this would flush and close stdout if necessary.
    // For this example, it's a no-op.
}
char *base_name(const char *path) {
    // A simplified basename implementation for compilation.
    // Real base_name in coreutils is more robust (e.g., handling "//", "/.", etc.)
    char *copy = strdup(path); // Must be freed by caller
    if (!copy) {
        perror("strdup");
        exit(EXIT_FAILURE);
    }
    char *last_slash = strrchr(copy, '/');
    if (last_slash) {
        if (last_slash[1] == '\0' && last_slash != copy) { // Trailing slash, not root
            *last_slash = '\0'; // Temporarily remove trailing slash
            last_slash = strrchr(copy, '/'); // Find previous slash
            if (last_slash) {
                return last_slash + 1;
            } else {
                return copy; // No other slash, return the whole (now shorter) string
            }
        }
        return last_slash + 1;
    }
    return copy;
}
void strip_trailing_slashes(char *path) {
    // Simplified strip_trailing_slashes
    size_t len = strlen(path);
    while (len > 0 && path[len - 1] == '/') {
        len--;
    }
    path[len] = '\0';
}
void error(int status, int errnum, const char *format, ...) {
    (void)errnum; // errnum is unused in this simplified stub
    va_list args;
    va_start(args, format);
    fprintf(stderr, "%s: ", _program_name ? _program_name : "basename");
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);
    if (status != 0) {
        exit(status);
    }
}
const char *quote(const char *arg) {
    // Simple quoting for example. Real quote is more complex.
    return arg; // Just return the arg itself
}
const char *proper_name_lite(const char *name, const char *email) {
    (void)email; // Unused parameter
    return name; // Simply returns the name for this stub
}
// This function in coreutils takes a variable number of authors.
void version_etc(FILE *stream, const char *package, const char *version, ...) {
    fprintf(stream, "%s (%s) %s\n", package, "GNU coreutils", version);
    // For simplicity, just print one author from the original snippet.
    fprintf(stream, "Copyright (C) 2024 Free Software Foundation, Inc.\n");
    fprintf(stream, "License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\n");
    fprintf(stream, "This is free software: you are free to change and redistribute it.\n");
    fprintf(stream, "There is NO WARRANTY, to the extent permitted by law.\n\n");
    fprintf(stream, "Written by David MacKenzie.\n"); // Hardcoding as per original `proper_name_lite` call
}
void emit_mandatory_arg_note(void) {
    fputs_unlocked(gettext("Mandatory arguments to long options are mandatory for short options too.\n"), stdout);
}
void emit_ancillary_info(const char *program) {
    (void)program; // Unused parameter
    fputs_unlocked(gettext("GNU coreutils online help: <https://www.gnu.org/software/coreutils/>\n"), stdout);
    fputs_unlocked(gettext("Full documentation <https://www.gnu.org/software/coreutils/basename>\n"), stdout);
    fputs_unlocked(gettext("or available locally via: info '(coreutils) basename invocation'\n"), stdout);
}

// Global long options array for getopt_long
static const struct option longopts[] = {
    {"multiple", no_argument, NULL, 'a'},
    {"suffix", required_argument, NULL, 's'},
    {"zero", no_argument, NULL, 'z'},
    {"help", no_argument, NULL, -130}, // Corresponds to -0x82
    {"version", no_argument, NULL, -131}, // Corresponds to -0x83
    {NULL, 0, NULL, 0}
};

// Function: usage
void usage(int status) {
  const char *program_name_str = _program_name;
  
  if (status == 0) {
    printf(gettext("Usage: %s NAME [SUFFIX]\n  or:  %s OPTION... NAME...\n"), program_name_str, program_name_str);
    fputs_unlocked(gettext("Print NAME with any leading directory components removed.\nIf specified, also remove a trailing SUFFIX.\n"), stdout);
    emit_mandatory_arg_note();
    fputs_unlocked(gettext("  -a, --multiple       support multiple arguments and treat each as a NAME\n  -s, --suffix=SUFFIX  remove a trailing SUFFIX; implies -a\n  -z, --zero           end each output line with NUL, not newline\n"), stdout);
    fputs_unlocked(gettext("      --help        display this help and exit\n"), stdout);
    fputs_unlocked(gettext("      --version     output version information and exit\n"), stdout);
    printf(gettext("\nExamples:\n  %s /usr/bin/sort          -> \"sort\"\n  %s include/stdio.h .h     -> \"stdio\"\n  %s -s .h include/stdio.h  -> \"stdio\"\n  %s -a any/str1 any/str2   -> \"str1\" followed by \"str2\"\n"), program_name_str, program_name_str, program_name_str, program_name_str);
    emit_ancillary_info("basename");
  } else {
    fprintf(stderr, gettext("Try \'%s --help\' for more information.\n"), program_name_str);
  }
  exit(status);
}

// Function: remove_suffix
void remove_suffix(char *str, const char *suffix) {
  size_t str_len = strlen(str);
  size_t suffix_len = strlen(suffix);
  
  // If the suffix is longer than or equal to the string itself, no removal possible
  if (suffix_len == 0 || suffix_len >= str_len) {
    return;
  }

  // Compare characters from the end of both strings
  char *str_ptr = str + str_len - 1;
  const char *suffix_ptr = suffix + suffix_len - 1;

  while (suffix_ptr >= suffix && str_ptr >= str) {
    if (*str_ptr != *suffix_ptr) {
      return; // Mismatch, suffix not found
    }
    str_ptr--;
    suffix_ptr--;
  }

  // If suffix_ptr is less than suffix, it means the entire suffix matched.
  // str_ptr now points to the character *before* where the suffix started in 'str'.
  // So, str_ptr + 1 is the new end of the string.
  if (suffix_ptr < suffix) {
    *(str_ptr + 1) = '\0';
  }
  return;
}

// Function: perform_basename
void perform_basename(const char *name, const char *suffix_arg, bool zero_terminate) {
  char *base_name_str = base_name(name);
  strip_trailing_slashes(base_name_str);

  if ((suffix_arg != NULL) && (*base_name_str != '/')) {
    remove_suffix(base_name_str, suffix_arg);
  }
  fputs_unlocked(base_name_str, stdout);
  putchar_unlocked(zero_terminate ? '\0' : '\n');
  free(base_name_str);
  return;
}

// Function: main
int main(int argc, char *argv[]) {
  bool multiple_args_allowed = false;
  bool zero_terminate = false;
  const char *suffix_arg = NULL;
  int opt_char;
  
  set_program_name(argv[0]);
  setlocale(LC_ALL, "");
  bindtextdomain("coreutils","/usr/local/share/locale"); // Path may vary
  textdomain("coreutils");
  atexit(close_stdout);

  // Use a while loop for getopt_long, and a switch for options
  while ((opt_char = getopt_long(argc, argv, "+as:z", longopts, NULL)) != -1) {
    switch (opt_char) {
      case 'a':
        multiple_args_allowed = true;
        break;
      case 's':
        suffix_arg = optarg;
        multiple_args_allowed = true; // -s implies -a
        break;
      case 'z':
        zero_terminate = true;
        break;
      case -130: // --help
        usage(0); // This exits
        break;
      case -131: { // --version
        const char *author = proper_name_lite("David MacKenzie","David MacKenzie");
        version_etc(stdout,"basename","GNU coreutils",_Version,author,NULL); // Last NULL indicates end of authors
        exit(0);
      }
      default: // Unknown option or argument error
        usage(1); // This exits
    }
  }

  // After option parsing, handle operands
  if (optind >= argc) {
    error(0, 0, gettext("missing operand"));
    usage(1);
  }

  if (!multiple_args_allowed && (optind + 2 < argc)) {
    error(0, 0, gettext("extra operand %s"), quote(argv[optind + 2]));
    usage(1);
  }

  if (multiple_args_allowed) {
    for (; optind < argc; optind++) {
      perform_basename(argv[optind], suffix_arg, zero_terminate);
    }
  } else {
    // If not multiple args, then only one NAME and optionally one SUFFIX
    const char *name_operand = argv[optind];
    const char *actual_suffix = NULL;
    if (optind + 1 < argc) { // If there's a second operand, it's the suffix
      actual_suffix = argv[optind + 1];
    }
    perform_basename(name_operand, actual_suffix, zero_terminate);
  }
  
  return 0;
}