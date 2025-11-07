#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <locale.h>
#include <stdarg.h>

// --- Mock/External Declarations for compilation ---

// Standard library globals
extern char *optarg;
extern int optind;
extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

// Coreutils specific globals/data
extern char *_program_name;
extern const char *_Version;
extern const char DAT_00101ae2[]; // Placeholder for external data
extern const char DAT_00102271[]; // Placeholder for external data
extern char dev_ino_buf_1[]; // Assuming it's a buffer, placeholder for external data
extern struct option long_options[]; // Placeholder for external data
extern const char *update_type_string[]; // Placeholder for external data
extern const int update_type[]; // Placeholder for external data
extern long __stack_chk_guard; // For stack canary, typically provided by linker/compiler

// Mock/External functions for compilation
char *gettext(const char *msgid) { return (char *)msgid; }
void error(int status, int errnum, const char *format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, "%s: ", _program_name ? _program_name : "unknown");
    vfprintf(stderr, format, args);
    if (errnum) {
        fprintf(stderr, ": %s", strerror(errnum));
    }
    fputc('\n', stderr);
    va_end(args);
    exit(status);
}
int *__errno_location(void) { return &errno; }
char *quotearg_style(int style, const char *arg) { return (char *)arg; }
unsigned long long get_root_dev_ino(char *buf) { return 1; }
void cp_options_default(char *opts) { /* Do nothing for mock */ }
void set_program_name(const char *name) { _program_name = (char *)name; }
void close_stdin(void) { /* Do nothing for mock */ }
void priv_set_remove_linkdir(void) { /* Do nothing for mock */ }
const char *proper_name_lite(const char *name1, const char *name2) { return name1; }
void version_etc(FILE *stream, const char *package, const char *program, const char *version, ...) {
    fprintf(stream, "%s (%s) %s\n", program, package, version);
    fprintf(stream, "Copyright (C) 2023 Free Software Foundation, Inc.\n");
}
void emit_mandatory_arg_note(void) { fprintf(stdout, "\nMandatory arguments notes.\n"); }
void emit_update_parameters_note(void) { fprintf(stdout, "\nUpdate parameters notes.\n"); }
void emit_backup_suffix_note(void) { fprintf(stdout, "\nBackup suffix notes.\n"); }
void emit_ancillary_info(const char *info) { fprintf(stdout, "\nAncillary info: %s\n", info); }
long __xargmatch_internal(const char *name, const char *arg, const char *const *arglist, const int *valuelist, size_t valuelist_len, void (*exit_fn)(void), bool allow_abbrev) { return 0; }
void _argmatch_die(void) { exit(1); }
int target_directory_operand(const char *path, char *buf) { return 0xffffff9c; }
bool target_dirfd_valid(int dirfd) { return dirfd != 0xffffff9c; }
int renameatu(int old_dirfd, const char *old_name, int new_dirfd, const char *new_name, int flags) { return 0; }
void strip_trailing_slashes(char *path) { /* Do nothing for mock */ }
unsigned int xget_version(const char *type, const char *arg) { return 0; }
void set_simple_backup_suffix(const char *suffix) { /* Do nothing for mock */ }
void hash_init(void) { /* Do nothing for mock */ }
void dest_info_init(char *opts) { /* Do nothing for mock */ }
const char *last_component(const char *path) { return path; }
void *file_name_concat(const char *dir, const char *file, char **result_ptr) {
    if (dir == NULL || file == NULL) { *result_ptr = NULL; return NULL; }
    size_t dlen = strlen(dir);
    size_t flen = strlen(file);
    char *new_path = malloc(dlen + 1 + flen + 1);
    if (new_path == NULL) { *result_ptr = NULL; return NULL; }
    strcpy(new_path, dir);
    new_path[dlen] = '/';
    strcpy(new_path + dlen + 1, file);
    *result_ptr = new_path;
    return new_path;
}
int copy(void *src, void *dest) { return 1; }
int rm(void **src_ptr, char *rm_opts) { return 0; }
void __stack_chk_fail(void) { abort(); }

// --- Type Aliases for readability and direct mapping ---
typedef char byte;
typedef unsigned int uint4;
typedef unsigned long long uint8;

// --- Function: rm_option_init ---
void rm_option_init(char *param_1) {
    param_1[0] = 0;
    param_1[10] = 1;
    param_1[9] = 1;
    param_1[8] = 0;
    *(uint4 *)(param_1 + 4) = 5;
    param_1[0x19] = 0;
    param_1[0x1a] = 0;
    param_1[0x1b] = 1;

    *(uint8 *)(param_1 + 0x10) = get_root_dev_ino(dev_ino_buf_1);

    if (*(long *)(param_1 + 0x10) == 0) {
        error(1, *__errno_location(), gettext("failed to get attributes of %s"),
              quotearg_style(4, DAT_00101ae2));
    }
    param_1[0x18] = 0;
}

// --- Function: cp_option_init ---
void cp_option_init(char *param_1) {
    cp_options_default(param_1);
    *(byte *)(param_1 + 0x14) = 0;
    *(uint4 *)(param_1 + 0x4c) = 1;
    *(uint4 *)(param_1 + 4) = 2;
    *(byte *)(param_1 + 0x15) = 0;
    *(byte *)(param_1 + 0x16) = 0;
    *(byte *)(param_1 + 0x17) = 0;
    *(uint4 *)(param_1 + 8) = 0;
    *(byte *)(param_1 + 0x18) = 1;
    *(byte *)(param_1 + 0x1b) = 0;
    *(byte *)(param_1 + 0x1e) = 0;
    *(byte *)(param_1 + 0x1f) = 1;
    *(byte *)(param_1 + 0x30) = 1;
    *(byte *)(param_1 + 0x20) = 1;
    *(byte *)(param_1 + 0x21) = 1;
    *(byte *)(param_1 + 0x22) = 0;
    *(byte *)(param_1 + 0x33) = 0;
    *(uint8 *)(param_1 + 0x28) = 0;
    *(byte *)(param_1 + 0x37) = 0;
    *(byte *)(param_1 + 0x31) = 1;
    *(byte *)(param_1 + 0x32) = 0;
    *(byte *)(param_1 + 0x34) = 0;
    *(byte *)(param_1 + 0x35) = 1;
    *(byte *)(param_1 + 0x36) = 0;
    *(byte *)(param_1 + 0x38) = 1;
    *(uint4 *)(param_1 + 0xc) = 2;
    *(byte *)(param_1 + 0x3a) = 0;
    *(byte *)(param_1 + 0x39) = 0;
    *(uint4 *)(param_1 + 0x10) = 0;
    *(bool *)(param_1 + 0x43) = isatty(0) != 0;
    *(byte *)(param_1 + 0x44) = 0;
    *(uint4 *)(param_1 + 0x3c) = 0;
    *(byte *)(param_1 + 0x40) = 0;
    *(uint8 *)(param_1 + 0x50) = 0;
    *(uint8 *)(param_1 + 0x58) = 0;
}

// --- Function: do_move ---
char do_move(void *source, void *destination, char *cp_options_ptr) {
    long stack_chk_cookie = __stack_chk_guard;
    char status = (char)copy(source, destination);

    // local_57 and local_56 were uninitialized in the original snippet.
    // Assuming they are flags, initializing them to false (0) for safe behavior.
    bool local_remove_src_flag = false; // Corresponds to local_57
    bool local_force_remove_src_flag = false; // Corresponds to local_56

    void *rm_target = NULL;

    if (status != '\0') {
        if (!local_remove_src_flag) {
            rm_target = source;
            if (local_force_remove_src_flag) {
                rm_target = NULL;
            }
        } else {
            rm_target = NULL;
            status = '\0';
        }

        if (rm_target != NULL) {
            char rm_opts_buffer[32]; // Increased size to safely accommodate all rm_option_init accesses
            rm_option_init(rm_opts_buffer);
            
            void *rm_arg_ptr = rm_target;

            int rm_status = rm(&rm_arg_ptr, rm_opts_buffer);

            if (!((rm_status == 0) || (rm_status == 2) || (rm_status == 3) || (rm_status == 4) || (rm_status == 5))) {
                __assert_fail("((status) == RM_OK || (status) == RM_USER_ACCEPTED || (status) == RM_USER_DECLINED || (status) == RM_ERROR)",
                              "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/mv.c",
                              0xf1, "do_move");
            }
            if (rm_status == 5) {
                status = '\0';
            }
        }
    }

    if (stack_chk_cookie == __stack_chk_guard) {
        return status;
    }
    __stack_chk_fail();
}

// --- Function: usage ---
void usage(int exit_code) {
    if (exit_code == 0) {
        printf(gettext("Usage: %s [OPTION]... [-T] SOURCE DEST\n  or:  %s [OPTION]... SOURCE... DIRECTORY\n  or:  %s [OPTION]... -t DIRECTORY SOURCE...\n"),
               _program_name, _program_name, _program_name);
        fputs_unlocked(gettext("Rename SOURCE to DEST, or move SOURCE(s) to DIRECTORY.\n"), stdout);
        emit_mandatory_arg_note();
        fputs_unlocked(gettext("      --backup[=CONTROL]       make a backup of each existing destination file\n  -b                           like --backup but does not accept an argument\n"), stdout);
        fputs_unlocked(gettext("      --debug                  explain how a file is copied.  Implies -v\n"), stdout);
        fputs_unlocked(gettext("      --exchange               exchange source and destination\n"), stdout);
        fputs_unlocked(gettext("  -f, --force                  do not prompt before overwriting\n  -i, --interactive            prompt before overwrite\n  -n, --no-clobber             do not overwrite an existing file\nIf you specify more than one of -i, -f, -n, only the final one takes effect.\n"), stdout);
        fputs_unlocked(gettext("      --no-copy                do not copy if renaming fails\n      --strip-trailing-slashes  remove any trailing slashes from each SOURCE\n                                 argument\n  -S, --suffix=SUFFIX          override the usual backup suffix\n"), stdout);
        fputs_unlocked(gettext("  -t, --target-directory=DIRECTORY  move all SOURCE arguments into DIRECTORY\n  -T, --no-target-directory    treat DEST as a normal file\n"), stdout);
        fputs_unlocked(gettext("      --update[=UPDATE]        control which existing files are updated;\n                                 UPDATE={all,none,none-fail,older(default)}\n  -u                           equivalent to --update[=older].  See below\n"), stdout);
        fputs_unlocked(gettext("  -v, --verbose                explain what is being done\n  -Z, --context                set SELinux security context of destination\n                                 file to default type\n"), stdout);
        fputs_unlocked(gettext("      --help        display this help and exit\n"), stdout);
        fputs_unlocked(gettext("      --version     output version information and exit\n"), stdout);
        emit_update_parameters_note();
        emit_backup_suffix_note();
        emit_ancillary_info(DAT_00102271);
    } else {
        fprintf(stderr, gettext("Try '%s --help' for more information.\n"), _program_name);
    }
    exit(exit_code);
}

// --- Defines for getopt_long option values ---
#define GETOPT_HELP_OPTION_VAL (0x82)
#define GETOPT_VERSION_OPTION_VAL (0x83)
#define DEBUG_OPTION_VAL (0x80)
#define EXCHANGE_OPTION_VAL (0x81)
#define NO_COPY_OPTION_VAL (0x84)
#define STRIP_TRAILING_SLASHES_OPTION_VAL (0x85)

// --- Function: main ---
int main(int argc, char **argv) {
    long stack_chk_cookie = __stack_chk_guard;

    bool backup_enabled = false;
    char *backup_suffix = NULL;
    char *backup_control = NULL;
    bool no_target_dir = false;
    char *target_dir_path = NULL;
    bool strip_slashes = false;

    set_program_name(argv[0]);
    setlocale(LC_ALL, "");
    bindtextdomain("coreutils", "/usr/local/share/locale");
    textdomain("coreutils");
    atexit(close_stdin);

    char cp_options_buffer[100]; // Sufficiently large buffer for cp_option_init offsets
    cp_option_init(cp_options_buffer);
    priv_set_remove_linkdir();

    int interactive_mode = 0; // 0: default, 1: force, 2: no-clobber, 3: interactive
    int update_type_val = 0;  // 0: default, 1: older, 2: none, 3: none-fail, 4: all
    byte verbose_flag = 0;
    byte debug_flag = 0;
    char exchange_flag = 0;
    char no_copy_flag = 0;

    int opt;
    while ((opt = getopt_long(argc, argv, "bfint:uvS:TZ", long_options, NULL)) != -1) {
        switch (opt) {
            case GETOPT_HELP_OPTION_VAL:
                usage(0);
                break;
            case GETOPT_VERSION_OPTION_VAL:
                version_etc(stdout, DAT_00102271, "GNU coreutils", _Version,
                            proper_name_lite("Mike Parker", "Mike Parker"),
                            proper_name_lite("David MacKenzie", "David MacKenzie"),
                            proper_name_lite("Jim Meyering", "Jim Meyering"), NULL);
                exit(0);
                break;
            case 'S':
                backup_enabled = true;
                backup_suffix = optarg;
                break;
            case 'T':
                no_target_dir = true;
                break;
            case 'Z':
                break;
            case 'b':
                backup_enabled = true;
                if (optarg != NULL) {
                    backup_control = optarg;
                }
                break;
            case 'f':
                interactive_mode = 1;
                break;
            case 'i':
                interactive_mode = 3;
                break;
            case 'n':
                interactive_mode = 2;
                break;
            case 't':
                if (target_dir_path != NULL) {
                    error(1, 0, gettext("multiple target directories specified"));
                }
                target_dir_path = optarg;
                break;
            case 'u':
                update_type_val = 1;
                if (optarg != NULL) {
                    long arg_idx = __xargmatch_internal("--update", optarg, update_type_string, update_type, 4, _argmatch_die, true);
                    update_type_val = update_type[arg_idx];
                }
                break;
            case 'v':
                verbose_flag = 1;
                break;
            case DEBUG_OPTION_VAL:
                verbose_flag = 1;
                debug_flag = 1;
                break;
            case EXCHANGE_OPTION_VAL:
                exchange_flag = 1;
                break;
            case NO_COPY_OPTION_VAL:
                no_copy_flag = 1;
                break;
            case STRIP_TRAILING_SLASHES_OPTION_VAL:
                strip_slashes = true;
                break;
            default:
                usage(1);
        }
    }

    int file_operand_count = argc - optind;
    char **file_operands = argv + optind;

    if (file_operand_count <= (target_dir_path == NULL)) {
        if (file_operand_count < 1) {
            error(0, 0, gettext("missing file operand"));
        } else {
            error(0, 0, gettext("missing destination file operand after %s"),
                  quotearg_style(4, file_operands[0]));
        }
        usage(1);
    }

    int target_dir_fd = 0;
    int rename_attempt_errno = 0;

    if (no_target_dir) {
        if (target_dir_path != NULL) {
            error(1, 0, gettext("cannot combine --target-directory (-t) and --no-target-directory (-T)"));
        }
        if (file_operand_count > 2) {
            error(0, 0, gettext("extra operand %s"), quotearg_style(4, file_operands[2]));
            usage(1);
        }
    }

    if (target_dir_path != NULL) {
        target_dir_fd = target_directory_operand(target_dir_path, NULL);
        if (!target_dirfd_valid(target_dir_fd)) {
            error(1, *__errno_location(), gettext("target directory %s"), quotearg_style(4, target_dir_path));
        }
    }

    char *dest_operand = file_operands[file_operand_count - 1];

    if ((file_operand_count == 2) && (!exchange_flag)) {
        if (renameatu(0xffffff9c, file_operands[0], 0xffffff9c, dest_operand, 1) != 0) {
            rename_attempt_errno = *__errno_location();
        }
    }

    if (rename_attempt_errno != 0) {
        int temp_target_dir_fd = target_directory_operand(dest_operand, NULL);
        if (!target_dirfd_valid(temp_target_dir_fd)) {
            if (file_operand_count > 2) {
                error(1, *__errno_location(), gettext("target %s"), quotearg_style(4, dest_operand));
            }
        } else {
            rename_attempt_errno = -1;
            target_dir_path = dest_operand;
            file_operand_count--;
            target_dir_fd = temp_target_dir_fd;
        }
    }

    if (strip_slashes) {
        for (int i = 0; i < file_operand_count; i++) {
            strip_trailing_slashes(file_operands[i]);
        }
    }

    if (interactive_mode == 2) {
        update_type_val = 2;
    }

    if (backup_enabled && (exchange_flag || (update_type_val == 2) || (update_type_val == 3))) {
        error(0, 0, gettext("cannot combine --backup with --exchange, -n, or --update=none-fail"));
        usage(1);
    }

    *(uint4 *)(cp_options_buffer + 0x10) = backup_enabled ? xget_version(gettext("backup type"), backup_control) : 0;
    set_simple_backup_suffix(backup_suffix);
    hash_init();

    bool overall_success = true;

    if (target_dir_path == NULL) {
        overall_success = do_move(file_operands[0], file_operands[1], cp_options_buffer);
    } else {
        if (file_operand_count > 1) {
            dest_info_init(cp_options_buffer);
        }
        for (int i = 0; i < file_operand_count; i++) {
            char *src_path = file_operands[i];
            const char *base_name = last_component(src_path);
            char *dest_full_path = NULL;
            file_name_concat(target_dir_path, base_name, &dest_full_path);
            strip_trailing_slashes(dest_full_path);

            bool move_succeeded = do_move(src_path, dest_full_path, cp_options_buffer);
            overall_success &= move_succeeded;
            free(dest_full_path);
        }
    }

    if (stack_chk_cookie != __stack_chk_guard) {
        __stack_chk_fail();
    }

    return (overall_success ? 0 : 1);
}