#define _GNU_SOURCE // For getline, ferror_unlocked, fputs_unlocked, strcasecmp, error
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <locale.h>
#include <libintl.h>
#include <getopt.h>
#include <error.h>
#include <errno.h>
#include <fnmatch.h>
#include <ctype.h>
#include <stdarg.h> // For va_list in error stub

// Global variables used in the snippet
char *program_name;
bool print_ls_colors = false;
const char *VERSION = "1.0"; // Placeholder for _Version

// --- OBSTACK IMPLEMENTATION (Simplified for compilation) ---
// This structure mimics the memory layout implied by the original snippet's access patterns.
struct obstack {
    char *_16_8_; // object_start / current_object_base
    char *_24_8_; // next_free / current_write_ptr
    char *_32_8_; // chunk_limit / end_of_buffer
    long _48_8_;  // alignment_mask (unused in this snippet's logic, but present)
    void *(*malloc_fn)(size_t);
    void (*free_fn)(void *);
};

struct obstack lsc_obstack;

// Initializes the obstack.
void rpl_obstack_begin(struct obstack *ob, size_t chunk_size_hint, size_t alignment_hint,
                       void *(*malloc_fn)(size_t), void (*free_fn)(void *)) {
    ob->malloc_fn = malloc_fn;
    ob->free_fn = free_fn;
    ob->_48_8_ = alignment_hint ? alignment_hint - 1 : 0; // Alignment mask
    
    size_t actual_chunk_size = chunk_size_hint > 0 ? chunk_size_hint : 4096;
    
    ob->_16_8_ = (char *)ob->malloc_fn(actual_chunk_size);
    if (!ob->_16_8_) {
        fprintf(stderr, "Fatal: obstack initial allocation failed.\n");
        exit(EXIT_FAILURE);
    }
    ob->_24_8_ = ob->_16_8_; // Current write pointer starts at the beginning
    ob->_32_8_ = ob->_16_8_ + actual_chunk_size; // Chunk limit
}

// Reallocates a new, larger chunk for the obstack when capacity is reached.
void rpl_obstack_newchunk(struct obstack *ob, size_t min_needed) {
    size_t current_used = ob->_24_8_ - ob->_16_8_; // Data already written in current object
    size_t current_chunk_capacity = ob->_32_8_ - ob->_16_8_; // Current total chunk size
    
    size_t new_chunk_capacity = current_chunk_capacity * 2;
    if (new_chunk_capacity < current_used + min_needed) {
        new_chunk_capacity = current_used + min_needed + 1024; // Ensure enough space
    }
    
    char *new_chunk = (char *)ob->malloc_fn(new_chunk_capacity);
    if (!new_chunk) {
        fprintf(stderr, "Fatal: obstack reallocation failed.\n");
        exit(EXIT_FAILURE);
    }
    memcpy(new_chunk, ob->_16_8_, current_used); // Copy existing data
    ob->free_fn(ob->_16_8_); // Free old chunk
    
    ob->_16_8_ = new_chunk;
    ob->_24_8_ = new_chunk + current_used;
    ob->_32_8_ = new_chunk + new_chunk_capacity;
}

// Appends a single character to the obstack, growing if necessary.
void obstack_1grow_char(struct obstack *ob, char c) {
    if (ob->_24_8_ >= ob->_32_8_) {
        rpl_obstack_newchunk(ob, 1);
    }
    *ob->_24_8_++ = c;
}

// Frees all memory associated with the obstack.
void obstack_free_all(struct obstack *ob) {
    if (ob->_16_8_) {
        ob->free_fn(ob->_16_8_);
        ob->_16_8_ = NULL;
    }
    ob->_24_8_ = NULL;
    ob->_32_8_ = NULL;
}

// --- STUB FUNCTIONS (Simplified versions of GNU coreutils functions) ---

// Sets the program name for error messages.
void set_program_name(const char *name) {
    program_name = (char *)name;
}

// Closes stdout and handles potential write errors.
void close_stdout(void) {
    if (fclose(stdout) != 0) {
        error(0, errno, "%s: write error", program_name);
        exit(EXIT_FAILURE);
    }
}

// Placeholder for ancillary information output (e.g., license, bug reports).
void emit_ancillary_info(const char *program) {
    (void)program; // Suppress unused parameter warning
}

// Returns a pointer to the last component of a path (filename).
const char *last_component(const char *path) {
    const char *p = strrchr(path, '/');
    return p ? p + 1 : path;
}

// Case-sensitive string equality check.
bool streq(const char *s1, const char *s2) {
    return strcmp(s1, s2) == 0;
}

// Wrapper for isspace from ctype.h.
int c_isspace(int c) {
    return isspace(c);
}

// Allocates memory, copies N bytes from S, and null-terminates the result.
char *ximemdup0(const char *s, size_t n) {
    char *p = (char *)malloc(n + 1);
    if (!p) {
        fprintf(stderr, "Fatal: memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }
    memcpy(p, s, n);
    p[n] = '\0';
    return p;
}

// Case-insensitive string comparison.
int c_strcasecmp(const char *s1, const char *s2) {
    return strcasecmp(s1, s2);
}

// Placeholder for quoting arguments for error messages.
const char *quotearg_n_style_colon(int n, int style, const char *arg) {
    (void)n; (void)style; // Unused parameters
    return arg;
}

// Simplified GNU error function.
void error(int status, int errnum, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "%s: ", program_name ? program_name : "dircolors");
    vfprintf(stderr, fmt, args);
    if (errnum != 0) {
        fprintf(stderr, ": %s", strerror(errnum));
    }
    fprintf(stderr, "\n");
    va_end(args);
    if (status != 0) {
        exit(status);
    }
}

// Wrapper for the standard getline function.
ssize_t rpl_getline(char **lineptr, size_t *n, FILE *stream) {
    return getline(lineptr, n, stream);
}

// Wrapper for the standard freopen function.
FILE *freopen_safer(const char *path, const char *mode, FILE *stream) {
    return freopen(path, mode, stream);
}

// Wrapper for the standard fclose function.
int rpl_fclose(FILE *stream) {
    return fclose(stream);
}

// Placeholder for quoting strings.
const char *quote(const char *str) {
    return str;
}

// Placeholder for formatting author names.
const char *proper_name_lite(const char *s1, const char *s2) {
    (void)s2; // Unused parameter
    return s1;
}

// Placeholder for printing version and copyright information.
void version_etc(FILE *stream, const char *program, const char *package,
                 const char *version, const char *authors, ...) {
    fprintf(stream, "%s (%s) %s\n", program, package, version);
    fprintf(stream, "Copyright (C) 2023 Free Software Foundation, Inc.\n");
    fprintf(stream, "License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\n");
    fprintf(stream, "This is free software: you are free to change and redistribute it.\n");
    fprintf(stream, "There is NO WARRANTY, to the extent permitted by law.\n\n");
    fprintf(stream, "Written by %s.\n", authors);
}

// --- CONSTANTS ---
const char *SHELL_BASH = "bash"; // Example for DAT_00103007
const char *SHELL_CSH = "csh";   // Example for DAT_0010333e
const char *TERM_KEYWORD = "TERM"; // Example for DAT_0010334b
const char *STDIN_FILENAME = "-"; // Example for DAT_001033e0
const char *READ_MODE = "r";      // Example for DAT_001033e2
const char *FILE_ERROR_MSG = "%s: %s"; // Example for DAT_001033e4
const char *GETOPT_SHORT_OPTIONS = "bcp"; // Example for DAT_0010341c

// ANSI escape sequences for --print-ls-colors output
const char *LS_COLORS_PREFIX_ESC = "\033["; // Example for DAT_00103343
const char *LS_COLORS_SUFFIX_ESC = "\033[0m"; // Example for DAT_00103346

// Global arrays for `slack_codes` and `ls_codes`.
// These are simplified examples based on typical dircolors keywords and their corresponding LS_COLORS codes.
const char *slack_codes[] = {
    "NORMAL", "FILE", "DIR", "LINK", "FIFO", "SOCK", "BLK", "CHR", "ORPHAN",
    "MISSING", "SETUID", "SETGID", "CAPABILITY", "STICKY_OTHER_WRITABLE",
    "OTHER_WRITABLE", "STICKY", "EXEC", "DOOR", "BAD_CAP", "MULTI_HARDLINK",
    NULL // Sentinel
};

const char *ls_codes[] = {
    "no", "fi", "di", "ln", "pi", "so", "bd", "cd", "or",
    "mi", "su", "sg", "ca", "tw",
    "ow", "st", "ex", "do", "bc", "mh",
    NULL // Sentinel
};

// Internal database lines for `--print-database` and default parsing.
// This is a simplified representation of `dircolors`'s built-in database.
const char *internal_db_lines[] = {
    "# Configuration file for dircolors, a utility to help you set the",
    "# LS_COLORS environment variable.",
    "NORMAL 0",
    "FILE 0",
    "DIR 01;34",
    "LINK 01;36",
    "FIFO 40;33;01",
    "SOCK 01;35",
    "BLK 40;33;01",
    "CHR 40;33;01",
    "ORPHAN 40;31;01",
    "MISSING 00",
    "SETUID 37;41",
    "SETGID 30;43",
    "CAPABILITY 30;41",
    "STICKY_OTHER_WRITABLE 30;42",
    "OTHER_WRITABLE 30;43",
    "STICKY 37;44",
    "EXEC 01;32",
    "DOOR 01;35",
    "BAD_CAP 40;31;01",
    "MULTI_HARDLINK 00",
    // Common extensions/types, simplified
    ".tar=01;31", ".tgz=01;31", ".arc=01;31", ".arj=01;31", ".taz=01;31", ".lha=01;31",
    ".lz4=01;31", ".lzma=01;31", ".tlz=01;31", ".txz=01;31", ".tzo=01;31", ".t7z=01;31",
    ".zip=01;31", ".z=01;31", ".dz=01;31", ".gz=01;31", ".lrz=01;31", ".lz=01;31",
    ".lzo=01;31", ".xz=01;31", ".bz2=01;31", ".bz=01;31", ".tz=01;31", ".deb=01;31",
    ".rpm=01;31", ".jar=01;31", ".rar=01;31", ".ace=01;31", ".zoo=01;31", ".cpio=01;31",
    ".7z=01;31", ".iso=01;31", ".dmg=01;31", ".flac=01;35", ".mp3=01;35", ".mp4=01;35",
    ".ogg=01;35", ".wav=01;35", ".au=01;35", ".avi=01;35", ".mov=01;35", ".mpg=01;35",
    ".mpeg=01;35", ".m2v=01;35", ".mkv=01;35", ".webm=01;35", ".ogm=01;35", ".wmv=01;35",
    ".xvid=01;35", ".flv=01;35", ".vob=01;35", ".doc=01;36", ".docx=01;36", ".pdf=01;36",
    ".xls=01;36", ".xlsx=01;36", ".ppt=01;36", ".pptx=01;36", ".odt=01;36", ".ods=01;36",
    ".odp=01;36", ".jpg=01;35", ".jpeg=01;35", ".gif=01;35", ".bmp=01;35", ".pbm=01;35",
    ".pgm=01;35", ".ppm=01;35", ".tga=01;35", ".xbm=01;35", ".xpm=01;35", ".tif=01;35",
    ".tiff=01;35", ".png=01;35", ".svg=01;35", ".svgz=01;35", ".mng=01;35", ".pcx=01;35",
    "NORMAL", // Sentinel to mark end of database for parsing
    NULL      // Final sentinel for the array itself
};

// long_options array for getopt_long
static const struct option long_options[] = {
    {"sh", no_argument, NULL, 'b'},
    {"bourne-shell", no_argument, NULL, 'b'},
    {"csh", no_argument, NULL, 'c'},
    {"c-shell", no_argument, NULL, 'c'},
    {"print-database", no_argument, NULL, 'p'},
    {"print-ls-colors", no_argument, NULL, 0x80}, // Custom value for this option
    {"help", no_argument, NULL, -0x82}, // Custom value for --help
    {"version", no_argument, NULL, -0x83}, // Custom value for --version
    {NULL, 0, NULL, 0}
};

// Function prototypes
void usage(int status);
int guess_shell_syntax(void);
void parse_line(char *line, char **key, char **value);
void append_quoted(const char *param_1);
void append_entry(char prefix_char, const char *key, const char *value);
bool dc_parse_stream(FILE *stream, const char *filename);
bool dc_parse_file(const char *filename);

// Function: usage
void usage(int status) {
    if (status == 0) {
        printf(gettext("Usage: %s [OPTION]... [FILE]\n"), program_name);
        fputs(gettext(
            "Output commands to set the LS_COLORS environment variable.\n\n"
            "Determine format of output:\n"
            "  -b, --sh, --bourne-shell    output Bourne shell code to set LS_COLORS\n"
            "  -c, --csh, --c-shell        output C shell code to set LS_COLORS\n"
            "  -p, --print-database        output defaults\n"
            "      --print-ls-colors       output fully escaped colors for display\n"
        ), stdout);
        fputs(gettext("      --help        display this help and exit\n"), stdout);
        fputs(gettext("      --version     output version information and exit\n"), stdout);
        fputs(gettext(
            "\nIf FILE is specified, read it to determine which colors to use for which\n"
            "file types and extensions.  Otherwise, a precompiled database is used.\n"
            "For details on the format of these files, run 'dircolors --print-database'.\n"
        ), stdout);
        emit_ancillary_info("dircolors");
    } else {
        fprintf(stderr, gettext("Try '%s --help' for more information.\n"), program_name);
    }
    exit(status);
}

// Function: guess_shell_syntax
// Returns 0 for Bourne-like shell, 1 for C-like shell, 2 if shell is unknown or unset.
int guess_shell_syntax(void) {
    const char *shell_env = getenv("SHELL");
    if (!shell_env || *shell_env == '\0') {
        return 2; // Indicates unknown, will be handled in main
    }
    const char *shell_name = last_component(shell_env);
    // Assuming SHELL_BASH covers Bourne-like shells (sh, bash, ksh)
    if (streq(shell_name, SHELL_BASH)) {
        return 0; // Bourne shell
    }
    // Assuming SHELL_CSH covers C-like shells (csh, tcsh)
    if (streq(shell_name, SHELL_CSH)) {
        return 1; // C shell
    }
    return 0; // Default to Bourne if not recognized
}

// Function: parse_line
// Parses a single line into a key and a value, trimming whitespace.
void parse_line(char *line, char **key, char **value) {
    *key = NULL;
    *value = NULL;

    char *p = line;

    // Skip leading whitespace
    while (c_isspace((unsigned char)*p)) {
        p++;
    }

    if (*p == '\0' || *p == '#') {
        return; // Empty line or comment
    }

    char *key_start = p;
    // Find end of key (first token)
    while (!c_isspace((unsigned char)*p) && *p != '\0') {
        p++;
    }
    *key = ximemdup0(key_start, p - key_start);

    // Skip whitespace between key and value
    while (c_isspace((unsigned char)*p)) {
        p++;
    }

    if (*p == '\0' || *p == '#') {
        return; // No value found, or comment after key
    }

    char *value_start = p;
    // Find end of value (second token, up to comment or end of line)
    while (*p != '\0' && *p != '#') {
        p++;
    }
    // Trim trailing whitespace from value
    char *value_end = p;
    while (value_end > value_start && c_isspace((unsigned char)*(value_end - 1))) {
        value_end--;
    }
    *value = ximemdup0(value_start, value_end - value_start);
}

// Function: append_quoted
// Appends a string to the global obstack, applying specific quoting rules.
void append_quoted(const char *param_1) {
    const char *p = param_1;
    bool needs_backslash_escape_next_special_char = true;

    while (*p != '\0') {
        char c = *p;

        if (!print_ls_colors) { // If not printing raw LS_COLORS values
            if (c == '^' || c == '\\') {
                needs_backslash_escape_next_special_char = !needs_backslash_escape_next_special_char;
            } else {
                if (c == '\'') {
                    // Special handling for single quote: '\'''
                    obstack_1grow_char(&lsc_obstack, '\'');
                    obstack_1grow_char(&lsc_obstack, '\\');
                    obstack_1grow_char(&lsc_obstack, '\'');
                    needs_backslash_escape_next_special_char = true; // Reset state
                } else if (c == '=' || c == ':') {
                    if (needs_backslash_escape_next_special_char) {
                        obstack_1grow_char(&lsc_obstack, '\\');
                    }
                } else {
                    needs_backslash_escape_next_special_char = true; // Reset state for other characters
                }
            }
        }
        obstack_1grow_char(&lsc_obstack, c);
        p++;
    }
}

// Function: append_entry
// Appends a key-value pair to the LS_COLORS string in the obstack,
// applying different formatting based on `print_ls_colors` and `prefix_char`.
void append_entry(char prefix_char, const char *key, const char *value) {
    if (print_ls_colors) {
        append_quoted(LS_COLORS_PREFIX_ESC); // "\033["
        append_quoted(value);
        obstack_1grow_char(&lsc_obstack, 'm');
    }

    if (prefix_char != '\0') {
        obstack_1grow_char(&lsc_obstack, prefix_char);
    }
    
    append_quoted(key);

    obstack_1grow_char(&lsc_obstack, print_ls_colors ? '\t' : '='); // Separator
    
    append_quoted(value);

    if (print_ls_colors) {
        append_quoted(LS_COLORS_SUFFIX_ESC); // "\033[0m"
    }

    obstack_1grow_char(&lsc_obstack, print_ls_colors ? '\n' : ':'); // Entry terminator
}

// Function: dc_parse_stream
// Parses a stream (file or internal database) to build the LS_COLORS string.
typedef int TermMatchState;
enum {
    TERM_MATCH_INITIAL = 3,
    TERM_MATCH_ACTIVE = 2,
    TERM_MATCH_INACTIVE = 0,
    TERM_MATCH_OVERRIDDEN = 1
};

bool dc_parse_stream(FILE *stream, const char *filename) {
    bool success = true;
    long line_number = 0;
    char *line_buffer = NULL;
    size_t line_buffer_size = 0;

    TermMatchState term_match_state = TERM_MATCH_INITIAL;
    const char *term_env = getenv("TERM");
    if (!term_env || *term_env == '\0') {
        term_env = "none";
    }
    const char *colorterm_env = getenv("COLORTERM");
    if (!colorterm_env) {
        colorterm_env = "";
    }

    int internal_db_idx = 0;

    while (true) {
        line_number++;
        char *current_line_text = NULL;

        if (!stream) { // Internal database (stream is NULL)
            current_line_text = (char *)internal_db_lines[internal_db_idx];
            if (current_line_text == NULL) {
                break; // Reached end of internal_db_lines array
            }
            if (streq(current_line_text, "NORMAL") && internal_db_idx > 0) {
                break; // Sentinel "NORMAL" marks the end of entries for parsing
            }
            internal_db_idx++;
        } else { // File stream
            ssize_t bytes_read = rpl_getline(&line_buffer, &line_buffer_size, stream);
            if (bytes_read < 0) {
                if (ferror(stream)) {
                    error(0, errno, gettext("%s: read error"), quotearg_n_style_colon(0, 3, filename));
                    success = false;
                }
                break; // EOF or error
            }
            // getline includes the newline, remove it for parsing
            if (bytes_read > 0 && line_buffer[bytes_read - 1] == '\n') {
                line_buffer[bytes_read - 1] = '\0';
            }
            current_line_text = line_buffer;
        }

        char *key = NULL;
        char *value = NULL;
        parse_line(current_line_text, &key, &value);

        if (key) {
            if (!value) {
                error(0, 0, gettext("%s:%td: invalid line; missing second token"),
                      quotearg_n_style_colon(0, 3, filename), line_number);
                success = false;
                free(key);
            } else {
                bool is_recognized_keyword = false;
                if (c_strcasecmp(key, TERM_KEYWORD) == 0) {
                    is_recognized_keyword = true;
                    if (term_match_state != TERM_MATCH_ACTIVE) {
                        if (fnmatch(value, term_env, 0) == 0) {
                            term_match_state = TERM_MATCH_ACTIVE;
                        } else {
                            term_match_state = TERM_MATCH_INACTIVE;
                        }
                    }
                } else if (c_strcasecmp(key, "COLORTERM") == 0) {
                    is_recognized_keyword = true;
                    if (term_match_state != TERM_MATCH_ACTIVE) {
                        if (fnmatch(value, colorterm_env, 0) == 0) {
                            term_match_state = TERM_MATCH_ACTIVE;
                        } else {
                            term_match_state = TERM_MATCH_INACTIVE;
                        }
                    }
                } else { // Not TERM or COLORTERM
                    if (term_match_state == TERM_MATCH_ACTIVE) {
                        term_match_state = TERM_MATCH_OVERRIDDEN; // Matched, but now processing general entries
                    }
                    if (term_match_state == TERM_MATCH_INACTIVE) {
                        is_recognized_keyword = false; // All subsequent entries are "unrecognized" if TERM/COLORTERM didn't match
                    } else if (c_strcasecmp(key, "OPTIONS") == 0 ||
                               c_strcasecmp(key, "COLOR") == 0 ||
                               c_strcasecmp(key, "EIGHTBIT") == 0) {
                        is_recognized_keyword = true; // Recognized, but not added to LS_COLORS
                    } else {
                        // Check against `slack_codes` for known types
                        int i = 0;
                        while (slack_codes[i] && c_strcasecmp(key, slack_codes[i]) != 0) {
                            i++;
                        }
                        if (slack_codes[i]) { // Found in `slack_codes`
                            append_entry('\0', ls_codes[i], value);
                            is_recognized_keyword = true;
                        } else if (*key == '.') { // File extension
                            append_entry('*', key, value);
                            is_recognized_keyword = true;
                        } else if (*key == '*') { // Glob pattern
                            append_entry('\0', key, value);
                            is_recognized_keyword = true;
                        } else {
                            is_recognized_keyword = false; // Truly unrecognized keyword
                        }
                    }
                }
                
                // Report error if keyword is unrecognized AND a TERM/COLORTERM match was active
                if (!is_recognized_keyword && (term_match_state == TERM_MATCH_ACTIVE || term_match_state == TERM_MATCH_OVERRIDDEN)) {
                    error(0, 0, gettext("%s:%td: unrecognized keyword %s"),
                          (filename == NULL) ? gettext("<internal>") : quotearg_n_style_colon(0, 3, filename),
                          line_number, key);
                    success = false;
                }
                free(key);
                free(value);
            }
        }
    }
    free(line_buffer);
    return success;
}

// Function: dc_parse_file
// Opens a file and passes it to `dc_parse_stream`.
bool dc_parse_file(const char *filename) {
    bool success = true;
    // Check for stdin or attempt to open the file
    if (streq(filename, STDIN_FILENAME) || freopen_safer(filename, READ_MODE, stdin)) {
        success = dc_parse_stream(stdin, filename);
        if (rpl_fclose(stdin) != 0) { // Close the file if it was opened
            error(0, errno, gettext(FILE_ERROR_MSG), quotearg_n_style_colon(0, 3, filename), strerror(errno));
            success = false;
        }
    } else {
        error(0, errno, gettext(FILE_ERROR_MSG), quotearg_n_style_colon(0, 3, filename), strerror(errno));
        success = false;
    }
    return success;
}

// Function: main
// Main entry point of the program, handles command-line arguments and orchestrates logic.
enum {
    BOURNE_SHELL = 0,
    C_SHELL = 1,
    GUESS_SHELL = 2 // Initial state, will try to guess shell syntax
};

int main(int argc, char *argv[]) {
    bool success = true;
    int shell_syntax = GUESS_SHELL;
    bool print_database = false;

    set_program_name(argv[0]);
    setlocale(LC_ALL, ""); // Set locale for internationalization
    bindtextdomain("coreutils", "/usr/local/share/locale");
    textdomain("coreutils");
    atexit(close_stdout); // Register cleanup function

    int opt;
    // Parse command-line options
    while ((opt = getopt_long(argc, argv, GETOPT_SHORT_OPTIONS, long_options, NULL)) != -1) {
        switch (opt) {
            case 'b': // --sh, --bourne-shell
                shell_syntax = BOURNE_SHELL;
                break;
            case 'c': // --csh, --c-shell
                shell_syntax = C_SHELL;
                break;
            case 'p': // --print-database
                print_database = true;
                break;
            case 0x80: // --print-ls-colors (custom value from long_options)
                print_ls_colors = true;
                break;
            case -0x82: // --help (custom value)
                usage(0);
                break;
            case -0x83: // --version (custom value)
                version_etc(stdout, "dircolors", "GNU coreutils", VERSION, proper_name_lite("H. Peter Anvin", "H. Peter Anvin"));
                exit(0);
            default: // Unknown or invalid option
                usage(1);
        }
    }

    // Check for mutually exclusive options
    if ((print_ls_colors || print_database) && shell_syntax != GUESS_SHELL) {
        error(0, 0, gettext("the options to output non shell syntax,\nand to select a shell syntax are mutually exclusive"));
        usage(1);
    }
    if (print_database && print_ls_colors) {
        error(0, 0, gettext("options --print-database and --print-ls-colors are mutually exclusive"));
        usage(1);
    }

    // Check for extra operands (file arguments)
    int extra_operands_count = argc - optind;
    if ((print_database && extra_operands_count > 0) || (!print_database && extra_operands_count > 1)) {
        const char *extra_operand = argv[optind + (print_database ? 0 : 1)];
        error(0, 0, gettext("extra operand %s"), quote(extra_operand));
        if (print_database) {
            fprintf(stderr, "%s\n", gettext("file operands cannot be combined with --print-database (-p)"));
        }
        usage(1);
    }

    if (print_database) {
        // Output the internal database lines directly
        for (int i = 0; internal_db_lines[i] != NULL; ++i) {
            puts(internal_db_lines[i]);
        }
    } else {
        // Determine shell syntax if not explicitly set and not printing raw LS_COLORS
        if (shell_syntax == GUESS_SHELL && !print_ls_colors) {
            shell_syntax = guess_shell_syntax();
            if (shell_syntax == GUESS_SHELL) { // Still unknown after guessing
                error(1, 0, gettext("no SHELL environment variable, and no shell type option given"));
            }
        }

        rpl_obstack_begin(&lsc_obstack, 0, 0, malloc, free); // Initialize obstack

        if (argc == optind) { // No file argument provided, use internal database
            success = dc_parse_stream(NULL, NULL);
        } else { // File argument provided
            success = dc_parse_file(argv[optind]);
        }
        
        if (success) {
            obstack_1grow_char(&lsc_obstack, '\0'); // Null-terminate the accumulated string
            char *ls_colors_output = lsc_obstack._16_8_; // Get pointer to the start of the string
            size_t output_length = lsc_obstack._24_8_ - lsc_obstack._16_8_ - 1; // Length excluding null terminator

            if (!print_ls_colors) {
                const char *prefix_str = (shell_syntax == BOURNE_SHELL) ? "LS_COLORS='" : "setenv LS_COLORS '";
                const char *suffix_str = (shell_syntax == BOURNE_SHELL) ? "';\nexport LS_COLORS\n" : "'\n";
                
                fputs(prefix_str, stdout);
                fwrite(ls_colors_output, 1, output_length, stdout);
                fputs(suffix_str, stdout);
            } else {
                fwrite(ls_colors_output, 1, output_length, stdout); // Just print the raw LS_COLORS value
            }
        }
    }

    obstack_free_all(&lsc_obstack); // Clean up obstack memory

    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}