#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libintl.h>
#include <locale.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>
#include <unistd.h> // For umask, lchmod, mknod, mkfifo

// --- Dummy Implementations and Declarations for Coreutils Environment ---
// In a real coreutils setup, these would be provided by other modules or system headers.

// External variables
char *_program_name = NULL;
FILE *_stdout = NULL; // Will be initialized to stdout in main
FILE *_stderr = NULL; // Will be initialized to stderr in main
const char *_Version = "8.32"; // Dummy version

// Command-line options structure
struct option longopts[] = {
    {"mode", required_argument, NULL, 'm'},
    {"context", optional_argument, NULL, 'Z'},
    {"help", no_argument, NULL, -0x82}, // Corresponds to -0x82 in original
    {"version", no_argument, NULL, -0x83}, // Corresponds to -0x83 in original
    {NULL, 0, NULL, 0}
};

// Short options string for getopt_long
const char DAT_0010175f[] = "m:Z::";

// Dummy error message for xstrtoumax (often an empty string if error handled elsewhere)
const char DAT_00101224[] = "";

// Dummy format string for mknod/mkfifo error messages
const char DAT_001018dc[] = "%s";

// Function prototypes for dummy implementations
char *set_program_name(char *name);
void close_stdout(void);
char *gettext(const char *msgid);
void emit_mandatory_arg_note(void);
void emit_ancillary_info(const char *program);
char *proper_name_lite(const char *name1, const char *name2);
void version_etc(FILE *stream, const char *package, const char *version, const char *authors, const char *copyright_year, int flags);
char is_smack_enabled(void);
int setfscreatecon(const char *context);
int smack_set_label_for_self(const char *context);
void error(int status, int errnum, const char *format, ...);
char *quote(const char *arg);
char *quotearg_n_style_colon(int n, int style, const char *arg);
char *quotearg_style(int style, const char *arg);
void *mode_compile(const char *s);
mode_t mode_adjust(mode_t mode, mode_t new_umask, mode_t old_umask, void *compiled_mode, bool *p_is_directory);
unsigned long xstrtoumax(const char *str, char **ptr, int str_idx, unsigned long *res, const char *err_msg);
dev_t gnu_dev_makedev(unsigned int major, unsigned int minor);
int rpl_mknod(const char *pathname, mode_t mode, dev_t dev);
void defaultcon(const char *context, const char *path, mode_t mode);
int *__errno_location(void);
void __stack_chk_fail(void);

// Dummy Implementations
char *set_program_name(char *name) { _program_name = name; return name; }
void close_stdout(void) { if (_stdout) fflush(_stdout); }
char *gettext(const char *msgid) { return (char *)msgid; }
void emit_mandatory_arg_note(void) { fputs("Mandatory arguments to long options are mandatory for short options too.\n", _stdout); }
void emit_ancillary_info(const char *program) { printf("\nReport %s bugs to <bug-coreutils@gnu.org>\n", program); }
char *proper_name_lite(const char *name1, const char *name2) { return (char *)name1; }
void version_etc(FILE *stream, const char *package, const char *version, const char *authors, const char *copyright_year, int flags) {
    fprintf(stream, "%s (%s) %s\n", package, version, authors);
    fprintf(stream, "Copyright (C) %s Free Software Foundation, Inc.\n", copyright_year);
    fprintf(stream, "License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\n");
    fprintf(stream, "This is free software: you are free to change and redistribute it.\n");
    fprintf(stream, "There is NO WARRANTY, to the extent permitted by law.\n\n");
}
char is_smack_enabled(void) { return 0; } // Dummy: Assume SMACK is not enabled
int setfscreatecon(const char *context) { return 0; } // Dummy: Assume SELinux context setting works
int smack_set_label_for_self(const char *context) { return 0; } // Dummy
void error(int status, int errnum, const char *format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(_stderr, "%s: ", _program_name ? _program_name : "mknod");
    vfprintf(_stderr, format, args);
    if (errnum) {
        fprintf(_stderr, ": %s", strerror(errnum));
    }
    fputc('\n', _stderr);
    va_end(args);
    if (status) exit(status);
}
char *quote(const char *arg) { return (char *)arg; } // Dummy: Returns the string itself
char *quotearg_n_style_colon(int n, int style, const char *arg) { return (char *)arg; } // Dummy
char *quotearg_style(int style, const char *arg) { return (char *)arg; } // Dummy
void *mode_compile(const char *s) { return (void *)1; } // Dummy: Always succeeds with a non-NULL pointer
mode_t mode_adjust(mode_t mode, mode_t new_umask, mode_t old_umask, void *compiled_mode, bool *p_is_directory) { return mode; } // Dummy
unsigned long xstrtoumax(const char *str, char **ptr, int str_idx, unsigned long *res, const char *err_msg) {
    *res = strtoul(str, ptr, 0); // Base 0 for auto-detection (0x, 0, decimal)
    return 0; // Dummy: Assume success
}
dev_t gnu_dev_makedev(unsigned int major, unsigned int minor) {
    // Standard Linux makedev implementation (or compatible)
    // This is a common implementation, but the exact bit shifts can vary by system/kernel version.
    return (minor & 0xff) | ((major & 0xfff) << 8) | ((minor & ~0xff) << 12) | ((uint64_t)(major & ~0xfff) << 32);
}
int rpl_mknod(const char *pathname, mode_t mode, dev_t dev) { return mknod(pathname, mode, dev); } // Wrapper for mknod
void defaultcon(const char *context, const char *path, mode_t mode) {} // Dummy
int *__errno_location(void) { return &errno; } // Standard glibc function
void __stack_chk_fail(void) { exit(1); } // Dummy

// For fputs_unlocked, which might not be directly available or just a macro for fputs
#ifndef fputs_unlocked
#define fputs_unlocked fputs
#endif

// Function: usage
void usage(int status) {
  if (status == 0) {
    printf(gettext("Usage: %s [OPTION]... NAME TYPE [MAJOR MINOR]\n"), _program_name);
    fputs_unlocked(gettext("Create the special file NAME of the given TYPE.\n"), _stdout);
    emit_mandatory_arg_note();
    fputs_unlocked(gettext("  -m, --mode=MODE    set file permission bits to MODE, not a=rw - umask\n"), _stdout);
    fputs_unlocked(gettext("  -Z                   set the SELinux security context to default type\n      --context[=CTX]  like -Z, or if CTX is specified then set the SELinux\n                         or SMACK security context to CTX\n"), _stdout);
    fputs_unlocked(gettext("      --help        display this help and exit\n"), _stdout);
    fputs_unlocked(gettext("      --version     output version information and exit\n"), _stdout);
    fputs_unlocked(gettext("\nBoth MAJOR and MINOR must be specified when TYPE is b, c, or u, and they\nmust be omitted when TYPE is p.  If MAJOR or MINOR begins with 0x or 0X,\nit is interpreted as hexadecimal; otherwise, if it begins with 0, as octal;\notherwise, as decimal.  TYPE may be:\n"), _stdout);
    fputs_unlocked(gettext("\n  b      create a block (buffered) special file\n  c, u   create a character (unbuffered) special file\n  p      create a FIFO\n"), _stdout);
    printf(gettext("\nYour shell may have its own version of %s, which usually supersedes\nthe version described here.  Please refer to your shell\'s documentation\nfor details about the options it supports.\n"), "mknod");
    emit_ancillary_info("mknod");
  } else {
    fprintf(_stderr, gettext("Try \'%s --help\' for more information.\n"), _program_name);
  }
  exit(status);
}

// Function: main
int main(int argc, char *argv[]) {
  // Initialize _stdout and _stderr to standard streams
  _stdout = stdout;
  _stderr = stderr;

  long stack_guard_value = *(long *)((char *)__builtin_frame_address(0) + 0x28);

  const char *mode_arg = NULL;
  const char *context_arg = NULL;
  mode_t file_mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH; // 0x1b6 is 0666, S_IRWXU | S_IRWXG | S_IRWXO is 0777
  // The original 0x1b6 is 0666 (rw-rw-rw-). Let's use symbolic constants.
  // 0x1b6 = 438 in decimal = 0666 octal.
  file_mode = 0666; // Initial mode: a=rw

  int opt_ret;
  
  set_program_name(argv[0]);
  setlocale(LC_ALL, "");
  bindtextdomain("coreutils", "/usr/local/share/locale");
  textdomain("coreutils");
  atexit(close_stdout);

  while ((opt_ret = getopt_long(argc, argv, DAT_0010175f, longopts, NULL)) != -1) {
    switch (opt_ret) {
      case 'm':
        mode_arg = optarg;
        break;
      case 'Z':
        if (is_smack_enabled()) {
          context_arg = optarg;
        } else {
          if (optarg != NULL) {
            error(0, 0, gettext("warning: ignoring --context; it requires an SELinux/SMACK-enabled kernel"));
          }
        }
        break;
      case -0x83: // --version
        version_etc(_stdout, "mknod", "GNU coreutils", _Version, proper_name_lite("David MacKenzie", "David MacKenzie"), 0);
        exit(0);
      case -0x82: // --help
        usage(0); // usage(0) calls exit(0)
        break;
      case '?': // Unknown option or missing argument for required option
      default: // Should not happen with current longopts, but good practice
        usage(1); // usage(1) calls exit(1)
        break;
    }
  }

  if (mode_arg != NULL) {
    void *compiled_mode = mode_compile(mode_arg);
    if (compiled_mode == NULL) {
      error(1, 0, gettext("invalid mode"));
    }
    mode_t current_umask = umask(0); // Get current umask
    umask(current_umask); // Restore umask
    file_mode = mode_adjust(0666, 0, current_umask, compiled_mode, NULL);
    free(compiled_mode);
    if (S_IRWXU | S_IRWXG | S_IRWXO /*0777*/ < file_mode) { // Check if mode specifies only file permission bits
      error(1, 0, gettext("mode must specify only file permission bits"));
    }
  }

  int min_operands;
  if ((optind < argc) && ((argc <= optind + 1 || (argv[optind + 1][0] != 'p')))) {
    min_operands = 4; // NAME TYPE MAJOR MINOR
  } else {
    min_operands = 2; // NAME TYPE
  }

  int num_operands = argc - optind;

  if (num_operands < min_operands) {
    if (optind < argc) { // At least NAME is present
      error(0, 0, gettext("missing operand after %s"), quote(argv[argc - 1]));
    } else {
      error(0, 0, gettext("missing operand"));
    }
    if ((min_operands == 4) && (num_operands == 2)) {
      fprintf(_stderr, "%s\n", gettext("Special files require major and minor device numbers."));
    }
    usage(1);
  }

  if (num_operands > min_operands) {
    error(0, 0, gettext("extra operand %s"), quote(argv[optind + min_operands]));
    if ((min_operands == 2) && (num_operands == 4)) {
      fprintf(_stderr, "%s\n", gettext("Fifos do not have major and minor device numbers."));
    }
    usage(1);
  }

  if (context_arg != NULL) {
    int setcon_ret;
    if (is_smack_enabled()) {
      setcon_ret = smack_set_label_for_self(context_arg);
    } else {
      setcon_ret = setfscreatecon(context_arg);
    }
    if (setcon_ret < 0) {
      error(1, *__errno_location(), gettext("failed to set default file creation context to %s"), quote(context_arg));
    }
  }

  char *name = argv[optind];
  char type_char = argv[optind + 1][0];
  mode_t type_mode_bits;
  dev_t dev_num = 0;
  int mknod_ret;

  bool is_fifo = false;

  switch (type_char) {
    case 'b':
      type_mode_bits = S_IFBLK;
      break;
    case 'c':
    case 'u': // 'u' falls through to 'c'
      type_mode_bits = S_IFCHR;
      break;
    case 'p':
      type_mode_bits = S_IFIFO;
      is_fifo = true;
      break;
    default:
      error(0, 0, gettext("invalid device type %s"), quote(argv[optind + 1]));
      usage(1); // usage(1) calls exit(1)
  }

  if (is_fifo) {
    if (context_arg != NULL) {
      defaultcon(context_arg, name, S_IFIFO);
    }
    mknod_ret = mkfifo(name, file_mode); // mkfifo includes S_IFIFO implicitly
  } else { // Block or character device
    const char *major_str = argv[optind + 2];
    const char *minor_str = argv[optind + 3];
    unsigned long major_val, minor_val;

    if (xstrtoumax(major_str, NULL, 0, &major_val, DAT_00101224) != 0 || (major_val & ~0U) != major_val) {
      error(1, 0, gettext("invalid major device number %s"), quote(major_str));
    }
    if (xstrtoumax(minor_str, NULL, 0, &minor_val, DAT_00101224) != 0 || (minor_val & ~0U) != minor_val) {
      error(1, 0, gettext("invalid minor device number %s"), quote(minor_str));
    }

    dev_num = gnu_dev_makedev((unsigned int)major_val, (unsigned int)minor_val);
    if (dev_num == (dev_t)-1) {
      error(1, 0, gettext("invalid device %s %s"), major_str, minor_str);
    }

    if (context_arg != NULL) {
      defaultcon(context_arg, name, type_mode_bits);
    }
    mknod_ret = rpl_mknod(name, file_mode | type_mode_bits, dev_num);
  }

  if (mknod_ret != 0) {
    error(1, *__errno_location(), DAT_001018dc, quotearg_n_style_colon(0, 3, name));
  }

  if ((mode_arg != NULL) && (lchmod(name, file_mode) != 0)) {
    error(1, *__errno_location(), gettext("cannot set permissions of %s"), quotearg_style(4, name));
  }

  if (stack_guard_value != *(long *)((char *)__builtin_frame_address(0) + 0x28)) {
    __stack_chk_fail();
  }

  return 0;
}