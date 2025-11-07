#include <stdio.h>   // FILE, fdopen, SEEK_SET, clearerr_unlocked, fileno_unlocked
#include <stdlib.h>  // realloc, free, mkstemp
#include <unistd.h>  // unlink, close, ftruncate, off_t
#include <errno.h>   // errno, EINVAL
#include <stdbool.h> // bool, true

// Dummy declarations for external functions to make the code compilable.
// In a real project, these would come from appropriate header files or linked libraries.
void error(int status, int errnum, const char *format, ...);
const char *gettext(const char *msgid);
char *quotearg_style(int style, const char *arg);
int path_search(char *buf, size_t buflen, int flags, const char *pattern, int type);
void *rpl_realloc(void *ptr, size_t size); // Assume it's realloc or wrapper
int mkstemp_safer(char *template); // Assume it's mkstemp or wrapper
int rpl_fseeko(FILE *stream, off_t offset, int whence); // Assume it's fseeko or wrapper
int *__errno_location(void); // GNU extension for errno

// Assume these are declared globally or file-scope static in the original context
static char *tempfile_1 = NULL;
static FILE *tmp_fp_0 = NULL;

// Function: record_or_unlink_tempfile
void record_or_unlink_tempfile(char *param_1) {
  unlink(param_1);
  return;
}

// Function: temp_stream
int temp_stream(FILE **param_1, char **param_2) { // Changed undefined8 to int for return type 0/1
  if (tempfile_1 == NULL) { // First time setup
    char *name_buf = NULL;
    size_t name_buf_len = 0x80; // Initial buffer size 128

    while (true) { // Loop to find a suitable temporary file name
      name_buf = (char *)rpl_realloc(name_buf, name_buf_len);
      if (name_buf == NULL) {
        error(0, *__errno_location(), gettext("failed to make temporary file name"));
        return 0; // Failure due to realloc
      }

      if (path_search(name_buf, name_buf_len, 0, "cutmp", 1) == 0) { // Found a path
        tempfile_1 = name_buf; // Store the chosen name globally
        int fd = mkstemp_safer(tempfile_1); // Create the temporary file

        if (fd < 0) { // Failed to create file
          error(0, *__errno_location(), gettext("failed to create temporary file %s"), quotearg_style(4, tempfile_1));
          free(tempfile_1); // Free the buffer
          tempfile_1 = NULL; // Reset global pointer
          return 0; // Failure
        }

        tmp_fp_0 = fdopen(fd, "w+"); // Open the file stream
        if (tmp_fp_0 == NULL) { // Failed to open stream
          error(0, *__errno_location(), gettext("failed to open %s for writing"), quotearg_style(4, tempfile_1));
          close(fd); // Close the file descriptor
          unlink(tempfile_1); // Remove the created file
          free(tempfile_1); // Free the buffer
          tempfile_1 = NULL; // Reset global pointer
          return 0; // Failure
        }

        // Success: file created and stream opened
        record_or_unlink_tempfile(tempfile_1); // Call cleanup handler (original logic implies this is part of successful setup)
        *param_1 = tmp_fp_0;
        if (param_2 != NULL) {
          *param_2 = tempfile_1;
        }
        return 1; // Success
      }

      // path_search failed, check if recoverable
      int err = *__errno_location();
      // Original logic: if (err != EINVAL || name_buf_len > 0x800) then unrecoverable.
      if (err != EINVAL || name_buf_len > 0x800) { // Check for unrecoverable errors (0x800 is 2048)
        // Special handling for EINVAL: return 0x24 (36) instead of EINVAL (22)
        error(0, (err == EINVAL) ? 0x24 : err, gettext("failed to make temporary file name"));
        free(name_buf); // Free buffer before returning
        return 0; // Failure
      }

      // Recoverable error (EINVAL and buffer not too large), double buffer size and retry
      name_buf_len <<= 1;
    }
  } else { // tempfile_1 is not NULL, rewind existing stream
    clearerr_unlocked(tmp_fp_0); // Clear any error indicators

    if (rpl_fseeko(tmp_fp_0, 0, SEEK_SET) < 0 || ftruncate(fileno_unlocked(tmp_fp_0), 0) < 0) {
      // Failed to rewind or truncate
      error(0, *__errno_location(), gettext("failed to rewind stream for %s"), quotearg_style(4, tempfile_1));
      return 0; // Failure
    }

    // Success: stream rewound and truncated
    *param_1 = tmp_fp_0;
    if (param_2 != NULL) {
      *param_2 = tempfile_1;
    }
    return 1; // Success
  }
}