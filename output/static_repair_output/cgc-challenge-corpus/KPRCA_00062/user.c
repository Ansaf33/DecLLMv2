#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Global variable for user ID generation
static int g_user_id = 0;

// Define User structure based on `create_user` and `print_user` functions
// Total size: 4 (id) + 128 (username) + 512 (about) + 4 (field_a1) + 4 (field_a2) = 652 bytes (0x28c)
typedef struct User {
    int id;
    char username[128];
    char about[512];
    int field_a1;
    int field_a2;
} User;

// Define ListNode structure for the linked list, as implied by `find_user_from_list` traversal
typedef struct ListNode {
    struct ListNode *next;
    User *user_data;
} ListNode;

// --- Stub/Helper Functions (to make the provided code compilable) ---

// Dummy q_and_a function for user input
// param_5 is used as a status pointer: 0 for success, -1 for error/empty input
char *q_and_a(const char *prompt, unsigned int param_2, int param_3, unsigned int param_4, int *status_ptr, int param_6) {
    char buffer[256]; // Max input size
    printf("%s", prompt);
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        if (status_ptr) *status_ptr = -1;
        return NULL;
    }
    buffer[strcspn(buffer, "\n")] = 0; // Remove trailing newline
    if (strlen(buffer) == 0) {
        if (status_ptr) *status_ptr = -1; // Indicate empty input
        return NULL;
    }
    if (status_ptr) *status_ptr = 0; // Indicate success
    return strdup(buffer); // Duplicate string to heap, caller must free
}

// Dummy popfront function for a list of User pointers
// Removes the first node from the list, frees the node, and returns the User data pointer
void *popfront(ListNode **list_head_ptr) {
    if (!list_head_ptr || !*list_head_ptr) {
        return NULL;
    }
    ListNode *node = *list_head_ptr;
    *list_head_ptr = node->next;
    User *user = node->user_data;
    free(node); // Free the list node itself
    return user; // Return the user data pointer for the caller to free
}

// Dummy find function
// This stub simulates searching the list for a user by username.
// Returns a pointer to the User if found, NULL otherwise.
User *find(ListNode *list_head, const char *search_key, int (*compare_func)(const void*, const void*)) {
    ListNode *current = list_head;
    while (current != NULL) {
        if (current->user_data && strcmp(current->user_data->username, search_key) == 0) {
            return current->user_data;
        }
        current = current->next;
    }
    return NULL; // Not found
}

// Dummy insert_in_order function
// Inserts a User into a sorted linked list based on the comparison function.
// Returns true on success, false on failure (e.g., allocation error).
bool insert_in_order(ListNode **list_head_ptr, User *user_to_insert, int (*compare_func)(const void*, const void*), int p4, int p5, int p6, int p7) {
    if (!list_head_ptr || !user_to_insert) {
        return false;
    }

    ListNode *new_node = (ListNode *)malloc(sizeof(ListNode));
    if (!new_node) {
        perror("Failed to allocate list node");
        return false;
    }
    new_node->user_data = user_to_insert;
    new_node->next = NULL;

    // Insert at head if list is empty or new user is smaller than head
    if (*list_head_ptr == NULL || compare_func(user_to_insert, (*list_head_ptr)->user_data) < 0) {
        new_node->next = *list_head_ptr;
        *list_head_ptr = new_node;
    } else {
        // Traverse to find the correct insertion point
        ListNode *current = *list_head_ptr;
        while (current->next != NULL && compare_func(user_to_insert, current->next->user_data) >= 0) {
            current = current->next;
        }
        new_node->next = current->next;
        current->next = new_node;
    }
    return true;
}

// Dummy print function for a list of User pointers
void print(ListNode *list_head, void (*print_func)(User*)) {
    ListNode *current = list_head;
    while (current != NULL) {
        if (current->user_data) {
            print_func(current->user_data);
        }
        current = current->next;
    }
}

// --- Original Functions (fixed and refactored) ---

// Function: create_user
// Allocates and initializes a new User struct.
User *create_user(char *username, char *about_text) {
    if (username && strlen(username) != 0 && about_text && strlen(about_text) != 0) {
        User *newUser = (User *)calloc(1, sizeof(User));
        if (!newUser) {
            perror("Failed to allocate user");
            return NULL;
        }
        newUser->id = g_user_id++; // Assign ID and then increment global counter
        strncpy(newUser->username, username, sizeof(newUser->username) - 1);
        newUser->username[sizeof(newUser->username) - 1] = '\0';
        strncpy(newUser->about, about_text, sizeof(newUser->about) - 1);
        newUser->about[sizeof(newUser->about) - 1] = '\0';
        newUser->field_a1 = 0;
        newUser->field_a2 = 0;
        return newUser;
    }
    return NULL;
}

// Function: delete_user
// Frees the memory pointed to by `param_1` and sets the pointer to NULL.
bool delete_user(void **param_1) {
    if (*param_1 != NULL) {
        free(*param_1);
        *param_1 = NULL;
        return true;
    }
    return false;
}

// Function: compare_users
// Comparison function for User structs, typically used for sorting (e.g., by username).
// Takes `const void*` pointers, casts them to `User*`, and compares usernames.
int compare_users(const void *param_1, const void *param_2) {
    if (!param_1 || !param_2) {
        return -1; // Indicate invalid comparison
    }
    const User *user1 = (const User *)param_1;
    const User *user2 = (const User *)param_2;
    return strcmp(user1->username, user2->username);
}

// Function: print_user
// Prints detailed information about a User.
void print_user(User *param_1) {
    if (!param_1) {
        printf("Invalid user data.\n");
        return;
    }
    printf("User id = %d\n", param_1->id);
    printf("Username is \"%s\"\n", param_1->username);
    printf("About %s: %s\n", param_1->username, param_1->about);
    printf("---------------------\n");
}

// Function: print_simple
// Prints brief information about a User.
void print_simple(User *param_1) {
    if (!param_1) {
        printf("Invalid user data.\n");
        return;
    }
    printf("Id=%d, username=%s\n", param_1->id, param_1->username);
}

// Function: clear_list
// Clears a linked list of User pointers by popping and freeing each User struct and its node.
void clear_list(ListNode **list_head_ptr) {
    User *user_to_free;
    while ((user_to_free = (User *)popfront(list_head_ptr)) != NULL) {
        free(user_to_free);
    }
}

// Function: find_user_from_list
// Prompts user for a username, then searches a given list for a matching user.
// Returns a pointer to the found User, or NULL if not found or an error occurred.
User *find_user_from_list(int list_context, unsigned int list_size_limit, int *status, ListNode *search_list_head, int max_iterations) {
    if (list_context == 0 || list_size_limit < 2 || !status) {
        return NULL;
    }

    char *search_username = q_and_a("Enter username: ", list_size_limit, list_context, list_size_limit, status, 1);
    if (!search_username) {
        return NULL;
    }

    if (*status != 0) { // q_and_a indicated an error or empty input
        free(search_username);
        return NULL;
    }

    User *found_user = NULL;

    // Path 1: Linear scan for smaller lists or specific search logic
    if (max_iterations < 101) { // 0x65 is 101
        ListNode *current_node = search_list_head;
        for (int i = 0; i < max_iterations && current_node != NULL; ++i) {
            if (current_node->user_data && strcmp(search_username, current_node->user_data->username) == 0) {
                found_user = current_node->user_data;
                break;
            }
            current_node = current_node->next;
        }
    } else {
        // Path 2: Call a more general `find` function (potentially for a different data structure)
        // Note: The `compare_users` function signature does not match direct `char*` comparison.
        // The `find` stub implements direct `strcmp` for the username.
        found_user = find(search_list_head, search_username, compare_users);
    }

    free(search_username); // Free the dynamically allocated search_username
    return found_user;
}

// Function: add_user_to_list
// Prompts for a username, searches for it in `search_list_head`,
// and if found, attempts to insert that User into `target_list_head`.
// Retries up to 5 times if user not found or insertion fails.
int add_user_to_list(int list_context, unsigned int list_size_limit, int *status, ListNode *search_list_head, ListNode **target_list_head) {
    if (!status) return 0;

    *status = 1; // Default status: not found/not added initially
    int retry_count = 0;

    while (retry_count < 5) {
        User *found_user = find_user_from_list(list_context, list_size_limit, status, search_list_head, 1000);

        if (*status == -1) { // Error during input or internal find
            printf("Error during user input or search. Exiting add_user_to_list.\n");
            return 0;
        }

        if (found_user != NULL) { // User found, attempt to insert it
            if (insert_in_order(target_list_head, found_user, compare_users, 0, 0, 0, 0)) {
                printf("User '%s' found and added to list.\n", found_user->username);
                return 1; // Successfully added
            } else {
                printf("User '%s' found but could not be added (e.g., already exists or insertion error).\n", found_user->username);
            }
        } else { // User not found
            printf("User not found in search list.\n");
        }

        printf("Try again (retry %d/5)\n", retry_count + 1);
        retry_count++;
        *status = 1; // Reset status for next retry if not successful
    }
    printf("Max retries reached. Exiting add_user_to_list.\n");
    return 0; // Max retries reached
}

// Function: print_user_list
// Prints a title, then iterates through the list and prints each user using `print_user`.
void print_user_list(const char *title, ListNode *list_head) {
    printf("%s", title);
    print(list_head, print_user);
}

// --- Main function for testing ---

// Helper to add a user to a simple linked list (used for the global pool)
void add_to_simple_list(ListNode **head, User *user) {
    ListNode *new_node = (ListNode *)malloc(sizeof(ListNode));
    if (new_node) {
        new_node->user_data = user;
        new_node->next = *head; // Prepend
        *head = new_node;
    }
}

int main() {
    ListNode *main_user_list_head = NULL; // The primary list we manage
    ListNode *global_user_pool_head = NULL; // A pool of all created users for `find_user_from_list` to search
    int status_code = 0;

    printf("--- Creating Users ---\n");
    User *user1 = create_user("alice", "Loves Wonderland");
    User *user2 = create_user("bob", "Builder by trade");
    User *user3 = create_user("charlie", "Chocolate factory owner");
    User *user4 = create_user("david", "Likes to explore");

    // Add created users to a global pool so they can be "found" by `find_user_from_list`
    printf("\n--- Adding Created Users to Global Pool ---\n");
    if (user1) { add_to_simple_list(&global_user_pool_head, user1); printf("Added alice to pool.\n"); }
    if (user2) { add_to_simple_list(&global_user_pool_head, user2); printf("Added bob to pool.\n"); }
    if (user3) { add_to_simple_list(&global_user_pool_head, user3); printf("Added charlie to pool.\n"); }
    if (user4) { add_to_simple_list(&global_user_pool_head, user4); printf("Added david to pool.\n"); }

    printf("\n--- Adding Users to Main List from Pool (interactive) ---\n");
    printf("You will be prompted to enter usernames. Try 'alice', 'bob', 'charlie', 'david'.\n");
    printf("Enter username to add to main list: (e.g., alice)\n");
    if (add_user_to_list(0, 0, &status_code, global_user_pool_head, &main_user_list_head)) {
        printf("User added successfully to main list.\n");
    } else {
        printf("Failed to add user to main list.\n");
    }

    printf("Enter another username to add to main list: (e.g., david)\n");
    if (add_user_to_list(0, 0, &status_code, global_user_pool_head, &main_user_list_head)) {
        printf("User added successfully to main list.\n");
    } else {
        printf("Failed to add user to main list.\n");
    }

    printf("\n--- Printing Main User List ---\n");
    print_user_list("Current Main User List:\n", main_user_list_head);

    printf("\n--- Searching for a User (interactive, e.g., 'alice') ---\n");
    User *found = find_user_from_list(1, 10, &status_code, main_user_list_head, 10);
    if (found) {
        printf("Found user: %s (ID: %d)\n", found->username, found->id);
        print_simple(found);
    } else {
        printf("User not found in main list. Status: %d\n", status_code);
    }

    printf("\n--- Searching for a User (interactive, e.g., 'zach') ---\n");
    found = find_user_from_list(1, 10, &status_code, main_user_list_head, 10);
    if (found) {
        printf("Found user: %s (ID: %d)\n", found->username, found->id);
    } else {
        printf("User not found in main list. Status: %d\n", status_code);
    }

    printf("\n--- Deleting a User (user2 - bob) ---\n");
    // Note: This only frees the User struct, not its corresponding ListNode in `main_user_list_head` or `global_user_pool_head`.
    // A robust solution would involve a `delete_user_from_list` function to remove the node as well.
    if (user2 && delete_user((void **)&user2)) {
        printf("User 'bob' data deleted from memory.\n");
    } else {
        printf("Failed to delete user 'bob' data.\n");
    }

    printf("\n--- Printing Main User List after 'bob' data deletion ---\n");
    printf("(Note: 'bob' might still appear in list if its node wasn't removed, but accessing its data would be use-after-free)\n");
    print_user_list("User List after Bob's data deletion:\n", main_user_list_head);

    printf("\n--- Clearing Main User List ---\n");
    clear_list(&main_user_list_head);
    printf("Main user list cleared.\n");

    printf("\n--- Printing Main User List after clearing (should be empty) ---\n");
    print_user_list("User List after clearing:\n", main_user_list_head);

    printf("\n--- Clearing Global User Pool ---\n");
    // Any users still in global_user_pool_head that were not added to main_user_list_head
    // or were added and then freed by clear_list, will now be handled.
    // In this specific test, all users were added to the pool and then to the main list.
    // `clear_list(&main_user_list_head)` already freed the `User` objects.
    // We only need to free the `ListNode`s of the global pool now.
    ListNode *current_pool_node = global_user_pool_head;
    while (current_pool_node != NULL) {
        ListNode *next_node = current_pool_node->next;
        // User data was already freed by `clear_list(&main_user_list_head)`
        // or by `delete_user((void**)&user2)`.
        // So we only free the node itself.
        free(current_pool_node);
        current_pool_node = next_node;
    }
    global_user_pool_head = NULL;
    printf("Global user pool cleared.\n");

    return 0;
}