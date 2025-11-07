#ifndef _GNU_SOURCE
#define _GNU_SOURCE // For fputs_unlocked, fileno_unlocked, etc.
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libintl.h> // For gettext, bindtextdomain, textdomain
#include <locale.h>  // For setlocale
#include <unistd.h>  // For getopt_long, read, close, optind, optarg, STDIN_FILENO, STDOUT_FILENO
#include <signal.h>  // For signal, SIGINT, SIG_IGN, SIGPIPE
#include <errno.h>   // For errno, EPIPE, EINTR
#include <getopt.h>  // For struct option
#include <stdbool.h> // For bool
#include <stdarg.h>  // For va_list in error stub
#include <sys/types.h> // For ssize_t

// --- Global variables (coreutils specific) ---
char *program_name = NULL;
int append = 0;
char ignore_interrupts = 0;
int output_error = 0; // 0: default, 1: warn, 2: warn-nopipe, 3: exit, 4: exit-nopipe

// DAT_001016cb is likely the short options string for getopt_long
const char *short_options = "aip";

// long_options structure for getopt_long
struct option long_options[] = {
    {"append", no_argument, NULL, 'a'},
    {"ignore-interrupts", no_argument, NULL, 'i'},
    {"output-error", required_argument, NULL, 'p'},
    {"help", no_argument, NULL, -0x82}, // Example values for --help and --version
    {"version", no_argument, NULL, -0x83},
    {NULL, 0, NULL, 0}
};

// For --output-error argument matching
const char *output_error_args[] = {
    "warn", "warn-nopipe", "exit", "exit-nopipe", NULL
};
int output_error_types[] = {
    1, 2, 3, 4 // Corresponding output_error values
};

// DAT_00101662 is likely package info for version_etc
const char *package_info = "tee";
const char *_Version = "8.32"; // Example version

// --- Coreutils stub functions ---
void set_program_name(const char *name) {
    if (program_name) free(program_name);
    program_name = strdup(name);
}

void close_stdout(void) {
    fflush(stdout);
}

// Stub: always ok for input
char iopoll_input_ok(int fd) { return 1; }

// Generic error reporting function, simplified from coreutils' error()
void error(int status, int errnum, const char *format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, "%s: ", program_name ? program_name : "unknown");
    vfprintf(stderr, format, args);
    if (errnum) {
        fprintf(stderr, ": %s\n", strerror(errnum));
    } else {
        fprintf(stderr, "\n");
    }
    va_end(args);
    if (status) exit(status);
}

// Use standard errno directly, or provide a stub if __errno_location is not available
int *__errno_location(void) { return &errno; }

// Stubs for proper_name_lite and version_etc
const char *proper_name_lite(const char *name, const char *litename) { return name; }
void version_etc(FILE *stream, const char *package, const char *program_id, const char *version, ...) {
    fprintf(stream, "%s %s\n", package, version);
    // Simplified, ignores additional authors for this stub
}

// Stub for quotearg_n_style_colon
const char *quotearg_n_style_colon(int n, int style, const char *arg) { return arg; }

// Stub for __xargmatch_internal
long __xargmatch_internal(const char *context, const char *arg, const char *const *arglist, const int *valuelist, size_t valuelist_len, void (*exit_fn)(void), int allow_abbrev) {
    for (size_t i = 0; i < valuelist_len; ++i) {
        if (strcmp(arg, arglist[i]) == 0) {
            return i;
        }
    }
    fprintf(stderr, "%s: invalid argument \'%s\' for \'%s\'\n", program_name, arg, context);
    exit_fn(); // Call the provided exit function
    return -1; // Should not reach here
}

// Stub for _argmatch_die
void _argmatch_die(void) { exit(2); }

// Stub for emit_ancillary_info
void emit_ancillary_info(const char *package) {
    fprintf(stdout, "\n%s home page: <https://www.gnu.org/software/coreutils/>\n", package);
    fprintf(stdout, "GNU C library home page: <https://www.gnu.org/software/libc/>\n");
    fprintf(stdout, "Report tee bugs to: <https://bugs.gnu.org/gnu-coreutils>.\n");
}

// Stubs for binary mode, fadvise, fopen_safer, iopoll_output_ok, bad_cast, fwrite_wait, fclose_wait, iopoll, xnmalloc
void xset_binary_mode(int fd, int mode) { /* No-op for this example */ }
void fadvise(FILE *fp, int advice) { /* No-op for this example */ }
FILE *fopen_safer(const char *filename, const char *mode) { return fopen(filename, mode); }
char iopoll_output_ok(int fd) { return 1; } // Stub: always ok
const char *bad_cast(const char *s) { return s; }
char fwrite_wait(const void *ptr, size_t size, FILE *stream) {
    return (fwrite(ptr, 1, size, stream) == size);
}
char fclose_wait(FILE *stream) {
    return (fclose(stream) == 0);
}
int iopoll(int in_fd, int out_fd, int timeout) {
    // A more realistic stub would use select/poll, but for compilation, 0 means ready.
    // -2 for broken pipe, -3 for error
    return 0; // Always ready
}
void *xnmalloc(size_t n, size_t s) {
    void *p = calloc(n, s);
    if (!p) {
        error(1, 0, gettext("memory exhausted"));
    }
    return p;
}

// Stack smashing protection (if not provided by system headers)
#if defined(__linux__) && !defined(_FORTIFY_SOURCE)
void __stack_chk_fail(void) {
    error(1, 0, gettext("Stack smashing detected!"));
}
#endif

// --- Function: usage ---
void usage(int status) {
    if (status == 0) {
        printf(gettext("Usage: %s [OPTION]... [FILE]...\n"), program_name);
        fputs_unlocked(gettext(
                            "Copy standard input to each FILE, and also to standard output.\n\n  -a, --append              append to the given FILEs, do not overwrite\n  -i, --ignore-interrupts   ignore interrupt signals\n"
                            ), stdout);
        fputs_unlocked(gettext(
                            "  -p                        operate in a more appropriate MODE with pipes\n      --output-error[=MODE]   set behavior on write error.  See MODE below\n"
                            ), stdout);
        fputs_unlocked(gettext("      --help        display this help and exit\n"), stdout);
        fputs_unlocked(gettext("      --version     output version information and exit\n"), stdout);
        fputs_unlocked(gettext(
                            "\nMODE determines behavior with write errors on the outputs:\n  warn           diagnose errors writing to any output\n  warn-nopipe    diagnose errors writing to any output not a pipe\n  exit           exit on error writing to any output\n  exit-nopipe    exit on error writing to any output not a pipe\nThe default MODE for the -p option is \'warn-nopipe\'.\nWith \"nopipe\" MODEs, exit immediately if all outputs become broken pipes.\nThe default operation when --output-error is not specified, is to\nexit immediately on error writing to a pipe, and diagnose errors\nwriting to non pipe outputs.\n"
                            ), stdout);
        emit_ancillary_info(package_info);
    } else {
        fprintf(stderr, gettext("Try \'%s --help\' for more information.\n"), program_name);
    }
    exit(status);
}

// --- Function: get_next_out ---
int get_next_out(FILE **output_files, int num_actual_files, int current_idx) {
    int i = current_idx + 1;
    while (i <= num_actual_files && output_files[i] == NULL) {
        i++;
    }
    return (i > num_actual_files) ? -1 : i;
}

// --- Function: fail_output ---
char fail_output(FILE **output_files, char **error_filenames, int idx) {
    int err = *__errno_location();
    bool should_report_error = true;

    if ((err == EPIPE) && (output_error != 3) && (output_error != 1)) {
        should_report_error = false;
    }

    if (output_files[idx] == stdout) {
        clearerr_unlocked(stdout);
    }

    if (should_report_error) {
        bool exit_on_error = ((output_error == 3) || (output_error == 4));
        error(exit_on_error, err, quotearg_n_style_colon(0, 3, error_filenames[idx]));
    }
    output_files[idx] = NULL; // Mark this output as failed
    return should_report_error;
}

// --- Function: tee_files ---
bool tee_files(int argc_left, char **argv_left, bool use_iopoll) {
    char buffer[8192]; // 0x2000 bytes
    ssize_t bytes_read = 0;
    bool success_flag = true;
    int current_output_idx = 0; // Index for iopoll, starting with stdout

    const char *open_mode = append ? "a" : "w";

    xset_binary_mode(STDIN_FILENO, 0);
    xset_binary_mode(STDOUT_FILENO, 0);
    fadvise(stdin, 2); // FADV_SEQUENTIAL

    // Create an array for FILE* pointers (including stdout at index 0)
    FILE **output_files = (FILE **)xnmalloc((size_t)(argc_left + 1), sizeof(FILE*));
    // Create an array for filenames (for error messages) (including "standard output" at index 0)
    char **error_filenames = (char **)xnmalloc((size_t)(argc_left + 1), sizeof(char*));

    // For iopoll functionality, track output readiness
    char *iopoll_ok_flags = NULL;
    if (use_iopoll) {
        iopoll_ok_flags = (char *)xnmalloc((size_t)(argc_left + 1), sizeof(char));
    }

    output_files[0] = stdout;
    error_filenames[0] = (char*)bad_cast(gettext("standard output"));
    if (use_iopoll) {
        iopoll_ok_flags[0] = iopoll_output_ok(fileno_unlocked(stdout));
    }
    setvbuf(stdout, NULL, _IOLBF, 0); // Line buffered for stdout

    int active_outputs_count = 1; // Start with stdout active

    // Open other files
    for (int i = 1; i <= argc_left; i++) {
        error_filenames[i] = argv_left[i-1]; // argv_left starts from 0, output_files from 1
        output_files[i] = fopen_safer(error_filenames[i], open_mode);
        if (output_files[i] == NULL) {
            if (use_iopoll) {
                iopoll_ok_flags[i] = 0; // Mark as not ok
            }
            bool exit_on_error = ((output_error == 3) || (output_error == 4));
            error(exit_on_error, *__errno_location(), quotearg_n_style_colon(0, 3, error_filenames[i]));
            success_flag = false;
        } else {
            if (use_iopoll) {
                iopoll_ok_flags[i] = iopoll_output_ok(fileno_unlocked(output_files[i]));
            }
            setvbuf(output_files[i], NULL, _IOLBF, 0); // Line buffered
            active_outputs_count++;
        }
    }

    // Main read/write loop
    while (active_outputs_count > 0) {
        bool should_read_input = true;

        if (use_iopoll && output_files[current_output_idx] != NULL && iopoll_ok_flags[current_output_idx]) {
            int iopoll_status = iopoll(STDIN_FILENO, fileno_unlocked(output_files[current_output_idx]), 1);
            if (iopoll_status == -2) { // Broken pipe
                *__errno_location() = EPIPE;
                if (fail_output(output_files, error_filenames, current_output_idx)) {
                    success_flag = false;
                }
                active_outputs_count--;
                current_output_idx = get_next_out(output_files, argc_left, current_output_idx);
                should_read_input = false;
            } else if (iopoll_status == -3) { // iopoll error
                error(0, *__errno_location(), gettext("iopoll error"));
                success_flag = false;
                should_read_input = false;
            }
            // If iopoll_status is 0 or 1, should_read_input remains true
        }

        if (should_read_input) {
            bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer));
            if (bytes_read == -1 && *__errno_location() == EINTR) {
                continue; // Retry read on interrupt
            }
            if (bytes_read < 1) { // EOF or unrecoverable error
                break;
            }

            for (int i = 0; i <= argc_left; i++) {
                if (output_files[i] != NULL) {
                    if (!fwrite_wait(buffer, bytes_read, output_files[i])) {
                        if (fail_output(output_files, error_filenames, i)) {
                            success_flag = false;
                        }
                        active_outputs_count--;
                        if (i == current_output_idx) {
                            current_output_idx = get_next_out(output_files, argc_left, current_output_idx);
                        }
                    }
                }
            }
        }
    }

    if (bytes_read == -1) { // Error during read
        error(0, *__errno_location(), gettext("read error"));
        success_flag = false;
    }

    // Close all output files
    for (int i = 1; i <= argc_left; i++) {
        if (output_files[i] != NULL) {
            if (!fclose_wait(output_files[i])) {
                error(0, *__errno_location(), quotearg_n_style_colon(0, 3, error_filenames[i]));
                success_flag = false;
            }
        }
    }

    free(output_files);
    free(error_filenames);
    if (use_iopoll) {
        free(iopoll_ok_flags);
    }

    return success_flag;
}

// --- Function: main ---
int main(int argc, char *argv[]) {
    set_program_name(argv[0]);
    setlocale(LC_ALL, ""); // LC_ALL is 6
    bindtextdomain("coreutils", "/usr/local/share/locale");
    textdomain("coreutils");
    atexit(close_stdout);

    append = 0;
    ignore_interrupts = 0;

    while (true) {
        int option_index = 0;
        int c = getopt_long(argc, argv, short_options, long_options, &option_index);

        if (c == -1) {
            break; // End of options
        }

        switch (c) {
            case 'a': // --append
                append = 1;
                break;
            case 'i': // --ignore-interrupts
                ignore_interrupts = 1;
                break;
            case 'p': // --output-error
                if (optarg == NULL) {
                    output_error = 2; // Default for -p without arg is warn-nopipe
                } else {
                    long match_idx = __xargmatch_internal("--output-error", optarg, output_error_args, output_error_types, sizeof(output_error_types)/sizeof(output_error_types[0]), _argmatch_die, 1);
                    output_error = output_error_types[match_idx];
                }
                break;
            case -0x82: // --help
                usage(0); // This function exits
                // No break/return needed
            case -0x83: // --version
                version_etc(stdout, package_info, "GNU coreutils", _Version,
                            proper_name_lite("Mike Parker","Mike Parker"),
                            proper_name_lite("Richard M. Stallman","Richard M. Stallman"),
                            proper_name_lite("David MacKenzie","David MacKenzie"), NULL); // NULL for final author
                exit(0);
            case '?': // Unknown option or missing argument
            default:  // Any other unrecognized option
                usage(1); // This function exits
                // No break/return needed
        }
    }

    if (ignore_interrupts) {
        signal(SIGINT, SIG_IGN); // 2 is SIGINT, 0x1 is SIG_IGN
    }
    if (output_error != 0) {
        signal(SIGPIPE, SIG_IGN); // 0xd is SIGPIPE, 0x1 is SIG_IGN
    }

    bool use_iopoll_flag = false;
    if (((output_error == 2) || (output_error == 4)) && iopoll_input_ok(STDIN_FILENO)) {
        use_iopoll_flag = true;
    }

    bool tee_result = tee_files(argc - optind, argv + optind, use_iopoll_flag);

    if (close(STDIN_FILENO) != 0) {
        error(1, *__errno_location(), gettext("standard input"));
    }

    return !tee_result; // Invert result: 0 for success, non-zero for failure
}