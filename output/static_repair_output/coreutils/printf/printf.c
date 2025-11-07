#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <libintl.h> // For gettext, bindtextdomain, textdomain
#include <errno.h>   // For __errno_location
#include <ctype.h>   // For isdigit, isxdigit (using c_isdigit for locale-aware version)
#include <stdbool.h> // For bool
#include <stdint.h>  // For intmax_t, uintmax_t, uintptr_t

// Forward declarations for coreutils specific functions/globals
extern char *_program_name; // Assuming char* based on usage
extern FILE *_stdout;
extern FILE *_stderr;
extern int exit_status;
extern bool posixly_correct;
extern const char *_Version; // Assuming this is a const char*

// Coreutils specific utility functions - assuming these signatures
extern void emit_ancillary_info(const char *program);
extern void error(int status, int errnum, const char *format, ...);
extern const char *quote(const char *str);
extern int c_isxdigit(int c); // Assuming it's similar to isxdigit but might be locale-aware
extern int c_isdigit(int c);  // Assuming it's similar to isdigit but might be locale-aware
extern void print_unicode_char(FILE *stream, unsigned int unicode_char, int flags);
extern long double vstrtold(const char *str); // Placeholder, actual signature might differ
extern uintmax_t vstrtoumax(const char *str); // Placeholder
extern intmax_t vstrtoimax(const char *str); // Placeholder
extern void xprintf(const char *format, ...); // Placeholder for a printf variant
extern void *xmalloc(size_t size); // Placeholder for malloc that exits on failure
extern void close_stdout(void);
extern const char *proper_name_lite(const char *name1, const char *name2);
extern void version_etc(FILE *stream, const char *program, const char *package, const char *version, const char *authors, ...);
extern char *quotearg_style(int style, const char *arg);
extern int streq(const char *s1, const char *s2); // Assuming a string equality function
extern void __stack_chk_fail(void); // For stack canary check

// Data constants (from disassembly)
extern const char DAT_00102b2d[]; // Likely an error message format string
extern const char DAT_00102e4f[]; // Likely "--" string

// Global state for argument indexing in printf-like functions
static int current_arg_index_global = 0;
static int prev_arg_index_global = -1;
static int max_arg_index_global = -1;

// Function: usage
void usage(int exit_code) {
  if (exit_code == 0) {
    printf(gettext("Usage: %s FORMAT [ARGUMENT]...\n  or:  %s OPTION\n"), _program_name, _program_name);
    fputs_unlocked(gettext("Print ARGUMENT(s) according to FORMAT, or execute according to OPTION:\n\n"), _stdout);
    fputs_unlocked(gettext("      --help        display this help and exit\n"), _stdout);
    fputs_unlocked(gettext("      --version     output version information and exit\n"), _stdout);
    fputs_unlocked(gettext("\nFORMAT controls the output as in C printf.  Interpreted sequences are:\n\n  \\\"      double quote\n"), _stdout);
    fputs_unlocked(gettext("  \\\\      backslash\n  \\a      alert (BEL)\n  \\b      backspace\n  \\c      produce no further output\n  \\e      escape\n  \\f      form feed\n  \\n      new line\n  \\r      carriage return\n  \\t      horizontal tab\n  \\v      vertical tab\n"), _stdout);
    fputs_unlocked(gettext("  \\NNN    byte with octal value NNN (1 to 3 digits)\n  \\xHH    byte with hexadecimal value HH (1 to 2 digits)\n  \\uHHHH  Unicode (ISO/IEC 10646) character with hex value HHHH (4 digits)\n  \\UHHHHHHHH  Unicode character with hex value HHHHHHHH (8 digits)\n"), _stdout);
    fputs_unlocked(gettext("  %%      a single %\n  %b      ARGUMENT as a string with \'\\\' escapes interpreted,\n          except that octal escapes should have a leading 0 like \\0NNN\n  %q      ARGUMENT is printed in a format that can be reused as shell input,\n          escaping non-printable characters with the POSIX $\'\' syntax\n\nand all C format specifications ending with one of diouxXfeEgGcs, with\nARGUMENTs converted to proper type first.  Variable widths are handled.\n"), _stdout);
    printf(gettext("\nYour shell may have its own version of %s, which usually supersedes\nthe version described here.  Please refer to your shell\'s documentation\nfor details about the options it supports.\n"), "printf");
    emit_ancillary_info("printf");
  } else {
    fprintf(_stderr, gettext("Try \'%s --help\' for more information.\n"), _program_name);
  }
  exit(exit_code);
}

// Function: verify_numeric
void verify_numeric(const char *arg_str, const char *end_ptr) {
  if (arg_str == end_ptr) {
    error(0, 0, gettext("%s: expected a numeric value"), quote(arg_str));
    exit_status = 1;
  } else {
    int *errno_loc = __errno_location();
    if (*errno_loc == 0) {
      if (*end_ptr != '\0') {
        error(0, 0, gettext("%s: value not completely converted"), quote(arg_str));
        exit_status = 1;
      }
    } else {
      error(0, *errno_loc, DAT_00102b2d, quote(arg_str));
      exit_status = 1;
    }
  }
}

// Function: print_esc_char
void print_esc_char(char c) {
  switch(c) {
  case 'a': putchar_unlocked(7); break;
  case 'b': putchar_unlocked(8); break;
  case 'c': exit(0);
  case 'e': putchar_unlocked(0x1b); break;
  case 'f': putchar_unlocked(0xc); break;
  case 'n': putchar_unlocked(10); break;
  case 'r': putchar_unlocked(0xd); break;
  case 't': putchar_unlocked(9); break;
  case 'v': putchar_unlocked(0xb); break;
  default:  putchar_unlocked((int)c); break;
  }
}

// Function: print_esc
int print_esc(const char *format_ptr, char is_b_format) {
  const char *current_ptr = format_ptr + 1;
  int parsed_char_val = 0;
  int char_count;
  unsigned int unicode_val;

  if (*current_ptr == 'x') {
    current_ptr++; // Skip 'x'
    for (char_count = 0; char_count < 2; char_count++) {
      if (!c_isxdigit((int)*current_ptr)) break;
      int digit_val;
      if (*current_ptr >= '0' && *current_ptr <= '9') digit_val = *current_ptr - '0';
      else if (*current_ptr >= 'a' && *current_ptr <= 'f') digit_val = *current_ptr - 'a' + 10;
      else digit_val = *current_ptr - 'A' + 10;
      parsed_char_val = parsed_char_val * 16 + digit_val;
      current_ptr++;
    }
    if (char_count == 0) {
      error(1, 0, gettext("missing hexadecimal number in escape"));
    }
    putchar_unlocked(parsed_char_val);
  } else if (*current_ptr < '0' || *current_ptr > '7') {
    if (*current_ptr != '\0') {
      if (strchr("\"\\abcefnrtv", (int)*current_ptr) != NULL) {
        print_esc_char(*current_ptr);
        current_ptr++;
      } else if (*current_ptr == 'u' || *current_ptr == 'U') {
        char unicode_type = *current_ptr;
        int hex_digits_expected = (unicode_type == 'u') ? 4 : 8;
        current_ptr++; // Skip 'u' or 'U'
        unicode_val = 0;
        for (char_count = 0; char_count < hex_digits_expected; char_count++) {
          if (!c_isxdigit((int)*current_ptr)) {
            error(1, 0, gettext("missing hexadecimal number in escape"));
          }
          int digit_val;
          if (*current_ptr >= '0' && *current_ptr <= '9') digit_val = *current_ptr - '0';
          else if (*current_ptr >= 'a' && *current_ptr <= 'f') digit_val = *current_ptr - 'a' + 10;
          else digit_val = *current_ptr - 'A' + 10;
          unicode_val = unicode_val * 16 + digit_val;
          current_ptr++;
        }
        if ((unicode_val > 0xD7FF && unicode_val < 0xE000)) { // Surrogate range check
          error(1, 0, gettext("invalid universal character name \\%c%0*x"), unicode_type, hex_digits_expected, unicode_val);
        }
        print_unicode_char(_stdout, unicode_val, 0);
      } else {
        putchar_unlocked('\\');
        putchar_unlocked((int)*current_ptr);
        current_ptr++;
      }
    } else { // Backslash at end of string
        putchar_unlocked('\\');
    }
  } else { // Octal escape
    if (is_b_format && *current_ptr == '0') {
        current_ptr++; // Skip leading '0' for %b mode octal
    }
    for (char_count = 0; char_count < 3 && (*current_ptr >= '0' && *current_ptr <= '7'); char_count++) {
      parsed_char_val = parsed_char_val * 8 + (*current_ptr - '0');
      current_ptr++;
    }
    putchar_unlocked(parsed_char_val);
  }
  return (int)(current_ptr - format_ptr);
}

// Function: print_esc_string
void print_esc_string(const char *s) {
  for (; *s != '\0'; s++) {
    if (*s == '\\') {
      int chars_consumed = print_esc(s, 1); // 1 for %b format
      s += (chars_consumed - 1); // Adjust s to point to the last char consumed
    } else {
      putchar_unlocked((int)*s);
    }
  }
}

// Function: print_direc
void print_direc(const char *format_prefix, char conversion_char, bool has_width, int width, bool has_precision, int precision, const char *arg_value) {
  const char *length_modifier = NULL;
  size_t modifier_len = 0;

  unsigned int char_val = (unsigned char)conversion_char; // Ensure char promotes correctly
  if (char_val >= 'A' && char_val <= 'Z') {
    unsigned long mask = 1UL << (char_val - 'A');
    if ((mask & 0x7100000071ULL) != 0) { // L (e.g. A, E, F, G)
      length_modifier = "L";
      modifier_len = 1;
    } else if ((mask & 0x90410800800000ULL) != 0) { // j (e.g. X)
      length_modifier = "j";
      modifier_len = 1;
    }
  }

  size_t format_len = strlen(format_prefix);
  char *full_format = xmalloc(format_len + modifier_len + 2); // +1 for conversion_char, +1 for null
  memcpy(full_format, format_prefix, format_len);
  if (length_modifier) {
    memcpy(full_format + format_len, length_modifier, modifier_len);
  }
  full_format[format_len + modifier_len] = conversion_char;
  full_format[format_len + modifier_len + 1] = '\0';

  // Prepare argument for vstrto functions, defaulting to "0" or "" for NULL
  const char *safe_arg_value = arg_value ? arg_value : ((conversion_char == 's') ? "" : "0");

  // Call xprintf based on flags and conversion type
  if (has_width && has_precision) {
    switch(conversion_char) {
    case 'A': case 'E': case 'F': case 'G': case 'a': case 'e': case 'f': case 'g':
      xprintf(full_format, width, precision, vstrtold(safe_arg_value)); break;
    case 'X': case 'o': case 'u': case 'x':
      xprintf(full_format, width, precision, vstrtoumax(safe_arg_value)); break;
    case 'c':
      xprintf(full_format, width, precision, safe_arg_value[0]); break;
    case 'd': case 'i':
      xprintf(full_format, width, precision, vstrtoimax(safe_arg_value)); break;
    case 's':
      xprintf(full_format, width, precision, safe_arg_value); break;
    }
  } else if (has_width) {
    switch(conversion_char) {
    case 'A': case 'E': case 'F': case 'G': case 'a': case 'e': case 'f': case 'g':
      xprintf(full_format, width, vstrtold(safe_arg_value)); break;
    case 'X': case 'o': case 'u': case 'x':
      xprintf(full_format, width, vstrtoumax(safe_arg_value)); break;
    case 'c':
      xprintf(full_format, width, safe_arg_value[0]); break;
    case 'd': case 'i':
      xprintf(full_format, width, vstrtoimax(safe_arg_value)); break;
    case 's':
      xprintf(full_format, width, safe_arg_value); break;
    }
  } else if (has_precision) {
    switch(conversion_char) {
    case 'A': case 'E': case 'F': case 'G': case 'a': case 'e': case 'f': case 'g':
      xprintf(full_format, precision, vstrtold(safe_arg_value)); break;
    case 'X': case 'o': case 'u': case 'x':
      xprintf(full_format, precision, vstrtoumax(safe_arg_value)); break;
    case 'c':
      xprintf(full_format, precision, safe_arg_value[0]); break;
    case 'd': case 'i':
      xprintf(full_format, precision, vstrtoimax(safe_arg_value)); break;
    case 's':
      xprintf(full_format, precision, safe_arg_value); break;
    }
  } else {
    switch(conversion_char) {
    case 'A': case 'E': case 'F': case 'G': case 'a': case 'e': case 'f': case 'g':
      xprintf(full_format, vstrtold(safe_arg_value)); break;
    case 'X': case 'o': case 'u': case 'x':
      xprintf(full_format, vstrtoumax(safe_arg_value)); break;
    case 'c':
      xprintf(full_format, safe_arg_value[0]); break;
    case 'd': case 'i':
      xprintf(full_format, vstrtoimax(safe_arg_value)); break;
    case 's':
      xprintf(full_format, safe_arg_value); break;
    }
  }
  free(full_format);
}

// Function: get_curr_arg
// This function parses N$ specifiers and updates argument indices.
// It uses a `long long*` array (passed as `char**` to match original signature)
// to return the updated format string pointer and argument indices.
// `param_1_ptr_to_array[0]` stores `char *` (updated format string pointer).
// `param_1_ptr_to_array[1]` stores `long long` (argument index to use, 0-indexed).
// `param_1_ptr_to_array[2]` stores `long long` (previous argument index for state).
char **get_curr_arg(char **param_1_ptr_to_array, int mode) {
  long long *arg_info_array = (long long *)param_1_ptr_to_array;

  char *format_str_in = (char *)(uintptr_t)arg_info_array[0];
  char *current_pos = format_str_in;
  int parsed_arg_num = 0;
  bool overflow = false;

  if (mode < 3 && c_isdigit((int)*current_pos)) {
    parsed_arg_num = *current_pos - '0';
    current_pos++;
    while (c_isdigit((int)*current_pos)) {
      long long temp_parsed_arg_num = (long long)parsed_arg_num * 10 + (*current_pos - '0');
      if (temp_parsed_arg_num > 0x7fffffff) { // Check for 32-bit int overflow
          overflow = true;
      }
      parsed_arg_num = (int)temp_parsed_arg_num;
      current_pos++;
    }
    if (*current_pos == '$') {
      if (overflow) parsed_arg_num = 0x7fffffff;
      current_pos++; // Skip '$'
    } else { // Not an N$ specifier
        parsed_arg_num = 0;
        current_pos = format_str_in; // Revert
    }
  }

  int arg_index_to_use;
  if (parsed_arg_num < 1) { // No N$ or N$ parsing failed
    if (mode == 0) { // General conversion specifier
      prev_arg_index_global = -1;
      arg_index_to_use = -1;
    } else if (mode < 3 || prev_arg_index_global < 0) { // Width/precision '*' or sequential
      current_arg_index_global++;
      arg_index_to_use = current_arg_index_global;
    } else { // Use previous N$ index
      arg_index_to_use = prev_arg_index_global;
    }
  } else { // N$ specifier found
    arg_index_to_use = parsed_arg_num - 1; // Convert to 0-indexed
    if (mode == 0) { // Only update prev_arg_index_global for primary specifiers
      prev_arg_index_global = arg_index_to_use;
    }
  }

  if (arg_index_to_use > max_arg_index_global) {
    max_arg_index_global = arg_index_to_use;
  }

  arg_info_array[0] = (long long)(uintptr_t)current_pos;
  arg_info_array[1] = (long long)arg_index_to_use;
  arg_info_array[2] = (long long)prev_arg_index_global;

  return param_1_ptr_to_array;
}

// Function: print_formatted
int print_formatted(const char *format_str, int arg_count, const char **args_array) {
  long long arg_parse_results[3]; // [0]=format_ptr, [1]=arg_idx, [2]=prev_arg_idx

  const char *current_format_ptr = format_str;
  
  char *temp_format_segment = xmalloc(strlen(format_str) + 1);

  bool valid_conversion_specifiers[256] = {false};
  valid_conversion_specifiers['d'] = true; valid_conversion_specifiers['i'] = true;
  valid_conversion_specifiers['o'] = true; valid_conversion_specifiers['u'] = true;
  valid_conversion_specifiers['x'] = true; valid_conversion_specifiers['X'] = true;
  valid_conversion_specifiers['f'] = true; valid_conversion_specifiers['F'] = true;
  valid_conversion_specifiers['e'] = true; valid_conversion_specifiers['E'] = true;
  valid_conversion_specifiers['g'] = true; valid_conversion_specifiers['G'] = true;
  valid_conversion_specifiers['a'] = true; valid_conversion_specifiers['A'] = true;
  valid_conversion_specifiers['c'] = true; valid_conversion_specifiers['s'] = true;
  valid_conversion_specifiers['p'] = true; valid_conversion_specifiers['n'] = true;
  valid_conversion_specifiers['b'] = true; // GNU extension
  valid_conversion_specifiers['q'] = true; // GNU extension

  while (*current_format_ptr != '\0') {
    if (*current_format_ptr == '%') {
      const char *specifier_start = current_format_ptr;
      char *temp_segment_ptr = temp_format_segment + 1;
      *temp_format_segment = '%';

      current_format_ptr++; // Skip '%'

      if (*current_format_ptr == '%') {
        putchar_unlocked('%');
        current_format_ptr++;
        continue;
      }

      // Prepare for get_curr_arg: pass current format ptr
      arg_parse_results[0] = (long long)(uintptr_t)current_format_ptr;
      get_curr_arg((char **)arg_parse_results, 0); // Mode 0 for general arg parsing
      current_format_ptr = (char *)(uintptr_t)arg_parse_results[0]; // Get updated format ptr
      int arg_index_to_fetch = (int)arg_parse_results[1]; // Get argument index

      // Parse flags
      while (true) {
        switch(*current_format_ptr) {
        case ' ': case '+': case '-': case '#': case '\'': case 'I': case '0':
          *temp_segment_ptr++ = *current_format_ptr++;
          break;
        default: goto end_flag_parsing;
        }
      }
      end_flag_parsing:;

      // Special handling for %b and %q
      if (*current_format_ptr == 'b') {
        *temp_segment_ptr++ = *current_format_ptr++;
        *temp_segment_ptr = '\0';
        arg_parse_results[0] = (long long)(uintptr_t)current_format_ptr;
        get_curr_arg((char **)arg_parse_results, 3); // Mode 3 for %b
        current_format_ptr = (char *)(uintptr_t)arg_parse_results[0];
        arg_index_to_fetch = (int)arg_parse_results[1];

        if (arg_index_to_fetch < arg_count) {
          print_esc_string(args_array[arg_index_to_fetch]);
        }
        continue;
      }

      if (*current_format_ptr == 'q') {
        *temp_segment_ptr++ = *current_format_ptr++;
        *temp_segment_ptr = '\0';
        arg_parse_results[0] = (long long)(uintptr_t)current_format_ptr;
        get_curr_arg((char **)arg_parse_results, 3); // Mode 3 for %q
        current_format_ptr = (char *)(uintptr_t)arg_parse_results[0];
        arg_index_to_fetch = (int)arg_parse_results[1];

        if (arg_index_to_fetch < arg_count) {
          fputs_unlocked(quotearg_style(3, args_array[arg_index_to_fetch]), _stdout);
        }
        continue;
      }

      // Parse width
      int width = 0;
      bool has_width = false;
      if (*current_format_ptr == '*') {
        *temp_segment_ptr++ = *current_format_ptr++;
        arg_parse_results[0] = (long long)(uintptr_t)current_format_ptr;
        get_curr_arg((char **)arg_parse_results, 1); // Mode 1 for width '*'
        current_format_ptr = (char *)(uintptr_t)arg_parse_results[0];
        int width_arg_idx = (int)arg_parse_results[1];

        if (width_arg_idx < arg_count) {
          long long val = vstrtoimax(args_array[width_arg_idx]);
          if (val < -0x80000000LL || val > 0x7fffffffLL) {
            error(1, 0, gettext("invalid field width: %s"), quote(args_array[width_arg_idx]));
          }
          width = (int)val;
        }
        has_width = true;
      } else {
        while (isdigit((int)*current_format_ptr)) {
          *temp_segment_ptr++ = *current_format_ptr++;
        }
      }

      // Parse precision
      int precision = -1;
      bool has_precision = false;
      if (*current_format_ptr == '.') {
        *temp_segment_ptr++ = *current_format_ptr++;
        if (*current_format_ptr == '*') {
          *temp_segment_ptr++ = *current_format_ptr++;
          arg_parse_results[0] = (long long)(uintptr_t)current_format_ptr;
          get_curr_arg((char **)arg_parse_results, 2); // Mode 2 for precision '*'
          current_format_ptr = (char *)(uintptr_t)arg_parse_results[0];
          int precision_arg_idx = (int)arg_parse_results[1];

          if (precision_arg_idx < arg_count) {
            long long val = vstrtoimax(args_array[precision_arg_idx]);
            if (val < 0) { precision = -1; }
            else {
              if (val > 0x7fffffffLL) {
                error(1, 0, gettext("invalid precision: %s"), quote(args_array[precision_arg_idx]));
              }
              precision = (int)val;
            }
          }
          has_precision = true;
        } else {
          precision = 0; // Default to 0 if '.' but no number/star
          while (isdigit((int)*current_format_ptr)) {
            precision = precision * 10 + (*current_format_ptr - '0');
            *temp_segment_ptr++ = *current_format_ptr++;
            has_precision = true;
          }
        }
      }

      // Parse length modifiers
      while (*current_format_ptr == 'h' || *current_format_ptr == 'l' ||
             *current_format_ptr == 'j' || *current_format_ptr == 't' ||
             *current_format_ptr == 'z' || *current_format_ptr == 'L') {
        *temp_segment_ptr++ = *current_format_ptr++;
      }
      char conversion_specifier = *current_format_ptr;
      *temp_segment_ptr++ = conversion_specifier;
      *temp_segment_ptr = '\0';

      if (!valid_conversion_specifiers[(unsigned char)conversion_specifier]) {
        error(1, 0, gettext("%.*s: invalid conversion specification"), (int)(current_format_ptr - specifier_start + 1), specifier_start);
      }

      const char *arg_value = NULL;
      if (arg_index_to_fetch < arg_count) {
        arg_value = args_array[arg_index_to_fetch];
      }

      print_direc(temp_format_segment, conversion_specifier, has_width, width, has_precision, precision, arg_value);

      current_format_ptr++; // Move past the conversion specifier
    } else if (*current_format_ptr == '\\') {
      int chars_consumed = print_esc(current_format_ptr, 0);
      current_format_ptr += (chars_consumed - 1);
    } else {
      putchar_unlocked((int)*current_format_ptr);
      current_format_ptr++;
    }
  }

  free(temp_format_segment);
  // Return the number of arguments consumed or referenced (highest index + 1)
  // Capped by the actual number of arguments available.
  int args_referenced = max_arg_index_global + 1;
  return (args_referenced > arg_count) ? arg_count : args_referenced;
}

// Function: main
int main(int argc, char **argv) {
  set_program_name(argv[0]);
  setlocale(LC_ALL, "");
  bindtextdomain("coreutils", "/usr/local/share/locale");
  textdomain("coreutils");
  atexit(close_stdout);
  exit_status = 0;
  posixly_correct = (getenv("POSIXLY_CORRECT") != NULL);

  if (argc == 2) {
    if (streq(argv[1], "--help")) {
      usage(0);
    }
    if (streq(argv[1], "--version")) {
      version_etc(_stdout, "printf", "GNU coreutils", _Version, proper_name_lite("David MacKenzie", "David MacKenzie"), NULL);
      return 0;
    }
  }

  char **current_argv_ptr = argv;
  int current_argc_count = argc;

  if (argc > 1 && streq(argv[1], DAT_00102e4f)) { // DAT_00102e4f is "--"
    current_argc_count--;
    current_argv_ptr++;
  }

  if (current_argc_count < 2) {
    error(0, 0, gettext("missing operand"));
    usage(1);
  }

  const char *current_format_string = current_argv_ptr[1];
  int remaining_args_for_printf = current_argc_count - 2; // Exclude program name and first format string
  const char **current_args_for_printf = (const char **)(current_argv_ptr + 2);

  int args_used_by_format;
  do {
    current_arg_index_global = 0;
    prev_arg_index_global = -1;
    max_arg_index_global = -1;

    args_used_by_format = print_formatted(current_format_string, remaining_args_for_printf, current_args_for_printf);

    if (args_used_by_format < 1) { // No arguments referenced, break loop
        break;
    }

    remaining_args_for_printf -= args_used_by_format;
    current_args_for_printf += args_used_by_format;

    // If there are still arguments remaining, the next one is treated as a new format string
    if (remaining_args_for_printf > 0) {
      current_format_string = current_args_for_printf[0];
      remaining_args_for_printf--;
      current_args_for_printf++;
    } else {
      break; // No more arguments, so no more format strings. Stop.
    }

  } while (true);

  if (remaining_args_for_printf > 0) { // Should not happen with the loop logic, but as a fallback
    error(0, 0, gettext("warning: ignoring excess arguments, starting with %s"), quote(current_args_for_printf[0]));
  }
  
  return exit_status;
}