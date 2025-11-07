#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <locale.h>
#include <getopt.h>
#include <stdarg.h> // For va_list in dummy error/fprintf

// Global variables, typically defined elsewhere in coreutils, but defined here for self-contained compilation
int ignore_fail_on_non_empty = 0;
int verbose = 0;
int remove_empty_parents = 0;
const char *_program_name = "rmdir"; // Will be updated in main
FILE *_stdout = NULL; // Will be set to stdout in main
FILE *_stderr = NULL; // Will be set to stderr in main
const char *_Version = "8.32"; // Placeholder version for coreutils

// Standard global for getopt_long (declared in unistd.h or getopt.h)
extern int optind;

// Forward declarations for coreutils specific functions (with dummy implementations for compilation)
// In a real coreutils build, these would be linked from lib/ or src/
void set_program_name(const char *name) { _program_name = name; }
void bindtextdomain(const char *domainname, const char *dirname) { (void)domainname; (void)dirname; }
void textdomain(const char *domainname) { (void)domainname; }
void close_stdout(void) { fflush(stdout); }
const char *quotearg_style(int style, const char *arg) { (void)style; return arg; } // Simple passthrough for compilation
const char *gettext(const char *msgid) { return msgid; } // Passthrough for compilation
int prog_fprintf(FILE *stream, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vfprintf(stream, format, args);
    va_end(args);
    return ret;
}
void error(int status, int errnum, const char *format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, "%s: ", _program_name);
    vfprintf(stderr, format, args);
    if (errnum != 0) {
        fprintf(stderr, ": %s", strerror(errnum));
    }
    fprintf(stderr, "\n");
    va_end(args);
    if (status != 0) exit(status);
}
void emit_ancillary_info(const char *program) { (void)program; }
const char *proper_name_lite(const char *first, const char *last) { (void)first; return last; }
void version_etc(FILE *stream, const char *program, const char *package, const char *version, const char *authors, ...) {
    fprintf(stream, "%s (%s) %s\n", program, package, version);
    fprintf(stream, "Copyright (C) 2023 Free Software Foundation, Inc.\n");
    fprintf(stream, "License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\n");
    fprintf(stream, "This is free software: you are free to change and redistribute it.\n");
    fprintf(stream, "There is NO WARRANTY, to the extent permitted by law.\n\n");
    fprintf(stream, "Written by %s.\n", authors);
}
void strip_trailing_slashes(char *path) {
    if (!path || *path == '\0') return;
    size_t len = strlen(path);
    if (len == 1 && path[0] == '/') return; // Don't strip root
    while (len > 1 && path[len - 1] == '/') {
        path[--len] = '\0';
    }
}
int directory_status(int status_code, const char *path) { (void)status_code; (void)path; return 0; } // Dummy, assuming 0 for success
char *xstrdup(const char *s) {
    char *new_s = strdup(s);
    if (!new_s) {
        error(EXIT_FAILURE, errno, "memory exhausted");
    }
    return new_s;
}
int issymlink(const char *path) {
    struct stat st;
    if (lstat(path, &st) == -1) {
        return -1; // Error
    }
    return S_ISLNK(st.st_mode);
}

// External variable for errno location (standard GNU extension)
extern int *__errno_location(void);

// Global long options for getopt_long
static const struct option longopts[] = {
    {"ignore-fail-on-non-empty", no_argument, NULL, 0x80},
    {"parents", no_argument, NULL, 'p'},
    {"verbose", no_argument, NULL, 'v'},
    {"help", no_argument, NULL, -0x82},
    {"version", no_argument, NULL, -0x83},
    {NULL, 0, NULL, 0}
};

// Short options string for getopt_long
static const char shortopts[] = "pv";

// Function: errno_rmdir_non_empty
int errno_rmdir_non_empty(int errnum) {
  // 0x27 is ENOTEMPTY, 0x11 is EEXIST on many systems
  return (errnum == ENOTEMPTY) || (errnum == EEXIST);
}

// Function: errno_may_be_non_empty
int errno_may_be_non_empty(unsigned int errnum) {
  // This bitmask checks for specific errno values: EPERM (1), EACCES (13), EEXIST (17), EROFS (30)
  return (errnum < 0x1f) && ((0x40012002UL >> (errnum & 0x3f) & 1) != 0);
}

// Function: ignorable_failure
int ignorable_failure(int errnum, const char *path) {
  if (ignore_fail_on_non_empty) {
    if (errno_rmdir_non_empty(errnum)) {
      return 1;
    }
    if (errno_may_be_non_empty(errnum) && directory_status(0xffffff9c, path) == 0) {
      return 1;
    }
  }
  return 0;
}

// Function: remove_parents
bool remove_parents(char *path) {
  bool success = true;
  char *temp_path_buffer = xstrdup(path); // Work on a copy to modify path segment by segment

  strip_trailing_slashes(temp_path_buffer);

  while (true) {
    char *last_slash = strrchr(temp_path_buffer, '/');
    if (last_slash == NULL) {
      break; // No more parent directories
    }

    // Truncate the path to its parent.
    // The original logic finds the character *before* the last component for truncation.
    char *truncate_pos = last_slash;
    while (temp_path_buffer < truncate_pos && *truncate_pos == '/') {
        truncate_pos--;
    }
    truncate_pos[1] = '\0'; // Truncate the path

    if (verbose) {
      prog_fprintf(_stdout, gettext("removing directory, %s"), quotearg_style(4, temp_path_buffer));
    }

    if (rmdir(temp_path_buffer) != 0) {
      int err = *__errno_location();
      if (!ignorable_failure(err, temp_path_buffer)) {
        const char *error_msg;
        if (err == ENOTDIR) { // 20
          error_msg = gettext("failed to remove %s"); // For non-directory or file
        } else {
          error_msg = gettext("failed to remove directory %s"); // For non-empty dir, etc.
        }
        error(0, err, error_msg, quotearg_style(4, temp_path_buffer));
        success = false;
      }
      break; // Stop removing parents on any unignorable failure
    }
  }
  free(temp_path_buffer); // Free the duplicated path buffer
  return success;
}

// Function: usage
void usage(int status) {
  // Ensure _stdout and _stderr are initialized for dummy functions
  if (_stdout == NULL) _stdout = stdout;
  if (_stderr == NULL) _stderr = stderr;

  if (status == EXIT_SUCCESS) {
    printf(gettext("Usage: %s [OPTION]... DIRECTORY...\n"), _program_name);
    fputs_unlocked(gettext("Remove the DIRECTORY(ies), if they are empty.\n\n"), _stdout);
    fputs_unlocked(gettext("      --ignore-fail-on-non-empty\n                    ignore each failure to remove a non-empty directory\n"), _stdout);
    fputs_unlocked(gettext("  -p, --parents     remove DIRECTORY and its ancestors;\n                    e.g., \'rmdir -p a/b\' is similar to \'rmdir a/b a\'\n\n"), _stdout);
    fputs_unlocked(gettext("  -v, --verbose     output a diagnostic for every directory processed\n"), _stdout);
    fputs_unlocked(gettext("      --help        display this help and exit\n"), _stdout);
    fputs_unlocked(gettext("      --version     output version information and exit\n"), _stdout);
    emit_ancillary_info("rmdir");
  } else {
    fprintf(_stderr, gettext("Try \'%s --help\' for more information.\n"), _program_name);
  }
  exit(status);
}

// Function: main
int main(int argc, char *argv[]) {
  // Initialize standard streams and program name
  _stdout = stdout;
  _stderr = stderr;
  set_program_name(argv[0]);

  bool overall_success = true;

  setlocale(LC_ALL, ""); // LC_ALL is 6
  bindtextdomain("coreutils", "/usr/local/share/locale");
  textdomain("coreutils");
  atexit(close_stdout);

  int opt_result;
  while ((opt_result = getopt_long(argc, argv, shortopts, longopts, NULL)) != -1) {
    switch (opt_result) {
      case 0x80: // --ignore-fail-on-non-empty
        ignore_fail_on_non_empty = 1;
        break;
      case 'p': // -p, --parents
        remove_empty_parents = 1;
        break;
      case 'v': // -v, --verbose
        verbose = 1;
        break;
      case -0x82: // --help
        usage(EXIT_SUCCESS); // usage() calls exit
      case -0x83: // --version
        version_etc(_stdout, "rmdir", "GNU coreutils", _Version, proper_name_lite("David MacKenzie", "David MacKenzie"), 0);
        exit(EXIT_SUCCESS);
      default: // Unknown option or argument error
        usage(EXIT_FAILURE); // usage() calls exit
    }
  }

  if (optind == argc) { // No operands provided
    error(0, 0, gettext("missing operand"));
    usage(EXIT_FAILURE);
  }

  // Process arguments
  for (int i = optind; i < argc; ++i) {
    char *path = argv[i]; // Path is mutable as per original logic

    if (verbose) {
      prog_fprintf(_stdout, gettext("removing directory, %s"), quotearg_style(4, path));
    }

    if (rmdir(path) != 0) {
      int err = *__errno_location();
      bool handled_symlink_case = false;

      // Complex symlink handling from original code for ENOTDIR on trailing slash
      if (err == ENOTDIR) { // 0x14
        char *last_slash = strrchr(path, '/');
        // Check if path ends with a slash (or multiple) and is not the root directory
        if (last_slash != NULL && last_slash[1] == '\0' && strcmp(path, "/") != 0) {
          char *temp_path_for_symlink_check = xstrdup(path);
          strip_trailing_slashes(temp_path_for_symlink_check); // Strip trailing slashes on the copy
          if (issymlink(temp_path_for_symlink_check) == 1) { // Check if the stripped path is a symlink
            error(0, 0, gettext("failed to remove %s: Symbolic link not followed"), quotearg_style(4, path));
            handled_symlink_case = true;
          }
          free(temp_path_for_symlink_check);
        }
      }

      if (!ignorable_failure(err, path) && !handled_symlink_case) {
        error(0, err, gettext("failed to remove %s"), quotearg_style(4, path));
        overall_success = false;
      }
    } else { // rmdir successful
      if (remove_empty_parents) {
        // remove_parents modifies the path it receives. Pass a copy to preserve argv[i].
        char *path_copy_for_parents = xstrdup(path);
        if (!remove_parents(path_copy_for_parents)) { // If removing parents failed
          overall_success = false;
        }
        free(path_copy_for_parents);
      }
    }
  }

  return overall_success ? EXIT_SUCCESS : EXIT_FAILURE;
}