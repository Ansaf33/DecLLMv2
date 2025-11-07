#include <stdio.h>    // For FILE, printf, fputs_unlocked, fprintf, puts, stdout, stderr
#include <stdlib.h>   // For exit, atexit, NULL
#include <locale.h>   // For setlocale, LC_ALL
#include <unistd.h>   // For getlogin
#include <libintl.h>  // For gettext, bindtextdomain, textdomain
#include <stdarg.h>   // For error function variadic arguments
#include <string.h>   // For strcmp in mock parse_gnu_standard_options_only

// --- Mock Declarations for GNU Coreutils functions and globals ---
// These declarations and dummy implementations are necessary to make the
// provided snippets compile as a standalone unit without the actual coreutils
// library source. In a real coreutils build, these would be provided by
// various internal header files and linked from the coreutils libraries.

// Global variables (usually defined elsewhere in coreutils)
extern char *_program_name;
extern const char *_Version;
extern int _optind; // From getopt.h or similar, used by parse_gnu_standard_options_only

// Alias _stdout and _stderr to standard stdout and stderr for compilation
#define _stdout stdout
#define _stderr stderr

// Function prototypes (simplified for compilation)
void set_program_name(const char *name);
void emit_ancillary_info(const char *name);
const char *proper_name_lite(const char *name1, const char *name2);
void parse_gnu_standard_options_only(
    int argc, char *argv[], const char *pkg, const char *prog,
    const char *version, int flags, void (*usage_func)(int),
    const char *author, int bug_report_address_is_zero
);
const char *quote(const char *str);
void error(int status, int errnum, const char *fmt, ...);
void close_stdout(void);

// Dummy implementations for mocks to ensure linkability if compiled as a standalone file
char *_program_name = NULL; // Will be set by set_program_name
const char *_Version = "1.0"; // Dummy version
int _optind = 1; // Dummy initial value for getopt processing

void set_program_name(const char *name) {
    _program_name = (char *)name; // In a real scenario, this would usually copy the string
}
void emit_ancillary_info(const char *name) {
    fprintf(stderr, "Emit ancillary info for: %s\n", name);
}
const char *proper_name_lite(const char *name1, const char *name2) {
    (void)name2; // Unused parameter
    return name1;
}
void parse_gnu_standard_options_only(
    int argc, char *argv[], const char *pkg, const char *prog,
    const char *version, int flags, void (*usage_func)(int),
    const char *author, int bug_report_address_is_zero
) {
    (void)pkg; (void)prog; (void)version; (void)flags; (void)author;
    (void)bug_report_address_is_zero;

    // Simulate basic option parsing for --help and --version
    if (argc > 1) {
        if (strcmp(argv[1], "--help") == 0) {
            usage_func(0);
        } else if (strcmp(argv[1], "--version") == 0) {
            printf("%s %s\n", prog, version);
            exit(0);
        }
    }
    // Advance _optind to simulate options consumed
    if (argc > 1 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "--version") == 0)) {
        _optind = 2;
    } else {
        _optind = 1; // No options, assume first non-option is at argv[1]
    }
}
const char *quote(const char *str) {
    return str; // Dummy implementation: returns the string itself
}
void error(int status, int errnum, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "%s: ", _program_name ? _program_name : "unknown");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
    if (status != 0) {
        exit(status);
    }
}
void close_stdout(void) {
    fflush(stdout);
}

// Function: usage
void usage(int status) {
    if (status == 0) {
        printf(gettext("Usage: %s [OPTION]\n"), _program_name);
        fputs_unlocked(gettext("Print the user\'s login name.\n\n"), _stdout);
        fputs_unlocked(gettext("      --help        display this help and exit\n"), _stdout);
        fputs_unlocked(gettext("      --version     output version information and exit\n"), _stdout);
        emit_ancillary_info("logname");
    } else {
        fprintf(_stderr, gettext("Try \'%s --help\' for more information.\n"), _program_name);
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
    
    const char *author_name = proper_name_lite("David MacKenzie", "David MacKenzie");
    parse_gnu_standard_options_only(
        argc, argv, "logname", "GNU coreutils", _Version, 1, usage, author_name, 0
    );

    if (_optind < argc) {
        error(0, 0, gettext("extra operand %s"), quote(argv[_optind]));
        usage(1);
    }

    char *login_name = getlogin();
    if (login_name == NULL) {
        error(1, 0, gettext("no login name"));
    }
    puts(login_name);
    return 0;
}