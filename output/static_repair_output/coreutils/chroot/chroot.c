#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <locale.h>
#include <libintl.h>
#include <getopt.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <stddef.h>
#include <ctype.h>   // For character type checks, though to_uchar is used
#include <inttypes.h> // For uintmax_t

// --- Coreutils specific function declarations and global variables ---
// These declarations are placeholders. In a real coreutils build,
// these would be linked from libcoreutils or other internal libraries.

extern const char *_program_name;
extern FILE *_stdout;
extern FILE *_stderr;
extern const char *_Version;

extern void set_program_name(const char *name);
extern void initialize_exit_failure(int status);
extern void close_stdout(void);
extern void version_etc(FILE *stream, const char *package, const char *program_name, const char *version, const char *authors, ...);
extern const char *proper_name_lite(const char *first, const char *last);
extern void emit_exec_status(const char *command);
extern void emit_ancillary_info(const char *command);
extern char *quotearg_style(int style, const char *arg);
extern char *quote(const char *arg);
extern void error(int status, int errnum, const char *format, ...);

extern char *xstrdup(const char *s);
// xstrtoumax signature: char *str, char **endptr, int base, unsigned long long *val, int *err
extern int xstrtoumax(const char *str, char **endptr, int base, unsigned long long *val, int *err);
// xpalloc signature: void *ptr, size_t *allocated_size_ptr, size_t count, size_t element_size, int type
extern void *xpalloc(void *ptr, size_t *allocated_size_ptr, size_t count, size_t element_size, int type);
extern int xgetgroups(const char *user, gid_t primary_gid, gid_t **groups_list);

extern int parse_user_spec(const char *spec, uid_t *uid, gid_t *gid, int flags, void *data);
// parse_user_spec_warn returns a pointer to an error string on failure, NULL on success.
extern const char *parse_user_spec_warn(const char *spec, uid_t *uid, gid_t *gid, int flags, void *data, bool *no_group_specified_p);
extern char *canonicalize_file_name(const char *path);
extern int streq(const char *s1, const char *s2); // Returns non-zero if equal
extern char *bad_cast(const char *s);

// --- Ghidra specific/compiler specific declarations ---
extern const unsigned short **__ctype_b_loc(void);
extern unsigned char to_uchar(int c); // Helper for ctype functions

// Stack protection
extern long __stack_chk_guard;
extern void __stack_chk_fail(void);

// --- Global data placeholders from original snippet ---
// Assuming DAT_001015ac is a placeholder for `errno` or similar in xstrtoumax.
// It's used as `&DAT_001015ac`, implying it's an int variable for error status.
int DAT_001015ac = 0; // Placeholder
const char DAT_00101634[] = "/"; // Likely root path string
const char DAT_001018a9[] = ""; // Short options string for getopt_long (empty as only long options seem used)
const char DAT_0010194d[] = "-i"; // Argument for shell
const char DAT_00101950[] = "parse_user_spec_warn failed: %s"; // Error message format string

// Definition of long options for getopt_long
// The values 0x100, 0x101, 0x102, -0x82, -0x83 are assumed to be the 'val' field
// in the struct option, which getopt_long returns.
static const struct option long_opts[] = {
    {"groups", required_argument, NULL, 0x100},
    {"userspec", required_argument, NULL, 0x101},
    {"skip-chdir", no_argument, NULL, 0x102},
    {"help", no_argument, NULL, -0x82},
    {"version", no_argument, NULL, -0x83},
    {NULL, 0, NULL, 0}
};

// Function: uid_unset
bool uid_unset(int uid) {
  return uid == (int)-1;
}

// Function: gid_unset
bool gid_unset(int gid) {
  return gid == (int)-1;
}

// Function: usage
void usage(int status) {
  if (status == EXIT_SUCCESS) { // param_1 == 0
    printf(gettext("Usage: %s [OPTION]... NEWROOT [COMMAND [ARG]...]\n"), _program_name);
    fputs_unlocked(gettext("Run COMMAND with root directory set to NEWROOT.\n\n"), _stdout);
    fputs_unlocked(gettext(
                            "      --groups=G_LIST        specify supplementary groups as g1,g2,..,gN\n"
                            ), _stdout);
    fputs_unlocked(gettext(
                            "      --userspec=USER:GROUP  specify user and group (ID or name) to use\n"
                            ), _stdout);
    printf(gettext("      --skip-chdir           do not change working directory to %s\n"),
           quotearg_style(4, DAT_00101634)); // DAT_00101634 is likely "/"
    fputs_unlocked(gettext("      --help        display this help and exit\n"), _stdout);
    fputs_unlocked(gettext("      --version     output version information and exit\n"), _stdout);
    fputs_unlocked(gettext(
                            "\nIf no command is given, run \'\"$SHELL\" -i\' (default: \'/bin/sh -i\').\n"
                            ), _stdout);
    emit_exec_status("chroot");
    emit_ancillary_info("chroot");
  } else {
    fprintf(_stderr, gettext("Try \'%s --help\' for more information.\n"), _program_name);
  }
  exit(status);
}

// Function: parse_additional_groups
int parse_additional_groups(const char *group_list_str, gid_t **groups_array_ptr, size_t *num_groups_ptr, bool warn_on_error) {
  // Stack protector setup
  long stack_guard_val = __stack_chk_guard;

  size_t allocated_groups = 0;
  size_t num_groups = 0;
  gid_t *groups_array = NULL;
  
  char *input_copy = xstrdup(group_list_str);
  int result_code = 0;
  char *token = strtok(input_copy, ",");

  while (token != NULL) {
    unsigned long long current_gid_val;
    int xstrtoumax_err_code;
    struct group *grp_entry = NULL;
    
    // Try parsing as a numeric GID
    int parse_result = xstrtoumax(token, NULL, 10, &current_gid_val, &xstrtoumax_err_code);
    
    bool numeric_parsed_ok = (parse_result == 0 && current_gid_val < (unsigned long long)0x100000000);
    bool group_found = false;
    gid_t resolved_gid = (gid_t)-1;

    char *name_or_id_str = token; // Pointer to the current token segment

    if (numeric_parsed_ok) {
        // If numeric parse was successful, try to resolve as name if conditions met
        char *tmp_token = token;
        const unsigned short *ctype_flags_ptr;
        unsigned char char_val;
        
        // Skip leading whitespace for name lookup in the token
        while(true) {
          ctype_flags_ptr = *__ctype_b_loc();
          char_val = to_uchar((int)*tmp_token);
          if (!(ctype_flags_ptr[char_val] & 0x2000)) break; // 0x2000 is _IS_SPACE
          tmp_token++;
        }

        if (*tmp_token != '+') { // Only try getgrnam if it doesn't start with '+'
            grp_entry = getgrnam(tmp_token);
        }
        if (grp_entry != NULL) {
            resolved_gid = grp_entry->gr_gid;
            group_found = true;
        } else {
            resolved_gid = (gid_t)current_gid_val; // Use the numeric value
            group_found = true; // Numeric parse was successful
        }
    } else { // Numeric parse failed or value too large, try only as name
        grp_entry = getgrnam(name_or_id_str);
        if (grp_entry != NULL) {
            resolved_gid = grp_entry->gr_gid;
            group_found = true;
        }
    }

    if (!group_found) {
      result_code = -1;
      if (!warn_on_error) { // if param_4 == '\0' (false), then break
        break;
      }
      error(0, *__errno_location(), gettext("invalid group %s"), quote(name_or_id_str));
    } else {
      // Add GID to array
      if (num_groups == allocated_groups) {
        // xpalloc manages reallocation and updates allocated_groups
        groups_array = xpalloc(groups_array, &allocated_groups, num_groups + 1, sizeof(gid_t), 0);
      }
      groups_array[num_groups] = resolved_gid;
      num_groups++;
    }
    token = strtok(NULL, ",");
  }

  if (result_code == 0 && num_groups == 0) {
    if (warn_on_error) {
      error(0, 0, gettext("invalid group list %s"), quote(group_list_str));
    }
    result_code = -1;
  }

  *groups_array_ptr = groups_array;
  if (result_code == 0) {
    *num_groups_ptr = num_groups;
  }

  free(input_copy);

  // Stack protector check
  if (stack_guard_val != __stack_chk_guard) {
    __stack_chk_fail();
  }
  return result_code;
}

// Function: is_root
char is_root(const char *path) {
  char *canonical_path = canonicalize_file_name(path);
  char result = 0; // Default to false

  if (canonical_path != NULL) {
    if (streq(DAT_00101634, canonical_path)) { // DAT_00101634 is likely "/"
      result = 1; // True
    }
    free(canonical_path);
  }
  return result;
}

// Function: main
int main(int argc, char **argv) {
  // Stack protector setup
  long stack_guard_val = __stack_chk_guard;

  char *user_spec_str = NULL;
  char *additional_groups_str = NULL;
  bool skip_chdir = false;

  uid_t target_uid = (uid_t)-1;
  gid_t target_gid = (gid_t)-1;
  
  gid_t *initial_groups = NULL; // Groups parsed before chroot
  size_t initial_num_groups = 0;

  set_program_name(*argv);
  setlocale(LC_ALL, ""); // LC_ALL is 6
  bindtextdomain("coreutils", "/usr/local/share/locale");
  textdomain("coreutils");
  initialize_exit_failure(EXIT_FAILURE); // 0x7d is 125, often EXIT_FAILURE
  atexit(close_stdout);

  int opt;
  while ((opt = getopt_long(argc, argv, DAT_001018a9, long_opts, NULL)) != -1) {
    switch (opt) {
      case 0x102: // --skip-chdir
        skip_chdir = true;
        break;
      case 0x101: // --userspec
        user_spec_str = optarg;
        size_t len = strlen(user_spec_str);
        if (len != 0 && user_spec_str[len - 1] == ':') {
          user_spec_str[len - 1] = '\0';
        }
        break;
      case 0x100: // --groups
        additional_groups_str = optarg;
        break;
      case -0x83: // --version
        version_etc(_stdout, "chroot", "GNU coreutils", _Version, proper_name_lite("Roland McGrath", "Roland McGrath"), 0);
        exit(EXIT_SUCCESS);
      case -0x82: // --help
        usage(EXIT_SUCCESS);
        break; // usage() calls exit.
      default:
        usage(EXIT_FAILURE);
    }
  }

  if (argc <= optind) {
    error(0, 0, gettext("missing operand"));
    usage(EXIT_FAILURE);
  }

  char *newroot_path = argv[optind];
  char root_check_result = is_root(newroot_path);

  if (root_check_result != 1 && skip_chdir) { // If not root and --skip-chdir
    error(0, 0, gettext("option --skip-chdir only permitted if NEWROOT is old %s"),
          quotearg_style(4, DAT_00101634)); // DAT_00101634 is likely "/"
    usage(EXIT_FAILURE);
  }

  // Pre-chroot user/group processing for initial groups
  if (root_check_result != 1) { // Only if not changing to root
    if (user_spec_str != NULL) {
      parse_user_spec(user_spec_str, &target_uid, &target_gid, 0, NULL);
    }

    bool uid_is_unset_pre = uid_unset(target_uid);
    bool gid_is_unset_pre = gid_unset(target_gid);
    char *pw_name_pre = NULL;

    if (!uid_is_unset_pre && (additional_groups_str == NULL || gid_is_unset_pre)) {
      struct passwd *pw_entry = getpwuid(target_uid);
      if (pw_entry != NULL) {
        if (gid_is_unset_pre) {
          target_gid = pw_entry->pw_gid;
        }
        pw_name_pre = pw_entry->pw_name;
      }
    }
    
    // Get initial groups from user's passwd entry if --groups not specified
    if (additional_groups_str == NULL && !uid_is_unset_pre && !gid_is_unset_pre && pw_name_pre != NULL) {
        int num_groups_ret = xgetgroups(pw_name_pre, target_gid, &initial_groups);
        if (num_groups_ret > 0) {
            initial_num_groups = (size_t)num_groups_ret;
        }
    }
    // Parse additional groups if specified
    if (additional_groups_str != NULL && *additional_groups_str != '\0') {
        parse_additional_groups(additional_groups_str, &initial_groups, &initial_num_groups, false);
    }
  }

  if (chroot(newroot_path) != 0) {
    error(EXIT_FAILURE, errno, gettext("cannot change root directory to %s"),
          quotearg_style(4, newroot_path));
  }

  if (!skip_chdir && chdir("/") != 0) {
    error(EXIT_FAILURE, errno, gettext("cannot chdir to root directory"));
  }

  char **command_args;
  if (argc == optind + 1) { // No command specified, run shell
    char *shell_path = getenv("SHELL");
    if (shell_path == NULL) {
      shell_path = bad_cast("/bin/sh");
    }
    argv[0] = shell_path;
    argv[1] = bad_cast(DAT_0010194d); // "-i"
    argv[2] = NULL;
    command_args = argv;
  } else {
    command_args = argv + optind + 1;
  }

  // Post-chroot user/group processing
  if (user_spec_str != NULL) {
    bool no_group_specified = false;
    const char *parse_err_str = parse_user_spec_warn(user_spec_str, &target_uid, &target_gid, 0, NULL, &no_group_specified);
    if (parse_err_str != NULL) {
      error(no_group_specified ? 0 : EXIT_FAILURE, 0, DAT_00101950, parse_err_str);
      if (!no_group_specified) {
          exit(EXIT_FAILURE);
      }
    }
  }

  bool uid_is_unset_final = uid_unset(target_uid);
  bool gid_is_unset_final = gid_unset(target_gid);
  char *pw_name_final = NULL;

  if (!uid_is_unset_final && (additional_groups_str == NULL || gid_is_unset_final)) {
    struct passwd *pw_entry = getpwuid(target_uid);
    if (pw_entry == NULL) {
      if (gid_is_unset_final) {
        error(EXIT_FAILURE, errno, gettext("no group specified for unknown uid: %ju"), (uintmax_t)target_uid);
      }
    } else {
      if (gid_is_unset_final) {
        target_gid = pw_entry->pw_gid;
      }
      pw_name_final = pw_entry->pw_name;
    }
  }

  gid_t *current_groups_array = initial_groups;
  size_t current_num_groups = initial_num_groups;

  // If no additional groups were specified (--groups was not used or empty),
  // and UID/GID are set, get groups for the user. This might override initial_groups.
  if ((additional_groups_str == NULL || *additional_groups_str == '\0')) {
      if (!uid_is_unset_final && !gid_is_unset_final && pw_name_final != NULL) {
          gid_t *user_groups = NULL;
          int num_user_groups = xgetgroups(pw_name_final, target_gid, &user_groups);
          if (num_user_groups < 1) {
              if (current_num_groups == 0) { // Only error if no groups at all
                  error(EXIT_FAILURE, errno, gettext("failed to get supplemental groups"));
              }
          } else {
              if (current_groups_array != NULL) { // Free previous array if any
                  free(current_groups_array);
              }
              current_groups_array = user_groups;
              current_num_groups = (size_t)num_user_groups;
          }
      }
  } else { // --groups was specified, parse again post-chroot with error reporting
      gid_t *parsed_additional_groups = NULL;
      size_t parsed_num_additional_groups = 0;
      // warn_on_error is true if no groups were provided at all (current_num_groups == 0)
      int parse_res = parse_additional_groups(additional_groups_str, &parsed_additional_groups, &parsed_num_additional_groups, current_num_groups == 0);
      if (parse_res == 0) {
          if (current_groups_array != NULL) { // Free previous array if any
              free(current_groups_array);
          }
          current_groups_array = parsed_additional_groups;
          current_num_groups = parsed_num_additional_groups;
      } else if (current_num_groups == 0) { // If parsing failed and no groups were available from other sources
          exit(EXIT_FAILURE);
      }
  }

  // Set supplemental groups
  if ((!uid_is_unset_final || additional_groups_str != NULL) &&
      setgroups(current_num_groups, current_groups_array) != 0) {
    error(EXIT_FAILURE, errno, gettext("failed to set supplemental groups"));
  }
  if (current_groups_array != NULL) {
      free(current_groups_array);
  }

  // Set GID
  if (!gid_is_unset_final && setgid(target_gid) != 0) {
    error(EXIT_FAILURE, errno, gettext("failed to set group-ID"));
  }

  // Set UID
  if (!uid_is_unset_final && setuid(target_uid) != 0) {
    error(EXIT_FAILURE, errno, gettext("failed to set user-ID"));
  }

  execvp(*command_args, command_args);

  // If execvp returns, it failed
  int exit_status = (errno == ENOENT) ? 127 : 126; // ENOENT is 2
  error(0, errno, gettext("failed to run command %s"), quote(*command_args));
  
  // Stack protector check
  if (stack_guard_val != __stack_chk_guard) {
    __stack_chk_fail();
  }
  return exit_status;
}

// The setgroups function is prototyped in <grp.h> and implemented by the system library.
// The original snippet had a "Bad instruction" warning for its body.
// We just need the prototype for compilation, which is already available via <grp.h>.
// int setgroups(size_t __n, const gid_t *__groups);