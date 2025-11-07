#define _GNU_SOURCE // For fputs_unlocked, fputc_unlocked if needed, and possibly other GNU extensions

#include <stdio.h>    // FILE, printf, fprintf, fputs_unlocked, fputc_unlocked, _stdout, _stderr
#include <stdlib.h>   // exit, atexit
#include <stdbool.h>  // bool
#include <locale.h>   // setlocale, LC_ALL
#include <libintl.h>  // gettext, bindtextdomain, textdomain
#include <getopt.h>   // getopt_long, optind, optarg, struct option
#include <errno.h>    // errno

// Assuming these are external/custom utility functions/variables from coreutils or libselinux.
// They are not standard C library, so they are declared as extern.

// External variables for coreutils context
extern FILE *_stdout;
extern FILE *_stderr;
extern char *_program_name;
// getopt_long uses global `optind` and `optarg`
// extern int optind; // Declared in getopt.h
// extern char *optarg; // Declared in getopt.h

// External functions
extern void set_program_name(const char *name);
extern void initialize_exit_failure(int status);
extern void close_stdout(void);
extern int getcon(char **context); // From libselinux, sets *context to a newly allocated string
extern void freecon(char *context); // From libselinux, frees context allocated by getcon
extern void error(int status, int errnum, const char *message, ...);
extern void emit_mandatory_arg_note(void);
extern void emit_exec_status(const char *program);
extern void emit_ancillary_info(const char *program);
extern const char *_Version; // Version string for coreutils
extern char *proper_name_lite(const char *name1, const char *name2);
extern void version_etc(FILE *stream, const char *program, const char *package, const char *version, const char *authors, ...);

// Function: usage
void usage(int status) {
  if (status == 0) {
    printf(gettext("Usage: %s CONTEXT COMMAND [args]\n  or:  %s [ -c ] [-u USER] [-r ROLE] [-t TYPE] [-l RANGE] COMMAND [args]\n"),
           _program_name, _program_name);
    fputs_unlocked(gettext("Run a program in a different SELinux security context.\nWith neither CONTEXT nor COMMAND, print the current security context.\n"), _stdout);
    emit_mandatory_arg_note();
    fputs_unlocked(gettext("  CONTEXT            Complete security context\n  -c, --compute      compute process transition context before modifying\n  -t, --type=TYPE    type (for same role as parent)\n  -u, --user=USER    user identity\n  -r, --role=ROLE    role\n  -l, --range=RANGE  levelrange\n"), _stdout);
    fputs_unlocked(gettext("      --help        display this help and exit\n"), _stdout);
    fputs_unlocked(gettext("      --version     output version information and exit\n"), _stdout);
    emit_exec_status("runcon");
    emit_ancillary_info("runcon");
  } else {
    fprintf(_stderr, gettext("Try \'%s --help\' for more information.\n"), _program_name);
  }
  exit(status);
}

// Define long options structure based on the original snippet's behavior
static const struct option long_options[] = {
    {"compute", no_argument, NULL, 'c'},
    {"type", required_argument, NULL, 't'},
    {"user", required_argument, NULL, 'u'},
    {"role", required_argument, NULL, 'r'},
    {"levelrange", required_argument, NULL, 'l'},
    {"help", no_argument, NULL, -130}, // -0x82
    {"version", no_argument, NULL, -131}, // -0x83
    {NULL, 0, NULL, 0}
};

// Function: main
int main(int argc, char *argv[]) {
  bool compute_transition = false;
  char *user = NULL;
  char *role = NULL;
  char *type = NULL;
  char *levelrange = NULL;
  char *context_arg = NULL; // For the CONTEXT argument (e.g., system_u:object_r:exec_t:s0)
  char *current_context = NULL; // For getcon() result

  set_program_name(argv[0]);
  setlocale(LC_ALL, "");
  bindtextdomain("coreutils", "/usr/local/share/locale");
  textdomain("coreutils");
  initialize_exit_failure(125); // 0x7d
  atexit(close_stdout);

  int opt;
  // Loop for parsing command-line options
  while ((opt = getopt_long(argc, argv, "+r:t:u:l:c", long_options, NULL)) != -1) {
    switch (opt) {
      case 'c':
        compute_transition = true;
        break;
      case 'l':
        if (levelrange != NULL) {
          error(125, 0, gettext("multiple levelranges")); // exits
        }
        levelrange = optarg;
        break;
      case 'r':
        if (role != NULL) {
          error(125, 0, gettext("multiple roles")); // exits
        }
        role = optarg;
        break;
      case 't':
        if (type != NULL) {
          error(125, 0, gettext("multiple types")); // exits
        }
        type = optarg;
        break;
      case 'u':
        if (user != NULL) {
          error(125, 0, gettext("multiple users")); // exits
        }
        user = optarg;
        break;
      case -130: // --help
        usage(0); // usage() exits
        break;
      case -131: // --version
        version_etc(_stdout, "runcon", "GNU coreutils", _Version, proper_name_lite("Russell Coker", "Russell Coker"), 0);
        exit(0); // exits
      default: // Unknown option or error from getopt_long
        usage(125); // usage() exits
        break;
    }
  }

  // After option parsing, handle remaining arguments and main logic
  if (argc == optind) { // No arguments after options (i.e., no CONTEXT or COMMAND)
    if (getcon(&current_context) < 0) {
      error(125, errno, gettext("failed to get current context")); // exits
    }
    fputs_unlocked(current_context, _stdout);
    fputc_unlocked('\n', _stdout);
    freecon(current_context); // Free memory allocated by getcon
    return 0;
  } else { // Arguments present after options
    // Check if any context-modifying options were given
    if (!user && !role && !type && !levelrange && !compute_transition) {
      // No specific options (-c, -t, -u, -l, -r) were given,
      // so the first argument must be the complete CONTEXT string.
      if (argc <= optind) { // This case should theoretically be covered by argc == optind, but safety check.
        error(0, 0, gettext("you must specify -c, -t, -u, -l, -r, or context"));
        usage(125); // usage() exits
      }
      context_arg = argv[optind];
      optind++; // Consume the CONTEXT argument
    }

    if (argc <= optind) { // No command specified after context/options
      error(0, 0, gettext("no command specified"));
      usage(125); // usage() exits
    }

    // This is where the actual `runcon` logic would go,
    // using `user`, `role`, `type`, `levelrange`, `compute_transition`, `context_arg`,
    // and `argv[optind]` onwards as the command and its arguments.
    // The original snippet implies that if SELinux is not supported, an error occurs here.
    error(125, 0, gettext("%s may be used only on a SELinux kernel"), _program_name); // exits
  }

  // This point should not be reached as all paths either return or call usage/error which exit.
  return 0;
}