#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <libgen.h> // For basename
#include <unistd.h> // For getopt, putenv, unsetenv
#include <getopt.h> // For getopt_long
#include <errno.h>  // For errno
#include <stdbool.h> // For bool type
#include <time.h>   // For struct timespec, time_t, stat
#include <sys/types.h> // For stat
#include <sys/stat.h>  // For stat

// --- Mock Definitions for external functions and global variables ---
// These are minimal definitions to allow the provided code snippet to compile.
// In a real GNU coreutils build, these would come from various libraries.

// Global variables (mocked)
char *_program_name = "date";
FILE *_stdout = NULL; // Will be assigned to stdout in main
FILE *_stderr = NULL; // Will be assigned to stderr in main
int _optind = 1;
char *_optarg = NULL;
int parse_datetime_flags = 0;

// Mock constants/data
const char DAT_00101b39[] = "coreutils date";
const char _Version[] = "8.32"; // Example version
const char DAT_00102f87[] = "C"; // For LC_TIME setting
const char DAT_00102f89[] = "-"; // For filename comparison ("-")
const char DAT_00102f9a[] = "r"; // For fopen mode ("r")
const char DAT_00102f9c[] = "%s"; // Generic format string

// Custom struct for time (like struct timespec)
typedef struct {
    time_t tv_sec;
    long tv_nsec;
} custom_timespec;

// Mock functions
const char *gettext(const char *msgid) { return msgid; }
void emit_mandatory_arg_note() { fputs("Mandatory argument notes...\n", _stdout); }
void emit_ancillary_info(const char *info) { fputs("Ancillary info: ", _stdout); fputs(info, _stdout); fputs("\n", _stdout); }
void close_stdout() { /* Does nothing in mock */ }
void set_program_name(const char *name) { _program_name = (char *)name; }
void bindtextdomain(const char *domainname, const char *dirname) { /* Does nothing in mock */ }
void textdomain(const char *domainname) { /* Does nothing in mock */ }
const char *quote(const char *str) { return str; } // Simple mock, returns original string
void error(int status, int errnum, const char *format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(_stderr, "%s: ", _program_name);
    vfprintf(_stderr, format, args);
    if (errnum != 0) {
        fprintf(_stderr, ": %s", strerror(errnum));
    }
    fprintf(_stderr, "\n");
    va_end(args);
    if (status != 0) {
        exit(status);
    }
}
void *tzalloc(const char *name) { return NULL; } // Mock: returns NULL
void xalloc_die() { fprintf(_stderr, "Fatal: memory allocation failed.\n"); exit(1); }
const char *proper_name_lite(const char *name1, const char *name2) { return name1; } // Mock
void version_etc(FILE *stream, const char *package, const char *program,
                 const char *version, const char *authors, ...) {
    fprintf(stream, "%s (%s) %s\n", program, package, version);
    fprintf(stream, "Copyright (C) 2023 Free Software Foundation, Inc.\n");
    fprintf(stream, "License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\n");
    fprintf(stream, "This is free software: you are free to change and redistribute it.\n");
    fprintf(stream, "There is NO WARRANTY, to the extent permitted by law.\n\n");
    fprintf(stream, "Written by %s.\n", authors);
}
FILE *rpl_fopen(const char *pathname, const char *mode) { return fopen(pathname, mode); }
ssize_t rpl_getline(char **lineptr, size_t *n, FILE *stream) { return getline(lineptr, n, stream); }
int ferror_unlocked(FILE *stream) { return ferror(stream); } // Use ferror as ferror_unlocked is GNU specific
int rpl_fclose(FILE *fp) { return fclose(fp); }
bool streq(const char *s1, const char *s2) { return strcmp(s1, s2) == 0; }
const char *quotearg_n_style_colon(int n, int style, const char *arg) { return arg; } // Simple mock
bool parse_datetime2(custom_timespec *ts, const char *str, time_t base, int flags, void *tz, const char *tz_name) {
    // Mock: simple parsing for seconds since epoch, or failure
    if (str && str[0] == '@') {
        ts->tv_sec = atoll(str + 1);
        ts->tv_nsec = 0;
        return true;
    }
    // For general date strings, just return false for mock
    return false;
}
char *xstrdup(const char *s) { return strdup(s); }
char *rpl_nl_langinfo(int item) { return (char*)"%a %b %e %H:%M:%S %Z %Y"; } // Mock, default format
char *bad_cast(const char *s) { return (char *)s; } // Mock: no cast needed
long long gettime_res() { return 1000000000; } // Mock: 1 nanosecond resolution
void gettime(custom_timespec *ts) {
    struct timespec current_ts;
    clock_gettime(CLOCK_REALTIME, &current_ts);
    ts->tv_sec = current_ts.tv_sec;
    ts->tv_nsec = current_ts.tv_nsec;
}
int settime(custom_timespec *ts) {
    // Mock: does nothing, always succeeds
    errno = 0; // Clear errno
    return 0;
}
bool posixtime(custom_timespec *ts, const char *str, int flags) {
    // Mock: simple parsing for seconds since epoch, or failure
    ts->tv_sec = atoll(str); // Assume str is epoch seconds for simplicity
    ts->tv_nsec = 0;
    return true;
}
custom_timespec get_stat_mtime(struct stat *st) {
    custom_timespec ts;
    ts.tv_sec = st->st_mtime;
    ts.tv_nsec = 0; // stat doesn't always provide nanoseconds easily
    return ts;
}
// For __xargmatch_internal, time_spec_string, time_spec, iso_8601_format_0, rfc_3339_format_1
// These are complex argument matching structures, provide minimal mocks.
// Assume time_spec is an array of ints representing format codes.
int time_spec[] = {0, 1, 2, 3, 4}; // Mock values for different precisions
const char *time_spec_string[] = {"date", "hours", "minutes", "seconds", "ns"}; // Mock strings
const char *iso_8601_format_0[] = {
    "%Y-%m-%d",            // date
    "%Y-%m-%dT%H",         // hours
    "%Y-%m-%dT%H:%M",      // minutes
    "%Y-%m-%dT%H:%M:%S",   // seconds
    "%Y-%m-%dT%H:%M:%S.%N" // ns
};
const char *rfc_3339_format_1[] = {
    "%Y-%m-%d",            // date (index 0)
    NULL,                  // hours (not supported by RFC 3339 in this context)
    NULL,                  // minutes (not supported)
    "%Y-%m-%d %H:%M:%S%z", // seconds (index 3)
    "%Y-%m-%d %H:%M:%S.%N%z" // ns (index 4)
};
long __xargmatch_internal(const char *option_name, const char *arg,
                          const char *const *arglist, const int *valuelist,
                          size_t valuelist_len, void (*exit_fn)(void), int allow_abbrev) {
    for (size_t i = 0; i < valuelist_len; ++i) {
        if (strcmp(arg, arglist[i]) == 0) {
            return i; // Return index
        }
    }
    // If not found, call exit_fn (e.g., usage(1))
    if (exit_fn) exit_fn();
    return -1; // Should not be reached
}
void _argmatch_die() { usage(1); } // Mock for exit_fn
void __stack_chk_fail() { fprintf(_stderr, "Stack smashing detected!\n"); exit(1); }

// Forward declarations for functions defined later
void usage(int param_1);
bool show_date_helper(const char *format, char lc_time_flag, time_t tv_sec, long tv_nsec, void *tz);
long adjust_resolution(char *param_1);
char *set_LC_TIME(char *param_1); // Returns char* as it's a duplicated LC_TIME value

// Using `putchar` instead of `putchar_unlocked`
#define putchar_unlocked putchar
// Using `fputs` instead of `fputs_unlocked`
#define fputs_unlocked fputs

// --- Option parsing values ---
enum {
    ISO_8601_OPT = 0x49, // 'I'
    RFC_EMAIL_OPT = 0x52, // 'R'
    DEBUG_OPT = 128,      // 0x80
    RESOLUTION_OPT = 129, // 0x81
    RFC_3339_OPT = 130,   // 0x82
    HELP_OPT = 131,       // For --help
    VERSION_OPT = 132     // For --version
};

// getopt_long definitions
static const char short_options[] = "d:f:I::r:s:u";
static const struct option long_options[] = {
    {"date", required_argument, NULL, 'd'},
    {"debug", no_argument, NULL, DEBUG_OPT},
    {"file", required_argument, NULL, 'f'},
    {"help", no_argument, NULL, HELP_OPT},
    {"iso-8601", optional_argument, NULL, ISO_8601_OPT},
    {"reference", required_argument, NULL, 'r'},
    {"resolution", no_argument, NULL, RESOLUTION_OPT},
    {"rfc-email", no_argument, NULL, RFC_EMAIL_OPT},
    {"rfc-3339", required_argument, NULL, RFC_3339_OPT},
    {"set", required_argument, NULL, 's'},
    {"utc", no_argument, NULL, 'u'},
    {"universal", no_argument, NULL, 'u'},
    {"version", no_argument, NULL, VERSION_OPT},
    {NULL, 0, NULL, 0}
};

// Mock for show_date (actual implementation not provided, assume it returns bool for success)
bool show_date(const char *format, time_t tv_sec, long tv_nsec, void *tz) {
    struct tm *tm_info;
    char buffer[256];
    time_t rawtime = tv_sec;
    tm_info = localtime(&rawtime); // Using localtime for mock
    // For %N, we need custom handling as strftime doesn't support it directly.
    char temp_format[256];
    strncpy(temp_format, format, sizeof(temp_format) - 1);
    temp_format[sizeof(temp_format) - 1] = '\0';

    char *ns_pos = strstr(temp_format, "%N");
    if (ns_pos) {
        *ns_pos = '\0'; // Temporarily terminate for strftime
        strftime(buffer, sizeof(buffer), temp_format, tm_info);
        char ns_buffer[10]; // 9 digits + null
        snprintf(ns_buffer, sizeof(ns_buffer), "%09ld", tv_nsec);
        strcat(buffer, ns_buffer);
        strcat(buffer, ns_pos + 2); // Append rest of format string
    } else {
        strftime(buffer, sizeof(buffer), format, tm_info);
    }
    fputs(buffer, _stdout);
    return true;
}

// Function: usage
void usage(int exit_code) {
  if (exit_code == 0) {
    printf(gettext("Usage: %s [OPTION]... [+FORMAT]\n  or:  %s [-u|--utc|--universal] [MMDDhhmm[[CC]YY][.ss]]\n"), _program_name, _program_name);
    fputs_unlocked(gettext("Display date and time in the given FORMAT.\nWith -s, or with [MMDDhhmm[[CC]YY][.ss]], set the date and time.\n"), _stdout);
    emit_mandatory_arg_note();
    fputs_unlocked(gettext("  -d, --date=STRING          display time described by STRING, not \'now\'\n"), _stdout);
    fputs_unlocked(gettext("      --debug                annotate the parsed date, and\n                              warn about questionable usage to standard error\n"), _stdout);
    fputs_unlocked(gettext("  -f, --file=DATEFILE        like --date; once for each line of DATEFILE\n"), _stdout);
    fputs_unlocked(gettext("  -I[FMT], --iso-8601[=FMT]  output date/time in ISO 8601 format.\n                               FMT=\'date\' for date only (the default),\n                               \'hours\', \'minutes\', \'seconds\', or \'ns\'\n                               for date and time to the indicated precision.\n                               Example: 2006-08-14T02:34:56-06:00\n"), _stdout);
    fputs_unlocked(gettext("  --resolution               output the available resolution of timestamps\n                               Example: 0.000000001\n"), _stdout);
    fputs_unlocked(gettext("  -R, --rfc-email            output date and time in RFC 5322 format.\n                               Example: Mon, 14 Aug 2006 02:34:56 -0600\n"), _stdout);
    fputs_unlocked(gettext("      --rfc-3339=FMT         output date/time in RFC 3339 format.\n                               FMT=\'date\', \'seconds\', or \'ns\'\n                               for date and time to the indicated precision.\n                               Example: 2006-08-14 02:34:56-06:00\n"), _stdout);
    fputs_unlocked(gettext("  -r, --reference=FILE       display the last modification time of FILE\n"), _stdout);
    fputs_unlocked(gettext("  -s, --set=STRING           set time described by STRING\n  -u, --utc, --universal     print or set Coordinated Universal Time (UTC)\n"), _stdout);
    fputs_unlocked(gettext("      --help        display this help and exit\n"), _stdout);
    fputs_unlocked(gettext("      --version     output version information and exit\n"), _stdout);
    fputs_unlocked(gettext("\nAll options that specify the date to display are mutually exclusive.\nI.e.: --date, --file, --reference, --resolution.\n"), _stdout);
    fputs_unlocked(gettext("\nFORMAT controls the output.  Interpreted sequences are:\n\n  %%   a literal %\n  %a   locale\'s abbreviated weekday name (e.g., Sun)\n"), _stdout);
    fputs_unlocked(gettext("  %A   locale\'s full weekday name (e.g., Sunday)\n  %b   locale\'s abbreviated month name (e.g., Jan)\n  %B   locale\'s full month name (e.g., January)\n  %c   locale\'s date and time (e.g., Thu Mar  3 23:05:25 2005)\n"), _stdout);
    fputs_unlocked(gettext("  %C   century; like %Y, except omit last two digits (e.g., 20)\n  %d   day of month (e.g., 01)\n  %D   date (ambiguous); same as %m/%d/%y\n  %e   day of month, space padded; same as %_d\n"), _stdout);
    fputs_unlocked(gettext("  %F   full date; like %+4Y-%m-%d\n  %g   last two digits of year of ISO week number (ambiguous; 00-99); see %G\n  %G   year of ISO week number; normally useful only with %V\n"), _stdout);
    fputs_unlocked(gettext("  %h   same as %b\n  %H   hour (00..23)\n  %I   hour (01..12)\n  %j   day of year (001..366)\n"), _stdout);
    fputs_unlocked(gettext("  %k   hour, space padded ( 0..23); same as %_H\n  %l   hour, space padded ( 1..12); same as %_I\n  %m   month (01..12)\n  %M   minute (00..59)\n"), _stdout);
    fputs_unlocked(gettext("  %n   a newline\n  %N   nanoseconds (000000000..999999999)\n  %p   locale\'s equivalent of either AM or PM; blank if not known\n  %P   like %p, but lower case\n  %q   quarter of year (1..4)\n  %r   locale\'s 12-hour clock time (e.g., 11:11:04 PM)\n  %R   24-hour hour and minute; same as %H:%M\n  %s   seconds since the Epoch (1970-01-01 00:00 UTC)\n"), _stdout);
    fputs_unlocked(gettext("  %S   second (00..60)\n  %t   a tab\n  %T   time; same as %H:%M:%S\n  %u   day of week (1..7); 1 is Monday\n"), _stdout);
    fputs_unlocked(gettext("  %U   week number of year, with Sunday as first day of week (00..53)\n  %V   ISO week number, with Monday as first day of week (01..53)\n  %w   day of week (0..6); 0 is Sunday\n  %W   week number of year, with Monday as first day of week (00..53)\n"), _stdout);
    fputs_unlocked(gettext("  %x   locale\'s date (can be ambiguous; e.g., 12/31/99)\n  %X   locale\'s time representation (e.g., 23:13:48)\n  %y   last two digits of year (ambiguous; 00..99)\n  %Y   year\n"), _stdout);
    fputs_unlocked(gettext("  %z   +hhmm numeric time zone (e.g., -0400)\n  %:z  +hh:mm numeric time zone (e.g., -04:00)\n  %::z  +hh:mm:ss numeric time zone (e.g., -04:00:00)\n  %:::z  numeric time zone with : to necessary precision (e.g., -04, +05:30)\n  %Z   alphabetic time zone abbreviation (e.g., EDT)\n\nBy default, date pads numeric fields with zeroes.\n"), _stdout);
    fputs_unlocked(gettext("The following optional flags may follow \'%\':\n\n  -  (hyphen) do not pad the field\n  _  (underscore) pad with spaces\n  0  (zero) pad with zeros\n  +  pad with zeros, and put \'+\' before future years with >4 digits\n  ^  use upper case if possible\n  #  use opposite case if possible\n"), _stdout);
    fputs_unlocked(gettext("\nAfter any flags comes an optional field width, as a decimal number;\nthen an optional modifier, which is either\nE to use the locale\'s alternate representations if available, or\nO to use the locale\'s alternate numeric symbols if available.\n"), _stdout);
    fputs_unlocked(gettext("\nExamples:\nConvert seconds since the Epoch (1970-01-01 UTC) to a date\n  $ date --date=\'@2147483647\'\n\nShow the time on the west coast of the US (use tzselect(1) to find TZ)\n  $ TZ=\'America/Los_Angeles\' date\n\nShow the local time for 9AM next Friday on the west coast of the US\n  $ date --date=\'TZ=\"America/Los_Angeles\" 09:00 next Fri\'\n"), _stdout);
    emit_ancillary_info(DAT_00101b39);
  } else {
    fprintf(_stderr, gettext("Try \'%s --help\' for more information.\n"), _program_name);
  }
  exit(exit_code);
}

// Function: res_width
int res_width(long long param_1) {
  int width = 9;
  long long power_of_10 = 1;
  while (power_of_10 <= param_1 / 10) { // Divide by 10 to avoid overflow on power_of_10
    power_of_10 *= 10;
    width--;
  }
  return width;
}

// Function: adjust_resolution
long adjust_resolution(char *param_1) {
  char *new_format = NULL;
  char *current_char = param_1;
  while (*current_char != '\0') {
    if (*current_char == '%') {
      if ((current_char[1] == '-') && (current_char[2] == 'N')) {
        if (new_format == NULL) {
          new_format = xstrdup(param_1);
          if (new_format == NULL) { xalloc_die(); }
        }
        long long resolution_ns = gettime_res();
        char width_char = res_width(resolution_ns) + '0';
        // Modify the duplicated string at the corresponding position
        new_format[(current_char - param_1) + 1] = width_char;
        current_char += 2; // Skip -N
      } else {
        current_char += (current_char[1] == '%'); // Skip second '%' for '%%'
      }
    }
    current_char++;
  }
  return (long)new_format; // Return the pointer to the new format string, cast to long for signature compatibility
}

// Function: set_LC_TIME
char *set_LC_TIME(char *new_lc_time) {
  char *old_lc_all = getenv("LC_ALL");
  if ((old_lc_all != NULL) && (*old_lc_all != '\0')) {
    xsetenv("LC_CTYPE", old_lc_all, 1);
    xsetenv("LC_TIME", old_lc_all, 1);
    xsetenv("LC_MESSAGES", old_lc_all, 1);
    xsetenv("LC_NUMERIC", old_lc_all, 1);
    unsetenv("LC_ALL");
  }

  char *old_lc_time_val = getenv("LC_TIME");
  char *saved_lc_time = (old_lc_time_val == NULL || *old_lc_time_val == '\0') ? NULL : xstrdup(old_lc_time_val);

  if (new_lc_time == NULL) {
    unsetenv("LC_TIME");
  } else {
    xsetenv("LC_TIME", new_lc_time, 1);
  }
  setlocale(LC_ALL, ""); // Set locale based on environment
  return saved_lc_time;
}

// Function: show_date_helper
bool show_date_helper(const char *format, char lc_time_flag, time_t tv_sec, long tv_nsec, void *tz) {
  if ((parse_datetime_flags & 1) != 0) {
    error(0, 0, gettext("output format: %s"), quote(format));
  }
  bool result;
  if (lc_time_flag == '\0') {
    result = show_date(format, tv_sec, tv_nsec, tz);
  } else {
    char *saved_lc_time = set_LC_TIME((char*)DAT_00102f87);
    result = show_date(format, tv_sec, tv_nsec, tz);
    char *restored_lc_time = set_LC_TIME(saved_lc_time);
    free(restored_lc_time); // Free the result of the second set_LC_TIME (which is the LC_TIME value before restore)
    free(saved_lc_time); // Free the original saved LC_TIME value
  }
  putchar_unlocked('\n');
  return result;
}

// Function: batch_convert
bool batch_convert(const char *filename, const char *format_string, char lc_time_flag, void *tz, const char *tz_name) {
  FILE *input_file;
  const char *display_filename;

  if (streq(filename, DAT_00102f89)) { // Check if filename is "-"
    display_filename = gettext("standard input");
    input_file = stdin;
  } else {
    display_filename = filename;
    input_file = rpl_fopen(filename, DAT_00102f9a);
    if (input_file == NULL) {
      error(1, errno, DAT_00102f9c, quotearg_n_style_colon(0, 3, filename));
    }
  }

  char *line = NULL;
  size_t line_size = 0;
  bool overall_success = true;
  custom_timespec parsed_time;

  while (rpl_getline(&line, &line_size, input_file) >= 0) {
    // Remove trailing newline if present
    size_t line_len = strlen(line);
    if (line_len > 0 && line[line_len - 1] == '\n') {
      line[line_len - 1] = '\0';
    }

    if (parse_datetime2(&parsed_time, line, 0, parse_datetime_flags, tz, tz_name)) {
      overall_success &= show_date_helper(format_string, lc_time_flag, parsed_time.tv_sec, parsed_time.tv_nsec, tz);
    } else {
      error(0, 0, gettext("invalid date %s"), quote(line));
      overall_success = false;
    }
  }

  if (ferror_unlocked(input_file)) {
    error(1, errno, gettext("%s: read error"), quotearg_n_style_colon(0, 3, display_filename));
  }

  if (input_file != stdin && rpl_fclose(input_file) == EOF) {
    error(1, errno, DAT_00102f9c, quotearg_n_style_colon(0, 3, display_filename));
  }

  free(line);
  return overall_success;
}

// Function: main
int main(int argc, char **argv) {
  // Initialize standard streams (for mock environment)
  _stdout = stdout;
  _stderr = stderr;

  bool set_time_flag = false;
  bool date_opt_seen_multiple = false;
  bool set_opt_seen_multiple = false;
  char *date_string = NULL;
  char *set_string = NULL;
  char *format_string = NULL;
  char *file_arg = NULL;
  char *reference_file = NULL;
  char iso_rfc_output_flag = '\0'; // If ISO or RFC output is requested (1 for ISO, 2 for RFC-email, 3 for RFC-3339)
  char resolution_flag = '\0'; // If --resolution is requested
  bool result_status = true; // For exit code

  set_program_name(argv[0]);
  setlocale(LC_ALL, "");
  bindtextdomain("coreutils", "/usr/local/share/locale");
  textdomain("coreutils");
  atexit(close_stdout);

  int c;
  while ((c = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
    switch (c) {
      case 'd': // -d, --date
        if (date_string != NULL) { date_opt_seen_multiple = true; }
        date_string = _optarg;
        break;
      case 'f': // -f, --file
        file_arg = _optarg;
        break;
      case ISO_8601_OPT: // -I[FMT], --iso-8601[=FMT]
        if (_optarg == NULL) {
          format_string = (char*)iso_8601_format_0[0]; // Default to date only
        } else {
          long idx = __xargmatch_internal("--iso-8601", _optarg, time_spec_string, time_spec,
                                          sizeof(time_spec)/sizeof(time_spec[0]), _argmatch_die, 1);
          format_string = (char*)iso_8601_format_0[idx];
        }
        iso_rfc_output_flag = 1;
        break;
      case RFC_EMAIL_OPT: // -R, --rfc-email
        format_string = (char*)"%a, %d %b %Y %H:%M:%S %z";
        iso_rfc_output_flag = 2;
        break;
      case 'r': // -r, --reference
        reference_file = _optarg;
        break;
      case 's': // -s, --set
        if (set_string != NULL) { set_opt_seen_multiple = true; }
        set_string = _optarg;
        set_time_flag = true;
        break;
      case 'u': // -u, --utc, --universal
        if (putenv(bad_cast("TZ=UTC0")) != 0) {
          xalloc_die();
        }
        break;
      case DEBUG_OPT: // --debug (128)
        parse_datetime_flags |= 1;
        break;
      case RESOLUTION_OPT: // --resolution (129)
        resolution_flag = '\x01';
        break;
      case RFC_3339_OPT: // --rfc-3339 (130)
        long idx = __xargmatch_internal("--rfc-3339", _optarg, time_spec_string, time_spec,
                                        sizeof(time_spec)/sizeof(time_spec[0]), _argmatch_die, 1);
        format_string = (char*)rfc_3339_format_1[idx];
        iso_rfc_output_flag = 3;
        break;
      case HELP_OPT: // --help (131)
        usage(0);
        break;
      case VERSION_OPT: // --version (132)
        version_etc(_stdout, DAT_00101b39, "GNU coreutils", _Version, proper_name_lite("David MacKenzie", "David MacKenzie"), 0);
        exit(0);
      case '?': // Unrecognized option or missing argument
      default:
        usage(1);
        break;
    }
  }

  // Post-option processing
  char date_specifiers_count = (date_string != NULL) + (file_arg != NULL) + (reference_file != NULL) + (resolution_flag != '\0');
  if (date_specifiers_count > 1) {
    error(0, 0, gettext("the options to specify dates for printing are mutually exclusive"));
    usage(1);
  }
  if (set_time_flag && date_specifiers_count != 0) {
    error(0, 0, gettext("the options to print and set the time may not be used together"));
    usage(1);
  }
  if (date_opt_seen_multiple && (parse_datetime_flags & 1) != 0) {
    error(0, 0, gettext("only using last of multiple -d options"));
  }
  if (set_opt_seen_multiple && (parse_datetime_flags & 1) != 0) {
    error(0, 0, gettext("only using last of multiple -s options"));
  }

  // Handle remaining non-option arguments
  if (_optind < argc) {
    if (_optind + 1 < argc) {
      error(0, 0, gettext("extra operand %s"), quote(argv[_optind + 1]));
      usage(1);
    }
    if (argv[_optind][0] == '+') {
      if (format_string != NULL) {
        error(1, 0, gettext("multiple output formats specified"));
      }
      format_string = argv[_optind] + 1;
      _optind++;
    } else if (set_time_flag || date_specifiers_count != 0) {
      error(0, 0, gettext("the argument %s lacks a leading \'+\';\nwhen using an option to specify date(s), any non-option\nargument must be a format string beginning with \'+\'"), quote(argv[_optind]));
      usage(1);
    }
  }

  // Set default format if not specified
  if (format_string == NULL) {
    if (resolution_flag == '\0') {
      format_string = rpl_nl_langinfo(0x2006c); // D_T_FMT
      if (*format_string == '\0') {
        format_string = (char*)"%a %b %e %H:%M:%S %Z %Y";
      }
    } else {
      format_string = (char*)"%s.%N";
    }
  }

  // Adjust format string for resolution if needed
  char *adjusted_format = (char*)adjust_resolution(format_string);
  if (adjusted_format != NULL) {
    format_string = adjusted_format;
  }

  void *timezone_ptr = tzalloc(getenv("TZ"));
  const char *timezone_name = getenv("TZ"); // Pass TZ environment string

  custom_timespec current_time;
  bool time_parsed_ok = true;

  if (file_arg == NULL) { // Not batch conversion
    char lc_time_flag_for_output = iso_rfc_output_flag != '\0';

    if (reference_file != NULL) {
      struct stat stat_buf;
      if (stat(reference_file, &stat_buf) != 0) {
        error(1, errno, DAT_00102f9c, quotearg_n_style_colon(0, 3, reference_file));
      }
      current_time = get_stat_mtime(&stat_buf);
    } else if (resolution_flag != '\0') {
      long long res_val = gettime_res();
      current_time.tv_sec = res_val / 1000000000;
      current_time.tv_nsec = res_val % 1000000000;
    } else if (set_string != NULL) {
      // --set option
      date_string = set_string;
      time_parsed_ok = parse_datetime2(&current_time, date_string, 0, parse_datetime_flags, timezone_ptr, timezone_name);
    } else if (date_string != NULL) {
      // --date option
      time_parsed_ok = parse_datetime2(&current_time, date_string, 0, parse_datetime_flags, timezone_ptr, timezone_name);
    } else if (_optind < argc) {
      // Positional argument for setting date (MMDDhhmm[[CC]YY][.ss])
      set_time_flag = true;
      date_string = argv[_optind];
      time_parsed_ok = posixtime(&current_time, date_string, 7); // Assume '7' is a flag for posix format
      current_time.tv_nsec = 0; // Posix time usually doesn't have nsec from string
    } else {
      // No date specified, use current time
      gettime(&current_time);
    }

    if (!time_parsed_ok) {
      error(1, 0, gettext("invalid date %s"), quote(date_string ? date_string : ""));
    }

    if (set_time_flag) {
      if (settime(&current_time) != 0) {
        error(0, errno, gettext("cannot set date"));
        result_status = false;
      }
    }
    result_status &= show_date_helper(format_string, lc_time_flag_for_output, current_time.tv_sec, current_time.tv_nsec, timezone_ptr);
  } else {
    // Batch conversion from file
    result_status = batch_convert(file_arg, format_string, iso_rfc_output_flag != '\0', timezone_ptr, timezone_name);
  }

  free(adjusted_format);
  exit((int)(!result_status)); // Exit 0 on success, 1 on failure
}