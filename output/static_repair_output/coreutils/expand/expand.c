#define _GNU_SOURCE // For GNU extensions like __ctype_b_loc, fputs_unlocked, etc.

#include <stdio.h>    // FILE, printf, fprintf, fputs_unlocked, getc_unlocked, putchar_unlocked, stdout, stderr, NULL, EOF
#include <stdlib.h>   // exit, atexit
#include <locale.h>   // setlocale, LC_ALL
#include <libintl.h>  // gettext, bindtextdomain, textdomain
#include <getopt.h>   // getopt_long, struct option, optind, optarg (using _optind, _optarg as in snippet)
#include <limits.h>   // For LONG_MAX
#include <stdbool.h>  // For bool type

// --- External Declarations (based on coreutils common patterns) ---
// Global variables used by the snippet. These are typically defined in other coreutils source files.
extern FILE *_stdout;
extern FILE *_stderr;
extern char *_program_name; // Assuming `_program_name` is `char*`
extern char convert_entire_line; // Global flag
extern int _exit_status; // Global exit status
extern const char *_Version; // Global version string
extern const char *shortopts; // For getopt_long
extern const struct option longopts[]; // For getopt_long

// getopt_long variables (standard ones are optind, optarg)
// The snippet uses _optind, _optarg, which might be coreutils specific.
extern int _optind;
extern char *_optarg;

// Stack protector guard variable (GCC specific)
extern long __stack_chk_guard;

// Functions
extern void usage(int status); // Declared first for main to use
extern void set_program_name(const char *name);
extern void close_stdout(void);
extern void emit_stdin_note(void);
extern void emit_mandatory_arg_note(void);
extern void emit_tab_list_info(void);
extern void emit_ancillary_info(const char *program);
extern FILE *next_file(FILE *f);
extern long get_next_tab_column(long current_col, long *prev_col, void *arg); // arg is likely an unused context pointer
extern void write_error(void);
extern const unsigned short **__ctype_b_loc(void); // For _ISBLANK check (GNU extension)
#define _ISBLANK_LOC 1 // Assuming 1 is the bit for ISBLANK from __ctype_b_loc
extern void error(int status, int errnum, const char *message, ...); // GNU error function
extern void finalize_tab_stops(void);
extern void set_file_list(char **files);
extern void cleanup_file_list_stdin(void);
extern const char *proper_name_lite(const char *fname, const char *lname);
extern void version_etc(FILE *stream, const char *command_name, const char *package, const char *version, const char *authors, ...);
extern void parse_tab_stops(const char *s);
extern void __stack_chk_fail(void) __attribute__((noreturn)); // Stack protector failure (GCC specific)

// Function: usage
void usage(int status) {
  if (status == 0) {
    printf(gettext("Usage: %s [OPTION]... [FILE]...\n"), _program_name);
    fputs_unlocked(gettext("Convert tabs in each FILE to spaces, writing to standard output.\n"), _stdout);
    emit_stdin_note();
    emit_mandatory_arg_note();
    fputs_unlocked(gettext(
                            "  -i, --initial    do not convert tabs after non blanks\n  -t, --tabs=N     have tabs N characters apart, not 8\n"
                            ), _stdout);
    emit_tab_list_info();
    fputs_unlocked(gettext("      --help        display this help and exit\n"), _stdout);
    fputs_unlocked(gettext("      --version     output version information and exit\n"), _stdout);
    emit_ancillary_info("expand");
  } else {
    fprintf(_stderr, gettext("Try \'%s --help\' for more information.\n"), _program_name);
  }
  exit(status);
}

// Function: expand
void expand(void) {
  long stack_canary_value = __stack_chk_guard;

  FILE *current_file = next_file(NULL); // Get the first file
  bool keep_processing_files = (current_file != NULL);

  while (keep_processing_files) { // Outer loop: Iterate through files
    bool initial_line_processing = true; // Corresponds to local_55
    long current_column = 0;             // Corresponds to local_48
    long previous_column = 0;            // Corresponds to local_40
    void *dummy_arg_for_tab_column = NULL; // Corresponds to local_56, not used by expand logic

    bool keep_processing_chars_in_file = true;
    while (keep_processing_chars_in_file) { // Inner loop: Iterate through characters in the current file/line
      int c = getc_unlocked(current_file); // Corresponds to local_54

      if (c == EOF) {
        current_file = next_file(current_file); // Try to get the next file
        if (current_file == NULL) {
          keep_processing_files = false; // No more files, exit outer loop
        }
        keep_processing_chars_in_file = false; // Stop processing characters in this (or next) file
        continue; // Go to outer loop condition check (which will exit if no more files)
      }

      if (initial_line_processing) {
        if (c == '\t') {
          long next_tab_col = get_next_tab_column(current_column, &previous_column, dummy_arg_for_tab_column);
          while (current_column < next_tab_col) {
            if (putchar_unlocked(' ') < 0) {
              write_error();
            }
            current_column++;
          }
          c = ' '; // Simulate original behavior where the tab character effectively becomes a space for subsequent checks/printing
        } else if (c == '\b') {
          current_column -= (current_column != 0); // Decrement if not zero
          previous_column -= (previous_column != 0); // Decrement if not zero
        } else {
          // Check for line length overflow (SCARRY8(current_column, 1))
          // For signed long, this typically happens if current_column is LONG_MAX
          if (current_column == LONG_MAX) {
            error(1, 0, gettext("input line is too long"));
          }
          current_column++;
        }

        // Update initial_line_processing based on character type
        const unsigned short *ctype_b = *__ctype_b_loc();
        bool is_blank_char = (ctype_b[c] & _ISBLANK_LOC) != 0;
        if (!convert_entire_line && !is_blank_char) {
          initial_line_processing = false;
        }
      }

      if (putchar_unlocked(c) < 0) {
        write_error();
      }

      if (c == '\n') {
        keep_processing_chars_in_file = false; // End of current line, break inner loop to process next line (or next file)
      }
    } // End of inner while loop for characters
  } // End of outer while loop for files

  if (stack_canary_value != __stack_chk_guard) {
    __stack_chk_fail();
  }
  return;
}

// Function: main
int main(int argc, char **argv) {
  long stack_canary_value = __stack_chk_guard;

  set_program_name(argv[0]);
  setlocale(LC_ALL, ""); // Use LC_ALL for category 6 (LC_MESSAGES) and other locale categories
  bindtextdomain("coreutils", "/usr/local/share/locale");
  textdomain("coreutils");
  atexit(close_stdout);
  convert_entire_line = 1;

  int opt_char; // Renamed iVar1 to opt_char for clarity

  while (true) { // Loop for option parsing
    opt_char = getopt_long(argc, argv, shortopts, longopts, NULL);

    if (opt_char == -1) { // No more options
      break; // Exit option parsing loop
    }

    switch (opt_char) {
      case 't': // -t, --tabs=N
        parse_tab_stops(_optarg);
        break;
      case 'i': // -i, --initial
        convert_entire_line = 0;
        break;
      // Coreutils long options often return negative values defined in an enum.
      // Assuming -0x82 corresponds to --help and -0x83 to --version based on original snippet.
      case -0x82: // --help
        usage(0); // usage(0) prints help and exits with status 0.
        // Code here is unreachable as usage() calls exit().
        break;
      case -0x83: // --version
        version_etc(_stdout, "expand", "GNU coreutils", _Version, proper_name_lite("David MacKenzie", "David MacKenzie"), 0);
        exit(0); // version_etc does not exit itself, so explicit exit is needed.
      default:
        // Handle numeric options '0' through '9' as arguments for parse_tab_stops.
        // This is a custom GNU extension for `expand`'s short options (e.g., `expand -5`).
        if (opt_char >= '0' && opt_char <= '9') {
          if (_optarg == NULL) {
            char opt_str[2] = {(char)opt_char, '\0'}; // Create a null-terminated string from the option character
            parse_tab_stops(opt_str);
          } else {
            // Original code had `_optarg + -1`. This is likely a decompiler artifact.
            // Assuming the intent is to pass `_optarg` directly for the argument provided.
            parse_tab_stops(_optarg);
          }
        } else {
          // Unknown option or getopt_long error (e.g., '?' for unknown option, ':' for missing argument)
          usage(1); // usage(1) prints an error message and exits with status 1.
          // Code here is unreachable as usage() calls exit().
        }
        break;
    }
  }

  // After all options have been parsed
  finalize_tab_stops();

  char **file_list_ptr;
  if (_optind < argc) {
    file_list_ptr = argv + _optind; // Remaining arguments are file names
  } else {
    file_list_ptr = NULL; // No files specified, implying standard input
  }
  set_file_list(file_list_ptr);

  expand(); // Perform the tab expansion
  cleanup_file_list_stdin(); // Clean up file list, specifically stdin if it was used

  if (stack_canary_value != __stack_chk_guard) {
    __stack_chk_fail();
  }
  return _exit_status;
}