#include <stdlib.h> // For malloc, realloc, free
#include <string.h> // For strcmp, memcpy
#include <stdint.h> // For uint32_t, intptr_t

// Decompiler's undefined4 is typically uint32_t
typedef uint32_t undefined4;

// Placeholder for the FlagSystem struct and its functions.
// Their actual implementation is not provided, so stubs are used.
// The size of FlagSystem is inferred to be 24 bytes (0x28 to 0x40 in CTFState).
struct FlagSystem {
    uint32_t data[6]; // Placeholder to match inferred size of 24 bytes
};

// Stubs for flag-related functions
void flg_init(struct FlagSystem *fs, void *chal_data, uint32_t *chal_count, uint32_t some_val) {
    (void)fs; (void)chal_data; (void)chal_count; (void)some_val; // Suppress unused parameter warnings
}
int flg_add_submit(struct FlagSystem *fs, uint32_t param_2, uint32_t param_3, uint32_t some_val) {
    (void)fs; (void)param_2; (void)param_3; (void)some_val;
    return 0; // Placeholder return value
}
int flg_get_solves(struct FlagSystem *fs, int team_ptr_int, int param_3, int *out_param) {
    (void)fs; (void)team_ptr_int; (void)param_3; (void)out_param;
    return 0; // Placeholder return value
}
int flg_team_did_solve(struct FlagSystem *fs, int team_ptr_int, int chal_ptr_int, uint32_t some_val) {
    (void)fs; (void)team_ptr_int; (void)chal_ptr_int; (void)some_val;
    return 0; // Placeholder return value
}

// Structure for CTFState, inferred from memory access patterns.
// Assumes a 32-bit environment where sizeof(void*) == sizeof(uint32_t) == 4.
struct CTFState {
    uint32_t field_0x00;        // 0x00
    uint32_t chal_count;        // 0x04 (param_1[1])
    uint32_t team_count;        // 0x08 (param_1[2])
    uint32_t user_count;        // 0x0c (param_1[3])
    uint32_t chal_capacity;     // 0x10 (param_1[4])
    uint32_t team_capacity;     // 0x14 (param_1[5])
    uint32_t user_capacity;     // 0x18 (param_1[6])
    char **chal_data;           // 0x1c (param_1[7]) - array of char* (challenge pointers)
    void **team_data;           // 0x20 (param_1[8]) - array of void* (team pointers)
    char **user_data;           // 0x24 (param_1[9]) - array of char* (user pointers)
    struct FlagSystem flag_system; // 0x28 (offset 40, param_1[10])
    char **chal_data_copy;      // 0x38 (offset 56) - another pointer to chal_data
    uint32_t field_0x40;        // 0x40 (offset 64, param_1[16])
};

// Helper function to get the score from a team pointer.
// Assumes team_ptr points to a struct where score is at offset 0x58.
static inline uint32_t get_team_score(void *team_ptr) {
    return *(uint32_t*)((char*)team_ptr + 0x58);
}

// Helper function to set the score for a team pointer.
static inline void set_team_score(void *team_ptr, uint32_t score) {
    *(uint32_t*)((char*)team_ptr + 0x58) = score;
}

// Helper function to get challenge status.
// Assumes chal_ptr points to a struct where status is at offset 0x184.
static inline uint32_t get_chal_status(char *chal_ptr) {
    return *(uint32_t*)((char*)chal_ptr + 0x184);
}

// Helper function to get challenge points.
// Assumes chal_ptr points to a struct where points are at offset 0x180.
static inline uint32_t get_chal_points(char *chal_ptr) {
    return *(uint32_t*)((char*)chal_ptr + 0x180);
}


// Function: ctf_init
uint32_t ctf_init(struct CTFState *ctf_state) {
  ctf_state->field_0x00 = 0;
  ctf_state->chal_count = 0;
  ctf_state->team_count = 0;
  ctf_state->user_count = 0;

  ctf_state->chal_data = malloc(0x20); // Allocate for 8 char* pointers (8 * 4 bytes)
  ctf_state->chal_capacity = 8;

  ctf_state->team_data = malloc(0x40); // Allocate for 16 void* pointers (16 * 4 bytes)
  ctf_state->team_capacity = 0x10; // 16

  ctf_state->user_data = malloc(0x80); // Allocate for 32 char* pointers (32 * 4 bytes)
  ctf_state->user_capacity = 0x20; // 32

  ctf_state->field_0x40 = 0;
  flg_init(&ctf_state->flag_system, ctf_state->chal_data, &ctf_state->chal_count, 0x113e9);
  return 0;
}

// Function: ctf_submit_flag
int ctf_submit_flag(struct CTFState *ctf_state, uint32_t param_2, uint32_t param_3) {
  return flg_add_submit(&ctf_state->flag_system, param_2, param_3, 0x114b0);
}

// Function: ctf_add_user
uint32_t ctf_add_user(struct CTFState *ctf_state, char *username) {
  if (ctf_state == NULL || username == NULL) {
    return 1; // Error: Invalid arguments
  }

  for (unsigned int i = 0; i < ctf_state->user_count; ++i) {
    if (strcmp(ctf_state->user_data[i], username) == 0) {
      return 0xb; // Error: User already exists
    }
  }

  if (ctf_state->user_count == ctf_state->user_capacity) {
    char **new_user_data = realloc(ctf_state->user_data, ctf_state->user_capacity * 2 * sizeof(char*));
    if (new_user_data == NULL) return 1; // Realloc failed
    ctf_state->user_data = new_user_data;
    ctf_state->user_capacity *= 2;
  }

  ctf_state->user_data[ctf_state->user_count] = username;
  ctf_state->user_count++;
  return 0;
}

// Function: ctf_auth_user
uint32_t ctf_auth_user(struct CTFState *ctf_state, char **out_user_ptr, char *username, char *password) {
  if (ctf_state == NULL || out_user_ptr == NULL || username == NULL || password == NULL) {
    return 1; // Error: Invalid arguments
  }

  for (unsigned int i = 0; i < ctf_state->user_count; ++i) {
    char *user_entry = ctf_state->user_data[i];
    if (strcmp(user_entry, username) == 0) {
      // Assuming password is at offset 0x10 (16 bytes) from the user_entry start.
      // This implies user_entry points to a struct like { char username[16]; char password[...]; ... }
      if (strcmp(user_entry + 0x10, password) == 0) {
        *out_user_ptr = user_entry;
        return 0; // Success
      }
      return 0xe; // Error: Incorrect password
    }
  }
  return 5; // Error: User not found
}

// Function: ctf_add_team
uint32_t ctf_add_team(struct CTFState *ctf_state, void *team_ptr) {
  if (ctf_state == NULL || team_ptr == NULL) {
    return 1; // Error: Invalid arguments
  }

  for (unsigned int i = 0; i < ctf_state->team_count; ++i) {
    // Assuming team_ptr points to a struct with a name/identifier at offset 0x21
    if (strcmp((char *)ctf_state->team_data[i] + 0x21, (char *)team_ptr + 0x21) == 0) {
      return 0xc; // Error: Team already exists
    }
  }

  if (ctf_state->team_count == ctf_state->team_capacity) {
    void **new_team_data = realloc(ctf_state->team_data, ctf_state->team_capacity * 2 * sizeof(void*));
    if (new_team_data == NULL) return 1; // Realloc failed
    ctf_state->team_data = new_team_data;
    ctf_state->team_capacity *= 2;
  }

  ctf_state->team_data[ctf_state->team_count] = team_ptr;
  ctf_state->team_count++;
  return 0;
}

// Function: ctf_add_chal
uint32_t ctf_add_chal(struct CTFState *ctf_state, char *chal_ptr) {
  if (ctf_state == NULL || chal_ptr == NULL) {
    return 1; // Error: Invalid arguments
  }

  for (unsigned int i = 0; i < ctf_state->chal_count; ++i) {
    char *existing_chal = ctf_state->chal_data[i];
    // Compare challenge name (at offset 0) or another identifier (at offset 0x18c)
    if (strcmp(existing_chal, chal_ptr) == 0 || strcmp(existing_chal + 0x18c, chal_ptr + 0x18c) == 0) {
      return 0xd; // Error: Challenge already exists
    }
  }

  if (ctf_state->chal_count == ctf_state->chal_capacity) {
    char **new_chal_data = realloc(ctf_state->chal_data, ctf_state->chal_capacity * 2 * sizeof(char*));
    if (new_chal_data == NULL) return 1; // Realloc failed
    ctf_state->chal_data = new_chal_data;
    ctf_state->chal_data_copy = new_chal_data; // Update the copy as well
    ctf_state->chal_capacity *= 2;
  }

  ctf_state->chal_data[ctf_state->chal_count] = chal_ptr;
  ctf_state->chal_count++;
  return 0;
}

// Function: ctf_open_chal
uint32_t ctf_open_chal(struct CTFState *ctf_state, unsigned int chal_idx) {
  if (ctf_state == NULL) {
    return 1; // Error: Invalid arguments
  }

  if (chal_idx >= ctf_state->chal_count) {
    return 6; // Error: Challenge index out of bounds
  }

  uint32_t *chal_status_ptr = (uint32_t*)((char*)ctf_state->chal_data[chal_idx] + 0x184);

  if (*chal_status_ptr == 0) { // If challenge is not open (status 0)
    *chal_status_ptr = 3; // Open challenge (set status to 3)
    return 0; // Success
  } else {
    return 8; // Error: Challenge already open
  }
}

// Function: ctf_get_solves
int ctf_get_solves(struct CTFState *ctf_state, int team_ptr_int, int *out_solve_count) {
  if (ctf_state == NULL || team_ptr_int == 0 || out_solve_count == NULL) {
    return 1; // Error: Invalid arguments
  }

  return flg_get_solves(&ctf_state->flag_system, team_ptr_int, 0, out_solve_count);
}

// Function: ctf_select_nth
// Implements a partition step for Quickselect.
// arr: array of pointers (void**)
// count: number of elements in the current partition
// k_index: the 0-indexed rank to find
uint32_t ctf_select_nth(void **arr, int count, int k_index) {
  void *pivot_ptr = arr[count - 1];
  uint32_t pivot_score = get_team_score(pivot_ptr);

  int store_index = 0;
  for (int i = 0; i < count - 1; ++i) {
    if (get_team_score(arr[i]) <= pivot_score) {
      void *temp = arr[i];
      arr[i] = arr[store_index];
      arr[store_index] = temp;
      store_index++;
    }
  }

  // Move pivot to its final sorted position
  void *temp = arr[count - 1];
  arr[count - 1] = arr[store_index];
  arr[store_index] = temp;

  if (k_index == store_index) {
    return (uint32_t)(intptr_t)arr[store_index]; // Found the k-th element
  } else if (k_index < store_index) {
    return ctf_select_nth(arr, store_index, k_index); // Search in left partition
  } else {
    // Search in right partition. The original code includes the pivot in the right partition.
    return ctf_select_nth(arr + store_index, count - store_index, k_index - store_index);
  }
}

// Function: ctf_get_ranks
uint32_t ctf_get_ranks(struct CTFState *ctf_state, void ***out_ranks_array, unsigned int *out_rank_start_index) {
  if (ctf_state == NULL || out_ranks_array == NULL || out_rank_start_index == NULL) {
    return 1; // Error: Invalid arguments
  }

  // Initialize all team scores to 0
  for (unsigned int i = 0; i < ctf_state->team_count; ++i) {
    set_team_score(ctf_state->team_data[i], 0);
  }

  // Calculate scores for each team
  for (unsigned int i = 0; i < ctf_state->chal_count; ++i) {
    char *chal_ptr = ctf_state->chal_data[i];
    if (get_chal_status(chal_ptr) != 0) { // If challenge is open
      for (unsigned int j = 0; j < ctf_state->team_count; ++j) {
        void *team_ptr = ctf_state->team_data[j];
        if (flg_team_did_solve(&ctf_state->flag_system, (int)(intptr_t)team_ptr, (int)(intptr_t)chal_ptr, 0x11abd) != 0) {
          set_team_score(team_ptr, get_team_score(team_ptr) + get_chal_points(chal_ptr));
        }
      }
    }
  }

  unsigned int max_display_ranks = ctf_state->team_count;
  if (5 < max_display_ranks) { // Display at most 5 ranks
    max_display_ranks = 5;
  }

  unsigned int requested_start_page = *out_rank_start_index; // Input parameter for page number
  *out_ranks_array = NULL;
  *out_rank_start_index = 0; // Reset output start index

  if (max_display_ranks == 0) {
    return 0; // No teams, no ranks
  }

  // Calculate actual starting index for ranking (0-indexed)
  unsigned int actual_start_index;
  if (requested_start_page * 5 > 0 && requested_start_page * 5 - 5 < ctf_state->team_count) {
    actual_start_index = requested_start_page * 5 - 5;
  } else {
    actual_start_index = 0;
  }
  *out_rank_start_index = actual_start_index;

  // Allocate memory for the output ranks array
  *out_ranks_array = malloc(max_display_ranks * sizeof(void*));
  if (*out_ranks_array == NULL) return 1; // Malloc failed

  // Create a temporary copy of team_data for sorting, as ctf_select_nth modifies the array in place.
  void **temp_team_array = malloc(ctf_state->team_count * sizeof(void*));
  if (temp_team_array == NULL) {
      free(*out_ranks_array);
      *out_ranks_array = NULL;
      return 1; // Malloc failed
  }
  memcpy(temp_team_array, ctf_state->team_data, ctf_state->team_count * sizeof(void*));

  // Use ctf_select_nth to find the top ranks
  for (unsigned int i = 0; i < max_display_ranks; ++i) {
    if (actual_start_index + i < ctf_state->team_count) {
        // ctf_select_nth finds the (actual_start_index + i)-th element
        // (by score) and partitions the array such that it's at that position.
        // It returns the pointer to that element.
        (*out_ranks_array)[i] = (void*)(intptr_t)ctf_select_nth(temp_team_array, ctf_state->team_count, actual_start_index + i);
    } else {
        // No more teams to rank, adjust max_display_ranks for remaining iterations
        max_display_ranks = i;
        break;
    }
  }

  free(temp_team_array);
  return 0;
}

// Function: ctf_get_team
uint32_t ctf_get_team(struct CTFState *ctf_state, void **out_team_ptr, unsigned int team_idx) {
  if (ctf_state == NULL || out_team_ptr == NULL) {
    return 1; // Error: Invalid arguments
  }

  if (team_idx >= ctf_state->team_count) {
    return 7; // Error: Team index out of bounds
  }

  *out_team_ptr = ctf_state->team_data[team_idx];
  return 0;
}

// Function: ctf_get_chal
uint32_t ctf_get_chal(struct CTFState *ctf_state, char **out_chal_ptr, unsigned int chal_idx) {
  if (ctf_state == NULL || out_chal_ptr == NULL) {
    return 1; // Error: Invalid arguments
  }

  if (chal_idx >= ctf_state->chal_count) {
    return 6; // Error: Challenge index out of bounds
  }

  *out_chal_ptr = ctf_state->chal_data[chal_idx];
  return 0;
}