#include <stdio.h>    // printf, fputs, fprintf, stdout, stderr, FILE, ferror, fclose
#include <stdlib.h>   // exit, atexit, EXIT_FAILURE, EXIT_SUCCESS
#include <unistd.h>   // getopt_long, optind, sync, close, fsync, fdatasync, syncfs
#include <stdbool.h>  // bool
#include <string.h>   // strerror
#include <errno.h>    // errno
#include <fcntl.h>    // open, fcntl, O_RDONLY, O_WRONLY, O_NONBLOCK, F_GETFL, F_SETFL
#include <locale.h>   // setlocale, LC_ALL
#include <libintl.h>  // bindtextdomain, textdomain, gettext (if available, otherwise dummy)
#include <getopt.h>   // getopt_long, struct option
#include <stdarg.h>   // va_list, va_start, va_end

// --- Coreutils specific dummies/replacements for self-contained compilation ---

// Global program name
static const char *program_name;

// Dummy gettext if libintl.h is not fully linked
#ifndef HAVE_LIBINTL_H
#define gettext(s) (s)
#endif

// Simplified error function (similar to coreutils error)
static void error(int status, int errnum, const char *format, ...) {
    va_list args;
    fprintf(stderr, "%s: ", program_name ? program_name : "unknown_program");
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    if (errnum != 0) {
        fprintf(stderr, ": %s", strerror(errnum));
    }
    fputc('\n', stderr);
    if (status != 0) {
        exit(status);
    }
}

// Dummy quotearg_style (returns the argument directly for simplicity)
static const char *quotearg_style(int style, const char *arg) {
    return arg;
}

// rpl_fcntl is likely a wrapper for fcntl, use fcntl directly
#define rpl_fcntl fcntl

// set_program_name
static void set_program_name(const char *name) {
    program_name = name;
}

// close_stdout (simplified version of coreutils close_stdout)
static void close_stdout(void) {
    if (ferror(stdout) || fclose(stdout) != 0) {
        error(EXIT_FAILURE, errno, gettext("write error"));
    }
}

// proper_name_lite (returns the first name for simplicity)
static const char *proper_name_lite(const char *first, const char *last) {
    return first;
}

// Version info placeholder
static const char _Version[] = "8.32"; // Example version string

// This string is used by version_etc and emit_ancillary_info
static const char *version_info_string =
    "sync (GNU coreutils) %s\n"
    "Copyright (C) 2020 Free Software Foundation, Inc.\n"
    "License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\n"
    "This is free software: you are free to change and redistribute it.\n"
    "There is NO WARRANTY, to the extent permitted by law.\n\n"
    "Written by Jim Meyering and Giuseppe Scrivano.";

// emit_ancillary_info (dummy - prints the supplied string)
static void emit_ancillary_info(const char *info_str) {
    fputs(info_str, stdout);
}

// version_etc (dummy, simplified)
static void version_etc(FILE *stream, const char *program_info_format,
                        const char *package, const char *version,
                        const char *author1, const char *author2, ...) {
    fprintf(stream, program_info_format, version);
    fputc('\n', stream);
    fprintf(stream, gettext("Written by %s and %s.\n"), author1, author2);
}

// getopt_long options
static const char short_options[] = "df"; // d and f are short options

enum {
    GETOPT_HELP_OPTION = 1,
    GETOPT_VERSION_OPTION
};

static struct option const long_options[] = {
    {"data", no_argument, NULL, 'd'},
    {"file-system", no_argument, NULL, 'f'},
    {"help", no_argument, NULL, GETOPT_HELP_OPTION},
    {"version", no_argument, NULL, GETOPT_VERSION_OPTION},
    {NULL, 0, NULL, 0}
};

// Function: usage
void usage(int status) { // param_1 -> status
    if (status == 0) {
        printf(gettext("Usage: %s [OPTION] [FILE]...\n"), program_name);
        fputs(gettext(
                "Synchronize cached writes to persistent storage\n\nIf one or more files are specified, sync only them,\nor their containing file systems.\n\n"
            ), stdout);
        fputs(gettext("  -d, --data             sync only file data, no unneeded metadata\n"), stdout);
        fputs(gettext(
                "  -f, --file-system      sync the file systems that contain the files\n"
            ), stdout);
        fputs(gettext("      --help        display this help and exit\n"), stdout);
        fputs(gettext("      --version     output version information and exit\n"), stdout);
        emit_ancillary_info(version_info_string);
    } else {
        fprintf(stderr, gettext("Try \'%s --help\' for more information.\n"), program_name);
    }
    exit(status);
}

// Function: sync_arg
// sync_mode: 0=fsync, 1=fdatasync, 2=syncfs
char sync_arg(unsigned int sync_mode, char *filepath) { // param_1 -> sync_mode, param_2 -> filepath
    bool success = true;
    int fd = -1;
    int saved_errno = 0;

    // Attempt to open the file for reading, with O_NONBLOCK.
    // O_NONBLOCK is often used to prevent blocking on special files (e.g., FIFOs).
    fd = open(filepath, O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        saved_errno = errno;
        // If the first attempt failed, try again without O_NONBLOCK.
        fd = open(filepath, O_RDONLY);
    }

    if (fd < 0) {
        error(0, saved_errno, gettext("error opening %s"), quotearg_style(4, filepath));
        return false;
    }

    // Get current file flags
    int flags = fcntl(fd, F_GETFL);
    if (flags == -1) {
        error(0, errno, gettext("couldn\'t get file flags for %s"), quotearg_style(4, filepath));
        success = false;
    } else {
        // If O_NONBLOCK was set, attempt to clear it.
        // This is important because fsync/fdatasync might behave unexpectedly with O_NONBLOCK.
        if ((flags & O_NONBLOCK) && (rpl_fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) == -1)) {
            error(0, errno, gettext("couldn\'t reset non-blocking mode for %s"), quotearg_style(4, filepath));
            success = false;
        }
    }

    if (success) {
        int sync_result = -1;
        switch (sync_mode) {
            case 0: // fsync
                sync_result = fsync(fd);
                break;
            case 1: // fdatasync
                sync_result = fdatasync(fd);
                break;
            case 2: // syncfs
                sync_result = syncfs(fd);
                break;
            // Case 3 (global sync()) is handled in main, not here.
        }

        if (sync_result < 0) {
            error(0, errno, gettext("error syncing %s"), quotearg_style(4, filepath));
            success = false;
        }
    }

    if (close(fd) < 0) {
        error(0, errno, gettext("failed to close %s"), quotearg_style(4, filepath));
        success = false;
    }
    return success;
}

// Function: main
int main(int argc, char *argv[]) { // param_1 -> argc, param_2 -> argv
    bool data_only = false;
    bool file_system_only = false;
    bool overall_success = true;
    int sync_operation_mode; // 0: fsync, 1: fdatasync, 2: syncfs, 3: global sync()

    set_program_name(argv[0]);
    setlocale(LC_ALL, ""); // LC_ALL (6)
    bindtextdomain("coreutils", "/usr/local/share/locale");
    textdomain("coreutils");
    atexit(close_stdout);

    int opt;
    while ((opt = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
        switch (opt) {
            case 'd':
                data_only = true;
                break;
            case 'f':
                file_system_only = true;
                break;
            case GETOPT_HELP_OPTION:
                usage(EXIT_SUCCESS);
                // usage() calls exit, so no further code here is reachable
            case GETOPT_VERSION_OPTION:
                version_etc(stdout, version_info_string, "GNU coreutils", _Version,
                            proper_name_lite("Jim Meyering", "Jim Meyering"),
                            proper_name_lite("Giuseppe Scrivano", "Giuseppe Scrivano"), NULL);
                exit(EXIT_SUCCESS);
            case '?': // getopt_long returns '?' for unknown option
            default: // Other unknown options or error
                usage(EXIT_FAILURE);
        }
    }

    // Post-option processing logic
    bool has_arguments = (optind < argc);

    if (data_only && file_system_only) {
        error(EXIT_FAILURE, 0, gettext("cannot specify both --data and --file-system"));
    }
    if (!has_arguments && data_only) {
        error(EXIT_FAILURE, 0, gettext("--data needs at least one argument"));
    }

    if (has_arguments) {
        if (file_system_only) {
            sync_operation_mode = 2; // syncfs
        } else if (data_only) {
            sync_operation_mode = 1; // fdatasync
        } else {
            sync_operation_mode = 0; // fsync
        }
    } else {
        sync_operation_mode = 3; // global sync()
    }

    if (sync_operation_mode == 3) {
        sync();
    } else {
        for (; optind < argc; optind++) {
            if (!sync_arg(sync_operation_mode, argv[optind])) {
                overall_success = false;
            }
        }
    }

    return overall_success ? EXIT_SUCCESS : EXIT_FAILURE;
}