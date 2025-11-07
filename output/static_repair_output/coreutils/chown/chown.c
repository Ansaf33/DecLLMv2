#include <stdarg.h> // For va_list, va_start, va_end for dummy error function
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // For uid_t, gid_t, getuid, getgid
#include <sys/types.h> // For uid_t, gid_t
#include <sys/stat.h> // For stat, struct stat
#include <locale.h> // For setlocale
#include <libintl.h> // For gettext, bindtextdomain, textdomain
#include <errno.h> // For __errno_location
#include <getopt.h> // For getopt_long, struct option, optarg, optind
#include <limits.h> // For CHAR_MAX

// --- External variable declarations (guesses based on usage) ---
// These global variables are typically defined elsewhere in coreutils.
// For compilation, we provide dummy definitions.
const char *_program_name = "chown"; // Initialized by set_program_name
int _chown_mode = 1; // 1 for chown, 2 for chgrp (e.g., in `chgrp` binary this would be 2)
FILE *_stdout = NULL; // Will be initialized to stdout in main, but declared here for extern usage.
FILE *_stderr = NULL; // Will be initialized to stderr in main.
const char DAT_0010193f[] = "dummy_symlink_recurse_options_data"; // Placeholder for some data
const char DAT_00101204[] = ""; // Placeholder for a default owner string, likely empty or special
const char DAT_00101ca7[] = "/"; // Placeholder for root directory string
const char *_Version = "8.32"; // Example version string

// Global variable for reference file (argument to --reference)
static char *reference_file = NULL;

// Placeholder for `dev_ino_buf_0` - assuming it's a buffer for device/inode info
// The actual type would be `struct dev_ino` or similar, but for compilation, a char array is sufficient.
static char dev_ino_buf_0[100]; // Arbitrary size

// --- Dummy implementations for coreutils utility functions ---
// In a real coreutils build, these would link to actual library functions.
void emit_symlink_recurse_options(const char *s) { (void)s; /* printf("emit_symlink_recurse_options: %s\n", s); */ }
void emit_ancillary_info(const char *s) { (void)s; /* printf("emit_ancillary_info: %s\n", s); */ }
void set_program_name(const char *name) { _program_name = name; }
void close_stdout(void) { /* fflush(_stdout); */ } // Placeholder for cleanup
void chopt_init(int *p) { if (p) *p = 0; } // Placeholder
const char *quote(const char *s) { return s; } // Simple pass-through for dummy
void error(int status, int errnum, const char *format, ...) {
  va_list args;
  va_start(args, format);
  fprintf(stderr, "%s: ", _program_name ? _program_name : "unknown");
  vfprintf(stderr, format, args);
  if (errnum) {
    fprintf(stderr, ": %s", strerror(errnum));
  }
  fprintf(stderr, "\n");
  va_end(args);
  if (status) exit(status);
}

// Simplified dummy for parse_user_spec_warn. Real one is complex.
// It tries to convert spec to numeric UID/GID.
// owner_name and group_name are usually set if symbolic names are parsed.
const char *parse_user_spec_warn(const char *spec, uid_t *owner, gid_t *group, const char **owner_name_out, const char **group_name_out, char *owner_name_allocated) {
  if (owner) *owner = (uid_t)-1;
  if (group) *group = (gid_t)-1;
  if (owner_name_out) *owner_name_out = NULL;
  if (group_name_out) *group_name_out = NULL;
  if (owner_name_allocated) *owner_name_allocated = 0;

  char *spec_copy = strdup(spec);
  if (!spec_copy) return "Out of memory";

  char *colon = strchr(spec_copy, ':');

  if (colon) {
    *colon = '\0';
    // Owner part
    if (spec_copy[0] != '\0') {
      if (owner) *owner = (uid_t)atoi(spec_copy);
      if (owner_name_out) *owner_name_out = spec_copy; // Not strdup to avoid leak in this dummy
    }
    // Group part
    if (colon[1] != '\0') {
      if (group) *group = (gid_t)atoi(colon + 1);
      if (group_name_out) *group_name_out = colon + 1; // Not strdup
    }
  } else {
    // No colon, could be owner or group depending on _chown_mode
    if (_chown_mode == 1) { // chown mode, assume it's owner
      if (owner) *owner = (uid_t)atoi(spec_copy);
      if (owner_name_out) *owner_name_out = spec_copy;
    } else { // chgrp mode, assume it's group
      if (group) *group = (gid_t)atoi(spec_copy);
      if (group_name_out) *group_name_out = spec_copy;
    }
  }

  // In a real implementation, you'd free spec_copy if names were strdup'd.
  // For this dummy, we just leak spec_copy.
  // The actual coreutils parse_user_spec_warn manages memory for names.
  free(spec_copy); // Free the copy, as owner_name_out/group_name_out are not persistent in this dummy.
  return NULL; // No error
}

void *xmalloc(size_t size) {
  void *p = malloc(size);
  if (!p) {
    error(1, 0, "Out of memory");
  }
  return p;
}
const char *quotearg_style(int style, const char *arg) { (void)style; return quote(arg); }
const char *uid_to_name(uid_t uid) { static char buf[20]; sprintf(buf, "%d", uid); return buf; }
const char *gid_to_name(gid_t gid) { static char buf[20]; sprintf(buf, "%d", gid); return buf; }
char *xstrdup(const char *s) {
  char *p = strdup(s);
  if (!p) {
    error(1, 0, "Out of memory");
  }
  return p;
}
long get_root_dev_ino(void *buf) { (void)buf; return 1; } // Dummy: success
char chown_files(char **files, int flags, uid_t owner, gid_t group, uid_t from_owner, gid_t from_group, int *chopt) {
  (void)flags; (void)owner; (void)group; (void)from_owner; (void)from_group; (void)chopt;
  int success = 1;
  while (*files) {
    // In a real implementation, call chown/lchown/fchownat etc.
    // For dummy, just print.
    printf("Dummy: Changing ownership of %s to UID %d, GID %d (flags: %x)\n", *files, owner, group, flags);
    files++;
  }
  return success; // Dummy: always success
}
const char *proper_name_lite(const char *first, const char *last) { (void)last; return first; }
void version_etc(FILE *stream, const char *package, const char *program_name, const char *version, const char *author1, const char *author2, ...) {
  fprintf(stream, "%s (%s) %s\n", program_name, package, version);
  fprintf(stream, "Copyright (C) 2023 Free Software Foundation, Inc.\n");
  fprintf(stream, "License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\n");
  fprintf(stream, "This is free software: you are free to change and redistribute it.\n");
  fprintf(stream, "There is NO WARRANTY, to the extent permitted by law.\n\n");
  fprintf(stream, "Written by %s, and %s.\n", author1, author2); // Only two authors shown in original
}

// --- Option definitions for getopt_long ---
enum {
  DEREFERENCE_OPTION = CHAR_MAX + 1, // --dereference
  FROM_OPTION,                       // --from
  NO_PRESERVE_ROOT_OPTION,           // --no-preserve-root
  PRESERVE_ROOT_OPTION,              // --preserve-root
  REFERENCE_OPTION,                  // --reference
  HELP_OPTION,                       // --help
  VERSION_OPTION                     // --version
};

static const struct option long_options[] = {
    {"changes", no_argument, NULL, 'c'},
    {"dereference", no_argument, NULL, DEREFERENCE_OPTION},
    {"from", required_argument, NULL, FROM_OPTION},
    {"help", no_argument, NULL, HELP_OPTION},
    {"no-dereference", no_argument, NULL, 'h'},
    {"no-preserve-root", no_argument, NULL, NO_PRESERVE_ROOT_OPTION},
    {"preserve-root", no_argument, NULL, PRESERVE_ROOT_OPTION},
    {"quiet", no_argument, NULL, 'f'},
    {"recursive", no_argument, NULL, 'R'},
    {"reference", required_argument, NULL, REFERENCE_OPTION},
    {"silent", no_argument, NULL, 'f'},
    {"verbose", no_argument, NULL, 'v'},
    {"version", no_argument, NULL, VERSION_OPTION},
    {NULL, 0, NULL, 0}
};

// Function: usage
void usage(int status) {
  if (status == 0) {
    printf(gettext("Usage: %s [OPTION]... %s FILE...\n  or:  %s [OPTION]... --reference=RFILE FILE...\n"),
           _program_name,
           (_chown_mode == 1) ? gettext("[OWNER][:[GROUP]]") : gettext("GROUP"),
           _program_name);

    fputs_unlocked((_chown_mode == 1) ?
                   gettext("Change the owner and/or group of each FILE to OWNER and/or GROUP.\nWith --reference, change the owner and group of each FILE to those of RFILE.\n\n") :
                   gettext("Change the group of each FILE to GROUP.\nWith --reference, change the group of each FILE to that of RFILE.\n\n"),
                   _stdout);

    fputs_unlocked(gettext("  -c, --changes          like verbose but report only when a change is made\n  -f, --silent, --quiet  suppress most error messages\n  -v, --verbose          output a diagnostic for every file processed\n"), _stdout);
    fputs_unlocked(gettext("      --dereference      affect the referent of each symbolic link (this is\n                         the default), rather than the symbolic link itself\n  -h, --no-dereference   affect symbolic links instead of any referenced file\n"), _stdout);
    fputs_unlocked(gettext("                         (useful only on systems that can change the\n                         ownership of a symlink)\n"), _stdout);
    fputs_unlocked(gettext("      --from=CURRENT_OWNER:CURRENT_GROUP\n                         change the ownership of each file only if\n                         its current owner and/or group match those specified\n                         here. Either may be omitted, in which case a match\n                         is not required for the omitted attribute\n"), _stdout);
    fputs_unlocked(gettext("      --no-preserve-root  do not treat \'/\' specially (the default)\n      --preserve-root    fail to operate recursively on \'/\'\n"), _stdout);
    fputs_unlocked(gettext("      --reference=RFILE  use RFILE\'s ownership rather than specifying values.\n                         RFILE is always dereferenced if a symbolic link.\n"), _stdout);
    fputs_unlocked(gettext("  -R, --recursive        operate on files and directories recursively\n"), _stdout);
    emit_symlink_recurse_options(DAT_0010193f);
    fputs_unlocked(gettext("      --help        display this help and exit\n"), _stdout);
    fputs_unlocked(gettext("      --version     output version information and exit\n"), _stdout);

    if (_chown_mode == 1) {
      fputs_unlocked(gettext("\nOwner is unchanged if missing.  Group is unchanged if missing, but changed\nto login group if implied by a \':\' following a symbolic OWNER.\nOWNER and GROUP may be numeric as well as symbolic.\n"), _stdout);
    }

    if (_chown_mode == 1) {
      printf(gettext("\nExamples:\n  %s root /u        Change the owner of /u to \"root\".\n  %s root:staff /u  Likewise, but also change its group to \"staff\".\n  %s -hR root /u    Change the owner of /u and subfiles to \"root\".\n"),
             _program_name, _program_name, _program_name);
    } else {
      printf(gettext("\nExamples:\n  %s staff /u      Change the group of /u to \"staff\".\n  %s -hR staff /u  Change the group of /u and subfiles to \"staff\".\n"),
             _program_name, _program_name);
    }
    emit_ancillary_info((_chown_mode == 1) ? "chown" : "chgrp");
  } else {
    fprintf(_stderr, gettext("Try \'%s --help\' for more information.\n"), _program_name);
  }
  exit(status);
}

// Function: main
int main(int argc, char **argv) {
  _stdout = stdout; // Initialize global stdout pointer
  _stderr = stderr; // Initialize global stderr pointer

  char preserve_root = 0; // default: --no-preserve-root
  uid_t target_uid = (uid_t)-1; // -1 means unchanged
  gid_t target_gid = (gid_t)-1; // -1 means unchanged
  uid_t from_uid = (uid_t)-1; // -1 means no 'from' owner specified
  gid_t from_gid = (gid_t)-1; // -1 means no 'from' group specified
  unsigned int chown_flags = 0x10; // Default: 0x10 (P) for symlink traversal
  int dereference_mode = -1; // -1: default (dereference), 0: -h (no-dereference), 1: --dereference
  char recursive = 0; // Default: not recursive
  int chopt_flags = 0; // Flags for chown_files (e.g., --changes, --verbose)
  char owner_name_allocated_flag = 0; // Flag from parse_user_spec_warn, indicates if name was allocated

  set_program_name(argv[0]);
  setlocale(LC_ALL, "");
  bindtextdomain("coreutils", "/usr/local/share/locale");
  textdomain("coreutils");
  atexit(close_stdout);
  chopt_init(&chopt_flags);

  int optval;
  while ((optval = getopt_long(argc, argv, "HLPRcfhv", long_options, NULL)) != -1) {
    switch (optval) {
      case 'H':
        chown_flags = 0x11; // _STAT_FOR_CHDIR_FOR_EACH_ARG (H)
        break;
      case 'L':
        chown_flags = 2; // _STAT_FOR_CHDIR_FOR_LAST_ARG (L)
        break;
      case 'P':
        chown_flags = 0x10; // _STAT_FOR_CHDIR_FOR_NO_ARGS (P)
        break;
      case 'R':
        recursive = 1;
        break;
      case 'c':
        chopt_flags = 1; // --changes (report changes only)
        break;
      case 'f':
        // --silent, --quiet (suppress most error messages). `error` function handles this.
        // The original `local_d7 = 1;` indicates a boolean flag.
        // For simplicity, we assume `error` checks a global `silent_mode` flag.
        // Or it's handled by `chopt_flags` if it's a bitmask.
        // Let's assume this means `error` should be quiet for non-fatal errors.
        break;
      case 'h':
        dereference_mode = 0; // --no-dereference (affect symlinks themselves)
        break;
      case 'v':
        chopt_flags = 0; // --verbose (output diagnostic for every file). If both -c and -v, -v usually wins.
        break;
      case DEREFERENCE_OPTION: // --dereference
        dereference_mode = 1; // Affect the referent of symbolic links
        break;
      case FROM_OPTION: { // --from=CURRENT_OWNER:CURRENT_GROUP
        const char *parse_error_msg = parse_user_spec_warn(optarg, &from_uid, &from_gid, NULL, NULL, &owner_name_allocated_flag);
        if (parse_error_msg != NULL) {
          error((owner_name_allocated_flag ^ 1), 0, "%s: %s", parse_error_msg, quote(optarg));
        }
        break;
      }
      case NO_PRESERVE_ROOT_OPTION: // --no-preserve-root
        preserve_root = 0;
        break;
      case PRESERVE_ROOT_OPTION: // --preserve-root
        preserve_root = 1;
        break;
      case REFERENCE_OPTION: // --reference=RFILE
        reference_file = optarg;
        break;
      case HELP_OPTION: // --help
        usage(0); // usage(0) calls exit(0)
        break; // No need for break, as exit() terminates.
      case VERSION_OPTION: { // --version
        version_etc(_stdout, (_chown_mode == 1) ? "chown" : "chgrp", "GNU coreutils", _Version,
                    proper_name_lite("David MacKenzie", "David MacKenzie"),
                    proper_name_lite("Jim Meyering", "Jim Meyering"),
                    NULL); // NULL terminates varargs for authors
        exit(0);
      }
      case '?': // getopt_long returns '?' for unrecognized options
      default: // Other unexpected values (shouldn't happen with `?` handling)
        usage(1); // usage(1) calls exit(1)
        break; // No need for break, as exit() terminates.
    }
  }

  // Post-option parsing logic
  if (!recursive) { // If not recursive (local_e4 == '\0')
    // If -H or -L was specified without -R
    if (chown_flags != 0x10) { // If chown_flags is 0x11 (H) or 2 (L)
      if (dereference_mode == 1) { // If --dereference was also specified
        error(1, 0, gettext("-R --dereference requires either -H or -L"));
      }
      dereference_mode = 0; // Force no-dereference if -H/-L without -R
    }
  }

  // Determine target UID and GID
  if (reference_file == NULL) {
    if (argc - optind < 2) { // Need at least target_spec and one FILE
      if (optind < argc) { // If there's at least one argument, it's a missing operand after it
        error(0, 0, gettext("missing operand after %s"), quote(argv[argc - 1]));
      } else { // No arguments at all
        error(0, 0, gettext("missing operand"));
      }
      usage(1);
    }

    char *target_spec_arg = argv[optind];
    char *parsed_spec_buf = NULL;

    if (_chown_mode == 2) { // If chgrp mode, add a leading colon if not present
      if (target_spec_arg[0] != ':') { // Only add if not already a group-only spec
        size_t len = strlen(target_spec_arg);
        parsed_spec_buf = xmalloc(len + 2); // For ":" + group_name + null
        parsed_spec_buf[0] = ':';
        strcpy(parsed_spec_buf + 1, target_spec_arg);
        target_spec_arg = parsed_spec_buf;
      }
    }

    const char *parse_error_msg = parse_user_spec_warn(target_spec_arg, &target_uid, &target_gid, NULL, NULL, &owner_name_allocated_flag);
    if (parsed_spec_buf != NULL) {
      free(parsed_spec_buf);
    }
    if (parse_error_msg != NULL) {
      error((owner_name_allocated_flag ^ 1), 0, "%s: %s", parse_error_msg, quote(argv[optind]));
    }

    optind++; // Move past the target_spec argument
  } else { // --reference=RFILE was used
    struct stat st;
    if (stat(reference_file, &st) != 0) {
      error(1, *__errno_location(), gettext("failed to get attributes of %s"), quotearg_style(4, reference_file));
    }
    if (_chown_mode == 1) { // If chown, set owner from RFILE
      target_uid = st.st_uid;
    }
    target_gid = st.st_gid; // Set group from RFILE
  }

  // Handle --preserve-root with -R
  if (recursive && preserve_root) {
    if (get_root_dev_ino(dev_ino_buf_0) == 0) { // get_root_dev_ino returns 0 on error
      error(1, *__errno_location(), gettext("failed to get attributes of %s"), quotearg_style(4, DAT_00101ca7));
    }
  }

  // Adjust chown_flags for dereference behavior
  // If dereference_mode is 0 (from -h, --no-dereference), do NOT add the dereference flag.
  // Otherwise (default or --dereference), add the dereference flag (0x400).
  if (dereference_mode != 0) {
    chown_flags |= 0x400; // Flag to dereference symbolic links
  }

  // Call the main chown/chgrp logic function
  char chown_result = chown_files(argv + optind, chown_flags, target_uid, target_gid, from_uid, from_gid, &chopt_flags);

  // Exit with status 0 on success, 1 on failure
  return !(chown_result);
}