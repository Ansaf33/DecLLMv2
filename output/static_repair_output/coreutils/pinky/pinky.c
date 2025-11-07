#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <pwd.h>
#include <sys/stat.h>
#include <unistd.h>
#include <locale.h>
#include <libintl.h>
#include <getopt.h>
#include <errno.h>
#include <stdarg.h> // For va_list in error stub
#include <fcntl.h>  // For open flags O_RDONLY, O_CLOEXEC, O_DIRECTORY
#include <limits.h> // For LONG_MAX, LONG_MIN

// Global variables (from original snippet)
static time_t now_3 = 0;
static char buf_1[32]; // For time_string
static char buf_2[32]; // For idle_string
static int dev_dirfd_0 = 0; // File descriptor for /dev, initialized to 0 (invalid)
static int include_fullname = 1; // Flags, default to 1 (true)
static int include_idle = 1;
static int include_where = 1;
static int include_home_and_shell = 1;
static int include_project = 1;
static int include_plan = 1;
static int include_heading = 1;
static int do_lookup = 0; // Flag for --lookup
static int do_short_format = 1; // Flag for -s (default) or -l

static const char *time_format = NULL;
static int time_format_width = 0;

// GNU coreutils specific functions (stubs or replacements)
// These are minimal stubs to allow compilation. Real implementations are in coreutils.
static void xalloc_die(void) {
    fprintf(stderr, "Fatal: Memory allocation failed.\n");
    exit(1);
}

static void *xmalloc(size_t size) {
    void *ptr = malloc(size);
    if (!ptr && size > 0) {
        xalloc_die();
    }
    return ptr;
}

#define to_uchar(c) ((unsigned char)(c))

// Replacement for rpl_time
static int rpl_time(time_t *t) {
    *t = time(NULL);
    return (*t == (time_t)-1) ? -1 : 0;
}

#define rpl_fopen fopen
#define rpl_fclose fclose

static char *canon_host(char *host) {
    return strdup(host); // Simplistic stub; real implementation would do DNS lookup
}

static int streq(const char *s1, const char *s2) {
    return strcmp(s1, s2) == 0;
}

static char hard_locale(int category) {
    const char *lc_val = setlocale(category, NULL);
    if (lc_val && (strcmp(lc_val, "C") == 0 || strcmp(lc_val, "POSIX") == 0)) {
        return 0; // C or POSIX locale
    }
    return 1; // Non-C/POSIX locale
}

// Simplified struct for utmp entries based on usage inferred from the snippet
typedef struct {
    char *fields[9]; // fields[0]=user, fields[2]=line, fields[3]=host
    short type;      // Corresponds to *(short *)(local_28 + 7)
    time_t login_time; // For time_string
} UtmpRecord;

static UtmpRecord *utmp_records = NULL;
static long utmp_record_count = 0;

// Placeholder for `read_utmp`. A real implementation would parse /var/run/utmp.
static int read_utmp(const char *filename, long *count_out, UtmpRecord **records_out, int type_filter) {
    // Free previous records if any
    if (utmp_records) {
        for (long i = 0; i < utmp_record_count; ++i) {
            for (int j = 0; j < 9; ++j) {
                free(utmp_records[i].fields[j]);
            }
        }
        free(utmp_records);
        utmp_records = NULL;
    }

    utmp_record_count = 1; // Create one dummy entry for demonstration
    utmp_records = xmalloc(sizeof(UtmpRecord) * utmp_record_count);

    utmp_records[0].fields[0] = strdup("testuser"); // user
    utmp_records[0].fields[1] = strdup("tty1");     // id or other internal field
    utmp_records[0].fields[2] = strdup("tty1");     // line (used by print_entry)
    utmp_records[0].fields[3] = strdup("localhost"); // host (used by print_entry)
    for(int i = 4; i < 9; ++i) utmp_records[0].fields[i] = strdup(""); // fill remaining fields
    utmp_records[0].type = 7; // Assuming 7 is the type for active user processes (USER_PROCESS)
    utmp_records[0].login_time = time(NULL) - 3600; // Logged in 1 hour ago

    *count_out = utmp_record_count;
    *records_out = utmp_records;
    return 0; // 0 on success
}

static char *quotearg_n_style_colon(int n, int style, const char *arg) {
    return (char *)arg; // Simplistic stub
}

static const char *_program_name = NULL;
static void set_program_name(const char *name) {
    _program_name = name;
}

static void error(int status, int errnum, const char *format, ...) {
    va_list args;
    fprintf(stderr, "%s: ", _program_name ? _program_name : "pinky");
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    if (errnum) {
        fprintf(stderr, ": %s", strerror(errnum));
    }
    fprintf(stderr, "\n");
    if (status) exit(status);
}

static void emit_ancillary_info(const char *program_name) {
    printf("For bug reporting instructions, see:\n<%s>\n", "https://www.gnu.org/software/coreutils/bugs/");
}

static void close_stdout(void) {
    if (fflush(stdout) != 0) {
        exit(1);
    }
}

static const char *_Version = "1.0";

static const char *proper_name_lite(const char *name, const char *email) {
    return name; // Simplistic stub
}

static void version_etc(FILE *stream, const char *prog_name, const char *package,
                        const char *version, ...) {
    fprintf(stream, "%s (%s) %s\n", prog_name, package, version);
    fprintf(stream, "Copyright (C) 2023 Free Software Foundation, Inc.\n");
    fprintf(stream, "License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\n");
    fprintf(stream, "This is free software: you are free to change and redistribute it.\n");
    fprintf(stream, "There is NO WARRANTY, to the extent permitted by law.\n\n");
}

// getopt_long options
static struct option longopts[] = {
    {"lookup", no_argument, NULL, 0x80}, // Custom value for --lookup
    {"help", no_argument, NULL, 0x81},   // Custom value for --help
    {"version", no_argument, NULL, 0x82}, // Custom value for --version
    {NULL, 0, NULL, 0}
};

extern int optind; // From <unistd.h> or <getopt.h>

// Forward declarations
long count_ampersands(char *param_1);
char *create_fullname(char *param_1, char *param_2);
char *idle_string(long param_1);
char *time_string(time_t timestamp);
void print_entry(UtmpRecord *entry_record);
void print_long_entry(char *param_1);
void print_heading(void);
void scan_entries(long count, UtmpRecord *records, int num_users, char **users_list);
void short_pinky(const char *utmp_file, int num_users, char **users_list);
void long_pinky(int num_users, char **users_list);
void usage(int param_1);

// Function: count_ampersands
long count_ampersands(char *param_1) {
  long count = 0;
  for (char *ptr = param_1; *ptr != '\0'; ++ptr) {
    count += (*ptr == '&');
  }
  return count;
}

// Function: create_fullname
char *create_fullname(char *param_1, char *param_2) {
    size_t len_p1 = strlen(param_1);
    long final_len = len_p1 + 1; // +1 for null terminator

    long ampersand_count = count_ampersands(param_1);
    if (ampersand_count != 0) {
        size_t len_p2 = strlen(param_2);
        long replacement_len_change = ampersand_count * (len_p2 - 1);
        // Simple overflow check, more robust checks might be needed for production
        if ((replacement_len_change > 0 && final_len > LONG_MAX - replacement_len_change) ||
            (replacement_len_change < 0 && final_len < LONG_MIN - replacement_len_change)) {
             xalloc_die(); // Treat as allocation failure due to potential overflow
        }
        final_len += replacement_len_change;
    }

    char *result = (char *)xmalloc(final_len);
    char *current_pos = result;

    for (char *src_ptr = param_1; *src_ptr != '\0'; ++src_ptr) {
        if (*src_ptr == '&') {
            char *replacement_ptr = param_2;
            if (isupper(to_uchar(*param_2))) {
                *current_pos++ = (char)toupper(to_uchar(*param_2));
                replacement_ptr++;
            }
            while (*replacement_ptr != '\0') {
                *current_pos++ = *replacement_ptr++;
            }
        } else {
            *current_pos++ = *src_ptr;
        }
    }
    *current_pos = '\0';

    return result;
}

// Function: idle_string
char *idle_string(long param_1) {
  if (now_3 == 0) {
    rpl_time(&now_3);
  }
  long elapsed_seconds = now_3 - param_1;
  if (elapsed_seconds < 60) { // Less than 60 seconds
    return "     ";
  } else {
    if (elapsed_seconds < 86400) { // Less than 86400 seconds (1 day)
      sprintf(buf_2, "%02ld:%02ld", elapsed_seconds / 3600, (elapsed_seconds % 3600) / 60);
    } else {
      sprintf(buf_2, "%ldd", elapsed_seconds / 86400);
    }
    return buf_2;
  }
}

// Function: time_string
char *time_string(time_t timestamp) {
  struct tm *tp = localtime(&timestamp);
  if (tp == NULL) {
    char *ctime_str = ctime(&timestamp);
    if (ctime_str && strlen(ctime_str) > 0) {
        strncpy(buf_1, ctime_str, sizeof(buf_1) - 1);
        buf_1[sizeof(buf_1) - 1] = '\0';
        char *newline = strchr(buf_1, '\n');
        if (newline) *newline = '\0';
    } else {
        strcpy(buf_1, "Invalid Time");
    }
  } else {
    strftime(buf_1, sizeof(buf_1), time_format, tp);
  }
  return buf_1;
}

// Function: print_entry
void print_entry(UtmpRecord *entry_record) {
  char *user_name = entry_record->fields[0];
  char *line_name = entry_record->fields[2];
  char *host_name = entry_record->fields[3];
  time_t login_time = entry_record->login_time;

  char *line_display_ptr = line_name;
  if (strchr(line_display_ptr, ' ') != NULL) {
    line_display_ptr = strchr(line_display_ptr, ' ') + 1;
  }

  int fd_dev_or_special_val;
  if (*line_display_ptr == '/') {
    fd_dev_or_special_val = -100; // Special value, not a file descriptor
  } else {
    if (dev_dirfd_0 == 0) {
      dev_dirfd_0 = open("/dev", O_RDONLY | O_CLOEXEC | O_DIRECTORY);
      if (dev_dirfd_0 < 0) {
        dev_dirfd_0 = -101; // Indicate error in opening /dev
      }
    }
    fd_dev_or_special_val = dev_dirfd_0;
  }

  char file_status_char;
  long access_time_sec = 0;
  struct stat file_stat;

  if (fd_dev_or_special_val < -100 || fstatat(fd_dev_or_special_val, line_display_ptr, &file_stat, 0) != 0) {
    file_status_char = '?';
  } else {
    if ((file_stat.st_mode & S_IFMT) == S_IFBLK || (file_stat.st_mode & S_IFMT) == S_IFCHR) {
      file_status_char = '*';
    } else {
      file_status_char = ' ';
    }
    access_time_sec = file_stat.st_atim.tv_sec;
  }

  size_t len_user = strnlen(user_name, 8);
  if (len_user < 8) {
    printf("%-8s", user_name);
  } else {
    fputs_unlocked(user_name, stdout);
  }

  if (include_fullname) {
    struct passwd *pw_entry = getpwnam(user_name);
    if (pw_entry == NULL) {
      printf(" %19s", gettext("        ???"));
    } else {
      char *gecos_comma = strchr(pw_entry->pw_gecos, ',');
      if (gecos_comma != NULL) {
        *gecos_comma = '\0';
      }
      char *full_name_ptr = create_fullname(pw_entry->pw_gecos, pw_entry->pw_name);
      printf(" %-19.19s", full_name_ptr);
      free(full_name_ptr);
    }
  }

  fputc_unlocked(' ', stdout);
  fputc_unlocked(file_status_char, stdout);

  size_t len_line = strnlen(line_name, 8);
  if (len_line < 8) {
    printf("%-8s", line_name);
  } else {
    fputs_unlocked(line_name, stdout);
  }

  if (include_idle) {
    if (access_time_sec == 0) {
      printf(" %-6s", gettext("?????"));
    } else {
      printf(" %-6s", idle_string(access_time_sec));
    }
  }

  printf(" %s", time_string(login_time));

  if (include_where && (*host_name != '\0')) {
    char *display_host = NULL;
    char *colon_pos = strchr(host_name, ':');
    char *port_or_display = NULL;

    if (colon_pos != NULL) {
      *colon_pos = '\0';
      port_or_display = colon_pos + 1;
    }

    if ((*host_name != '\0') && do_lookup) {
      display_host = canon_host(host_name);
    }

    if (display_host == NULL) {
      display_host = host_name;
    }

    fputc_unlocked(' ', stdout);
    fputs_unlocked(display_host, stdout);

    if (port_or_display != NULL) {
      fputc_unlocked(':', stdout);
      fputs_unlocked(port_or_display, stdout);
    }

    if (display_host != host_name) {
      free(display_host);
    }
    if (colon_pos != NULL) {
        *colon_pos = ':'; // Restore original string if it was modified
    }
  }
  putchar_unlocked('\n');
}

// Function: print_long_entry
void print_long_entry(char *param_1_username) {
  struct passwd *pw_entry = getpwnam(param_1_username);

  printf("%s", gettext("Login name: "));
  printf("%-28s", param_1_username);
  printf("%s", gettext("In real life: "));

  if (pw_entry == NULL) {
    printf(" %s", gettext("???"));
  } else {
    char *gecos_comma = strchr(pw_entry->pw_gecos, ',');
    if (gecos_comma != NULL) {
      *gecos_comma = '\0';
    }
    char *full_name_ptr = create_fullname(pw_entry->pw_gecos, pw_entry->pw_name);
    printf(" %s", full_name_ptr);
    free(full_name_ptr);
    putchar_unlocked('\n');

    if (include_home_and_shell) {
      printf("%s", gettext("Directory: "));
      printf("%-29s", pw_entry->pw_dir);
      printf("%s", gettext("Shell: "));
      printf(" %s", pw_entry->pw_shell);
      putchar_unlocked('\n');
    }

    size_t dir_len = strlen(pw_entry->pw_dir);
    
    if (include_project) {
      char *path_buffer = (char *)xmalloc(dir_len + strlen("/.project") + 1);
      strcpy(path_buffer, pw_entry->pw_dir);
      strcat(path_buffer, "/.project");

      FILE *project_file = rpl_fopen(path_buffer, "r");
      if (project_file != NULL) {
        printf("%s", gettext("Project: "));
        char read_buffer[1024];
        size_t bytes_read;
        while ((bytes_read = fread_unlocked(read_buffer, 1, sizeof(read_buffer), project_file)) > 0) {
          fwrite_unlocked(read_buffer, 1, bytes_read, stdout);
        }
        rpl_fclose(project_file);
      }
      free(path_buffer);
    }

    if (include_plan) {
      char *path_buffer = (char *)xmalloc(dir_len + strlen("/.plan") + 1);
      strcpy(path_buffer, pw_entry->pw_dir);
      strcat(path_buffer, "/.plan");

      FILE *plan_file = rpl_fopen(path_buffer, "r");
      if (plan_file != NULL) {
        printf("%s", gettext("Plan:\n"));
        char read_buffer[1024];
        size_t bytes_read;
        while ((bytes_read = fread_unlocked(read_buffer, 1, sizeof(read_buffer), plan_file)) > 0) {
          fwrite_unlocked(read_buffer, 1, bytes_read, stdout);
        }
        rpl_fclose(plan_file);
      }
      free(path_buffer);
    }
    putchar_unlocked('\n');
  }
}

// Function: print_heading
void print_heading(void) {
  printf("%-8s", gettext("Login"));
  if (include_fullname) {
    printf(" %-19s", gettext("Name"));
  }
  printf(" %-9s", gettext("Tty"));
  if (include_idle) {
    printf(" %-6s", gettext("Idle"));
  }
  printf(" %-*s", time_format_width, gettext("When"));
  if (include_where) {
    printf(" %s", gettext("Where"));
  }
  putchar_unlocked('\n');
}

// Function: scan_entries
void scan_entries(long count, UtmpRecord *records, int num_users, char **users_list) {
  char is_hard_locale = hard_locale(LC_TIME);
  if (is_hard_locale == 0) {
    time_format = "%b %e %H:%M";
    time_format_width = 12;
  } else {
    time_format = "%Y-%m-%d %H:%M";
    time_format_width = 16;
  }

  if (include_heading) {
    print_heading();
  }

  for (long i = 0; i < count; ++i) {
    UtmpRecord *current_record = &records[i];
    if (current_record->fields[0] != NULL && *current_record->fields[0] != '\0' && current_record->type == 7) {
      if (num_users == 0) {
        print_entry(current_record);
      } else {
        for (int j = 0; j < num_users; ++j) {
          if (streq(current_record->fields[0], users_list[j])) {
            print_entry(current_record);
            break;
          }
        }
      }
    }
  }
}

// Function: short_pinky
void short_pinky(const char *utmp_file, int num_users, char **users_list) {
  long count;
  UtmpRecord *records;

  if (read_utmp(utmp_file, &count, &records, 2) != 0) {
    error(1, errno, gettext("Failed to read utmp file %s"), quotearg_n_style_colon(0, 3, utmp_file));
  }
  scan_entries(count, records, num_users, users_list);
  
  // Free the records allocated by read_utmp
  if (records) {
      for (long i = 0; i < count; ++i) {
          for (int j = 0; j < 9; ++j) {
              free(records[i].fields[j]);
          }
      }
      free(records);
  }
  exit(0);
}

// Function: long_pinky
void long_pinky(int num_users, char **users_list) {
  for (int i = 0; i < num_users; ++i) {
    print_long_entry(users_list[i]);
  }
}

// Function: usage
void usage(int param_1) {
  printf(gettext("Usage: %s [OPTION]... [USER]...\n"), _program_name);
  fputs_unlocked(gettext(
                            "\n  -l              produce long format output for the specified USERs\n  -b              omit the user\'s home directory and shell in long format\n  -h              omit the user\'s project file in long format\n  -p              omit the user\'s plan file in long format\n  -s              do short format output, this is the default\n"
                            ), stdout);
  fputs_unlocked(gettext(
                            "  -f              omit the line of column headings in short format\n  -w              omit the user\'s full name in short format\n  -i              omit the user\'s full name and remote host in short format\n  -q              omit the user\'s full name, remote host and idle time\n                  in short format\n"
                            ), stdout);
  fputs_unlocked(gettext("      --lookup    attempt to canonicalize hostnames via DNS\n"), stdout);
  fputs_unlocked(gettext("      --help        display this help and exit\n"), stdout);
  fputs_unlocked(gettext("      --version     output version information and exit\n"), stdout);
  printf(gettext(
                            "\nA lightweight \'finger\' program;  print user information.\nThe utmp file will be %s.\n"
                            ), "/var/run/utmp");
  emit_ancillary_info("pinky");
  if (param_1 != 0) {
    fprintf(stderr, gettext("Try \'%s --help\' for more information.\n"), _program_name);
  }
  exit(param_1);
}

// Function: main
int main(int argc, char **argv) {
  set_program_name(argv[0]);
  setlocale(LC_ALL, ""); // Use LC_ALL for general locale settings
  bindtextdomain("coreutils","/usr/local/share/locale");
  textdomain("coreutils");
  atexit(close_stdout);

  int opt_char;
  while ((opt_char = getopt_long(argc, argv, "sfwiqbhlp", longopts, NULL)) != -1) {
    switch(opt_char) {
      case 0: // For long options that don't have a short equivalent and return 0
        if (longopts[optind - 1].val == 0x80) { // --lookup
            do_lookup = 1;
        } else {
            usage(1); // Unknown long option
        }
        break;
      case 's':
        do_short_format = 1;
        break;
      case 'f':
        include_heading = 0;
        break;
      case 'w':
        include_fullname = 0;
        break;
      case 'i':
        include_fullname = 0;
        include_where = 0;
        break;
      case 'q':
        include_fullname = 0;
        include_where = 0;
        include_idle = 0;
        break;
      case 'b':
        include_home_and_shell = 0;
        break;
      case 'h': // This 'h' is for "omit project file" in long format
        include_project = 0;
        break;
      case 'l':
        do_short_format = 0;
        break;
      case 'p':
        include_plan = 0;
        break;
      case 0x81: // --help, custom value
        usage(0);
        break;
      case 0x82: // --version, custom value
        version_etc(stdout, "pinky", "GNU coreutils", _Version,
                    proper_name_lite("Joseph Arceneaux", "Joseph Arceneaux"),
                    proper_name_lite("David MacKenzie", "David MacKenzie"),
                    proper_name_lite("Kaveh Ghazi", "Kaveh Ghazi"), 0);
        exit(0);
      case '?': // Unknown option character or missing argument
        usage(1);
        break;
      default:
        usage(1);
        break;
    }
  }

  argc -= optind;
  argv += optind;

  if (!do_short_format && argc == 0) {
    error(0, 0, gettext("no username specified; at least one must be specified when using -l"));
    usage(1);
  }

  if (!do_short_format) {
    long_pinky(argc, argv);
  } else {
    short_pinky("/var/run/utmp", argc, argv);
  }

  return 0;
}