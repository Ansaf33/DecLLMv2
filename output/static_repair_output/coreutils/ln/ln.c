#include <errno.h>   // For __errno_location, EEXIST, ENOTDIR, ELOOP, EMLINK, EXDEV, EPERM, EROFS, ENOENT
#include <fcntl.h>   // For AT_FDCWD, AT_SYMLINK_FOLLOW, O_RDONLY, O_DIRECTORY, O_NOFOLLOW
#include <getopt.h>  // For getopt_long, struct option, optarg, optind
#include <libintl.h> // For gettext, bindtextdomain, textdomain
#include <locale.h>  // For setlocale, LC_ALL
#include <stdbool.h> // For bool type
#include <stdarg.h>  // For va_list
#include <stdio.h>   // For FILE, fprintf, printf, fputs_unlocked, sprintf, stderr, stdout
#include <stdlib.h>  // For exit, free, atexit, size_t, malloc, strdup
#include <string.h>  // For strlen, memcpy, strcpy, strrchr, strerror
#include <sys/stat.h> // For struct stat, stat, fstatat, S_ISDIR, S_IFMT, S_IFDIR
#include <unistd.h>  // For linkat, symlinkat, close, unlinkat

// Global flags and variables (initialized to common defaults or false)
bool symbolic_link = false;
bool hard_dir_link = false; // Corresponds to -d/F option
bool beware_hard_dir_link = true; // Default to true, set to false by priv_set_remove_linkdir
bool logical = false;
bool relative = false;
bool remove_existing_files = false;
bool interactive = false;
int backup_type = 0; // 0 for no backup
bool verbose = false;
bool dereference_dest_dir_symlinks = true; // Default behavior
void *dest_set = NULL; // Pointer to a hash table, NULL if not used

// External variables for program info and streams
const char *_program_name = NULL;
FILE *_stderr = NULL; // Should be stderr
FILE *_stdout = NULL; // Should be stdout

// External data for usage messages (placeholders)
const char DAT_00102b4a[] = "physical"; // Example, needs actual value from coreutils
const char DAT_00102b4d[] = "link (GNU coreutils) 9.3"; // Example
const char _Version[] = "9.3"; // Example version

// Stubs for coreutils library functions
void set_program_name(const char *name) { _program_name = name; _stderr = stderr; _stdout = stdout; }
void close_stdin(void) { /* no-op for stub */ }
void priv_set_remove_linkdir(void) { beware_hard_dir_link = false; } // Simplified stub logic
void error(int status, int errnum, const char *format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, "%s: ", _program_name ? _program_name : "link");
    vfprintf(stderr, format, args);
    if (errnum != 0) {
        fprintf(stderr, ": %s", strerror(errnum));
    }
    fprintf(stderr, "\n");
    va_end(args);
    if (status != 0) exit(status);
}
void *xmalloc(size_t size) { void *p = malloc(size); if (!p) { fprintf(stderr, "Out of memory\n"); exit(1); } return p; }
char *xstrdup(const char *s) { char *p = strdup(s); if (!p) { fprintf(stderr, "Out of memory\n"); exit(1); } return p; }
void xalloc_die(void) { fprintf(stderr, "Memory allocation error\n"); exit(1); }

// Simplified stubs for various coreutils functions
char *dir_name(const char *path) { return xstrdup(path ? path : ""); }
char *canonicalize_filename_mode(const char *path, int mode) { return xstrdup(path ? path : ""); }
bool relpath(const char *target, const char *base, char *buf, size_t buflen) {
    if (!target || !base || !buf || buflen == 0) return false;
    if (strlen(target) < buflen) {
        strcpy(buf, target);
        return true;
    }
    return false;
}
const char *quotearg_style(int style, const char *arg) { return arg ? arg : ""; }
const char *quotearg_n_style_colon(int n, int style, const char *arg) { return arg ? arg : ""; }
const char *quotearg_n_style(int n, int style, const char *arg) { return arg ? arg : ""; }
bool seen_file(void *dest_set_ptr, const char *filename, const struct stat *st) { return false; }
bool psame_inode(const struct stat *st1, const struct stat *st2) {
    if (!st1 || !st2) return false;
    return (st1->st_dev == st2->st_dev && st1->st_ino == st2->st_ino);
}
bool same_nameat(int fd, const char *name1, int fd2, const char *name2) { return strcmp(name1, name2) == 0; }
char yesno(void) { char c; fprintf(stderr, "(y/n)? "); scanf(" %c", &c); return (c == 'y' || c == 'Y') ? '\x01' : '\0'; }
char *find_backup_file_name(int dirfd, const char *filename, int backup_type_val) {
    char *backup_name = xmalloc(strlen(filename) + 2); // For "~" and null terminator
    strcpy(backup_name, filename);
    strcat(backup_name, "~");
    return backup_name;
}
int force_linkat(int olddirfd, const char *oldpath, int newdirfd, const char *newpath, int flags, bool remove_existing, int prev_errno) {
    if (remove_existing) unlinkat(newdirfd, newpath, 0);
    int res = linkat(olddirfd, oldpath, newdirfd, newpath, flags);
    if (res != 0) return *__errno_location();
    return 0;
}
int force_symlinkat(const char *oldpath, int newdirfd, const char *newpath, bool remove_existing, int prev_errno) {
    if (remove_existing) unlinkat(newdirfd, newpath, 0);
    int res = symlinkat(oldpath, newdirfd, newpath);
    if (res != 0) return *__errno_location();
    return 0;
}
void record_file(void *dest_set_ptr, const char *filename, const struct stat *st) { /* no-op for stub */ }
void emit_mandatory_arg_note(void) { fputs_unlocked(gettext("\nMandatory arguments to long options are mandatory for short options too.\n"), _stdout); }
void emit_backup_suffix_note(void) { fputs_unlocked(gettext("The backup suffix is '~', unless set with --suffix or SIMPLE_BACKUP_SUFFIX.\nThe version control may be: none, off, numbered, t, existing, nil, simple, never, always.\n"), _stdout); }
void emit_ancillary_info(const char *program_name_and_version_info) { fputs_unlocked(gettext("For full documentation, run: info coreutils 'link invocation'\n"), _stdout); }
const char *proper_name_lite(const char *p1, const char *p2) { return p1; }
void version_etc(FILE *stream, const char *program_name, const char *package, const char *version, ...) {
    fprintf(stream, "%s (%s) %s\n", program_name, package, version);
}
int xget_version(const char *type_name, const char *arg) { return 1; }
void set_simple_backup_suffix(const char *suffix) { /* no-op for stub */ }
void *hash_initialize(size_t n_buckets, void *hasher, void *comparator, void *freer) { return (void*)1; }
char *last_component(const char *path) { const char *slash = strrchr(path, '/'); return (char*)(slash ? slash + 1 : path); }
char *file_name_concat(const char *dir, const char *file, char **buffer_ptr) {
    size_t dir_len = strlen(dir);
    size_t file_len = strlen(file);
    bool needs_slash = (dir_len > 0 && dir[dir_len - 1] != '/');
    size_t total_len = dir_len + (needs_slash ? 1 : 0) + file_len;
    char *buf = xmalloc(total_len + 1);
    strcpy(buf, dir);
    if (needs_slash) strcat(buf, "/");
    strcat(buf, file);
    if (buffer_ptr) *buffer_ptr = buf;
    return buf;
}
void strip_trailing_slashes(char *path) {
    if (!path) return;
    size_t len = strlen(path);
    while (len > 1 && path[len - 1] == '/') {
        path[--len] = '\0';
    }
}
int openat_safer(int dirfd, const char *pathname, int flags) {
    return openat(dirfd, pathname, flags);
}

// long_options structure for getopt_long
struct option long_options[] = {
    {"backup", optional_argument, NULL, 'b'},
    {"directory", no_argument, NULL, 'd'},
    {"force", no_argument, NULL, 'f'},
    {"interactive", no_argument, NULL, 'i'},
    {"logical", no_argument, NULL, 'L'},
    {"no-dereference", no_argument, NULL, 'n'},
    {"physical", no_argument, NULL, 'P'},
    {"relative", no_argument, NULL, 'r'},
    {"symbolic", no_argument, NULL, 's'},
    {"suffix", required_argument, NULL, 'S'},
    {"target-directory", required_argument, NULL, 't'},
    {"no-target-directory", no_argument, NULL, 'T'},
    {"verbose", no_argument, NULL, 'v'},
    {"help", no_argument, NULL, 256}, // Unique integer value for --help
    {"version", no_argument, NULL, 257}, // Unique integer value for --version
    {NULL, 0, NULL, 0}
};

// Function: errnoize
int errnoize(int result) {
    if (result < 0) {
        return *__errno_location();
    }
    return 0;
}

// Function: convert_abs_rel
char *convert_abs_rel(const char *param_1, const char *param_2) {
    char *result_path = NULL;
    char *dir_name_ptr = dir_name(param_2);
    char *canonical_dir_ptr = canonicalize_filename_mode(dir_name_ptr, 2);
    char *canonical_param1_ptr = canonicalize_filename_mode(param_1, 2);

    if (canonical_dir_ptr && canonical_param1_ptr) {
        result_path = xmalloc(4096); // Assuming 0x1000 is PATH_MAX, using 4096
        if (result_path) {
            if (!relpath(canonical_param1_ptr, canonical_dir_ptr, result_path, 4096)) {
                free(result_path);
                result_path = NULL;
            }
        }
    }

    free(dir_name_ptr);
    free(canonical_dir_ptr);
    free(canonical_param1_ptr);

    if (!result_path) {
        result_path = xstrdup(param_1);
    }
    return result_path;
}

// Function: atomic_link
int atomic_link(char *param_1, int param_2, char *param_3) {
    int link_result;

    if (!symbolic_link) { // symbolic_link == '\0'
        if (!beware_hard_dir_link) { // beware_hard_dir_link == '\0'
            int flags = logical ? AT_SYMLINK_FOLLOW : 0;
            link_result = linkat(AT_FDCWD, param_1, param_2, param_3, flags);
            return errnoize(link_result);
        } else {
            return -1; // Indicate error: hard link to dir not allowed
        }
    } else { // symbolic_link != '\0'
        if (!relative) { // relative == '\0'
            link_result = symlinkat(param_1, param_2, param_3);
            return errnoize(link_result);
        } else {
            return -1; // Indicate error: relative symlink not supported
        }
    }
}

// Function: do_link
const char DAT_00102004_str[] = ""; // Represents empty string or specific format
const char DAT_00102199_str[] = ", "; // Represents separator for backup info

bool do_link(char *src_path, int dest_dir_fd, char *dest_path, char *dest_display_name, int prev_errno_val) {
    bool success = true;
    int fstat_src_result = 1; // 1 means error, 0 means success
    char *backup_file_name = NULL;
    char *relative_src_path = NULL; // Holds result of convert_abs_rel if used

    struct stat src_stat;
    struct stat dest_stat;

    int stat_flags = logical ? AT_SYMLINK_FOLLOW : 0;

    int link_result = prev_errno_val;
    if (prev_errno_val < 0) {
        link_result = atomic_link(src_path, dest_dir_fd, dest_path);
    }

    if (((link_result != 0) || (dest_set != NULL)) && !symbolic_link) {
        fstat_src_result = fstatat(AT_FDCWD, src_path, &src_stat, stat_flags);
        if (fstat_src_result != 0) {
            error(0, *__errno_location(), gettext("failed to access %s"), quotearg_style(4, src_path));
            success = false;
            goto cleanup;
        }
    }

    char *current_src_path = src_path;

    if (link_result != 0) {
        if (!symbolic_link && hard_dir_link && ((src_stat.st_mode & S_IFMT) == S_IFDIR)) {
            error(0, 0, gettext("%s: hard link not allowed for directory"), quotearg_n_style_colon(0, 3, src_path));
            success = false;
            goto cleanup;
        }

        if (relative) {
            relative_src_path = convert_abs_rel(src_path, dest_display_name);
            current_src_path = relative_src_path;
        }

        bool handle_existing_dest = (remove_existing_files || interactive || backup_type != 0);

        if (handle_existing_dest) {
            int fstat_dest_result = fstatat(dest_dir_fd, dest_path, &dest_stat, AT_SYMLINK_FOLLOW);
            if (fstat_dest_result == 0) { // Destination exists
                if ((dest_stat.st_mode & S_IFMT) == S_IFDIR) {
                    error(0, 0, gettext("%s: cannot overwrite directory"), quotearg_n_style_colon(0, 3, dest_display_name));
                    success = false;
                    goto cleanup;
                } else {
                    if (!seen_file(dest_set, dest_display_name, &dest_stat)) {
                        bool do_check_same_file = remove_existing_files;
                        if (backup_type != 0) {
                            do_check_same_file = !symbolic_link;
                        }

                        if (do_check_same_file) {
                            if (fstat_src_result != 0) {
                                fstat_src_result = stat(current_src_path, &src_stat);
                            }
                            if ((fstat_src_result == 0) && psame_inode(&src_stat, &dest_stat) &&
                                ((src_stat.st_nlink == 1) || same_nameat(AT_FDCWD, current_src_path, dest_dir_fd, dest_path))) {
                                error(0, 0, gettext("%s and %s are the same file"), quotearg_n_style(0, 4, current_src_path), quotearg_n_style(1, 4, dest_display_name));
                                success = false;
                                goto cleanup;
                            }
                        }

                        if ((link_result < 0) || (link_result == EEXIST)) {
                            if (interactive) {
                                fprintf(_stderr, gettext("%s: replace %s? "), _program_name, quotearg_style(4, dest_display_name));
                                if (yesno() != '\x01') {
                                    success = false;
                                    goto cleanup;
                                }
                            }
                            if (backup_type != 0) {
                                backup_file_name = find_backup_file_name(dest_dir_fd, dest_path, backup_type);
                                if (renameat(dest_dir_fd, dest_path, dest_dir_fd, backup_file_name) != 0) {
                                    int saved_errno = *__errno_location();
                                    free(backup_file_name);
                                    backup_file_name = NULL;
                                    if (saved_errno != ENOENT) {
                                        error(0, saved_errno, gettext("cannot backup %s"), quotearg_style(4, dest_display_name));
                                        success = false;
                                        goto cleanup;
                                    }
                                    handle_existing_dest = false;
                                }
                            }
                        }
                    } else {
                        error(0, 0, gettext("will not overwrite just-created %s with %s"), quotearg_n_style(0, 4, dest_display_name), quotearg_n_style(1, 4, current_src_path));
                        success = false;
                        goto cleanup;
                    }
                }
            } else { // fstat_dest_result != 0, destination does not exist or other error
                int saved_errno = *__errno_location();
                if (saved_errno == ENOENT) {
                    handle_existing_dest = false;
                } else {
                    error(0, saved_errno, gettext("failed to access %s"), quotearg_style(4, dest_display_name));
                    success = false;
                    goto cleanup;
                }
            }
        }
        
        if (!symbolic_link) {
            int link_flags = logical ? AT_SYMLINK_FOLLOW : 0;
            link_result = force_linkat(AT_FDCWD, current_src_path, dest_dir_fd, dest_path, link_flags, handle_existing_dest, link_result);
        } else {
            link_result = force_symlinkat(current_src_path, dest_dir_fd, dest_path, handle_existing_dest, link_result);
        }
    }

    if (link_result < 1) { // 0 or negative indicates success in some coreutils functions
        if (!symbolic_link) {
            record_file(dest_set, dest_display_name, &src_stat);
        }
        if (verbose) {
            const char *prefix_str = DAT_00102004_str;
            const char *suffix_str = DAT_00102004_str;
            char *combined_backup_msg = NULL;

            if (backup_file_name) {
                char *quoted_backup_name = (char *)quotearg_n_style(2, 4, backup_file_name);
                size_t msg_len = strlen(gettext(" (backing up %s)")) + strlen(quoted_backup_name);
                combined_backup_msg = xmalloc(msg_len + 1);
                sprintf(combined_backup_msg, gettext(" (backing up %s)"), quoted_backup_name);
                prefix_str = combined_backup_msg;
                suffix_str = DAT_00102199_str;
            }

            char link_char = symbolic_link ? '-' : '=';
            printf("%s%s%s %c> %s\n",
                   prefix_str,
                   suffix_str,
                   quotearg_n_style(0, 4, dest_display_name),
                   link_char,
                   quotearg_n_style(1, 4, current_src_path));
            free(combined_backup_msg);
        }
    } else { // link_result >= 1, meaning error code
        const char *error_msg_format;
        const char *quoted_src = quotearg_n_style(1, 4, current_src_path);
        const char *quoted_dest = quotearg_n_style(0, 4, dest_display_name);

        if (!symbolic_link) {
            if (link_result == EMLINK) {
                error_msg_format = gettext("failed to create hard link to %.0s%s");
            } else if ((link_result == EXDEV) || (link_result == EEXIST) || (link_result == EPERM) || (link_result == EROFS)) {
                error_msg_format = gettext("failed to create hard link %s");
            } else {
                error_msg_format = gettext("failed to create hard link %s => %s");
            }
        } else {
            if ((link_result == ELOOP) || (current_src_path[0] == '\0')) {
                error_msg_format = gettext("failed to create symbolic link %s -> %s");
            } else {
                error_msg_format = gettext("failed to create symbolic link %s");
            }
        }
        error(0, link_result, error_msg_format, quoted_dest, quoted_src);

        if (backup_file_name) {
            if (renameat(dest_dir_fd, backup_file_name, dest_dir_fd, dest_path) != 0) {
                error(0, *__errno_location(), gettext("cannot un-backup %s"), quotearg_style(4, dest_display_name));
            }
        }
        success = false;
    }

cleanup:
    free(backup_file_name);
    free(relative_src_path);
    return success;
}

// Function: usage
void usage(int status) {
    if (status == 0) {
        printf(gettext(
            "Usage: %s [OPTION]... [-T] TARGET LINK_NAME\n"
            "  or:  %s [OPTION]... TARGET\n"
            "  or:  %s [OPTION]... TARGET... DIRECTORY\n"
            "  or:  %s [OPTION]... -t DIRECTORY TARGET...\n"
        ), _program_name, _program_name, _program_name, _program_name);
        fputs_unlocked(gettext(
            "In the 1st form, create a link to TARGET with the name LINK_NAME.\n"
            "In the 2nd form, create a link to TARGET in the current directory.\n"
            "In the 3rd and 4th forms, create links to each TARGET in DIRECTORY.\n"
            "Create hard links by default, symbolic links with --symbolic.\n"
            "By default, each destination (name of new link) should not already exist.\n"
            "When creating hard links, each TARGET must exist.  Symbolic links\n"
            "can hold arbitrary text; if later resolved, a relative link is\n"
            "interpreted in relation to its parent directory.\n"
        ), _stdout);
        emit_mandatory_arg_note();
        fputs_unlocked(gettext(
            "      --backup[=CONTROL]      make a backup of each existing destination file\n"
            "  -b                          like --backup but does not accept an argument\n"
            "  -d, -F, --directory         allow the superuser to attempt to hard link\n"
            "                                directories (this will probably fail due to\n"
            "                                system restrictions, even for the superuser)\n"
            "  -f, --force                 remove existing destination files\n"
        ), _stdout);
        fputs_unlocked(gettext(
            "  -i, --interactive           prompt whether to remove destinations\n"
            "  -L, --logical               dereference TARGETs that are symbolic links\n"
            "                                it is a symbolic link to a directory\n"
            "  -n, --no-dereference        treat LINK_NAME as a normal file if\n"
            "  -P, --physical              make hard links directly to symbolic links\n"
            "  -r, --relative              with -s, create links relative to link location\n"
            "  -s, --symbolic              make symbolic links instead of hard links\n"
        ), _stdout);
        fputs_unlocked(gettext(
            "  -S, --suffix=SUFFIX         override the usual backup suffix\n"
            "  -t, --target-directory=DIRECTORY  specify the DIRECTORY in which to create\n"
            "                                the links\n"
            "  -T, --no-target-directory   treat LINK_NAME as a normal file always\n"
            "  -v, --verbose               print name of each linked file\n"
        ), _stdout);
        fputs_unlocked(gettext("      --help        display this help and exit\n"), _stdout);
        fputs_unlocked(gettext("      --version     output version information and exit\n"), _stdout);
        emit_backup_suffix_note();
        printf(gettext(
            "\nUsing -s ignores -L and -P.  Otherwise, the last option specified controls\n"
            "behavior when a TARGET is a symbolic link, defaulting to %s.\n"
        ), DAT_00102b4a);
        emit_ancillary_info(DAT_00102b4d);
    } else {
        fprintf(_stderr, gettext("Try '%s --help' for more information.\n"), _program_name);
    }
    exit(status);
}

// Function: main
int main(int argc, char **argv) {
    bool treat_as_no_target_dir = false;
    bool backup_specified = false;
    char *backup_suffix = NULL;
    char *backup_control_arg = NULL;
    char *target_dir = NULL;
    int link_attempt_errno = -1;
    int dest_dir_fd = AT_FDCWD;

    set_program_name(argv[0]);
    setlocale(LC_ALL, "");
    bindtextdomain("coreutils", "/usr/local/share/locale");
    textdomain("coreutils");
    atexit(close_stdin);

    hard_dir_link = false; // Corresponds to -d/F option
    verbose = false;
    interactive = false;
    remove_existing_files = false;
    symbolic_link = false;
    dereference_dest_dir_symlinks = true; // Default

    int c;
    while ((c = getopt_long(argc, argv, "bdfinrst:vFLPS:T", long_options, NULL)) != -1) {
        switch (c) {
            case 257: // --version
                version_etc(_stdout, DAT_00102b4d, "GNU coreutils", _Version,
                            proper_name_lite("Mike Parker", "Mike Parker"),
                            proper_name_lite("David MacKenzie", "David MacKenzie"),
                            (char *)0);
                exit(0);
            case 256: // --help
                usage(0);
                // usage exits
            case 'd':
            case 'F':
                hard_dir_link = true;
                break;
            case 'L':
                logical = true;
                break;
            case 'P':
                logical = false;
                break;
            case 'S':
                backup_specified = true;
                backup_suffix = optarg;
                break;
            case 'T':
                treat_as_no_target_dir = true;
                break;
            case 'b':
                backup_specified = true;
                if (optarg != NULL) {
                    backup_control_arg = optarg;
                }
                break;
            case 'f':
                remove_existing_files = true;
                interactive = false;
                break;
            case 'i':
                remove_existing_files = false;
                interactive = true;
                break;
            case 'n':
                dereference_dest_dir_symlinks = false;
                break;
            case 'r':
                relative = true;
                break;
            case 's':
                symbolic_link = true;
                break;
            case 't': {
                if (target_dir != NULL) {
                    error(1, 0, gettext("multiple target directories specified"));
                }
                struct stat st;
                if (stat(optarg, &st) != 0) {
                    error(1, *__errno_location(), gettext("failed to access %s"), quotearg_style(4, optarg));
                }
                if (!S_ISDIR(st.st_mode)) {
                    error(1, 0, gettext("target %s is not a directory"), quotearg_style(4, optarg));
                }
                target_dir = optarg;
                break;
            }
            case 'v':
                verbose = true;
                break;
            default:
                usage(1);
        }
    }

    int num_operands = argc - optind;
    char **operands = argv + optind;

    if (num_operands < 1) {
        error(0, 0, gettext("missing file operand"));
        usage(1);
    }

    if (relative && !symbolic_link) {
        error(1, 0, gettext("cannot do --relative without --symbolic"));
    }

    if (!hard_dir_link) {
        priv_set_remove_linkdir();
        // beware_hard_dir_link is set to false by priv_set_remove_linkdir
    }

    if (treat_as_no_target_dir) {
        if (target_dir != NULL) {
            error(1, 0, gettext("cannot combine --target-directory and --no-target-directory"));
        }
        if (num_operands == 2) {
            // This is the TARGET LINK_NAME form with -T
            // Proceed to link directly.
        } else if (num_operands < 2) {
            error(0, 0, gettext("missing destination file operand after %s"), quotearg_style(4, operands[0]));
            usage(1);
        } else {
            error(0, 0, gettext("extra operand %s"), quotearg_style(4, operands[2]));
            error(0, 0, gettext("when --no-target-directory is used, only two operands are allowed"));
            usage(1);
        }
    } else { // Not --no-target-directory
        if ((num_operands < 2) && (target_dir == NULL)) {
            target_dir = ".";
            dest_dir_fd = AT_FDCWD;
        } else {
            if ((num_operands == 2) && (target_dir == NULL)) {
                link_attempt_errno = atomic_link(operands[0], AT_FDCWD, operands[1]);
            }

            if (((link_attempt_errno < 0) || (link_attempt_errno == EEXIST) || (link_attempt_errno == ENOTDIR) ||
                 (link_attempt_errno == ELOOP) || (link_attempt_errno == EMLINK))) {
                char *actual_target_dir_path = target_dir;
                if (target_dir == NULL) {
                    actual_target_dir_path = operands[num_operands - 1];
                }

                int open_flags = O_RDONLY | O_DIRECTORY;
                if (!dereference_dest_dir_symlinks) {
                    open_flags |= O_NOFOLLOW;
                }
                
                dest_dir_fd = openat_safer(AT_FDCWD, actual_target_dir_path, open_flags);
                if (dest_dir_fd < 0) {
                    if (!((num_operands == 2) && (target_dir == NULL))) {
                        error(1, *__errno_location(), gettext("target %s"), quotearg_style(4, actual_target_dir_path));
                    }
                } else {
                    if (target_dir == NULL) {
                        num_operands--;
                        target_dir = actual_target_dir_path;
                    }
                }
            }
        }
    }

    if (backup_specified) {
        backup_type = xget_version(gettext("backup type"), backup_control_arg);
    } else {
        backup_type = 0;
    }
    set_simple_backup_suffix(backup_suffix);

    bool overall_success = true;

    if (target_dir == NULL) {
        overall_success = do_link(operands[0], AT_FDCWD, operands[1], operands[1], link_attempt_errno);
    } else {
        if ((num_operands > 1) && remove_existing_files && !symbolic_link && (backup_type != 3)) {
            dest_set = hash_initialize(0x3d, NULL, NULL, NULL);
            if (dest_set == NULL) {
                xalloc_die();
            }
        }

        for (int i = 0; i < num_operands; ++i) {
            char *source_file = operands[i];
            char *dest_file_name_component = last_component(source_file);
            char *full_dest_path_buffer = NULL;
            char *full_dest_path = file_name_concat(target_dir, dest_file_name_component, &full_dest_path_buffer);
            strip_trailing_slashes(full_dest_path);

            bool current_link_success = do_link(source_file, dest_dir_fd, full_dest_path, full_dest_path, -1);
            overall_success &= current_link_success;

            free(full_dest_path_buffer);
        }
    }

    if (dest_dir_fd != AT_FDCWD) {
        close(dest_dir_fd);
    }

    return !overall_success; // Exit 0 on success, 1 on failure
}