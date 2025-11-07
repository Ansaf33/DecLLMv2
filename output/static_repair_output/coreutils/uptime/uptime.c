#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h> // For exit, atexit, free
#include <errno.h>  // For __errno_location
#include <libintl.h> // For gettext, ngettext, select_plural
#include <locale.h> // For setlocale, LC_ALL
// #include <sys/loadavg.h> // For getloadavg, or sometimes in stdlib.h

// --- Dummy declarations for GNU specific functions and globals ---
// In a real GNU system, these would be provided by glibc, libintl, etc.
// For compilation, we need to provide declarations if not linking to glibc.

// From error.h in glibc
extern int error(int status, int errnum, const char *fmt, ...);

// From libintl.h
extern char *gettext(const char *msgid);
extern char *ngettext(const char *msgid1, const char *msgid2, unsigned long n);
extern unsigned long select_plural(unsigned long n);

// From stdio.h (unlocked versions, or custom)
extern int putchar_unlocked(int c);
extern int fputs_unlocked(const char *s, FILE *stream);

// From time.h (rpl_time might be a GNU replacement for time)
extern time_t rpl_time(time_t *timer);

// From sys/loadavg.h or stdlib.h
extern int getloadavg(double *loadavg, int nelem);

// Global variables typically from GNU getopt.h or custom option parsing
extern int _optind;

// Custom/application-specific functions (or GNU specific)
extern int *__errno_location(void); // From errno.h/glibc
extern void set_program_name(const char *name); // From progname.h/glibc
extern void close_stdout(void); // From close-stream.h/glibc
extern char *proper_name_lite(const char *name1, const char *name2); // From propername.h/glibc
extern void parse_gnu_standard_options_only(int argc, char *argv[], const char *package,
                                            const char *version, int flags,
                                            void (*usage_func)(int),
                                            const char *bug_report_url,
                                            const char *translator_name,
                                            const char *author_name,
                                            void *context); // From argp-parse.h/glibc
extern char *quotearg_n_style_colon(int n, int style, const char *arg); // From quotearg.h/glibc
extern char *quote(const char *arg); // From quotearg.h/glibc
extern void emit_ancillary_info(const char *program); // From ancillary.h/glibc

// Assuming fprintftime is a custom function or glibc specific
extern size_t fprintftime(FILE *stream, const char *format, const struct tm *tm, int options, int width);

// Custom/application-specific function, likely reads /var/run/utmp or similar
extern int read_utmp(const char *file, long *num_entries, void **entries_ptr, int param_2);

// Global variables used by coreutils
const char *_program_name = "uptime"; // Initialized here for compilation
const char *_Version = "1.0"; // Placeholder for _Version

// Define offsets for accessing data within a 0x48-byte utmp-like entry.
// These are inferred from the decompiler output's raw memory access patterns.
// Assuming 64-bit pointers (8 bytes).
#define UTMP_FIRST_CHAR_PTR_OFFSET 0
#define UTMP_LOGIN_TIME_OFFSET (4 * sizeof(char*)) // 32 bytes
#define UTMP_TYPE_OFFSET (7 * sizeof(char*))       // 56 bytes

// Function: print_uptime
bool print_uptime(long num_entries, void *entries_ptr) {
    long login_time_val = 0; // Stores the boot time (timestamp)
    long logged_in_users = 0;

    // Iterate through utmp-like entries to find boot time and count users
    for (long i = 0; i < num_entries; ++i) {
        // `current_entry_base` points to the start of the current 0x48-byte entry
        char *current_entry_base = (char *)entries_ptr + i * 0x48;

        // Original: `if ((**local_48 == '\0') || (*(short *)(local_48 + 7) != 7))`
        // Interpretation: Check if the first character pointed to by the first `char*` in the entry is not null,
        // AND if the 'type' field (at byte offset UTMP_TYPE_OFFSET) is 7 (USER_PROCESS).
        if (*(*(char**)current_entry_base) != '\0' && *(short*)(current_entry_base + UTMP_TYPE_OFFSET) == 7) {
            logged_in_users++;
        }

        // Original: `if (*(short *)(local_48 + 7) == 2)` and `local_70 = local_48[4];`
        // Interpretation: If the 'type' field is 2 (BOOT_TIME), store the timestamp
        // found at byte offset UTMP_LOGIN_TIME_OFFSET.
        if (*(short*)(current_entry_base + UTMP_TYPE_OFFSET) == 2) {
            login_time_val = *(long*)(current_entry_base + UTMP_LOGIN_TIME_OFFSET);
        }
    }

    bool has_error = false;

    if (login_time_val == 0) {
        error(0, *__errno_location(), gettext("No utmp entry for system boot time."));
        has_error = true;
    }

    time_t current_time = rpl_time(NULL);
    struct tm *current_tm = NULL;

    if (current_time == (time_t)-1) {
        current_tm = NULL;
    } else {
        current_tm = localtime(&current_time);
    }

    if (current_tm == NULL) {
        printf(gettext("Cannot determine current time.\n"));
        has_error = true;
    } else {
        fprintftime(stdout, gettext(" %H:%M:%S  "), current_tm, 0, 0);
    }

    // Calculate and print uptime duration
    if (current_time == (time_t)-1 || login_time_val == 0 || current_time < login_time_val) {
        printf(gettext("up ???? days ??:??,  "));
        has_error = true;
    } else {
        long uptime_seconds = current_time - login_time_val;
        long days = uptime_seconds / 86400;
        unsigned long hours = (uptime_seconds % 86400) / 3600;
        unsigned long minutes = ((uptime_seconds % 86400) % 3600) / 60;

        if (days < 1) {
            printf(gettext("up %lu:%02lu,  "), hours, minutes);
        } else {
            printf(ngettext("up %ld day, %lu:%02lu,  ", "up %ld days, %lu:%02lu,  ", select_plural(days)),
                   days, hours, minutes);
        }
    }

    // Print number of logged in users
    printf(ngettext("%ld user", "%ld users", select_plural(logged_in_users)), logged_in_users);

    // Get and print load averages
    double loadavg[3];
    int num_load_avgs = getloadavg(loadavg, 3);

    if (num_load_avgs == -1) {
        putchar_unlocked('\n');
    } else {
        if (num_load_avgs > 0) {
            printf(gettext(",  load average: %.2f"), loadavg[0]);
        }
        if (num_load_avgs > 1) {
            printf(", %.2f", loadavg[1]);
        }
        if (num_load_avgs > 2) {
            printf(", %.2f", loadavg[2]);
        }
        putchar_unlocked('\n');
    }

    return has_error;
}

// Function: uptime
void uptime(const char *utmp_file_path, int param_2) {
    long num_entries = 0;
    void *entries_ptr = NULL; // Pointer to the array of utmp entries

    // read_utmp is a custom function that populates num_entries and entries_ptr
    int read_utmp_ret_val = read_utmp(utmp_file_path, &num_entries, &entries_ptr, param_2);

    int exit_status = 0;

    if (read_utmp_ret_val != 0) {
        error(0, *__errno_location(), gettext("Cannot open or read %s"),
              quotearg_n_style_colon(0, 3, utmp_file_path));
        // Reset num_entries and entries_ptr if read_utmp failed, to prevent issues in print_uptime
        num_entries = 0;
        entries_ptr = NULL;
        exit_status = 1; // Indicate error
    }

    bool print_uptime_error = print_uptime(num_entries, entries_ptr);

    if (print_uptime_error) {
        exit_status = 1;
    }

    // Free memory allocated by read_utmp if necessary
    if (entries_ptr != NULL) {
        free(entries_ptr);
    }

    exit(exit_status);
}

// Function: usage
void usage(int status) {
    if (status == 0) {
        printf(gettext("Usage: %s [OPTION]...\n"), _program_name);
        printf(gettext("Show how long the system has been running.\n\n"));
        printf(gettext("      --help     display this help and exit\n"));
        printf(gettext("      --version  output version information and exit\n"));
        fputs_unlocked(gettext("\nReport bugs to: bug-coreutils@gnu.org\n"), stdout);
        fputs_unlocked(gettext("GNU coreutils online help: <https://www.gnu.org/software/coreutils/>\n"), stdout);
        emit_ancillary_info("uptime");
    } else {
        fprintf(stderr, gettext("Try '%s --help' for more information.\n"), _program_name);
    }
    exit(status);
}

// Function: main
int main(int argc, char *argv[]) {
    set_program_name(argv[0]);
    setlocale(LC_ALL, "");
    bindtextdomain("coreutils", "/usr/share/locale");
    textdomain("coreutils");
    atexit(close_stdout);

    // These proper_name_lite calls are likely for authors/translators info
    const char *author_name = proper_name_lite("David MacKenzie", "David MacKenzie");
    const char *translator_name = proper_name_lite("GNU", "GNU");
    const char *bug_report_url = proper_name_lite("bug-coreutils@gnu.org", "bug-coreutils@gnu.org");

    // parse_gnu_standard_options_only handles standard GNU options like --help and --version
    parse_gnu_standard_options_only(argc, argv, "uptime",
                                    _Version, // Use the global _Version
                                    1, usage,
                                    bug_report_url, translator_name, author_name, NULL);

    // Handle command-line arguments (utmp file path)
    if (argc == _optind) { // No non-option arguments, use default utmp file
        uptime("/var/run/utmp", 1); // LAB_00100eb1 corresponds to "/var/run/utmp"
    } else if (argc - _optind == 1) { // One non-option argument, use it as utmp file path
        uptime(argv[_optind], 0);
    } else { // More than one non-option argument or other error
        error(0, 0, gettext("extra operand %s"), quote(argv[_optind + 1]));
        usage(1); // Call usage with error status
    }

    return 0; // exit() is called in uptime and usage, so this return might not be reached.
}