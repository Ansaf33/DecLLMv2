#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <locale.h>
#include <libintl.h>
#include <unistd.h> // For getopt_long, optind, optarg
#include <getopt.h> // For struct option
#include <errno.h> // For errno, __errno_location
#include <sys/stat.h> // For umask, mkdir, mode_t, S_IXUSR
#include <sys/types.h> // For mode_t
#include <stdarg.h> // For error function

// Global program name
char *program_name;

// Forward declarations for external functions
void emit_mandatory_arg_note(void);
void emit_ancillary_info(const char *program);
const char *quotearg_style(int style, const char *arg);
void prog_fprintf(FILE *stream, const char *format, const char *arg);
int defaultcon(const char *context, const char *path, int type);
bool ignorable_ctx_err(int errnum);
// error function has variable arguments
void error(int status, int errnum, const char *format, ...);
void close_stdout(void);
void set_program_name(const char *name);
bool is_smack_enabled(void);
int setfscreatecon(const char *context);
int smack_set_label_for_self(const char *context);
const char *quote(const char *str);
void *mode_compile(const char *mode_str);
mode_t mode_adjust(mode_t old_mode, bool is_dir, mode_t umask_val, void *compiled_mode, int *set_mode_mask_ptr);
// savewd_process_files: processor takes (const char *path, const char *base, void *options)
int savewd_process_files(int argc, char **argv, int (*processor)(const char *, const char *, void *), void *options);
const char *last_component(const char *path);
bool restorecon(const char *context, const char *path, int flags);
const char *proper_name_lite(const char *name1, const char *name2);
void version_etc(FILE *stream, const char *program, const char *package, const char *version, const char *authors, int flags);

// The version string might be needed
const char *Version = "1.0"; // Placeholder if not defined elsewhere

// Struct to hold mkdir options, carefully aligned to match original offsets
// based on assembly-like access patterns.
// All `long` fields are 8 bytes, `mode_t` (usually `unsigned int`) is 4 bytes.
// Padding is inserted to ensure correct alignment for subsequent fields.
struct mkdir_options {
    long use_parents_flag;        // Offset 0 (e.g., param_3[0] in process_dir)
    mode_t create_mode;           // Offset 8 (e.g., *(int*)(param_3+8) in make_ancestor)
    mode_t umask_before_mode;     // Offset 12 (e.g., *(int*)(param_3+0xc) in make_ancestor, *(int*)((long)param_3+0x14) in process_dir)
    long _pad_before_context;     // Offset 16 (padding to align context_string to 8-byte boundary)
    const char *context_string;   // Offset 24 (e.g., *(long*)(param_3+0x18) in make_ancestor, param_3[3] in process_dir)
    const char *verbose_message;  // Offset 32 (e.g., *(long*)(param_2+0x20) in announce_mkdir)
};

// Function: usage
void usage(int status) {
  if (status == 0) {
    printf(gettext("Usage: %s [OPTION]... DIRECTORY...\n"), program_name);
    fputs_unlocked(gettext("Create the DIRECTORY(ies), if they do not already exist.\n"), stdout);
    emit_mandatory_arg_note();
    fputs_unlocked(gettext(
                            "  -m, --mode=MODE   set file mode (as in chmod), not a=rwx - umask\n  -p, --parents     no error if existing, make parent directories as needed,\n                    with their file modes unaffected by any -m option\n  -v, --verbose     print a message for each created directory\n"
                            ), stdout);
    fputs_unlocked(gettext(
                            "  -Z                   set SELinux security context of each created directory\n                         to the default type\n      --context[=CTX]  like -Z, or if CTX is specified then set the SELinux\n                         or SMACK security context to CTX\n"
                            ), stdout);
    fputs_unlocked(gettext("      --help        display this help and exit\n"), stdout);
    fputs_unlocked(gettext("      --version     output version information and exit\n"), stdout);
    emit_ancillary_info("mkdir");
  } else {
    fprintf(stderr, gettext("Try \'%s --help\' for more information.\n"), program_name);
  }
  exit(status);
}

// Function: announce_mkdir
void announce_mkdir(const char *path, struct mkdir_options *options) {
  if (options->verbose_message) {
    prog_fprintf(stdout, options->verbose_message, quotearg_style(4, path));
  }
}

// Function: make_ancestor
// param_1 is path, param_2 is dir_base, param_3 is options
bool make_ancestor(const char *path, char *dir_base, struct mkdir_options *options) {
  if (options->context_string) {
    if (defaultcon(options->context_string, dir_base, 0x4000) < 0) {
      if (!ignorable_ctx_err(errno)) {
        error(0, errno, gettext("failed to set default creation context for %s"), quotearg_style(4, path));
      }
    }
  }

  mode_t old_umask = 0;
  if (options->create_mode != options->umask_before_mode) {
    old_umask = umask(options->create_mode);
  }

  int mkdir_result = mkdir(dir_base, 0x1ff); // 0x1ff is 0777 octal (all permissions)

  if (options->create_mode != options->umask_before_mode) {
    int saved_errno = errno;
    umask(old_umask);
    errno = saved_errno;
  }

  if (mkdir_result == 0) { // mkdir succeeded
    announce_mkdir(path, options);
    return (options->create_mode & S_IXUSR) != 0; // Return true if S_IXUSR is set in create_mode
  }
  return false;
}

// Function: process_dir
// param_1 is path, param_2 is dir_base, param_3 is options
int process_dir(const char *path, const char *dir_base, struct mkdir_options *options) {
  if (options->context_string && !options->use_parents_flag) {
    if (defaultcon(options->context_string, path, 0x4000) < 0) {
      if (!ignorable_ctx_err(errno)) {
        error(0, errno, gettext("failed to set default creation context for %s"), quotearg_style(4, path));
      }
    }
  }

  // `make_ancestors_only` parameter in make_dir_parents is `1` in original code.
  // This means the `make_dir_parents` function is always called with `make_ancestors_only = true`.
  // It indicates that the function should only create parent directories, not the final one.
  // The `use_parents_flag` passed as `parents_option` to `make_dir_parents` controls error behavior for existing dirs.
  bool dir_created = make_dir_parents(path, dir_base, options->use_parents_flag != 0, options,
                                     options->create_mode, (void (*)(const char *, void *))announce_mkdir,
                                     options->umask_before_mode, 0xffffffffffffffffUL, 0xffffffffffffffffUL, true);

  if (!dir_created && options->context_string && options->use_parents_flag) {
    if (!restorecon(options->context_string, last_component(path), 0)) {
      if (!ignorable_ctx_err(errno)) {
        error(0, errno, gettext("failed to restore context for %s"), quotearg_style(4, path));
      }
    }
  }
  return !dir_created; // Return 1 (error) if directory was NOT created, 0 (success) if created.
}

// `longopts` array for getopt_long
static const struct option longopts[] = {
    {"mode", required_argument, NULL, 'm'},
    {"parents", no_argument, NULL, 'p'},
    {"verbose", no_argument, NULL, 'v'},
    {"context", optional_argument, NULL, 'Z'},
    {"help", no_argument, NULL, -130}, // -0x82
    {"version", no_argument, NULL, -131}, // -0x83
    {NULL, 0, NULL, 0}
};

int main(int argc, char **argv) {
    set_program_name(argv[0]);
    setlocale(LC_ALL, "");
    bindtextdomain("coreutils", "/usr/local/share/locale");
    textdomain("coreutils");
    atexit(close_stdout);

    struct mkdir_options options = {0}; // Initialize all fields to 0
    options.create_mode = 0x1ff; // Default mode 0777

    const char *mode_str_arg = NULL;

    int opt;
    while ((opt = getopt_long(argc, argv, "pm:vZ", longopts, NULL)) != -1) {
        switch (opt) {
            case 'p':
                options.use_parents_flag = 1; // Set flag for -p
                break;
            case 'm':
                mode_str_arg = optarg;
                break;
            case 'v':
                options.verbose_message = gettext("created directory %s");
                break;
            case 'Z': {
                if (!is_smack_enabled()) {
                    if (optarg != NULL) {
                        error(0, 0, gettext("warning: ignoring --context; it requires an SELinux/SMACK-enabled kernel"));
                    }
                } else {
                    options.context_string = optarg;
                }
                break;
            }
            case -130: // --help
                usage(0); // Prints help and exits
                // Original code also prints version after help
                proper_name_lite("David MacKenzie", "David MacKenzie");
                version_etc(stdout, "mkdir", "GNU coreutils", Version, "David MacKenzie", 0);
                exit(0);
            case -131: // --version
                proper_name_lite("David MacKenzie", "David MacKenzie");
                version_etc(stdout, "mkdir", "GNU coreutils", Version, "David MacKenzie", 0);
                exit(0);
            case '?': // Unknown option or missing argument
                usage(1);
                break;
            default: // Should not be reached for valid options
                usage(1);
                break;
        }
    }

    if (argc == optind) { // No operands left
        error(0, 0, gettext("missing operand"));
        usage(1);
    }

    // Handle context setting globally before processing files
    if (options.context_string) {
        int set_context_res;
        if (!is_smack_enabled()) {
            set_context_res = setfscreatecon(options.context_string);
        } else {
            set_context_res = smack_set_label_for_self(options.context_string);
        }
        if (set_context_res < 0) {
            error(1, errno, gettext("failed to set default file creation context to %s"), quote(options.context_string));
        }
    }

    // Handle mode setting and umask
    mode_t current_umask = umask(0); // Get current umask and set to 0 temporarily
    options.umask_before_mode = current_umask; // Store original umask for restoration in sub-functions

    if (mode_str_arg || options.use_parents_flag) {
        if (mode_str_arg) {
            void *compiled_mode = mode_compile(mode_str_arg);
            if (!compiled_mode) {
                error(1, 0, gettext("invalid mode %s"), quote(mode_str_arg));
            }
            int set_mode_mask_val = 0;
            options.create_mode = mode_adjust(0x1ff, true, current_umask, compiled_mode, &set_mode_mask_val);
            free(compiled_mode);
        }
        // Set umask for the duration of file processing
        umask(~options.create_mode & current_umask);
    } else {
        umask(current_umask); // Restore original umask if no mode was specified
    }

    // Call the file processing function
    int exit_status = savewd_process_files(argc - optind, argv + optind,
                                           (int (*)(const char *, const char *, void *))process_dir, &options);

    return exit_status;
}