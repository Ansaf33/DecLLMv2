#include <stdio.h>    // For printf, fprintf, fputs, stdout, stderr, fflush
#include <stdlib.h>   // For exit, atexit, getenv, environ
#include <string.h>   // For strlen, strchr, strncmp
#include <stdbool.h>  // For bool type
#include <libintl.h>  // For gettext, bindtextdomain, textdomain
#include <locale.h>   // For setlocale, LC_ALL
#include <getopt.h>   // For getopt_long, struct option, optind

// Define _GNU_SOURCE to ensure fputs_unlocked is available on Linux
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

// --- Dummy declarations for coreutils specific functions/globals ---
// In a real coreutils build, these would be linked from other source files.
// For standalone compilation, we provide minimal definitions.

// _program_name: A global variable holding the program's name, typically argv[0].
const char *_program_name = "printenv";

// set_program_name: Sets the global program name.
void set_program_name(const char *name) {
    _program_name = name;
}

// initialize_exit_failure: In coreutils, this sets a global variable for exit status on failure.
// For this snippet, we can just ignore it.
void initialize_exit_failure(int status) {
    (void)status; // Suppress unused parameter warning
}

// close_stdout: In coreutils, this flushes stdout and checks for write errors.
// For this snippet, flushing is sufficient.
void close_stdout(void) {
    fflush(stdout);
}

// emit_ancillary_info: In coreutils, this prints common info like bug report addresses.
void emit_ancillary_info(const char *program) {
    (void)program; // Suppress unused parameter warning
}

// _Version: A global constant for the package version.
const char *_Version = "8.32"; // Dummy version string

// proper_name_lite: In coreutils, this formats author names.
// For simplicity, we just return one of the input names.
const char *proper_name_lite(const char *name1, const char *name2) {
    (void)name2; // Suppress unused parameter warning
    return name1;
}

// version_etc: In coreutils, this prints standard version information.
// Simplified for compilation.
void version_etc(FILE *stream, const char *program_name, const char *package,
                 const char *version, const char *author1, const char *author2, int unused) {
    fprintf(stream, "%s (%s) %s\n", program_name, package, version);
    fprintf(stream, "Copyright (C) 2020 Free Software Foundation, Inc.\n");
    fprintf(stream, "License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\n");
    fprintf(stream, "This is free software: you are free to change and redistribute it.\n");
    fprintf(stream, "There is NO WARRANTY, to the extent permitted by law.\n\n");
    fprintf(stream, "Written by %s, %s.\n", author1, author2);
    (void)unused; // Suppress unused parameter warning
}

// _environ: Global variable from stdlib.h (or unistd.h) containing environment variables.
extern char **environ;

// optind: Global variable from getopt.h, index into argv for next option.
extern int optind;

// --- End of dummy declarations ---


// Option codes for getopt_long to avoid magic numbers
enum {
    HELP_OPTION = -0x82,    // Arbitrary negative value not used by ASCII chars
    VERSION_OPTION = -0x83, // Arbitrary negative value not used by ASCII chars
};

// longopts array for getopt_long
static const struct option longopts[] = {
    {"null", no_argument, NULL, '0'}, // '0' (0x30) as per original code
    {"help", no_argument, NULL, HELP_OPTION},
    {"version", no_argument, NULL, VERSION_OPTION},
    {NULL, 0, NULL, 0} // Sentinel to mark the end of the array
};

// Function: usage
// Prints usage information and exits with the given status.
void usage(int status) {
  if (status == 0) {
    printf(gettext(
        "Usage: %s [OPTION] [VARIABLE]...\nPrint the values of the specified environment VARIABLE(s).\nIf no VARIABLE is specified, print name and value pairs for them all.\n\n"
    ), _program_name);
    fputs_unlocked(gettext("  -0, --null     end each output line with NUL, not newline\n"), stdout);
    fputs_unlocked(gettext("      --help        display this help and exit\n"), stdout);
    fputs_unlocked(gettext("      --version     output version information and exit\n"), stdout);
    printf(gettext(
        "\nYour shell may have its own version of %s, which usually supersedes\nthe version described here.  Please refer to your shell\'s documentation\nfor details about the options it supports.\n"
    ), "printenv");
    emit_ancillary_info("printenv");
  } else {
    fprintf(stderr, gettext("Try \'%s --help\' for more information.\n"), _program_name);
  }
  exit(status);
}

// Function: main
int main(int argc, char *argv[]) {
  bool null_terminated_output = false;
  int opt;
  char output_delimiter;

  set_program_name(argv[0]);
  setlocale(LC_ALL, ""); // Set locale for internationalization
  bindtextdomain("coreutils", "/usr/local/share/locale"); // Bind text domain for translations
  textdomain("coreutils"); // Set current text domain
  initialize_exit_failure(2); // Initialize exit status on failure
  atexit(close_stdout); // Register function to flush stdout on exit

  // Process command-line options
  while ((opt = getopt_long(argc, argv, "+iu:0", longopts, NULL)) != -1) {
    switch (opt) {
      case '0': // Corresponds to --null option
        null_terminated_output = true;
        break;
      case HELP_OPTION: // Corresponds to --help option
        usage(0); // usage(0) prints help and exits with status 0
        break; // Unreachable, but good practice
      case VERSION_OPTION: // Corresponds to --version option
        version_etc(stdout, "printenv", "GNU coreutils", _Version,
                    proper_name_lite("David MacKenzie", "David MacKenzie"),
                    proper_name_lite("Richard Mlynarik", "Richard Mlynarik"), 0);
        exit(0);
      case '?': // getopt_long returns '?' for unknown option or missing argument
      default:
        usage(2); // usage(2) prints error message and exits with status 2
        break; // Unreachable, but good practice
    }
  }

  output_delimiter = null_terminated_output ? '\0' : '\n';

  bool all_args_found_in_env = true;
  if (optind < argc) { // Specific environment variables requested by name
    int num_found = 0;
    for (int i = optind; i < argc; i++) {
      const char *arg_name = argv[i];
      bool found_in_current_arg = false;

      // The original code explicitly checks for '=' and only processes if not found.
      // This means arguments like "VAR=VALUE" are ignored, only "VAR" is processed.
      if (strchr(arg_name, '=') == NULL) { // Only process if argument is just a name
        for (char **env_ptr = environ; *env_ptr != NULL; env_ptr++) {
          const char *env_entry = *env_ptr;
          size_t arg_len = strlen(arg_name);

          // Check if env_entry starts with arg_name followed by '='
          if (strncmp(env_entry, arg_name, arg_len) == 0 && env_entry[arg_len] == '=') {
            printf("%s%c", env_entry, output_delimiter);
            found_in_current_arg = true;
            break; // Found the variable, move to the next command-line argument
          }
        }
        if (found_in_current_arg) {
            num_found++;
        }
      }
    }
    // Determine if all requested variables were found
    all_args_found_in_env = (num_found == (argc - optind));
  } else { // No specific variables, print all environment variables
    for (char **env_ptr = environ; *env_ptr != NULL; env_ptr++) {
      printf("%s%c", *env_ptr, output_delimiter);
    }
    all_args_found_in_env = true; // All are "found" if we print them all
  }

  // Exit status: 0 for success (all found or all printed), 1 for failure (some not found)
  return all_args_found_in_env ? 0 : 1;
}