#include <stdio.h>    // For printf, getline, stdin
#include <stdlib.h>   // For malloc, free, exit
#include <string.h>   // For memset, memcpy, strchr, strncpy
#include <unistd.h>   // For _exit (if _terminate is meant to be a direct system exit)

// Assuming 32-bit compilation based on the size 0x7c (124 bytes) for Ingredient
// and the offsets used in the original snippet.
typedef struct Ingredient {
    char name[20];         // Stores up to 19 characters + null terminator. Offset 0x0.
    char measurement[100]; // Stores up to 99 characters + null terminator. Offset 0x14.
                           // (After 20 bytes for name, this implies 4 bytes padding or alignment)
    struct Ingredient *next; // Pointer to the next ingredient in a linked list. Offset 0x78.
                             // (Total size 124 bytes (0x7c) fits if pointer is 4 bytes).
} Ingredient;

// Placeholder for the main recipe structure, inferred from param_1 + 0xcc.
// The original code treated param_1 as a base address and accessed an Ingredient*
// at offset 0xcc. We define a struct to represent this.
typedef struct Recipe {
    // Placeholder for other members that might exist before the head_ingredient pointer.
    // This padding ensures that head_ingredient is at the correct offset (0xcc).
    char _padding_before_head[0xcc - sizeof(Ingredient*)];
    Ingredient *head_ingredient; // Pointer to the first ingredient in the list.
} Recipe;

// Function prototype for _terminate, replacing the decompiler's artifact.
// Assuming it's a simple program termination.
void _terminate() {
    exit(1);
}

// Function prototype for split_ingredient.
// It takes the input line and two buffers for measurement and ingredient name.
// Buffers are assumed to be large enough (e.g., 1024 bytes as in original code).
void split_ingredient(char *line, char *measurement_buf, char *ingredient_name_buf);

// Function: get_ingredients
// param_1 is assumed to be a pointer to a Recipe structure.
int get_ingredients(Recipe *recipe_ptr) {
    char *line_buffer = NULL;       // Buffer for getline, will be allocated by getline
    size_t line_buffer_len = 0;     // Current size of line_buffer
    char ingredient_name_buf[1024]; // Temporary buffer for parsed ingredient name
    char measurement_buf[1024];     // Temporary buffer for parsed measurement

    Ingredient *current_ingredient = NULL; // Pointer to the current ingredient node being processed
    int ingredient_count = 0;               // Counter for ingredients

    printf("Enter the measurement and ingredients, one per line. A blank line ends.\n\n");

    // Read the first line of input
    ssize_t chars_read = getline(&line_buffer, &line_buffer_len, stdin);

    // If the first line is blank or an error occurred, return 0 ingredients.
    // A blank line (just newline) results in chars_read == 1.
    if (chars_read < 2) {
        ingredient_count = 0;
    } else {
        // Allocate memory for the first ingredient structure
        current_ingredient = (Ingredient *)malloc(sizeof(Ingredient));
        if (current_ingredient == NULL) {
            printf("unable to malloc memory\n");
            free(line_buffer); // Free the buffer allocated by getline
            _terminate();
        }
        // Store the pointer to the first ingredient in the Recipe structure
        recipe_ptr->head_ingredient = current_ingredient;

        // Loop as long as valid lines are being read (not blank or error)
        while (chars_read >= 2) {
            // Clear temporary buffers before parsing the new line
            memset(ingredient_name_buf, 0, sizeof(ingredient_name_buf));
            memset(measurement_buf, 0, sizeof(measurement_buf));

            // Split the input line into measurement and ingredient name
            split_ingredient(line_buffer, measurement_buf, ingredient_name_buf);

            // Copy the parsed data into the current ingredient node
            // strncpy is used to prevent buffer overflows and ensure null-termination.
            strncpy(current_ingredient->name, ingredient_name_buf, sizeof(current_ingredient->name) - 1);
            current_ingredient->name[sizeof(current_ingredient->name) - 1] = '\0'; // Ensure null-termination

            strncpy(current_ingredient->measurement, measurement_buf, sizeof(current_ingredient->measurement) - 1);
            current_ingredient->measurement[sizeof(current_ingredient->measurement) - 1] = '\0'; // Ensure null-termination

            current_ingredient->next = NULL; // Initialize next pointer for the current node

            ingredient_count++; // Increment the count of ingredients

            // Read the next line of input
            chars_read = getline(&line_buffer, &line_buffer_len, stdin);

            // If another non-blank line was read, prepare for the next ingredient
            if (chars_read >= 2) {
                Ingredient *next_ingredient = (Ingredient *)malloc(sizeof(Ingredient));
                if (next_ingredient == NULL) {
                    printf("unable to malloc\n");
                    free(line_buffer); // Free getline buffer
                    // In a real application, you might want to free all previously
                    // allocated Ingredient nodes before exiting, but following the
                    // original snippet's behavior, we just terminate.
                    _terminate();
                }
                current_ingredient->next = next_ingredient; // Link the current node to the new one
                current_ingredient = next_ingredient;       // Move to the new node to process it
            }
        }
    }

    free(line_buffer); // Free the buffer allocated by getline at the end

    return ingredient_count;
}

// --- Implementation of split_ingredient function ---
// This function attempts to split a line like "2 cups flour" into "2 cups" (measurement)
// and "flour" (ingredient name). It handles cases where no space is found.
void split_ingredient(char *line, char *measurement_buf, char *ingredient_name_buf) {
    char *space_pos = strchr(line, ' '); // Find the first space
    if (space_pos) {
        // Copy the part before the first space as measurement
        size_t measurement_len = space_pos - line;
        if (measurement_len >= 1024) measurement_len = 1023; // Prevent buffer overflow
        strncpy(measurement_buf, line, measurement_len);
        measurement_buf[measurement_len] = '\0'; // Null-terminate

        // Copy the part after the first space as ingredient name
        char *name_start = space_pos + 1;
        size_t name_len = strlen(name_start);
        if (name_len >= 1024) name_len = 1023; // Prevent buffer overflow
        strncpy(ingredient_name_buf, name_start, name_len);
        ingredient_name_buf[name_len] = '\0'; // Null-terminate

        // Remove a trailing newline character if present in the ingredient name
        size_t current_name_len = strlen(ingredient_name_buf);
        if (current_name_len > 0 && ingredient_name_buf[current_name_len - 1] == '\n') {
            ingredient_name_buf[current_name_len - 1] = '\0';
        }
    } else {
        // If no space is found, treat the whole line as the ingredient name
        // and leave the measurement buffer empty.
        measurement_buf[0] = '\0'; // Ensure measurement is an empty string

        size_t line_len = strlen(line);
        if (line_len >= 1024) line_len = 1023; // Prevent buffer overflow
        strncpy(ingredient_name_buf, line, line_len);
        ingredient_name_buf[line_len] = '\0'; // Null-terminate

        // Remove a trailing newline character if present
        size_t current_name_len = strlen(ingredient_name_buf);
        if (current_name_len > 0 && ingredient_name_buf[current_name_len - 1] == '\n') {
            ingredient_name_buf[current_name_len - 1] = '\0';
        }
    }
}

// --- Main function for demonstration and testing ---
int main() {
    // Create a dummy Recipe structure to pass to get_ingredients.
    // We explicitly initialize head_ingredient to NULL.
    Recipe my_recipe = { .head_ingredient = NULL };

    printf("--- Starting ingredient input process ---\n");
    int num_ingredients = get_ingredients(&my_recipe);

    printf("\n--- Input finished. Total ingredients entered: %d ---\n", num_ingredients);

    // Print the collected ingredients to verify and free allocated memory.
    Ingredient *current = my_recipe.head_ingredient;
    int i = 1;
    while (current != NULL) {
        printf("Ingredient %d: Measurement='%s', Name='%s'\n", i++, current->measurement, current->name);
        Ingredient *temp = current; // Keep a pointer to the current node for freeing
        current = current->next;    // Move to the next node
        free(temp);                 // Free the memory for the processed node
    }
    printf("--- All ingredient memory freed ---\n");

    return 0;
}