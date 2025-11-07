#define _GNU_SOURCE // For fputs_unlocked, putchar_unlocked
#include <stdio.h>
#include <stdlib.h> // For malloc, free, qsort, exit, calloc
#include <string.h> // For strcmp, strdup, strerror
#include <locale.h> // For setlocale, LC_ALL
#include <libintl.h> // For bindtextdomain, textdomain, gettext
#include <errno.h>  // For errno
#include <unistd.h> // For optind
#include <stdarg.h> // For va_list, va_start, vfprintf

// --- Placeholder/Mock functions and globals ---

// For `undefined` and `undefined8` from decompiler
typedef long long undefined8;
typedef unsigned int uint;

// Mock for GNU coreutils specific functions/globals
char *_program_name = "users";
int _optind = 1; // Default for no options
static const char DAT_001008db[] = "%s: %s"; // Inferred format string for error

// Mock _Version (for parse_gnu_standard_options_only)
static const char _Version[] = "1.0";

// Mock xinmalloc (xmalloc is common in coreutils, usually malloc + error checking)
static void* xinmalloc(long count, size_t size) {
    void* ptr = calloc(count, size);
    if (!ptr) {
        fprintf(stderr, "%s: memory exhausted\n", _program_name);
        exit(EXIT_FAILURE);
    }
    return ptr;
}

// Mock extract_trimmed_name: Takes char** (the "record") and returns a new string
// Assuming record_ptr[0] contains the name string.
static char* extract_trimmed_name(char **record_ptr) {
    if (!record_ptr || !record_ptr[0]) return strdup("");
    // In a real scenario, this would trim whitespace etc.
    return strdup(record_ptr[0]);
}

// Mock read_utmp: Populates a count and a pointer to an array of records.
// param_1: filename, param_2: &count, param_3: &data_ptr, param_4: flags
// It returns 0 on success, non-zero on failure.
// For this mock, we'll create some dummy data.
static int read_utmp(const char *filename, long *count_out, void **data_out, uint flags) {
    // Simulate reading some utmp entries
    // Each record is 9 char* slots
    // record[0] is username, record[7] is ut_type (as short)
    long num_entries = 3;
    // Allocate space for 3 records, each 9 char* long
    char ***data = xinmalloc(num_entries, 9 * sizeof(char*));

    // Entry 1: User "alice", type 7 (USER_PROCESS)
    data[0][0] = strdup("alice");
    *(short*)&data[0][7] = 7;
    // Entry 2: User "bob", type 7
    data[1][0] = strdup("bob");
    *(short*)&data[1][7] = 7;
    // Entry 3: User "charlie", type 1 (BOOT_TIME - should be filtered out by `type == 7`)
    data[2][0] = strdup("charlie");
    *(short*)&data[2][7] = 1;

    *count_out = num_entries;
    *data_out = data;
    return 0; // Success
}

// Mock quotearg_n_style_colon
static char* quotearg_n_style_colon(int n, int style, const char *arg) {
    return strdup(arg ? arg : "(null)"); // Simple mock
}

// Mock error function (from GNU error.h)
// error (status, errnum, message, ...)
static void error(int status, int errnum, const char *format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, "%s: ", _program_name);
    vfprintf(stderr, format, args);
    va_end(args);
    if (errnum) {
        fprintf(stderr, ": %s", strerror(errnum));
    }
    fprintf(stderr, "\n");
    if (status) {
        exit(status);
    }
}

// Mock emit_ancillary_info
static void emit_ancillary_info(const char *program) {
    printf("Report bugs to: <bug-coreutils@gnu.org>\n");
    printf("GNU coreutils home page: <https://www.gnu.org/software/coreutils/>\n");
    printf("General help using GNU software: <https://www.gnu.org/get_help/>\n");
}

// Mock close_stdout (used with atexit)
static void close_stdout(void) {
    // In a real coreutils program, this would check for write errors on stdout
    // and exit if any occurred. For a simple mock, do nothing.
    fflush(stdout);
}

// Mock proper_name_lite
static char* proper_name_lite(const char *full_name, const char *first_name) {
    return (char*)full_name; // Simple mock, returns a literal
}

// Mock parse_gnu_standard_options_only
// This is a complex function, just stub it out to advance _optind
static void parse_gnu_standard_options_only(int argc, char **argv, const char *command_name,
                                            const char *package, const char *version,
                                            int flags, void (*usage_func)(int),
                                            const char *authors1, const char *authors2,
                                            const char *copyright_year) {
    // Simulate argument parsing for --help and --version
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--help") == 0) {
            usage_func(0);
        } else if (strcmp(argv[i], "--version") == 0) {
            printf("%s (%s) %s\n", command_name, package, version);
            printf("Copyright (C) %s Free Software Foundation, Inc.\n", copyright_year ? copyright_year : "YEAR");
            printf("License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\n");
            printf("This is free software: you are free to change and redistribute it.\n");
            printf("There is NO WARRANTY, to the extent permitted by law.\n\n");
            printf("Written by %s, and %s.\n", authors1, authors2);
            exit(0);
        } else if (argv[i][0] == '-') {
            // Assume any other option consumes one argument slot for simplicity
            _optind++;
        } else {
            // Non-option argument
            _optind = i;
            return;
        }
    }
    _optind = argc; // All arguments were options, or no arguments.
}

// Mock quote
static char* quote(const char *str) {
    if (!str) return strdup("''");
    char *quoted = (char*)malloc(strlen(str) + 3); // For '', and null terminator
    if (quoted) {
        sprintf(quoted, "'%s'", str);
    }
    return quoted;
}

// Mock stack canary (simplified)
static unsigned long __stack_chk_guard = 0xDEADBEEF; // A dummy value
static unsigned long get_stack_chk_guard(void) {
    return __stack_chk_guard;
}

static void __stack_chk_fail(void) {
    fprintf(stderr, "%s: stack smashing detected\n", _program_name);
    exit(EXIT_FAILURE);
}

// --- End Placeholder/Mock functions ---

// Function: userid_compare
// qsort comparison function: takes const void* and casts to char**
int userid_compare(const void *a, const void *b) {
  const char * const *param_1 = (const char * const *)a;
  const char * const *param_2 = (const char * const *)b;
  return strcmp(*param_1,*param_2);
}

// Function: list_entries_users
void list_entries_users(long num_entries, char **entries_data) {
  // trimmed_names will store the array of char* (trimmed names)
  char **trimmed_names = (char **)xinmalloc(num_entries, sizeof(char*));
  size_t valid_name_count = 0;
  
  // Iterate through the raw entries_data
  // Each logical "record" in entries_data is 9 char* slots long.
  // entries_data[i*9] is the name pointer.
  // entries_data[i*9 + 7] is the short type field.
  for (long i = 0; i < num_entries; ++i) {
    char **current_record_ptr = &entries_data[i * 9];
    // Check if the name field is not empty and the type field is 7 (USER_PROCESS)
    // The original decompiler output `*(short *)(local_48 + 7)` implies
    // `local_48` (char**) is treated as an array of pointers, and the 8th pointer's address
    // is cast to `short*` and dereferenced. This is type punning.
    if ((current_record_ptr[0] != NULL && *current_record_ptr[0] != '\0') && 
        (*(short*)&current_record_ptr[7] == 7)) {
      trimmed_names[valid_name_count] = extract_trimmed_name(current_record_ptr);
      valid_name_count++;
    }
  }
  
  qsort(trimmed_names, valid_name_count, sizeof(char*), userid_compare);
  
  for (size_t i = 0; i < valid_name_count; ++i) {
    fputs_unlocked(trimmed_names[i], stdout);
    putchar_unlocked((int)(i < valid_name_count - 1 ? ' ' : '\n'));
  }
  
  for (size_t i = 0; i < valid_name_count; ++i) {
    free(trimmed_names[i]);
  }
  free(trimmed_names);
}

// Function: users
void users(const char *filename, uint flags) {
  long entry_count;
  void *entry_data_ptr; // Will be char***
  
  // Stack canary setup (mocked)
  unsigned long stack_canary_val = get_stack_chk_guard(); // Corresponds to *(long *)(in_FS_OFFSET + 0x28)

  if (read_utmp(filename, &entry_count, &entry_data_ptr, flags | 2) != 0) {
    char *quoted_filename = quotearg_n_style_colon(0, 3, filename);
    error(1, errno, DAT_001008db, _program_name, quoted_filename);
    free(quoted_filename); // Free the quoted string
  }
  
  list_entries_users(entry_count, (char **)entry_data_ptr);
  free(entry_data_ptr); // Free the raw data from read_utmp
  
  // Stack canary check (mocked)
  if (stack_canary_val != get_stack_chk_guard()) {
    __stack_chk_fail();
  }
}

// Function: usage
void usage(int status) {
  if (status == 0) {
    printf(gettext("Usage: %s [OPTION]... [FILE]\n"), _program_name);
    printf(gettext("Output who is currently logged in according to FILE.\nIf FILE is not specified, use %s.  %s as FILE is common.\n\n"),
           "/var/run/utmp","/var/log/wtmp");
    fputs_unlocked(gettext("      --help        display this help and exit\n"), stdout);
    fputs_unlocked(gettext("      --version     output version information and exit\n"), stdout);
    emit_ancillary_info("users");
  } else {
    fprintf(stderr, gettext("Try \'%s --help\' for more information.\n"), _program_name);
  }
  exit(status);
}

// Function: main
int main(int argc, char **argv) {
  set_program_name(argv[0]);
  setlocale(LC_ALL, "");
  bindtextdomain("coreutils","/usr/local/share/locale");
  textdomain("coreutils");
  atexit(close_stdout);
  
  char *author1 = proper_name_lite("David MacKenzie","David MacKenzie");
  char *author2 = proper_name_lite("Joseph Arceneaux","Joseph Arceneaux");
  
  parse_gnu_standard_options_only(argc, argv, "users", "GNU coreutils", _Version, 1,
                                  usage, author2, author1, "2024");
  
  // _optind is updated by parse_gnu_standard_options_only
  if (argc == _optind) { // No file specified
    users("/var/run/utmp", 1); // Default file, and some flags
  } else if (argc - _optind == 1) { // One file specified
    users(argv[_optind], 0); // Specified file, and different flags
  } else { // More than one file specified
    char *extra_operand_quoted = quote(argv[_optind + 1]);
    error(0, 0, gettext("extra operand %s"), extra_operand_quoted);
    free(extra_operand_quoted);
    usage(1);
  }
  
  return 0;
}