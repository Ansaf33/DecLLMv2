#include <stdio.h>    // For FILE, printf, fprintf, fputs_unlocked, putchar_unlocked, stdout, stderr, fflush, EOF
#include <stdlib.h>   // For exit, atexit
#include <stdbool.h>  // For bool
#include <libintl.h>  // For gettext, bindtextdomain, textdomain
#include <locale.h>   // For setlocale, LC_ALL
#include <unistd.h>   // For getopt_long (often also in getopt.h)
#include <getopt.h>   // For struct option, getopt_long, optind
#include <string.h>   // For strlen, strerror
#include <stdarg.h>   // For va_list, va_start, va_end, va_arg

// --- External/Global variables and forward declarations ---
// These would typically be defined in other compilation units or global scope.
// For a self-contained compilable unit, we'll provide dummy definitions.

// Global program name
const char *_program_name = "dirname";

// Dummy implementation for coreutils functions
void set_program_name(const char *name) {
    _program_name = name;
}

void close_stdout(void) {
    if (fflush(stdout) == EOF) {
        // In a real coreutils program, this would handle the error
        // e.g., error(EXIT_FAILURE, errno, "%s", gettext("write error"));
        // For this exercise, a simple return is fine.
    }
}

// Emits ancillary info, e.g., bug report address
void emit_ancillary_info(const char *program) {
    fprintf(stderr, gettext("Report bugs to: %s\n"), "bug-coreutils@gnu.org");
    fprintf(stderr, gettext("GNU coreutils home page: <%s>\n"), "https://www.gnu.org/software/coreutils/");
    fprintf(stderr, gettext("General help using GNU software: <%s>\n"), "https://www.gnu.org/gettext/help/");
}

// Error function (simplified)
void error(int status, int errnum, const char *format, ...) {
    va_list args;
    fprintf(stderr, "%s: ", _program_name);
    va_start(args, format);
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

// dir_len: Returns the length of the directory component of a path.
// e.g., "/usr/local/bin" -> 10 ("/usr/local")
//       "/usr/local/"    -> 4 ("/usr")
//       "foo/bar"        -> 3 ("foo")
//       "foo"            -> 0 (no slash, means ".")
//       "/"              -> 1 ("/")
//       "//"             -> 1 ("/")
size_t dir_len(const char *path) {
    size_t len = strlen(path);

    // 1. Handle empty string
    if (len == 0) return 0; // Represents "."

    // 2. Remove trailing slashes
    while (len > 0 && path[len - 1] == '/') {
        len--;
    }

    // 3. Handle case where only slashes remain (e.g., "/", "///")
    if (len == 0) return 1; // Represents "/"

    // 4. Find the last slash in the remaining string
    // Iterate backwards from the end of the non-slash part
    for (size_t i = len - 1; i > 0; --i) {
        if (path[i] == '/') {
            return i; // Return the index of the slash, which is the length of the directory part
        }
    }

    // 5. No slashes found (e.g., "foo", "bar.txt")
    return 0; // Represents "."
}

// These are typically for authorship information.
const char *proper_name_lite(const char *name, const char *fallback) {
    // In a real scenario, this might check environment variables or config files.
    // For this exercise, just return the name.
    return name;
}

// Version information utility
const char *_Version = "8.32"; // Example version string for coreutils

// Coreutils-style version_etc function (variadic)
void version_etc(FILE *stream, const char *program_name, const char *package_name, const char *version_string, ...) {
    fprintf(stream, "%s %s\n", package_name, version_string);
    fprintf(stream, gettext("Copyright (C) 2024 Free Software Foundation, Inc.\n"));
    fprintf(stream, gettext("License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\n"));
    fprintf(stream, gettext("This is free software: you are free to change and redistribute it.\n"));
    fprintf(stream, gettext("There is NO WARRANTY, to the extent permitted by law.\n\n"));

    fprintf(stream, gettext("Written by "));
    va_list ap;
    va_start(ap, version_string);
    const char *author = va_arg(ap, const char *); // First author
    if (author) {
        fprintf(stream, "%s", author);
        while ((author = va_arg(ap, const char *)) != NULL) {
            fprintf(stream, ", %s", author);
        }
    }
    va_end(ap);
    fprintf(stream, ".\n");
}

// getopt_long configuration
enum {
    HELP_OPTION = 129,    // Custom value for --help
    VERSION_OPTION = 130  // Custom value for --version
};

const char *short_options = "z";

const struct option longopts[] = {
    {"zero", no_argument, NULL, 'z'},
    {"help", no_argument, NULL, HELP_OPTION},
    {"version", no_argument, NULL, VERSION_OPTION},
    {NULL, 0, NULL, 0} // Sentinel for end of array
};

// dot_0 (for when dir_len returns 0)
const char *dot_0 = ".";

// Function: usage
void usage(int param_1) {
    if (param_1 == 0) {
        printf(gettext("Usage: %s [OPTION] NAME...\n"), _program_name);
        fputs_unlocked(gettext(
                               "Output each NAME with its last non-slash component and trailing slashes\nremoved; if NAME contains no /\'s, output \'.\' (meaning the current directory).\n\n"
                           ), stdout);
        fputs_unlocked(gettext("  -z, --zero     end each output line with NUL, not newline\n"), stdout);
        fputs_unlocked(gettext("      --help        display this help and exit\n"), stdout);
        fputs_unlocked(gettext("      --version     output version information and exit\n"), stdout);
        printf(gettext(
                           "\nExamples:\n  %s /usr/bin/          -> \"/usr\"\n  %s dir1/str dir2/str  -> \"dir1\" followed by \"dir2\"\n  %s stdio.h            -> \".\"\n"
                       ), _program_name, _program_name, _program_name);
        emit_ancillary_info("dirname");
    } else {
        fprintf(stderr, gettext("Try \'%s --help\' for more information.\n"), _program_name);
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

    bool zero_terminated_output = false;
    int opt_char;

    while (true) {
        opt_char = getopt_long(argc, argv, short_options, longopts, NULL);

        if (opt_char == -1) { // No more options
            break;
        }

        switch (opt_char) {
            case 'z':
                zero_terminated_output = true;
                break;
            case HELP_OPTION:
                usage(0); // This function calls exit(0)
                break;
            case VERSION_OPTION:
                version_etc(stdout, "dirname", "GNU coreutils", _Version,
                            proper_name_lite("David MacKenzie", "David MacKenzie"),
                            proper_name_lite("Jim Meyering", "Jim Meyering"), NULL);
                exit(0);
            case '?': // getopt_long already prints an error for unknown options
            default:
                usage(1); // This function calls exit(1)
        }
    }

    // After option parsing, process operands
    if (optind >= argc) {
        error(0, 0, gettext("missing operand"));
        usage(1); // This function calls exit(1)
    }

    for (; optind < argc; optind++) {
        const char *filename = argv[optind];
        size_t len = dir_len(filename);

        if (len == 0) {
            filename = dot_0;
            len = 1;
        }
        fwrite_unlocked(filename, 1, len, stdout);
        putchar_unlocked(zero_terminated_output ? '\0' : '\n');
    }

    return 0;
}