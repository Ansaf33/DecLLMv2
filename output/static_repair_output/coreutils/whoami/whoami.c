#include <stdio.h>    // For printf, fputs_unlocked, fprintf, puts, FILE, stdout, stderr
#include <stdlib.h>   // For exit, atexit
#include <libintl.h>  // For gettext, bindtextdomain, textdomain
#include <locale.h>   // For setlocale, LC_ALL
#include <errno.h>    // For __errno_location
#include <unistd.h>   // For geteuid
#include <pwd.h>      // For getpwuid, struct passwd
#include <sys/types.h> // For uid_t
#include <stdint.h>   // For uintmax_t
#include <stdarg.h>   // For va_list in error function (if variadic)
#include <string.h>   // For strerror in error function (if used by error)

// External declarations for GNU coreutils specific functions/variables.
// These are typically provided by a library like 'libcoreutils' or 'libgnu'.
// They are declared here to satisfy the compiler; actual definitions are linked externally.
extern char *_program_name;
extern FILE *_stdout;
extern FILE *_stderr;
extern const char *_Version;
extern int _optind; // Usually from <getopt.h>

extern void set_program_name(const char *name);
extern void emit_ancillary_info(const char *program);
extern void close_stdout(void);
extern void *proper_name_lite(const char *author_name, const char *author_email);
extern void parse_gnu_standard_options_only(int argc, char **argv, const char *package,
                                            const char *program_name, const char *version,
                                            int flags, void (*usage_func)(int), void *author,
                                            int option_mask);
extern char *quote(const char *str);
extern void error(int status, int errnum, const char *format, ...);

// Function: usage
void usage(int status) {
  if (status == 0) {
    printf(gettext("Usage: %s [OPTION]...\n"), _program_name);
    fputs_unlocked(gettext(
                            "Print the user name associated with the current effective user ID.\nSame as id -un.\n\n"
                            ), _stdout);
    fputs_unlocked(gettext("      --help        display this help and exit\n"), _stdout);
    fputs_unlocked(gettext("      --version     output version information and exit\n"), _stdout);
    emit_ancillary_info("whoami");
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

  parse_gnu_standard_options_only(argc, argv, "whoami", "GNU coreutils", _Version, 1, usage,
                                  proper_name_lite("Richard Mlynarik", "Richard Mlynarik"), 0);

  if (argc != _optind) {
    error(0, 0, gettext("extra operand %s"), quote(argv[_optind]));
    usage(1);
  }

  *__errno_location() = 0; // Clear errno before calling geteuid
  uid_t uid = geteuid();
  struct passwd *pw = NULL; // Initialize passwd pointer to NULL

  // The original logic with goto implies:
  // If geteuid() returns -1 AND errno is non-zero, then pw remains NULL.
  // Otherwise (geteuid() succeeded OR geteuid() returned -1 but errno is 0), call getpwuid.
  if (!(uid == (uid_t)-1 && *__errno_location() != 0)) {
    pw = getpwuid(uid);
  }

  if (pw == NULL) {
    error(1, *__errno_location(), gettext("cannot find name for user ID %ju"), (uintmax_t)uid);
  }
  puts(pw->pw_name);
  return 0;
}