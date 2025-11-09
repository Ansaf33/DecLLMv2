#include <stdio.h>      // For printf, putchar_unlocked
#include <sys/types.h>  // For uid_t, gid_t
#include <pwd.h>        // For struct passwd, getpwuid
#include <grp.h>        // For struct group, getgrgid
#include <stdlib.h>     // For free
#include <libintl.h>    // For gettext
#include <error.h>      // For error
#include <errno.h>      // For __errno_location
#include <stddef.h>     // For NULL

// --- Forward declarations for non-standard functions or functions defined elsewhere ---

// Declaration for print_group
char print_group(gid_t gid, char use_name);

// Placeholder for xgetgroups: Assumed to allocate memory for groups_array_ptr
// and return the number of groups or -1 on error.
int xgetgroups(long user_id_or_flag, gid_t primary_gid, gid_t **groups_array_ptr);

// Placeholder for quote: Assumed to convert a long to a string and then quote it.
char *quote(long val);

// --- End of forward declarations ---


// Function: print_group
char print_group(gid_t gid, char use_name) {
  char success_flag = 1;
  struct group *gr_entry = NULL;

  if (use_name != '\0') {
    gr_entry = getgrgid(gid);
    if (gr_entry == NULL) {
      error(0, 0, gettext("cannot find name for group ID %ju"), (unsigned long)gid);
      success_flag = 0;
    }
  }

  if (gr_entry == NULL) {
    printf("%ju", (unsigned long)gid);
  } else {
    printf("%s", gr_entry->gr_name);
  }
  return success_flag;
}


// Function: print_group_list
char print_group_list(long user_param, uid_t uid_param, gid_t primary_gid_param, gid_t effective_gid_param,
                      char use_name_flag, char separator_char) {
  char success_flag = 1;
  struct passwd *pw_entry = NULL;

  if (user_param != 0) {
    pw_entry = getpwuid(uid_param);
    if (pw_entry == NULL) {
      success_flag = 0;
    }
  }

  if (!print_group(primary_gid_param, use_name_flag)) {
    success_flag = 0;
  }

  if (effective_gid_param != primary_gid_param) {
    putchar_unlocked((int)separator_char);
    if (!print_group(effective_gid_param, use_name_flag)) {
      success_flag = 0;
    }
  }

  gid_t lookup_gid = effective_gid_param;
  if (pw_entry != NULL) {
    lookup_gid = pw_entry->pw_gid;
  }

  gid_t *groups_array = NULL;
  int num_groups = xgetgroups(user_param, lookup_gid, &groups_array);

  if (num_groups < 0) {
    int current_errno = *__errno_location();
    if (user_param == 0) {
      error(0, current_errno, gettext("failed to get groups for the current process"));
    } else {
      error(0, current_errno, gettext("failed to get groups for user %s"), quote(user_param));
    }
    success_flag = 0;
  } else {
    for (int i = 0; i < num_groups; ++i) {
      gid_t current_group_id = groups_array[i];
      if ((primary_gid_param != current_group_id) && (effective_gid_param != current_group_id)) {
        putchar_unlocked((int)separator_char);
        if (!print_group(current_group_id, use_name_flag)) {
          success_flag = 0;
        }
      }
    }
    free(groups_array);
  }
  
  // The original stack protector code (local_20 and __stack_chk_fail) is compiler-generated
  // and not part of standard C source code. It is implicitly handled by the compiler
  // when stack protection is enabled, and thus has been removed from the C source.
  return success_flag;
}