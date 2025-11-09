#include <stdio.h>
#include <string.h>
#include <stdlib.h> // For malloc, free

// Type definition for undefined4, assuming it's a 4-byte unsigned integer
typedef unsigned int undefined4;

// Define the User struct based on observed offsets and sizes:
// username[15] (0x0-0xE)
// password[15] (0xF-0x1D)
// Padding (0x1E-0x1F) to align 'id' to 4-byte boundary for 0x20
// id (0x20-0x23)
// status (0x24-0x27)
// some_field (0x28-0x2B)
// Total size: 0x2C (44 bytes)
typedef struct User {
    char username[15];
    char password[15];
    char _padding[2]; // Explicit padding for alignment
    int id;
    int status;
    int some_field;
} User;

// Global variables
// Assuming max 10 users as per the create_user logic (user_count < 10)
User *listOfUsers[10];
int user_count = 0;
User *current_user = NULL; // Use User* for the current logged-in user

// Placeholder for receive_until.
// Reads characters into buf until 'terminator', EOF, or (max_len - 1) chars are read.
// Returns the number of characters read (excluding the null terminator).
// If input is longer than max_len-1, it clears the rest of the line.
int receive_until(char *buf, char terminator, int max_len) {
    int i = 0;
    int c;
    while (i < max_len - 1 && (c = getchar()) != EOF && c != terminator) {
        buf[i++] = (char)c;
    }
    // If the buffer was filled before the terminator, clear the rest of the line.
    // Or if the terminator was not newline, clear up to newline.
    if (c != EOF && c != '\n' && c != terminator) {
        while ((c = getchar()) != EOF && c != '\n');
    }
    buf[i] = '\0'; // Null-terminate the string
    return i;
}

// Function: login
// param_1: username, param_2: password
// Returns 1 on successful login, 0 otherwise.
int login(char *param_1, char *param_2) {
    for (int i = 0; i < user_count; ++i) {
        if (strcmp(param_1, listOfUsers[i]->username) == 0 &&
            strcmp(param_2, listOfUsers[i]->password) == 0) {
            current_user = listOfUsers[i];
            return 1; // Login successful
        }
    }
    return 0; // Login failed
}

// Function: logout
// Returns 1 on successful logout, 0 if no user was logged in.
int logout(void) {
    if (current_user != NULL) {
        current_user->status = 0; // Set user status to logged out/inactive
        current_user = NULL;      // Clear current_user pointer
        return 1;                 // Logout successful
    }
    return 0; // No user was logged in
}

// Function: get_user_index
// param_1: username
// Returns the index of the user in listOfUsers, or -1 if not found.
int get_user_index(char *param_1) {
    for (int i = 0; i < user_count; ++i) {
        if (strcmp(param_1, listOfUsers[i]->username) == 0) {
            return i; // User found at index i
        }
    }
    return -1; // User not found
}

// Function: create_user
// param_1: If 0, attempts to create a new user. If >= 1, attempts to update an existing user.
void create_user(int param_1) {
    char username_input[sizeof(((User*)0)->username)]; // Buffer for username input
    char password_input[sizeof(((User*)0)->password)]; // Buffer for password input
    int chars_read;

    printf("Username: \n");
    chars_read = receive_until(username_input, '\n', sizeof(username_input));
    // receive_until already null-terminates, but ensure max length safety
    username_input[sizeof(username_input) - 1] = '\0';

    if (chars_read == 0) {
        printf("Username cannot be empty.\n");
        return;
    }

    int user_idx = get_user_index(username_input); // Find if user already exists

    if (param_1 < 1) { // Mode: Create new user
        if (user_idx == -1) { // User does not exist, proceed to create
            if (user_count < 10) { // Check if max users limit is reached
                User *new_user = (User *)malloc(sizeof(User));
                if (new_user == NULL) {
                    perror("Failed to allocate memory for new user");
                    return;
                }
                listOfUsers[user_count] = new_user;

                strncpy(new_user->username, username_input, sizeof(new_user->username) - 1);
                new_user->username[sizeof(new_user->username) - 1] = '\0'; // Ensure null termination

                new_user->id = user_count;
                new_user->status = 1; // Default status: active
                new_user->some_field = 0; // Initialize undefined field

                printf("Password: \n");
                chars_read = receive_until(password_input, '\n', sizeof(password_input));
                password_input[sizeof(password_input) - 1] = '\0'; // Ensure null termination

                strncpy(new_user->password, password_input, sizeof(new_user->password) - 1);
                new_user->password[sizeof(new_user->password) - 1] = '\0'; // Ensure null termination

                user_count++;
                printf("User '%s' created successfully.\n", new_user->username);
            } else {
                printf("Maximum number of users reached.\n");
            }
        } else {
            printf("User '%s' already exists. Cannot create a duplicate.\n", username_input);
        }
    } else { // Mode: Update existing user (param_1 >= 1)
        if (user_idx != -1) { // User exists, proceed to update
            listOfUsers[user_idx]->status = 1; // Set status to active
            printf("Password: \n");
            chars_read = receive_until(password_input, '\n', sizeof(password_input));
            password_input[sizeof(password_input) - 1] = '\0'; // Ensure null termination

            // Clear existing password with memset (standard C equivalent of bzero)
            memset(listOfUsers[user_idx]->password, 0, sizeof(listOfUsers[user_idx]->password));
            strncpy(listOfUsers[user_idx]->password, password_input, sizeof(listOfUsers[user_idx]->password) - 1);
            listOfUsers[user_idx]->password[sizeof(listOfUsers[user_idx]->password) - 1] = '\0'; // Ensure null termination
            printf("User '%s' updated successfully.\n", listOfUsers[user_idx]->username);
        } else {
            printf("User '%s' not found for update.\n", username_input);
        }
    }
}

// Function: list_users
void list_users(void) {
    if (user_count == 0) {
        printf("No users registered.\n");
        return;
    }
    printf("--- Registered Users ---\n");
    for (int i = 0; i < user_count; ++i) {
        // Correct printf format specifiers and access struct members directly
        printf("%s -- %s (ID: %d, Status: %d)\n",
               listOfUsers[i]->username,
               listOfUsers[i]->password, // Note: printing password directly is insecure in a real system
               listOfUsers[i]->id,
               listOfUsers[i]->status);
    }
    printf("------------------------\n");
}

// Main function to demonstrate functionality
int main() {
    printf("Welcome to the User Management System!\n");

    // Test create_user
    printf("\n--- Creating Users ---\n");
    printf("Enter details for user1:\n");
    create_user(0); // Create a new user
    printf("Enter details for user2:\n");
    create_user(0); // Create another new user
    printf("Enter details for user3:\n");
    create_user(0); // Create a third user
    printf("Enter details for user1 (should report existing):\n");
    create_user(0); // Try creating user with existing username (should fail if param_1=0)
    printf("Enter details for user1 (should update password):\n");
    create_user(1); // Try updating a user (should succeed if param_1=1)

    // Test list_users
    printf("\n--- Listing Users ---\n");
    list_users();

    // Test login
    printf("\n--- Logging In ---\n");
    char login_user1[] = "testuser1"; // Assuming input "testuser1" was given
    char login_pass1[] = "newpass1";  // Assuming input "newpass1" was given for update
    char login_user2[] = "testuser2";
    char login_pass2[] = "pass2";

    printf("Attempting to login as '%s' with password '%s'\n", login_user1, login_pass1);
    if (login(login_user1, login_pass1)) {
        printf("Login successful! Current user: %s\n", current_user->username);
    } else {
        printf("Login failed.\n");
    }

    printf("Attempting to login as '%s' with password '%s'\n", login_user2, login_pass2);
    if (login(login_user2, login_pass2)) {
        printf("Login successful! Current user: %s\n", current_user->username);
    } else {
        printf("Login failed.\n");
    }

    // Test get_user_index
    printf("\n--- Getting User Index ---\n");
    int idx = get_user_index(login_user1);
    if (idx != -1) {
        printf("Index of '%s': %d\n", login_user1, idx);
    } else {
        printf("User '%s' not found.\n", login_user1);
    }

    // Test logout
    printf("\n--- Logging Out ---\n");
    if (current_user != NULL) {
        printf("Logging out user: %s\n", current_user->username);
        logout();
        printf("Logout successful. Current user is now NULL.\n");
    } else {
        printf("No user logged in to logout.\n");
    }

    // Free allocated memory for all users
    for (int i = 0; i < user_count; ++i) {
        free(listOfUsers[i]);
    }

    return 0;
}