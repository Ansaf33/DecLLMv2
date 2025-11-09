#include <stdio.h>   // For printf, fgets, stdin, FILE
#include <stdlib.h>  // For malloc, free, exit
#include <string.h>  // For strncpy, strlen

// Assuming a 32-bit target architecture based on malloc(0xd8) (216 bytes)
// and undefined4 (4 bytes).
typedef struct Recipe {
    char title[200];        // Offset 0 (0x0)
    unsigned int status;    // Offset 200 (0xc8)
    char padding[8];        // Offset 204 (0xcc) to 211 (0xd3)
    struct Recipe *next;    // Offset 212 (0xd4)
} Recipe;

// Placeholder for external functions.
// These would be defined elsewhere in a complete program.
int get_ingredients() {
    printf("Please enter ingredients (type 'done' on a new line to finish):\n");
    // Simulate some input. A real implementation would read ingredients.
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        // Remove newline
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
        if (strcmp(buffer, "done") == 0) {
            break;
        }
        printf("Ingredient: %s\n", buffer);
    }
    return 1; // Simulate success
}

void get_instructions() {
    printf("Please enter instructions (type 'done' on a new line to finish):\n");
    // Simulate some input. A real implementation would read instructions.
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        // Remove newline
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
        if (strcmp(buffer, "done") == 0) {
            break;
        }
        printf("Instruction: %s\n", buffer);
    }
}

// Replaces the decompiled _terminate function with a standard exit.
void _terminate() {
    exit(EXIT_FAILURE);
}

// Function: new_recipe
int new_recipe(Recipe **head) {
    Recipe *new_node = NULL;
    Recipe *current_node = NULL; // Represents the newly created node or the one being processed
    Recipe *prev_node = NULL;    // Represents the node before current_node in the list

    // Allocate memory for a new recipe node
    if (*head == NULL) {
        // List is empty, create the first node
        new_node = (Recipe *)malloc(sizeof(Recipe));
        if (new_node == NULL) {
            printf("Failed to allocate memory\n");
            _terminate();
        }
        *head = new_node;
        current_node = new_node;
    } else {
        // List is not empty, find the last node
        current_node = *head;
        while (current_node->next != NULL) {
            current_node = current_node->next;
        }
        // current_node now points to the last node in the existing list.
        prev_node = current_node; // Store the last node as the previous one for the new node.

        new_node = (Recipe *)malloc(sizeof(Recipe));
        if (new_node == NULL) {
            printf("Failed to allocate memory\n");
            _terminate();
        }
        current_node->next = new_node; // Link the new node to the end of the list.
        current_node = new_node;       // current_node now points to the newly allocated node.
    }

    // Initialize the new node
    current_node->status = 0;
    current_node->next = NULL;

    // Get recipe title
    char input_buffer[200];
    size_t bytes_read = 0;

    printf("Enter Title: ");
    if (fgets(input_buffer, sizeof(input_buffer), stdin) == NULL) {
        bytes_read = 0; // Error or EOF
    } else {
        bytes_read = strlen(input_buffer);
        // Remove trailing newline character if present
        if (bytes_read > 0 && input_buffer[bytes_read - 1] == '\n') {
            input_buffer[bytes_read - 1] = '\0';
            bytes_read--;
        }
    }

    // Check if title input was too short (empty or just a single character)
    if (bytes_read < 1) { // Original logic `local_18 < 2` implies empty or single char after stripping newline
        if (current_node == *head) {
            *head = NULL; // If the failed node was the only one, clear the head.
        }
        free(current_node); // Free the newly allocated node.
        if (prev_node != NULL) {
            prev_node->next = NULL; // Unlink the failed node from the previous one.
        }
        return -1; // Indicate failure
    } else {
        // Copy the title to the recipe node
        strncpy(current_node->title, input_buffer, sizeof(current_node->title) - 1);
        current_node->title[sizeof(current_node->title) - 1] = '\0'; // Ensure null-termination

        // Get ingredients
        int ingredients_result = get_ingredients();
        if (ingredients_result == 0) { // get_ingredients failed
            if (current_node == *head) {
                *head = NULL; // If the failed node was the only one, clear the head.
            }
            free(current_node); // Free the newly allocated node.
            if (prev_node != NULL) {
                prev_node->next = NULL; // Unlink the failed node from the previous one.
            }
            return -1; // Indicate failure
        } else {
            // Get instructions
            get_instructions();
            return 0; // Indicate success
        }
    }
}

// Example main function to demonstrate usage
int main() {
    Recipe *recipe_list = NULL; // Head of the linked list

    printf("Creating new recipe 1:\n");
    if (new_recipe(&recipe_list) == 0) {
        printf("Recipe 1 added successfully: %s\n", recipe_list->title);
    } else {
        printf("Failed to add recipe 1.\n");
    }

    printf("\nCreating new recipe 2:\n");
    if (new_recipe(&recipe_list) == 0) {
        // Find the second recipe in the list
        Recipe *temp = recipe_list;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        printf("Recipe 2 added successfully: %s\n", temp->title);
    } else {
        printf("Failed to add recipe 2.\n");
    }

    // Clean up the linked list
    Recipe *current = recipe_list;
    while (current != NULL) {
        Recipe *next_recipe = current->next;
        printf("Freeing recipe: %s\n", current->title);
        free(current);
        current = next_recipe;
    }
    recipe_list = NULL;

    return 0;
}