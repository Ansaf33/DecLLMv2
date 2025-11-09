#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <time.h>

// Type aliases from original snippet
// Assuming uint means unsigned int (32-bit on typical systems)
typedef uint32_t uint;
typedef uint8_t undefined;
typedef uint32_t undefined4;
// undefined3 is problematic. In random_string, it was used to store 0x3938 (2 bytes)
// and then accessed as part of a character array. It's replaced by a static char array.

// --- Mock Structures ---
// Forward declarations
typedef struct user_t user_t;
typedef struct team_t team_t;
typedef struct challenge_t challenge_t;
typedef struct flag_t flag_t;
typedef struct solve_t solve_t;
typedef struct ctf_t ctf_t;

// Minimal definitions for compilation.
// The original code uses pointer arithmetic (e.g., param_1 + 0x40)
// which implies specific memory layouts. For C compilation,
// using struct member access (ctf->member) is preferred.
// The mock structs are defined to have the necessary members
// that are accessed, without attempting to replicate exact byte offsets
// unless explicitly required (e.g., in random_string charset).

struct user_t {
    char nick[64];
    char pass[64];
    team_t* team; // Used in `ctf->logged_in_user->team`
};

struct team_t {
    char token[33]; // Size 0x20 + null terminator
    char name[64];  // Used as `team->name`
    char shoutout[128]; // Used as `team->shoutout`
    user_t* leader;
    user_t** members;
    uint32_t member_count;
    uint32_t score; // Used as `team->score`
};

struct challenge_t {
    char name[64];          // Used as `challenge->name`
    char description[256];  // Used as `challenge->description`
    uint32_t points;        // Used as `challenge->points`
    uint32_t status;        // Used as `challenge->status` (0=hidden, 1=available, 2=solved)
    uint32_t category;      // Used as `challenge->category`
    char flag[64];          // Used as `challenge->flag`
};

struct flag_t {
    challenge_t* challenge;
    team_t* team;
    uint32_t solve_time;
};

struct solve_t {
    team_t* team;
    challenge_t* challenge;
};

struct ctf_t {
    uint32_t active;             // Used as `ctf->active`
    uint32_t challenge_count;    // Used as `ctf->challenge_count`
    challenge_t** challenges;    // Used as `ctf->challenges`
    uint32_t team_count;         // Used as `ctf->team_count`
    team_t** teams;              // Used as `ctf->teams`
    flag_t** flags;              // Used as `ctf->flags`
    uint32_t flag_count;         // Added for internal mock logic
    user_t* logged_in_user;      // Used as `ctf->logged_in_user`
};

// --- Global variables ---
static uint32_t r; // For prng

// --- Mock function declarations ---
int user_new(user_t** user_ptr);
int user_set_nick(user_t* user, const char* nick);
int user_set_pass(user_t* user, const char* pass);
int user_set_team(user_t* user, team_t* team, int is_leader);

int team_new(team_t** team_ptr);
int team_change_name(team_t* team, const char* name);
int team_set_shoutout(team_t* team, const char* shoutout);
int team_add_member(team_t* team, user_t* user);

int ctf_add_user(ctf_t* ctf, user_t* user);
int ctf_add_team(ctf_t* ctf, team_t* team);
int ctf_auth_user(ctf_t* ctf, user_t** logged_in_user_ptr, const char* nick, const char* pass);
int ctf_get_ranks(ctf_t* ctf, team_t*** ranks_ptr, uint32_t* count);
int ctf_get_solves(ctf_t* ctf, solve_t*** solves_ptr, uint32_t* count);
int ctf_get_chal(ctf_t* ctf, challenge_t** chal_ptr, unsigned long index);
int ctf_submit_flag(ctf_t* ctf, team_t* current_team, const char* flag_str); // Modified signature
int ctf_add_chal(ctf_t* ctf, challenge_t* chal);
int ctf_open_chal(ctf_t* ctf, uint32_t chal_idx);

const char* chal_cat_to_string(uint32_t category);
const char* chal_status_to_string(uint32_t status);

int flg_team_did_solve(ctf_t* ctf, team_t* team, challenge_t* chal); // Modified signature
int flg_get_solves(ctf_t* ctf, solve_t*** solves_ptr, challenge_t* chal, uint32_t* count); // Modified signature
int flg_find_chal(ctf_t* ctf, challenge_t** chal_ptr, const char* flag_str); // Modified signature

int freaduntil(char* buffer, int max_len, char delimiter, FILE* stream);
const char* error_to_string(int error_code);


// --- Mock Implementations ---

// Mock for freaduntil: Reads a line, removes newline
int freaduntil(char* buffer, int max_len, char delimiter, FILE* stream) {
    if (fgets(buffer, max_len, stream) == NULL) {
        return -1; // Error or EOF
    }
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
        len--;
    }
    char* delim_pos = strchr(buffer, delimiter);
    if (delim_pos != NULL && *delim_pos == delimiter) { // Ensure it's the actual delimiter
        *delim_pos = '\0';
        len = delim_pos - buffer;
    }
    return (int)len;
}

// Mock for error codes
enum ErrorCodes {
    ERR_NONE = 0,
    ERR_GENERIC = 1,
    ERR_INVALID_INPUT = 2,
    ERR_USER_EXISTS = 3,
    ERR_USER_NOT_FOUND = 4,
    ERR_INVALID_PASSWORD = 5,
    ERR_TEAM_EXISTS = 6,
    ERR_TEAM_NOT_FOUND = 7,
    ERR_CHALLENGE_NOT_FOUND = 8,
    ERR_FLAG_INVALID = 9,
    ERR_ALREADY_SOLVED = 10,
    ERR_OUT_OF_MEMORY = 0xf, // Matches 0xf in handle_register_user
    ERR_NOT_LOGGED_IN = 0x40 // A placeholder error code
};

const char* error_to_string(int error_code) {
    switch (error_code) {
        case ERR_NONE: return "No error";
        case ERR_GENERIC: return "Generic error";
        case ERR_INVALID_INPUT: return "Invalid input";
        case ERR_USER_EXISTS: return "User already exists";
        case ERR_USER_NOT_FOUND: return "User not found";
        case ERR_INVALID_PASSWORD: return "Invalid password";
        case ERR_TEAM_EXISTS: return "Team already exists";
        case ERR_TEAM_NOT_FOUND: return "Team not found";
        case ERR_CHALLENGE_NOT_FOUND: return "Challenge not found";
        case ERR_FLAG_INVALID: return "Invalid flag";
        case ERR_ALREADY_SOLVED: return "Challenge already solved";
        case ERR_OUT_OF_MEMORY: return "Out of memory";
        case ERR_NOT_LOGGED_IN: return "Not logged in";
        default: return "Unknown error";
    }
}

// Mock for user functions
int user_new(user_t** user_ptr) {
    *user_ptr = (user_t*)calloc(1, sizeof(user_t));
    return (*user_ptr == NULL) ? ERR_OUT_OF_MEMORY : ERR_NONE;
}
int user_set_nick(user_t* user, const char* nick) {
    if (!user || !nick) return ERR_INVALID_INPUT;
    strncpy(user->nick, nick, sizeof(user->nick) - 1);
    user->nick[sizeof(user->nick) - 1] = '\0';
    return ERR_NONE;
}
int user_set_pass(user_t* user, const char* pass) {
    if (!user || !pass) return ERR_INVALID_INPUT;
    strncpy(user->pass, pass, sizeof(user->pass) - 1);
    user->pass[sizeof(user->pass) - 1] = '\0';
    return ERR_NONE;
}
int user_set_team(user_t* user, team_t* team, int is_leader) {
    if (!user) return ERR_INVALID_INPUT;
    user->team = team;
    return ERR_NONE;
}

// Mock for team functions
int team_new(team_t** team_ptr) {
    *team_ptr = (team_t*)calloc(1, sizeof(team_t));
    return (*team_ptr == NULL) ? ERR_OUT_OF_MEMORY : ERR_NONE;
}
int team_change_name(team_t* team, const char* name) {
    if (!team || !name) return ERR_INVALID_INPUT;
    strncpy(team->name, name, sizeof(team->name) - 1);
    team->name[sizeof(team->name) - 1] = '\0';
    return ERR_NONE;
}
int team_set_shoutout(team_t* team, const char* shoutout) {
    if (!team || !shoutout) return ERR_INVALID_INPUT;
    strncpy(team->shoutout, shoutout, sizeof(team->shoutout) - 1);
    team->shoutout[sizeof(team->shoutout) - 1] = '\0';
    return ERR_NONE;
}
int team_add_member(team_t* team, user_t* user) {
    if (!team || !user) return ERR_INVALID_INPUT;
    team->members = (user_t**)realloc(team->members, (team->member_count + 1) * sizeof(user_t*));
    if (!team->members) return ERR_OUT_OF_MEMORY;
    team->members[team->member_count++] = user;
    if (!team->leader) {
        team->leader = user;
    }
    return ERR_NONE;
}

// Mock for CTF functions
int ctf_add_user(ctf_t* ctf, user_t* user) {
    if (!ctf || !user) return ERR_INVALID_INPUT;
    // In this mock, users are managed through teams
    return ERR_NONE;
}

int ctf_add_team(ctf_t* ctf, team_t* team) {
    if (!ctf || !team) return ERR_INVALID_INPUT;
    for (uint32_t i = 0; i < ctf->team_count; ++i) {
        if (strcmp(ctf->teams[i]->name, team->name) == 0) {
            return ERR_TEAM_EXISTS;
        }
    }
    ctf->teams = (team_t**)realloc(ctf->teams, (ctf->team_count + 1) * sizeof(team_t*));
    if (!ctf->teams) return ERR_OUT_OF_MEMORY;
    ctf->teams[ctf->team_count++] = team;
    return ERR_NONE;
}

int ctf_auth_user(ctf_t* ctf, user_t** logged_in_user_ptr, const char* nick, const char* pass) {
    if (!ctf || !logged_in_user_ptr || !nick || !pass) return ERR_INVALID_INPUT;
    for (uint32_t i = 0; i < ctf->team_count; ++i) {
        for (uint32_t j = 0; j < ctf->teams[i]->member_count; ++j) {
            user_t* user = ctf->teams[i]->members[j];
            if (strcmp(user->nick, nick) == 0) {
                if (strcmp(user->pass, pass) == 0) {
                    *logged_in_user_ptr = user;
                    ctf->logged_in_user = user;
                    return ERR_NONE;
                }
                return ERR_INVALID_PASSWORD;
            }
        }
    }
    return ERR_USER_NOT_FOUND;
}

int ctf_get_ranks(ctf_t* ctf, team_t*** ranks_ptr, uint32_t* count) {
    if (!ctf || !ranks_ptr || !count) return ERR_INVALID_INPUT;
    *ranks_ptr = (team_t**)calloc(ctf->team_count, sizeof(team_t*));
    if (!*ranks_ptr) return ERR_OUT_OF_MEMORY;
    for (uint32_t i = 0; i < ctf->team_count; ++i) {
        (*ranks_ptr)[i] = ctf->teams[i];
    }
    *count = ctf->team_count;
    return ERR_NONE;
}

int ctf_get_solves(ctf_t* ctf, solve_t*** solves_ptr, uint32_t* count) {
    if (!ctf || !solves_ptr || !count) return ERR_INVALID_INPUT;
    solve_t** temp_solves = NULL;
    uint32_t temp_count = 0;

    for (uint32_t i = 0; i < ctf->flag_count; ++i) {
        temp_solves = (solve_t**)realloc(temp_solves, (temp_count + 1) * sizeof(solve_t*));
        if (!temp_solves) {
            // Free already allocated temp_solves before returning error
            for (uint32_t j = 0; j < temp_count; ++j) free(temp_solves[j]);
            free(temp_solves);
            return ERR_OUT_OF_MEMORY;
        }
        solve_t* new_solve = (solve_t*)malloc(sizeof(solve_t));
        if (!new_solve) {
            for (uint32_t j = 0; j < temp_count; ++j) free(temp_solves[j]);
            free(temp_solves);
            return ERR_OUT_OF_MEMORY;
        }
        new_solve->team = ctf->flags[i]->team;
        new_solve->challenge = ctf->flags[i]->challenge;
        temp_solves[temp_count++] = new_solve;
    }

    uint32_t limit = (*count < temp_count) ? *count : temp_count;
    *solves_ptr = (solve_t**)calloc(limit, sizeof(solve_t*));
    if (!*solves_ptr) {
        for (uint32_t j = 0; j < temp_count; ++j) free(temp_solves[j]);
        free(temp_solves);
        return ERR_OUT_OF_MEMORY;
    }
    for (uint32_t i = 0; i < limit; ++i) {
        (*solves_ptr)[i] = temp_solves[i];
    }
    *count = limit;
    free(temp_solves);

    return ERR_NONE;
}

int ctf_get_chal(ctf_t* ctf, challenge_t** chal_ptr, unsigned long index) {
    if (!ctf || !chal_ptr || index >= ctf->challenge_count) {
        return ERR_CHALLENGE_NOT_FOUND;
    }
    *chal_ptr = ctf->challenges[index];
    return ERR_NONE;
}

int ctf_submit_flag(ctf_t* ctf, team_t* current_team, const char* flag_str) {
    if (!ctf || !current_team || !flag_str) return ERR_INVALID_INPUT;

    challenge_t* solved_chal = NULL;
    for (uint32_t i = 0; i < ctf->challenge_count; ++i) {
        if (strcmp(ctf->challenges[i]->flag, flag_str) == 0) {
            solved_chal = ctf->challenges[i];
            break;
        }
    }

    if (!solved_chal) {
        return ERR_FLAG_INVALID;
    }

    if (flg_team_did_solve(ctf, current_team, solved_chal)) {
        return ERR_ALREADY_SOLVED;
    }

    flag_t* new_flag = (flag_t*)malloc(sizeof(flag_t));
    if (!new_flag) return ERR_OUT_OF_MEMORY;
    new_flag->challenge = solved_chal;
    new_flag->team = current_team;
    new_flag->solve_time = (uint32_t)time(NULL);

    ctf->flags = (flag_t**)realloc(ctf->flags, (ctf->flag_count + 1) * sizeof(flag_t*));
    if (!ctf->flags) { free(new_flag); return ERR_OUT_OF_MEMORY; }
    ctf->flags[ctf->flag_count++] = new_flag;

    current_team->score += solved_chal->points;
    // Note: The original logic updates a challenge's status to 2 (solved) after submission.
    // This is simplified and assumes a challenge is "solved" globally once any team solves it.
    // In a real CTF, status is usually per-team.
    solved_chal->status = 2;
    return ERR_NONE;
}

int ctf_add_chal(ctf_t* ctf, challenge_t* chal) {
    if (!ctf || !chal) return ERR_INVALID_INPUT;
    ctf->challenges = (challenge_t**)realloc(ctf->challenges, (ctf->challenge_count + 1) * sizeof(challenge_t*));
    if (!ctf->challenges) return ERR_OUT_OF_MEMORY;
    ctf->challenges[ctf->challenge_count++] = chal;
    return ERR_NONE;
}

int ctf_open_chal(ctf_t* ctf, uint32_t chal_idx) {
    if (!ctf || chal_idx >= ctf->challenge_count) return ERR_INVALID_INPUT;
    ctf->challenges[chal_idx]->status = 1; // Mark as visible
    return ERR_NONE;
}

// Mock for challenge category/status to string
const char* chal_cat_to_string(uint32_t category) {
    switch (category) {
        case 0: return "Pwn"; case 1: return "Rev"; case 2: return "Web";
        case 3: return "Crypto"; case 4: return "Forensics"; case 5: return "Misc";
        case 6: return "OSINT"; default: return "Unknown";
    }
}

const char* chal_status_to_string(uint32_t status) {
    switch (status) {
        case 0: return "Hidden"; case 1: return "Available"; case 2: return "Solved";
        default: return "Unknown";
    }
}

// Mock for flag functions
int flg_team_did_solve(ctf_t* ctf, team_t* team, challenge_t* chal) {
    if (!ctf || !team || !chal) return 0;
    for (uint32_t i = 0; i < ctf->flag_count; ++i) {
        if (ctf->flags[i]->team == team && ctf->flags[i]->challenge == chal) {
            return 1;
        }
    }
    return 0;
}

int flg_get_solves(ctf_t* ctf, solve_t*** solves_ptr, challenge_t* chal, uint32_t* count_limit) {
    if (!ctf || !solves_ptr || !chal || !count_limit) return ERR_INVALID_INPUT;
    solve_t** temp_solves = NULL;
    uint32_t temp_count = 0;

    for (uint32_t i = 0; i < ctf->flag_count; ++i) {
        if (ctf->flags[i]->challenge == chal) {
            temp_solves = (solve_t**)realloc(temp_solves, (temp_count + 1) * sizeof(solve_t*));
            if (!temp_solves) {
                for (uint32_t j = 0; j < temp_count; ++j) free(temp_solves[j]);
                free(temp_solves);
                return ERR_OUT_OF_MEMORY;
            }
            solve_t* new_solve = (solve_t*)malloc(sizeof(solve_t));
            if (!new_solve) {
                for (uint32_t j = 0; j < temp_count; ++j) free(temp_solves[j]);
                free(temp_solves);
                return ERR_OUT_OF_MEMORY;
            }
            new_solve->team = ctf->flags[i]->team;
            new_solve->challenge = ctf->flags[i]->challenge;
            temp_solves[temp_count++] = new_solve;
        }
    }

    uint32_t limit = (*count_limit < temp_count) ? *count_limit : temp_count;
    *solves_ptr = (solve_t**)calloc(limit, sizeof(solve_t*));
    if (!*solves_ptr) {
        for (uint32_t j = 0; j < temp_count; ++j) free(temp_solves[j]);
        free(temp_solves);
        return ERR_OUT_OF_MEMORY;
    }
    for (uint32_t i = 0; i < limit; ++i) {
        (*solves_ptr)[i] = temp_solves[i];
    }
    *count_limit = limit;
    free(temp_solves);

    return ERR_NONE;
}

int flg_find_chal(ctf_t* ctf, challenge_t** chal_ptr, const char* flag_str) {
    if (!ctf || !chal_ptr || !flag_str) return ERR_INVALID_INPUT;
    *chal_ptr = NULL;
    for (uint32_t i = 0; i < ctf->challenge_count; ++i) {
        if (strcmp(ctf->challenges[i]->flag, flag_str) == 0) {
            *chal_ptr = ctf->challenges[i];
            return ERR_NONE;
        }
    }
    return ERR_CHALLENGE_NOT_FOUND;
}

// --- Original Snippet Functions (Fixed) ---

// Function: prng
uint prng(void) {
  r = (r ^ (r >> 0x1f) ^ (r >> 0x1e) ^ (r >> 10)) << 0x1f | (r >> 1);
  return r;
}

// Function: random_string
void random_string(char* output_buffer, uint length) {
  static const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  static const uint charset_len = sizeof(charset) - 1;

  for (uint i = 0; i < length; ++i) {
    output_buffer[i] = charset[prng() % charset_len];
  }
}

// Function: print_main_menu
void print_main_menu(void) {
  printf("\n=== Menu ===================\n");
  printf("1. Register User\n");
  printf("2. Login User\n");
  printf("3. Quit\n");
  printf("> ");
}

// Function: is_alphanum
int is_alphanum(const char *input_str) {
  if (input_str == NULL) return 0;
  for (int i = 0; input_str[i] != '\0'; ++i) {
    if (!isalnum((unsigned char)input_str[i])) {
      return 0;
    }
  }
  return 1;
}

// Function: handle_register_user
void handle_register_user(ctf_t* ctf) {
  int result_code = ERR_NONE;
  user_t* new_user = NULL;
  team_t* new_team = NULL;
  char nickname_buffer[256];
  char password_buffer[256];
  char team_input_buffer[256]; // Used for team token or team name
  char shoutout_buffer[128];
  int is_new_team_created = 0;

  do { // Replaced goto with do-while(0) for error handling
    result_code = user_new(&new_user);
    if (result_code != ERR_NONE) break;

    printf("\n=== User Creation ===================\n");
    printf("Nickname: ");
    fflush(stdout);
    if (freaduntil(nickname_buffer, sizeof(nickname_buffer), '\n', stdin) < 1 || !is_alphanum(nickname_buffer)) {
        result_code = ERR_INVALID_INPUT;
        break;
    }
    result_code = user_set_nick(new_user, nickname_buffer);
    if (result_code != ERR_NONE) break;

    printf("Password: ");
    fflush(stdout);
    if (freaduntil(password_buffer, sizeof(password_buffer), '\n', stdin) < 1 || !is_alphanum(password_buffer)) {
        result_code = ERR_INVALID_INPUT;
        break;
    }
    result_code = user_set_pass(new_user, password_buffer);
    if (result_code != ERR_NONE) break;

    printf("Team token (leave empty to create new team): ");
    fflush(stdout);
    if (freaduntil(team_input_buffer, sizeof(team_input_buffer), '\n', stdin) == -1) {
        result_code = ERR_INVALID_INPUT;
        break;
    }

    if (strcmp(team_input_buffer, "") == 0) { // Create new team
      result_code = team_new(&new_team);
      if (result_code != ERR_NONE) break;

      printf("\n=== Team Creation ===================\n");
      printf("Team name: ");
      fflush(stdout);
      if (freaduntil(team_input_buffer, sizeof(team_input_buffer), '\n', stdin) < 1) {
          result_code = ERR_INVALID_INPUT;
          break;
      }
      result_code = team_change_name(new_team, team_input_buffer);
      if (result_code != ERR_NONE) break;

      printf("Shoutout: ");
      fflush(stdout);
      if (freaduntil(shoutout_buffer, sizeof(shoutout_buffer), '\n', stdin) < 1) {
          result_code = ERR_INVALID_INPUT;
          break;
      }
      result_code = team_set_shoutout(new_team, shoutout_buffer);
      if (result_code != ERR_NONE) break;

      random_string(new_team->token, sizeof(new_team->token) - 1);
      new_team->token[sizeof(new_team->token) - 1] = '\0';
      is_new_team_created = 1;
    } else { // Join existing team by token
      uint i;
      for (i = 0; i < ctf->team_count; ++i) {
        if (memcmp(ctf->teams[i]->token, team_input_buffer, sizeof(new_team->token) - 1) == 0) {
          new_team = ctf->teams[i];
          break;
        }
      }
      if (i == ctf->team_count) {
        result_code = ERR_TEAM_NOT_FOUND;
        break;
      }
    }

    result_code = ctf_add_user(ctf, new_user);
    if (result_code != ERR_NONE) break;

    if (is_new_team_created) {
      result_code = ctf_add_team(ctf, new_team);
      if (result_code != ERR_NONE) break;
    }

    result_code = team_add_member(new_team, new_user);
    if (result_code != ERR_NONE) break;

    result_code = user_set_team(new_user, new_team, is_new_team_created);
    if (result_code != ERR_NONE) break;

    printf("[INFO] Successfully registered.\n");
    if (is_new_team_created) {
      printf("[INFO] Team code: %s\n", new_team->token);
    }
  } while (0); // End of do-while(0) block

  if (result_code != ERR_NONE) {
    printf("[ERROR] %s\n", error_to_string(result_code));
    if (new_user != NULL) {
      free(new_user);
    }
    if (is_new_team_created && new_team != NULL) {
      free(new_team);
    }
  }
}

// Function: handle_login_user
void handle_login_user(ctf_t* ctf) {
  int result_code = ERR_GENERIC;
  char nickname_buffer[64];
  char password_buffer[64];

  printf("\n=== Login ===================\n");
  printf("Nick: ");
  fflush(stdout);
  if (freaduntil(nickname_buffer, sizeof(nickname_buffer), '\n', stdin) < 1 || !is_alphanum(nickname_buffer)) {
    result_code = ERR_INVALID_INPUT;
  } else {
    printf("Password: ");
    fflush(stdout);
    if (freaduntil(password_buffer, sizeof(password_buffer), '\n', stdin) < 1 || !is_alphanum(password_buffer)) {
      result_code = ERR_INVALID_INPUT;
    } else {
      result_code = ctf_auth_user(ctf, &ctf->logged_in_user, nickname_buffer, password_buffer);
      if (result_code == ERR_NONE) {
        printf("[INFO] Successfully logged in as %s.\n", nickname_buffer);
        return;
      }
    }
  }
  printf("[ERROR] %s\n", error_to_string(result_code));
}

// Function: print_ctf_ticker
void print_ctf_ticker(ctf_t* ctf) {
  team_t** ranks = NULL;
  uint32_t total_ranks_count = 0;
  solve_t** solves = NULL;
  uint32_t solve_count_limit = 5;

  const char* team_name = "N/A";
  uint32_t team_score = 0;

  if (ctf->logged_in_user && ctf->logged_in_user->team) {
      team_name = ctf->logged_in_user->team->name;
      team_score = ctf->logged_in_user->team->score;
  }

  ctf_get_ranks(ctf, &ranks, &total_ranks_count);
  free(ranks);

  printf("\n================================\n");
  printf("| [%s] - %d\n", team_name, team_score);
  printf("================================\n");

  ctf_get_solves(ctf, &solves, &solve_count_limit);

  if (solve_count_limit == 0) {
    printf("No solves yet.\n");
  } else {
    for (uint32_t i = 0; i < solve_count_limit; ++i) {
      printf("| %s solved %s (%d pts)\n",
             solves[i]->team->name,
             solves[i]->challenge->name,
             solves[i]->challenge->points);
    }
  }

  if (solves != NULL) {
    for (uint32_t i = 0; i < solve_count_limit; ++i) {
        free(solves[i]);
    }
    free(solves);
  }
  printf("================================\n");
}

// Function: print_ctf_menu
void print_ctf_menu(ctf_t* ctf) {
  print_ctf_ticker(ctf);
  printf("\n=== CTF Menu ===================\n");
  printf("1. View challenge list\n");
  printf("2. View challenge detail\n");
  printf("3. View ranking\n");
  printf("4. View team profile\n");
  printf("5. Submit flag\n");
  printf("6. Logout\n");
  printf("> ");
}

// Function: print_challenge
void print_challenge(ctf_t* ctf, challenge_t* challenge, int full_detail) {
  const char* category_str = chal_cat_to_string(challenge->category);
  uint32_t status_val = challenge->status;

  if (ctf->logged_in_user && ctf->logged_in_user->team &&
      flg_team_did_solve(ctf, ctf->logged_in_user->team, challenge)) {
    status_val = 2; // Solved
  }
  const char* status_str = chal_status_to_string(status_val);

  if (full_detail == 0) {
    printf("[%s] [%d pts] %s - %s\n", category_str, challenge->points, challenge->name, status_str);
  } else {
    printf("[%s] [%d pts] %s - %s\n", category_str, challenge->points, challenge->name, status_str);
    if (challenge->status == 0) {
      printf("Hidden.\n");
    } else {
      printf("%s\n", challenge->description);
    }

    solve_t** top_solvers = NULL;
    uint32_t solvers_count_limit = 3;

    flg_get_solves(ctf, &top_solvers, challenge, &solvers_count_limit);

    if (solvers_count_limit != 0) {
      printf("\nTop Solvers\n");
      printf("===========\n");
      for (uint32_t i = 0; i < solvers_count_limit; ++i) {
        printf("%d. %s\n", i + 1, top_solvers[i]->team->name);
      }
      for (uint32_t i = 0; i < solvers_count_limit; ++i) {
          free(top_solvers[i]);
      }
      free(top_solvers);
    }
  }
}

// Function: handle_view_challenge_list
void handle_view_challenge_list(ctf_t* ctf) {
  if (ctf->challenge_count == 0) {
    printf("[INFO] No challenges are available.\n");
  } else {
    printf("\n=== Challenge list ===================\n");
    for (uint32_t i = 0; i < ctf->challenge_count; ++i) {
      printf("%d. ", i);
      print_challenge(ctf, ctf->challenges[i], 0);
    }
  }
}

// Function: handle_view_challenge_detail
void handle_view_challenge_detail(ctf_t* ctf) {
  char input_buffer[32];
  int result_code = ERR_GENERIC;
  challenge_t* selected_challenge = NULL;

  handle_view_challenge_list(ctf);
  if (ctf->challenge_count == 0) {
      return;
  }

  printf("\nChoose idx: ");
  fflush(stdout);
  if (freaduntil(input_buffer, sizeof(input_buffer), '\n', stdin) == -1) {
    result_code = ERR_INVALID_INPUT;
  } else {
    unsigned long challenge_idx = strtoul(input_buffer, NULL, 10);
    result_code = ctf_get_chal(ctf, &selected_challenge, challenge_idx);
    if (result_code == ERR_NONE) {
      printf("\n=== Challenge detail ===================\n");
      print_challenge(ctf, selected_challenge, 1);
      return;
    }
  }
  printf("[ERROR] %s\n", error_to_string(result_code));
}

// Function: view_ranking_page
void view_ranking_page(ctf_t* ctf, int page_num) {
  team_t** ranks = NULL;
  uint32_t total_teams_count = ctf->team_count;

  ctf_get_ranks(ctf, &ranks, &total_teams_count);

  uint32_t start_idx = (page_num - 1) * 5;
  uint32_t display_count;

  if (start_idx >= total_teams_count) {
      display_count = 0;
  } else {
      display_count = (total_teams_count - start_idx > 5) ? 5 : (total_teams_count - start_idx);
  }

  for (uint32_t i = 0; i < display_count; ++i) {
    uint32_t rank_num = start_idx + i + 1;
    printf("%d. %s (%d pts)\n", rank_num, ranks[start_idx + i]->name, ranks[start_idx + i]->score);
  }
  free(ranks);
}

// Function: handle_view_ranking
void handle_view_ranking(ctf_t* ctf) {
  char input_buffer[32];
  int result_code = ERR_NONE;
  unsigned long current_page = 1;
  unsigned long total_pages = 0;

  printf("\n=== Ranking ===================\n");

  uint32_t total_teams = ctf->team_count;
  total_pages = (total_teams + 4) / 5;
  if (total_pages == 0 && total_teams > 0) total_pages = 1;

  view_ranking_page(ctf, (int)current_page);
  printf("\n=== %lu / %lu ===================\n", current_page, total_pages);

  while(1) {
    printf("\nPage (q to quit): ");
    fflush(stdout);
    if (freaduntil(input_buffer, sizeof(input_buffer), '\n', stdin) == -1) {
      result_code = ERR_INVALID_INPUT;
      break;
    }
    if (strcmp(input_buffer, "q") == 0) {
      return;
    }

    unsigned long requested_page = strtoul(input_buffer, NULL, 10);
    if (requested_page == 0 || requested_page > total_pages) {
      result_code = ERR_INVALID_INPUT;
      break;
    }
    current_page = requested_page;
    view_ranking_page(ctf, (int)current_page);
    printf("\n=== %lu / %lu ===================\n", current_page, total_pages);
  }
  printf("[ERROR] %s\n", error_to_string(result_code));
}

// Function: handle_view_team_profile
void handle_view_team_profile(ctf_t* ctf) {
  char team_name_buffer[64];
  int result_code = ERR_TEAM_NOT_FOUND;
  team_t* found_team = NULL;

  printf("\nTeam name: ");
  fflush(stdout);
  if (freaduntil(team_name_buffer, sizeof(team_name_buffer), '\n', stdin) < 1) {
    result_code = ERR_INVALID_INPUT;
  } else {
    for (uint32_t i = 0; i < ctf->team_count; ++i) {
      if (strcmp(ctf->teams[i]->name, team_name_buffer) == 0) {
        found_team = ctf->teams[i];
        break;
      }
    }

    if (found_team != NULL) {
      printf("\n=== Team profile ===================\n");
      printf("%s\n", found_team->name);
      printf("Leader: %s\n", found_team->leader ? found_team->leader->nick : "N/A");
      printf("Members (%d)\n - ", found_team->member_count);
      for (uint32_t i = 0; i < found_team->member_count; ++i) {
        printf("%s", found_team->members[i]->nick);
        if (found_team->member_count - 1 != i) {
          printf(", ");
        }
      }
      printf("\nShoutout: %s\n", found_team->shoutout);
      return;
    }
  }
  printf("[ERROR] %s\n", error_to_string(result_code));
}

// Function: handle_submit_flag
void handle_submit_flag(ctf_t* ctf) {
  char flag_buffer[512];
  int result_code = ERR_FLAG_INVALID;
  challenge_t* solved_challenge = NULL;

  printf("\n=== Submit flag ===================\n");
  printf("Flag: ");
  fflush(stdout);
  if (freaduntil(flag_buffer, sizeof(flag_buffer), '\n', stdin) < 1) {
    result_code = ERR_INVALID_INPUT;
  } else {
    if (!ctf->logged_in_user || !ctf->logged_in_user->team) {
        result_code = ERR_NOT_LOGGED_IN;
    } else {
        result_code = ctf_submit_flag(ctf, ctf->logged_in_user->team, flag_buffer);
        if (result_code == ERR_NONE) {
            flg_find_chal(ctf, &solved_challenge, flag_buffer);
            printf("[INFO] Congrats! You scored %d points!\n", solved_challenge->points);
            return;
        }
    }
  }
  printf("[ERROR] %s\n", error_to_string(result_code));
}

// Function: handle_logout_user
void handle_logout_user(ctf_t* ctf) {
  printf("[INFO] Successfully logged out.\n");
  ctf->logged_in_user = NULL;
}

// Function: handle_quit
void handle_quit(void) {
  printf("Bye.\n");
  fflush(stdout);
  exit(0);
}

// Function: do_random_action
void do_random_action(ctf_t* ctf) {
  ctf->active = 1;

  uint action_choice = prng() % 10;

  if (action_choice == 9) {
    // No action
  } else if (action_choice == 8) {
    ctf->active = 0;
  } else if (action_choice < 8) {
    if (action_choice < 4) { // Create challenge
      challenge_t* new_chal = (challenge_t*)malloc(sizeof(challenge_t));
      if (!new_chal) return;

      random_string(new_chal->name, sizeof(new_chal->name) - 1);
      new_chal->name[sizeof(new_chal->name) - 1] = '\0';

      random_string(new_chal->description, sizeof(new_chal->description) - 1);
      new_chal->description[sizeof(new_chal->description) - 1] = '\0';

      new_chal->points = prng() % 600;
      if (new_chal->points == 0) {
        new_chal->points = 1;
      }

      new_chal->status = 0; // Hidden
      new_chal->category = prng() % 7;

      random_string(new_chal->flag, sizeof(new_chal->flag) - 1);
      new_chal->flag[sizeof(new_chal->flag) - 1] = '\0';

      ctf_add_chal(ctf, new_chal);
    } else if (action_choice - 4 < 4) { // Open challenge (action_choice 4, 5, 6, 7)
      for (uint32_t i = 0; i < ctf->challenge_count; ++i) {
        if (ctf->challenges[i]->status == 0) { // If hidden
          if (prng() % 100 < 50) { // 50% chance to open
            ctf_open_chal(ctf, i);
            return;
          }
        }
      }
    }
  }
}

// --- Main Function ---
int main() {
    r = (uint32_t)time(NULL);
    if (r == 0) r = 1;

    ctf_t ctf_instance = {0};
    ctf_instance.active = 1;

    // --- Setup initial dummy data ---
    user_t* user1; team_t* teamA;
    user_new(&user1); team_new(&teamA);
    user_set_nick(user1, "player1"); user_set_pass(user1, "pass1");
    team_change_name(teamA, "TeamAlpha"); team_set_shoutout(teamA, "Go Alpha!");
    random_string(teamA->token, sizeof(teamA->token) - 1); teamA->token[sizeof(teamA->token)-1] = '\0';
    ctf_add_user(&ctf_instance, user1); ctf_add_team(&ctf_instance, teamA);
    team_add_member(teamA, user1); user_set_team(user1, teamA, 1);

    user_t* user2; team_t* teamB;
    user_new(&user2); team_new(&teamB);
    user_set_nick(user2, "player2"); user_set_pass(user2, "pass2");
    team_change_name(teamB, "TeamBeta"); team_set_shoutout(teamB, "Beta rules!");
    random_string(teamB->token, sizeof(teamB->token) - 1); teamB->token[sizeof(teamB->token)-1] = '\0';
    ctf_add_user(&ctf_instance, user2); ctf_add_team(&ctf_instance, teamB);
    team_add_member(teamB, user2); user_set_team(user2, teamB, 1);

    challenge_t* chal1 = (challenge_t*)malloc(sizeof(challenge_t));
    strcpy(chal1->name, "Warmup");
    strcpy(chal1->description, "A simple challenge to get started.");
    chal1->points = 100; chal1->status = 1; chal1->category = 0; strcpy(chal1->flag, "FLAG{warmup_flag}");
    ctf_add_chal(&ctf_instance, chal1);

    challenge_t* chal2 = (challenge_t*)malloc(sizeof(challenge_t));
    strcpy(chal2->name, "HiddenGem");
    strcpy(chal2->description, "This challenge is hidden by default.");
    chal2->points = 200; chal2->status = 0; chal2->category = 1; strcpy(chal2->flag, "FLAG{hidden_flag}");
    ctf_add_chal(&ctf_instance, chal2);

    int choice;
    char input_buffer[16];

    while (1) {
        if (ctf_instance.logged_in_user == NULL) {
            print_main_menu();
            if (freaduntil(input_buffer, sizeof(input_buffer), '\n', stdin) == -1) {
                printf("Error reading input. Exiting.\n");
                break;
            }
            choice = atoi(input_buffer);

            switch (choice) {
                case 1: handle_register_user(&ctf_instance); break;
                case 2: handle_login_user(&ctf_instance); break;
                case 3: handle_quit(); break;
                default: printf("Invalid choice.\n"); break;
            }
        } else {
            print_ctf_menu(&ctf_instance);
            if (freaduntil(input_buffer, sizeof(input_buffer), '\n', stdin) == -1) {
                printf("Error reading input. Exiting.\n");
                break;
            }
            choice = atoi(input_buffer);

            switch (choice) {
                case 1: handle_view_challenge_list(&ctf_instance); break;
                case 2: handle_view_challenge_detail(&ctf_instance); break;
                case 3: handle_view_ranking(&ctf_instance); break;
                case 4: handle_view_team_profile(&ctf_instance); break;
                case 5: handle_submit_flag(&ctf_instance); break;
                case 6: handle_logout_user(&ctf_instance); break;
                default: printf("Invalid choice.\n"); break;
            }
        }
        do_random_action(&ctf_instance);
        if (!ctf_instance.active) {
            printf("[INFO] CTF is no longer active. Exiting.\n");
            break;
        }
    }

    // --- Cleanup allocated resources ---
    for (uint32_t i = 0; i < ctf_instance.challenge_count; ++i) {
        free(ctf_instance.challenges[i]);
    }
    free(ctf_instance.challenges);

    for (uint32_t i = 0; i < ctf_instance.team_count; ++i) {
        team_t* team = ctf_instance.teams[i];
        for (uint32_t j = 0; j < team->member_count; ++j) {
            free(team->members[j]); // Free user_t
        }
        free(team->members);
        free(team); // Free team_t
    }
    free(ctf_instance.teams);

    for (uint32_t i = 0; i < ctf_instance.flag_count; ++i) {
        free(ctf_instance.flags[i]);
    }
    free(ctf_instance.flags);

    return 0;
}