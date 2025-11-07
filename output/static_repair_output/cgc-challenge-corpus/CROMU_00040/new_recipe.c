#include <stdio.h>    // For printf, getline, stdin, NULL
#include <stdlib.h>   // For malloc, free, exit, EXIT_FAILURE
#include <string.h>   // For strncpy
#include <stdint.h>   // For uint32_t, uintptr_t
#include <unistd.h>   // For ssize_t (used by getline)

// Forward declarations for functions not provided in the snippet
// Assuming they exist and have these signatures
int get_ingredients();
void get_instructions();

// Define the Recipe structure based on the memory access patterns from the snippet.
// The original code implies a 32-bit architecture where pointers are 4 bytes.
// For compatibility on 64-bit Linux, we need to ensure the memory layout matches
// the offsets (200 for field_200, 212 for next pointer) and total size (216 bytes, 0xd8).
// Using __attribute__((packed)) helps enforce the offsets and prevents compiler padding.
// The `next` pointer field is stored as a `uint32_t` and cast to/from `Recipe*`
// to maintain the 4-byte size implied by the original code's `malloc` and offsets.

#define TITLE_MAX_LEN 200

typedef struct Recipe {
    char title[TITLE_MAX_LEN]; // 0x00 - 0xc7 (200 bytes)
    uint32_t field_200;        // 0xc8 - 0xcb (Offset 200, 4 bytes)
    char padding_to_next[8];   // 0xcc - 0xd3 (Padding to reach offset 212)
    uint32_t next_ptr_storage; // 0xd4 - 0xd7 (Offset 212, 4 bytes to store a pointer value)
} __attribute__((packed)) Recipe;

// Helper function to safely retrieve the next Recipe pointer from storage.
static inline Recipe* get_next_recipe_ptr(Recipe* r) {
    // Cast uint32_t storage to uintptr_t, then to Recipe*
    return (Recipe*)(uintptr_t)r->next_ptr_storage;
}

// Helper function to safely store a Recipe pointer.
static inline void set_next_recipe_ptr(Recipe* r, Recipe* next_r) {
    // Cast Recipe* to uintptr_t, then to uint32_t for storage
    r->next_ptr_storage = (uint32_t)(uintptr_t)next_r;
}

// Function: new_recipe
// Original return type was undefined4, assuming it's an int. 0 for success, -1 for failure.
int new_recipe(Recipe **head_param) {
    Recipe *current_recipe = NULL;
    Recipe *prev_recipe = NULL;
    
    // Allocate memory for the new recipe node, using the exact size inferred (0xd8 = 216 bytes)
    current_recipe = (Recipe *)malloc(sizeof(Recipe));
    if (current_recipe == NULL) {
        printf("Failed to allocate memory\n");
        exit(EXIT_FAILURE); // Terminate the program on critical memory allocation failure
    }

    // Initialize fields of the new recipe node
    current_recipe->field_200 = 0;
    set_next_recipe_ptr(current_recipe, NULL); // Set next pointer to NULL

    // Find the end of the linked list or set as head
    if (*head_param == NULL) {
        *head_param = current_recipe; // New node is the head
    } else {
        prev_recipe = *head_param;
        // Iterate through the list until the next pointer is NULL
        while (get_next_recipe_ptr(prev_recipe) != NULL) {
            prev_recipe = get_next_recipe_ptr(prev_recipe);
        }
        set_next_recipe_ptr(prev_recipe, current_recipe); // Append new node
    }

    printf("Enter Title: ");
    
    // Use getline for robust input handling. It allocates buffer, so it needs to be freed.
    char *line = NULL;
    size_t len = 0;
    ssize_t chars_read = getline(&line, &len, stdin);

    // Check if input was too short (e.g., just a newline or empty)
    if (chars_read < 2) { // Minimum 1 char + newline, so < 2 means empty or just newline
        printf("Title cannot be empty.\n");
        if (*head_param == current_recipe) { // If this was the head, reset the head
            *head_param = NULL;
        } else if (prev_recipe != NULL) { // If it was appended, detach it from the previous node
            set_next_recipe_ptr(prev_recipe, NULL);
        }
        free(current_recipe); // Free the newly allocated node
        free(line);           // Free buffer allocated by getline
        return -1;            // Indicate failure
    }

    // Remove trailing newline character if present
    if (line[chars_read - 1] == '\n') {
        line[chars_read - 1] = '\0';
    } else {
        line[chars_read] = '\0'; // Ensure null termination if no newline (e.g., input buffer full)
    }

    // Copy the title to the recipe struct, ensuring it doesn't exceed TITLE_MAX_LEN
    strncpy(current_recipe->title, line, TITLE_MAX_LEN);
    current_recipe->title[TITLE_MAX_LEN - 1] = '\0'; // Guarantee null-termination in case of truncation
    free(line); // Free buffer allocated by getline

    // Call get_ingredients and check its return value
    if (get_ingredients() == 0) { // Assuming 0 indicates failure
        printf("Failed to get ingredients.\n");
        if (*head_param == current_recipe) { // If this was the head, reset the head
            *head_param = NULL;
        } else if (prev_recipe != NULL) { // If it was appended, detach it
            set_next_recipe_ptr(prev_recipe, NULL);
        }
        free(current_recipe); // Free the node
        return -1;            // Indicate failure
    }

    // Call get_instructions
    get_instructions();

    return 0; // Indicate success
}

// Dummy implementations for get_ingredients and get_instructions
// These functions are placeholders for the actual logic in the original program.
int get_ingredients() {
    printf("Getting ingredients...\n");
    char input[10];
    printf("Enter 1 to continue with ingredients, 0 to cancel: ");
    // Read user input to simulate success/failure
    if (fgets(input, sizeof(input), stdin) != NULL && atoi(input) == 1) {
        return 1; // Simulate success
    }
    return 0; // Simulate failure
}

void get_instructions() {
    printf("Getting instructions...\n");
    // Simulate some instructions being added
    printf("Instructions added.\n");
}

// Simple main function to demonstrate the usage of new_recipe
int main() {
    Recipe *recipe_list = NULL; // Head of the linked list of recipes

    printf("--- Creating first recipe ---\n");
    if (new_recipe(&recipe_list) == 0) {
        printf("First recipe created successfully: %s\n", recipe_list->title);
    } else {
        printf("Failed to create first recipe.\n");
    }

    printf("\n--- Creating second recipe ---\n");
    if (new_recipe(&recipe_list) == 0) {
        // Find the second recipe to print its title (assuming it was appended)
        Recipe *temp = recipe_list;
        while (temp != NULL && get_next_recipe_ptr(temp) != NULL) {
            temp = get_next_recipe_ptr(temp);
        }
        if (temp != NULL) {
             printf("Second recipe created successfully: %s\n", temp->title);
        }
    } else {
        printf("Failed to create second recipe.\n");
    }

    // Clean up: Free all allocated memory in the linked list
    Recipe *current = recipe_list;
    while (current != NULL) {
        Recipe *next_node = get_next_recipe_ptr(current); // Get next before freeing current
        printf("Freeing recipe: %s\n", current->title);
        free(current);
        current = next_node;
    }

    return 0;
}