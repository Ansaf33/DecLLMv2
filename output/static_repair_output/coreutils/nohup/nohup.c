#include <stdio.h>    // For FILE, printf, fprintf, fputs_unlocked, ferror_unlocked, stdout, stderr
#include <stdlib.h>   // For exit, getenv, atexit, free
#include <stdbool.h>  // For _Bool
#include <string.h>   // For NULL
#include <unistd.h>   // For isatty, dup2, execvp, close, STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO
#include <fcntl.h>    // For open, fcntl flags (O_RDONLY, O_WRONLY, O_CREAT, O_APPEND, F_DUPFD_CLOEXEC)
#include <errno.h>    // For errno, EIO, ENOENT
#include <signal.h>   // For signal, SIGHUP, SIG_IGN
#include <locale.h>   // For setlocale, LC_ALL
#include <libintl.h>  // For gettext, bindtextdomain, textdomain
#include <sys/types.h> // For mode_t
#include <sys/stat.h> // For umask, S_IRUSR, S_IWUSR, S_IRGRP, S_IWGRP

// Coreutils specific externs (simplified declarations; these would typically be in coreutils system headers)
extern char *program_name;
extern const char *Version;
extern int optind;

// Function prototypes for coreutils functions
extern void set_program_name(const char *);
extern void initialize_exit_failure(int);
extern void close_stdout(void);
extern const char *proper_name_lite(const char *, const char *);
extern void parse_gnu_standard_options_only(int, char **, const char *, const char *, const char *, int, void (*)(int), const char *, int);
extern void error(int, int, const char *, ...);
extern int fd_reopen(int, const char *, int, mode_t);
extern char *file_name_concat(const char *, const char *, int);
extern const char *quotearg_style(int, const char *);
extern int rpl_fcntl(int, int, ...);
extern void emit_exec_status(const char *);
extern void emit_ancillary_info(const char *);

// Function: usage
void usage(int status) {
  const char *prog_name_str = program_name;
  if (status == 0) {
    printf(gettext("Usage: %s COMMAND [ARG]...\n  or:  %s OPTION\n"), prog_name_str, prog_name_str);
    fputs_unlocked(gettext("Run COMMAND, ignoring hangup signals.\n\n"), stdout);
    fputs_unlocked(gettext("      --help        display this help and exit\n"), stdout);
    fputs_unlocked(gettext("      --version     output version information and exit\n"), stdout);
    printf(gettext(
                            "\nIf standard input is a terminal, redirect it from an unreadable file.\nIf standard output is a terminal, append output to \'nohup.out\' if possible,\n\'$HOME/nohup.out\' otherwise.\nIf standard error is a terminal, redirect it to standard output.\nTo save output to FILE, use \'%s COMMAND > FILE\'.\n"
                            ), prog_name_str);
    printf(gettext(
                            "\nYour shell may have its own version of %s, which usually supersedes\nthe version described here.  Please refer to your shell\'s documentation\nfor details about the options it supports.\n"
                            ), "nohup");
    emit_exec_status("nohup");
    emit_ancillary_info("nohup");
  } else {
    fprintf(stderr, gettext("Try \'%s --help\' for more information.\n"), prog_name_str);
  }
  exit(status);
}

// Function: main
int main(int argc, char **argv) {
  int exit_failure_code;
  int output_fd_for_closing = -1; // FD for nohup.out, -1 if not opened or already closed
  int saved_stderr_fd = -1;       // Original STDERR_FILENO, if redirected

  set_program_name(argv[0]);
  setlocale(LC_ALL, "");
  bindtextdomain("coreutils", "/usr/local/share/locale");
  textdomain("coreutils");

  exit_failure_code = (getenv("POSIXLY_CORRECT") == NULL) ? 125 : 127;
  initialize_exit_failure(exit_failure_code);
  atexit(close_stdout);

  parse_gnu_standard_options_only(argc, argv, "nohup", "GNU coreutils", Version, 0, usage, proper_name_lite("Jim Meyering", "Jim Meyering"), 0);

  if (argc <= optind) {
    error(0, 0, gettext("missing operand"));
    usage(exit_failure_code);
  }

  _Bool input_is_tty = isatty(STDIN_FILENO);
  _Bool output_is_tty = isatty(STDOUT_FILENO);
  // bVar1 in original code is true if !output_is_tty && errno == EIO (EIO is 9)
  _Bool ignore_eio_on_output = !output_is_tty && (errno == EIO);
  _Bool error_is_tty = isatty(STDERR_FILENO);

  if (input_is_tty) {
    if (fd_reopen(STDIN_FILENO, "/dev/null", O_RDONLY, 0) < 0) {
      error(exit_failure_code, errno, gettext("failed to render standard input unusable"));
    }
    if (!output_is_tty && !error_is_tty) {
      error(0, 0, gettext("ignoring input"));
    }
  }

  if (output_is_tty || (error_is_tty && ignore_eio_on_output)) {
    mode_t old_mask = umask(0);
    int open_flags = O_WRONLY | O_CREAT | O_APPEND;
    mode_t file_mode = S_IRUSR | S_IWUSR; // 0600 octal

    const char *current_output_filename = "nohup.out";
    char *home_nohup_out_path = NULL; // Allocated only if needed

    if (output_is_tty) {
      output_fd_for_closing = fd_reopen(STDOUT_FILENO, current_output_filename, open_flags, file_mode);
    } else {
      output_fd_for_closing = open(current_output_filename, open_flags, file_mode);
    }

    if (output_fd_for_closing < 0) {
      int first_errno = errno;
      const char *home_dir = getenv("HOME");
      if (home_dir != NULL) {
        home_nohup_out_path = file_name_concat(home_dir, "nohup.out", 0);
        if (home_nohup_out_path != NULL) {
          if (output_is_tty) {
            output_fd_for_closing = fd_reopen(STDOUT_FILENO, home_nohup_out_path, open_flags, file_mode);
          } else {
            output_fd_for_closing = open(home_nohup_out_path, open_flags, file_mode);
          }
        }
      }

      if (output_fd_for_closing < 0) {
        // Both attempts failed
        error(0, first_errno, gettext("failed to open %s"), quotearg_style(4, "nohup.out"));
        if (home_nohup_out_path != NULL) {
            error(0, errno, gettext("failed to open %s"), quotearg_style(4, home_nohup_out_path));
            free(home_nohup_out_path);
        }
        umask(old_mask);
        return exit_failure_code;
      }
      current_output_filename = home_nohup_out_path; // Use the path in HOME
    }
    umask(old_mask);

    error(0, 0, gettext(input_is_tty ? "ignoring input and appending output to %s" : "appending output to %s"), quotearg_style(4, current_output_filename));
    free(home_nohup_out_path); // Safe to call free(NULL)
  }

  if (error_is_tty) {
    saved_stderr_fd = rpl_fcntl(STDERR_FILENO, F_DUPFD_CLOEXEC, 3);
    if (!output_is_tty) {
      error(0, 0, gettext(input_is_tty ? "ignoring input and redirecting standard error to standard output" : "redirecting standard error to standard output"));
    }
    // Determine the FD to redirect stderr to: either STDOUT_FILENO or the opened nohup.out file.
    int target_output_fd = (output_fd_for_closing == -1) ? STDOUT_FILENO : output_fd_for_closing;
    if (dup2(target_output_fd, STDERR_FILENO) < 0) {
      error(exit_failure_code, errno, gettext("failed to redirect standard error"));
    }
    if (ignore_eio_on_output && output_fd_for_closing != -1) {
      close(output_fd_for_closing);
      output_fd_for_closing = -1; // Indicate it's closed
    }
  }

  if (ferror_unlocked(stderr) == 0) {
    signal(SIGHUP, SIG_IGN);
    char **command_argv = argv + optind;
    execvp(command_argv[0], command_argv);

    // If execvp fails
    int exec_errno = errno;
    int final_exit_status = (exec_errno == ENOENT) ? 127 : 126;

    // Try to restore stderr if it was redirected
    if (saved_stderr_fd != -1) {
        if (dup2(saved_stderr_fd, STDERR_FILENO) != STDERR_FILENO) {
            // Failed to restore stderr, cannot print error message reliably
        }
        close(saved_stderr_fd); // Close the duplicated FD
    }
    error(0, exec_errno, gettext("failed to run command %s"), quotearg_style(4, command_argv[0]));
    return final_exit_status;
  }
  // If ferror_unlocked(stderr) is not 0
  return exit_failure_code;
}