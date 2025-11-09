#include <stdio.h>    // For FILE, printf, fprintf, fputs_unlocked, stdout, stderr
#include <stdlib.h>   // For exit, atexit
#include <libintl.h>  // For gettext, bindtextdomain, textdomain
#include <locale.h>   // For setlocale, LC_ALL
#include <unistd.h>   // For link
#include <errno.h>    // For __errno_location, errno

// Dummy declarations for GNU coreutils specific functions/variables
// In a real coreutils build, these would come from various headers like system.h, error.h, quote.h, etc.
extern char *_program_name;
extern void emit_ancillary_info(void *info);
extern void set_program_name(const char *name);
extern void close_stdout(void);
extern const char *proper_name_lite(const char *name1, const char *name2);
extern int optind;
extern void parse_gnu_standard_options_only(int argc, char **argv, void *longopts, const char *package, const char *version, int flags, void (*usage_func)(int), const char *author, int exit_status_if_error);
extern char *quote(const char *str);
extern void error(int status, int errnum, const char *format, ...);
extern char *quotearg_n_style(int n, int style, const char *arg);
extern const char *_Version;
extern void *DAT_001008d1; // Placeholder for the options structure

// Function: usage
void usage(int status) {
  if (status == 0) {
    printf(gettext("Usage: %s FILE1 FILE2\n  or:  %s OPTION\n"), _program_name, _program_name);
    fputs_unlocked(gettext("Call the link function to create a link named FILE2 to an existing FILE1.\n\n"), stdout);
    fputs_unlocked(gettext("      --help        display this help and exit\n"), stdout);
    fputs_unlocked(gettext("      --version     output version information and exit\n"), stdout);
    emit_ancillary_info(&DAT_001008d1);
  } else {
    fprintf(stderr, gettext("Try \'%s --help\' for more information.\n"), _program_name);
  }
  exit(status);
}

// Function: main
int main(int argc, char *argv[]) {
  set_program_name(argv[0]);
  setlocale(LC_ALL, "");
  bindtextdomain("coreutils", "/usr/local/share/locale");
  textdomain("coreutils");
  atexit(close_stdout);

  parse_gnu_standard_options_only(
    argc, argv, &DAT_001008d1, "GNU coreutils", _Version, 1, usage,
    proper_name_lite("Michael Stone", "Michael Stone"), 0
  );

  if (argc <= optind + 1) {
    if (optind < argc) {
      error(0, 0, gettext("missing operand after %s"), quote(argv[optind]));
    } else {
      error(0, 0, gettext("missing operand"));
    }
    usage(1);
  }

  if (optind + 2 < argc) {
    error(0, 0, gettext("extra operand %s"), quote(argv[optind + 2]));
    usage(1);
  }

  if (link(argv[optind], argv[optind + 1]) != 0) {
    error(1, *__errno_location(), gettext("cannot create link %s to %s"),
          quotearg_n_style(0, 4, argv[optind + 1]),
          quotearg_n_style(1, 4, argv[optind]));
  }

  return 0;
}