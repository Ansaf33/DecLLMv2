#include <stdio.h>    // FILE, printf, fputs_unlocked, fprintf, stdout, stderr
#include <stdlib.h>   // exit, atexit, free
#include <locale.h>   // setlocale, LC_ALL
#include <libintl.h>  // gettext, bindtextdomain, textdomain
#include <getopt.h>   // getopt_long, struct option, optarg, optind
#include <sys/stat.h> // mkfifo, umask, mode_t, S_IRUSR, S_IWUSR, S_IRGRP, S_IWGRP, S_IROTH, S_IWOTH
#include <errno.h>    // __errno_location

// --- External declarations from coreutils environment (replace with actual headers if available) ---
extern char *_program_name;
extern void emit_mandatory_arg_note(void);
extern void emit_ancillary_info(const char *);
extern void close_stdout(void);
extern void set_program_name(const char *);
extern void error(int status, int errnum, const char *message, ...);
extern char is_smack_enabled(void);
extern int setfscreatecon(const char *context); // For SELinux context
extern int smack_set_label_for_self(const char *context); // For SMACK context
extern char *quote(const char *arg);
extern char *quotearg_style(int style, const char *arg);
extern void *mode_compile(const char *modestring);
extern mode_t mode_adjust(mode_t current_mode, mode_t new_mode, mode_t umask_val, void *compiled_mode, int type);
extern int lchmod(const char *path, mode_t mode); // Non-POSIX, common on BSD/macOS, assumed available in coreutils env
extern char *_Version;
extern char *proper_name_lite(const char *name1, const char *name2);
extern void version_etc(FILE *stream, const char *package, const char *version, const char *last_author_name, const char *more_authors, ...);
// --- End of external declarations ---


// Function: usage
void usage(int exit_code) {
  if (exit_code == 0) {
    printf(gettext("Usage: %s [OPTION]... NAME...\n"), _program_name);
    fputs_unlocked(gettext("Create named pipes (FIFOs) with the given NAMEs.\n"), stdout);
    emit_mandatory_arg_note();
    fputs_unlocked(gettext("  -m, --mode=MODE    set file permission bits to MODE, not a=rw - umask\n"), stdout);
    fputs_unlocked(gettext("  -Z                   set the SELinux security context to default type\n      --context[=CTX]  like -Z, or if CTX is specified then set the SELinux\n                         or SMACK security context to CTX\n"), stdout);
    fputs_unlocked(gettext("      --help        display this help and exit\n"), stdout);
    fputs_unlocked(gettext("      --version     output version information and exit\n"), stdout);
    emit_ancillary_info("mkfifo");
  } else {
    fprintf(stderr, gettext("Try \'%s --help\' for more information.\n"), _program_name);
  }
  exit(exit_code);
}

// Function: main
int main(int argc, char **argv) {
  char *mode_string = NULL;
  int exit_status = 0;
  char *context_string = NULL; // Stores argument for --context, if SMACK is enabled

  set_program_name(argv[0]);
  setlocale(LC_ALL, "");
  bindtextdomain("coreutils", "/usr/local/share/locale");
  textdomain("coreutils");
  atexit(close_stdout);

  // Define long options. The negative values are arbitrary unique integers
  // to represent long-only options not mapping to a short option.
  static const struct option longopts[] = {
      {"mode", required_argument, NULL, 'm'},
      {"context", optional_argument, NULL, 'Z'},
      {"help", no_argument, NULL, -130}, // -0x82
      {"version", no_argument, NULL, -131}, // -0x83
      {NULL, 0, NULL, 0}
  };
  const char *short_options = "m:Z"; // Corresponds to original DAT_00100e4f

  int opt;
  while ((opt = getopt_long(argc, argv, short_options, longopts, NULL)) != -1) {
    switch (opt) {
      case 'm':
        mode_string = optarg;
        break;
      case 'Z':
        // Original logic means context_string is only set if SMACK is enabled.
        if (!is_smack_enabled()) {
            if (optarg != NULL) { // If --context=CTX was specified
                error(0, 0, gettext("warning: ignoring --context; it requires an SELinux/SMACK-enabled kernel"));
            }
        } else {
            context_string = optarg;
        }
        break;
      case -130: // --help
        usage(0);
        // Original code's goto logic implies that both --help and --version lead to
        // the same version_etc and exit(0) sequence.
        proper_name_lite("David MacKenzie", "David MacKenzie");
        version_etc(stdout, "mkfifo", "GNU coreutils", _Version, proper_name_lite("David MacKenzie", "David MacKenzie"), 0);
        exit(0);
      case -131: // --version
        proper_name_lite("David MacKenzie", "David MacKenzie");
        version_etc(stdout, "mkfifo", "GNU coreutils", _Version, proper_name_lite("David MacKenzie", "David MacKenzie"), 0);
        exit(0);
      case '?': // getopt_long returns '?' for unknown options or missing arguments
      default:
        usage(1);
    }
  }

  // Post-option processing (executed after all options are parsed)
  if (optind == argc) { // No non-option arguments (file names) provided
    error(0, 0, gettext("missing operand"));
    usage(1);
  }

  // Apply security context if specified
  if (context_string != NULL) {
    // As per option parsing logic, context_string is only set if SMACK is enabled.
    // If SMACK is not enabled, context_string remains NULL or a warning is issued.
    // Thus, we can directly call smack_set_label_for_self here.
    if (smack_set_label_for_self(context_string) < 0) {
        error(1, *__errno_location(), gettext("failed to set default file creation context to %s"), quote(context_string));
    }
  }

  mode_t final_mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH; // Default mode 0666 (0x1B6)
  if (mode_string != NULL) {
    void *compiled_mode = mode_compile(mode_string);
    if (compiled_mode == NULL) {
      error(1, 0, gettext("invalid mode"));
    }
    mode_t old_umask = umask(0);
    umask(old_umask); // Restore umask
    final_mode = mode_adjust(final_mode, 0, old_umask, compiled_mode, 0);
    free(compiled_mode);

    // Original check `if (local_b8 < 0x200) goto LAB_001009f3;` implies that
    // if final_mode contains file type bits (e.g., >= 0x200), it's an error.
    // 0x200 is S_IFCHR, for example. Permission bits are less than 0x200.
    if (final_mode >= 0x200) {
      error(1, 0, gettext("mode must specify only file permission bits"));
    }
  }

  // Loop through operands (file names) to create FIFOs
  while (optind < argc) {
    if (mkfifo(argv[optind], final_mode) == 0) {
      // If a mode string was explicitly given, attempt to set permissions with lchmod.
      // mkfifo usually sets the mode, but lchmod might be needed for specific cases
      // or if the initial mode was affected by umask.
      if (mode_string != NULL && lchmod(argv[optind], final_mode) != 0) {
        error(0, *__errno_location(), gettext("cannot set permissions of %s"), quotearg_style(4, argv[optind]));
        exit_status = 1;
      }
    } else {
      error(0, *__errno_location(), gettext("cannot create fifo %s"), quotearg_style(4, argv[optind]));
      exit_status = 1;
    }
    optind++;
  }

  return exit_status;
}