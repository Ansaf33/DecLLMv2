#include <stdio.h>   // Required for printf in main
#include <stdlib.h>  // Required for malloc, free, exit
#include <string.h>  // Required for strlen, strcpy

// Function: GenerateUsers
// This function dynamically allocates an array of strings (usernames)
// and returns a pointer to this array. The array is NULL-terminated.
char **GenerateUsers(void) {
    // Sample usernames to be generated
    const char *sample_users[] = {
        "Alice",
        "Bob",
        "Charlie",
        "David",
        NULL // Sentinel to mark the end of the sample list
    };

    // Calculate the number of users
    int count = 0;
    for (int i = 0; sample_users[i] != NULL; ++i) {
        count++;
    }

    // Allocate memory for the array of char pointers (+1 for the NULL terminator)
    char **users_array = (char **)malloc((count + 1) * sizeof(char *));
    if (users_array == NULL) {
        perror("Failed to allocate memory for users_array");
        exit(EXIT_FAILURE);
    }

    // Populate the array with dynamically allocated strings
    for (int i = 0; i < count; ++i) {
        size_t len = strlen(sample_users[i]);
        users_array[i] = (char *)malloc((len + 1) * sizeof(char)); // +1 for null terminator
        if (users_array[i] == NULL) {
            perror("Failed to allocate memory for user string");
            // Clean up previously allocated strings before exiting
            for (int j = 0; j < i; ++j) {
                free(users_array[j]);
            }
            free(users_array);
            exit(EXIT_FAILURE);
        }
        strcpy(users_array[i], sample_users[i]);
    }

    users_array[count] = NULL; // Mark the end of the array of pointers

    return users_array;
}

// Main function to demonstrate the usage of GenerateUsers
int main(void) {
    char **users = GenerateUsers();

    printf("Generated Users:\n");
    for (int i = 0; users[i] != NULL; ++i) {
        printf("- %s\n", users[i]);
    }

    // Free the dynamically allocated memory
    for (int i = 0; users[i] != NULL; ++i) {
        free(users[i]); // Free each individual user string
    }
    free(users); // Free the array of pointers itself

    return 0;
}