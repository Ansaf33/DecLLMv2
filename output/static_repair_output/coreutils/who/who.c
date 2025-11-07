#define _GNU_SOURCE // For asprintf, fputs_unlocked, stpcpy, __errno_location
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>
#include <locale.h>
#include <libintl.h>
#include <getopt.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <limits.h> // For LLONG_MIN

// --- Dummy/Mock implementations for external functions and globals ---
// In a real application, these would be provided by other modules or libraries.

// Global variables (simulating the decompiler's `_` naming convention or DAT_ addresses)
static time_t now_val = LLONG_MIN; // Equivalent to -0x8000000000000000
static char idle_hhmm_buf[16];
static char buf_time_string[34]; // 0x21 + null terminator
static char mesg_status_char; // For mesg_6._1_1_
static int dev_dir_fd = 0;
static size_t host_buffer_len = 0;
static char *host_buffer = NULL;
static char *runlevel_line_buffer = NULL;
static char *comment_buffer = NULL;
static char *exit_string_buffer = NULL;

// Flags
static bool include_idle = false;
static bool short_output = false;
static bool include_exit = false;
static bool include_mesg = false;
static bool do_lookup = false;
static bool include_heading = false;
static bool my_line_only = false;
static bool need_users = false;
static bool need_runlevel = false;
static bool need_boottime = false;
static bool need_clockchange = false;
static bool need_initspawn = false;
static bool need_login = false;
static bool need_deadprocs = false;
static bool short_list = false;

static const char *time_format;
static int time_format_width;

extern char *program_name; // From set_program_name
extern int optind; // From unistd.h, for getopt_long

// String literal equivalents for DAT_ addresses
static const char *DAT_empty_string = "";
static const char *DAT_id_equals = "id=";
static const char *DAT_comma_space = ", ";
static const char *DAT_column_entry = "ENTRY";
static const char *DAT_column_user = "USER";
static const char *DAT_column_line = "LINE";
static const char *DAT_column_idle = "IDLE";
static const char *DAT_column_login_time = "LOGIN TIME";
static const char *DAT_column_comment = "COMMENT";
static const char *DAT_column_exit = "EXIT";
static const char *DAT_error_utmp = "cannot open %s: %s";
static const char *DAT_ancillary_info_arg = "who"; // Placeholder for emit_ancillary_info

// Stack protection (compiler intrinsics normally)
static unsigned long __stack_chk_guard = 0xDEADBEEFDEADBEEFUL; // Dummy value
void __stack_chk_fail(void) {
    fprintf(stderr, "Stack smashing detected!\n");
    exit(EXIT_FAILURE);
}

// Dummy rpl_time
time_t *rpl_time(time_t *t) {
    if (t) {
        *t = time(NULL);
    }
    return t;
}

// Dummy timetostr (simplified)
char *timetostr(time_t t, char *buf) {
    struct tm *tm_info = localtime(&t);
    if (tm_info) {
        strftime(buf, 32, "%Y-%m-%d %H:%M:%S", tm_info);
    } else {
        strcpy(buf, "INVALID_TIME");
    }
    return buf;
}

// Dummy xmalloc (basic error-checking malloc)
void *xmalloc(size_t size) {
    void *ptr = malloc(size);
    if (!ptr && size > 0) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

// Dummy xalloc_die
void xalloc_die(void) {
    fprintf(stderr, "Error: Memory allocation failed (xalloc_die).\n");
    exit(EXIT_FAILURE);
}

// Dummy xpalloc (realloc-like, but for growing)
void *xpalloc(void *oldptr, size_t *oldsize, size_t new_capacity_delta, size_t element_size, size_t count) {
    size_t new_size = *oldsize + new_capacity_delta;
    void *newptr = realloc(oldptr, new_size * element_size);
    if (!newptr && new_size * element_size > 0) {
        fprintf(stderr, "Error: Memory reallocation failed.\n");
        exit(EXIT_FAILURE);
    }
    *oldsize = new_size;
    return newptr;
}

// Dummy canon_host (returns a copy of the input for simplicity)
char *canon_host(const char *host) {
    return strdup(host);
}

// Dummy set_program_name
char *program_name;
void set_program_name(char *name) {
    program_name = name;
}

// Dummy close_stdout
void close_stdout(void) {
    fflush(stdout);
    // In a real program, might check for write errors and close.
}

// Dummy proper_name_lite
const char *proper_name_lite(const char *name_ascii, const char *name_utf8) {
    return name_utf8;
}

// Dummy version_etc
void version_etc(FILE *stream, const char *package, const char *program, const char *version, const char *author1, const char *author2, const char *author3, ...) {
    fprintf(stream, "%s (%s) %s\n", program, package, version);
    fprintf(stream, "Copyright (C) 2023 Free Software Foundation, Inc.\n");
    fprintf(stream, "License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\n");
    fprintf(stream, "This is free software: you are free to change and redistribute it.\n");
    fprintf(stream, "There is NO WARRANTY, to the extent permitted by law.\n\n");
    fprintf(stream, "Written by %s, %s, and %s.\n", author1, author2, author3);
}

// Dummy hard_locale
bool hard_locale(int category) {
    // For simplicity, always return true to use localized time format
    return true;
}

// Dummy str_endswith
bool str_endswith(const char *str, const char *suffix) {
    if (!str || !suffix) {
        return false;
    }
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    if (suffix_len > str_len) {
        return false;
    }
    return strcmp(str + str_len - suffix_len, suffix) == 0;
}

// Dummy read_utmp
// param_1: filename, param_2: *num_entries, param_3: **entries_data, param_4: flags
int read_utmp(const char *filename, long *num_entries, void **entries_data, unsigned int flags) {
    // Simulate reading some dummy data
    *num_entries = 0;
    *entries_data = NULL;
    // For demonstration, let's return success but no data
    return 0;
}

// Dummy quotearg_n_style_colon
const char *quotearg_n_style_colon(int n, int style, const char *arg) {
    return arg;
}

// Dummy error function (simplified)
void error(int status, int errnum, const char *format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, "%s: ", program_name ? program_name : "unknown");
    vfprintf(stderr, format, args);
    va_end(args);
    if (errnum) {
        fprintf(stderr, ": %s", strerror(errnum));
    }
    fprintf(stderr, "\n");
    if (status) {
        exit(status);
    }
}

// Dummy emit_ancillary_info
void emit_ancillary_info(const char *program) {
    fprintf(stderr, "Full documentation <https://www.gnu.org/software/coreutils/%s>\n", program);
    fprintf(stderr, "or available locally via: info '(coreutils) %s invocation'\n", program);
}

// Dummy extract_trimmed_name
char *extract_trimmed_name(char **entry_data) {
    // entry_data[0] is user name
    return strdup(entry_data[0]); // Return a copy of the user name
}

// --- End of Dummy/Mock implementations ---

// Function: idle_string
char *idle_string(long login_time_sec, long current_boot_time_sec) {
    unsigned long stack_chk_guard = __stack_chk_guard; // Stack canary
    char *result_string;

    if (now_val == LLONG_MIN) {
        rpl_time(&now_val);
    }

    long idle_seconds_long = now_val - login_time_sec;
    int idle_seconds_int = (int)idle_seconds_long;

    // Check conditions for " old "
    // 1. Login time <= boot time (param_1 <= param_2)
    // 2. Current time < login time (now_val < login_time_sec)
    // 3. idle_seconds_long does not fit into int (overflow/underflow)
    // 4. idle_seconds_int is >= 1 day (0x15180 seconds or 86400)
    if (login_time_sec <= current_boot_time_sec ||
        now_val < login_time_sec ||
        idle_seconds_long != (long)idle_seconds_int || // Check if long to int cast truncated
        idle_seconds_int >= 86400) { // 0x1517f < iVar2 means iVar2 >= 0x15180
        result_string = gettext(" old ");
    } else if (idle_seconds_int < 60) { // Less than 1 minute
        result_string = "  .  ";
    } else {
        // Format as HH:MM
        sprintf(idle_hhmm_buf, "%02d:%02d", idle_seconds_int / 3600, (idle_seconds_int % 3600) / 60);
        result_string = idle_hhmm_buf;
    }

    if (stack_chk_guard != __stack_chk_guard) {
        __stack_chk_fail();
    }
    return result_string;
}

// Function: time_string
char *time_string(long param_entry_ptr) {
    unsigned long stack_chk_guard = __stack_chk_guard; // Stack canary
    time_t timestamp = *(time_t *)(param_entry_ptr + 0x20); // Assuming 0x20 is offset to time_t
    struct tm *tm_info = localtime(&timestamp);
    char *result_buf;

    if (tm_info == NULL) {
        result_buf = timetostr(timestamp, buf_time_string);
    } else {
        strftime(buf_time_string, sizeof(buf_time_string), time_format, tm_info);
        result_buf = buf_time_string;
    }

    if (stack_chk_guard != __stack_chk_guard) {
        __stack_chk_fail();
    }
    return result_buf;
}

// Function: print_line
void print_line(const char *param_1_str, char message_status, const char *param_3_str, const char *param_4_str,
                const char *param_5_str, const char *param_6_str, const char *param_7_str, const char *param_8_str) {
    unsigned long stack_chk_guard = __stack_chk_guard; // Stack canary
    char *output_line = NULL;
    char idle_field[10]; // " %-6s" + null
    char tty_field[15];  // " %10s" + null
    char *exit_field_buffer = NULL;
    const char *message_status_display = DAT_empty_string;
    const char *login_name = param_1_str ? param_1_str : DAT_empty_string;

    // Handle mesg_status_char
    mesg_status_char = message_status;
    if (include_mesg) {
        static char mesg_status_str[4]; // "+ ", "- ", "? ", "\0"
        sprintf(mesg_status_str, "%c ", mesg_status_char);
        message_status_display = mesg_status_str;
    }

    if (include_idle && !short_output && strlen(param_5_str) <= 6) {
        sprintf(idle_field, " %-6s", param_5_str);
    } else {
        idle_field[0] = '\0';
    }

    if (!short_output && strlen(param_6_str) <= 11) {
        sprintf(tty_field, " %10s", param_6_str);
    } else {
        tty_field[0] = '\0';
    }

    if (include_exit) {
        size_t exit_str_len = strlen(param_8_str);
        size_t required_len = (exit_str_len < 12) ? 14 : exit_str_len + 2; // " %-12s" or " %s"
        exit_field_buffer = (char *)xmalloc(required_len);
        sprintf(exit_field_buffer, " %-12s", param_8_str);
    } else {
        exit_field_buffer = (char *)xmalloc(1);
        exit_field_buffer[0] = '\0';
    }

    int result = asprintf(&output_line, "%-8s%s%-12s %-*s%s%s %-8s%s",
                          login_name,
                          message_status_display,
                          param_3_str,
                          time_format_width, param_4_str,
                          idle_field,
                          tty_field,
                          param_7_str,
                          exit_field_buffer);

    if (result == -1) {
        xalloc_die();
    }

    // Trim trailing spaces
    char *end = output_line + strlen(output_line) - 1;
    while (end >= output_line && *end == ' ') {
        end--;
    }
    *(end + 1) = '\0';

    puts(output_line);
    free(output_line);
    free(exit_field_buffer);

    if (stack_chk_guard != __stack_chk_guard) {
        __stack_chk_fail();
    }
}

// Function: is_tty_writable
bool is_tty_writable(const struct stat *st) {
    return (st->st_mode & S_IWOTH) != 0; // S_IWOTH is 0x10 in typical systems
}

// Function: print_user
void print_user(long *entry_ptr, long current_boot_time_sec) {
    unsigned long stack_chk_guard = __stack_chk_guard; // Stack canary
    char tty_pid_str[12]; // For PID, max 10 digits + null
    const char *tty_device_name = (char *)entry_ptr[2]; // Entry's tty field
    const char *host_name = (char *)entry_ptr[3];       // Entry's host field
    char status_char;
    time_t access_time_sec;

    sprintf(tty_pid_str, "%ld", (long)*(int *)(entry_ptr + 6)); // Assuming offset 6 * sizeof(long) for PID

    const char *tty_path_start = strchr(tty_device_name, ' ');
    if (tty_path_start) {
        tty_path_start++; // Skip space
    } else {
        tty_path_start = tty_device_name;
    }

    int stat_dir_fd = AT_FDCWD; // Default to current directory
    if (*tty_path_start == '/') {
        stat_dir_fd = AT_FDCWD; // Absolute path, fstatat with AT_FDCWD
    } else {
        if (dev_dir_fd == 0) { // Not opened yet
            dev_dir_fd = open("/dev", O_RDONLY | O_DIRECTORY);
            if (dev_dir_fd < 0) {
                dev_dir_fd = -101; // Sentinel for error
            }
        }
        stat_dir_fd = dev_dir_fd;
    }

    struct stat tty_stat;
    if (stat_dir_fd == -101 || fstatat(stat_dir_fd, tty_path_start, &tty_stat, 0) != 0) {
        status_char = '?';
        access_time_sec = 0;
    } else {
        status_char = is_tty_writable(&tty_stat) ? '+' : '-';
        access_time_sec = tty_stat.st_atime; // Use st_atime for last access
    }

    char idle_display[10];
    if (access_time_sec == 0) {
        sprintf(idle_display, "  ?");
    } else {
        sprintf(idle_display, "%.*s", 6, idle_string(access_time_sec, current_boot_time_sec));
    }

    // Hostname processing
    if (*host_name == '\0') {
        if (host_buffer_len < 1) {
            host_buffer = (char *)xpalloc(host_buffer, &host_buffer_len, 1, sizeof(char), 1);
        }
        *host_buffer = '\0';
    } else {
        char *colon_pos = strchr(host_name, ':');
        char *hostname_part = (char *)host_name;
        char *display_part = NULL;
        char *canonical_host = NULL;

        if (colon_pos) {
            *colon_pos = '\0'; // Temporarily null-terminate hostname part
            display_part = colon_pos + 1;
        }

        if (*hostname_part != '\0' && do_lookup) {
            canonical_host = canon_host(hostname_part);
        }
        if (canonical_host == NULL) {
            canonical_host = hostname_part;
        }

        size_t canonical_len = strlen(canonical_host);
        size_t display_len = display_part ? strlen(display_part) : 0;
        size_t total_len = canonical_len + (display_part ? display_len + 1 : 0) + 3; // (hostname:display)\0

        if (host_buffer_len < total_len) {
            free(host_buffer);
            host_buffer = (char *)xpalloc(NULL, &host_buffer_len, total_len, sizeof(char), 1);
        }

        char *current_pos = host_buffer;
        *current_pos++ = '(';
        current_pos = stpcpy(current_pos, canonical_host);
        if (display_part) {
            *current_pos++ = ':';
            current_pos = stpcpy(current_pos, display_part);
        }
        *current_pos++ = ')';
        *current_pos = '\0';

        if (canonical_host != hostname_part) {
            free(canonical_host);
        }
        if (colon_pos) {
            *colon_pos = ':'; // Restore original string
        }
    }

    const char *host_display = host_buffer && *host_buffer ? host_buffer : DAT_empty_string;
    const char *login_time_str = time_string(entry_ptr);

    print_line((char *)entry_ptr[0], status_char, (char *)entry_ptr[2], login_time_str,
               idle_display, tty_pid_str, host_display, DAT_empty_string);

    if (stack_chk_guard != __stack_chk_guard) {
        __stack_chk_fail();
    }
}

// Function: print_boottime
void print_boottime(long entry_ptr) {
    unsigned long stack_chk_guard = __stack_chk_guard; // Stack canary
    print_line(DAT_empty_string, ' ', gettext("system boot"), time_string(entry_ptr),
               DAT_empty_string, DAT_empty_string, DAT_empty_string, DAT_empty_string);
    if (stack_chk_guard != __stack_chk_guard) {
        __stack_chk_fail();
    }
}

// Function: make_id_equals_comment
void *make_id_equals_comment(long entry_ptr) {
    unsigned long stack_chk_guard = __stack_chk_guard; // Stack canary
    const char *id_str = *(const char **)(entry_ptr + 8); // Assuming offset 8 for id string
    const char *id_equals_prefix = gettext(DAT_id_equals);

    size_t id_str_len = strlen(id_str);
    size_t prefix_len = strlen(id_equals_prefix);
    char *buffer = (char *)xmalloc(id_str_len + prefix_len + 1);

    char *current_pos = mempcpy(buffer, id_equals_prefix, prefix_len);
    mempcpy(current_pos, id_str, id_str_len);
    buffer[id_str_len + prefix_len] = '\0';

    if (stack_chk_guard != __stack_chk_guard) {
        __stack_chk_fail();
    }
    return buffer;
}

// Function: print_deadprocs
void print_deadprocs(long entry_ptr) {
    unsigned long stack_chk_guard = __stack_chk_guard; // Stack canary
    char *id_comment = (char *)make_id_equals_comment(entry_ptr);
    char pid_str[12]; // For PID
    sprintf(pid_str, "%ld", (long)*(int *)(entry_ptr + 0x30)); // Assuming 0x30 for PID

    if (exit_string_buffer == NULL) {
        const char *term_prefix = gettext("term=");
        const char *exit_prefix = gettext("exit=");
        size_t required_len = strlen(term_prefix) + strlen(exit_prefix) + 10 + 10 + 1 + 1; // "term=N exit=N\0"
        exit_string_buffer = (char *)xmalloc(required_len);
    }

    unsigned int exit_status = *(unsigned int *)(entry_ptr + 0x40); // Assuming 0x40 for exit status
    unsigned int termination_status = *(unsigned int *)(entry_ptr + 0x3C); // Assuming 0x3C for termination status
    sprintf(exit_string_buffer, "%s%d %s%d",
            gettext("term="), termination_status,
            gettext("exit="), exit_status);

    print_line(DAT_empty_string, ' ', *(const char **)(entry_ptr + 0x10), time_string(entry_ptr),
               DAT_empty_string, pid_str, id_comment, exit_string_buffer);

    free(id_comment);
    if (stack_chk_guard != __stack_chk_guard) {
        __stack_chk_fail();
    }
}

// Function: print_login
void print_login(long entry_ptr) {
    unsigned long stack_chk_guard = __stack_chk_guard; // Stack canary
    char *id_comment = (char *)make_id_equals_comment(entry_ptr);
    char pid_str[12];
    sprintf(pid_str, "%ld", (long)*(int *)(entry_ptr + 0x30)); // Assuming 0x30 for PID

    print_line(gettext("LOGIN"), ' ', *(const char **)(entry_ptr + 0x10), time_string(entry_ptr),
               DAT_empty_string, pid_str, id_comment, DAT_empty_string);

    free(id_comment);
    if (stack_chk_guard != __stack_chk_guard) {
        __stack_chk_fail();
    }
}

// Function: print_initspawn
void print_initspawn(long entry_ptr) {
    unsigned long stack_chk_guard = __stack_chk_guard; // Stack canary
    char *id_comment = (char *)make_id_equals_comment(entry_ptr);
    char pid_str[12];
    sprintf(pid_str, "%ld", (long)*(int *)(entry_ptr + 0x30)); // Assuming 0x30 for PID

    print_line(DAT_empty_string, ' ', *(const char **)(entry_ptr + 0x10), time_string(entry_ptr),
               DAT_empty_string, pid_str, id_comment, DAT_empty_string);

    free(id_comment);
    if (stack_chk_guard != __stack_chk_guard) {
        __stack_chk_fail();
    }
}

// Function: print_clockchange
void print_clockchange(long entry_ptr) {
    unsigned long stack_chk_guard = __stack_chk_guard; // Stack canary
    print_line(DAT_empty_string, ' ', gettext("clock change"), time_string(entry_ptr),
               DAT_empty_string, DAT_empty_string, DAT_empty_string, DAT_empty_string);
    if (stack_chk_guard != __stack_chk_guard) {
        __stack_chk_fail();
    }
}

// Function: print_runlevel
void print_runlevel(long entry_ptr) {
    unsigned long stack_chk_guard = __stack_chk_guard; // Stack canary
    int runlevel_data = *(int *)(entry_ptr + 0x30); // Assuming 0x30 for runlevel info
    char current_runlevel = (char)(runlevel_data & 0xFF);
    char previous_runlevel = (char)((runlevel_data >> 8) & 0xFF);

    if (runlevel_line_buffer == NULL) {
        const char *prefix = gettext("run-level");
        runlevel_line_buffer = (char *)xmalloc(strlen(prefix) + 3); // "run-level X\0"
    }
    sprintf(runlevel_line_buffer, "%s %c", gettext("run-level"), current_runlevel);

    if (comment_buffer == NULL) {
        const char *prefix = gettext("last=");
        comment_buffer = (char *)xmalloc(strlen(prefix) + 2); // "last=X\0"
    }
    char prev_display_char = (previous_runlevel == 'N') ? 'S' : previous_runlevel;
    sprintf(comment_buffer, "%s%c", gettext("last="), prev_display_char);

    const char *comment_display = c_isprint((unsigned char)previous_runlevel) ? comment_buffer : DAT_empty_string;

    print_line(DAT_empty_string, ' ', runlevel_line_buffer, time_string(entry_ptr),
               DAT_empty_string, DAT_empty_string, comment_display, DAT_empty_string);
    if (stack_chk_guard != __stack_chk_guard) {
        __stack_chk_fail();
    }
}

// Function: list_entries_who
void list_entries_who(long num_entries, char **entries_data) {
    unsigned long stack_chk_guard = __stack_chk_guard; // Stack canary
    long user_count = 0;
    const char *separator = DAT_empty_string;

    for (long i = 0; i < num_entries; ++i) {
        char **current_entry = entries_data + (i * 9); // Assuming 9 fields per entry
        // Assuming entry_data[0] is user, entry_data[7] is type
        if (*current_entry[0] != '\0' && *(short *)(current_entry + 7) == 7) { // Assuming type 7 is USER_PROCESS
            char *trimmed_name = extract_trimmed_name(current_entry);
            printf("%s%s", separator, trimmed_name);
            free(trimmed_name);
            separator = DAT_comma_space;
            user_count++;
        }
    }
    printf(gettext("\n# users=%td\n"), user_count);
    if (stack_chk_guard != __stack_chk_guard) {
        __stack_chk_fail();
    }
}

// Function: print_heading
void print_heading(void) {
    unsigned long stack_chk_guard = __stack_chk_guard; // Stack canary
    print_line(gettext(DAT_column_user), ' ', gettext(DAT_column_line), gettext(DAT_column_login_time),
               gettext(DAT_column_idle), gettext(DAT_column_entry), gettext(DAT_column_comment), gettext(DAT_column_exit));
    if (stack_chk_guard != __stack_chk_guard) {
        __stack_chk_fail();
    }
}

// Function: scan_entries
void scan_entries(long num_entries, char **entries_data) {
    unsigned long stack_chk_guard = __stack_chk_guard; // Stack canary
    const char *my_tty_name = NULL;
    long last_boot_time = 0; // Initialize with 0 or a known invalid time_t

    if (include_heading) {
        print_heading();
    }

    if (my_line_only) {
        my_tty_name = ttyname(STDIN_FILENO);
        if (my_tty_name == NULL) {
            // Cannot determine TTY, so no entries will match
            return;
        }
        if (strncmp(my_tty_name, "/dev/", 5) == 0) {
            my_tty_name += 5; // Skip "/dev/" prefix
        }
    }

    for (long i = 0; i < num_entries; ++i) {
        char **current_entry = entries_data + (i * 9); // Assuming 9 fields per entry
        // Assuming current_entry[0] is user, current_entry[2] is line, current_entry[7] is type, current_entry[4] is time_t
        short entry_type = *(short *)(current_entry + 7);

        bool matches_my_line = !my_line_only || str_endswith(current_entry[2], my_tty_name);

        if (matches_my_line) {
            if (need_users && *current_entry[0] != '\0' && entry_type == 7) { // USER_PROCESS
                print_user((long *)current_entry, last_boot_time);
            } else if (need_runlevel && entry_type == 1) { // RUN_LVL
                print_runlevel((long)current_entry);
            } else if (need_boottime && entry_type == 2) { // BOOT_TIME
                print_boottime((long)current_entry);
            } else if (need_clockchange && entry_type == 3) { // OLD_TIME or NEW_TIME
                print_clockchange((long)current_entry);
            } else if (need_initspawn && entry_type == 5) { // INIT_PROCESS
                print_initspawn((long)current_entry);
            } else if (need_login && entry_type == 6) { // LOGIN_PROCESS
                print_login((long)current_entry);
            } else if (need_deadprocs && entry_type == 8) { // DEAD_PROCESS
                print_deadprocs((long)current_entry);
            }
        }

        // Update last_boot_time if this is a BOOT_TIME entry
        if (entry_type == 2) { // BOOT_TIME
            last_boot_time = *(long *)(current_entry + 4); // Assuming offset 4 * sizeof(long) for timestamp
        }
    }
    if (stack_chk_guard != __stack_chk_guard) {
        __stack_chk_fail();
    }
}

// Function: who
void who(const char *filename, unsigned int flags) {
    unsigned long stack_chk_guard = __stack_chk_guard; // Stack canary
    long num_entries;
    void *entries_data;
    unsigned int read_flags = flags;

    if (short_list) {
        read_flags |= 2; // Add flag for short list
    }

    if (read_utmp(filename, &num_entries, &entries_data, read_flags) != 0) {
        error(1, *__errno_location(), gettext(DAT_error_utmp), quotearg_n_style_colon(0, 3, filename));
    }

    if (short_list) {
        list_entries_who(num_entries, (char **)entries_data);
    } else {
        scan_entries(num_entries, (char **)entries_data);
    }

    free(entries_data);
    if (stack_chk_guard != __stack_chk_guard) {
        __stack_chk_fail();
    }
}

// Function: usage
void usage(int status) {
    unsigned long stack_chk_guard = __stack_chk_guard; // Stack canary
    if (status == 0) {
        printf(gettext("Usage: %s [OPTION]... [ FILE | ARG1 ARG2 ]\n"), program_name);
        fputs_unlocked(gettext("Print information about users who are currently logged in.\n"), stdout);
        fputs_unlocked(gettext("\n  -a, --all         same as -b -d --login -p -r -t -T -u\n  -b, --boot        time of last system boot\n  -d, --dead        print dead processes\n  -H, --heading     print line of column headings\n"), stdout);
        fputs_unlocked(gettext("  -l, --login       print system login processes\n"), stdout);
        fputs_unlocked(gettext("      --lookup      attempt to canonicalize hostnames via DNS\n  -m                only hostname and user associated with standard input\n  -p, --process     print active processes spawned by init\n"), stdout);
        fputs_unlocked(gettext("  -q, --count       all login names and number of users logged on\n  -r, --runlevel    print current runlevel\n  -s, --short       print only name, line, and time (default)\n  -t, --time        print last system clock change\n"), stdout);
        fputs_unlocked(gettext("  -T, -w, --mesg    add user\'s message status as +, - or ?\n  -u, --users       list users logged in\n      --message     same as -T\n      --writable    same as -T\n"), stdout);
        fputs_unlocked(gettext("      --help        display this help and exit\n"), stdout);
        fputs_unlocked(gettext("      --version     output version information and exit\n"), stdout);
        printf(gettext("\nIf FILE is not specified, use %s.  %s as FILE is common.\nIf ARG1 ARG2 given, -m presumed: \'am i\' or \'mom likes\' are usual.\n"), "/var/run/utmp", "/var/log/wtmp");
        emit_ancillary_info(DAT_ancillary_info_arg);
    } else {
        fprintf(stderr, gettext("Try '%s --help' for more information.\n"), program_name);
    }
    exit(status);
    if (stack_chk_guard != __stack_chk_guard) {
        __stack_chk_fail();
    }
}

// Global long options for getopt_long
static const struct option longopts[] = {
    {"all", no_argument, NULL, 'a'},
    {"boot", no_argument, NULL, 'b'},
    {"dead", no_argument, NULL, 'd'},
    {"heading", no_argument, NULL, 'H'},
    {"login", no_argument, NULL, 'l'},
    {"lookup", no_argument, (int *)&do_lookup, 1}, // Set do_lookup to 1
    {"process", no_argument, NULL, 'p'},
    {"count", no_argument, NULL, 'q'},
    {"runlevel", no_argument, NULL, 'r'},
    {"short", no_argument, NULL, 's'},
    {"time", no_argument, NULL, 't'},
    {"mesg", no_argument, NULL, 'T'},
    {"message", no_argument, NULL, 'T'}, // Same as --mesg
    {"writable", no_argument, NULL, 'T'}, // Same as --mesg
    {"users", no_argument, NULL, 'u'},
    {"help", no_argument, NULL, 0x81},    // Custom value for --help
    {"version", no_argument, NULL, 0x82}, // Custom value for --version
    {NULL, 0, NULL, 0}};

// Function: main
int main(int argc, char **argv) {
    unsigned long stack_chk_guard = __stack_chk_guard; // Stack canary
    bool default_display_mode = true;
    int opt;

    set_program_name(argv[0]);
    setlocale(LC_ALL, ""); // Use LC_ALL for full localization
    bindtextdomain("coreutils", "/usr/local/share/locale");
    textdomain("coreutils");
    atexit(close_stdout);

    while ((opt = getopt_long(argc, argv, "abdlmpqrstuwHT", longopts, NULL)) != -1) {
        switch (opt) {
        case 'a':
            need_boottime = true;
            need_deadprocs = true;
            need_login = true;
            need_initspawn = true;
            need_runlevel = true;
            need_clockchange = true;
            need_users = true;
            include_mesg = true;
            include_idle = true;
            include_exit = true;
            default_display_mode = false;
            break;
        case 'b':
            need_boottime = true;
            default_display_mode = false;
            break;
        case 'd':
            need_deadprocs = true;
            include_idle = true;
            include_exit = true;
            default_display_mode = false;
            break;
        case 'H':
            include_heading = true;
            break;
        case 'l':
            need_login = true;
            include_idle = true;
            default_display_mode = false;
            break;
        case 'm':
            my_line_only = true;
            break;
        case 'p':
            need_initspawn = true;
            default_display_mode = false;
            break;
        case 'q':
            short_list = true;
            break;
        case 'r':
            need_runlevel = true;
            include_idle = true;
            default_display_mode = false;
            break;
        case 's':
            short_output = true;
            break;
        case 't':
            need_clockchange = true;
            default_display_mode = false;
            break;
        case 'T':
        case 'w': // -w is also --mesg
            include_mesg = true;
            break;
        case 'u':
            need_users = true;
            include_idle = true;
            default_display_mode = false;
            break;
        case 0: // Long option with a flag set
            if (do_lookup) { // This means --lookup was specified
                // flag already set by getopt_long
            }
            break;
        case 0x81: // --help
            usage(0);
            break;
        case 0x82: // --version
            version_etc(stdout, DAT_ancillary_info_arg, "GNU coreutils", "0.0.1", // Placeholder version
                        proper_name_lite("Michael Stone", "Michael Stone"),
                        proper_name_lite("David MacKenzie", "David MacKenzie"),
                        proper_name_lite("Joseph Arceneaux", "Joseph Arceneaux"),
                        NULL);
            exit(0);
        default: // Unknown option
            usage(1);
        }
    }

    if (default_display_mode) {
        need_users = true;
        short_output = true;
    }

    if (include_exit) {
        short_output = false; // If exit info is needed, don't use short output
    }

    // Determine time format based on locale
    if (!hard_locale(LC_TIME)) { // If locale is not "hard" (e.g., C locale)
        time_format = "%b %e %H:%M"; // e.g., "Jan 1 00:00"
        time_format_width = 12;
    } else {
        time_format = "%Y-%m-%d %H:%M"; // e.g., "2023-01-01 00:00"
        time_format_width = 16;
    }

    int remaining_args = argc - optind;
    if (remaining_args == 0) {
        who("/var/run/utmp", 0);
    } else if (remaining_args == 1) {
        who(argv[optind], 0);
    } else if (remaining_args == 2) {
        // If two args, -m is presumed. Example: "am i"
        my_line_only = true;
        who("/var/run/utmp", 0);
    } else {
        error(0, 0, gettext("extra operand %s"), quote(argv[optind + 2]));
        usage(1);
    }

    if (stack_chk_guard != __stack_chk_guard) {
        __stack_chk_fail();
    }
    return 0;
}