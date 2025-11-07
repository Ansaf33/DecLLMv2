#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h> // For NULL, size_t
#include <libintl.h> // For gettext, bindtextdomain, textdomain
#include <locale.h> // For setlocale, LC_ALL
#include <getopt.h> // For getopt_long, struct option, optarg, optind
#include <ctype.h>  // For isspace
#include <limits.h> // For LONG_MAX

// --- Mocked/Global Variables and Function Declarations for coreutils context ---
// These are external to the snippet and would be provided by coreutils libraries.
// Defined here for compilation purposes only to make the snippet self-contained.

// Global variables (example values)
char *_program_name = "unexpand";
int _exit_status = 0;
size_t _max_column_width = 8192;
bool convert_entire_line = false;
char *_Version = "8.32";

// For stack canary. In real code, this is handled by GCC.
// For a standalone compilable snippet, we mock it.
long __stack_chk_guard = 0xDEADBEEF; // Dummy value

// Dummy function implementations
void emit_stdin_note(void) { /* fprintf(stderr, "Emit stdin note\n"); */ }
void emit_mandatory_arg_note(void) { /* fprintf(stderr, "Emit mandatory arg note\n"); */ }
void emit_tab_list_info(void) { /* fprintf(stderr, "Emit tab list info\n"); */ }
void emit_ancillary_info(const char *tool) { /* fprintf(stderr, "Emit ancillary info for %s\n", tool); */ }
void *ximalloc(size_t size) {
    void *ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    return ptr;
}
FILE *next_file(FILE *f) {
    static int file_idx = 0;
    if (f == NULL && file_idx == 0) { // First call, simulate stdin
        file_idx = 1;
        return stdin;
    } else if (f == stdin) { // Simulate end of stdin
        return NULL;
    }
    return NULL; // No more files
}
void write_error(void) { fprintf(stderr, "Write error occurred\n"); exit(1); }
long get_next_tab_column(long current_col, long *prev_tab_col, char *is_first_only_flag) {
    // Simplified dummy implementation: next tab stop is 8 characters away
    if (is_first_only_flag) *is_first_only_flag = 0; // Don't trigger first-only for this dummy
    long tab_size = 8;
    long next_stop = ((current_col / tab_size) + 1) * tab_size;
    *prev_tab_col = next_stop; // Update prev_tab_col
    return next_stop;
}
void error(int status, int errnum, const char *message) {
    fprintf(stderr, "Error: %s (status=%d, errno=%d)\n", message, status, errnum);
    exit(status);
}
void set_program_name(const char *name) { _program_name = (char *)name; }
void close_stdout(void) { /* fflush(stdout); */ }
void add_tab_stop(long val) { /* fprintf(stderr, "Add tab stop: %ld\n", val); */ }
void finalize_tab_stops(void) { /* fprintf(stderr, "Finalize tab stops\n"); */ }
void parse_tab_stops(const char *arg) { /* fprintf(stderr, "Parse tab stops: %s\n", arg); */ }
void set_file_list(char **list) { /* fprintf(stderr, "Set file list\n"); */ }
void cleanup_file_list_stdin(void) { /* fprintf(stderr, "Cleanup file list stdin\n"); */ }
const char *proper_name_lite(const char *s1, const char *s2) { return s1; }
void version_etc(FILE *stream, const char *prog_name, const char *package, const char *version, const char *authors, ...) {
    fprintf(stream, "%s (%s) %s\n", prog_name, package, version);
    fprintf(stream, "Copyright (C) 2024 %s.\n", authors);
}
void __stack_chk_fail(void) {
    fprintf(stderr, "Stack smashing detected!\n");
    exit(2);
}

// Global struct for getopt_long
static struct option const longopts[] = {
    {"all", no_argument, NULL, 'a'},
    {"first-only", no_argument, NULL, 0x80},
    {"tabs", required_argument, NULL, 't'},
    {"help", no_argument, NULL, -0x82},
    {"version", no_argument, NULL, -0x83},
    {NULL, 0, NULL, 0}
};
// --- End of Mocked/Global Declarations ---

// Function: usage
void usage(int exit_code) {
    if (exit_code == 0) {
        printf(gettext("Usage: %s [OPTION]... [FILE]...\n"), _program_name);
        fputs_unlocked(gettext("Convert blanks in each FILE to tabs, writing to standard output.\n"), stdout);
        emit_stdin_note();
        emit_mandatory_arg_note();
        fputs_unlocked(gettext(
                               "  -a, --all        convert all blanks, instead of just initial blanks\n"
                               "      --first-only  convert only leading sequences of blanks (overrides -a)\n"
                               "  -t, --tabs=N     have tabs N characters apart instead of 8 (enables -a)\n"
                               ), stdout);
        emit_tab_list_info();
        fputs_unlocked(gettext("      --help        display this help and exit\n"), stdout);
        fputs_unlocked(gettext("      --version     output version information and exit\n"), stdout);
        emit_ancillary_info("unexpand");
    } else {
        fprintf(stderr, gettext("Try '%s --help' for more information.\n"), _program_name);
    }
    exit(exit_code);
}

// Function: unexpand
void unexpand(void) {
    void *line_buffer_ptr = NULL;
    long stack_chk_guard_val = __stack_chk_guard;

    FILE *current_file = next_file(NULL);
    if (current_file == NULL) {
        if (stack_chk_guard_val != __stack_chk_guard) {
            __stack_chk_fail();
        }
        return;
    }

    line_buffer_ptr = ximalloc(_max_column_width);

    while (current_file != NULL) { // Loop through files
        unsigned char is_initial_blanks = 1;
        long current_column = 0;
        long tab_stop_column = 0;
        long prev_tab_stop_column = 0;
        unsigned char has_buffered_tab = 0;
        unsigned char prev_char_was_blank = 1;
        size_t buffer_idx = 0;

        int current_char;

        while (true) { // Loop through characters in a line
            current_char = getc_unlocked(current_file);

            if (current_char == EOF) {
                current_file = next_file(current_file);
                break; // End of current file, break inner loop to handle new file or exit
            }

            if (!is_initial_blanks) {
                if (putchar_unlocked(current_char) < 0) {
                    write_error();
                }
            } else {
                unsigned char current_char_is_blank = (isspace(current_char) != 0);

                if (!current_char_is_blank) { // Not a blank character
                    if (current_char == '\b') { // Backspace
                        current_column = current_column - (current_column != 0);
                        prev_tab_stop_column = prev_tab_stop_column - (prev_tab_stop_column != 0);
                        tab_stop_column = current_column;
                    } else {
                        current_column++;
                        if (current_column == 0) { // Overflow check for long line
                            error(1, 0, gettext("input line is too long"));
                        }
                    }

                    if (buffer_idx != 0) {
                        if ((1 < buffer_idx) && has_buffered_tab) {
                            ((char*)line_buffer_ptr)[0] = '\t';
                        }
                        if (fwrite_unlocked(line_buffer_ptr, 1, buffer_idx, stdout) != buffer_idx) {
                            write_error();
                        }
                        buffer_idx = 0;
                        has_buffered_tab = 0;
                    }

                    prev_char_was_blank = current_char_is_blank;
                    if (!convert_entire_line && !current_char_is_blank) {
                        is_initial_blanks = 0;
                    }
                    if (putchar_unlocked(current_char) < 0) {
                        write_error();
                    }

                } else { // It is a blank character
                    char is_tab_stop_char_flag_val;
                    tab_stop_column = get_next_tab_column(current_column, &prev_tab_stop_column, &is_tab_stop_char_flag_val);
                    if (is_tab_stop_char_flag_val != '\0') {
                        is_initial_blanks = 0;
                    }

                    if (!is_initial_blanks) {
                        if (buffer_idx != 0) {
                            if ((1 < buffer_idx) && has_buffered_tab) {
                                ((char*)line_buffer_ptr)[0] = '\t';
                            }
                            if (fwrite_unlocked(line_buffer_ptr, 1, buffer_idx, stdout) != buffer_idx) {
                                write_error();
                            }
                            buffer_idx = 0;
                            has_buffered_tab = 0;
                        }
                        prev_char_was_blank = current_char_is_blank;
                        if (putchar_unlocked(current_char) < 0) {
                            write_error();
                        }
                    } else {
                        if (current_char == '\t') {
                            current_column = tab_stop_column;
                            buffer_idx = has_buffered_tab ? 1 : 0;
                            if (has_buffered_tab) {
                                ((char*)line_buffer_ptr)[0] = '\t';
                            }
                        } else { // current_char is a space
                            current_column++;
                            if (prev_char_was_blank && current_column == tab_stop_column) {
                                current_char = '\t';
                                buffer_idx = has_buffered_tab ? 1 : 0;
                                if (has_buffered_tab) {
                                    ((char*)line_buffer_ptr)[0] = '\t';
                                }
                            } else if (current_column == tab_stop_column) {
                                has_buffered_tab = 1;
                            }
                            ((char*)line_buffer_ptr)[buffer_idx] = (char)current_char;
                            prev_char_was_blank = 1;
                            buffer_idx++;
                        }
                        if (buffer_idx != 0) {
                            if ((1 < buffer_idx) && has_buffered_tab) {
                                ((char*)line_buffer_ptr)[0] = '\t';
                            }
                            if (fwrite_unlocked(line_buffer_ptr, 1, buffer_idx, stdout) != buffer_idx) {
                                write_error();
                            }
                            buffer_idx = 0;
                            has_buffered_tab = 0;
                        }
                        prev_char_was_blank = current_char_is_blank;
                    }
                }
            }

            if (current_char == '\n') {
                break; // End of line, reset line-specific state for next line.
            }
        } // End of inner while(true) loop for characters
    } // End of outer while(current_file != NULL) loop for files

    free(line_buffer_ptr);
    if (stack_chk_guard_val != __stack_chk_guard) {
        __stack_chk_fail();
    }
}

// Function: main
int main(int argc, char **argv) {
    long stack_chk_guard_val = __stack_chk_guard;
    bool building_tab_stops = false;
    long current_tab_stop_val = 0;
    bool first_only_set = false;

    set_program_name(argv[0]);
    setlocale(LC_ALL, "");
    bindtextdomain("coreutils", "/usr/local/share/locale");
    textdomain("coreutils");
    atexit(close_stdout);

    while (true) {
        int opt_char = getopt_long(argc, argv, ",0123456789at:", longopts, NULL);
        if (opt_char == -1) {
            break; // End of options
        }

        switch (opt_char) {
            case 0x80: // --first-only
                first_only_set = true;
                break;
            case 't': // -t, --tabs=N
                convert_entire_line = true;
                parse_tab_stops(optarg);
                break;
            case 'a': // -a, --all
                convert_entire_line = true;
                break;
            case '?': // getopt_long returns '?' for unknown option or missing argument
                usage(1); // Exits
                break;
            case ',': // Separator for tab stops
                if (building_tab_stops) {
                    add_tab_stop(current_tab_stop_val);
                }
                building_tab_stops = false;
                current_tab_stop_val = 0; // Reset for next tab stop
                break;
            case -0x83: // --version (arbitrary negative value for long option)
                version_etc(stdout, "unexpand", "GNU coreutils", _Version, proper_name_lite("David MacKenzie", "David MacKenzie"), 0);
                exit(0);
                break;
            case -0x82: // --help (arbitrary negative value for long option)
                usage(0); // Exits
                break;
            default: // Digits '0'-'9' or other short options (if any)
                if (opt_char >= '0' && opt_char <= '9') {
                    if (!building_tab_stops) {
                        current_tab_stop_val = 0;
                        building_tab_stops = true;
                    }
                    long digit = opt_char - '0';
                    // Check for overflow before multiplication
                    if (current_tab_stop_val > LONG_MAX / 10 ||
                        (current_tab_stop_val == LONG_MAX / 10 && digit > LONG_MAX % 10)) {
                        error(1, 0, gettext("tab stop value is too large"));
                    }
                    current_tab_stop_val = current_tab_stop_val * 10 + digit;
                } else {
                    usage(1); // Treat as a general error for unhandled/invalid option
                }
                break;
        }
    }

    // Post-loop processing
    if (first_only_set) {
        convert_entire_line = false; // --first-only overrides -a
    }
    if (building_tab_stops) {
        add_tab_stop(current_tab_stop_val);
    }
    finalize_tab_stops();

    char **file_list_ptr;
    if (optind < argc) {
        file_list_ptr = &argv[optind];
    } else {
        file_list_ptr = NULL;
    }
    set_file_list(file_list_ptr);

    unexpand();
    cleanup_file_list_stdin();

    if (stack_chk_guard_val != __stack_chk_guard) {
        __stack_chk_fail();
    }
    return _exit_status;
}