#include <stdio.h>    // For printf, fprintf, perror
#include <stdlib.h>   // For malloc, free, exit, abort
#include <string.h>   // For strdup, strlen, strcpy, strcmp, stpcpy
#include <stdbool.h>  // For bool type
#include <sys/types.h> // For uid_t, gid_t, dev_t, ino_t
#include <sys/stat.h> // For struct stat, S_IFMT, S_IFREG, S_IFDIR, fstat, fstatat
#include <unistd.h>   // For chownat, lchownat, close, getuid, getgid
#include <grp.h>      // For struct group, getgrgid
#include <pwd.h>      // For struct passwd, getpwuid
#include <errno.h>    // For errno, EACCES, ENOTSUP
#include <assert.h>   // For __assert_fail
#include <fcntl.h>    // For openat, O_RDONLY, O_WRONLY, O_RDWR, O_DIRECTORY, AT_SYMLINK_NOFOLLOW
#include <stdarg.h>   // For va_list in mock error

// --- MOCK/HELPER FUNCTIONS AND TYPES ---

// Replace undefined types with standard C types
typedef unsigned int uint;

// Mock for xstrdup
char *xstrdup(const char *s) {
    char *dup = strdup(s);
    if (!dup) {
        perror("xstrdup: Out of memory");
        exit(EXIT_FAILURE);
    }
    return dup;
}

// Mock for xmalloc
void *xmalloc(size_t size) {
    void *ptr = malloc(size);
    if (!ptr) {
        perror("xmalloc: Out of memory");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

// Mock for quotearg_style, quotearg_n_style, quotearg_n_style_colon
// In a real coreutils environment, these would handle quoting for shell safety.
// For compilation, we return the input string.
const char *quotearg_style(int style, const char *arg) { (void)style; return arg; }
const char *quotearg_n_style(int n, int style, const char *arg) { (void)n; (void)style; return arg; }
const char *quotearg_n_style_colon(int n, int style, const char *arg) { (void)n; (void)style; return arg; }

// Mock for gettext (from libintl)
const char *gettext(const char *msgid) { return msgid; }

// Mock for error function (from GNU error.h)
void error(int status, int errnum, const char *format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, "ERROR: ");
    vfprintf(stderr, format, args);
    if (errnum != 0) {
        fprintf(stderr, ": %s", strerror(errnum));
    }
    fprintf(stderr, "\n");
    va_end(args);
    if (status != 0) {
        exit(status);
    }
}

// Mock for __errno_location (GNU extension)
int *__errno_location(void) { return &errno; }

// Mock for __stack_chk_fail (stack smashing protection)
void __stack_chk_fail(void) {
    fprintf(stderr, "Stack smashing detected!\n");
    exit(EXIT_FAILURE);
}

// Mock for __assert_fail (standard assert, but original uses GNU specific signature)
void __assert_fail(const char *assertion, const char *file, unsigned int line, const char *function) {
    fprintf(stderr, "%s:%u: %s: Assertion `%s' failed.\n", file, line, function, assertion);
    abort();
}

// Mock for psame_inode
char psame_inode(const struct stat *s1, const struct stat *s2) {
    return (s1->st_dev == s2->st_dev && s1->st_ino == s2->st_ino);
}

// Mock for streq
char streq(const char *s1, const char *s2) {
    if (s1 == NULL || s2 == NULL) return s1 == s2; // Handle NULL pointers
    return strcmp(s1, s2) == 0;
}

// Mock for is_ENOTSUP
char is_ENOTSUP(int errnum) {
    return errnum == ENOTSUP;
}

// Mock for cycle_warning_required
char cycle_warning_required(void *fts, void *ftsent) { (void)fts; (void)ftsent; return 0; }

// Mock for FTS related types and functions (simplified for compilation)
#define FTS_D 1    // Directory
#define FTS_F 2    // File
#define FTS_DP 4   // Directory (post-order visit)
#define FTS_SL 6   // Symbolic link
#define FTS_NS 7   // No stat info
#define FTS_DC 10  // Directory causes a cycle
#define FTS_DNR 11 // Directory not readable (pre-order)
#define FTS_NSOK 12 // No stat info, but OK to proceed (e.g. broken symlink)

typedef struct _ftsent FTSENT;
typedef struct _fts FTS;

struct _ftsent {
    unsigned short fts_info;
    char *fts_path;
    char *fts_name;
    dev_t fts_dev;
    ino_t fts_ino;
    struct stat fts_statp; // Mock for the stat struct within ftsent
    int fts_errno; // Mock for errno associated with this entry
};

struct _fts {
    int fts_fd; // Directory file descriptor for openat calls
    // In a real FTS implementation, there would be more fields
    // For this mock, we just need fts_fd for openat.
};

// Mock for rpl_fts_set, rpl_fts_read, rpl_fts_close, xfts_open
void rpl_fts_set(FTS *fts, FTSENT *ftsent, int info) {
    (void)fts;
    if (ftsent) ftsent->fts_info = info;
}

// This mock will return a dummy entry once, then NULL.
FTSENT *rpl_fts_read(FTS *fts) {
    (void)fts;
    static FTSENT dummy_ftsent = {0};
    static bool first_call = true;
    if (first_call) {
        first_call = false;
        dummy_ftsent.fts_info = FTS_F;
        dummy_ftsent.fts_path = (char*)"./dummy_file";
        dummy_ftsent.fts_name = (char*)"dummy_file";
        dummy_ftsent.fts_dev = 1;
        dummy_ftsent.fts_ino = 1;
        dummy_ftsent.fts_statp.st_uid = getuid();
        dummy_ftsent.fts_statp.st_gid = getgid();
        dummy_ftsent.fts_statp.st_mode = S_IFREG | 0644;
        dummy_ftsent.fts_errno = 0;
        return &dummy_ftsent;
    }
    return NULL;
}

int rpl_fts_close(FTS *fts) { (void)fts; return 0; }

FTS *xfts_open(char *const *argv, int options, void *compar) {
    (void)argv; (void)options; (void)compar;
    static FTS dummy_fts = { .fts_fd = AT_FDCWD }; // Use current working directory as base
    return &dummy_fts;
}

// Mock for umaxtostr
char *umaxtostr(unsigned int val, char *buf) {
    sprintf(buf, "%u", val);
    return buf;
}

// --- STRUCT DEFINITIONS ---

// Based on chopt_init and chopt_free access patterns
struct ChoptOptions {
    unsigned int field_0;
    unsigned char field_4;
    unsigned long long field_8;
    unsigned char field_16;
    unsigned char field_17;
    char _pad_1[6]; // Padding to align field_24_ptr
    void *field_24_ptr;
    void *field_32_ptr;
};

// Based on param_7 in change_file_owner and chown_files
struct ChownState {
    int flags;
    char no_preserve_root;
    char _pad_2[2]; // Padding
    char dereference;
    char _pad_3[3]; // Padding
    dev_t root_dev;
    ino_t root_ino;
    char verbose;
    char _pad_4[6]; // Padding
    char *new_owner_name_str;
    char *new_group_name_str;
};

// Global data references from original snippet
const char DAT_0010193e[] = "/"; // Likely root path string
const char DAT_001019f2[] = "cannot get attributes of %s"; // Error message

// --- ORIGINAL FUNCTIONS, FIXED ---

// Function: chopt_init
void chopt_init(struct ChoptOptions *opts) {
  opts->field_0 = 2;
  opts->field_8 = 0;
  opts->field_16 = 1;
  opts->field_4 = 0;
  opts->field_17 = 0;
  opts->field_24_ptr = NULL;
  opts->field_32_ptr = NULL;
}

// Function: chopt_free
void chopt_free(struct ChoptOptions *opts) {
  free(opts->field_24_ptr);
  free(opts->field_32_ptr);
}

// Function: uid_to_str
char *uid_to_str(uid_t uid) {
  char buffer[24]; // local_28 in original
  umaxtostr(uid, buffer);
  return xstrdup(buffer);
}

// Function: gid_to_str
char *gid_to_str(gid_t gid) {
  char buffer[24]; // local_28 in original
  umaxtostr(gid, buffer);
  return xstrdup(buffer);
}

// Function: gid_to_name
char *gid_to_name(gid_t gid) {
  struct group *pgVar1 = getgrgid(gid);
  if (pgVar1 == NULL) {
    return gid_to_str(gid);
  } else {
    return xstrdup(pgVar1->gr_name);
  }
}

// Function: uid_to_name
char *uid_to_name(uid_t uid) {
  struct passwd *ppVar1 = getpwuid(uid);
  if (ppVar1 == NULL) {
    return uid_to_str(uid);
  } else {
    return xstrdup(ppVar1->pw_name);
  }
}

// Function: user_group_str
char *user_group_str(const char *user, const char *group) {
  if (user == NULL) {
    if (group != NULL) {
      return xstrdup(group);
    }
  } else if (group == NULL) {
    return xstrdup(user);
  } else {
    size_t user_len = strlen(user);
    size_t group_len = strlen(group);
    char *result = (char *)xmalloc(user_len + group_len + 2); // +1 for ':', +1 for null terminator
    char *p = stpcpy(result, user);
    *p++ = ':';
    strcpy(p, group);
    return result;
  }
  return NULL; // Both user and group are NULL
}

// Function: describe_change
void describe_change(const char *file_name, uint change_type,
                    const char *old_uid_name_str, const char *old_gid_name_str,
                    const char *new_uid_name_str, const char *new_gid_name_str) {
  char *format_string = NULL;
  char *new_user_group_combined = NULL;
  char *old_user_group_combined = NULL;

  if (change_type == 1) { // neither symbolic link nor referent has been changed
    printf(gettext("neither symbolic link %s nor referent has been changed\n"),
           quotearg_style(4, file_name));
    return;
  }

  // Prepare combined user:group strings for printing
  // These are for the 'from' and 'to' parts of the message.
  new_user_group_combined = user_group_str(new_uid_name_str, new_gid_name_str);
  old_user_group_combined = user_group_str(old_uid_name_str, old_gid_name_str);

  const char *print_arg2 = old_user_group_combined;
  const char *print_arg3 = new_user_group_combined;

  switch (change_type) {
    case 4: // ownership retained
      if (new_uid_name_str == NULL) {
        if (new_gid_name_str == NULL) {
          format_string = gettext("ownership of %s retained\n");
          print_arg2 = NULL; print_arg3 = NULL; // Only 1 arg
        } else {
          format_string = gettext("group of %s retained as %s\n");
          print_arg2 = new_user_group_combined; print_arg3 = NULL; // Only 2 args
        }
      } else {
        format_string = gettext("ownership of %s retained as %s\n");
        print_arg2 = new_user_group_combined; print_arg3 = NULL; // Only 2 args
      }
      break;
    case 2: // changed ownership/group
      if (new_uid_name_str == NULL) {
        if (new_gid_name_str == NULL) {
          format_string = gettext("no change to ownership of %s\n");
          print_arg2 = NULL; print_arg3 = NULL; // Only 1 arg
        } else {
          format_string = gettext("changed group of %s from %s to %s\n");
        }
      } else {
        format_string = gettext("changed ownership of %s from %s to %s\n");
      }
      break;
    case 3: // failed to change
      if (old_user_group_combined == NULL) { // If old ownership info couldn't be determined
        if (new_uid_name_str == NULL) {
          if (new_gid_name_str == NULL) {
            format_string = gettext("failed to change ownership of %s\n");
            print_arg2 = NULL; print_arg3 = NULL; // Only 1 arg
          } else {
            format_string = gettext("failed to change group of %s to %s\n");
            print_arg2 = new_user_group_combined; print_arg3 = NULL; // Only 2 args
          }
        } else {
          format_string = gettext("failed to change ownership of %s to %s\n");
          print_arg2 = new_user_group_combined; print_arg3 = NULL; // Only 2 args
        }
      } else if (new_uid_name_str == NULL) {
        if (new_gid_name_str == NULL) {
          format_string = gettext("failed to change ownership of %s\n");
          print_arg2 = NULL; print_arg3 = NULL; // Only 1 arg
        } else {
          format_string = gettext("failed to change group of %s from %s to %s\n");
        }
      } else {
        format_string = gettext("failed to change ownership of %s from %s to %s\n");
      }
      break;
    default:
      __assert_fail("0", "chown-core.c", 0xca, "describe_change");
  }

  printf(format_string, quotearg_style(4, file_name), print_arg2, print_arg3);

  free(old_user_group_combined);
  free(new_user_group_combined);
}

// Function: restricted_chown
uint restricted_chown(int dirfd, const char *path, const struct stat *stat_info_param,
                     uid_t new_uid, gid_t new_gid, uid_t from_uid, gid_t from_gid) {
  uint result_code = 2; // Default to 'changed'

  if ((from_uid == (uid_t)-1) && (from_gid == (gid_t)-1)) {
    return 5; // No change requested for uid/gid
  }

  uint open_flags = O_RDONLY;
  if ((stat_info_param->st_mode & S_IFMT) == S_IFREG) {
    // Regular file, O_RDONLY is fine
  } else if ((stat_info_param->st_mode & S_IFMT) == S_IFDIR) {
    open_flags |= O_DIRECTORY;
  } else {
    return 5; // Not a regular file or directory, unchanged.
  }

  int fd = openat(dirfd, path, open_flags);
  if (fd < 0) {
    if (errno == EACCES && (stat_info_param->st_mode & S_IFMT) == S_IFREG) {
      // If permission denied for a regular file, try opening with O_RDWR
      fd = openat(dirfd, path, O_RDWR);
      if (fd < 0) { // Still failed
        if (errno == EACCES) {
          return 5; // Permission denied
        } else {
          return 6; // Other error
        }
      }
    } else { // Initial openat failed for other reasons or non-regular file
      if (errno == EACCES) {
        return 5; // Permission denied
      } else {
        return 6; // Other error
      }
    }
  }

  // If we reach here, fd is open
  struct stat actual_stat;
  if (fstat(fd, &actual_stat) == 0) {
    if (psame_inode(stat_info_param, &actual_stat)) {
      if (((from_uid == (uid_t)-1) || (from_uid == actual_stat.st_uid)) &&
          ((from_gid == (gid_t)-1) || (from_gid == actual_stat.st_gid))) {
        if (fchown(fd, new_uid, new_gid) == 0) {
          result_code = 2; // Changed successfully
        } else {
          result_code = 6; // Error changing ownership
        }
      } else {
        result_code = 4; // Not changed (e.g., --from mismatch)
      }
    } else {
      result_code = 4; // Inode mismatch
    }
  } else {
    result_code = 6; // Error stat'ing file
  }

  int saved_errno = errno;
  close(fd);
  errno = saved_errno;

  return result_code;
}

// Function: change_file_owner
bool change_file_owner(FTS *fts, FTSENT *ftsent, uid_t new_uid, gid_t new_gid,
                      uid_t from_uid, gid_t from_gid, struct ChownState *state) {
  const char *file_path = ftsent->fts_path;
  const char *file_name = ftsent->fts_name;
  bool success = true;
  bool do_chown = true;
  struct stat *current_stat = &ftsent->fts_statp;
  struct stat deref_stat; // For dereferenced stat if needed

  switch (ftsent->fts_info) {
    case FTS_D: // Directory
      if (state->no_preserve_root == 0) { // If preserve-root is active
        if ((state->root_dev == 0 && state->root_ino == 0) ||
            (state->root_dev != ftsent->fts_dev || state->root_ino != ftsent->fts_ino)) {
            // Not root, or root not yet set. Set root_dev_ino if it's the first entry.
            if (state->root_dev == 0 && state->root_ino == 0) {
                state->root_dev = ftsent->fts_dev;
                state->root_ino = ftsent->fts_ino;
            }
        } else { // It is the root directory
          if (!streq(file_name, DAT_0010193e)) { // Not "/"
            error(0, 0, gettext("it is dangerous to operate recursively on %s (same as %s)"),
                  quotearg_n_style(0, 4, file_name), quotearg_n_style(1, 4, DAT_0010193e));
          } else {
            error(0, 0, gettext("it is dangerous to operate recursively on %s"),
                  quotearg_style(4, file_name));
          }
          error(0, 0, gettext("use --no-preserve-root to override this failsafe"));
          rpl_fts_set(fts, ftsent, FTS_DP); // Prune traversal
          // rpl_fts_read(fts); // Original had this, but rpl_fts_set to FTS_DP handles pruning
          success = false;
        }
      }
      break;
    case FTS_DC: // Directory causes cycle
      if (cycle_warning_required(fts, ftsent)) {
        error(0, 0, gettext("WARNING: Circular directory structure.\n"
                            "This almost certainly means that you have a corrupted file system.\n"
                            "NOTIFY YOUR SYSTEM MANAGER.\n"
                            "The following directory is part of the cycle:\n"
                            "  %s\n"),
              quotearg_n_style_colon(0, 3, file_path));
        success = false;
      }
      break;
    case FTS_NS: // No stat info
    case FTS_DNR: // Directory not readable (pre-order)
      if (state->verbose) {
        error(0, ftsent->fts_errno, gettext("cannot read directory %s"),
              quotearg_style(4, file_path));
      }
      success = false;
      break;
    case FTS_SL: // Symbolic link
      if (state->no_preserve_root) { // If --no-preserve-root, treat symlinks as files
        // Continue processing, will be handled by dereference logic below
      } else {
        success = false; // Skip if preserve-root
      }
      break;
    case FTS_NSOK: // No stat info, but OK to proceed (e.g. broken symlink)
      if (state->verbose) {
        error(0, ftsent->fts_errno, DAT_001019f2,
              quotearg_n_style_colon(0, 3, file_path));
      }
      success = false;
      break;
  }

  if (!success) { // If any of the above cases set success to false, skip chown logic
    return success;
  }

  // Determine if chown operation is actually needed
  if (state->flags == 2 && from_uid == (uid_t)-1 && from_gid == (gid_t)-1 && state->dereference == 0) {
    // If silent, no specific from_uid/gid, and not dereferencing symlinks,
    // and root not set, then it's a no-op (but proceed to check if chown needed).
    do_chown = true;
    current_stat = &ftsent->fts_statp;
  } else {
    if (state->dereference && (ftsent->fts_statp.st_mode & S_IFMT) == S_IFLNK) {
      if (fstatat(fts->fts_fd, file_name, &deref_stat, 0) != 0) { // Dereference symlink
        if (state->verbose) {
          error(0, errno, gettext("cannot dereference %s"), quotearg_style(4, file_path));
        }
        success = false;
      }
      current_stat = &deref_stat; // Use dereferenced stat
    }
    // Check if actual change is needed based on current_stat
    if (!success ||
        ((from_uid != (uid_t)-1 && from_uid != current_stat->st_uid) ||
         (from_gid != (gid_t)-1 && from_gid != current_stat->st_gid))) {
      do_chown = false; // No change needed from 'from' conditions
    }
  }

  if (!success) { // Re-check success after dereference logic
    return success;
  }

  // Perform chown operation if `do_chown` is true
  if (do_chown) {
    bool chown_successful = true;
    bool reported_not_supported = false; // Flag for ENOTSUP

    if (state->dereference) { // Follow symlinks
      uint restricted_chown_res = restricted_chown(fts->fts_fd, file_name, current_stat,
                                                   new_uid, new_gid, from_uid, from_gid);
      if (restricted_chown_res == 6) { // Error
        chown_successful = false;
      } else if (restricted_chown_res == 5) { // Permission denied (restricted_chown decided not to chown)
        // Try direct chownat (without restricted_chown's internal logic)
        if (chownat(fts->fts_fd, file_name, new_uid, new_gid, 0) != 0) {
          chown_successful = false;
        }
      } else if (restricted_chown_res == 4) { // Inode mismatch
        chown_successful = false; // Treat as failure to change
      }
    } else { // Do not follow symlinks (lchownat)
      if (lchownat(fts->fts_fd, file_name, new_uid, new_gid, AT_SYMLINK_NOFOLLOW) != 0) {
        if (is_ENOTSUP(errno)) { // If lchownat not supported, it's not an error
          chown_successful = true;
          reported_not_supported = true; // Indicate no actual chown was done
        } else {
          chown_successful = false;
        }
      }
    }

    if (!chown_successful && state->verbose) {
      error(0, errno,
            (new_uid == (uid_t)-1) ? gettext("changing group of %s") : gettext("changing ownership of %s"),
            quotearg_style(4, file_path));
    }
    success = chown_successful;
    do_chown = !reported_not_supported; // If not supported, no chown was actually performed
  }

  // Report change (if not silent)
  if (state->flags != 2) { // If not silent
    bool changed_actual = (do_chown && success &&
                           ((new_uid != (uid_t)-1 && new_uid != current_stat->st_uid) ||
                            (new_gid != (gid_t)-1 && new_gid != current_stat->st_gid)));

    if (changed_actual || state->flags == 0) { // If actual change or verbose
      uint desc_change_type;
      if (success) {
        if (changed_actual) { // Successfully changed
          desc_change_type = 2;
        } else { // Retained (no change needed or not supported)
          desc_change_type = 4;
        }
      } else { // Failed
        desc_change_type = 3;
      }

      char *old_uid_name = uid_to_name(current_stat->st_uid);
      char *old_gid_name = gid_to_name(current_stat->st_gid);
      char *new_uid_name_for_desc = (state->new_owner_name_str != NULL) ? state->new_owner_name_str : ((new_uid == (uid_t)-1) ? NULL : uid_to_str(new_uid));
      char *new_gid_name_for_desc = (state->new_group_name_str != NULL) ? state->new_group_name_str : ((new_gid == (gid_t)-1) ? NULL : gid_to_str(new_gid));

      describe_change(file_path, desc_change_type,
                      old_uid_name, old_gid_name,
                      new_uid_name_for_desc, new_gid_name_for_desc);

      free(old_uid_name);
      free(old_gid_name);
      if (state->new_owner_name_str == NULL && new_uid != (uid_t)-1 && new_uid_name_for_desc != NULL) free(new_uid_name_for_desc);
      if (state->new_group_name_str == NULL && new_gid != (gid_t)-1 && new_gid_name_for_desc != NULL) free(new_gid_name_for_desc);
    }
  }

  if (!state->no_preserve_root && ftsent->fts_info == FTS_D) { // If preserve-root is active, prune directories (FTS_D)
    rpl_fts_set(fts, ftsent, FTS_DP);
  }

  return success;
}

// Function: chown_files
bool chown_files(char *const *file_list, uint fts_options, uid_t new_uid, gid_t new_gid,
                uid_t from_uid, gid_t from_gid, struct ChownState *state) {
  bool overall_success = true;

  // Initialize root_dev_ino for --preserve-root
  if (state->no_preserve_root == 0) {
      struct stat st_root;
      if (stat("/", &st_root) == 0) {
          state->root_dev = st_root.st_dev;
          state->root_ino = st_root.st_ino;
      } else {
          // If stat("/") fails, root_dev/ino remains 0,0, which will prevent --preserve-root check
          // for the actual root directory, which is fine for this context.
          state->root_dev = 0;
          state->root_ino = 0;
      }
  }

  // If no from_uid/gid and not dereferencing, and flags indicate silent, use FTS_COMFOLLOW
  if ((from_uid == (uid_t)-1) && (from_gid == (gid_t)-1) &&
      (state->dereference == 0) && (state->flags == 2)) {
    fts_options |= (1 << 3); // FTS_COMFOLLOW (often 0x8)
  }

  FTS *fts_handle = xfts_open(file_list, fts_options, NULL);
  FTSENT *ftsent_entry;

  while ((ftsent_entry = rpl_fts_read(fts_handle)) != NULL) {
    bool current_file_success = change_file_owner(fts_handle, ftsent_entry,
                                                  new_uid, new_gid,
                                                  from_uid, from_gid, state);
    overall_success &= current_file_success;
  }

  if (errno != 0) {
    if (state->verbose) {
      error(0, errno, gettext("fts_read failed"));
    }
    overall_success = false;
  }

  if (rpl_fts_close(fts_handle) != 0) {
    error(0, errno, gettext("fts_close failed"));
    overall_success = false;
  }
  return overall_success;
}