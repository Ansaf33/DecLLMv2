#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <libintl.h>
#include <getopt.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h> // For pathconf, _PC_NAME_MAX
#include <stdarg.h> // For va_list in stub error/version_etc

// Forward declarations for functions defined later or in other files
// Assuming these are from common coreutils headers
extern const char *_program_name;
extern FILE *_stdout;
extern FILE *_stderr;
extern void emit_ancillary_info(const char *);
extern char *gettext(const char *);
extern int set_program_name(const char *);
extern void close_stdout(void);
extern void error(int, int, const char *, ...);
extern const char *_Version;
extern const char *proper_name_lite(const char *, const char *);
extern void version_etc(FILE *, const char *, const char *, const char *, const char *, ...);
extern const char *quotearg_style(int, const char *);
extern const char *quotearg_n_style(int, int, const char *);
extern const char *quotearg_n_style_mem(int, int, const char *, size_t);
extern const char *quotearg_n_style_colon(int, int, const char *);
extern char *quote(const char *);
extern int mbszero(void *); // Usually takes `mbstate_t *`
extern size_t rpl_mbrlen(const char *, size_t, void *); // Usually takes `mbstate_t *`

// Constants for getopt_long, typical GNU coreutils values
#define PORTABILITY_OPTION_CODE 0x80
#define HELP_OPTION_CODE -0x82
#define VERSION_OPTION_CODE -0x83

// Global variables from coreutils context
// These would typically be defined in system.h or similar, and initialized in main
const char *_program_name = "pathchk";
const char *_Version = "8.32"; // Example version
FILE *_stdout = NULL; // Initialized in main
FILE *_stderr = NULL; // Initialized in main

// Definition of long options for getopt_long
static const struct option longopts[] = {
    {"portability", no_argument, NULL, PORTABILITY_OPTION_CODE},
    {"help", no_argument, NULL, HELP_OPTION_CODE},
    {"version", no_argument, NULL, VERSION_OPTION_CODE},
    {NULL, 0, NULL, 0}
};

// Minimal stubs for external functions if not compiling with coreutils library
#ifndef COREUTILS_BUILD
void emit_ancillary_info(const char *program) { fprintf(_stderr, "Full documentation for %s is available online.\n", program); }
char *gettext(const char *msgid) { return (char *)msgid; }
int set_program_name(const char *name) { _program_name = name; return 0; }
void close_stdout(void) { fflush(_stdout); }
void error(int status, int errnum, const char *message, ...) {
    va_list args;
    fprintf(_stderr, "%s: ", _program_name);
    va_start(args, message);
    vfprintf(_stderr, message, args);
    va_end(args);
    if (errnum) fprintf(_stderr, ": %s", strerror(errnum));
    fprintf(_stderr, "\n");
    if (status) exit(status);
}
const char *proper_name_lite(const char *name, const char *alt) { return name; }
void version_etc(FILE *stream, const char *program_name, const char *package_name,
                 const char *version, const char *copyright_holder, ...) {
    fprintf(stream, "%s (%s) %s\n", program_name, package_name, version);
    fprintf(stream, "Copyright (C) 2020 Free Software Foundation, Inc.\n"); // Example copyright
    fprintf(stream, "License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\n");
    fprintf(stream, "This is free software: you are free to change and redistribute it.\n");
    fprintf(stream, "There is NO WARRANTY, to the extent permitted by law.\n\n");
    fprintf(stream, "Written by ");
    va_list args;
    va_start(args, copyright_holder);
    const char *author = copyright_holder;
    bool first = true;
    while (author) {
        if (!first) fprintf(stream, ", ");
        fprintf(stream, "%s", author);
        first = false;
        author = va_arg(args, const char *);
    }
    va_end(args);
    fprintf(stream, ".\n");
}
const char *quotearg_style(int style, const char *arg) { return arg; } // Simplified
const char *quotearg_n_style(int n, int style, const char *arg) { return arg; } // Simplified
const char *quotearg_n_style_mem(int n, int style, const char *arg, size_t len) { return arg; } // Simplified
const char *quotearg_n_style_colon(int n, int style, const char *arg) { return arg; } // Simplified
char *quote(const char *arg) { return (char*)arg; } // Simplified

// Minimal mbstate_t for mbszero and rpl_mbrlen if needed
typedef struct { int __count; union { unsigned int __wch; char __wchb[4]; } __value; } mbstate_t;
int mbszero(void *ps) { memset(ps, 0, sizeof(mbstate_t)); return 0; }
size_t rpl_mbrlen(const char *s, size_t n, void *ps) {
    if (!s || !*s) return 0;
    // Simplified for ASCII compatibility. Real mbrlen is complex.
    return 1;
}
#endif // COREUTILS_BUILD

// Function: usage
void usage(int status) {
  if (status == 0) {
    printf(gettext("Usage: %s [OPTION]... NAME...\n"), _program_name);
    fputs_unlocked(gettext(
                            "Diagnose invalid or non-portable file names.\n\n  -p                  check for most POSIX systems\n  -P                  check for empty names and leading \"-\"\n      --portability   check for all POSIX systems (equivalent to -p -P)\n"
                            ), _stdout);
    fputs_unlocked(gettext("      --help        display this help and exit\n"), _stdout);
    fputs_unlocked(gettext("      --version     output version information and exit\n"), _stdout);
    emit_ancillary_info("pathchk");
  } else {
    fprintf(_stderr, gettext("Try \'%s --help\' for more information.\n"), _program_name);
  }
  exit(status);
}

// Function: no_leading_hyphen
bool no_leading_hyphen(const char *path) {
  const char *p = path;
  while (true) {
    p = strchr(p, '-');
    if (p == NULL) {
      return true;
    }
    if ((p == path) || (p[-1] == '/')) {
      error(0, 0, gettext("leading \'-\' in a component of file name %s"), quotearg_style(4, path));
      return false;
    }
    p++;
  }
}

// Function: portable_chars_only
bool portable_chars_only(const char *path, size_t path_len) {
  size_t span = strspn(path, "/ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789._-");
  const char *non_portable_char_ptr = path + span;
  
  if (*non_portable_char_ptr == '\0') {
    return true;
  } else {
    mbstate_t ps;
    mbszero(&ps);
    size_t char_len = rpl_mbrlen(non_portable_char_ptr, path_len - span, &ps);
    if (char_len > 16) char_len = 1; // Limit for display
    
    error(0, 0, gettext("non-portable character %s in file name %s"),
          quotearg_n_style_mem(1, 8, non_portable_char_ptr, char_len),
          quotearg_n_style(0, 4, path));
    return false;
  }
}

// Function: component_start
char * component_start(char *path) {
  char *p = path;
  while (*p == '/') {
    p++;
  }
  return p;
}

// Function: component_len
long component_len(const char *component) {
  long len = 0;
  while (component[len] != '/' && component[len] != '\0') {
    len++;
  }
  return len;
}

// Function: validate_file_name
bool validate_file_name(const char *filename_in, bool check_posix, bool check_empty_leading_hyphen) {
  bool result = true;
  char *filename = strdup(filename_in); // Create a mutable copy
  if (!filename) {
      error(EXIT_FAILURE, errno, gettext("memory exhausted"));
  }

  size_t len = strlen(filename);
  bool lstat_succeeded = false;
  struct stat st;

  // 1. Check for leading hyphens
  if (check_empty_leading_hyphen && !no_leading_hyphen(filename)) {
    result = false;
  }

  if (result) {
    // 2. Check for empty name
    if ((!check_posix && !check_empty_leading_hyphen) || (len != 0)) {
      // 3. lstat check (if not check_posix)
      if (!check_posix) {
        if (lstat(filename, &st) == 0) {
          lstat_succeeded = true;
        } else {
          if (*__errno_location() != ENOENT || len == 0) { // ENOENT = 2
            error(0, *__errno_location(), "%s", quotearg_n_style_colon(0, 3, filename));
            result = false;
          }
        }
      } else {
        // 4. Portable characters only (if check_posix)
        if (!portable_chars_only(filename, len)) {
          result = false;
        }
      }
    } else { // len == 0
      error(0, 0, gettext("empty file name"));
      result = false;
    }
  }

  if (result) {
    // 5. Check total filename length limit
    long name_max_limit = -1; // -1 means no specific limit determined or needed
    if (!check_posix) {
      if (!lstat_succeeded && len > 255) { // If file doesn't exist and path is long
        const char *pathconf_target = (*filename == '/') ? "/" : ".";
        *__errno_location() = 0;
        name_max_limit = pathconf(pathconf_target, _PC_NAME_MAX); // 4 is _PC_NAME_MAX
        if (name_max_limit < 0 && *__errno_location() != 0) {
          error(0, *__errno_location(), gettext("%s: unable to determine maximum file name length"), pathconf_target);
          result = false;
        }
      }
    } else {
      name_max_limit = 255; // POSIX NAME_MAX is 255
    }

    if (result && name_max_limit != -1 && name_max_limit < (long)len) {
      error(0, 0, gettext("limit %td exceeded by length %td of file name %s"),
            name_max_limit, len, quotearg_style(4, filename));
      result = false;
    }
  }

  if (result) {
    // 6. Check individual component lengths
    bool check_component_len_limit = check_posix;
    if (!check_posix && !lstat_succeeded) { // If not POSIX check and file doesn't exist
      char *current_component_ptr = component_start(filename);
      while (*current_component_ptr != '\0') {
        long comp_len = component_len(current_component_ptr);
        if (comp_len > 14) { // POSIX component limit is 14
          check_component_len_limit = true;
          break;
        }
        current_component_ptr += comp_len;
        current_component_ptr = component_start(current_component_ptr); // Skip slashes
      }
    }

    if (check_component_len_limit) {
      long component_name_max_val = 14; // Default POSIX _PC_NAME_MAX for components
      
      char *current_component_ptr = filename;
      while (*(current_component_ptr = component_start(current_component_ptr)) != '\0') {
        long current_comp_len = component_len(current_component_ptr);

        if (!check_posix) { // If not full POSIX check, determine limit dynamically
          const char *pathconf_target_for_component = (current_component_ptr == filename) ? "." : filename;

          char saved_char = current_component_ptr[current_comp_len];
          current_component_ptr[current_comp_len] = '\0'; // Temporarily null-terminate component

          *__errno_location() = 0;
          long dynamic_component_name_max = pathconf(pathconf_target_for_component, _PC_NAME_MAX);
          
          current_component_ptr[current_comp_len] = saved_char; // Restore char

          if (dynamic_component_name_max < 0) {
            if (*__errno_location() == 0) {
              component_name_max_val = 0x7fffffffffffffffL; // No limit
            } else if (*__errno_location() != ENOENT) {
              error(0, *__errno_location(), "%s", quotearg_n_style_colon(0, 3, pathconf_target_for_component));
              result = false;
              break;
            } else {
              component_name_max_val = 14; // Default POSIX NAME_MAX for component if ENOENT
            }
          } else {
            component_name_max_val = dynamic_component_name_max;
          }
        }
        
        if (!result) break; // Break if error occurred in pathconf or prior

        if (component_name_max_val != -1 && component_name_max_val < current_comp_len) {
          char saved_char = current_component_ptr[current_comp_len];
          current_component_ptr[current_comp_len] = '\0';
          error(0, 0, gettext("limit %td exceeded by length %td of file name component %s"),
                component_name_max_val, current_comp_len, quote(current_component_ptr));
          current_component_ptr[current_comp_len] = saved_char;
          result = false;
          break;
        }
        current_component_ptr += current_comp_len;
      }
    }
  }

  free(filename); // Free the duplicated string
  return result;
}

// Function: main
int main(int argc, char **argv) {
  _stdout = stdout; // Initialize global FILE pointers
  _stderr = stderr;

  set_program_name(argv[0]);
  setlocale(LC_ALL, "");
  bindtextdomain("coreutils","/usr/local/share/locale");
  textdomain("coreutils");
  atexit(close_stdout);

  bool check_posix = false;
  bool check_empty_leading_hyphen = false;
  int opt;

  while (true) {
    opt = getopt_long(argc, argv, "pP", longopts, NULL);

    if (opt == -1) {
      break; // No more options
    }

    switch (opt) {
      case 'p':
        check_posix = true;
        break;
      case 'P':
        check_empty_leading_hyphen = true;
        break;
      case PORTABILITY_OPTION_CODE: // --portability
        check_posix = true;
        check_empty_leading_hyphen = true;
        break;
      case HELP_OPTION_CODE: // --help
        usage(0);
        // usage(0) calls exit(0)
      case VERSION_OPTION_CODE: // --version
        version_etc(_stdout, _program_name, "GNU coreutils", _Version,
                    proper_name_lite("Paul Eggert", "Paul Eggert"),
                    proper_name_lite("David MacKenzie", "David MacKenzie"),
                    proper_name_lite("Jim Meyering", "Jim Meyering"),
                    (char *)NULL);
        exit(0);
      default: // Unknown option or error
        usage(1);
        // usage(1) calls exit(1)
    }
  }

  if (optind == argc) { // No non-option arguments provided
    error(0, 0, gettext("missing operand"));
    usage(1); // usage(1) calls exit(1)
  }

  bool all_valid = true;
  for (; optind < argc; optind++) {
    if (!validate_file_name(argv[optind], check_posix, check_empty_leading_hyphen)) {
      all_valid = false;
    }
  }

  return all_valid ? EXIT_SUCCESS : EXIT_FAILURE;
}