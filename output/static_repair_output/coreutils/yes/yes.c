#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <locale.h>
#include <errno.h>
#include <unistd.h> // For STDOUT_FILENO, write
#include <libintl.h> // For gettext, bindtextdomain, textdomain
#include <getopt.h> // For getopt_long and optind

// --- Coreutils-like declarations for compilation ---
// These are simplified or dummy implementations to make the code compile.
// In a real coreutils build, these would come from various coreutils libraries.

// Global program name, usually set by set_program_name
char *program_name = NULL;

// Version string
const char *Version = "1.0";

// Dummy data for emit_ancillary_info
static const char DAT_00100909[] = "";

// Dummy data for default 'y' string
static const char DAT_00100943[] = "y";

// Function to set program name
void set_program_name(char *name) {
    program_name = name;
}

// Function to flush stdout at exit
void close_stdout(void) {
    fflush(stdout);
}

// Dummy for ancillary info
void emit_ancillary_info(const char *data) {
    (void)data; // Suppress unused parameter warning
}

// Wrapper for malloc that exits on failure (x stands for exit)
void *xmalloc(size_t size) {
    void *p = malloc(size);
    if (!p) {
        fprintf(stderr, "%s: Out of memory\n", program_name ? program_name : "unknown");
        exit(EXIT_FAILURE);
    }
    return p;
}

// Wrapper for write that attempts to write all bytes
ssize_t full_write(int fd, const void *buf, size_t count) {
    size_t total_written = 0;
    const char *ptr = buf;
    while (total_written < count) {
        ssize_t bytes_written = write(fd, ptr + total_written, count - total_written);
        if (bytes_written == -1) {
            if (errno == EINTR) {
                continue; // Retry on interrupt
            }
            return -1; // Actual error
        }
        if (bytes_written == 0) {
            // This should ideally not happen for full_write unless count was 0 or pipe closed
            break;
        }
        total_written += bytes_written;
    }
    return total_written;
}

// Coreutils error reporting function
void error(int status, int errnum, const char *message) {
    fprintf(stderr, "%s: %s: %s\n", program_name ? program_name : "unknown", message, strerror(errnum));
    if (status != 0) {
        exit(status);
    }
}

// Dummy for proper_name_lite
char *proper_name_lite(const char *name1, const char *name2) {
    (void)name2;
    return (char *)name1;
}

// Dummy for bad_cast (seems to be just a type cast in the original context)
char *bad_cast(const void *ptr) {
    return (char *)ptr;
}

// Forward declaration for usage, needed by parse_gnu_standard_options_only
void usage(int exit_code);

// Dummy for parse_gnu_standard_options_only
// This function normally handles --help, --version etc. and sets optind.
void parse_gnu_standard_options_only(int argc, char **argv, const char *data, const char *pkg, const char *ver, int flags, void (*usage_func)(int), char *authors, int bug_report_address) {
    (void)data; (void)flags; (void)authors; (void)bug_report_address;
    int c;
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {0, 0, 0, 0}
    };

    while ((c = getopt_long(argc, argv, "", long_options, NULL)) != -1) {
        switch (c) {
            case 'h': // --help
                usage_func(EXIT_SUCCESS);
                break;
            case 'v': // --version
                printf("%s %s\n", pkg, ver);
                exit(EXIT_SUCCESS);
                break;
            case '?': // Unknown option
                usage_func(EXIT_FAILURE);
                break;
        }
    }
    // optind is now set by getopt_long to the index of the first non-option argument.
}

// Function: usage
void usage(int exit_code) {
  if (exit_code == 0) {
    printf(gettext("Usage: %s [STRING]...\n  or:  %s OPTION\n"), program_name, program_name);
    fputs_unlocked(gettext("Repeatedly output a line with all specified STRING(s), or \'y\'.\n\n"), stdout);
    fputs_unlocked(gettext("      --help        display this help and exit\n"), stdout);
    fputs_unlocked(gettext("      --version     output version information and exit\n"), stdout);
    emit_ancillary_info(DAT_00100909);
  } else {
    fprintf(stderr, gettext("Try \'%s --help\' for more information.\n"), program_name);
  }
  exit(exit_code);
}

// Function: main
int main(int argc, char **argv) {
  set_program_name(argv[0]);
  setlocale(LC_ALL, "");
  bindtextdomain("coreutils", "/usr/local/share/locale");
  textdomain("coreutils");
  atexit(close_stdout);

  parse_gnu_standard_options_only(argc, argv, DAT_00100909, "GNU coreutils", Version, 1, usage, proper_name_lite("David MacKenzie", "David MacKenzie"), 0);

  char **args_start = argv + optind;
  char **args_end = argv + argc;
  char **current_args_end = args_end;

  if (argc == optind) { // No arguments provided, use default "y"
    current_args_end = args_end + 1;
    *args_end = (char *)&DAT_00100943; // Assign "y" to the position after actual args
  }

  size_t total_required_buffer_len = 0;
  bool is_contiguous_input = true;

  char **current_arg_ptr = args_start;
  while (current_arg_ptr < current_args_end) {
    size_t len = strlen(*current_arg_ptr);
    total_required_buffer_len += len + 1; // +1 for space/newline

    // Check if the arguments form a contiguous block of memory
    // This optimization is for cases where argv entries are already contiguous,
    // allowing direct use of argv's memory without copying.
    if ((current_arg_ptr + 1 < current_args_end) && (*current_arg_ptr + len + 1 != current_arg_ptr[1])) {
      is_contiguous_input = false;
    }
    current_arg_ptr++;
  }

  // If the total length is small, force a larger allocation and disable contiguity optimization.
  // This is a heuristic to prevent very small outputs from being repeatedly copied from argv memory
  // if is_contiguous_input is true, and instead use a larger buffer.
  if (total_required_buffer_len < 0x1001) { // 4KB threshold
    total_required_buffer_len = 0x2000; // 8KB
    is_contiguous_input = false;
  }

  char *output_buffer;
  if (is_contiguous_input) {
    output_buffer = *args_start; // Use the first argument's memory directly
  } else {
    output_buffer = (char *)xmalloc(total_required_buffer_len);
  }

  size_t current_buffer_offset = 0;
  current_arg_ptr = args_start;

  while (current_arg_ptr < current_args_end) {
    size_t len = strlen(*current_arg_ptr);
    if (!is_contiguous_input) {
      memcpy(output_buffer + current_buffer_offset, *current_arg_ptr, len);
    }
    current_buffer_offset += len;
    output_buffer[current_buffer_offset] = ' '; // Append space
    current_buffer_offset++;
    current_arg_ptr++;
  }
  // Replace the last space with a newline character
  output_buffer[current_buffer_offset - 1] = '\n';

  size_t length_of_one_line = current_buffer_offset; // This is the final length of the first generated line.

  // Calculate how many times this line needs to be copied to fill the buffer.
  size_t num_repetitions = total_required_buffer_len / length_of_one_line;

  // Copy the generated line repeatedly to fill the buffer, effectively creating a larger output.
  // The loop runs num_repetitions - 1 times, as the first line is already in place.
  while (--num_repetitions > 0) {
    memcpy(output_buffer + current_buffer_offset, output_buffer, length_of_one_line);
    current_buffer_offset += length_of_one_line;
  }

  // Write the entire constructed buffer to standard output.
  if (full_write(STDOUT_FILENO, output_buffer, current_buffer_offset) != current_buffer_offset) {
    error(0, errno, gettext("standard output")); // Prints error message
    exit(EXIT_FAILURE); // Exits due to write failure
  }

  exit(EXIT_SUCCESS);
}