#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>
#include <locale.h>
#include <fcntl.h> // For O_WRONLY, O_CREAT, O_NONBLOCK
#include <stdarg.h> // For va_list in mock error/version_etc

// PACKAGE_VERSION is typically defined by autoconf/configure
#ifndef PACKAGE_VERSION
#define PACKAGE_VERSION "unknown"
#endif

// Constants for utimensat
#ifndef UTIME_NOW
#define UTIME_NOW ((1L << 30) - 2) // Common coreutils value
#endif
#ifndef UTIME_OMIT
#define UTIME_OMIT ((1L << 30) - 1) // Common coreutils value
#endif

// AT_FDCWD and AT_SYMLINK_NOFOLLOW are from fcntl.h or sys/stat.h on Linux
#ifndef AT_FDCWD
#define AT_FDCWD (-100) // Defined in <fcntl.h>
#endif
#ifndef AT_SYMLINK_NOFOLLOW
#define AT_SYMLINK_NOFOLLOW 0x100 // Defined in <fcntl.h>
#endif

// Global variables (mocked or actual coreutils globals)
char *_program_name = "touch";
FILE *_stdout = NULL; // Will be initialized in main
FILE *_stderr = NULL; // Will be initialized in main

// These store the access and modification times to be applied.
// Initialized to UTIME_OMIT, meaning "do not change this time".
struct timespec specified_atime_val = {0, UTIME_OMIT};
struct timespec specified_mtime_val = {0, UTIME_OMIT};

char no_create = 0;        // Flag for -c, --no-create
char no_dereference = 0;   // Flag for -h, --no-dereference
char amtime_now = 0;       // If true, use current time for unspecified (access/mod) times
int change_times = 0;      // Bitmask: 1 for access, 2 for modification, 3 for both
char use_ref = 0;          // Flag for -r, --reference
char *ref_file = NULL;     // Argument for -r
char DAT_0010156b[] = "-"; // Represents the special "-" filename

// getopt_long related globals
extern int _optind;
extern char *_optarg;

// Mock function implementations for compilation
// In a real coreutils build, these would come from various headers/libraries.

void error(int status, int errnum, const char *message, ...) {
    va_list args;
    va_start(args, message);
    fprintf(_stderr, "%s: ", _program_name);
    vfprintf(_stderr, message, args);
    if (errnum) {
        fprintf(_stderr, ": %s", strerror(errnum));
    }
    fprintf(_stderr, "\n");
    va_end(args);
    if (status) exit(status);
}

const char *gettext(const char *msgid) { return msgid; } // Passthrough for simplicity
const char *quote(const char *str) { return str; }       // Passthrough
const char *quotearg_style(int style, const char *arg) { return arg; } // Passthrough

bool parse_datetime(struct timespec *result, const char *datestr, const struct timespec *base_time) {
    // Mock implementation: just set a dummy time based on base_time.
    // A real implementation would parse 'datestr' relative to 'base_time'.
    result->tv_sec = base_time->tv_sec + 3600; // Simulate adding an hour
    result->tv_nsec = base_time->tv_nsec;
    return true; // Always succeed for mock
}

int fd_reopen(int oldfd, const char *path, int flags, mode_t mode) {
    // Mock: just open the file. In coreutils, this is a wrapper.
    int new_fd = open(path, flags, mode);
    if (new_fd < 0) return -1;
    if (oldfd >= 0 && new_fd != oldfd) {
        dup2(new_fd, oldfd);
        close(new_fd);
        return oldfd;
    }
    return new_fd;
}

bool streq(const char *s1, const char *s2) { return strcmp(s1, s2) == 0; }

int fdutimensat(int dirfd, int fd, const char *pathname, const struct timespec *times, int flags) {
    // Mock: simulate success or specific errors.
    // In a real system, this would call futimens or utimensat.
    if (fd == 1 && pathname == NULL) { // Special case for stdout
        // Cannot change timestamps of stdout using futimens, but coreutils tries.
        // Return EBADF as the original code implies for this case.
        errno = EBADF;
        return -1;
    }
    // Simulate success for other cases.
    return 0;
}

int *__errno_location(void) { return &errno; } // Standard GNU extension

void __stack_chk_fail(void) {
    fprintf(_stderr, "Stack smashing detected!\n");
    exit(EXIT_FAILURE);
}

void __assert_fail(const char *assertion, const char *file, unsigned int line, const char *function) {
    fprintf(_stderr, "%s:%u: %s: Assertion `%s' failed.\n", file, line, function, assertion);
    exit(EXIT_FAILURE);
}

void close_stdout(void) {
    // Standard coreutils function to ensure stdout is flushed and closed cleanly.
    if (_stdout) fflush(_stdout);
}

void set_program_name(const char *name) { _program_name = (char *)name; }

struct timespec current_timespec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts;
}

struct timespec get_stat_atime(const struct stat *st) {
    struct timespec ts;
#ifdef __linux__ // Linux uses st_atim
    ts = st->st_atim;
#else // Fallback for other systems
    ts.tv_sec = st->st_atime;
    ts.tv_nsec = 0;
#endif
    return ts;
}

struct timespec get_stat_mtime(const struct stat *st) {
    struct timespec ts;
#ifdef __linux__ // Linux uses st_mtim
    ts = st->st_mtim;
#else // Fallback for other systems
    ts.tv_sec = st->st_mtime;
    ts.tv_nsec = 0;
#endif
    return ts;
}

bool posixtime(time_t *timer, const char *s, int form) {
    // Mock: very basic parsing or fixed time.
    // A real posixtime (from gnulib/lib/posixtime.c) is complex.
    // For compilation, assume it always succeeds with current time.
    *timer = time(NULL);
    return true;
}

// localtime is a standard C library function.
// getenv is a standard C library function.

int posix2_version(void) {
    // Mock: return a value that triggers the legacy path in main.
    return 0x30da0; // Older than 0x30db0 (coreutils 2.27.0)
}

const char *proper_name_lite(const char *a, const char *b) { return a; } // Use first name for simplicity

void version_etc(FILE *stream, const char *package, const char *version, const char *long_version, ...) {
    fprintf(stream, "%s (%s) %s\n", package, "GNU coreutils", long_version);
    fprintf(stream, "Copyright (C) 2023 Free Software Foundation, Inc.\n");
    fprintf(stream, "License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\n");
    fprintf(stream, "This is free software: you are free to change and redistribute it.\n");
    fprintf(stream, "There is NO WARRANTY, to the extent permitted by law.\n\n");
    fprintf(stream, "Written by ");
    va_list args;
    va_start(args, long_version);
    bool first = true;
    const char *author;
    while ((author = va_arg(args, const char *)) != NULL) {
        if (!first) fprintf(stream, ", ");
        fprintf(stream, "%s", author);
        first = false;
    }
    fprintf(stream, ".\n");
    va_end(args);
}

void emit_mandatory_arg_note(void) {
    fputs_unlocked(gettext("Mandatory arguments to long options are mandatory for short options too.\n"), _stdout);
}

void emit_ancillary_info(const char *program) {
    fprintf(_stdout, gettext("GNU coreutils online help: <https://www.gnu.org/software/coreutils/>\n"));
    fprintf(_stdout, gettext("Full documentation <https://www.gnu.org/software/coreutils/%s>\n"), program);
    fprintf(_stdout, gettext("or available locally via: info '(coreutils) %s invocation'\n"), program);
}

size_t fputs_unlocked(const char *s, FILE *stream) { return fputs(s, stream); } // Mock: use fputs

long __xargmatch_internal(const char *context, const char *arg, const char *const *arglist, const char *const *valuelist, size_t valuelistsize, void (*exit_fn)(void), bool allow_abbrev) {
    // Mock: simplified argmatch logic
    for (size_t i = 0; arglist[i] != NULL; ++i) {
        if (strcmp(arg, arglist[i]) == 0) {
            // Assuming valuelist contains int values directly cast to char* for simplicity in mock
            return i; // Return index of match
        }
    }
    // No match, call exit_fn (which is usage(1) in this context)
    fprintf(_stderr, "%s: unrecognized argument '%s' for option '%s'\n", _program_name, arg, context);
    exit_fn();
    return -1; // Should not be reached
}

// getopt_long options
const struct option longopts[] = {
    {"access", no_argument, NULL, 'a'},
    {"no-create", no_argument, NULL, 'c'},
    {"date", required_argument, NULL, 'd'},
    {"force", no_argument, NULL, 'f'}, // Ignored
    {"no-dereference", no_argument, NULL, 'h'},
    {"modify", no_argument, NULL, 'm'},
    {"reference", required_argument, NULL, 'r'},
    {"time", required_argument, NULL, 0x80}, // custom value
    {"help", no_argument, NULL, 0x82},
    {"version", no_argument, NULL, 0x83},
    {NULL, 0, NULL, 0}
};

// For --time=WORD
const char *const time_args[] = {
    "access", "atime", "use",
    "modify", "mtime",
    NULL
};
const unsigned int time_masks[] = {
    1, 1, 1, // for access time
    2, 2,    // for modification time
};

// --- End Mock/External Declarations ---


// Function: date_relative
struct timespec date_relative(const char *datestr, time_t base_sec, long base_nsec) {
    struct timespec result_ts;
    struct timespec base_ts = { .tv_sec = base_sec, .tv_nsec = base_nsec };

    if (!parse_datetime(&result_ts, datestr, &base_ts)) {
        error(1, 0, gettext("invalid date format %s"), quote(datestr));
    }
    return result_ts;
}

// Function: touch
bool touch(char *filename) {
    int fd = -1;
    int create_errno = 0;
    int utimensat_errno = 0;
    bool success = false;
    struct stat st;

    // Prepare timespecs for fdutimensat.
    struct timespec times[2];
    struct timespec *times_ptr = times;

    if (amtime_now) {
        times_ptr = NULL; // Use current time for both access and modification
    } else {
        times[0] = specified_atime_val; // Access time
        times[1] = specified_mtime_val; // Modification time

        if (change_times == 2) { // Only modification time specified
            // If access time was not explicitly set, set it to UTIME_NOW
            if (times[0].tv_nsec == UTIME_OMIT) {
                times[0].tv_sec = 0; // tv_sec is ignored for UTIME_NOW
                times[0].tv_nsec = UTIME_NOW;
            }
        } else if (change_times == 1) { // Only access time specified
            // If modification time was not explicitly set, set it to UTIME_NOW
            if (times[1].tv_nsec == UTIME_OMIT) {
                times[1].tv_sec = 0; // tv_sec is ignored for UTIME_NOW
                times[1].tv_nsec = UTIME_NOW;
            }
        }
        // If change_times is 3 (both) or 0 (implicitly handled by amtime_now),
        // specified_atime_val/mtime_val are used directly.
    }

    // Handle special filename "-" (standard output)
    if (streq(filename, DAT_0010156b)) {
        fd = 1; // Use stdout's file descriptor
        filename = NULL; // pathname should be NULL for fdutimensat when using an fd
    } else {
        if (!no_create && !no_dereference) {
            // O_WRONLY | O_CREAT | O_NONBLOCK (0x941)
            // S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH (0x1B6 = 0o666)
            fd = fd_reopen(0, filename, O_WRONLY | O_CREAT | O_NONBLOCK, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
            if (fd < 0) {
                create_errno = *__errno_location();
            }
        }
    }

    int utimensat_flags = 0;
    if (no_dereference) {
        utimensat_flags |= AT_SYMLINK_NOFOLLOW;
    }

    // Call fdutimensat
    int ret_utimensat = fdutimensat(fd, AT_FDCWD, filename, times_ptr, utimensat_flags);
    if (ret_utimensat != 0) {
        utimensat_errno = *__errno_location();
    }

    if (utimensat_errno == 0) {
        success = true;
    } else {
        if (fd == 0) { // If fd_reopen opened fd 0 (stdin)
            int close_ret = close(0);
            if (close_ret != 0) {
                error(0, *__errno_location(), gettext("failed to close %s"), quotearg_style(4, filename));
            }
        } else if ((fd == 1) && (utimensat_errno == EBADF) && no_create) {
            // Special success condition for "-" with --no-create
            success = true;
        } else {
            // General error handling
            // Check if error from initial creation attempt or from utimensat itself.
            // ELOOP (0x15), EROFS (0x1E), EACCES (0xD)
            if ((create_errno == 0) || (create_errno == ELOOP) ||
                (((create_errno == EROFS || create_errno == EACCES)) &&
                 (stat(filename, &st) == 0 && S_ISDIR(st.st_mode))))
            {
                if (no_create && utimensat_errno == ENOENT) {
                    success = true; // File didn't exist and --no-create was specified, not an error.
                } else {
                    error(0, utimensat_errno, gettext("setting times of %s"), quotearg_style(4, filename));
                }
            } else {
                error(0, create_errno, gettext("cannot touch %s"), quotearg_style(4, filename));
            }
        }
    }
    return success;
}

// Function: usage
void usage(int status) {
    if (status == 0) {
        printf(gettext("Usage: %s [OPTION]... FILE...\n"), _program_name);
        fputs_unlocked(gettext(
                            "Update the access and modification times of each FILE to the current time.\n\nA FILE argument that does not exist is created empty, unless -c or -h\nis supplied.\n\nA FILE argument string of - is handled specially and causes touch to\nchange the times of the file associated with standard output.\n"
                            ), _stdout);
        emit_mandatory_arg_note();
        fputs_unlocked(gettext(
                            "  -a                     change only the access time\n  -c, --no-create        do not create any files\n  -d, --date=STRING      parse STRING and use it instead of current time\n  -f                     (ignored)\n"
                            ), _stdout);
        fputs_unlocked(gettext(
                            "  -h, --no-dereference   affect each symbolic link instead of any referenced\n                         file (useful only on systems that can change the\n                         timestamps of a symlink)\n  -m                     change only the modification time\n"
                            ), _stdout);
        fputs_unlocked(gettext(
                            "  -r, --reference=FILE   use this file\'s times instead of current time\n  -t [[CC]YY]MMDDhhmm[.ss]  use specified time instead of current time,\n                         with a date-time format that differs from -d\'s\n"
                            ), _stdout);
        fputs_unlocked(gettext(
                            "      --time=WORD        specify which time to change:\n                           access time (-a): \'access\', \'atime\', \'use\';\n                           modification time (-m): \'modify\', \'mtime\'\n"
                            ), _stdout);
        fputs_unlocked(gettext("      --help        display this help and exit\n"), _stdout);
        fputs_unlocked(gettext("      --version     output version information and exit\n"), _stdout);
        emit_ancillary_info("touch");
    } else {
        fprintf(_stderr, gettext("Try \'%s --help\' for more information.\n"), _program_name);
    }
    exit(status);
}

// Function: main
int main(int argc, char *argv[]) {
    // Initialize standard streams (mocked as globals)
    _stdout = stdout;
    _stderr = stderr;

    bool time_specified_explicitly = false;
    bool overall_success = true;
    char *date_string = NULL; // for -d

    set_program_name(argv[0]);
    setlocale(LC_ALL, "");
    bindtextdomain("coreutils", "/usr/local/share/locale");
    textdomain("coreutils");
    atexit(close_stdout);

    change_times = 0; // 0 means no -a or -m yet, will default to both later
    use_ref = 0;
    no_create = 0;

    int opt;
    while ((opt = getopt_long(argc, argv, "acd:fhmr:t:", longopts, NULL)) != -1) {
        switch (opt) {
            case 'a':
                change_times |= 1; // Set access time flag
                break;
            case 'c':
                no_create = 1;
                break;
            case 'd':
                date_string = _optarg;
                break;
            case 'f':
                // Ignored, do nothing
                break;
            case 'h':
                no_dereference = 1;
                break;
            case 'm':
                change_times |= 2; // Set modification time flag
                break;
            case 'r':
                use_ref = 1;
                ref_file = _optarg;
                break;
            case 't': {
                time_t tv_sec;
                if (!posixtime(&tv_sec, _optarg, 6)) { // 6 means [[CC]YY]MMDDhhmm[.ss]
                    error(1, 0, gettext("invalid date format %s"), quote(_optarg));
                }
                specified_atime_val.tv_sec = tv_sec;
                specified_atime_val.tv_nsec = 0;
                specified_mtime_val.tv_sec = tv_sec;
                specified_mtime_val.tv_nsec = 0;
                time_specified_explicitly = true;
                break;
            }
            case 0x80: { // --time=WORD
                long time_mask_idx = __xargmatch_internal("--time", _optarg, time_args, (const char *const *)time_masks, sizeof(time_masks) / sizeof(time_masks[0]), usage, true);
                change_times |= time_masks[time_mask_idx];
                break;
            }
            case 0x82: // --help
                usage(0);
                break;
            case 0x83: // --version
                version_etc(_stdout, "touch", "GNU coreutils", PACKAGE_VERSION,
                            proper_name_lite("Paul Rubin", "Paul Rubin"),
                            proper_name_lite("Arnold Robbins", "Arnold Robbins"),
                            proper_name_lite("Jim Kingdon", "Jim Kingdon"),
                            proper_name_lite("David MacKenzie", "David MacKenzie"),
                            proper_name_lite("Randy Smith", "Randy Smith"),
                            (char *)NULL);
                exit(0);
            default:
                usage(1);
                break;
        }
    }

    // After option parsing, determine final times
    if (change_times == 0) {
        change_times = 3; // Default to changing both access and modification times
    }

    if (time_specified_explicitly && (use_ref || date_string)) {
        error(0, 0, gettext("cannot specify times from more than one source"));
        usage(1);
    }

    if (!use_ref) { // No --reference
        if (date_string) { // --date specified
            struct timespec current_ts = current_timespec();
            struct timespec parsed_ts = date_relative(date_string, current_ts.tv_sec, current_ts.tv_nsec);

            specified_atime_val = parsed_ts;
            specified_mtime_val = parsed_ts;
            time_specified_explicitly = true;

            // Coreutils specific logic to ensure distinct time if parsed time is same as current.
            if ((change_times == 3) && (current_ts.tv_sec == parsed_ts.tv_sec && current_ts.tv_nsec == parsed_ts.tv_nsec)) {
                time_t test_base_sec = current_ts.tv_sec ^ 1; // Try with tv_sec ^ 1
                struct timespec test_expected_ts = { .tv_sec = test_base_sec, .tv_nsec = current_ts.tv_nsec };
                struct timespec test_parsed_ts = date_relative(date_string, test_base_sec, current_ts.tv_nsec);

                if (test_parsed_ts.tv_sec == test_expected_ts.tv_sec && test_parsed_ts.tv_nsec == test_expected_ts.tv_nsec) {
                    time_specified_explicitly = false; // Could not get a distinct time
                }
            }
        }
    } else { // --reference specified
        struct stat ref_st;
        int stat_ret;
        if (no_dereference) {
            stat_ret = lstat(ref_file, &ref_st);
        } else {
            stat_ret = stat(ref_file, &ref_st);
        }

        if (stat_ret != 0) {
            error(1, *__errno_location(), gettext("failed to get attributes of %s"), quotearg_style(4, ref_file));
        }

        specified_atime_val = get_stat_atime(&ref_st);
        specified_mtime_val = get_stat_mtime(&ref_st);
        time_specified_explicitly = true;

        if (date_string) { // If both -r and -d are used, -d applies on top of -r
            if ((change_times & 1) != 0) { // If access time is to be changed
                specified_atime_val = date_relative(date_string, specified_atime_val.tv_sec, specified_atime_val.tv_nsec);
            }
            if ((change_times & 2) != 0) { // If modification time is to be changed
                specified_mtime_val = date_relative(date_string, specified_mtime_val.tv_sec, specified_mtime_val.tv_nsec);
            }
        }
    }

    // Handle legacy `touch FILE1 FILE2 ...` where FILE1 is a time string
    if (!time_specified_explicitly && (argc - _optind > 1)) {
        if (posix2_version() < 0x30db0) { // Check for older coreutils version (< 2.27.0)
            time_t legacy_time_sec;
            if (posixtime(&legacy_time_sec, argv[_optind], 9)) { // Form 9 for legacy time string
                specified_atime_val.tv_sec = legacy_time_sec;
                specified_atime_val.tv_nsec = 0;
                specified_mtime_val.tv_sec = legacy_time_sec;
                specified_mtime_val.tv_nsec = 0;
                time_specified_explicitly = true;

                if (!getenv("POSIXLY_CORRECT")) {
                    struct tm *pt = localtime(&legacy_time_sec);
                    if (pt) {
                        error(0, 0, gettext("warning: 'touch %s' is obsolete; use 'touch -t %04d%02d%02d%02d%02d.%02d'"),
                              argv[_optind], pt->tm_year + 1900, pt->tm_mon + 1, pt->tm_mday,
                              pt->tm_hour, pt->tm_min, pt->tm_sec);
                    }
                }
                _optind++; // Consume the time argument
            }
        }
    }

    if (!time_specified_explicitly) {
        if (change_times == 3) { // If no specific time was given, and we need to change both
            amtime_now = 1; // Use current time for both
        } else { // If only -a or -m was specified, and no explicit time, the other time should be OMIT
            if (!(change_times & 1)) { // If access time is not to be changed
                specified_atime_val.tv_sec = 0;
                specified_atime_val.tv_nsec = UTIME_OMIT;
            }
            if (!(change_times & 2)) { // If modification time is not to be changed
                specified_mtime_val.tv_sec = 0;
                specified_mtime_val.tv_nsec = UTIME_OMIT;
            }
        }
    }

    // Process file operands
    if (argc == _optind) {
        error(0, 0, gettext("missing file operand"));
        usage(1);
    }

    for (int i = _optind; i < argc; ++i) {
        overall_success &= touch(argv[i]);
    }

    return overall_success ? EXIT_SUCCESS : EXIT_FAILURE;
}