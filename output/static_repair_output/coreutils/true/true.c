#include <stdio.h>    // For printf, fputs, FILE, stdout, fflush
#include <stdlib.h>   // For exit, atexit
#include <locale.h>   // For setlocale, LC_ALL
#include <libintl.h>  // For gettext, bindtextdomain, textdomain
#include <string.h>   // For strcmp

// Global variables (simulated from the original snippet's context)
char *_program_name = "my_program";
const char *_Version = "1.0";
const char DAT_00100761[] = "my_program"; // Represents a global string literal or data address

// Dummy implementations for external functions to make the code compilable
void set_program_name(const char *name) {
    _program_name = (char *)name;
}

int streq(const char *s1, const char *s2) {
    return strcmp(s1, s2) == 0;
}

void close_stdout(void) {
    fflush(stdout);
    // In a real application, this would also check for write errors and close stdout.
}

const char *proper_name_lite(const char *a, const char *b) {
    (void)b; // Unused parameter
    return a; // Simple dummy implementation
}

void version_etc(FILE *stream, const char *package_name, const char *program_name,
                 const char *version, const char *authors, int flags) {
    (void)flags; // Unused parameter
    fprintf(stream, "%s (%s) %s\n", program_name, package_name, version);
    fprintf(stream, "Copyright (C) 2023 Free Software Foundation, Inc.\n");
    fprintf(stream, "License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\n");
    fprintf(stream, "This is free software: you are free to change and redistribute it.\n");
    fprintf(stream, "There is NO WARRANTY, to the extent permitted by law.\n\n");
    fprintf(stream, "Written by %s.\n", authors);
}

void emit_ancillary_info(const char *program) {
    printf("Report bugs to: <bug-coreutils@gnu.org>\n");
    printf("GNU coreutils home page: <https://www.gnu.org/software/coreutils/>\n");
    printf("General help using GNU software: <https://www.gnu.org/get_help/>\n");
    printf("For complete documentation, run: info coreutils '%s invocation'\n", program);
}

// Function: usage
void usage(int param_1) {
  printf(gettext("Usage: %s [ignored command line arguments]\n  or:  %s OPTION\n"), _program_name, _program_name);
  printf("%s\n\n", gettext("Exit with a status code indicating success."));
  fputs(gettext("      --help        display this help and exit\n"), stdout);
  fputs(gettext("      --version     output version information and exit\n"), stdout);
  printf(gettext(
                          "\nYour shell may have its own version of %s, which usually supersedes\nthe version described here.  Please refer to your shell\'s documentation\nfor details about the options it supports.\n"
                          ), DAT_00100761);
  emit_ancillary_info(DAT_00100761);
  exit(param_1);
}

// Function: main
int main(int argc, char **argv) {
  if (argc == 2) {
    set_program_name(argv[0]);
    setlocale(LC_ALL, "");
    bindtextdomain("coreutils","/usr/local/share/locale");
    textdomain("coreutils");
    atexit(close_stdout);

    if (streq(argv[1],"--help")) {
      usage(0);
    }
    if (streq(argv[1],"--version")) {
      version_etc(stdout, DAT_00100761, "GNU coreutils", _Version,
                  proper_name_lite("Jim Meyering","Jim Meyering"), 0);
    }
  }
  return 0;
}