#include <stdio.h>    // For FILE, printf, fputs_unlocked, fprintf, putchar_unlocked, stdout, stderr
#include <stdlib.h>   // For exit, free
#include <stdbool.h>  // For bool
#include <string.h>   // For string manipulation (though path_prefix is custom)
#include <sys/stat.h> // For stat, struct stat, S_ISDIR
#include <errno.h>    // For __errno_location, ENOENT
#include <locale.h>   // For setlocale
#include <getopt.h>   // For getopt_long, struct option, optind, optarg
#include <unistd.h>   // For fputs_unlocked, putchar_unlocked (often available via stdio.h on Linux)

// --- Stub global variables ---
// These are assumed to be defined externally, likely by GNU coreutils' gnulib
char *_program_name = "realpath"; // Default program name, will be set by set_program_name
FILE *_stdout = NULL; // Initialized in main to stdout
FILE *_stderr = NULL; // Initialized in main to stderr

// Flags and options (initialized to typical defaults)
static bool logical = false;
static char *can_relative_to = NULL;
static char *can_relative_base = NULL;
static bool use_nuls = false;
static bool verbose = true; // Default is verbose, -q sets to false

// --- Stub function declarations ---
// These functions are assumed to be external, likely from GNU coreutils' gnulib.
// Minimal declarations are provided to allow compilation.
extern char *gettext(const char *msgid);
extern void emit_ancillary_info(const char *program);
extern void *canonicalize_filename_mode(const char *name, unsigned int flags);
extern int *__errno_location(void);
extern void error(int status, int errnum, const char *format, ...);
extern char *quotearg_style(int style, const char *arg);
extern char *quotearg_n_style_colon(int n, int style, const char *arg);
extern void set_program_name(const char *name);
extern char *setlocale(int category, const char *locale);
extern char *bindtextdomain(const char *domainname, const char *dirname);
extern char *textdomain(const char *domainname);
extern void close_stdout(void);
extern void version_etc(FILE *stream, const char *package, const char *version, const char *bug_report, const char *date, ...);
extern const char *proper_name_lite(const char *name, const char *date);
extern bool relpath(const char *path, const char *base, bool relative_to_cwd, bool verbose_flag);
extern void __stack_chk_fail(void); // Compiler intrinsic, but declared for completeness if explicitly used

// Example values for gnulib constants/globals
const char _Version[] = "8.28";
const char DAT_001011dd[] = "2016-09-12"; // Example date for proper_name_lite
const char DAT_001011b9[] = "%s: No such file or directory"; // Example error format string

// longopts array for getopt_long
static const struct option longopts[] = {
    {"canonicalize", no_argument, NULL, 'E'},
    {"canonicalize-existing", no_argument, NULL, 'e'},
    {"canonicalize-missing", no_argument, NULL, 'm'},
    {"logical", no_argument, NULL, 'L'},
    {"physical", no_argument, NULL, 'P'},
    {"quiet", no_argument, NULL, 'q'},
    {"strip", no_argument, NULL, 's'},
    {"no-symlinks", no_argument, NULL, 's'},
    {"zero", no_argument, NULL, 'z'},
    {"relative-to", required_argument, NULL, 0x80},
    {"relative-base", required_argument, NULL, 0x81},
    {"help", no_argument, NULL, 0x82},
    {"version", no_argument, NULL, 0x83},
    {NULL, 0, NULL, 0}
};

// Function: usage
void usage(int status) {
  if (status == 0) {
    printf(gettext("Usage: %s [OPTION]... FILE...\n"), _program_name);
    fputs_unlocked(gettext("Print the resolved absolute file name.\n"), _stdout);
    fputs_unlocked(gettext(
                            "  -E, --canonicalize           all but the last component must exist (default)\n  -e, --canonicalize-existing  all components of the path must exist\n  -m, --canonicalize-missing   no path components need exist or be a directory\n  -L, --logical                resolve \'..\' components before symlinks\n  -P, --physical               resolve symlinks as encountered (default)\n  -q, --quiet                  suppress most error messages\n      --relative-to=DIR        print the resolved path relative to DIR\n      --relative-base=DIR      print absolute paths unless paths below DIR\n  -s, --strip, --no-symlinks   don\'t expand symlinks\n  -z, --zero                   end each output line with NUL, not newline\n"
                            ), _stdout);
    fputs_unlocked(gettext("      --help        display this help and exit\n"), _stdout);
    fputs_unlocked(gettext("      --version     output version information and exit\n"), _stdout);
    emit_ancillary_info("realpath");
  } else {
    fprintf(_stderr, gettext("Try \'%s --help\' for more information.\n"), _program_name);
  }
  exit(status);
}

// Function: realpath_canon
void *realpath_canon(const char *path, unsigned int flags) {
  void *resolved_path = canonicalize_filename_mode(path, flags);
  if (logical && resolved_path) {
    void *physical_path = canonicalize_filename_mode(resolved_path, flags & ~4U); // 0xfffffffb is ~4U
    free(resolved_path);
    resolved_path = physical_path;
  }
  return resolved_path;
}

// Function: path_prefix
bool path_prefix(const char *prefix_path, const char *full_path) {
  // This function's parameter usage (param_1 + 1) suggests a custom string format.
  // Assuming 'prefix_path' and 'full_path' are pointers where the actual string starts at offset +1.
  const char *p_prefix = prefix_path + 1;
  const char *p_full = full_path + 1;

  if (*p_prefix == '\0') {
    return *p_full != '/';
  }
  // If prefix_path represents "/" (e.g., prefix_path points to something like "\0/\0")
  if ((*p_prefix == '/') && (*(prefix_path + 2) == '\0')) {
    return *p_full == '/';
  }
  
  while (*p_prefix != '\0' && *p_full != '\0' && *p_prefix == *p_full) {
    p_prefix++;
    p_full++;
  }
  return (*p_prefix == '\0') && (*p_full == '/' || *p_full == '\0');
}

// Function: isdir
bool isdir(const char *path) {
  struct stat st;
  // The original code calls error(1, ...) which exits on stat failure.
  if (stat(path, &st) != 0) {
    error(1, *__errno_location(), gettext("cannot stat %s"), quotearg_style(4, path));
  }
  // The original CONCAT31 is a decompiler artifact. It effectively checks S_ISDIR.
  return S_ISDIR(st.st_mode);
}

// Function: process_path
bool process_path(const char *path, unsigned int flags) {
  char *resolved_path = realpath_canon(path, flags);
  if (resolved_path) {
    if (!can_relative_to ||
        (can_relative_base && !path_prefix(can_relative_base, resolved_path)) ||
        (can_relative_to && !relpath(resolved_path, can_relative_to, false, false))) {
      fputs_unlocked(resolved_path, _stdout);
    }
    putchar_unlocked(use_nuls ? '\0' : '\n');
    free(resolved_path);
    return true;
  }

  if (verbose) {
    error(0, *__errno_location(), DAT_001011b9, quotearg_n_style_colon(0, 3, path));
  }
  return false;
}

// Function: main
int main(int argc, char *const *argv) {
  _stdout = stdout; // Initialize global _stdout
  _stderr = stderr; // Initialize global _stderr

  bool all_paths_processed_successfully = true;
  unsigned int canonicalize_flags = 1; // Corresponds to -E, --canonicalize (default)
  char *relative_to_dir_arg = NULL;
  char *relative_base_dir_arg = NULL;

  char *allocated_relative_to_path = NULL; // Stores the malloc'd pointer for --relative-to
  char *allocated_relative_base_path = NULL; // Stores the malloc'd pointer for --relative-base

  set_program_name(argv[0]);
  setlocale(LC_ALL, ""); // Use LC_ALL for category 6 (LC_MESSAGES) and others
  bindtextdomain("coreutils", "/usr/local/share/locale");
  textdomain("coreutils");
  atexit(close_stdout);

  while (true) {
    int opt = getopt_long(argc, argv, "EeLmPqsz", longopts, NULL);

    if (opt == -1) { // End of options
      break;
    }

    switch (opt) {
      case 'E': // --canonicalize
        canonicalize_flags = (canonicalize_flags & ~3U) | 1; // Clear bits 0,1, set bit 0
        break;
      case 'e': // --canonicalize-existing
        canonicalize_flags &= ~3U; // Clear bits 0,1
        break;
      case 'm': // --canonicalize-missing
        canonicalize_flags = (canonicalize_flags & ~3U) | 2; // Clear bits 0,1, set bit 1
        break;
      case 'L': // --logical
        canonicalize_flags |= 4;
        logical = true;
        break;
      case 'P': // --physical
        canonicalize_flags &= ~4U;
        logical = false;
        break;
      case 'q': // --quiet
        verbose = false;
        break;
      case 's': // --strip, --no-symlinks
        canonicalize_flags |= 4; // This flag is used to disable symlink expansion.
                                 // Original code sets logical = 0, which physical does.
                                 // "don't expand symlinks" usually means physical, not logical.
        logical = false;
        break;
      case 'z': // --zero
        use_nuls = true;
        break;
      case 0x80: // --relative-to
        relative_to_dir_arg = optarg;
        break;
      case 0x81: // --relative-base
        relative_base_dir_arg = optarg;
        break;
      case 0x82: // --help
        usage(0); // usage() calls exit(0)
      case 0x83: // --version
        version_etc(_stdout, "realpath", "GNU coreutils", _Version, proper_name_lite("Padraig Brady", DAT_001011dd), 0);
        exit(0); // version_etc does not exit, but the original code implies exit(0) here.
      default: // Unknown option or error
        usage(1); // usage() calls exit(1)
    }
  }

  if (argc <= optind) {
    error(0, 0, gettext("missing operand"));
    usage(1); // usage() calls exit(1)
  }

  // Post-option processing for --relative-to and --relative-base
  bool canonicalize_existing_for_dirs = (canonicalize_flags & 3) == 0; // if flags is 0 (00), i.e. -e

  // Step 1: Handle implicit --relative-to from --relative-base if --relative-to is not explicitly set
  if (relative_base_dir_arg && !relative_to_dir_arg) {
    relative_to_dir_arg = relative_base_dir_arg;
  }

  // Step 2: Resolve --relative-to path
  if (relative_to_dir_arg) {
    allocated_relative_to_path = realpath_canon(relative_to_dir_arg, canonicalize_flags);
    if (!allocated_relative_to_path) {
      error(1, *__errno_location(), DAT_001011b9, quotearg_n_style_colon(0, 3, relative_to_dir_arg));
    }
    if (canonicalize_existing_for_dirs && !isdir(allocated_relative_to_path)) {
      error(1, ENOENT, DAT_001011b9, quotearg_n_style_colon(0, 3, relative_to_dir_arg));
    }
    can_relative_to = allocated_relative_to_path; // Update global
  }

  // Step 3: Resolve --relative-base path
  if (relative_base_dir_arg) {
    if (relative_base_dir_arg == relative_to_dir_arg) { // Pointers match, implies same command-line argument
      allocated_relative_base_path = allocated_relative_to_path; // Point to same memory
      can_relative_base = can_relative_to; // Update global
    } else {
      allocated_relative_base_path = realpath_canon(relative_base_dir_arg, canonicalize_flags);
      if (!allocated_relative_base_path) {
        error(1, *__errno_location(), DAT_001011b9, quotearg_n_style_colon(0, 3, relative_base_dir_arg));
      }
      if (canonicalize_existing_for_dirs && !isdir(allocated_relative_base_path)) {
        error(1, ENOENT, DAT_001011b9, quotearg_n_style_colon(0, 3, relative_base_dir_arg));
      }

      if (can_relative_to && !path_prefix(allocated_relative_base_path, can_relative_to)) {
        // If relative-to is specified and base is NOT a prefix of relative-to
        free(allocated_relative_base_path); // Free this specific allocation
        allocated_relative_base_path = allocated_relative_to_path; // Base now points to 'to's memory
        can_relative_base = can_relative_to; // Update global
        can_relative_to = NULL; // Original logic sets 'to' global to NULL
      } else {
        can_relative_base = allocated_relative_base_path; // Update global
      }
    }
  } else if (relative_to_dir_arg) { // If only --relative-to was specified (explicitly, or implicitly from base)
      allocated_relative_base_path = allocated_relative_to_path; // Base points to 'to's memory
      can_relative_base = can_relative_to; // Update global
  }

  // Process remaining command-line arguments (files)
  for (int i = optind; i < argc; i++) {
    all_paths_processed_successfully &= process_path(argv[i], canonicalize_flags);
  }

  // Clean up allocated paths
  if (allocated_relative_to_path) {
      free(allocated_relative_to_path);
  }
  // Only free allocated_relative_base_path if it's distinct from allocated_relative_to_path
  if (allocated_relative_base_path && allocated_relative_base_path != allocated_relative_to_path) {
      free(allocated_relative_base_path);
  }

  return all_paths_processed_successfully ? 0 : 1;
}