#ifndef _GNU_SOURCE
#define _GNU_SOURCE // For fputs_unlocked, __errno_location, perhaps others
#endif

// Preprocessor directives and includes
#include <stdio.h>    // For printf, fprintf, fputs_unlocked, stderr, stdout, FILE
#include <stdlib.h>   // For exit, atexit, free, EXIT_SUCCESS, EXIT_FAILURE
#include <stdbool.h>  // For bool
#include <sys/stat.h> // For stat, fstatat, fchmodat, mode_t, S_ISDIR
#include <errno.h>    // For errno, __errno_location
#include <string.h>   // For strlen, memcpy
#include <locale.h>   // For setlocale, LC_ALL
#include <libintl.h>  // For gettext, bindtextdomain, textdomain
#include <unistd.h>   // For umask, _optind
#include <getopt.h>   // For getopt_long, struct option, no_argument, required_argument
#include <fcntl.h>    // For AT_SYMLINK_NOFOLLOW

// Placeholder definitions for types and functions not provided in standard headers
// These are based on common patterns in decompiled C code and POSIX types.

// Types
typedef unsigned int uint;
typedef unsigned int undefined4;     // Assuming 32-bit unsigned int
typedef unsigned long long undefined8; // Assuming 64-bit unsigned long long
typedef unsigned char undefined;     // Assuming 8-bit unsigned char
typedef unsigned char byte;          // Assuming 8-bit unsigned char
typedef mode_t __mode_t;             // mode_t is defined in sys/stat.h

// Ghidra-specific CONCAT31. Assuming it returns 1 if low_byte is true, 0 otherwise,
// as implied by its usage in process_file (mode_changed(...) == 0).
#define CONCAT31(high_bytes, low_byte) ((low_byte) ? 1 : 0)

// External variables (placeholders)
extern char *program_name;
extern const char *Version;

extern bool force_silent;
extern int dereference; // -1, 0, 1
extern unsigned long long *root_dev_ino; // Array of 2 ulong for dev and ino
extern __mode_t umask_value;
extern void *change; // Type for mode_adjust/mode_compile/mode_create_from_ref
extern int verbosity; // 0 for verbose, 1 for changes, 2 for silent (default)
extern bool diagnose_surprises;
extern bool recurse;

extern char DAT_001020ba[]; // Placeholder for string data
extern char DAT_001021d2[]; // Placeholder for string data (likely "/")
extern char DAT_0010273b[]; // Placeholder for string data

// Global variables (actual definitions for this file, or extern if defined elsewhere)
char *program_name = NULL;
const char *Version = "UNKNOWN_VERSION";

bool force_silent = false;
int dereference = -1; // -1 for default, 0 for --no-dereference, 1 for --dereference
unsigned long long *root_dev_ino = NULL;
__mode_t umask_value = 0;
void *change = NULL;
int verbosity = 2; // 0: verbose, 1: changes, 2: silent (default)
bool diagnose_surprises = false;
bool recurse = false;

char DAT_001020ba[] = "error message placeholder";
char DAT_001021d2[] = "/"; // Common root path
char DAT_0010273b[] = "symlink recurse options placeholder";

// For getopt_long, needs to be defined
struct option long_options[] = {
    {"changes", no_argument, NULL, 'c'},
    {"silent", no_argument, NULL, 'f'},
    {"quiet", no_argument, NULL, 'f'},
    {"verbose", no_argument, NULL, 'v'},
    {"dereference", no_argument, NULL, 0x80},
    {"no-dereference", no_argument, NULL, 'h'},
    {"no-preserve-root", no_argument, NULL, 0x81},
    {"preserve-root", no_argument, NULL, 0x82},
    {"reference", required_argument, NULL, 0x83},
    {"recursive", no_argument, NULL, 'R'},
    {"help", no_argument, NULL, 0x84}, // Using 0x84 for --help
    {"version", no_argument, NULL, 0x85}, // Using 0x85 for --version
    {0, 0, 0, 0}
};

// To make _optind accessible (from unistd.h or getopt.h)
int _optind = 1;

// For get_root_dev_ino
unsigned long long dev_ino_buf_0[2];

// External function declarations (placeholders)
extern undefined8 quotearg_style(int style, const char *arg);
extern undefined8 quotearg_n_style_colon(int n, int style, const char *arg);
extern undefined8 quotearg_n_style(int n, int style, const char *arg);
extern void error(int status, int errnum, const char *format, ...);
extern bool cycle_warning_required(long param_1, long param_2);
extern void rpl_fts_set(long param_1, long param_2, int val);
extern bool is_ENOTSUP(int err);
extern __mode_t mode_adjust(__mode_t current_mode, bool is_dir, __mode_t umask_val, void *change_data, int param_5);
extern void strmode(__mode_t mode, char *buf); // buf should be char[11] or char[12] for null terminator
extern undefined8 xfts_open(char *const *argv, int options, void *compare_func);
extern long rpl_fts_read(undefined8 fts); // FTS* type
extern int rpl_fts_close(undefined8 fts); // FTS* type
extern void set_program_name(char *name);
extern undefined8 proper_name_lite(const char *name1, const char *name2);
extern void version_etc(FILE *stream, const char *prog_name, const char *package, const char *version, ...);
extern void emit_symlink_recurse_options(char *param_1);
extern void emit_ancillary_info(const char *program_name);
extern undefined8 quote(const char *arg);
extern void *mode_compile(const char *mode_str);
extern void *mode_create_from_ref(const char *ref_file);
extern unsigned long long *get_root_dev_ino(unsigned long long *buf); // buf is ulong[2]
extern bool streq(const char *s1, const char *s2);
extern void close_stdout(void); // Function registered with atexit
extern void __stack_chk_fail(void); // Stack smash detection
extern void *xpalloc(void *ptr, size_t *alloc_size, size_t num_elements_to_add, size_t max_size, size_t element_size);

// __builtin_frame_address(0) is a GCC extension for stack canary
#if defined(__GNUC__) || defined(__clang__)
#define in_FS_OFFSET __builtin_frame_address(0)
#else
// Fallback for other compilers, might not be accurate or safe
#define in_FS_OFFSET 0
#endif


// Function: mode_changed
undefined4 mode_changed(int param_1, char *param_2, const char *param_3, uint param_4, uint param_5) {
    int iVar1;
    uint uVar2;
    undefined4 uVar3_result = 0; // Default to 0 (no change or error)
    struct stat local_b8;
    long stack_chk_guard = *(long *)(in_FS_OFFSET + 0x28);

    uint current_mode_val = param_5;
    bool error_in_fstat = false;

    if ((param_5 & 0xe00) != 0) { // Check if param_5 has specific bits set (e.g., S_IFLNK, S_IFDIR)
        iVar1 = fstatat(param_1, param_2, &local_b8, 0);
        if (iVar1 != 0) {
            if (!force_silent) {
                error(0, *__errno_location(), gettext("getting new attributes of %s"), quotearg_style(4, param_3));
            }
            error_in_fstat = true;
        } else {
            current_mode_val = local_b8.st_mode;
        }
    }

    if (!error_in_fstat) {
        uVar2 = (param_4 ^ current_mode_val) & 0xfff; // XOR and mask to get changed mode bits
        uVar3_result = CONCAT31((uint)(uVar2 >> 8), uVar2 != 0); // Interpreted as (uVar2 != 0) ? 1 : 0
    }

    if (stack_chk_guard != *(long *)(in_FS_OFFSET + 0x28)) {
        __stack_chk_fail();
    }
    return uVar3_result;
}

// Function: describe_change
void describe_change(const char *param_1, uint *param_2) {
    uint uVar1_old_mode;
    uint uVar2_new_mode;
    uint uVar3_status;
    undefined8 quoted_param_1 = quotearg_style(4, param_1);
    long stack_chk_guard = *(long *)(in_FS_OFFSET + 0x28);

    char old_mode_str[12] = {0}; // e.g., "drwxrwxrwx\0"
    char new_mode_str[12] = {0};

    uVar3_status = *param_2;

    if (uVar3_status == 2) { // Neither symlink nor referent changed
        printf(gettext("neither symbolic link %s nor referent has been changed\n"), quoted_param_1);
    } else if (uVar3_status == 0) { // Could not be accessed
        printf(gettext("%s could not be accessed\n"), quoted_param_1);
    } else { // Status is 1, 3, or 4
        uVar1_old_mode = param_2[1];
        uVar2_new_mode = param_2[2];

        strmode(uVar2_new_mode, new_mode_str);
        strmode(uVar1_old_mode, old_mode_str);

        if (uVar3_status == 4) { // Mode changed
            printf(gettext("mode of %s changed from %04lo (%s) to %04lo (%s)\n"),
                   quoted_param_1, (unsigned long)(uVar1_old_mode & 0xfff), old_mode_str,
                   (unsigned long)(uVar2_new_mode & 0xfff), new_mode_str);
        } else if (uVar3_status == 1) { // Failed to change mode
            printf(gettext("failed to change mode of %s from %04lo (%s) to %04lo (%s)\n"),
                   quoted_param_1, (unsigned long)(uVar1_old_mode & 0xfff), old_mode_str,
                   (unsigned long)(uVar2_new_mode & 0xfff), new_mode_str);
        } else if (uVar3_status == 3) { // Mode retained
            printf(gettext("mode of %s retained as %04lo (%s)\n"),
                   quoted_param_1, (unsigned long)(uVar2_new_mode & 0xfff), new_mode_str);
        } else { // Should not happen
            __assert_fail("0", "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/chmod.c", 200, "describe_change");
        }
    }

    if (stack_chk_guard != *(long *)(in_FS_OFFSET + 0x28)) {
        __stack_chk_fail();
    }
    return;
}

// Function: process_file
bool process_file(long param_1, long param_2) {
    char *file_path = *(char **)(param_2 + 0x30);
    const char *quoted_file_path_str = (const char *)*(undefined8 *)(param_2 + 0x38); // Cast to const char*
    struct stat *file_stat = (struct stat *)(param_2 + 0x70);
    long stack_chk_guard = *(long *)(in_FS_OFFSET + 0x28);

    bool return_status = true; // Default to success
    bool continue_processing_main_logic = true; // Flag to control flow after switch

    uint status_code = 0; // 0 for initial, 2 for eligible for processing
    __mode_t original_mode = 0;
    __mode_t adjusted_mode = 0;

    struct stat temp_stat; // For dereference case

    switch (*(unsigned short *)(param_2 + 0x68)) { // FTS_INFO from param_2 + 0x68
    case 2: // FTS_DC (directory cycle)
        if (cycle_warning_required(param_1, param_2)) {
            error(0, 0, gettext(
                "WARNING: Circular directory structure.\nThis almost certainly means that you have a corrupted file system.\nNOTIFY YOUR SYSTEM MANAGER.\nThe following directory is part of the cycle:\n  %s\n"
            ), quotearg_n_style_colon(0, 3, quoted_file_path_str));
            return_status = false;
            continue_processing_main_logic = false;
        }
        status_code = 2; // Eligible for processing
        break;
    case 4: // FTS_DNR (directory not readable)
        if (!force_silent) {
            error(0, *(int *)(param_2 + 0x40), gettext("cannot read directory %s"), quotearg_style(4, quoted_file_path_str));
        }
        return_status = false; // Treat as failure
        continue_processing_main_logic = false;
        break;
    case 6: // FTS_NSOK (no stat, but OK for user)
        return_status = true; // Already true, explicit
        continue_processing_main_logic = false;
        break;
    case 7: // FTS_NS (no stat)
        if (!force_silent) {
            error(0, *(int *)(param_2 + 0x40), DAT_001020ba, quotearg_n_style_colon(0, 3, quoted_file_path_str));
        }
        return_status = false; // Treat as failure
        continue_processing_main_logic = false;
        break;
    case 10: // FTS_SLN (symbolic link to non-existent file)
        if ((*(long *)(param_2 + 0x58) == 0) && (*(long *)(param_2 + 0x20) == 0)) {
            *(undefined8 *)(param_2 + 0x20) = 1; // Set FTS_ISW-like flag
            rpl_fts_set(param_1, param_2, 1); // FTS_AGAIN
            return_status = true;
            continue_processing_main_logic = false;
        } else if (!force_silent) {
            error(0, *(int *)(param_2 + 0x40), gettext("cannot access %s"), quotearg_style(4, quoted_file_path_str));
        }
        return_status = false; // Treat as failure
        continue_processing_main_logic = false;
        break;
    case 0xc: // FTS_SL (symbolic link)
        if (dereference == 1) { // --dereference
            if (fstatat(*(int *)(param_1 + 0x2c), file_path, &temp_stat, 0) != 0) {
                if (!force_silent) {
                    error(0, *__errno_location(), gettext("cannot dereference %s"), quotearg_style(4, quoted_file_path_str));
                }
                return_status = false; // Error in dereference
                continue_processing_main_logic = false;
            } else {
                file_stat = &temp_stat; // Use the stat of the dereferenced file
                status_code = 2; // Eligible for processing
            }
        } else { // No --dereference or --no-dereference
            status_code = 2; // Eligible for processing, operate on the symlink itself
        }
        break;
    case 0xd: // FTS_D (directory) or FTS_DP (directory post-order)
        if (dereference == 0) { // --no-dereference
            status_code = 2; // Eligible for processing
        } else if (!force_silent) { // Default or --dereference on dangling symlink
            error(0, 0, gettext("cannot operate on dangling symlink %s"), quotearg_style(4, quoted_file_path_str));
            return_status = false; // Treat as failure
            continue_processing_main_logic = false;
        }
        break;
    default:
        status_code = 2; // Eligible for processing for regular files or directories
        break;
    }

    if (!continue_processing_main_logic) {
        if (stack_chk_guard != *(long *)(in_FS_OFFSET + 0x28)) {
            __stack_chk_fail();
        }
        return return_status;
    }

    // Main processing logic
    if (status_code == 2) {
        if (root_dev_ino != NULL &&
            ((root_dev_ino[0] ^ file_stat->st_ino) | (file_stat->st_dev ^ root_dev_ino[1])) == 0) { // Check if it's the root directory
            if (!streq(quoted_file_path_str, DAT_001021d2)) { // Not the same as "/"
                error(0, 0, gettext("it is dangerous to operate recursively on %s (same as %s)"),
                      quotearg_n_style(0, 4, quoted_file_path_str), quotearg_n_style(1, 4, DAT_001021d2));
            } else {
                error(0, 0, gettext("it is dangerous to operate recursively on %s"), quotearg_style(4, quoted_file_path_str));
            }
            error(0, 0, gettext("use --no-preserve-root to override this failsafe"));
            rpl_fts_set(param_1, param_2, 4); // FTS_SKIP
            rpl_fts_read(param_1); // Read past this entry to avoid infinite loop
            return_status = false;
        } else {
            original_mode = file_stat->st_mode;
            adjusted_mode = mode_adjust(original_mode, S_ISDIR(original_mode), umask_value, change, 0);

            int fchmodat_flags = 0;
            if (dereference == 0) { // --no-dereference
                fchmodat_flags = AT_SYMLINK_NOFOLLOW;
            } else if (dereference == -1 && *(long *)(param_2 + 0x58) != 0) { // Default behavior for symlinks, if not the target itself (FTS_SL)
                fchmodat_flags = AT_SYMLINK_NOFOLLOW;
            }

            if (fchmodat(*(int *)(param_1 + 0x2c), file_path, adjusted_mode, fchmodat_flags) == 0) {
                status_code = 4; // Success: mode changed
            } else {
                if (!is_ENOTSUP(*__errno_location())) { // Not a "not supported" error
                    if (!force_silent) {
                        error(0, *__errno_location(), gettext("changing permissions of %s"), quotearg_style(4, quoted_file_path_str));
                    }
                    status_code = 1; // Failure: failed to change mode
                }
            }

            if (verbosity != 2) { // If not completely silent
                if (status_code == 4 && mode_changed(*(int *)(param_1 + 0x2c), file_path, quoted_file_path_str, original_mode, adjusted_mode) == 0) {
                    status_code = 3; // Mode retained if mode_changed reports no actual change
                }
                if (status_code == 4 || verbosity == 0) { // If changed or verbose
                    uint describe_params[3] = {status_code, original_mode, adjusted_mode};
                    describe_change(quoted_file_path_str, describe_params);
                }
            }

            if (status_code > 2 && diagnose_surprises) { // If mode was changed/retained/failed, and diagnose surprises is on
                __mode_t expected_mode_after_umask = mode_adjust(original_mode, S_ISDIR(original_mode), 0, change, 0);
                if ((adjusted_mode & ~expected_mode_after_umask) != 0) { // If bits are set in adjusted that shouldn't be
                    char actual_mode_str[12] = {0};
                    char expected_mode_str[12] = {0};
                    strmode(adjusted_mode, actual_mode_str);
                    strmode(expected_mode_after_umask, expected_mode_str);
                    error(0, 0, gettext("%s: new permissions are %s, not %s"),
                          quotearg_n_style_colon(0, 3, quoted_file_path_str), actual_mode_str, expected_mode_str);
                    status_code = 1; // Treat as failure due to surprise
                }
            }

            if (!recurse) {
                rpl_fts_set(param_1, param_2, 4); // FTS_SKIP (don't recurse into this directory)
            }
            return_status = (status_code > 1); // Success if status_code is 2, 3, 4
        }
    } else { // If initial status_code was not 2 (e.g., an error in switch cases that didn't exit)
        return_status = false; // Treat as failure
    }

    if (stack_chk_guard != *(long *)(in_FS_OFFSET + 0x28)) {
        __stack_chk_fail();
    }
    return return_status;
}

// Function: process_files
bool process_files(char *const *argv, undefined4 fts_options) { // Changed param_1 to char *const *argv
    bool overall_success = true;
    long stack_chk_guard = *(long *)(in_FS_OFFSET + 0x28);

    undefined8 fts_handle = xfts_open(argv, fts_options, NULL); // Assuming NULL for compare_func
    if (fts_handle == 0) {
        if (!force_silent) {
            error(1, *__errno_location(), gettext("failed to open file tree"));
        }
        overall_success = false;
    } else {
        long fts_entry;
        while ((fts_entry = rpl_fts_read(fts_handle)) != 0) {
            overall_success &= process_file(fts_handle, fts_entry);
        }

        if (*__errno_location() != 0) {
            if (!force_silent) {
                error(0, *__errno_location(), gettext("fts_read failed"));
            }
            overall_success = false;
        }

        if (rpl_fts_close(fts_handle) != 0) {
            error(0, *__errno_location(), gettext("fts_close failed"));
            overall_success = false;
        }
    }

    if (stack_chk_guard != *(long *)(in_FS_OFFSET + 0x28)) {
        __stack_chk_fail();
    }
    return overall_success;
}

// Function: usage
void usage(int status) {
    FILE *out_stream = (status == 0) ? stdout : stderr;

    if (status == 0) {
        fprintf(out_stream, gettext(
            "Usage: %s [OPTION]... MODE[,MODE]... FILE...\n"
            "  or:  %s [OPTION]... OCTAL-MODE FILE...\n"
            "  or:  %s [OPTION]... --reference=RFILE FILE...\n"
        ), program_name, program_name, program_name);
        fputs_unlocked(gettext(
            "Change the mode of each FILE to MODE.\n"
            "With --reference, change the mode of each FILE to that of RFILE.\n\n"
        ), out_stream);
        fputs_unlocked(gettext(
            "  -c, --changes          like verbose but report only when a change is made\n"
            "  -f, --silent, --quiet  suppress most error messages\n"
            "  -v, --verbose          output a diagnostic for every file processed\n"
        ), out_stream);
        fputs_unlocked(gettext(
            "      --dereference      affect the referent of each symbolic link,\n"
            "                           rather than the symbolic link itself\n"
            "  -h, --no-dereference   affect each symbolic link, rather than the referent\n"
        ), out_stream);
        fputs_unlocked(gettext(
            "      --no-preserve-root  do not treat '/' specially (the default)\n"
            "      --preserve-root    fail to operate recursively on '/'\n"
        ), out_stream);
        fputs_unlocked(gettext(
            "      --reference=RFILE  use RFILE's mode instead of specifying MODE values.\n"
            "                         RFILE is always dereferenced if a symbolic link.\n"
        ), out_stream);
        fputs_unlocked(gettext("  -R, --recursive        change files and directories recursively\n"), out_stream);
        emit_symlink_recurse_options(DAT_0010273b);
        fputs_unlocked(gettext("      --help        display this help and exit\n"), out_stream);
        fputs_unlocked(gettext("      --version     output version information and exit\n"), out_stream);
        fputs_unlocked(gettext(
            "\nEach MODE is of the form '[ugoa]*([-+=]([rwxXst]*|[ugo]))+|[-+=][0-7]+'.\n"
        ), out_stream);
        emit_ancillary_info("chmod");
    } else {
        fprintf(out_stream, gettext("Try '%s --help' for more information.\n"), program_name);
    }
    exit(status);
}

// Function: main
int main(int argc, char **argv) {
    long stack_chk_guard = *(long *)(in_FS_OFFSET + 0x28);

    char *mode_string = NULL;
    const char *reference_file = NULL;
    bool preserve_root = false;
    int fts_options = 0x11; // FTS_COMFOLLOW | FTS_PHYSICAL (default)

    set_program_name(argv[0]);
    setlocale(LC_ALL, "");
    bindtextdomain("coreutils", "/usr/local/share/locale");
    textdomain("coreutils");
    atexit(close_stdout);

    diagnose_surprises = false;
    force_silent = false;
    recurse = false;

    // Variables for mode string building
    size_t current_mode_str_len = 0;
    size_t mode_str_buffer_size = 0;
    char *mode_str_buffer = NULL; // Dynamically allocated

    int c;
    while ((c = getopt_long(argc, argv,
                            "HLPRcfhvr::w::x::X::s::t::u::g::o::a::,::+::=::0::1::2::3::4::5::6::7::",
                            long_options, NULL)) != -1) {
        switch (c) {
            case 'H':
                fts_options = 2; // FTS_COMFOLLOW
                break;
            case 'L':
                fts_options = 0x11; // FTS_COMFOLLOW | FTS_PHYSICAL
                break;
            case 'P':
                fts_options = 0x10; // FTS_PHYSICAL
                break;
            case 'R':
                recurse = true;
                break;
            case 'c':
                verbosity = 1; // --changes
                break;
            case 'f':
                force_silent = true;
                break;
            case 'h':
                dereference = 0; // --no-dereference
                break;
            case 'v':
                verbosity = 0; // --verbose
                break;
            case 0x80: // --dereference
                dereference = 1;
                break;
            case 0x81: // --no-preserve-root
                preserve_root = false;
                break;
            case 0x82: // --preserve-root
                preserve_root = true;
                break;
            case 0x83: // --reference=RFILE
                reference_file = optarg;
                break;
            case 0x84: // --help
                usage(0);
                // usage exits
            case 0x85: // --version
                version_etc(stdout, "chmod", "GNU coreutils", Version,
                            proper_name_lite("David MacKenzie", "David MacKenzie"),
                            proper_name_lite("Jim Meyering", "Jim Meyering"), NULL);
                exit(0);
            case '?': // getopt_long returns '?' for unknown option or missing argument for required_argument
                usage(1);
                // usage exits
            default: // Catches mode characters and octal digits for mode string accumulation
                diagnose_surprises = true; // Implies a mode string is being processed
                char *current_opt_arg = argv[_optind - 1]; // Current option string
                size_t opt_arg_len = strlen(current_opt_arg);

                size_t required_len = current_mode_str_len + (current_mode_str_len != 0) + opt_arg_len;
                if (mode_str_buffer_size <= required_len) {
                    // Grow buffer by at least opt_arg_len + 1 (for comma/null) + some extra
                    mode_str_buffer_size = required_len + (opt_arg_len > 10 ? opt_arg_len : 10);
                    mode_str_buffer = xpalloc(mode_str_buffer, &mode_str_buffer_size, 0, (size_t)-1, 1);
                }

                if (current_mode_str_len != 0) {
                    mode_str_buffer[current_mode_str_len] = ',';
                    current_mode_str_len++;
                }
                memcpy(mode_str_buffer + current_mode_str_len, current_opt_arg, opt_arg_len + 1); // +1 for null terminator
                current_mode_str_len += opt_arg_len;
                mode_string = mode_str_buffer; // Update mode_string to point to the buffer
                break;
        }
    }

    if (recurse && fts_options == 0x10) { // -R and -P (physical)
        if (dereference == 1) { // --dereference is incompatible
            error(1, 0, gettext("-R --dereference requires either -H or -L"));
        }
        dereference = 0; // Default to --no-dereference for -R -P
    }

    if (dereference == -1 && fts_options == 2) { // Default behavior for -H (FTS_COMFOLLOW)
        dereference = 1; // Dereference for -H
    }

    // Handle mode_string or reference_file
    if (reference_file != NULL) {
        if (mode_string != NULL) {
            error(0, 0, gettext("cannot combine mode and --reference options"));
            usage(1);
        }
    } else { // No reference file, must have a mode string
        if (mode_string == NULL) {
            // If _optind points to the first file operand and it's the mode string
            if (argc > _optind) {
                mode_string = argv[_optind];
                _optind++;
            }
        }
    }

    if (mode_string == NULL) {
        if (argc <= _optind) { // No mode string and no file operands
            error(0, 0, gettext("missing operand"));
        } else { // No mode string, but there are file operands
            error(0, 0, gettext("missing operand after %s"), quote(argv[argc - 1]));
        }
        usage(1);
    }

    if (reference_file != NULL) {
        change = mode_create_from_ref(reference_file);
        if (change == NULL) {
            error(1, *__errno_location(), gettext("failed to get attributes of %s"), quotearg_style(4, reference_file));
            // mode_create_from_ref failure is fatal here, exits via error(1, ...)
        }
    } else {
        change = mode_compile(mode_string);
        if (change == NULL) {
            error(0, 0, gettext("invalid mode: %s"), quote(mode_string));
            usage(1);
        }
        umask_value = umask(0); // Set umask to 0 temporarily to capture original
    }

    if (recurse && preserve_root) {
        root_dev_ino = get_root_dev_ino(dev_ino_buf_0);
        if (root_dev_ino == NULL) {
            error(1, *__errno_location(), gettext("failed to get attributes of %s"), quotearg_style(4, DAT_001021d2));
            // get_root_dev_ino failure is fatal here, exits via error(1, ...)
        }
    } else {
        root_dev_ino = NULL; // No root protection
    }

    bool success = process_files(argv + _optind, fts_options);

    if (mode_str_buffer != NULL) {
        free(mode_str_buffer); // Free dynamically allocated mode string buffer
    }

    if (stack_chk_guard != *(long *)(in_FS_OFFSET + 0x28)) {
        __stack_chk_fail();
    }
    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}