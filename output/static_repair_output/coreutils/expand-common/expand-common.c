#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h> // For LONG_MAX
#include <stdarg.h> // For va_list in error function

// --- Global Variables (as inferred from usage) ---
static long max_column_width = 0;
static long *tab_list = NULL;
static long first_free_tab = 0;   // Number of active tab stops
static long n_tabs_allocated = 0; // Current allocated capacity for tab_list
static long extend_size = 0;
static long increment_size = 0;
static long tab_size = 0;
static char have_read_stdin = 0;
static char **file_list = NULL;
static char *stdin_argv[] = {"-", NULL}; // Default for stdin
static char *prev_file_0 = NULL;         // Stores the name of the previously processed file
static FILE *_stdin = NULL;              // Pointer to stdin FILE object, initialized in main/setup
static FILE *_stdout = NULL;             // Pointer to stdout FILE object, initialized in main/setup
static int exit_status = 0;

// --- Dummy Implementations for external/coreutils functions ---
// These are minimal to allow compilation and demonstrate logic.
// Real implementations would be much more complex.

// Mimics GNU error function behavior
void error(int status, int errnum, const char *format, ...) {
    va_list args;
    fprintf(stderr, "Error: ");
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    if (errnum != 0) {
        fprintf(stderr, ": %s", strerror(errnum));
    }
    fprintf(stderr, "\n");
    if (status != 0) {
        exit(status);
    }
}

// Mimics GNU gettext for internationalization
const char *gettext(const char *msgid) {
    return msgid; // No actual translation for this snippet
}

// Mimics GNU quote function
char *quote(const char *str) {
    // For simplicity, just return a duplicate. In real coreutils, it adds quotes.
    return strdup(str);
}

// Mimics GNU quotearg_n_style_colon
char *quotearg_n_style_colon(int n, int style, const char *arg) {
    // For simplicity, just return a duplicate.
    return strdup(arg);
}

// Mimics GNU fadvise
void fadvise(FILE *fp, int advice) {
    // No-op for this snippet
}

// Mimics GNU xpalloc for dynamic array reallocation
// Original call: tab_list = xpalloc(tab_list,&n_tabs_allocated,1,0xffffffffffffffff,8);
// This function typically handles reallocating an array, doubling its size, and updating the capacity variable.
void *xpalloc(void *ptr, long *n_allocated_ptr, size_t n_elements_to_add, size_t max_total_elements,
              size_t element_size) {
    size_t current_capacity = *n_allocated_ptr;
    size_t required_capacity = first_free_tab + n_elements_to_add; // first_free_tab is current count

    if (required_capacity > max_total_elements) {
        error(1, 0, gettext("allocation would exceed maximum allowed size"));
    }

    if (required_capacity > current_capacity) {
        size_t new_capacity = current_capacity == 0 ? 1 : current_capacity * 2;
        if (new_capacity < required_capacity) {
            new_capacity = required_capacity;
        }
        if (new_capacity > max_total_elements) {
            new_capacity = max_total_elements;
        }

        void *new_ptr = realloc(ptr, new_capacity * element_size);
        if (!new_ptr) {
            error(1, errno, gettext("memory allocation failed"));
        }
        *n_allocated_ptr = new_capacity;
        return new_ptr;
    }
    return ptr; // No reallocation needed
}

// Mimics GNU ximemdup0 for duplicating memory and null-terminating
void *ximemdup0(const void *ptr, size_t size) {
    void *new_ptr = malloc(size + 1);
    if (!new_ptr) {
        error(1, errno, gettext("memory allocation failed"));
    }
    memcpy(new_ptr, ptr, size);
    ((char *)new_ptr)[size] = '\0';
    return new_ptr;
}

// Mimics GNU streq for string equality
bool streq(const char *s1, const char *s2) {
    return strcmp(s1, s2) == 0;
}

// Mimics GNU rpl_fclose (replacement for fclose)
int rpl_fclose(FILE *fp) {
    return fclose(fp);
}

// Mimics GNU rpl_fopen (replacement for fopen)
FILE *rpl_fopen(const char *filename, const char *mode) {
    return fopen(filename, mode);
}

// Mimics GNU __errno_location()
// This is typically provided by glibc, but for a standalone snippet, we make a wrapper.
int *__errno_location() {
    return &errno;
}

// Mimics GNU ferror_unlocked (if it exists, otherwise use ferror)
int ferror_unlocked(FILE *fp) {
#ifdef __GLIBC__
    return ferror(fp); // No actual locking for this dummy
#else
    return ferror(fp);
#endif
}

// Mimics GNU clearerr_unlocked (if it exists, otherwise use clearerr)
void clearerr_unlocked(FILE *fp) {
#ifdef __GLIBC__
    clearerr(fp); // No actual locking for this dummy
#else
    clearerr(fp);
#endif
}

// --- Original Functions, fixed and refactored ---

// Function: set_max_column_width
void set_max_column_width(long width) {
    if (max_column_width < width) {
        max_column_width = width;
    }
}

// Function: add_tab_stop
void add_tab_stop(long tab_position) {
    long prev_tab_stop = (first_free_tab == 0) ? 0 : tab_list[first_free_tab - 1];
    long diff_from_prev = (tab_position < prev_tab_stop) ? 0 : tab_position - prev_tab_stop;

    if (first_free_tab == n_tabs_allocated) {
        tab_list = (long *)xpalloc(tab_list, &n_tabs_allocated, 1, 0xffffffffffffffffUL, sizeof(long));
    }
    tab_list[first_free_tab++] = tab_position;
    set_max_column_width(diff_from_prev);
}

// Function: set_extend_size
bool set_extend_size(long size) {
    if (extend_size != 0) {
        error(0, 0, gettext("\'/\' specifier only allowed with the last value"));
        return false;
    }
    extend_size = size;
    set_max_column_width(size);
    return true;
}

// Function: set_increment_size
bool set_increment_size(long size) {
    if (increment_size != 0) {
        error(0, 0, gettext("\'+\' specifier only allowed with the last value"));
        return false;
    }
    increment_size = size;
    set_max_column_width(size);
    return true;
}

// Function: parse_tab_stops
void parse_tab_stops(char *param_1) {
    bool parsing_number = false;
    long current_number_value = 0;
    bool had_slash = false;
    bool had_plus = false;
    char *number_start = NULL;
    bool success = true;

    char *current_char_ptr = param_1;

    while (*current_char_ptr != '\0' && success) {
        char c = *current_char_ptr;

        if (c == ',' || isspace((unsigned char)c)) {
            if (parsing_number) {
                if (had_slash) {
                    if (!set_extend_size(current_number_value)) {
                        success = false;
                    }
                } else if (had_plus) {
                    if (!set_increment_size(current_number_value)) {
                        success = false;
                    }
                } else {
                    add_tab_stop(current_number_value);
                }
            }
            // Reset for next number/specifier
            parsing_number = false;
            had_slash = false;
            had_plus = false;
            current_number_value = 0;
            number_start = NULL;
        } else if (c == '/') {
            if (parsing_number) {
                char *quoted_arg = quote(current_char_ptr);
                error(0, 0, gettext("\'/\' specifier not at start of number: %s"), quoted_arg);
                free(quoted_arg);
                success = false;
            }
            had_slash = true;
            had_plus = false;
        } else if (c == '+') {
            if (parsing_number) {
                char *quoted_arg = quote(current_char_ptr);
                error(0, 0, gettext("\'+\' specifier not at start of number: %s"), quoted_arg);
                free(quoted_arg);
                success = false;
            }
            had_plus = true;
            had_slash = false;
        } else if (isdigit((unsigned char)c)) {
            if (!parsing_number) {
                current_number_value = 0;
                parsing_number = true;
                number_start = current_char_ptr;
            }

            long digit = c - '0';
            // Check for overflow before multiplication and addition
            if (current_number_value > (LONG_MAX - digit) / 10) {
                size_t len = strspn(number_start, "0123456789");
                char *num_str = (char *)ximemdup0(number_start, len);
                char *quoted_num = quote(num_str);
                error(0, 0, gettext("tab stop is too large %s"), quoted_num);
                free(quoted_num);
                free(num_str);
                success = false;
                // Advance current_char_ptr past the large number to avoid re-parsing
                current_char_ptr += (len - 1); // Loop will increment it by 1 more
            } else {
                current_number_value = current_number_value * 10 + digit;
            }
        } else {
            char *quoted_arg = quote(current_char_ptr);
            error(0, 0, gettext("tab size contains invalid character(s): %s"), quoted_arg);
            free(quoted_arg);
            success = false;
        }
        current_char_ptr++;
    }

    // Post-loop logic: process the last number if any, or if loop terminated by '\0'
    if (success && parsing_number) {
        if (had_slash) {
            success = set_extend_size(current_number_value);
        } else if (had_plus) {
            success = set_increment_size(current_number_value);
        } else {
            add_tab_stop(current_number_value);
        }
    }

    if (!success) {
        exit(1);
    }
}

// Function: validate_tab_stops
void validate_tab_stops(long *tab_array, long num_tabs) {
    long last_tab_stop = 0;
    for (long i = 0; i < num_tabs; i++) {
        if (tab_array[i] == 0) {
            error(1, 0, gettext("tab size cannot be 0"));
        }
        if (tab_array[i] <= last_tab_stop) {
            error(1, 0, gettext("tab sizes must be ascending"));
        }
        last_tab_stop = tab_array[i];
    }
    if ((increment_size != 0) && (extend_size != 0)) {
        error(1, 0, gettext("\'/\' specifier is mutually exclusive with \'+\'"));
    }
}

// Function: finalize_tab_stops
void finalize_tab_stops(void) {
    validate_tab_stops(tab_list, first_free_tab);
    if (first_free_tab == 0) {
        max_column_width = extend_size;
        tab_size = extend_size;
        // The original code's comma operator and assignment in condition is unusual but valid.
        // It means: assign increment_size to max_column_width, assign increment_size to tab_size, then check if increment_size is 0.
        if ((extend_size == 0) &&
            (max_column_width = increment_size, tab_size = increment_size, increment_size == 0)) {
            max_column_width = 8;
            tab_size = max_column_width;
        }
    } else if ((first_free_tab == 1) && (extend_size == 0) && (increment_size == 0)) {
        tab_size = tab_list[0];
    } else {
        tab_size = 0;
    }
}

// Function: get_next_tab_column
long get_next_tab_column(long current_column, long *tab_list_idx_ptr, char *end_of_tabs_flag) {
    *end_of_tabs_flag = 0; // Indicates if no more explicit tab stops or implicit rules

    if (tab_size == 0) {
        long current_tab_list_index = *tab_list_idx_ptr;
        while (current_tab_list_index < first_free_tab) {
            long explicit_tab_stop = tab_list[current_tab_list_index];
            if (current_column < explicit_tab_stop) {
                *tab_list_idx_ptr = current_tab_list_index; // Update index for next call
                return explicit_tab_stop;
            }
            current_tab_list_index++;
        }
        *tab_list_idx_ptr = current_tab_list_index; // Update index to first_free_tab

        long implicit_tab_increment;
        if (extend_size == 0) {
            if (increment_size == 0) {
                *end_of_tabs_flag = 1; // Signal to use default tab size (e.g., 8)
                // Calculate next tab stop based on default 8 if no other rules apply
                implicit_tab_increment = 8 - (current_column % 8);
                if (implicit_tab_increment == 0)
                    implicit_tab_increment = 8; // If already on a multiple of 8, go to next multiple
            } else {
                implicit_tab_increment =
                    increment_size - (current_column - tab_list[first_free_tab - 1]) % increment_size;
                if (implicit_tab_increment == 0)
                    implicit_tab_increment = increment_size; // If already on a multiple, go to next multiple
            }
        } else {
            implicit_tab_increment = extend_size - (current_column % extend_size);
            if (implicit_tab_increment == 0)
                implicit_tab_increment = extend_size; // If already on a multiple, go to next multiple
        }

        long next_column = current_column + implicit_tab_increment;
        if (next_column < current_column) { // Simple overflow check for positive numbers
            error(1, 0, gettext("input line is too long"));
        }
        return next_column;

    } else {
        long next_column = current_column + (tab_size - (current_column % tab_size));
        if (next_column < current_column) { // Simple overflow check
            error(1, 0, gettext("input line is too long"));
        }
        return next_column;
    }
}

// Function: set_file_list
void set_file_list(char **files) {
    have_read_stdin = 0;
    file_list = files;
    if (files == NULL) {
        file_list = stdin_argv;
    }
}

// Function: next_file
long next_file(FILE *current_file) {
    // Handle cleanup of the previous file if it was opened
    if (current_file != NULL) {
        int current_errno_val = 0; // Store errno before other calls potentially change it

        if (ferror_unlocked(current_file) != 0) {
            current_errno_val = *__errno_location();
        }

        if (!streq(prev_file_0, "-")) { // If not stdin
            if (rpl_fclose(current_file) != 0) {
                current_errno_val = *__errno_location();
            }
        } else {
            clearerr_unlocked(current_file);
        }

        if (current_errno_val != 0) {
            char *quoted_arg = quotearg_n_style_colon(0, 3, prev_file_0);
            error(0, current_errno_val, "%s", quoted_arg);
            free(quoted_arg);
            exit_status = 1;
        }
    }

    // Iterate through the file_list to find the next file to open
    while (true) {
        char *current_filename = *file_list;
        if (current_filename == NULL) {
            file_list++; // Advance past the NULL terminator
            return 0;    // End of file list
        }
        file_list++;

        FILE *opened_file = NULL;
        if (!streq(current_filename, "-")) { // "-" is the conventional name for stdin
            opened_file = rpl_fopen(current_filename, "r"); // "r" is for read mode
        } else {
            have_read_stdin = 1;
            opened_file = _stdin;
        }

        if (opened_file != NULL) {
            prev_file_0 = current_filename;
            fadvise(opened_file, 2); // FADV_SEQUENTIAL
            return (long)opened_file; // Return the FILE* cast to long
        }

        // If fopen failed
        char *quoted_arg = quotearg_n_style_colon(0, 3, current_filename);
        error(0, *__errno_location(), "%s", quoted_arg);
        free(quoted_arg);
        exit_status = 1;
    }
}

// Function: cleanup_file_list_stdin
void cleanup_file_list_stdin(void) {
    if (have_read_stdin != 0) {
        if (rpl_fclose(_stdin) != 0) {
            error(1, *__errno_location(), gettext("standard input"));
        }
    }
}

// Function: emit_tab_list_info
void emit_tab_list_info(void) {
    fputs_unlocked(gettext("  -t, --tabs=LIST  use comma separated list of tab positions.\n"), _stdout);
    fputs_unlocked(gettext(
                       "                     The last specified position can be prefixed with \'/\'\n                     to specify a tab size to use after the last\n                     explicitly specified tab stop.  Also a prefix of \'+\'\n                     can be used to align remaining tab stops relative to\n                     the last specified tab stop instead of the first column\n"),
                   _stdout);
}

// A setup function to initialize global FILE pointers, typically called once at program start.
void setup_global_streams() {
    _stdin = stdin;
    _stdout = stdout;
}