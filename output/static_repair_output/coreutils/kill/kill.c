#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libintl.h>
#include <locale.h>
#include <getopt.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h> // For INT_MAX, INT_MIN
#include <stdbool.h> // For bool type
#include <stdarg.h> // For va_list in error function

// --- Dummy/Mock declarations for coreutils functions and globals ---
// In a real coreutils build, these would come from various headers.
// For compilation, we provide minimal definitions.

char *program_name; // Replaces _program_name, typically set from argv[0]

// Placeholders for global string literals
static const char DAT_00101321[] = "kill";
static const char DAT_00101356[] = "%s: invalid signal %d";
static const char DAT_00101359[] = "%s: cannot send signal to process";
static const char *Version = "1.0"; // Placeholder for _Version

// Dummy function implementations
void emit_mandatory_arg_note() { /* dummy */ }
void emit_ancillary_info(const char *program) { /* dummy */ }
void set_program_name(const char *name) { program_name = (char *)name; }
void close_stdout() { /* dummy */ }
const char *proper_name_lite(const char *name, const char *alt_name) { return name; }
void version_etc(FILE *stream, const char *program, const char *package, const char *version, const char *authors, ...) {
    fprintf(stream, "%s %s\n", package, version);
    fprintf(stream, "Copyright (C) 2023 Free Software Foundation, Inc.\n");
    fprintf(stream, "License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\n");
    fprintf(stream, "This is free software: you are free to change and redistribute it.\n");
    fprintf(stream, "There is NO WARRANTY, to the extent permitted by law.\n\n");
    fprintf(stream, "Written by %s.\n", authors);
}
const char *quote(const char *str) { return str; } // Simple mock, real coreutils quotes special chars
void error(int status, int errnum, const char *format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, "%s: ", program_name);
    vfprintf(stderr, format, args);
    if (errnum) {
        fprintf(stderr, ": %s", strerror(errnum));
    }
    fprintf(stderr, "\n");
    va_end(args);
    if (status) exit(status);
}
int c_isdigit(int c) { return isdigit(c); } // Wrapper for isdigit

// Minimal mock for sig2str (converts signal number to name)
int sig2str(int signum, char *str) {
    const char *s = strsignal(signum);
    if (s && strcmp(s, "Unknown signal") != 0) {
        strncpy(str, s, 23); // Max 23 chars for buffer
        str[23] = '\0';
        return 0;
    }
    return -1; // Indicate failure
}

// Minimal mock for operand2sig (converts signal name/number string to signal number)
int operand2sig(const char *operand) {
    if (!operand || *operand == '\0') {
        return -1; // Invalid operand
    }

    // Try to parse as integer
    char *endptr;
    long val = strtol(operand, &endptr, 10);
    if (*endptr == '\0' && endptr != operand) {
        if (val >= 0 && val <= SIGRTMAX) { // Assuming signals are non-negative and within OS limits
            return (int)val;
        }
        return -1; // Out of range
    }

    // Try to parse as signal name (case-insensitive, with optional "SIG" prefix)
    const char *name_to_check = operand;
    if (strncasecmp(operand, "SIG", 3) == 0) {
        name_to_check = operand + 3;
    }

    // This mock covers common signals; real coreutils has a comprehensive list.
    if (strcasecmp(name_to_check, "HUP") == 0) return SIGHUP;
    if (strcasecmp(name_to_check, "INT") == 0) return SIGINT;
    if (strcasecmp(name_to_check, "QUIT") == 0) return SIGQUIT;
    if (strcasecmp(name_to_check, "ILL") == 0) return SIGILL;
    if (strcasecmp(name_to_check, "TRAP") == 0) return SIGTRAP;
    if (strcasecmp(name_to_check, "ABRT") == 0) return SIGABRT;
    if (strcasecmp(name_to_check, "BUS") == 0) return SIGBUS;
    if (strcasecmp(name_to_check, "FPE") == 0) return SIGFPE;
    if (strcasecmp(name_to_check, "KILL") == 0) return SIGKILL;
    if (strcasecmp(name_to_check, "USR1") == 0) return SIGUSR1;
    if (strcasecmp(name_to_check, "SEGV") == 0) return SIGSEGV;
    if (strcasecmp(name_to_check, "USR2") == 0) return SIGUSR2;
    if (strcasecmp(name_to_check, "PIPE") == 0) return SIGPIPE;
    if (strcasecmp(name_to_check, "ALRM") == 0) return SIGALRM;
    if (strcasecmp(name_to_check, "TERM") == 0) return SIGTERM;
    if (strcasecmp(name_to_check, "STKFLT") == 0) return SIGSTKFLT;
    if (strcasecmp(name_to_check, "CHLD") == 0) return SIGCHLD;
    if (strcasecmp(name_to_check, "CONT") == 0) return SIGCONT;
    if (strcasecmp(name_to_check, "STOP") == 0) return SIGSTOP;
    if (strcasecmp(name_to_check, "TSTP") == 0) return SIGTSTP;
    if (strcasecmp(name_to_check, "TTIN") == 0) return SIGTTIN;
    if (strcasecmp(name_to_check, "TTOU") == 0) return SIGTTOU;
    if (strcasecmp(name_to_check, "URG") == 0) return SIGURG;
    if (strcasecmp(name_to_check, "XCPU") == 0) return SIGXCPU;
    if (strcasecmp(name_to_check, "XFSZ") == 0) return SIGXFSZ;
    if (strcasecmp(name_to_check, "VTALRM") == 0) return SIGVTALRM;
    if (strcasecmp(name_to_check, "PROF") == 0) return SIGPROF;
    if (strcasecmp(name_to_check, "WINCH") == 0) return SIGWINCH;
    if (strcasecmp(name_to_check, "IO") == 0) return SIGIO;
    if (strcasecmp(name_to_check, "PWR") == 0) return SIGPWR;
    if (strcasecmp(name_to_check, "SYS") == 0) return SIGSYS;
    
    return -1; // Not found
}

// --- End of Dummy/Mock declarations ---


// Function: usage
void usage(int exit_status) {
  if (exit_status == 0) {
    printf(gettext("Usage: %s [-s SIGNAL | -SIGNAL] PID...\n  or:  %s -l [SIGNAL]...\n  or:  %s -t [SIGNAL]...\n"),
           program_name, program_name, program_name);
    fputs_unlocked(gettext("Send signals to processes, or list signals.\n"), stdout);
    emit_mandatory_arg_note();
    fputs_unlocked(gettext("  -s, --signal=SIGNAL, -SIGNAL\n                   specify the name or number of the signal to be sent\n  -l, --list       list signal names, or convert signal names to/from numbers\n  -t, --table      print a table of signal information\n"),
                   stdout);
    fputs_unlocked(gettext("      --help        display this help and exit\n"), stdout);
    fputs_unlocked(gettext("      --version     output version information and exit\n"), stdout);
    fputs_unlocked(gettext("\nSIGNAL may be a signal name like \'HUP\', or a signal number like \'1\',\nor the exit status of a process terminated by a signal.\nPID is an integer; if negative it identifies a process group.\n"),
                   stdout);
    printf(gettext("\nYour shell may have its own version of %s, which usually supersedes\nthe version described here.  Please refer to your shell\'s documentation\nfor details about the options it supports.\n"),
           DAT_00101321);
    emit_ancillary_info(DAT_00101321);
  } else {
    fprintf(stderr, gettext("Try \'%s --help\' for more information.\n"), program_name);
  }
  exit(exit_status);
}

// Function: print_table_row
void print_table_row(unsigned int num_width, unsigned int signal_number, unsigned int name_width, const char* signal_name) {
  printf("%*d %-*s %s\n", (int)num_width, (int)signal_number, (int)name_width, signal_name, strsignal(signal_number) ? : "?");
}

// Function: list_signals
int list_signals(char is_table_mode, char **signal_args) {
  int ret_status = 0;
  
  if (is_table_mode == 0) { // List signal names/numbers
    if (signal_args == NULL) { // List all signals
      for (int i = 0; i <= SIGRTMAX; ++i) { 
        char signal_name_buf[24];
        if (sig2str(i, signal_name_buf) == 0) {
          puts(signal_name_buf);
        }
      }
    } else { // Convert specified signals
      for (; *signal_args != NULL; ++signal_args) {
        unsigned int sig_num = operand2sig(*signal_args);
        if ((int)sig_num < 0) {
          ret_status = 1;
        } else {
          // If operand starts with non-digit (likely a name), print number
          // If operand starts with digit (likely a number), try to print name, else print number
          if (c_isdigit((int)**signal_args) == 0) { 
            printf("%d\n", (int)sig_num);
          } else {
            char signal_name_buf[24];
            if (sig2str(sig_num, signal_name_buf) == 0) {
              puts(signal_name_buf);
            } else {
              printf("%d\n", (int)sig_num);
            }
          }
        }
      }
    }
  } else { // Print signal table
    int max_name_len = 0;
    int num_width = 1; 
    
    // Determine max signal number width
    for (int i = 1; i <= SIGRTMAX; i *= 10) { 
      num_width++;
    }

    // Determine max signal name length
    for (int i = 0; i <= SIGRTMAX; ++i) {
      char signal_name_buf[24];
      if (sig2str(i, signal_name_buf) == 0) {
        size_t len = strlen(signal_name_buf);
        if (max_name_len < (int)len) {
          max_name_len = (int)len;
        }
      }
    }
    
    if (signal_args == NULL) { // Print table for all signals
      for (int i = 0; i <= SIGRTMAX; ++i) {
        char signal_name_buf[24];
        if (sig2str(i, signal_name_buf) == 0) {
          print_table_row(num_width, i, max_name_len, signal_name_buf);
        }
      }
    } else { // Print table for specified signals
      for (; *signal_args != NULL; ++signal_args) {
        unsigned int sig_num = operand2sig(*signal_args);
        if ((int)sig_num < 0) {
          ret_status = 1;
        } else {
          char signal_name_buf[24];
          if (sig2str(sig_num, signal_name_buf) != 0) {
            snprintf(signal_name_buf, sizeof(signal_name_buf), "SIG%d", (int)sig_num);
          }
          print_table_row(num_width, sig_num, max_name_len, signal_name_buf);
        }
      }
    }
  }
  return ret_status;
}

// Function: send_signals
int send_signals(int signal_num, char **pid_strings) {
  int ret_status = 0;
  
  for (; *pid_strings != NULL; ++pid_strings) {
    errno = 0;
    char *endptr;
    intmax_t pid_val = strtoimax(*pid_strings, &endptr, 10);

    // Check for conversion errors and range
    if (errno == ERANGE || pid_val > INT_MAX || pid_val < INT_MIN || *pid_strings == endptr || *endptr != '\0') {
      error(0, 0, gettext("%s: invalid process id"), quote(*pid_strings));
      ret_status = 1;
    } else {
      if (kill((pid_t)pid_val, signal_num) != 0) {
        if (errno == EINVAL) { // Invalid signal number
          error(0, errno, DAT_00101356, signal_num);
        } else { // Other errors
          error(0, errno, DAT_00101359, quote(*pid_strings));
        }
        ret_status = 1;
      }
    }
  }
  return ret_status;
}

// long options for getopt_long
static const struct option long_options[] = {
    {"signal", required_argument, NULL, 's'},
    {"list", no_argument, NULL, 'l'},
    {"table", no_argument, NULL, 't'},
    {"help", no_argument, NULL, 258}, // Custom value for --help
    {"version", no_argument, NULL, 259}, // Custom value for --version
    {NULL, 0, NULL, 0}
};

// Function: main
int main(int argc, char **argv) {
  bool list_or_table_mode = false;
  char is_table_mode = 0; // 0 for list, 1 for table (includes -L or -t)
  int signal_number = -1; // -1 means no signal specified, default to SIGTERM
  
  set_program_name(argv[0]);
  setlocale(LC_ALL, ""); 
  bindtextdomain("coreutils","/usr/local/share/locale");
  textdomain("coreutils");
  atexit(close_stdout);

  int opt;
  char *signal_name_to_parse = NULL;

  while (true) {
    // The option string `0::1::...Z::Lln:s:t` indicates:
    // - '0' through '9' and 'A' through 'Z' can take an optional argument (e.g., -HUP where 'H' is opt, 'UP' is arg).
    // - 'L', 'l', 't' are no-argument options.
    // - 'n', 's' take a required argument.
    opt = getopt_long(argc, argv, "0::1::2::3::4::5::6::7::8::9::A::B::C::D::E::F::G::H::I::J::K::M::N::O::P::Q::R::S::T::U::V::W::X::Y::Z::Lln:s:t", long_options, NULL);
    
    if (opt == -1) { // End of options
      break;
    }

    // Handle custom long options
    if (opt == 259) { // --version
      version_etc(stdout, DAT_00101321, "GNU coreutils", Version, proper_name_lite("Paul Eggert", "Paul Eggert"), NULL);
      exit(0);
    }
    if (opt == 258) { // --help
      usage(0); // usage exits
    }

    switch(opt) {
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
    case 'H': case 'I': case 'J': case 'K': case 'M': case 'N': case 'O':
    case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V':
    case 'W': case 'X': case 'Y': case 'Z':
      // This block handles -SIGNAL (e.g., -HUP, -9) where SIGNAL starts with a digit or uppercase letter.
      // If this is not the first argument being processed (i.e., argv[1]),
      // then it's likely a PID that was misparsed as an option. Revert optind and stop option parsing.
      if (optind != 2) {
        optind--; // Revert optind to point to the argument that caused the parse.
        goto end_option_parsing; // Break out of the getopt_long loop
      }
      
      char* current_arg_str = argv[optind-1];
      // Check for valid format: -X (length 2) or -XARG (concatenated, where optarg points to ARG)
      if ((!optarg && strlen(current_arg_str) != 2) || (optarg && current_arg_str + 2 != optarg)) {
          error(0, 0, gettext("invalid option -- %c"), opt);
          usage(1);
      }
      signal_name_to_parse = current_arg_str + 1; // E.g., "-HUP" -> "HUP", "-9" -> "9"
      break;

    case 'l': // --list
      if (list_or_table_mode) {
        error(0, 0, gettext("multiple -l or -t options specified"));
        usage(1);
      }
      list_or_table_mode = true;
      is_table_mode = 0; // 'l' is just list, not table
      break;

    case 'L': // --list (from long_options), behaves like 't' in original snippet
    case 't': // --table
      if (list_or_table_mode) {
        error(0, 0, gettext("multiple -l or -t options specified"));
        usage(1);
      }
      list_or_table_mode = true;
      is_table_mode = 1; // 't' and 'L' are for table
      break;

    case 's': // --signal=SIGNAL or -s SIGNAL
    case 'n': // -n SIGNAL
      signal_name_to_parse = optarg;
      break;

    default: // Unknown option or other getopt_long error
      usage(1); // usage exits
    }

    // Process the parsed signal name if any
    if (signal_name_to_parse) {
      if (signal_number >= 0) { // Signal already specified
        error(0, 0, gettext("%s: multiple signals specified"), quote(signal_name_to_parse));
        usage(1);
      }
      signal_number = operand2sig(signal_name_to_parse);
      if (signal_number < 0) {
        usage(1); // operand2sig failed, usage exits
      }
      signal_name_to_parse = NULL; // Reset for next iteration
    }
  }

end_option_parsing:; // Label for breaking out of the getopt_long loop early.

  // Post-option processing
  if (signal_number < 0) {
    signal_number = SIGTERM; // Default signal if none specified
  } else if (list_or_table_mode) {
    error(0, 0, gettext("cannot combine signal with -l or -t"));
    usage(1);
  }

  if (!list_or_table_mode && (argc <= optind)) {
    error(0, 0, gettext("no process ID specified"));
    usage(1);
  }

  if (list_or_table_mode) {
    char **current_args = NULL;
    if (optind < argc) {
      current_args = argv + optind;
    }
    list_signals(is_table_mode, current_args);
  } else {
    send_signals(signal_number, argv + optind);
  }

  return 0; // Standard main return value
}