#include <stdio.h>    // For printf, fgets, perror
#include <string.h>   // For strlen, memset, strcmp, strcpy
#include <stdlib.h>   // For exit, EXIT_FAILURE

// Global variables as inferred from the snippet
#define MAX_TOTAL_USERS 100
// USER_RECORD_SIZE is 0x32e4 (13028 bytes), inferred from the original code.
// This is an unusually large size for a username string, but we maintain it as per the snippet.
#define USER_RECORD_SIZE 0x32e4

char USERS[MAX_TOTAL_USERS][USER_RECORD_SIZE];
int NUM_USERS = 0;      // Current number of active users
int CURRENT_USER = -1;  // Index of the currently logged-in user, -1 if no user is logged in

// Function: CreateUser
void CreateUser(void) {
    char username_buffer[16]; // Buffer for user input (original local_24)
    int available_slot = -1;  // Index of an empty user slot, if found (original local_14)
    int i;                    // Loop counter (original local_10)
    size_t len;               // Length of the entered username (original sVar2)

    // Clear the username buffer before reading input
    memset(username_buffer, 0, sizeof(username_buffer));

    printf("username: ");

    // Read username from stdin.
    // The original `read_until()` with `uStack_38 = 10` implies reading up to 9 chars + null terminator.
    // `fgets` reads up to `sizeof(username_buffer) - 1` characters or until a newline.
    if (fgets(username_buffer, sizeof(username_buffer), stdin) == NULL) {
        perror("Error reading username");
        exit(EXIT_FAILURE); // Equivalent to original `_terminate()`
    }

    // Remove trailing newline character if present
    len = strlen(username_buffer);
    if (len > 0 && username_buffer[len - 1] == '\n') {
        username_buffer[len - 1] = '\0';
        len--; // Adjust length after stripping newline
    }

    // Proceed only if a non-empty username was entered (original `sVar2 != 0`)
    if (len != 0) {
        // Iterate through existing users to find an empty slot or check for duplicates
        for (i = 0; i < NUM_USERS; i++) {
            // Check if the current user slot is empty (indicated by the first character being null)
            // This assumes that a deleted or available slot is zeroed out.
            if (USERS[i][0] == '\0') {
                available_slot = i;
            }

            // Check if the entered username already exists
            // Original `strmatch()` returns non-zero for match, 0 for no match.
            // `strcmp()` returns 0 for match, non-zero otherwise.
            if (strcmp(username_buffer, USERS[i]) == 0) {
                printf("[-] Error user exists\n");
                return; // Exit if user already exists
            }
        }

        // If no available slot was found, try to append a new user
        if (available_slot == -1) {
            if (NUM_USERS < MAX_TOTAL_USERS) {
                // Clear the new user's record area
                memset(USERS[NUM_USERS], 0, USER_RECORD_SIZE);
                // Copy the username into the new slot
                strcpy(USERS[NUM_USERS], username_buffer);
                NUM_USERS++; // Increment the total number of users
            } else {
                printf("[-] Max users already created\n");
            }
        } else { // An available slot was found, reuse it
            // Clear the existing slot
            memset(USERS[available_slot], 0, USER_RECORD_SIZE);
            // Copy the username into the reused slot
            strcpy(USERS[available_slot], username_buffer);
            // NUM_USERS does not increment as we are reusing an existing slot
        }
    }
    return;
}

// Function: Login
void Login(void) {
    char username_buffer[16]; // Buffer for user input (original local_20)
    int i;                    // Loop counter (original local_10)
    size_t len;               // Length of the entered username (original sVar2)

    // Clear the username buffer before reading input
    memset(username_buffer, 0, sizeof(username_buffer));

    printf("username: ");

    // Read username from stdin
    if (fgets(username_buffer, sizeof(username_buffer), stdin) == NULL) {
        perror("Error reading username");
        exit(EXIT_FAILURE); // Equivalent to original `_terminate()`
    }

    // Remove trailing newline character if present
    len = strlen(username_buffer);
    if (len > 0 && username_buffer[len - 1] == '\n') {
        username_buffer[len - 1] = '\0';
        len--; // Adjust length after stripping newline
    }

    // Proceed only if a non-empty username was entered (original `sVar2 != 0`)
    if (len != 0) {
        // Iterate through existing users to find a match
        for (i = 0; i < NUM_USERS; i++) {
            // Check if the entered username matches an existing user
            // Original `strmatch()` returns non-zero for match, 0 for no match.
            // `strcmp()` returns 0 for match, non-zero otherwise.
            if (strcmp(username_buffer, USERS[i]) == 0) {
                break; // Match found, exit loop
            }
        }

        // If loop finished without finding a match (i.e., i equals NUM_USERS)
        if (i == NUM_USERS) {
            printf("Login Failed\n");
        } else { // Match found at index `i`
            CURRENT_USER = i; // Set the current logged-in user
            printf("Login Success\n");
        }
    } else {
        printf("Login Failed\n"); // Explicitly fail for empty input
    }
    return;
}