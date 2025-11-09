#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <libintl.h> // For gettext, though a dummy is provided if not available
#include <limits.h> // For PATH_MAX
#include <stdio.h>  // For fprintf, perror
#include <stdarg.h> // For va_list
#include <libgen.h> // For dirname (POSIX)

// --- Dummy implementations for coreutils-like functions and `error` ---
// These are provided to make the code self-contained and compilable.
// In a real coreutils environment, these would be provided by the library.

// Global variable for program name, used by the dummy error function
static char *progname = "program";

// Define quotearg_style enum (usually in quote.h from coreutils)
#ifndef QUOTEARG_STYLE_T
enum quotearg_style
{
  shell_escape_always = 4 // Common style for path quoting in coreutils
};
#define QUOTEARG_STYLE_T
#endif

// Dummy gettext (returns the input string, effectively disabling localization)
#ifndef HAVE_GETTEXT
#define gettext(string) (string)
#endif

// Dummy error function (simplified version of coreutils' error)
// Prints an error message to stderr and optionally exits.
void error(int status, int errnum, const char *format, ...) {
    va_list args;
    fprintf(stderr, "%s: ", progname);
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

// Dummy xgetcwd: Returns a dynamically allocated string for the current working directory.
// Returns NULL on error, setting errno.
char *xgetcwd(void) {
    char *buf = malloc(PATH_MAX);
    if (buf == NULL) {
        errno = ENOMEM;
        return NULL;
    }
    if (getcwd(buf, PATH_MAX) == NULL) {
        free(buf);
        return NULL;
    }
    return buf;
}

// Dummy quotearg_style: Quotes a string according to a specified style.
// Returns a dynamically allocated string.
char *quotearg_style(enum quotearg_style style, const char *arg) {
    (void)style; // `style` parameter is unused in this dummy implementation
    return strdup(arg); // Simple duplication for demonstration
}

// Dummy dir_name: Returns the directory component of a path.
// Returns a dynamically allocated string (or NULL on error).
char *dir_name(const char *path) {
    char *path_copy = strdup(path);
    if (!path_copy) {
        errno = ENOMEM;
        return NULL;
    }
    char *result_ptr = dirname(path_copy); // `dirname` may modify `path_copy` or return a static buffer
    char *heap_result = NULL;
    if (result_ptr) {
        heap_result = strdup(result_ptr); // Make a heap-allocated copy
    }
    free(path_copy); // Free the temporary copy created by strdup
    return heap_result;
}

// Opaque state structure for saving/restoring CWD.
// The original snippet used `local_158 [16]`, indicating a small, fixed-size buffer.
// This struct matches that size for compatibility with the function signatures.
struct saved_cwd_state { char data[16]; };

// Dummy save_cwd: Saves the current working directory state.
// Returns 0 on success, non-zero on error.
int save_cwd(struct saved_cwd_state *state) {
    // In a real implementation, this would store the actual CWD path or a handle in `state->data`.
    (void)state; // `state` parameter is unused in this dummy implementation
    return 0; // Indicate success
}

// Dummy restore_cwd: Restores a previously saved working directory state.
// Returns 0 on success, non-zero on error.
int restore_cwd(struct saved_cwd_state *state) {
    // In a real implementation, this would use `state->data` to restore the CWD.
    (void)state; // `state` parameter is unused in this dummy implementation
    return 0; // Indicate success
}

// Dummy free_cwd: Frees resources associated with a saved CWD state.
void free_cwd(struct saved_cwd_state *state) {
    // In a real implementation, this would free any internal allocations made by `save_cwd`.
    (void)state; // `state` parameter is unused in this dummy implementation
}
// --- End of Dummy implementations ---

// Constants for ".." and "." paths
static const char DOTDOT[] = "..";
static const char DOT[] = ".";

// Function: find_mount_point
// Finds the mount point of the file system containing `path` relative to `initial_stat_info`.
//
// Parameters:
//   path: The path to start searching from.
//   initial_stat_info: A pointer to a `struct stat` containing the initial
//                      file status information for `path`.
//
// Returns:
//   A dynamically allocated string containing the canonical path of the mount point,
//   or NULL on error. The caller is responsible for freeing the returned string.
char *find_mount_point(const char *path, const struct stat *initial_stat_info) {
  char *final_result_path = NULL;
  struct saved_cwd_state cwd_state;
  int saved_errno = 0;
  char *temp_dirpath = NULL; // Used for the heap-allocated result of `dir_name()`

  // 1. Save the current working directory to allow restoration later.
  if (save_cwd(&cwd_state) != 0) {
    error(0, errno, gettext("cannot get current directory"));
    return NULL;
  }

  // Use a `do-while(0)` block to create a structured single exit point for cleanup,
  // replacing multiple `goto` statements.
  do {
    struct stat current_stat;
    int chdir_successful = 0; // Flag to track if an initial `chdir` was successful

    // Determine the starting directory for traversal based on `initial_stat_info`.
    if (S_ISDIR(initial_stat_info->st_mode)) {
      // If `path` is a directory, start by changing into it.
      current_stat = *initial_stat_info; // Copy the initial stat info
      if (chdir(path) < 0) {
        char *quoted_arg = quotearg_style(shell_escape_always, path);
        error(0, errno, gettext("cannot change to directory %s"), quoted_arg);
        free(quoted_arg); // Free the temporary quoted string
        break; // Exit `do-while(0)` for cleanup
      }
      chdir_successful = 1;
    } else {
      // If `path` is not a directory, we need to `chdir` to its parent directory.
      temp_dirpath = dir_name(path); // `dir_name` returns a heap-allocated string
      if (!temp_dirpath) {
          error(0, errno, gettext("cannot determine directory name for %s"), path);
          break;
      }

      if (chdir(temp_dirpath) < 0) {
        char *quoted_arg = quotearg_style(shell_escape_always, temp_dirpath);
        error(0, errno, gettext("cannot change to directory %s"), quoted_arg);
        free(quoted_arg);
        break;
      }
      chdir_successful = 1;

      // After changing directory, get the stat info for the *current* directory (".")
      if (stat(DOT, &current_stat) < 0) {
        char *quoted_arg = quotearg_style(shell_escape_always, DOT);
        error(0, errno, gettext("cannot stat current directory (now %s)"), quoted_arg);
        free(quoted_arg);
        break;
      }
    }

    // Common loop logic: Traverse upwards until the mount point or root is found.
    if (chdir_successful) {
      while (1) { // Loop indefinitely until a `break` or `return` condition is met
        struct stat parent_stat;
        if (stat(DOTDOT, &parent_stat) < 0) {
          char *quoted_arg = quotearg_style(shell_escape_always, DOTDOT);
          error(0, errno, gettext("cannot stat %s"), quoted_arg);
          free(quoted_arg);
          break; // Exit inner loop and proceed to cleanup
        }

        // Check for mount point or root:
        // - If device ID changes (`st_dev`), it's a mount point.
        // - If inode number is the same (`st_ino`), we've reached the root of the filesystem.
        if ((parent_stat.st_dev != current_stat.st_dev) || (parent_stat.st_ino == current_stat.st_ino)) {
          final_result_path = xgetcwd(); // Success: get the canonical path of the mount point
          break; // Exit inner loop and proceed to cleanup
        }

        // Move up to the parent directory
        if (chdir(DOTDOT) < 0) {
          char *quoted_arg = quotearg_style(shell_escape_always, DOTDOT);
          error(0, errno, gettext("cannot change to directory %s"), quoted_arg);
          free(quoted_arg);
          break; // Exit inner loop and proceed to cleanup
        }
        current_stat = parent_stat; // Update stat info for the new current directory
      }
    }

  } while (0); // End of `do-while(0)` block for cleanup

  // Cleanup section: This block is always executed before returning.
  saved_errno = errno; // Preserve `errno` before cleanup functions might change it
  if (restore_cwd(&cwd_state) != 0) {
    error(1, errno, gettext("failed to return to initial working directory"));
    // Note: The original code printed this as a fatal error (status 1) but
    // then continued, preserving the `final_result_path`. If `error(1, ...)`
    // calls `exit()`, this part is unreachable after the error.
  }
  free_cwd(&cwd_state); // Free any resources held by the saved CWD state
  errno = saved_errno; // Restore `errno` to its state before cleanup

  free(temp_dirpath); // Free memory allocated by `dir_name` if it was used

  return final_result_path;
}

// Minimal main function for compilation and demonstration
int main(int argc, char *argv[]) {
    progname = argv[0]; // Set global program name for the dummy error function

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <path>\n", progname);
        return 1;
    }

    const char *target_path = argv[1];
    struct stat st;

    // Get initial stat info for the target path.
    // Use `lstat` to get information about the symbolic link itself,
    // not its target, which is generally safer for path traversal logic.
    if (lstat(target_path, &st) < 0) {
        perror("lstat failed for target path");
        return 1;
    }

    // Call the refactored function to find the mount point.
    char *mount_point_path = find_mount_point(target_path, &st);

    if (mount_point_path) {
        printf("Mount point for '%s': %s\n", target_path, mount_point_path);
        free(mount_point_path); // Free the dynamically allocated string
    } else {
        fprintf(stderr, "Failed to find mount point for '%s'.\n", target_path);
        return 1;
    }

    return 0;
}