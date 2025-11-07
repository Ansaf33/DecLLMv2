#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <locale.h>
#include <libintl.h>
#include <getopt.h>
#include <stdbool.h>
#include <unistd.h> // For STDIN_FILENO, fileno_unlocked
#include <stdarg.h> // For va_list in error function

// --- Global Variables (mimicking decompiled globals) ---
char *delims = NULL;
char *delim_end = NULL;
char have_read_stdin = 0; // Boolean flag: 0 or 1
char line_delim = '\n';    // Default line delimiter
char serial_merge = 0;     // Boolean flag: 0 for parallel, 1 for serial

// Standard GNU coreutils globals
char *_program_name = NULL;
// getopt_long uses these extern variables
extern int optind; // Renamed from _optind to optind
extern char *optarg; // Renamed from _optarg to optarg
FILE *_stdin = NULL; // Initialized in main to stdin
FILE *_stdout = NULL; // Initialized in main to stdout
FILE *_stderr = NULL; // Initialized in main to stderr
const char *_Version = "1.0"; // Dummy version string for version_etc

// String literals from original decompilation
const char DAT_0010149c[] = "-";
const char DAT_0010149e[] = "r";
const char DAT_001014a0[] = "%s"; // Placeholder for error messages, often "%s: %s"
const char DAT_00101711[] = "d:sz"; // Short options string for getopt_long

// longopts structure for getopt_long
struct option longopts[] = {
    {"delimiters", required_argument, NULL, 'd'},
    {"serial", no_argument, NULL, 's'},
    {"zero-terminated", no_argument, NULL, 'z'},
    {"help", no_argument, NULL, 0}, // Returns 0, name checked
    {"version", no_argument, NULL, 0}, // Returns 0, name checked
    {NULL, 0, NULL, 0}
};

// --- Dummy/Minimal Implementations for non-standard functions ---

// GNU xstrdup: Allocates memory for a copy of a string.
char *xstrdup(const char *s) {
    char *copy = strdup(s);
    if (copy == NULL) {
        fprintf(stderr, "xstrdup: memory exhausted\n");
        exit(EXIT_FAILURE);
    }
    return copy;
}

// GNU xmalloc: Allocates memory, exits on failure.
void *xmalloc(size_t size) {
    void *ptr = malloc(size);
    if (ptr == NULL && size > 0) {
        fprintf(stderr, "xmalloc: memory exhausted\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

// GNU xnmalloc: Allocates N elements of SIZE bytes, exits on failure.
void *xnmalloc(size_t n, size_t size) {
    void *ptr = calloc(n, size);
    if (ptr == NULL && n > 0 && size > 0) {
        fprintf(stderr, "xnmalloc: memory exhausted\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

// GNU streq: String equality check (returns 0 if equal, non-zero otherwise)
// The original code implies streq returns 0 for true, which is strcmp's behavior.
int streq(const char *s1, const char *s2) {
    return strcmp(s1, s2);
}

// GNU rpl_fopen: Replacement for fopen, possibly with extra features/error handling.
FILE *rpl_fopen(const char *filename, const char *mode) {
    return fopen(filename, mode);
}

// GNU fadvise: Advises the kernel about file access patterns.
// Dummy implementation for compilation.
void fadvise(FILE *fp, int advice) {
    // No-op for this example
}

// GNU quotearg_n_style_colon: Returns a quoted string for error messages.
// Dummy implementation: just returns the input string.
char *quotearg_n_style_colon(int n, int style, const char *arg) {
    return (char *)arg; // Simplified for compilation
}

// Standard __errno_location: Returns a pointer to errno.
// On most systems, this is equivalent to &errno.
int *__errno_location(void) {
    return &errno;
}

// GNU error: Prints an error message and possibly exits.
// Simplified implementation.
void error(int status, int errnum, const char *format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(_stderr, "%s: ", _program_name ? _program_name : "unknown");
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

// GNU gettext: Internationalization function.
// Dummy implementation: returns the input string.
char *gettext(const char *msgid) {
    return (char *)msgid;
}

// GNU write_error: Handles write errors.
void write_error(void) {
    error(EXIT_FAILURE, errno, gettext("write error"));
}

// GNU rpl_fclose: Replacement for fclose.
int rpl_fclose(FILE *fp) {
    return fclose(fp);
}

// GNU proper_name_lite: Formats a name.
char *proper_name_lite(const char *n1, const char *n2) {
    return (char *)n1; // Simplified
}

// GNU version_etc: Prints version information.
void version_etc(FILE *fp, const char *package, const char *version, const char *authors_line, ...) {
    fprintf(fp, "%s (%s) %s\n", package, "GNU coreutils", version);
    fprintf(fp, "%s\n", authors_line);
}

// GNU set_program_name: Sets the global program name.
void set_program_name(const char *name) {
    _program_name = (char *)name;
}

// GNU close_stdout: Flushes and closes stdout, handles errors.
void close_stdout(void) {
    if (fflush(_stdout) != 0) {
        write_error();
    }
    if (fclose(_stdout) != 0) {
        write_error();
    }
}

// GNU bad_cast: A utility function, likely for casting literals.
const char *bad_cast(const char *s) {
    return s;
}

// Dummy emit functions for usage
void emit_stdin_note(void) {
    fputs_unlocked(gettext("\nWith no FILE, or when FILE is -, read standard input.\n"), _stdout);
}
void emit_mandatory_arg_note(void) {
    fputs_unlocked(gettext("\nMandatory arguments to long options are mandatory for short options too.\n"), _stdout);
}
void emit_ancillary_info(const char *program) {
    // No-op for this example
}

// --- Original Functions, fixed ---

// Function: collapse_escapes
// Replaced 'undefined' with 'int' for return type, simplified variables and removed goto.
int collapse_escapes(char *param_1) {
    char *src = param_1;
    delims = xstrdup(param_1);
    char *dest = delims;

    while (*src != '\0') {
        if (*src == '\\') {
            src++; // Move past the backslash
            char escaped_char = *src;

            if (escaped_char == '\0') {
                delim_end = dest;
                return 1; // Unterminated escape sequence
            }

            switch (escaped_char) {
                case '0': *dest = '\0'; break;
                case '\\': *dest = '\\'; break;
                case 'b': *dest = '\b'; break;
                case 'f': *dest = '\f'; break;
                case 'n': *dest = '\n'; break;
                case 'r': *dest = '\r'; break;
                case 't': *dest = '\t'; break;
                case 'v': *dest = '\v'; break;
                default: *dest = escaped_char; break; // Unrecognized escape sequence, treat as literal
            }
            src++; // Move past the escaped character
        } else {
            *dest = *src;
            src++;
        }
        dest++;
    }
    *dest = '\0'; // Null-terminate the resulting string
    delim_end = dest;
    return 0;
}

// Function: xputchar
void xputchar(char c) {
    if (putchar_unlocked((int)c) < 0) {
        write_error();
    }
}

// Function: paste_parallel
// Replaced 'undefined' with 'int', 'ulong' with 'size_t', 'long param_2' with 'char **filenames'.
// Refactored to remove goto, simplify variables.
int paste_parallel(size_t num_files, char **filenames) {
    int return_status = EXIT_SUCCESS; // Corresponds to local_85
    FILE **files = (FILE **)xnmalloc(num_files, sizeof(FILE *)); // Corresponds to __ptr_00
    char *line_buffer = (char *)xmalloc(num_files + 2); // Corresponds to __ptr, for holding delimiters

    bool some_file_is_stdin = false; // Corresponds to bVar1 (initial loop)

    // First loop: open files
    for (size_t i = 0; i < num_files; i++) {
        if (streq(filenames[i], DAT_0010149c) == 0) { // If filename is "-"
            have_read_stdin = 1;
            files[i] = _stdin;
        } else {
            files[i] = rpl_fopen(filenames[i], DAT_0010149e);
            if (!files[i]) {
                error(0, errno, DAT_001014a0, quotearg_n_style_colon(0, 3, filenames[i]));
                return_status = EXIT_FAILURE; // Mark failure but continue processing other files
                continue; // Skip further processing for this file
            }
            fadvise(files[i], 2); // FADV_SEQUENTIAL
        }
        if (fileno_unlocked(files[i]) == STDIN_FILENO) {
            some_file_is_stdin = true;
        }
    }

    if (some_file_is_stdin && have_read_stdin) {
        error(EXIT_FAILURE, 0, gettext("standard input is closed"));
    }

    size_t active_files = num_files; // Corresponds to local_60
    while (active_files > 0) {
        bool line_read_from_any_file_in_round = false; // Corresponds to bVar1 (inside loop)
        char *current_delim_ptr = delims; // Corresponds to local_58
        size_t buffer_idx = 0; // Corresponds to local_50

        for (size_t i = 0; i < num_files; i++) { // Corresponds to local_48
            bool char_read_from_current_file = false; // Corresponds to bVar2
            int c = EOF; // Current character read
            int file_errno_val = 0; // errno for the current file operation

            if (files[i] != NULL) {
                c = getc_unlocked(files[i]);
                file_errno_val = errno; // Save errno immediately

                if (c != EOF && buffer_idx != 0) { // If buffer has content, flush it before printing current char
                    size_t s_written = fwrite_unlocked(line_buffer, 1, buffer_idx, _stdout);
                    if (buffer_idx != s_written) {
                        write_error();
                    }
                    buffer_idx = 0;
                }

                while (c != EOF && (char_read_from_current_file = true, c != line_delim)) {
                    xputchar((char)c);
                    c = getc_unlocked(files[i]);
                    file_errno_val = errno; // Update errno
                }
            }

            if (char_read_from_current_file) {
                line_read_from_any_file_in_round = true;
                if (i == num_files - 1) { // Last file in the round
                    char output_char = line_delim;
                    if (c != EOF) {
                        output_char = (char)c;
                    }
                    xputchar(output_char);
                } else {
                    if (c != line_delim && c != EOF) {
                        xputchar((char)c);
                    }
                    if (*current_delim_ptr != '\0') {
                        xputchar(*current_delim_ptr);
                    }
                    current_delim_ptr++;
                    if (current_delim_ptr == delim_end) {
                        current_delim_ptr = delims;
                    }
                }
            } else { // No char read from current file (EOF or error)
                if (files[i] != NULL) { // If file was active
                    if (ferror_unlocked(files[i]) == 0) {
                        file_errno_val = 0; // Clear errno if no error
                    }

                    if (files[i] == _stdin) {
                        clearerr_unlocked(files[i]);
                    } else {
                        if (rpl_fclose(files[i]) == -1 && file_errno_val == 0) {
                            file_errno_val = errno; // Update errno if fclose failed
                        }
                    }

                    if (file_errno_val != 0) {
                        error(0, file_errno_val, DAT_001014a0, quotearg_n_style_colon(0, 3, filenames[i]));
                        return_status = EXIT_FAILURE; // Mark failure
                    }
                    files[i] = NULL; // Mark file as closed
                    active_files--;
                }

                if (i == num_files - 1) { // Last file in the round
                    if (line_read_from_any_file_in_round) { // If any line was read in this round
                        if (buffer_idx != 0) { // Flush buffer
                            size_t s_written = fwrite_unlocked(line_buffer, 1, buffer_idx, _stdout);
                            if (buffer_idx != s_written) {
                                write_error();
                            }
                            buffer_idx = 0;
                        }
                        xputchar(line_delim);
                    }
                } else {
                    if (*current_delim_ptr != '\0') {
                        line_buffer[buffer_idx++] = *current_delim_ptr;
                    }
                    current_delim_ptr++;
                    if (current_delim_ptr == delim_end) {
                        current_delim_ptr = delims;
                    }
                }
            }
        } // End of for loop (iterating through files)

        if (!line_read_from_any_file_in_round && active_files == 0) {
            // If no lines were read in this round AND all files are exhausted, break.
            break;
        }
    } // End of while (active_files > 0) loop

    free(files);
    free(line_buffer);
    return return_status;
}

// Function: paste_serial
// Replaced 'undefined' with 'int', 'long param_1' with 'size_t num_files', 'undefined8 *param_2' with 'char **filenames'.
// Refactored to remove goto, simplify variables.
int paste_serial(size_t num_files, char **filenames) {
    int return_status = EXIT_SUCCESS; // Corresponds to local_56

    for (size_t i = 0; i < num_files; i++) { // Corresponds to local_60 and local_68
        FILE *current_file = NULL;
        bool is_stdin_file = (streq(filenames[i], DAT_0010149c) == 0); // 0 means streq is true

        if (!is_stdin_file) {
            current_file = rpl_fopen(filenames[i], DAT_0010149e);
            if (!current_file) {
                error(0, errno, DAT_001014a0, quotearg_n_style_colon(0, 3, filenames[i]));
                return_status = EXIT_FAILURE;
                continue; // Move to next file
            }
            fadvise(current_file, 2);
        } else {
            have_read_stdin = 1;
            current_file = _stdin;
        }

        char *current_delim_ptr = delims; // Corresponds to local_38
        int prev_c = getc_unlocked(current_file); // Corresponds to local_54
        int file_errno_val = errno; // Corresponds to local_50

        if (prev_c != EOF) {
            while (true) {
                int c = getc_unlocked(current_file); // Corresponds to uVar2
                if (c == EOF) break;

                if (prev_c == line_delim) {
                    if (*current_delim_ptr != '\0') {
                        xputchar(*current_delim_ptr);
                    }
                    current_delim_ptr++;
                    if (current_delim_ptr == delim_end) {
                        current_delim_ptr = delims;
                    }
                } else {
                    xputchar((char)prev_c);
                }
                prev_c = c;
            }
            file_errno_val = errno; // Update errno after loop
            xputchar((char)prev_c); // Print the last character before EOF
        }

        // Print line_delim if the last character wasn't it
        if (prev_c != line_delim) {
            xputchar(line_delim);
        }

        if (ferror_unlocked(current_file) == 0) {
            file_errno_val = 0; // Clear errno if no error
        }

        if (!is_stdin_file) { // If not stdin, close the file
            if (rpl_fclose(current_file) == -1 && file_errno_val == 0) {
                file_errno_val = errno;
            }
        } else { // If stdin, clear error flags
            clearerr_unlocked(current_file);
        }

        if (file_errno_val != 0) {
            error(0, file_errno_val, DAT_001014a0, quotearg_n_style_colon(0, 3, filenames[i]));
            return_status = EXIT_FAILURE;
        }
    }
    return return_status;
}

// Function: usage
// Replaced 'undefined8' with 'char *'.
void usage(int status) {
    char *program_name_display = _program_name ? _program_name : "paste";

    if (status == EXIT_SUCCESS) {
        printf(gettext("Usage: %s [OPTION]... [FILE]...\n"), program_name_display);
        fputs_unlocked(gettext(
                            "Write lines consisting of the sequentially corresponding lines from\n"
                            "each FILE, separated by TABs, to standard output.\n"
                            ), _stdout);
        emit_stdin_note();
        emit_mandatory_arg_note();
        fputs_unlocked(gettext(
                            "  -d, --delimiters=LIST   reuse characters from LIST instead of TABs\n"
                            "  -s, --serial            paste one file at a time instead of in parallel\n"
                            ), _stdout);
        fputs_unlocked(gettext("  -z, --zero-terminated    line delimiter is NUL, not newline\n"), _stdout);
        fputs_unlocked(gettext("      --help        display this help and exit\n"), _stdout);
        fputs_unlocked(gettext("      --version     output version information and exit\n"), _stdout);
        emit_ancillary_info("paste");
    } else {
        fprintf(_stderr, gettext("Try \'%s --help\' for more information.\n"), program_name_display);
    }
    exit(status);
}

// Function: main
// Replaced 'byte' with 'int', 'undefined8 *param_2' with 'char **argv'.
// Refactored to remove goto and simplify variables.
int main(int argc, char **argv) {
    _stdin = stdin;
    _stdout = stdout;
    _stderr = stderr;

    set_program_name(argv[0]);
    setlocale(LC_ALL, ""); // LC_ALL corresponds to 6
    bindtextdomain("coreutils", "/usr/local/share/locale");
    textdomain("coreutils");
    atexit(close_stdout);

    have_read_stdin = 0;
    serial_merge = 0;
    line_delim = '\n'; // Default line delimiter

    char *delimiters_str = (char *)"\t"; // Corresponds to local_30
    int opt;

    while (true) {
        int option_index = 0;
        opt = getopt_long(argc, argv, DAT_00101711, longopts, &option_index);

        if (opt == -1) {
            break; // End of options
        }

        switch (opt) {
            case 'd':
                delimiters_str = optarg; // Use optarg from getopt.h
                if (*optarg == '\0') {
                    delimiters_str = (char *)"\\0"; // Special case for empty delimiter list
                }
                break;
            case 's':
                serial_merge = 1;
                break;
            case 'z':
                line_delim = '\0';
                break;
            case 0: // Long options that don't have a short option equivalent
                if (strcmp(longopts[option_index].name, "help") == 0) {
                    usage(EXIT_SUCCESS);
                } else if (strcmp(longopts[option_index].name, "version") == 0) {
                    version_etc(_stdout, "paste", _Version,
                                proper_name_lite("David M. Ihnat", "David M. Ihnat"),
                                proper_name_lite("David MacKenzie", "David MacKenzie"),
                                NULL);
                    exit(EXIT_SUCCESS);
                }
                break;
            default: // Unknown option or error (getopt_long prints error by default)
                usage(EXIT_FAILURE);
                break;
        }
    }

    int num_files = argc - optind; // Use optind from getopt.h
    char **input_filenames = argv + optind;

    // If no files are specified, default to reading from stdin ("-")
    char **default_input_filenames = NULL;
    if (num_files == 0) {
        num_files = 1;
        default_input_filenames = (char **)xmalloc(sizeof(char *));
        default_input_filenames[0] = (char *)DAT_0010149c; // "-"
        input_filenames = default_input_filenames;
    }

    if (collapse_escapes(delimiters_str) != 0) {
        error(EXIT_FAILURE, 0, gettext("delimiter list ends with an unescaped backslash: %s"),
              quotearg_n_style_colon(0, 6, delimiters_str));
    }

    int paste_result;
    if (serial_merge == 0) { // Parallel merge
        paste_result = paste_parallel(num_files, input_filenames);
    } else { // Serial merge
        paste_result = paste_serial(num_files, input_filenames);
    }

    free(delims);
    if (default_input_filenames != NULL) {
        free(default_input_filenames);
    }

    if (have_read_stdin != 0) {
        // The original decompiled code attempts to fclose(_stdin) if have_read_stdin is set.
        // This is generally bad practice for standard streams, but coreutils does this for certain utilities.
        // Sticking to the decompiled intent, if `rpl_fclose` returns -1, it's an error.
        // The original error message was `DAT_0010149c` ("-"), which is nonsensical.
        // Using a more descriptive message.
        if (rpl_fclose(_stdin) == -1) {
            error(EXIT_FAILURE, errno, gettext("failed to close standard input"));
        }
    }

    return paste_result == EXIT_SUCCESS ? EXIT_SUCCESS : EXIT_FAILURE; // Return 0 for success, 1 for failure
}