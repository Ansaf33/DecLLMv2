#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <locale.h>
#include <libintl.h>
#include <stdbool.h>
#include <fcntl.h>
#include <getopt.h>

// --- Coreutils/Glibc specific extern declarations ---
// These are minimal declarations to allow compilation.
// In a real coreutils build, these would come from specific headers.
extern char *_program_name;
extern FILE *_stdout;
extern FILE *_stderr;
extern char *optarg;
extern int optind;

extern void emit_stdin_note(void);
extern void emit_mandatory_arg_note(void);
extern void emit_ancillary_info(const char *);
extern void set_program_name(const char *);
extern void close_stdout(void);
extern const struct option longopts[];

extern char *separator;
extern size_t sentinel_length;
extern bool separator_ends_record;
extern char *G_buffer;
extern size_t G_buffer_size;
extern size_t read_size;
extern size_t match_length;
extern char buffer_1[0x2000]; // Assuming a fixed buffer size of 8KB
extern size_t bytes_in_buffer_2;
extern bool have_read_stdin;

// Minimal regex types and functions, as used by GNU regex
typedef struct re_pattern_buffer
{
  void *__buffer;
  size_t __allocated;
  size_t __used;
  char *__fastmap;
  char *__translate;
  // Other fields might be present, but these are sufficient for compilation
} re_pattern_buffer;

typedef struct re_registers
{
  int num_regs;
  int start[10]; // Assuming default max 10 registers
  int end[10];
} re_registers;

extern re_pattern_buffer compiled_separator;
extern char compiled_separator_fastmap[256]; // Assuming common fastmap size
extern re_registers regs;

extern char *re_compile_pattern(const char *__pattern, size_t __length, re_pattern_buffer *__buffer);
extern int re_search(re_pattern_buffer *__buffer, const char *__string, int __length, int __start, int __range, re_registers *__regs);

extern char *quotearg_n_style_colon(int n, int style, const char *arg);
extern char *quotearg_style(int style, const char *arg);
extern void error(int status, int errnum, const char *format, ...);
extern int *__errno_location(void);
extern void xalloc_die(void);
extern void *xrealloc(void *ptr, size_t size);
extern void *xmalloc(size_t size);
extern char temp_stream(FILE **fp, char **name); // Returns char, but acts as boolean (1 for success)
extern ssize_t full_read(int fd, void *buf, size_t count);
extern bool streq(const char *s1, const char *s2); // Returns char, but acts as boolean
extern void xset_binary_mode(int fd, int mode);
extern const char *proper_name_lite(const char *s1, const char *s2);
extern void version_etc(FILE *stream, const char *package, const char *program, const char *version, ...);
extern void __stack_chk_fail(void); // For stack canary check

// Global constants/data from the original snippet (e.g., string literals)
extern const char DAT_00103d19[];
extern const char DAT_00103d83[]; // Likely string for "stdin"
extern const char DAT_00103df6[]; // Likely short options string for getopt_long
extern const char DAT_00103e15[]; // Likely regex error message format string
extern const char *_Version;
extern const char *default_file_list_0[]; // Array of char* for default files

// --- Function Prototypes ---
void usage(int status);
void output(void *data, long end_ptr_val);
bool tac_seekable(int fd, const char *filename, size_t file_size);
long copy_to_temp(FILE **out_fp, char **out_filename, int in_fd, const char *input_name);
bool tac_nonseekable(int in_fd, const char *filename);
bool tac_file(char *filename_arg);

// Function: usage
void usage(int status) {
    if (status == 0) {
        printf(gettext("Usage: %s [OPTION]... [FILE]...\n"), _program_name);
        fputs_unlocked(gettext("Write each FILE to standard output, last line first.\n"), _stdout);
        emit_stdin_note();
        emit_mandatory_arg_note();
        fputs_unlocked(gettext(
                               "  -b, --before             attach the separator before instead of after\n  -r, --regex              interpret the separator as a regular expression\n  -s, --separator=STRING   use STRING as the separator instead of newline\n"
                               ), _stdout);
        fputs_unlocked(gettext("      --help        display this help and exit\n"), _stdout);
        fputs_unlocked(gettext("      --version     output version information and exit\n"), _stdout);
        emit_ancillary_info(DAT_00103d19);
    } else {
        fprintf(_stderr, gettext("Try '%s --help' for more information.\n"), _program_name);
    }
    exit(status);
}

// Function: output
void output(void *data, long end_ptr_val) {
    if (data == NULL) {
        fwrite_unlocked(buffer_1, 1, bytes_in_buffer_2, _stdout);
        bytes_in_buffer_2 = 0;
    } else {
        size_t remaining_bytes = end_ptr_val - (long)data;
        char *current_src = (char *)data;

        while (remaining_bytes > 0) {
            size_t space_in_buffer = sizeof(buffer_1) - bytes_in_buffer_2;
            size_t bytes_to_copy = (remaining_bytes < space_in_buffer) ? remaining_bytes : space_in_buffer;

            memcpy(buffer_1 + bytes_in_buffer_2, current_src, bytes_to_copy);
            bytes_in_buffer_2 += bytes_to_copy;
            current_src += bytes_to_copy;
            remaining_bytes -= bytes_to_copy;

            if (bytes_in_buffer_2 == sizeof(buffer_1)) {
                fwrite_unlocked(buffer_1, 1, sizeof(buffer_1), _stdout);
                bytes_in_buffer_2 = 0;
            }
        }
    }
}

// Function: tac_seekable
bool tac_seekable(int fd, const char *filename, size_t file_size_param) {
    bool first_record_in_file = true;
    char first_separator_char = *separator;
    const char *separator_tail = separator + 1;
    size_t separator_tail_len = match_length - 1; // Used for strncmp with sentinel_length != 0

    size_t current_file_pos = file_size_param;
    if (current_file_pos % read_size != 0) {
        current_file_pos -= (current_file_pos % read_size);
        if (lseek(fd, current_file_pos, SEEK_SET) < 0) {
            error(0, *__errno_location(), gettext("%s: seek failed"), quotearg_n_style_colon(0, 3, filename));
            return false;
        }
    }

    ssize_t bytes_read_current_block;
    // Loop to skip trailing empty blocks (or seek to start if file is empty)
    for (bytes_read_current_block = read(fd, G_buffer, read_size);
         bytes_read_current_block == 0 && current_file_pos != 0;
         bytes_read_current_block = read(fd, G_buffer, read_size)) {
        current_file_pos = (current_file_pos < read_size) ? 0 : (current_file_pos - read_size);
        if (lseek(fd, current_file_pos, SEEK_SET) < 0) {
            error(0, *__errno_location(), gettext("%s: seek failed"), quotearg_n_style_colon(0, 3, filename));
            return false;
        }
    }

    if (bytes_read_current_block < 0) {
        error(0, *__errno_location(), gettext("%s: read error"), quotearg_n_style_colon(0, 3, filename));
        return false;
    }

    char *record_end_ptr = G_buffer + bytes_read_current_block;
    char *current_search_ptr = record_end_ptr;
    if (sentinel_length != 0) {
        current_search_ptr -= (match_length - 1);
    }
    
    bool continue_processing_blocks = true;
    while (continue_processing_blocks) {
        char *found_separator_ptr = NULL;
        if (sentinel_length == 0) { // Regex mode
            int search_len = current_search_ptr - G_buffer;
            if (search_len < 0) search_len = 0;

            if (search_len == 0 || (search_len == 1 && *G_buffer == '\0')) {
                 found_separator_ptr = G_buffer - 1; // Indicate no match in current tiny buffer
            } else {
                // The original re_search arguments are tricky; this is a direct translation.
                // Assuming re_search takes (buffer, string, length, start_offset, range, regs)
                // where length is total length, start_offset is where to start searching,
                // and range is direction/max length from start_offset.
                int search_result = re_search(&compiled_separator, G_buffer, search_len, search_len - 1, -search_len, &regs);
                
                if (search_result == -1) { // No match found
                    found_separator_ptr = G_buffer - 1;
                } else if (search_result == -2) {
                    error(1, 0, gettext("error in regular expression search"));
                    return false;
                } else { // Match found
                    match_length = regs.end[0] - regs.start[0];
                    found_separator_ptr = G_buffer + regs.start[0];
                }
            }
        } else { // Fixed string separator mode
            current_search_ptr--; // Start search from one char before current_search_ptr
            while (current_search_ptr >= G_buffer && (*current_search_ptr != first_separator_char ||
                   (separator_tail_len != 0 && strncmp(current_search_ptr + 1, separator_tail, separator_tail_len) != 0))) {
                current_search_ptr--;
            }
            found_separator_ptr = current_search_ptr;
        }

        if (found_separator_ptr < G_buffer) { // Separator not found in current block, need more data or end of file
            if (current_file_pos == 0) { // At the very beginning of the file
                output(G_buffer, (long)record_end_ptr); // Output remaining content
                return true;
            }

            size_t consumed_bytes = (long)record_end_ptr - (long)G_buffer;
            
            // Reallocate buffer if needed
            size_t required_buffer_space = consumed_bytes + read_size + sentinel_length;
            if (required_buffer_space > G_buffer_size) {
                size_t old_read_size = read_size;
                read_size *= 2;
                if (read_size == 0 || read_size < old_read_size) xalloc_die(); // Overflow check

                size_t new_G_buffer_size = sentinel_length + read_size * 4 + 2; // Original logic
                if (new_G_buffer_size <= G_buffer_size) xalloc_die(); // Should be > G_buffer_size
                G_buffer_size = new_G_buffer_size;

                char *old_buffer_base = G_buffer;
                if (sentinel_length == 0) old_buffer_base--; // Adjust for G_buffer = (void*)((long)G_buffer + 1)
                else old_buffer_base -= sentinel_length; // Adjust for G_buffer = sentinel_length + (long)G_buffer

                char *new_buffer_base = (char *)xrealloc(old_buffer_base, G_buffer_size);
                
                if (sentinel_length == 0) G_buffer = new_buffer_base + 1;
                else G_buffer = new_buffer_base + sentinel_length;
            }

            size_t bytes_to_read_this_pass = read_size;
            if (current_file_pos < read_size) {
                bytes_to_read_this_pass = current_file_pos;
                current_file_pos = 0;
            } else {
                current_file_pos -= read_size;
            }
            
            if (lseek(fd, current_file_pos, SEEK_SET) < 0) {
                error(0, *__errno_location(), gettext("%s: seek failed"), quotearg_n_style_colon(0, 3, filename));
                return false;
            }
            
            memmove(G_buffer + bytes_to_read_this_pass, G_buffer, consumed_bytes);
            
            ssize_t new_bytes_read = full_read(fd, G_buffer, bytes_to_read_this_pass);
            if (new_bytes_read < 0) {
                error(0, *__errno_location(), gettext("%s: read error"), quotearg_n_style_colon(0, 3, filename));
                return false;
            }
            if (new_bytes_read != (ssize_t)bytes_to_read_this_pass && current_file_pos != 0) {
                // If not at the beginning of the file, a short read might indicate an error.
                // The original code considered this an error.
                error(0, *__errno_location(), gettext("%s: read error"), quotearg_n_style_colon(0, 3, filename));
                return false;
            }
            
            record_end_ptr = G_buffer + new_bytes_read + consumed_bytes;
            current_search_ptr = record_end_ptr;
            if (sentinel_length != 0) {
                current_search_ptr -= (match_length - 1);
            }
        } else { // Separator found within current block
            if (!separator_ends_record) { // Attach separator before
                output(found_separator_ptr, (long)record_end_ptr);
                record_end_ptr = found_separator_ptr;
            } else { // Attach separator after
                char *output_start = found_separator_ptr + match_length;
                // Avoid outputting an empty record if it's the first one and the separator is at EOF
                if (!(first_record_in_file && output_start == record_end_ptr)) {
                    output(output_start, (long)record_end_ptr);
                }
                first_record_in_file = false;
                record_end_ptr = found_separator_ptr;
            }
            current_search_ptr = found_separator_ptr;
            if (sentinel_length != 0) {
                current_search_ptr += (1 - match_length);
            }
        }
    }
}

// Function: copy_to_temp
long copy_to_temp(FILE **out_fp, char **out_filename, int in_fd, const char *input_name) {
    long stack_canary = *(long *)((char *)__builtin_frame_address(0) + 0x28);
    off_t total_written = 0;
    FILE *temp_file = NULL;
    char *temp_filename = NULL;

    if (temp_stream(&temp_file, &temp_filename) != '\x01') { // temp_stream returns 1 on success
        if (stack_canary != *(long *)((char *)__builtin_frame_address(0) + 0x28)) { __stack_chk_fail(); }
        return -1;
    }

    ssize_t bytes_read;
    while ((bytes_read = read(in_fd, G_buffer, read_size)) > 0) {
        if (fwrite_unlocked(G_buffer, 1, bytes_read, temp_file) != (size_t)bytes_read) {
            error(0, *__errno_location(), gettext("%s: write error"), quotearg_n_style_colon(0, 3, temp_filename));
            if (stack_canary != *(long *)((char *)__builtin_frame_address(0) + 0x28)) { __stack_chk_fail(); }
            return -1;
        }
        total_written += bytes_read;
    }

    if (bytes_read < 0) { // read error
        error(0, *__errno_location(), gettext("%s: read error"), quotearg_n_style_colon(0, 3, input_name));
        if (stack_canary != *(long *)((char *)__builtin_frame_address(0) + 0x28)) { __stack_chk_fail(); }
        return -1;
    }

    if (fflush_unlocked(temp_file) == 0) {
        *out_fp = temp_file;
        *out_filename = temp_filename;
        if (stack_canary != *(long *)((char *)__builtin_frame_address(0) + 0x28)) { __stack_chk_fail(); }
        return total_written;
    } else {
        error(0, *__errno_location(), gettext("%s: write error"), quotearg_n_style_colon(0, 3, temp_filename));
        if (stack_canary != *(long *)((char *)__builtin_frame_address(0) + 0x28)) { __stack_chk_fail(); }
        return -1;
    }
}

// Function: tac_nonseekable
bool tac_nonseekable(int in_fd, const char *filename) {
    long stack_canary = *(long *)((char *)__builtin_frame_address(0) + 0x28);
    FILE *temp_file = NULL;
    char *temp_filename = NULL;
    off_t file_size = copy_to_temp(&temp_file, &temp_filename, in_fd, filename);
    bool result = false;

    if (file_size >= 0) {
        int temp_fd = fileno_unlocked(temp_file);
        result = tac_seekable(temp_fd, temp_filename, file_size);
    }
    
    if (stack_canary != *(long *)((char *)__builtin_frame_address(0) + 0x28)) {
        __stack_chk_fail();
    }
    return result;
}

// Function: tac_file
bool tac_file(char *filename_arg) {
    bool is_stdin = streq(filename_arg, DAT_00103d83);
    int fd;
    const char *filename_for_errors;
    bool success = true;

    if (!is_stdin) {
        fd = open(filename_arg, O_RDONLY);
        filename_for_errors = filename_arg;
        if (fd < 0) {
            error(0, *__errno_location(), gettext("failed to open %s for reading"), quotearg_style(4, filename_arg));
            return false;
        }
    } else {
        have_read_stdin = true;
        fd = STDIN_FILENO;
        filename_for_errors = gettext("standard input");
        xset_binary_mode(STDIN_FILENO, 0);
    }

    off_t file_size = lseek(fd, 0, SEEK_END);
    if (file_size < 0 || isatty(fd)) {
        success = tac_nonseekable(fd, filename_for_errors);
    } else {
        success = tac_seekable(fd, filename_for_errors, file_size);
    }

    if (!is_stdin) {
        if (close(fd) != 0) {
            error(0, *__errno_location(), gettext("%s: read error"), quotearg_n_style_colon(0, 3, filename_for_errors));
            success = false;
        }
    }
    return success;
}

// Function: main
int main(int argc, char *argv[]) {
    set_program_name(argv[0]);
    setlocale(LC_ALL, "");
    bindtextdomain("coreutils", "/usr/local/share/locale");
    textdomain("coreutils");
    atexit(close_stdout);

    separator = "\n";
    sentinel_length = 1;
    separator_ends_record = true;

    int opt_char;
    while ((opt_char = getopt_long(argc, argv, DAT_00103df6, longopts, NULL)) != -1) {
        switch (opt_char) {
            case 's':
                separator = optarg;
                break;
            case 'r':
                sentinel_length = 0; // Regex mode
                break;
            case 'b':
                separator_ends_record = false; // Before mode
                break;
            case -0x82: // Corresponds to --help (assuming -130 from decompiler)
                usage(0);
                break;
            case -0x83: // Corresponds to --version (assuming -131 from decompiler)
                version_etc(_stdout, DAT_00103d19, "GNU coreutils", _Version,
                            proper_name_lite("Jay Lepreau", "Jay Lepreau"),
                            proper_name_lite("David MacKenzie", "David MacKenzie"),
                            NULL);
                exit(0);
                break;
            default:
                usage(1); // Unknown option or argument error
                break;
        }
    }

    if (sentinel_length == 0) { // Regex mode post-processing
        if (*separator == '\0') {
            error(1, 0, gettext("separator cannot be empty"));
        }
        // Initialize compiled_separator structure
        compiled_separator.__buffer = NULL;
        compiled_separator.__allocated = 0;
        compiled_separator.__used = 0;
        compiled_separator.__fastmap = compiled_separator_fastmap;
        compiled_separator.__translate = NULL;

        char *regex_error_msg = re_compile_pattern(separator, strlen(separator), &compiled_separator);
        if (regex_error_msg != NULL) {
            error(1, 0, DAT_00103e15, regex_error_msg);
        }
    }

    if (*separator == '\0') {
        sentinel_length = 1; // Treat empty separator as single null byte implicitly
        match_length = 1;
    } else if (sentinel_length != 0) { // Fixed string mode, re-calculate if not regex
        match_length = strlen(separator);
        sentinel_length = match_length; // Ensure sentinel_length reflects actual separator length
    }

    // Calculate initial read_size and G_buffer_size
    read_size = 0x2000;
    while (read_size / 2 <= sentinel_length) {
        read_size *= 2;
        if (read_size == 0) xalloc_die(); // Check for overflow
    }
    
    size_t min_buffer_size = sentinel_length + read_size + 1;
    G_buffer_size = min_buffer_size * 2;
    if (min_buffer_size <= read_size || G_buffer_size <= min_buffer_size) {
        xalloc_die(); // Check for overflow or illogical sizes
    }
    G_buffer = (char *)xmalloc(G_buffer_size);

    // Adjust G_buffer pointer based on sentinel_length
    if (sentinel_length == 0) {
        G_buffer = G_buffer + 1; // G_buffer points to 1 byte offset if regex mode
    } else {
        // Original logic: copies (is_not_empty_char_at_separator_start ? 1 : 0) + sentinel_length bytes
        // This copies the separator string and its null terminator if separator is not empty,
        // or just a null byte if separator is empty.
        size_t copy_len = (*separator != '\0' ? 1 : 0) + sentinel_length;
        memcpy(G_buffer, separator, copy_len);
        G_buffer = G_buffer + sentinel_length; // G_buffer points after the sentinel area
    }
    
    char **input_files;
    if (optind < argc) {
        input_files = argv + optind;
    } else {
        input_files = (char **)default_file_list_0;
    }

    xset_binary_mode(STDOUT_FILENO, 0); // Assuming 0 for non-text mode for stdout
    bool overall_success = true;
    for (int i = 0; input_files[i] != NULL; ++i) {
        overall_success &= tac_file(input_files[i]);
    }

    output(NULL, 0); // Flush any remaining buffer content

    if (have_read_stdin && close(STDIN_FILENO) < 0) {
        error(0, *__errno_location(), DAT_00103d83); // DAT_00103d83 is likely "stdin"
        overall_success = false;
    }

    exit(!overall_success); // Exit with 0 on success, 1 on failure
}