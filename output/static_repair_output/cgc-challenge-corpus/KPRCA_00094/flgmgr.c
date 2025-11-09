#include <stdlib.h> // For malloc, realloc, free
#include <string.h> // For strcmp, strdup, memset
#include <stdio.h>  // For printf, fprintf
#include <stdint.h> // For uintptr_t

// Forward declarations for structures
typedef struct chal_t chal_t;
typedef struct flg_submission_t flg_submission_t;
typedef struct flg_state_t flg_state_t;

// Structure for "challenge" objects, inferred from flg_find_chal
// The offsets 0x184 and 0x18c are relative to the start of this structure.
// This implies a large structure, but we only need to represent the relevant fields.
struct chal_t {
    char _padding1[0x184]; // Padding to reach offset 0x184
    unsigned int status;   // At offset 0x184
    char _padding2[0x18c - 0x184 - sizeof(unsigned int)]; // Padding to reach offset 0x18c
    char *name;            // At offset 0x18c
};

// Structure for individual submissions (0x18 or 24 bytes)
struct flg_submission_t {
    int team_id;                 // Offset 0x00: Corresponds to `*piVar2` in flg_add_submit, `param_2` in flg_team_did_solve
    uintptr_t chal_ptr_value;    // Offset 0x04: Corresponds to `piVar2[1]`. Stores a `chal_t*` cast to `uintptr_t`.
    char *flag_string;           // Offset 0x08: Corresponds to `piVar2[2]`. Result of `strdup`.
    unsigned int is_solved_flag; // Offset 0x0C: Corresponds to `piVar2[3]`. Boolean flag.
    int submission_index;        // Offset 0x10: Corresponds to `piVar2[4]`. Index in the submissions array.
    int unknown_field;           // Offset 0x14: Remaining field to make size 0x18.
};

// Main state structure
struct flg_state_t {
    unsigned int submission_count;     // Offset 0x00: `*param_1` (current number of submissions)
    unsigned int unknown1;             // Offset 0x04: `param_1[1]`
    unsigned int submission_capacity;  // Offset 0x08: `param_1[2]` (current capacity of the submissions array)
    flg_submission_t **submissions;    // Offset 0x0C: `param_1[3]` (dynamic array of pointers to submissions)
    unsigned int state_team_id;        // Offset 0x10: `param_1[4]` (param_2 in flg_init)
    unsigned int unknown2;             // Offset 0x14: `param_1[5]` (param_3 in flg_init)
    chal_t **challenges;               // Offset 0x18: Base pointer for challenges array
    unsigned int chal_count;           // Offset 0x1C: Count of challenges
};

// Function: flg_init
unsigned int flg_init(flg_state_t *state, unsigned int team_id, unsigned int unknown_param) {
    state->submission_count = 0;
    state->unknown1 = 0;
    state->submission_capacity = 0x10; // Initial capacity for 16 pointers
    state->submissions = (flg_submission_t **)malloc(state->submission_capacity * sizeof(flg_submission_t *));
    if (state->submissions == NULL) {
        return 1; // Indicate error
    }
    state->state_team_id = team_id;
    state->unknown2 = unknown_param;
    state->challenges = NULL; // Initialize challenge related fields
    state->chal_count = 0;
    return 0;
}

// Function: flg_is_duplicate
unsigned int flg_is_duplicate(flg_state_t *state, int team_id, char *submission_name) {
    for (unsigned int i = 0; i < state->submission_count; ++i) {
        flg_submission_t *sub = state->submissions[i];
        if ((team_id == sub->team_id) && (strcmp(sub->flag_string, submission_name) == 0)) {
            return 1; // Duplicate found
        }
    }
    return 0; // No duplicate
}

// Function: flg_find_chal
unsigned int flg_find_chal(flg_state_t *state, chal_t **found_chal_ptr, char *challenge_name) {
    if (state == NULL || found_chal_ptr == NULL || challenge_name == NULL) {
        return 1; // Invalid parameters
    }

    for (unsigned int i = 0; i < state->chal_count; ++i) {
        chal_t *chal = state->challenges[i];
        if (chal != NULL && chal->name != NULL && strcmp(chal->name, challenge_name) == 0) {
            *found_chal_ptr = chal;
            return 0; // Found
        }
    }
    return 6; // Not found
}

// Function: flg_add_submit
int flg_add_submit(flg_state_t *state, int team_id, char *submission_name) {
    if (state == NULL || team_id == 0 || submission_name == NULL) {
        return 1; // Invalid parameters
    }

    if (flg_is_duplicate(state, team_id, submission_name)) {
        return 9; // Duplicate submission
    }

    flg_submission_t *new_submission = (flg_submission_t *)malloc(sizeof(flg_submission_t));
    if (new_submission == NULL) {
        return 1; // Malloc failed
    }
    memset(new_submission, 0, sizeof(flg_submission_t)); // Initialize fields to 0

    new_submission->team_id = team_id;
    new_submission->flag_string = strdup(submission_name);
    if (new_submission->flag_string == NULL) {
        free(new_submission);
        return 1; // strdup failed
    }

    chal_t *found_chal = NULL;
    unsigned int find_chal_status = flg_find_chal(state, &found_chal, submission_name);
    if (find_chal_status != 0) {
        free(new_submission->flag_string);
        free(new_submission);
        return 10; // Challenge not found or error
    }

    new_submission->chal_ptr_value = (uintptr_t)found_chal; // Store pointer value
    new_submission->is_solved_flag = (found_chal != NULL); // If chal found, it's considered solved
    new_submission->submission_index = state->submission_count;

    if (found_chal != NULL && found_chal->status == 3) {
        found_chal->status = 1;
    }

    if (state->submission_count == state->submission_capacity) {
        unsigned int new_capacity = state->submission_capacity * 2;
        flg_submission_t **new_submissions_array = (flg_submission_t **)realloc(state->submissions, new_capacity * sizeof(flg_submission_t *));
        if (new_submissions_array == NULL) {
            free(new_submission->flag_string);
            free(new_submission);
            return 1; // Realloc failed
        }
        state->submissions = new_submissions_array;
        state->submission_capacity = new_capacity;
    }

    state->submissions[state->submission_count] = new_submission;
    state->submission_count++;
    return 0; // Success
}

// Function: flg_get_solves
unsigned int flg_get_solves(flg_state_t *state, flg_submission_t ***solves_array_ptr, uintptr_t filter_chal_ptr_value, unsigned int *max_solves_count) {
    if (state == NULL || solves_array_ptr == NULL || max_solves_count == NULL) {
        return 1; // Invalid parameters
    }

    unsigned int current_solves_count = 0;
    unsigned int i;

    if (filter_chal_ptr_value == 0) { // Get all solves
        unsigned int actual_solves_to_copy = 0;
        for (i = 0; i < state->submission_count; ++i) {
            flg_submission_t *sub = state->submissions[state->submission_count - 1 - i]; // Iterating backwards
            if (sub != NULL && sub->is_solved_flag != 0) {
                actual_solves_to_copy++;
            }
        }

        if (*max_solves_count < actual_solves_to_copy) {
            actual_solves_to_copy = *max_solves_count;
        }
        *max_solves_count = actual_solves_to_copy;

        if (actual_solves_to_copy == 0) {
            *solves_array_ptr = NULL;
            return 0;
        }

        *solves_array_ptr = (flg_submission_t **)malloc(actual_solves_to_copy * sizeof(flg_submission_t *));
        if (*solves_array_ptr == NULL) {
            return 1; // Malloc failed
        }

        current_solves_count = 0;
        for (i = 0; i < state->submission_count && current_solves_count < actual_solves_to_copy; ++i) {
            flg_submission_t *sub = state->submissions[state->submission_count - 1 - i]; // Iterating backwards
            if (sub != NULL && sub->is_solved_flag != 0) {
                (*solves_array_ptr)[current_solves_count] = sub;
                current_solves_count++;
            }
        }
    } else { // Filter by specific challenge pointer value
        // First pass: count how many solves match the criteria
        unsigned int matched_solves_count = 0;
        for (i = 0; i < state->submission_count; ++i) {
            flg_submission_t *sub = state->submissions[i];
            if (sub != NULL && filter_chal_ptr_value == sub->chal_ptr_value && sub->is_solved_flag != 0) {
                matched_solves_count++;
            }
        }

        if (*max_solves_count < matched_solves_count) {
            matched_solves_count = *max_solves_count;
        }
        *max_solves_count = matched_solves_count;

        if (matched_solves_count == 0) {
            *solves_array_ptr = NULL;
            return 0;
        }

        *solves_array_ptr = (flg_submission_t **)malloc(matched_solves_count * sizeof(flg_submission_t *));
        if (*solves_array_ptr == NULL) {
            return 1; // Malloc failed
        }

        // Second pass: copy matching solves
        current_solves_count = 0;
        for (i = 0; i < state->submission_count && current_solves_count < matched_solves_count; ++i) {
            flg_submission_t *sub = state->submissions[i];
            if (sub != NULL && filter_chal_ptr_value == sub->chal_ptr_value && sub->is_solved_flag != 0) {
                (*solves_array_ptr)[current_solves_count] = sub;
                current_solves_count++;
            }
        }
    }
    return 0; // Success
}

// Function: flg_team_did_solve
unsigned int flg_team_did_solve(flg_state_t *state, int team_id, uintptr_t chal_ptr_value) {
    if (state == NULL || team_id == 0 || chal_ptr_value == 0) {
        return 1; // Invalid parameters
    }

    for (unsigned int i = 0; i < state->submission_count; ++i) {
        flg_submission_t *sub = state->submissions[i];
        if (sub != NULL && sub->is_solved_flag != 0 && team_id == sub->team_id && chal_ptr_value == sub->chal_ptr_value) {
            return 1; // Found a solve by this team for this challenge
        }
    }
    return 0; // Not found
}

// Main function (minimal for compilation)
int main() {
    flg_state_t my_state;
    chal_t chal_example;
    memset(&chal_example, 0, sizeof(chal_example));
    chal_example.name = strdup("ExampleChallenge");
    chal_example.status = 3;

    chal_t *challenges_array[1] = {&chal_example};
    my_state.challenges = challenges_array;
    my_state.chal_count = 1;

    if (flg_init(&my_state, 1, 0) != 0) {
        fprintf(stderr, "flg_init failed\n");
        return 1;
    }

    flg_add_submit(&my_state, 101, "ExampleChallenge");
    // This will return 10 because "AnotherChallenge" is not in my_state.challenges
    flg_add_submit(&my_state, 102, "AnotherChallenge");

    flg_submission_t **solves = NULL;
    unsigned int count = 5;
    flg_get_solves(&my_state, &solves, 0, &count);
    if (solves) free(solves);

    flg_team_did_solve(&my_state, 101, (uintptr_t)&chal_example);

    // Basic cleanup
    for (unsigned int i = 0; i < my_state.submission_count; ++i) {
        if (my_state.submissions[i] != NULL) {
            free(my_state.submissions[i]->flag_string);
            free(my_state.submissions[i]);
        }
    }
    free(my_state.submissions);
    free(chal_example.name);

    return 0;
}