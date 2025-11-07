#include <stdio.h>    // For FILE, printf, fprintf, fputs_unlocked, sprintf, stdout, stderr
#include <stdlib.h>   // For exit, atexit
#include <string.h>   // For strlen
#include <stdbool.h>  // For bool
#include <libintl.h>  // For gettext, bindtextdomain, textdomain
#include <locale.h>   // For setlocale
#include <stddef.h>   // For size_t
#include <stdint.h>   // For uintmax_t, intmax_t (implied by %ju, %jd)

// --- External declarations (mimicking GNU coreutils environment) ---
// These would typically be provided by specific coreutils headers.
// Assuming _program_name is a globally accessible char pointer.
extern char *_program_name;
extern void set_program_name(char *name);
extern void close_stdout(void);
extern char *proper_name_lite(const char *name, const char *contact_info);
extern void parse_gnu_standard_options_only(int argc, char **argv,
                                            const char *program_name,
                                            const char *package_name,
                                            const char *version_string,
                                            int flags,
                                            void (*usage_func)(int),
                                            const char *authors,
                                            int bug_report_url_idx);
extern void emit_ancillary_info(const char *program_name);

// Global data from original snippet (assuming these are constants or initialized data)
// DAT_0010148b is likely a string literal for contact info.
static const char DAT_0010148b[] = "bug-report@example.com"; // Placeholder, real value unknown

// Assuming these are actual float/double values for limits.
static const float DAT_00101834 = 1.17549435e-38F; // Example FLT_MIN
static const float DAT_00101838 = 3.40282347e+38F; // Example FLT_MAX
static const double DAT_00101840 = 2.2250738585072014e-308; // Example DBL_MIN
static const double DAT_00101848 = 1.7976931348623157e+308; // Example DBL_MAX

// External print functions for float/double/long double
extern void print_FLT(float val);
extern void print_DBL(double val);
// The original code calls print_LDBL() twice without arguments.
// This is unusual for printing specific limit values. Assuming it's meant
// to print internal fixed long double limits.
extern void print_LDBL(void);

// Function: usage
void usage(int exit_code) {
    if (exit_code == 0) {
        printf(gettext("Usage: %s\n"), _program_name);
        fputs_unlocked(gettext("Output platform dependent limits in a format useful for shell scripts.\n\n"), stdout);
        fputs_unlocked(gettext("      --help        display this help and exit\n"), stdout);
        fputs_unlocked(gettext("      --version     output version information and exit\n"), stdout);
        emit_ancillary_info("getlimits");
    } else {
        fprintf(stderr, gettext("Try \'%s --help\' for more information.\n"), _program_name);
    }
    exit(exit_code);
}

// Function: decimal_absval_add_one
// buf_start points to the beginning of a char buffer.
// The numeric string (e.g., "127" or "-128") is expected to start at buf_start + 1.
// buf_start[0] is used as scratch space for a potential carry-over '1' or a '-' sign.
// Returns a pointer to the start of the modified string (which might be buf_start or buf_start + 1).
char *decimal_absval_add_one(char *buf_start) {
    bool is_negative = (buf_start[1] == '-'); // Check char at index 1 for sign
    char *digits_start = buf_start + 1 + (is_negative ? 1 : 0); // Start of actual digits
    char *current_digit_ptr = digits_start + strlen(digits_start); // Points to null terminator

    buf_start[0] = '0'; // Prepare for carry, this byte will hold '1' if e.g., "99" becomes "100"

    // Increment the number represented by digits_start
    do {
        current_digit_ptr--;
        if (*current_digit_ptr < '9') {
            *current_digit_ptr += 1;
            break; // No more carry
        }
        *current_digit_ptr = '0'; // Carry over
    } while (current_digit_ptr >= digits_start);

    // If there was a carry leading to an extra digit (e.g., "99" -> "100")
    if (current_digit_ptr < digits_start) {
        digits_start = current_digit_ptr; // New start for digits (e.g., '1' in "100")
        *digits_start = '1'; // Place the '1'
    }

    // Reapply negative sign if needed
    if (is_negative) {
        digits_start--;
        *digits_start = '-';
    }

    return digits_start; // Return pointer to the start of the (possibly modified) number string
}

// Function: main
int main(int argc, char *argv[]) {
    // The original `local_28` (1 byte) and `acStack_27` (23 bytes)
    // are combined into one buffer `buf[24]`.
    // `buf[0]` acts as `local_28`, `buf[1..23]` acts as `acStack_27`.
    // This allows `decimal_absval_add_one` to use `buf[0]` for carry/sign.
    char buf[24]; 

    set_program_name(argv[0]);
    setlocale(LC_ALL, ""); // LC_ALL is equivalent to 6
    bindtextdomain("coreutils", "/usr/local/share/locale");
    textdomain("coreutils");
    atexit(close_stdout);

    const char *authors_info = proper_name_lite("Padraig Brady", DAT_0010148b);
    parse_gnu_standard_options_only(argc, argv, "getlimits", "GNU coreutils",
                                    "9.8.77-8db63", 1, usage, authors_info, 0);

    // Macro to reduce repetition for MAX/OFLOW pairs
    #define PRINT_MAX_OFLOW(name, type_char, val) \
        sprintf(buf + 1, "%" type_char, (uintmax_t)val); \
        printf(#name "=%s\n", buf + 1); \
        printf(#name "_OFLOW=%s\n", decimal_absval_add_one(buf));

    // Macro to reduce repetition for MIN/UFLOW pairs
    // Note: The original values like 0xffffffffffffff80 are two's complement for negative numbers.
    #define PRINT_MIN_UFLOW(name, type_char, val) \
        sprintf(buf + 1, "%" type_char, (intmax_t)val); \
        printf(#name "=%s\n", buf + 1); \
        printf(#name "_UFLOW=%s\n", decimal_absval_add_one(buf));

    PRINT_MAX_OFLOW(CHAR_MAX, "ju", (unsigned long long)0x7f);
    PRINT_MIN_UFLOW(CHAR_MIN, "jd", (long long)0xffffffffffffff80);
    PRINT_MAX_OFLOW(SCHAR_MAX, "ju", (unsigned long long)0x7f);
    PRINT_MIN_UFLOW(SCHAR_MIN, "jd", (long long)0xffffffffffffff80);
    PRINT_MAX_OFLOW(UCHAR_MAX, "ju", (unsigned long long)0xff);

    PRINT_MAX_OFLOW(SHRT_MAX, "ju", (unsigned long long)0x7fff);
    PRINT_MIN_UFLOW(SHRT_MIN, "jd", (long long)0xffffffffffff8000);

    PRINT_MAX_OFLOW(INT_MAX, "ju", (unsigned long long)0x7fffffff);
    PRINT_MIN_UFLOW(INT_MIN, "jd", (long long)0xffffffff80000000);
    PRINT_MAX_OFLOW(UINT_MAX, "ju", (unsigned long long)0xffffffff);

    PRINT_MAX_OFLOW(LONG_MAX, "ju", (unsigned long long)0x7fffffffffffffff);
    PRINT_MIN_UFLOW(LONG_MIN, "jd", (long long)0x8000000000000000);
    PRINT_MAX_OFLOW(ULONG_MAX, "ju", (unsigned long long)0xffffffffffffffff);

    PRINT_MAX_OFLOW(SIZE_MAX, "ju", (unsigned long long)0xffffffffffffffff);
    PRINT_MAX_OFLOW(SSIZE_MAX, "ju", (unsigned long long)0x7fffffffffffffff);
    PRINT_MIN_UFLOW(SSIZE_MIN, "jd", (long long)0x8000000000000000);

    PRINT_MAX_OFLOW(TIME_T_MAX, "ju", (unsigned long long)0x7fffffffffffffff);
    PRINT_MIN_UFLOW(TIME_T_MIN, "jd", (long long)0x8000000000000000);

    PRINT_MAX_OFLOW(UID_T_MAX, "ju", (unsigned long long)0xffffffff);
    PRINT_MAX_OFLOW(GID_T_MAX, "ju", (unsigned long long)0xffffffff);

    PRINT_MAX_OFLOW(PID_T_MAX, "ju", (unsigned long long)0x7fffffff);
    PRINT_MIN_UFLOW(PID_T_MIN, "jd", (long long)0xffffffff80000000);

    PRINT_MAX_OFLOW(OFF_T_MAX, "ju", (unsigned long long)0x7fffffffffffffff);
    PRINT_MIN_UFLOW(OFF_T_MIN, "jd", (long long)0x8000000000000000);

    PRINT_MAX_OFLOW(INTMAX_MAX, "ju", (unsigned long long)0x7fffffffffffffff);
    PRINT_MIN_UFLOW(INTMAX_MIN, "jd", (long long)0x8000000000000000);
    PRINT_MAX_OFLOW(UINTMAX_MAX, "ju", (unsigned long long)0xffffffffffffffff);

    printf("FLT_MIN=");
    print_FLT(DAT_00101834);
    printf("FLT_MAX=");
    print_FLT(DAT_00101838);
    printf("DBL_MIN=");
    print_DBL(DAT_00101840);
    printf("DBL_MAX=");
    print_DBL(DAT_00101848);
    printf("LDBL_MIN=");
    print_LDBL();
    printf("LDBL_MAX=");
    print_LDBL();

    printf("IO_BUFSIZE=%ju\n", (uintmax_t)0x40000);

    return 0;
}