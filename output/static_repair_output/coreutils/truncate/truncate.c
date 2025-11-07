#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <libintl.h> // For gettext, bindtextdomain, textdomain
#include <locale.h>  // For setlocale
#include <unistd.h>  // For lseek, close, ftruncate, getopt_long, optarg, optind
#include <sys/stat.h> // For stat, fstat, struct stat, S_ISREG
#include <fcntl.h>   // For open, O_WRONLY, O_CREAT
#include <getopt.h>  // For struct option
#include <ctype.h>   // For isspace, toupper
#include <errno.h>   // For errno, ERANGE, ENOENT
#include <limits.h>  // For LLONG_MAX, LLONG_MIN
#include <string.h>  // For strerror
#include <stdarg.h>  // For va_list in error function

// --- Coreutils-like global variables and function declarations ---
// These are minimal definitions/declarations to make the code compile
// without linking to the full coreutils library.

static char *program_name; // Replaces _program_name

// Placeholder for gettext (from libintl.h, but defining for clarity if not linking)
#ifndef NGETTEXT_H
#define gettext(msgid) (msgid)
#define bindtextdomain(domainname, dirname)
#define textdomain(domainname)
#endif

// Placeholder for error function
static void error(int status, int errnum, const char *message, ...) {
    va_list args;
    va_start(args, message);
    fprintf(stderr, "%s: ", program_name ? program_name : "unknown");
    vfprintf(stderr, message, args);
    if (errnum != 0) {
        fprintf(stderr, ": %s", strerror(errnum));
    }
    fprintf(stderr, "\n");
    va_end(args);
    if (status != 0) {
        exit(status);
    }
}

// Placeholder for quotearg_style
static char *quotearg_style(int style, const char *arg) {
    (void)style; // Unused parameter
    return (char *)arg; // Simple pass-through
}

// Placeholder for quote_n
static char *quote_n(int n, const char *arg) {
    (void)n; // Unused parameter
    return (char *)arg; // Simple pass-through
}

// Placeholder for usable_st_size
static bool usable_st_size(const struct stat *st) {
    // A minimal check: if it's a regular file and size is non-negative
    return S_ISREG(st->st_mode) && st->st_size >= 0;
}

// Placeholder for xdectoimax (coreutils number parsing)
static long xdectoimax(const char *num_str, long min_val, long max_val, const char *suffixes, const char *error_msg_label, int flags) {
    long val;
    char *endptr;
    errno = 0;
    val = strtoll(num_str, &endptr, 10);

    if (errno == ERANGE && (val == LLONG_MAX || val == LLONG_MIN)) {
        error(1, 0, gettext("value out of range for '%s'"), error_msg_label);
    }
    if (endptr == num_str) {
        error(1, 0, gettext("invalid number: '%s'"), num_str);
    }

    // Basic suffix handling (e.g., 'K', 'M', 'G').
    // The actual coreutils xdectoimax is much more comprehensive.
    if (*endptr) {
        long multiplier = 1;
        bool suffix_found = false;
        if (suffixes) { // Check if suffix is in the allowed list
            char upper_suffix_char = toupper((unsigned char)*endptr);
            if (strchr(suffixes, upper_suffix_char)) {
                 suffix_found = true;
                switch (upper_suffix_char) {
                    case 'K': multiplier = 1024; break;
                    case 'M': multiplier = 1024 * 1024; break;
                    case 'G': multiplier = 1024 * 1024 * 1024; break;
                    // Add more suffixes as needed
                    default: suffix_found = false; break; // Unhandled valid suffix
                }
            }
        }
        if (!suffix_found) {
            error(1, 0, gettext("invalid suffix in number: '%s'"), num_str);
        }

        // Check for overflow before multiplication
        if (multiplier > 0 && val > LLONG_MAX / multiplier) {
             error(1, 0, gettext("overflow with multiplier for '%s'"), num_str);
        }
        if (multiplier < 0 && val < LLONG_MAX / multiplier) { // For negative multipliers if applicable
             error(1, 0, gettext("underflow with multiplier for '%s'"), num_str);
        }
        val *= multiplier;
    }

    if (val < min_val || val > max_val) {
        error(1, 0, gettext("value out of range for '%s'"), error_msg_label);
    }
    return val;
}

// Placeholder for proper_name_lite
static char *proper_name_lite(const char *name, const char *email) {
    (void)email; // Unused parameter
    return (char *)name;
}

// Placeholder for version_etc
static void version_etc(FILE *stream, const char *prog_name, const char *package,
                        const char *version, const char *authors, ...) {
    fprintf(stream, "%s (%s) %s\n", prog_name, package, version);
    fprintf(stream, "Copyright (C) 2023 Free Software Foundation, Inc.\n");
    fprintf(stream, "License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\n");
    fprintf(stream, "This is free software: you are free to change and redistribute it.\n");
    fprintf(stream, "There is NO WARRANTY, to the extent permitted by law.\n\n");
    fprintf(stream, "Written by %s.\n", authors);
}

// Placeholder for close_stdout
static void close_stdout(void) {
    fflush(stdout);
}

// Stack protector related (assuming GCC/Clang with stack protection)
#ifdef __GNUC__
extern void __stack_chk_fail(void) __attribute__((noreturn));
extern unsigned long __stack_chk_guard;
#else
// Minimal stub for non-GCC compilers
static unsigned long __stack_chk_guard = 0xDEADBEEF; // A dummy value
static void __stack_chk_fail(void) {
    fprintf(stderr, "Stack smashing detected!\n");
    exit(2);
}
#endif

// Global variables from the original snippet (declared static to limit scope)
static char *ref_file = NULL;
static char no_create = '\0';
static char block_mode = '\0';
static const char *_Version = "8.32"; // Example version
static const char DAT_00101ce6[] = "<padraig@brady.ie>"; // Example email

// Enum for size adjustment modes
enum {
    SIZE_ABSOLUTE = 0,
    SIZE_RELATIVE = 1, // + or -
    SIZE_AT_LEAST = 2, // '>'
    SIZE_AT_MOST = 3,  // '<'
    SIZE_ROUND_DOWN = 4, // '/'
    SIZE_ROUND_UP = 5  // '%'
};

// Forward declaration for usage, as it's called from main
void usage(int status);

// Function: usage
void usage(int status) {
    if (status == 0) {
        printf(gettext("Usage: %s OPTION... FILE...\n"), program_name);
        fputs(gettext(
            "Shrink or extend the size of each FILE to the specified size\n\n"
            "A FILE argument that does not exist is created.\n"
            "If a FILE is larger than the specified size, the extra data is lost.\n"
            "If a FILE is shorter, it is extended and the sparse extended part (hole)\n"
            "reads as zero bytes.\n"
        ), stdout);
        // emit_mandatory_arg_note(); // Placeholder for coreutils specific function
        fputs(gettext("  -c, --no-create        do not create any files\n"), stdout);
        fputs(gettext(
            "  -o, --io-blocks        treat SIZE as number of IO blocks instead of bytes\n"
        ), stdout);
        fputs(gettext(
            "  -r, --reference=RFILE  base size on RFILE\n"
            "  -s, --size=SIZE        set or adjust the file size by SIZE bytes\n"
        ), stdout);
        fputs(gettext("      --help        display this help and exit\n"), stdout);
        fputs(gettext("      --version     output version information and exit\n"), stdout);
        // emit_size_note(); // Placeholder for coreutils specific function
        fputs(gettext(
            "\nSIZE may also be prefixed by one of the following modifying characters:\n"
            "'+' extend by, '-' reduce by, '<' at most, '>' at least,\n"
            "'/' round down to multiple of, '%%' round up to multiple of.\n"
        ), stdout);
        // emit_ancillary_info("truncate"); // Placeholder for coreutils specific function
    } else {
        fprintf(stderr, gettext("Try '%s --help' for more information.\n"), program_name);
    }
    exit(status);
}

// Function: do_ftruncate
// Renamed params for clarity: fd, filename, size_val, ref_size_val, size_op
static bool do_ftruncate(int fd, const char *filename, long size_val, long ref_size_val, int size_op) {
    unsigned long stack_chk_guard_local = __stack_chk_guard; // Stack canary
    bool success = false;
    struct stat st;
    long current_file_size = -1;
    long target_size;
    long block_multiplier = 1; // Default multiplier if block_mode is off

    // fstat is needed if block_mode is on or if a relative size operation is requested
    // or if the reference size is not yet known (ref_size_val < 0)
    if (block_mode || (size_op != SIZE_ABSOLUTE && ref_size_val < 0)) {
        if (fstat(fd, &st) != 0) {
            error(0, errno, gettext("cannot fstat %s"), quotearg_style(4, filename));
            goto end_ftruncate;
        }
    }

    if (block_mode) {
        // Use st_blksize if valid, otherwise default to 512 bytes (0x200)
        if (st.st_blksize >= 1 && (unsigned long)st.st_blksize <= LLONG_MAX) {
            block_multiplier = st.st_blksize;
        } else {
            block_multiplier = 0x200; // Default if st_blksize is unusable
        }

        // Check for multiplication overflow: size_val * block_multiplier
        // size_val from xdectoimax is always positive when block_mode is used,
        // as modifiers like +,-,/,% are applied to the final byte size.
        if (size_val > 0 && block_multiplier > 0 && size_val > LLONG_MAX / block_multiplier) {
            error(0, 0, gettext("overflow in %jd * %td byte blocks for file %s"),
                  size_val, block_multiplier, quotearg_style(4, filename));
            goto end_ftruncate;
        }
        size_val *= block_multiplier;
    }

    if (size_op == SIZE_ABSOLUTE) {
        target_size = size_val;
    } else {
        // Determine current file size if needed for relative operations
        if (ref_size_val < 0) {
            if (!usable_st_size(&st)) {
                current_file_size = lseek(fd, 0, SEEK_END);
                if (current_file_size < 0) {
                    error(0, errno, gettext("cannot get the size of %s"), quotearg_style(4, filename));
                    goto end_ftruncate;
                }
            } else {
                current_file_size = st.st_size;
                if (current_file_size < 0) {
                    error(0, 0, gettext("%s has unusable, apparently negative size"), quotearg_style(4, filename));
                    goto end_ftruncate;
                }
            }
        } else {
            current_file_size = ref_size_val;
        }

        target_size = current_file_size; // Start with current size for relative ops

        switch (size_op) {
            case SIZE_AT_LEAST: // '>'
                if (current_file_size <= size_val) {
                    target_size = size_val;
                }
                break;
            case SIZE_AT_MOST: // '<'
                if (size_val <= current_file_size) {
                    target_size = size_val;
                }
                break;
            case SIZE_ROUND_DOWN: // '/'
                // Division by zero for size_val already handled in main
                target_size = current_file_size - (current_file_size % size_val);
                break;
            case SIZE_ROUND_UP: { // '%'
                // Division by zero for size_val already handled in main
                long remainder = current_file_size % size_val;
                long adjustment = (remainder == 0) ? 0 : (size_val - remainder);
                // Check for overflow before addition
                if (current_file_size > LLONG_MAX - adjustment) {
                    error(0, 0, gettext("overflow extending size of file %s"), quotearg_style(4, filename));
                    goto end_ftruncate;
                }
                target_size = current_file_size + adjustment;
                break;
            }
            case SIZE_RELATIVE: // '+' or '-'
                // Check for overflow/underflow before addition/subtraction
                if (size_val > 0 && current_file_size > LLONG_MAX - size_val) { // Positive addition overflow
                    error(0, 0, gettext("overflow extending size of file %s"), quotearg_style(4, filename));
                    goto end_ftruncate;
                }
                if (size_val < 0 && current_file_size < LLONG_MIN - size_val) { // Negative addition underflow
                    error(0, 0, gettext("underflow reducing size of file %s"), quotearg_style(4, filename));
                    goto end_ftruncate;
                }
                target_size = current_file_size + size_val;
                break;
            default:
                // Should not happen with current size_op values
                error(0, 0, gettext("internal error: unknown size operation %d"), size_op);
                goto end_ftruncate;
        }
    }

    if (target_size < 0) {
        target_size = 0; // Truncate to 0 if calculated size is negative
    }

    if (ftruncate(fd, target_size) == 0) {
        success = true;
    } else {
        error(0, errno, gettext("failed to truncate %s at %jd bytes"), quotearg_style(4, filename), target_size);
    }

end_ftruncate:
    if (stack_chk_guard_local != __stack_chk_guard) {
        __stack_chk_fail();
    }
    return success;
}

// Function: main
int main(int argc, char *argv[]) {
    unsigned long stack_chk_guard_local = __stack_chk_guard; // Stack canary
    bool size_specified = false;
    long size_val = 0; // The size specified by -s/--size
    long ref_size_val = -1; // The reference size from -r/--reference, or -1 if not specified
    int size_op = SIZE_ABSOLUTE; // Mode of size adjustment
    bool any_errors = false; // Flag to track if any file operation failed

    program_name = argv[0];
    setlocale(LC_ALL, ""); // Use LC_ALL for all locale categories
    bindtextdomain("coreutils", "/usr/local/share/locale");
    textdomain("coreutils");
    atexit(close_stdout);

    static const struct option longopts[] = {
        {"no-create", no_argument, NULL, 'c'},
        {"io-blocks", no_argument, NULL, 'o'},
        {"reference", required_argument, NULL, 'r'},
        {"size", required_argument, NULL, 's'},
        {"help", no_argument, NULL, -0x82}, // Corresponds to --help in original
        {"version", no_argument, NULL, -0x83}, // Corresponds to --version in original
        {NULL, 0, NULL, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "cor:s:", longopts, NULL)) != -1) {
        switch (opt) {
            case 'c':
                no_create = 1;
                break;
            case 'o':
                block_mode = 1;
                break;
            case 'r':
                ref_file = optarg;
                break;
            case 's': {
                size_specified = true;
                char *s_arg = optarg;
                // Skip leading whitespace
                while (isspace((unsigned char)*s_arg)) {
                    s_arg++;
                }

                // Check for size modifiers
                char modifier = *s_arg;
                if (modifier == '>') {
                    size_op = SIZE_AT_LEAST;
                    s_arg++;
                } else if (modifier == '<') {
                    size_op = SIZE_AT_MOST;
                    s_arg++;
                } else if (modifier == '/') {
                    size_op = SIZE_ROUND_DOWN;
                    s_arg++;
                } else if (modifier == '%') {
                    size_op = SIZE_ROUND_UP;
                    s_arg++;
                }

                // Skip whitespace after modifier
                while (isspace((unsigned char)*s_arg)) {
                    s_arg++;
                }

                // Check for relative size (+/-)
                if (*s_arg == '+' || *s_arg == '-') {
                    if (size_op != SIZE_ABSOLUTE) {
                        error(0, 0, gettext("multiple relative modifiers specified"));
                        usage(1);
                    }
                    size_op = SIZE_RELATIVE;
                }

                size_val = xdectoimax(s_arg, LLONG_MIN, LLONG_MAX, "EgGkKmMPQRtTYZ0", gettext("Invalid number"), 0);

                if ((size_op == SIZE_ROUND_DOWN || size_op == SIZE_ROUND_UP) && size_val == 0) {
                    error(1, 0, gettext("division by zero"));
                }
                break;
            }
            case -0x82: // --help
                usage(0);
                // usage exits, so no break needed
            case -0x83: // --version
                version_etc(stdout, "truncate", "GNU coreutils", _Version,
                            proper_name_lite("Padraig Brady", DAT_00101ce6), NULL);
                exit(0);
            case '?': // Unknown option or missing argument for option
                usage(1);
                // usage exits, so no break needed
            default:
                usage(1);
        }
    }

    // Post-option processing checks
    if (!ref_file && !size_specified) {
        error(0, 0, gettext("you must specify either %s or %s"),
              quote_n(0, "--size"), quote_n(1, "--reference"));
        usage(1);
    }
    if (ref_file && size_specified && size_op == SIZE_ABSOLUTE) {
        error(0, 0, gettext("you must specify a relative %s with %s"),
              quote_n(0, "--size"), quote_n(1, "--reference"));
        usage(1);
    }
    if (block_mode && !size_specified) {
        error(0, 0, gettext("%s was specified but %s was not"),
              quote_n(0, "--io-blocks"), quote_n(1, "--size"));
        usage(1);
    }
    if (argc - optind < 1) {
        error(0, 0, gettext("missing file operand"));
        usage(1);
    }

    // Process --reference file to get its size
    if (ref_file) {
        struct stat ref_st;
        long file_size = -1;
        if (stat(ref_file, &ref_st) != 0) {
            error(1, errno, gettext("cannot stat %s"), quotearg_style(4, ref_file));
        }

        if (!usable_st_size(&ref_st)) {
            int ref_fd = open(ref_file, O_RDONLY);
            if (ref_fd >= 0) {
                file_size = lseek(ref_fd, 0, SEEK_END);
                int saved_errno = errno; // Save errno from lseek
                close(ref_fd);
                errno = saved_errno; // Restore errno for potential errors from lseek
            }
        } else {
            file_size = ref_st.st_size;
        }

        if (file_size < 0) {
            error(1, errno, gettext("cannot get the size of %s"), quotearg_style(4, ref_file));
        }

        if (size_specified) {
            ref_size_val = file_size; // Used as base for relative size operations
        } else {
            size_val = file_size; // Used as absolute target size
        }
    }

    // Determine open flags
    int open_flags = O_WRONLY;
    if (!no_create) {
        open_flags |= O_CREAT;
    }

    // Process file operands
    for (int i = optind; i < argc; i++) {
        const char *file_path = argv[i];
        int fd = open(file_path, open_flags, 0666); // 0666 is octal for rw-rw-rw-

        if (fd < 0) {
            if (no_create && errno == ENOENT) {
                // If --no-create and file does not exist, silently skip (as per original logic)
                continue;
            }
            error(0, errno, gettext("cannot open %s for writing"), quotearg_style(4, file_path));
            any_errors = true;
        } else {
            if (!do_ftruncate(fd, file_path, size_val, ref_size_val, size_op)) {
                any_errors = true;
            }
            if (close(fd) != 0) {
                error(0, errno, gettext("failed to close %s"), quotearg_style(4, file_path));
                any_errors = true;
            }
        }
    }

    if (stack_chk_guard_local != __stack_chk_guard) {
        __stack_chk_fail();
    }

    return any_errors ? EXIT_FAILURE : EXIT_SUCCESS;
}