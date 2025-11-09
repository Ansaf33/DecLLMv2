#define _GNU_SOURCE // For asprintf and other GNU extensions
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <locale.h>
#include <libintl.h>
#include <getopt.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <stdarg.h> // For va_list in error stub
#include <limits.h> // For UINT_MAX
#include <math.h>   // For HUGE_VAL

// --- Global variables (inferred from usage) ---
char *program_invocation_name;

// These are likely custom globals for the timeout utility
static char *command = NULL;
static bool foreground = false;
static pid_t monitored_pid = 0;
static int term_signal = SIGTERM; // Default signal is TERM
static double kill_after = 0.0;
static bool preserve_status = false;
static int verbose = 0;
static bool timed_out = false; // Flag to indicate if timeout occurred

// Placeholder for _Version and DAT_00101fed
static const char *_Version = "1.0";
static const char *DAT_00101fed = "2023-10-27"; // Placeholder for author date

// Forward declarations for signal handlers
static void cleanup(int sig);
static void chld(void);

// For getopt_long
static struct option long_options[] = {
    {"foreground", no_argument, NULL, 'f'},
    {"kill-after", required_argument, NULL, 'k'},
    {"preserve-status", no_argument, NULL, 'p'},
    {"signal", required_argument, NULL, 's'},
    {"verbose", no_argument, NULL, 'v'},
    {"help", no_argument, NULL, 258}, // Custom value for --help
    {"version", no_argument, NULL, 259}, // Custom value for --version
    {NULL, 0, NULL, 0}
};

// --- Stubs for GNU libc / coreutils specific functions ---
// These would typically be provided by linking against libgnu and coreutils libraries.
// For a self-contained compilable unit, minimal stubs are provided.

void error(int status, int errnum, const char *format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, "%s: ", program_invocation_name ? program_invocation_name : "timeout");
    vfprintf(stderr, format, args);
    if (errnum != 0) {
        fprintf(stderr, ": %s", strerror(errnum));
    }
    fprintf(stderr, "\n");
    va_end(args);
    if (status != 0) {
        exit(status);
    }
}

char *quote(const char *str) {
    char *buf;
    if (strchr(str, ' ') || strchr(str, '\t') || strchr(str, '\'') || strchr(str, '\"')) {
        if (asprintf(&buf, "'%s'", str) == -1) {
            return strdup(str); // Fallback if asprintf fails
        }
    } else {
        buf = strdup(str);
    }
    return buf;
}

void set_program_name(char *name) {
    program_invocation_name = name;
}

void initialize_exit_failure(int status) {
    // In a real program, this might set a global exit status.
}

void close_stdout(void) {
    if (fclose(stdout) != 0) {
        error(0, errno, "failed to close stdout");
    }
}

char *proper_name_lite(const char *name, const char *date) {
    char *buf;
    if (asprintf(&buf, "%s %s", name, date) == -1) {
        return strdup(name); // Fallback
    }
    return buf;
}

void version_etc(FILE *stream, const char *package, const char *version,
                 const char *authors, const char *date_info, ...) {
    fprintf(stream, "%s (%s) %s\n", package, "GNU coreutils", version);
    fprintf(stream, "Copyright (C) 2023 Free Software Foundation, Inc.\n");
    fprintf(stream, "License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\n");
    fprintf(stream, "This is free software: you are free to change and redistribute it.\n");
    fprintf(stream, "There is NO WARRANTY, to the extent permitted by law.\n\n");
    fprintf(stream, "Written by %s.\n", authors);
}

int operand2sig(const char *operand) {
    if (operand == NULL) return -1;
    char *endptr;
    long val = strtol(operand, &endptr, 10);
    if (*endptr == '\0' && val > 0 && val < NSIG) {
        return (int)val;
    }
    // For now, only accept numbers. A real implementation would parse signal names.
    return -1;
}

struct timespec dtotimespec(double d) {
    struct timespec ts;
    ts.tv_sec = (time_t)d;
    ts.tv_nsec = (long)((d - (double)ts.tv_sec) * 1e9);
    if (ts.tv_nsec < 0) ts.tv_nsec = 0;
    if (ts.tv_nsec >= 1e9) {
        ts.tv_sec++;
        ts.tv_nsec -= 1e9;
    }
    return ts;
}

double dtimespec_bound(double d, int errnum) {
    if (errnum == ERANGE) {
        if (d > 0 && d == HUGE_VAL) return (double)LONG_MAX;
        if (d < 0 && d == -HUGE_VAL) return (double)LONG_MIN; // Or some other bound
    }
    return d;
}

double cl_strtod(const char *nptr, char **endptr) {
    return strtod(nptr, endptr);
}

void emit_mandatory_arg_note(void) {
    fputs(gettext("\nMandatory arguments to long options are mandatory for short options too.\n"), stdout);
}

void emit_ancillary_info(const char *program) {
    fprintf(stdout, gettext("\nReport %s bugs to: <https://savannah.gnu.org/bugs/?group=coreutils>\n"), program);
    fputs(gettext("GNU coreutils home page: <https://www.gnu.org/software/coreutils/>\n"), stdout);
    fputs(gettext("General help using GNU software: <https://www.gnu.org/get_help/>\n"), stdout);
    fprintf(stdout, gettext("For complete documentation, run: info coreutils \'%s invocation\'\n"), program);
}

// --- End of Stubs ---

// Function: chld
static void chld(void) {
  // This function is likely a signal handler for SIGCHLD,
  // typically used to prevent child processes from becoming zombies.
  // An empty handler is common for this purpose.
}

// Function: cleanup (signal handler)
static void cleanup(int sig) {
    if (monitored_pid > 0) {
        if (verbose) {
            fprintf(stderr, gettext("timeout: sending signal %d (%s) to command '%s'\n"),
                    term_signal, strsignal(term_signal), command ? command : "(unknown)");
        }
        kill(monitored_pid, term_signal);
        timed_out = true;
    }
    // If the timeout utility itself receives a terminating signal (e.g., SIGINT, SIGTERM, SIGHUP),
    // it should clean up and exit. SIGALRM is handled by setting timed_out flag.
    if (sig != SIGALRM && sig != term_signal) {
        exit(124); // Exit with timeout status if the timeout utility itself is killed.
    }
}

// Function: usage
void usage(int status) {
  if (status == 0) {
    printf(gettext("Usage: %s [OPTION]... DURATION COMMAND [ARG]...\n"), program_invocation_name);
    fputs(gettext("Start COMMAND, and kill it if still running after DURATION.\n"), stdout);
    emit_mandatory_arg_note();
    fputs(gettext(
                            "  -f, --foreground\n                 when not running timeout directly from a shell prompt,\n                   allow COMMAND to read from the TTY and get TTY signals;\n                   in this mode, children of COMMAND will not be timed out\n"
                            ), stdout);
    fputs(gettext(
                            "  -k, --kill-after=DURATION\n                 also send a KILL signal if COMMAND is still running\n                   this long after the initial signal was sent\n"
                            ), stdout);
    fputs(gettext(
                            "  -p, --preserve-status\n                 exit with the same status as COMMAND,\n                   even when the command times out\n"
                            ), stdout);
    fputs(gettext(
                            "  -s, --signal=SIGNAL\n                 specify the signal to be sent on timeout;\n                   SIGNAL may be a name like \'HUP\' or a number;\n                   see \'kill -l\' for a list of signals\n"
                            ), stdout);
    fputs(gettext(
                            "  -v, --verbose  diagnose to standard error any signal sent upon timeout\n"
                            ), stdout);
    fputs(gettext("      --help        display this help and exit\n"), stdout);
    fputs(gettext("      --version     output version information and exit\n"), stdout);
    fputs(gettext(
                            "\nDURATION is a floating point number with an optional suffix:\n\'s\' for seconds (the default), \'m\' for minutes, \'h\' for hours or \'d\' for days.\nA duration of 0 disables the associated timeout.\n"
                            ), stdout);
    fputs(gettext(
                            "\nUpon timeout, send the TERM signal to COMMAND, if no other SIGNAL specified.\nThe TERM signal kills any process that does not block or catch that signal.\nIt may be necessary to use the KILL signal, since this signal can\'t be caught.\n"
                            ), stdout);
    fputs(gettext(
                            "\nExit status:\n  124  if COMMAND times out, and --preserve-status is not specified\n  125  if the timeout command itself fails\n  126  if COMMAND is found but cannot be invoked\n  127  if COMMAND cannot be found\n  137  if COMMAND (or timeout itself) is sent the KILL (9) signal (128+9)\n  -    the exit status of COMMAND otherwise\n"
                            ), stdout);
    emit_ancillary_info("timeout");
  } else {
    fprintf(stderr, gettext("Try \'%s --help\' for more information.\n"), program_invocation_name);
  }
  exit(status);
}

// Function: apply_time_suffix
bool apply_time_suffix(double *duration_ptr, char suffix) {
  int multiplier = 1; // Default for 's' or '\0'

  if (suffix == '\0' || suffix == 's') {
    multiplier = 1;
  } else if (suffix == 'm') {
    multiplier = 60;
  } else if (suffix == 'h') {
    multiplier = 3600; // 60 * 60
  } else if (suffix == 'd') {
    multiplier = 86400; // 24 * 3600
  } else {
    return false; // Unknown suffix
  }

  *duration_ptr = dtimespec_bound(*duration_ptr * (double)multiplier, 0);
  return true;
}

// Function: parse_duration
double parse_duration(char *duration_str) {
  char *endptr;
  errno = 0; // Clear errno before strtod
  double duration = cl_strtod(duration_str, &endptr);
  int strtod_errno = errno; // Save errno from strtod

  duration = dtimespec_bound(duration, strtod_errno);

  // Check if parsing was successful and if suffix is valid
  if (duration_str != endptr && duration >= 0.0 && (*endptr == '\0' || endptr[1] == '\0')) {
    if (apply_time_suffix(&duration, *endptr)) {
      return duration;
    }
  }

  // If we reach here, parsing failed or suffix was invalid
  error(0, 0, gettext("invalid time interval %s"), quote(duration_str));
  usage(125); // 0x7d
  return 0.0; // Should not be reached due to exit in usage
}

// Function: unblock_signal
void unblock_signal(int sig) {
  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, sig);
  if (sigprocmask(SIG_UNBLOCK, &set, NULL) != 0) {
    error(0, errno, gettext("warning: sigprocmask"));
  }
}

// Function: install_sigchld
void install_sigchld(void) {
  struct sigaction sa;
  sigemptyset(&sa.sa_mask);
  sa.sa_handler = chld;
  sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
  sigaction(SIGCHLD, &sa, NULL);
  unblock_signal(SIGCHLD);
}

// Function: install_cleanup
void install_cleanup(int cleanup_sig) {
  struct sigaction sa;
  sigemptyset(&sa.sa_mask);
  sa.sa_handler = cleanup;
  sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
  sigaction(SIGALRM, &sa, NULL);
  sigaction(SIGHUP, &sa, NULL);
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGQUIT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);
  sigaction(cleanup_sig, &sa, NULL);
}

// Function: block_cleanup_and_chld
void block_cleanup_and_chld(int cleanup_sig, sigset_t *oldset) {
  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, SIGALRM);
  sigaddset(&set, SIGHUP);
  sigaddset(&set, SIGINT);
  sigaddset(&set, SIGQUIT);
  sigaddset(&set, SIGTERM);
  sigaddset(&set, cleanup_sig);
  sigaddset(&set, SIGCHLD);
  if (sigprocmask(SIG_BLOCK, &set, oldset) != 0) {
    error(0, errno, gettext("warning: sigprocmask"));
  }
}

// Function: disable_core_dumps
bool disable_core_dumps(void) {
  if (prctl(PR_SET_DUMPABLE, 0) != 0) {
    error(0, errno, gettext("warning: disabling core dumps failed"));
    return false;
  }
  return true;
}

// Function: settimeout
void settimeout(double duration_param, bool verbose_param) {
  timer_t timerid;
  struct itimerspec its;

  its.it_value = dtotimespec(duration_param);
  its.it_interval.tv_sec = 0;
  its.it_interval.tv_nsec = 0;

  int timer_create_ret = timer_create(CLOCK_REALTIME, NULL, &timerid);
  if (timer_create_ret == 0) {
    if (timer_settime(timerid, 0, &its, NULL) != 0) {
      if (verbose_param) {
        error(0, errno, gettext("warning: timer_settime"));
      }
      timer_delete(timerid); // Clean up the timer if settime failed
    }
  } else { // timer_create failed
    // Original code had a specific check for errno != 0x26 (EINVAL)
    // If EINVAL, it might be expected (e.g., system doesn't support timers well for 0 duration).
    if (verbose_param && errno != EINVAL) {
      error(0, errno, gettext("warning: timer_create"));
    }
  }

  // Simplified alarm logic based on common usage and inferred intent
  unsigned int alarm_duration;
  if (duration_param <= 0.0) { // If duration is non-positive, disable alarm
      alarm_duration = 0;
  } else if (duration_param >= (double)UINT_MAX) { // If duration exceeds unsigned int max for alarm()
      alarm_duration = UINT_MAX; // Max alarm duration
  } else {
      alarm_duration = (unsigned int)duration_param; // Truncate to seconds
  }
  alarm(alarm_duration);
}

// Function: send_sig
void send_sig(int pid, int sig) {
  if (pid == 0) {
    signal(sig, SIG_IGN); // SIG_IGN is 0x1
  }
  kill(pid, sig);
}

// Function: main
int main(int argc, char **argv) {
  set_program_name(argv[0]);
  setlocale(LC_ALL, "");
  bindtextdomain("coreutils", "/usr/local/share/locale");
  textdomain("coreutils");
  initialize_exit_failure(125); // 0x7d
  atexit(close_stdout);

  int c;
  while ((c = getopt_long(argc, argv, "+fk:ps:v", long_options, NULL)) != -1) {
    switch (c) {
      case 'f':
        foreground = true;
        break;
      case 'k':
        kill_after = parse_duration(optarg);
        break;
      case 'p':
        preserve_status = true;
        break;
      case 's':
        term_signal = operand2sig(optarg);
        if (term_signal == -1) {
          usage(125); // 0x7d
        }
        break;
      case 'v':
        verbose = 1;
        break;
      case 258: // --help
        usage(0);
        break; // usage calls exit, so this break is technically not reached
      case 259: // --version
        version_etc(stdout, "timeout", _Version,
                    proper_name_lite("Padraig Brady", DAT_00101fed), NULL);
        exit(0);
      default: // Unknown option or error
        usage(125); // 0x7d
    }
  }

  if (argc - optind < 2) {
    usage(125); // 0x7d
  }

  double timeout_duration = parse_duration(argv[optind]);
  command = argv[optind + 1];
  char **command_args = &argv[optind + 1];

  if (!foreground) {
    setpgid(0, 0); // Create a new process group for the child
  }

  install_cleanup(term_signal);
  signal(SIGPIPE, SIG_IGN); // 0x15
  signal(SIGXCPU, SIG_IGN); // 0x16

  install_sigchld();
  unblock_signal(SIGALRM); // Unblock SIGALRM so cleanup can handle it

  sigset_t blocked_signals_mask;
  block_cleanup_and_chld(term_signal, &blocked_signals_mask);

  monitored_pid = fork();
  if (monitored_pid == -1) {
    error(0, errno, gettext("fork system call failed"));
    return 125; // 0x7d
  } else if (monitored_pid == 0) { // Child process
    // Unblock signals in child
    if (sigprocmask(SIG_SETMASK, &blocked_signals_mask, NULL) != 0) {
      error(0, errno, gettext("child failed to reset signal mask"));
      _exit(125); // Use _exit in child to avoid flushing parent's stdio buffers
    }

    signal(SIGPIPE, SIG_DFL); // Restore default handlers
    signal(SIGXCPU, SIG_DFL);

    execvp(command_args[0], command_args);

    // If execvp returns, it failed
    int exec_errno = errno;
    int exit_status;
    if (exec_errno == ENOENT) { // 2
      exit_status = 127; // 0x7f
    } else {
      exit_status = 126; // 0x7e
    }
    error(0, exec_errno, gettext("failed to run command %s"), quote(command_args[0]));
    _exit(exit_status); // Use _exit in child
  } else { // Parent process
    settimeout(timeout_duration, verbose);

    int status;
    pid_t wpid;
    do {
      wpid = waitpid(monitored_pid, &status, WNOHANG);
      if (wpid == 0) { // Child is still running
        sigsuspend(&blocked_signals_mask); // Wait for a signal (e.g., SIGCHLD or SIGALRM)
      }
    } while (wpid == 0); // Loop until child exits or waitpid fails

    if (wpid < 0) {
      error(0, errno, gettext("error waiting for command"));
      status = 125; // 0x7d
    } else {
      if (WIFEXITED(status)) {
        status = WEXITSTATUS(status);
      } else if (WIFSIGNALED(status)) {
        int term_sig_val = WTERMSIG(status);
        if (WCOREDUMP(status)) {
          error(0, 0, gettext("the monitored command dumped core"));
        }

        // If not timed out, and core dumps can be disabled, parent re-raises the signal
        if (!timed_out && disable_core_dumps()) {
          signal(term_sig_val, SIG_DFL);
          unblock_signal(term_sig_val);
          raise(term_sig_val);
        }

        if (timed_out && term_sig_val == SIGKILL) {
          preserve_status = true;
        }
        status = 128 + term_sig_val; // Exit status for signal termination
      } else { // WIFSTOPPED, WIFCONTINUED, or unknown status
        error(0, 0, gettext("unknown status from command (%d)"), status);
        status = 1; // General error
      }
    }

    // Final exit status adjustment
    if (timed_out && !preserve_status) {
      return 124; // 0x7c
    } else {
      return status;
    }
  }
}