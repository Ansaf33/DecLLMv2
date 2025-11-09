#include <stdio.h>    // For FILE, printf, fputs_unlocked, fprintf, stdout, stderr
#include <stdlib.h>   // For exit, atexit
#include <libintl.h>  // For gettext, bindtextdomain, textdomain
#include <locale.h>   // For setlocale, LC_ALL
#include <unistd.h>   // For unlink, optind
#include <errno.h>    // For errno
#include <string.h>   // For strerror
#include <stdarg.h>   // For va_list, va_start, vfprintf, va_end

// --- Mock/External Declarations for coreutils specifics ---
// These declarations are typically provided by coreutils headers or linked libraries.
// For compilation without actual coreutils, we provide simple definitions/externs.

// Global program name, usually set by set_program_name
char *_program_name = NULL;

// Global version string
const char *_Version = "1.0";

// optind from getopt, usually declared in unistd.h or stdlib.h (for GNU)
extern int optind; 

// Dummy implementation for coreutils' set_program_name
void set_program_name(const char *name) {
    _program_name = (char *)name; 
}

// Dummy implementation for coreutils' emit_ancillary_info
void emit_ancillary_info(const char *s) {
    (void)s; 
}

// Dummy implementation for coreutils' close_stdout
void close_stdout(void) {
}

// Dummy implementation for coreutils' proper_name_lite
const char *proper_name_lite(const char *s1, const char *s2) {
    (void)s1; (void)s2; 
    return "Michael Stone"; 
}

// Dummy implementation for coreutils' parse_gnu_standard_options_only
void parse_gnu_standard_options_only(int argc, char *argv[], const char *package,
                                     const char *program_id, const char *version,
                                     int flags, void (*usage_func)(int),
                                     const char *author_name, int author_email) {
    (void)argc; (void)argv; (void)package; (void)program_id; (void)version;
    (void)flags; (void)usage_func; (void)author_name; (void)author_email;
    // For this mock, assume no options are processed and the first argument is at argv[1].
    optind = 1; 
}

// Dummy implementation for coreutils' error function
void error(int status, int errnum, const char *format, ...) {
    va_list args;
    fprintf(stderr, "%s: ", _program_name ? _program_name : "unknown");
    if (errnum) {
        fprintf(stderr, "%s: ", strerror(errnum));
    }
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n"); 
    if (status) {
        exit(status);
    }
}

// Dummy implementation for coreutils' quote
const char *quote(const char *s) {
    return s; 
}

// Dummy implementation for coreutils' quotearg_style
const char *quotearg_style(int style, const char *s) {
    (void)style; 
    return s; 
}

// --- End Mock/External Declarations ---


// Function: usage
void usage(int param_1) {
    if (param_1 == 0) {
        printf(gettext("Usage: %s FILE\n  or:  %s OPTION\n"), _program_name, _program_name);
        fputs_unlocked(gettext("Call the unlink function to remove the specified FILE.\n\n"), stdout);
        fputs_unlocked(gettext("      --help        display this help and exit\n"), stdout);
        fputs_unlocked(gettext("      --version     output version information and exit\n"), stdout);
        emit_ancillary_info("unlink");
    } else {
        fprintf(stderr, gettext("Try '%s --help' for more information.\n"), _program_name);
    }
    exit(param_1);
}

// Function: main
int main(int argc, char *argv[]) {
    set_program_name(argv[0]);
    setlocale(LC_ALL, "");
    bindtextdomain("coreutils", "/usr/local/share/locale");
    textdomain("coreutils");
    atexit(close_stdout);

    parse_gnu_standard_options_only(argc, argv, "unlink", "GNU coreutils", _Version, 1, usage,
                                    proper_name_lite("Michael Stone", "Michael Stone"), 0);

    if (optind >= argc) {
        error(0, 0, gettext("missing operand"));
        usage(1);
    }
    if (optind + 1 < argc) {
        error(0, 0, gettext("extra operand %s"), quote(argv[optind + 1]));
        usage(1);
    }

    if (unlink(argv[optind]) != 0) {
        error(1, errno, gettext("cannot unlink %s"), quotearg_style(4, argv[optind]));
    }

    return 0;
}