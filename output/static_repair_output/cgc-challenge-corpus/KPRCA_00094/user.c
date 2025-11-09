#include <stdbool.h> // For bool
#include <stdlib.h>  // For malloc, free, realloc, NULL
#include <string.h>  // For memset, strcpy, strlen, memmove, strcmp, strdup
#include <stdint.h>  // For uint32_t

// Define undefined4 as uint32_t
typedef uint32_t undefined4;

// Define uint as uint32_t for consistency, assuming it's a 32-bit unsigned integer
typedef uint32_t uint;

// Function: user_new
bool user_new(void **param_1) {
  if (param_1 == NULL) {
    return false;
  }
  *param_1 = malloc(0x34);
  if (*param_1 == NULL) {
    return false;
  }
  memset(*param_1, 0, 0x34);
  return true;
}

// Function: user_set_nick
undefined4 user_set_nick(char *param_1, char *param_2) {
  if (param_1 == NULL || param_2 == NULL) {
    return 1;
  }
  if (strlen(param_2) < 0x10) {
    memset(param_1, 0, 0x10);
    strcpy(param_1, param_2);
    return 0;
  }
  return 2;
}

// Function: user_set_pass
undefined4 user_set_pass(void *param_1, char *param_2) {
  if (param_1 == NULL || param_2 == NULL) {
    return 1;
  }
  if (strlen(param_2) < 0x20) {
    memset((char *)param_1 + 0x10, 0, 0x20);
    strcpy((char *)param_1 + 0x10, param_2);
    return 0;
  }
  return 3;
}

// Function: user_set_team
undefined4 user_set_team(void *param_1, int param_2, undefined4 *param_3) {
  if (param_1 == NULL || param_2 == 0) {
    return 1;
  }
  if (param_3 != NULL) {
    *param_3 = *(undefined4 *)((char *)param_1 + 0x30);
  }
  *(int *)((char *)param_1 + 0x30) = param_2;
  return 0;
}

// Function: team_new
bool team_new(void **param_1) {
  if (param_1 == NULL) {
    return false;
  }

  void *team_obj = malloc(0x5c);
  if (team_obj == NULL) {
    *param_1 = NULL;
    return false;
  }
  *param_1 = team_obj;
  memset(team_obj, 0, 0x5c);

  void *members_array = malloc(0x20); // Initial capacity for 8 members (0x20 / sizeof(void*))
  if (members_array == NULL) {
    free(team_obj);
    *param_1 = NULL;
    return false;
  }
  
  *(void **)((char *)team_obj + 0x44) = members_array;
  *(undefined4 *)((char *)team_obj + 0x50) = 8; // Capacity: 8

  // Clear the newly allocated members array. Assuming members are void* pointers.
  memset(members_array, 0, *(undefined4 *)((char *)team_obj + 0x50) * sizeof(void *));

  return true;
}

// Function: team_add_member
undefined4 team_add_member(void *param_1, void *param_2) {
  if (param_1 == NULL || param_2 == NULL) {
    return 1;
  }

  char *team_base = (char *)param_1;
  
  uint current_count = *(uint *)(team_base + 0x4c);
  uint current_capacity = *(uint *)(team_base + 0x50);
  void **members_array_ptr_slot = (void **)(team_base + 0x44);

  if (current_count == current_capacity) {
    void *new_members_array = realloc(*members_array_ptr_slot, current_capacity * 2 * sizeof(void *));
    if (new_members_array == NULL) {
      return 6; // Custom error code for realloc failure
    }
    *members_array_ptr_slot = new_members_array;
    *(uint *)(team_base + 0x50) = current_capacity * 2;
  }

  if (current_count == 0) {
    // This assumes offset 0x48 stores the first member's pointer directly.
    *(void **)(team_base + 0x48) = param_2;
  }
  
  *(void **)((char *)*members_array_ptr_slot + current_count * sizeof(void *)) = param_2;
  
  *(uint *)(team_base + 0x4c) = current_count + 1;

  return 0;
}

// Function: team_remove_member
undefined4 team_remove_member(void *param_1, char *param_2) {
  if (param_1 == NULL || param_2 == NULL) {
    return 1;
  }

  char *team_base = (char *)param_1;
  uint current_count = *(uint *)(team_base + 0x4c);
  void **members_array_ptr_slot = (void **)(team_base + 0x44);
  char *members_array_base = (char *)*members_array_ptr_slot;

  uint i;
  for (i = 0; i < current_count; ++i) {
    void *user_obj_ptr = *(void **)(members_array_base + i * sizeof(void *));
    
    // Assuming user_obj_ptr points to a user struct where the nickname (char[0x10]) is at offset 0.
    // So, user_obj_ptr itself can be cast to char* to get the nickname string.
    if (strcmp((char *)user_obj_ptr, param_2) == 0) { 
      break;
    }
  }

  if (i == current_count) {
    return 5;
  } else {
    memmove(members_array_base + i * sizeof(void *),
            members_array_base + (i + 1) * sizeof(void *),
            (current_count - i - 1) * sizeof(void *));
    
    *(uint *)(team_base + 0x4c) = current_count - 1;
    return 0;
  }
}

// Function: team_change_name
undefined4 team_change_name(void *param_1, char *param_2) {
  if (param_1 == NULL || param_2 == NULL) {
    return 1;
  }
  if (strlen(param_2) < 0x20) {
    memset((char *)param_1 + 0x21, 0, 0x20);
    strcpy((char *)param_1 + 0x21, param_2);
    return 0;
  }
  return 4;
}

// Function: team_set_shoutout
undefined4 team_set_shoutout(void *param_1, char *param_2) {
  if (param_1 == NULL || param_2 == NULL) {
    return 1;
  }
  
  char **shoutout_ptr = (char **)((char *)param_1 + 0x54);

  if (*shoutout_ptr != NULL) {
    free(*shoutout_ptr);
  }

  *shoutout_ptr = strdup(param_2);
  if (*shoutout_ptr == NULL) {
      return 7; // Custom error for strdup failure
  }
  return 0;
}