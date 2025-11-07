#include <stdio.h>    // For printf, fputs_unlocked, fprintf, stdout, stderr, FILE
#include <stdlib.h>   // For exit, atexit
#include <libintl.h>  // For gettext, bindtextdomain, textdomain
#include <locale.h>   // For setlocale, LC_ALL
#include <getopt.h>   // For getopt_long, struct option, optind, optarg
#include <limits.h>   // For ULONG_MAX

// Coreutils specific declarations (assuming these functions and globals are linked from coreutils libraries)
// These are typically global variables or macros provided by coreutils.
extern const char *_program_name; // Global variable for program name, set by set_program_name
extern const char *_Version;      // Global variable for coreutils version string

// Function prototypes for coreutils specific functions
extern void set_program_name(const char *name);
extern void close_stdout(void);
extern const char *quote(const char *str);
extern void error(int status, int errnum, const char *format, ...);
extern unsigned long num_processors(int mode); // mode: 0 for all, 2 for default (online)
extern unsigned long xnumtoumax(const char *str, int base, unsigned long min, unsigned long max, const char *err_format, const char *err_msg, int err_type, int flags);
extern void emit_ancillary_info(const char *program);
extern const char *proper_name_lite(const char *full_name, const char *short_name);
extern void version_etc(FILE *stream, const char *program_name, const char *package, const char *version, const char *authors, ...);


// long options for getopt_long to assign unique integer values
enum {
    ALL_OPTION = 0x80,    // --all
    IGNORE_OPTION = 0x81, // --ignore=N
    HELP_OPTION = 0x82,   // --help
    VERSION_OPTION = 0x83 // --version
};

// Definition of long options array for getopt_long
static struct option const longopts[] = {
    {"all", no_argument, NULL, ALL_OPTION},
    {"ignore", required_argument, NULL, IGNORE_OPTION},
    {"help", no_argument, NULL, HELP_OPTION},
    {"version", no_argument, NULL, VERSION_OPTION},
    {NULL, 0, NULL, 0} // Sentinel to mark the end of the array
};

// Function: usage
void usage(int status) {
  if (status == 0) {
    printf(gettext("Usage: %s [OPTION]...\n"), _program_name);
    fputs_unlocked(gettext(
                            "Print the number of processing units available to the current process,\nwhich may be less than the number of online processors\n\n"
                            ), stdout);
    fputs_unlocked(gettext(
                            "      --all      print the number of installed processors\n      --ignore=N  if possible, exclude N processing units\n"
                            ), stdout);
    fputs_unlocked(gettext("      --help        display this help and exit\n"), stdout);
    fputs_unlocked(gettext("      --version     output version information and exit\n"), stdout);
    emit_ancillary_info("nproc");
  }
  else {
    fprintf(stderr, gettext("Try '%s --help' for more information.\n"), _program_name);
  }
  exit(status); // The original code implies an exit here.
}

// Function: main
int main(int argc, char *argv[]) {
  unsigned long ignore_count = 0;
  // `mode` corresponds to `local_40` in the original snippet:
  // 2 for default (online processors), 0 for --all (installed processors).
  int mode = 2;

  set_program_name(argv[0]);
  setlocale(LC_ALL, ""); // Original category 6 maps to LC_ALL
  bindtextdomain("coreutils", "/usr/local/share/locale");
  textdomain("coreutils");
  atexit(close_stdout);

  int c;
  // Loop through command-line options using getopt_long
  while ((c = getopt_long(argc, argv, "", longopts, NULL)) != -1) {
    switch (c) {
      case ALL_OPTION:
        mode = 0; // Set mode to count all installed processors
        break;
      case IGNORE_OPTION:
        // Parse the argument for --ignore and store it in ignore_count
        ignore_count = xnumtoumax(optarg, 10, 0, ULONG_MAX, NULL, gettext("invalid number"), 0, 2);
        break;
      case HELP_OPTION:
        usage(0); // Display help message and exit with success status
        // usage() calls exit, so code after this is unreachable.
        break;
      case VERSION_OPTION:
        // Display version information and exit with success status
        version_etc(stdout, "nproc", "GNU coreutils", _Version,
                    proper_name_lite("Giuseppe Scrivano", "Giuseppe Scrivano"), NULL);
        exit(0); // exit() is called after printing version info
        // exit() is called, so code after this is unreachable.
        break;
      case '?': // getopt_long automatically prints an error message for unknown options
      default:  // Handles any other unexpected return value from getopt_long
        usage(1); // Display usage information and exit with error status
        // usage() calls exit, so code after this is unreachable.
    }
  }

  // After processing all options, check for any extra non-option arguments
  if (optind < argc) {
    error(0, 0, gettext("extra operand %s"), quote(argv[optind]));
    usage(1); // Report error and exit
  }

  // Determine the number of processing units based on the selected mode
  unsigned long num_units = num_processors(mode);
  unsigned long final_count;

  // Calculate the final count, ensuring it's at least 1
  if (ignore_count < num_units) {
    final_count = num_units - ignore_count;
  } else {
    final_count = 1; // Always report at least one processor if ignored count is too high
  }

  printf("%lu\n", final_count);

  return 0; // Exit successfully
}