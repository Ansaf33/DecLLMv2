#include <stdio.h>   // For printf, fgets, stdin
#include <string.h>  // For strlen, strcmp, strcpy, strncpy, strcspn, memset
#include <stdlib.h>  // For exit

// Global constants and variables
#define MAX_USERS 100
#define USER_BUFFER_SIZE 0x32e4 // 13028 bytes per user, as observed from 0x32e4 offset

char USERS[MAX_USERS][USER_BUFFER_SIZE];
int NUM_USERS = 0;
int CURRENT_USER = -1; // -1 indicates no user logged in

// Helper function to replace _terminate
void _terminate(void) {
    fprintf(stderr, "Fatal error, terminating.\n");
    exit(1);
}

// Function: CreateUser
void CreateUser(void) {
    char username_buffer[16];
    int found_empty_slot = -1;

    // Initialize the buffer to ensure it's clean before input
    memset(username_buffer, 0, sizeof(username_buffer));

    printf("username: ");

    // Read user input. fgets is safer than read_until, handling buffer overflow.
    if (fgets(username_buffer, sizeof(username_buffer), stdin) == NULL) {
        _terminate(); // Error reading input
    }

    // Remove trailing newline character if present
    username_buffer[strcspn(username_buffer, "\n")] = 0;

    // Check if the username is empty
    if (strlen(username_buffer) == 0) {
        printf("[-] Username cannot be empty.\n");
        return;
    }

    // Loop through existing users to find an empty slot or check for existing username
    for (int i = 0; i < NUM_USERS; i++) {
        // Check if this slot is empty (first character is null).
        // This implies a user can be "deleted" by clearing their username.
        if (USERS[i][0] == '\0') {
            found_empty_slot = i; // Found a re-usable slot
        }

        // Check if username already exists
        if (strcmp(USERS[i], username_buffer) == 0) {
            printf("[-] Error: User already exists\n");
            return;
        }
    }

    // If an empty slot was found, use it
    if (found_empty_slot != -1) {
        // Clear the old content and copy the new username
        memset(USERS[found_empty_slot], 0, USER_BUFFER_SIZE);
        // Use strncpy for safety to prevent buffer overflows
        strncpy(USERS[found_empty_slot], username_buffer, USER_BUFFER_SIZE - 1);
        USERS[found_empty_slot][USER_BUFFER_SIZE - 1] = '\0'; // Ensure null termination
        printf("[+] User '%s' created in slot %d\n", username_buffer, found_empty_slot);
    } else { // No empty slot found, try to create a new user at the end
        if (NUM_USERS < MAX_USERS) {
            // Clear the new slot and copy the username
            memset(USERS[NUM_USERS], 0, USER_BUFFER_SIZE);
            strncpy(USERS[NUM_USERS], username_buffer, USER_BUFFER_SIZE - 1);
            USERS[NUM_USERS][USER_BUFFER_SIZE - 1] = '\0';
            printf("[+] User '%s' created in new slot %d\n", username_buffer, NUM_USERS);
            NUM_USERS++;
        } else {
            printf("[-] Max users already created\n");
        }
    }
}

// Function: Login
void Login(void) {
    char username_buffer[16];

    // Initialize the buffer
    memset(username_buffer, 0, sizeof(username_buffer));

    printf("username: ");

    // Read user input
    if (fgets(username_buffer, sizeof(username_buffer), stdin) == NULL) {
        _terminate(); // Error reading input
    }

    // Remove trailing newline character if present
    username_buffer[strcspn(username_buffer, "\n")] = 0;

    // Check if the username is empty
    if (strlen(username_buffer) == 0) {
        printf("[-] Username cannot be empty.\n");
        return;
    }

    int user_idx;
    for (user_idx = 0; user_idx < NUM_USERS; user_idx++) {
        if (strcmp(USERS[user_idx], username_buffer) == 0) {
            break; // User found
        }
    }

    if (user_idx == NUM_USERS) { // Loop completed without finding user
        printf("Login Failed\n");
    } else { // User found at index user_idx
        CURRENT_USER = user_idx;
        printf("Login Success\n");
    }
}

// Minimal main function to demonstrate functionality
int main() {
    printf("Welcome to the User Management System!\n");
    printf("1. Create User\n");
    printf("2. Login\n");
    printf("3. Exit\n");

    int choice;
    while (1) {
        printf("\nEnter choice: ");
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Please enter a number.\n");
            // Clear stdin buffer after invalid scanf input
            while (getchar() != '\n');
            continue;
        }
        // Consume the newline character left by scanf before subsequent fgets calls
        while (getchar() != '\n');

        switch (choice) {
            case 1:
                CreateUser();
                break;
            case 2:
                Login();
                if (CURRENT_USER != -1) {
                    printf("Currently logged in as user %d: %s\n", CURRENT_USER, USERS[CURRENT_USER]);
                }
                break;
            case 3:
                printf("Exiting...\n");
                return 0;
            default:
                printf("Invalid choice. Please try again.\n");
        }
        printf("Current users in system: %d\n", NUM_USERS);
        if (CURRENT_USER != -1) {
            printf("Current logged in user: %s (index %d)\n", USERS[CURRENT_USER], CURRENT_USER);
        } else {
            printf("No user logged in.\n");
        }
    }

    return 0;
}