#include <stdio.h>   // For FILE, fputs_unlocked, fwrite_unlocked, vfprintf, fputc_unlocked
#include <stdarg.h>  // For va_list, va_start, va_end
#include <string.h>  // For strlen, if using it instead of sizeof for separator

// Declare program_name as a global variable. It's usually set by the C runtime
// or by the main function's argv[0].
char *program_name;

// DAT_00100126 from the original snippet is likely the string ": "
static const char program_name_separator[] = ": ";

// Function: prog_fprintf
void prog_fprintf(FILE *stream, const char *format, ...) {
    va_list ap;
    va_start(ap, format);

    fputs_unlocked(program_name, stream);
    // fwrite_unlocked writes specific number of bytes. sizeof(program_name_separator) - 1
    // gives the length of the string ": " (2 bytes), excluding the null terminator.
    fwrite_unlocked(program_name_separator, 1, sizeof(program_name_separator) - 1, stream);

    vfprintf(stream, format, ap);

    fputc_unlocked('\n', stream);

    va_end(ap);
    // The stack canary check and related variables (in_FS_OFFSET, local_c0, __stack_chk_fail)
    // are compiler-inserted security features and not part of standard C source code.
    // They are automatically handled by the compiler's prologue/epilogue when
    // stack protection is enabled (e.g., with -fstack-protector).
    // Explicitly writing them as in the disassembly is non-portable and unnecessary.
}

// Main function to demonstrate usage
int main(int argc, char *argv[]) {
    // Initialize program_name with argv[0]
    if (argc > 0) {
        program_name = argv[0];
    } else {
        program_name = "unknown_program"; // Fallback if argv is empty
    }

    // Example usage of prog_fprintf
    prog_fprintf(stderr, "This is a test error message with an integer: %d and a string: %s", 123, "hello");
    prog_fprintf(stdout, "Another message to stdout: %f", 3.14159);
    prog_fprintf(stdout, "A simple message.");

    return 0;
}