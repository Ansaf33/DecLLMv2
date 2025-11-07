#include <stdio.h>    // For printf, fprintf, fputs_unlocked, FILE, _stdout, _stderr
#include <stdlib.h>   // For exit, atexit
#include <locale.h>   // For setlocale, LC_ALL
#include <libintl.h>  // For gettext, bindtextdomain, textdomain
#include <unistd.h>   // For gethostid
#include <stdint.h>   // For uint32_t

// --- Coreutils specific declarations (minimal for compilation) ---
// These are typically defined in various coreutils headers and linked from libcoreutils.
// For a standalone compilation, we provide extern declarations.

// Global variables
extern char *_program_name;
extern FILE *_stdout;
extern FILE *_stderr;
extern int _optind;
extern const char *_Version;

// Function prototypes
void set_program_name(const char *name);
void error(int status, int errnum, const char *format, ...);
char *quote(const char *arg);
void parse_gnu_standard_options_only(int argc, char **argv, const char *pkgname, const char *product, const char *version, int flags, void (*usage_func)(int), const char *author, int bug_report_address_index);
void emit_ancillary_info(const char *program);
void close_stdout(void);
const char *proper_name_lite(const char *full_name, const char *email);

// --- End of coreutils specific declarations ---

// Function: usage
void usage(int status) {
  if (status == 0) {
    printf(gettext("Usage: %s [OPTION]\nPrint the numeric identifier (in hexadecimal) for the current host.\n\n"), _program_name);
    fputs_unlocked(gettext("      --help        display this help and exit\n"), _stdout);
    fputs_unlocked(gettext("      --version     output version information and exit\n"), _stdout);
    emit_ancillary_info("hostid");
  } else {
    fprintf(_stderr, gettext("Try \'%s --help\' for more information.\n"), _program_name);
  }
  exit(status);
}

// Function: main
int main(int argc, char **argv) {
  set_program_name(argv[0]);
  setlocale(LC_ALL, "");
  bindtextdomain("coreutils", "/usr/local/share/locale");
  textdomain("coreutils");
  atexit(close_stdout);

  parse_gnu_standard_options_only(argc, argv, "hostid", "GNU coreutils", _Version, 1, usage,
                                  proper_name_lite("Jim Meyering", "Jim Meyering"), 0);

  if (_optind < argc) {
    error(0, 0, gettext("extra operand %s"), quote(argv[_optind]));
    usage(1);
  }

  // gethostid returns `long`. The original code masked with 0xffffffff,
  // implying a desire for a 32-bit hexadecimal output.
  // Casting to uint32_t ensures this for printf with "%08x".
  printf("%08x\n", (uint32_t)gethostid());
  return 0;
}