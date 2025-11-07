#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <locale.h>
#include <getopt.h>
#include <stdbool.h>
#include <libintl.h> // For gettext
#include <stdarg.h> // For va_list in error function

// --- Global variables and constants (simulated from GNU coreutils context) ---
char *program_name = NULL;
#define VERSION "8.32"

// String literals and option definitions
static const char *const DAT_001015b9 = "pwd";
static const char *const DAT_001015bd = "../";
static const char *const DAT_00101659 = ".";
static const char *const DAT_001015f3 = ".";
static const char *const DAT_0010130c = ""; // For root directory "/"

static const char *const short_options = "LP";

static struct option const longopts[] = {
    {"logical", no_argument, NULL, 'L'},
    {"physical", no_argument, NULL, 'P'},
    {"help", no_argument, NULL, 0},
    {"version", no_argument, NULL, 0},
    {NULL, 0, NULL, 0}};

// --- Dummy/Simplified GNU coreutils functions ---

// Error reporting (simplified)
void error(int status, int errnum, const char *format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, "%s: ", program_name ? program_name : "unknown");
    vfprintf(stderr, format, args);
    if (errnum != 0) {
        fprintf(stderr, ": %s", strerror(errnum));
    }
    fprintf(stderr, "\n");
    va_end(args);
    if (status) exit(status);
}

// Memory allocation with error checking
void *xmalloc(size_t s) {
    void *p = malloc(s);
    if (!p) error(EXIT_FAILURE, errno, gettext("memory exhausted"));
    return p;
}

void *xnmalloc(size_t n, size_t s) {
    if (s == 0 || n == 0) return xmalloc(1);
    if ((size_t)-1 / n < s) { // Check for overflow
        error(EXIT_FAILURE, 0, gettext("memory exhausted (allocation too large)"));
    }
    return xmalloc(n * s);
}

// Simplified xpalloc. In coreutils, this is a more sophisticated realloc-like function
// that handles growing arrays with specific strategies. Here, it just reallocates.
void *xpalloc(int flags, size_t *capacity_ptr, size_t new_needed, size_t max_size, int elem_size) {
    size_t current_capacity = *capacity_ptr;
    size_t new_capacity = current_capacity + new_needed;
    if (new_capacity < current_capacity || (new_capacity > max_size && max_size != (size_t)-1)) {
        error(EXIT_FAILURE, 0, gettext("memory exhausted (allocation too large)"));
    }

    void *p = xmalloc(new_capacity);
    *capacity_ptr = new_capacity;
    return p;
}

// Other dummy functions
void set_program_name(char *name) {
    program_name = name;
}
void emit_ancillary_info(const char *program) {
    // fprintf(stderr, gettext("Report bugs to: %s\n"), "bug-coreutils@gnu.org");
}
void close_stdout(void) {
    if (fclose(stdout) != 0) {
        error(EXIT_FAILURE, errno, gettext("failed to close stdout"));
    }
}
char *quote(const char *str) { return (char *)str; } // Simplified
char *quotearg_style(int style, const char *arg) { return (char *)arg; } // Simplified
char *proper_name_lite(const char *s1, const char *s2) { return (char *)s1; } // Simplified
void version_etc(FILE *stream, const char *program, const char *package, const char *version, const char *authors, ...) {
    fprintf(stream, "%s (%s) %s\n", program, package, version);
    fprintf(stream, gettext("Copyright (C) 2020 Free Software Foundation, Inc.\n"));
    fprintf(stream, gettext("License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\n"));
    fprintf(stream, gettext("This is free software: you are free to change and redistribute it.\nThere is NO WARRANTY, to the extent permitted by law.\n\n"));
    fprintf(stream, gettext("Written by %s.\n"), authors);
}

// Dummy for readdir_ignoring_dot_and_dotdot
struct dirent *readdir_ignoring_dot_and_dotdot(DIR *dirp) {
    struct dirent *dp;
    while ((dp = readdir(dirp)) != NULL) {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) {
            return dp;
        }
    }
    return NULL;
}

// Dummy for get_root_dev_ino. Returns a static buffer with root's dev and ino.
static dev_t root_dev_ino_buffer[2]; // [st_dev, st_ino]
dev_t *get_root_dev_ino(void *buffer_unused) {
    struct stat st;
    if (stat("/", &st) == 0) {
        root_dev_ino_buffer[0] = st.st_dev;
        root_dev_ino_buffer[1] = st.st_ino;
        return root_dev_ino_buffer;
    }
    return NULL;
}

// Dummy for psame_inode
bool psame_inode(const struct stat *s1, const struct stat *s2) {
    return (s1->st_ino == s2->st_ino && s1->st_dev == s2->st_dev);
}

// Dummy for xgetcwd (wrapper around getcwd(3))
char *xgetcwd(void) {
    char *buf = NULL;
    size_t size = 128;
    while (true) {
        buf = realloc(buf, size);
        if (!buf) {
            error(EXIT_FAILURE, errno, gettext("memory exhausted"));
        }
        if (getcwd(buf, size) != NULL) {
            return buf;
        }
        if (errno != ERANGE) {
            free(buf);
            return NULL;
        }
        size *= 2;
    }
}

// --- Custom structure for file_name management ---
typedef struct FileName {
    char *buffer;
    size_t capacity;
    char *current_pos; // Points to the null terminator or one past the last character
} FileName;

// Function: usage
void usage(int exit_status) {
    if (exit_status == 0) {
        printf(gettext("Usage: %s [OPTION]...\n"), program_name);
        fputs_unlocked(gettext("Print the full filename of the current working directory.\n\n"), stdout);
        fputs_unlocked(gettext("  -L, --logical   use PWD from environment, even if it contains symlinks\n"), stdout);
        fputs_unlocked(gettext("  -P, --physical  resolve all symlinks\n"), stdout);
        fputs_unlocked(gettext("      --help        display this help and exit\n"), stdout);
        fputs_unlocked(gettext("      --version     output version information and exit\n"), stdout);
        fputs_unlocked(gettext("\nIf no option is specified, -P is assumed.\n"), stdout);
        printf(gettext("\nYour shell may have its own version of %s, which usually supersedes\nthe version described here.  Please refer to your shell\'s documentation\nfor details about the options it supports.\n"), DAT_001015b9);
        emit_ancillary_info(DAT_001015b9);
    } else {
        fprintf(stderr, gettext("Try \'%s --help\' for more information.\n"), program_name);
    }
    exit(exit_status);
}

// Function: file_name_free
void file_name_free(FileName *fn) {
    free(fn->buffer);
    free(fn);
}

// Function: file_name_init
FileName *file_name_init(void) {
    FileName *fn = (FileName *)xmalloc(sizeof(FileName));
    fn->capacity = 0x2000; // 8192 bytes
    fn->buffer = (char *)xmalloc(fn->capacity);
    fn->current_pos = fn->buffer + fn->capacity - 1; // Points to the last byte
    *fn->current_pos = '\0'; // Null-terminate at the end of the buffer
    return fn;
}

// Function: file_name_prepend
void file_name_prepend(FileName *fn, const char *name, size_t name_len) {
    size_t current_str_len_from_start = fn->current_pos - fn->buffer;
    size_t needed_space = name_len + 1; // +1 for the '/' character

    if (current_str_len_from_start < needed_space) {
        size_t old_capacity = fn->capacity;
        size_t new_extra_capacity = needed_space - current_str_len_from_start;
        char *new_buffer = (char *)xpalloc(0, &fn->capacity, new_extra_capacity, (size_t)-1, 1);

        size_t old_string_len = old_capacity - current_str_len_from_start;
        char *old_string_start = fn->current_pos;

        char *new_string_start_in_new_buf = new_buffer + (fn->capacity - old_string_len);
        memcpy(new_string_start_in_new_buf, old_string_start, old_string_len);

        free(fn->buffer);
        fn->buffer = new_buffer;
        fn->current_pos = new_string_start_in_new_buf;
    }

    fn->current_pos -= (name_len + 1); // Move pointer back for '/' and name
    *fn->current_pos = '/';
    memcpy(fn->current_pos + 1, name, name_len);
}

// Function: nth_parent
char *nth_parent(size_t n) {
    if (n == 0) return xmalloc(1);
    char *path = (char *)xnmalloc(n, 3);
    char *p = path;
    for (size_t i = 0; i < n; ++i) {
        memcpy(p, DAT_001015bd, 3); // Copy "../"
        p += 3;
    }
    if (n > 0) {
        *(p - 1) = '\0'; // Overwrite the last '/' with null terminator
    }
    return path;
}

// Function: find_dir_entry
void find_dir_entry(struct stat *current_dir_stat, FileName *file_name_info, size_t depth) {
    DIR *dirp = opendir("..");
    if (!dirp) {
        char *quoted_path = quote(nth_parent(depth));
        error(EXIT_FAILURE, errno, gettext("cannot open directory %s"), quoted_path);
        free(quoted_path);
    }

    int fd = dirfd(dirp);
    int chdir_ret;
    if (fd < 0) {
        chdir_ret = chdir("..");
    } else {
        chdir_ret = fchdir(fd);
    }

    if (chdir_ret < 0) {
        char *quoted_path = quote(nth_parent(depth));
        error(EXIT_FAILURE, errno, gettext("failed to chdir to %s"), quoted_path);
        free(quoted_path);
    }

    struct stat new_current_dir_stat;
    int stat_ret;
    if (fd < 0) {
        stat_ret = stat(".", &new_current_dir_stat);
    } else {
        stat_ret = fstat(fd, &new_current_dir_stat);
    }

    if (stat_ret < 0) {
        char *quoted_path = quote(nth_parent(depth));
        error(EXIT_FAILURE, errno, gettext("failed to stat %s"), quoted_path);
        free(quoted_path);
    }

    bool found_entry = false;
    struct dirent *dp;
    struct stat entry_stat;
    int original_errno;

    while (true) {
        errno = 0;
        dp = readdir_ignoring_dot_and_dotdot(dirp);

        if (!dp) {
            original_errno = errno;
            if (original_errno != 0) {
                closedir(dirp);
                dirp = NULL;
                char *quoted_path = quote(nth_parent(depth));
                error(EXIT_FAILURE, original_errno, gettext("reading directory %s"), quoted_path);
                free(quoted_path);
            }
            break;
        }

        bool same_device = (new_current_dir_stat.st_dev == current_dir_stat->st_dev);
        ino_t current_entry_ino = dp->d_ino;

        if (current_entry_ino == 0 || !same_device) {
            if (lstat(dp->d_name, &entry_stat) < 0) {
                continue;
            }
            current_entry_ino = entry_stat.st_ino;
        }

        if (current_entry_ino == current_dir_stat->st_ino && (same_device || entry_stat.st_dev == current_dir_stat->st_dev)) {
            size_t name_len = strlen(dp->d_name);
            file_name_prepend(file_name_info, dp->d_name, name_len);
            found_entry = true;
            break;
        }
    }

    if (dirp && closedir(dirp) != 0) {
        char *quoted_path = quote(nth_parent(depth));
        error(EXIT_FAILURE, errno, gettext("reading directory %s"), quoted_path);
        free(quoted_path);
    }

    if (!found_entry) {
        char *quoted_path = quote(nth_parent(depth));
        error(EXIT_FAILURE, 0, gettext("couldn\'t find directory entry in %s with matching i-node"), quoted_path);
        free(quoted_path);
    }

    *current_dir_stat = new_current_dir_stat;
}

// Function: robust_getcwd
void robust_getcwd(FileName *fn) {
    size_t depth = 1;
    dev_t *root_info = get_root_dev_ino(NULL);
    if (!root_info) {
        error(EXIT_FAILURE, errno, gettext("failed to get attributes of %s"), quotearg_style(4, DAT_00101659));
    }

    struct stat current_stat;
    if (stat(".", &current_stat) < 0) {
        error(EXIT_FAILURE, errno, gettext("failed to stat %s"), quotearg_style(4, DAT_001015f3));
    }

    while (current_stat.st_ino != root_info[1] || current_stat.st_dev != root_info[0]) {
        find_dir_entry(&current_stat, fn, depth);
        depth++;
    }

    if (*fn->current_pos == '\0') {
        file_name_prepend(fn, DAT_0010130c, 0);
    }
}

// Function: logical_getcwd
char *logical_getcwd(void) {
    char *pwd_env = getenv("PWD");
    if (!pwd_env || *pwd_env != '/') {
        return NULL;
    }

    char *path_to_check = strdup(pwd_env);
    if (!path_to_check) {
        error(EXIT_FAILURE, errno, gettext("memory exhausted"));
    }

    char *current_segment = path_to_check;
    while ((current_segment = strstr(current_segment, "/.")) != NULL) {
        if (current_segment[2] == '\0' || current_segment[2] == '/') {
            free(path_to_check);
            return NULL;
        }
        if (current_segment[2] == '.' && (current_segment[3] == '\0' || current_segment[3] == '/')) {
            free(path_to_check);
            return NULL;
        }
        current_segment++;
    }

    struct stat pwd_stat, current_stat;
    if (stat(pwd_env, &pwd_stat) != 0 || stat(".", &current_stat) != 0 || !psame_inode(&pwd_stat, &current_stat)) {
        free(path_to_check);
        return NULL;
    }

    free(path_to_check);
    return pwd_env;
}

// Function: main
int main(int argc, char **argv) {
    set_program_name(argv[0]);
    setlocale(LC_ALL, "");
    bindtextdomain("coreutils", "/usr/local/share/locale");
    textdomain("coreutils");
    atexit(close_stdout);

    bool logical_mode = false;
    const char *posixly_correct = getenv("POSIXLY_CORRECT");

    int opt;
    while ((opt = getopt_long(argc, argv, short_options, longopts, NULL)) != -1) {
        switch (opt) {
            case 'L':
                logical_mode = true;
                break;
            case 'P':
                logical_mode = false;
                break;
            case 0: // Long options `--help` or `--version`
                if (strcmp("help", longopts[optind - 1].name) == 0) {
                    usage(0);
                } else if (strcmp("version", longopts[optind - 1].name) == 0) {
                    version_etc(stdout, DAT_001015b9, "GNU coreutils", VERSION, proper_name_lite("Jim Meyering", "Jim Meyering"));
                    exit(0);
                }
                break;
            default:
                usage(EXIT_FAILURE);
        }
    }

    if (optind < argc) {
        error(0, 0, gettext("ignoring non-option arguments"));
    }

    char *result_path = NULL;
    FileName *fn = NULL;

    if (posixly_correct) {
        result_path = logical_getcwd();
    } else if (logical_mode) {
        result_path = logical_getcwd();
    }

    if (result_path) {
        puts(result_path);
    } else {
        char *physical_path = xgetcwd();
        if (physical_path) {
            puts(physical_path);
            free(physical_path);
        } else {
            fn = file_name_init();
            robust_getcwd(fn);
            puts(fn->buffer);
            file_name_free(fn);
        }
    }

    return 0;
}