#include <stdio.h>   // For printf, getline, FILE, stdin
#include <stdlib.h>  // For getline, free, size_t, ssize_t, NULL
#include <string.h>  // For strstr (used in match_str stub), NULL

// --- STUB DEFINITIONS (to make the code compilable) ---
// Assuming these structures based on the offsets observed in the original snippet.
// The actual sizes and content of other fields would depend on the full program.
typedef struct Ingredient {
    char name[64]; // Example size for ingredient name, offset 0x14 from Ingredient start
    // ... other fields for Ingredient ...
    struct Ingredient *next_ingredient; // Offset 0x78 from Ingredient start
} Ingredient;

typedef struct Recipe {
    char name[100]; // Example size for recipe name (main field to match against)
    // ... other fields for Recipe ...
    int is_tagged;           // At offset 200 (0xC8)
    Ingredient *ingredients; // At offset 204 (0xCC)
    struct Recipe *next;     // At offset 212 (0xD4)
} Recipe;

// Stub for match_str: returns 0 on a successful match, non-zero otherwise.
// This implements a simple case-sensitive substring search.
int match_str(char *search_term, const char *text) {
    if (search_term == NULL || text == NULL) {
        return 1; // No match if either is null
    }
    return (strstr(text, search_term) != NULL) ? 0 : 1;
}

// Stub for print_recipe
void print_recipe(Recipe *recipe) {
    if (recipe) {
        printf("  Recipe found: %s (Currently Tagged: %s)\n", recipe->name, recipe->is_tagged ? "Yes" : "No");
    } else {
        printf("  (NULL Recipe)\n");
    }
}

// --- END STUB DEFINITIONS ---

// Function: find_recipe
void find_recipe(Recipe *head_recipe) { // Changed param_1 type to Recipe* for clarity
  char *search_term_buffer = NULL;
  size_t search_term_len = 0;
  ssize_t read_bytes;

  printf("Enter search term: ");
  read_bytes = getline(&search_term_buffer, &search_term_len, stdin);

  if (read_bytes > 1) { // If something meaningful was read (more than just a newline)
    // Remove trailing newline character if present
    if (search_term_buffer[read_bytes - 1] == '\n') {
        search_term_buffer[read_bytes - 1] = '\0';
    }

    printf("\nSearching for '%s'...\n", search_term_buffer);
    Recipe *current_recipe = head_recipe; // Use a more descriptive variable name
    while (current_recipe != NULL) {
      int recipe_matched = 0;

      // First, check if the recipe's main name/field matches the search term
      if (match_str(search_term_buffer, current_recipe->name) == 0) {
        recipe_matched = 1;
      } else {
        // If no direct recipe match, iterate through its ingredients
        Ingredient *current_ingredient = current_recipe->ingredients;
        while (current_ingredient != NULL) {
          if (match_str(search_term_buffer, current_ingredient->name) == 0) {
            recipe_matched = 1;
            break; // Found an ingredient match, no need to check further ingredients for this recipe
          }
          current_ingredient = current_ingredient->next_ingredient;
        }
      }

      if (recipe_matched) {
        print_recipe(current_recipe);
        printf("  Would you like to tag this recipe? (y/N): ");

        char *tag_input_buffer = NULL;
        size_t tag_input_len = 0;
        ssize_t tag_read_bytes = getline(&tag_input_buffer, &tag_input_len, stdin);

        if (tag_read_bytes > 0) { // If input was provided
          // Check the first character for 'y' or 'Y'
          if (tag_input_buffer[0] == 'y' || tag_input_buffer[0] == 'Y') {
            current_recipe->is_tagged = 1; // Directly access struct member
            printf("  Recipe '%s' tagged.\n", current_recipe->name);
          }
        }
        free(tag_input_buffer); // Free memory allocated by getline for tag input
      }
      current_recipe = current_recipe->next; // Always advance to the next recipe
    }
  } else if (read_bytes == -1) {
      perror("Failed to read search term");
  } else {
      printf("No search term entered.\n");
  }

  free(search_term_buffer); // Free memory allocated by getline for search term
  return;
}

// --- Minimal main function for compilation and testing ---
int main() {
    // Create some dummy data for recipes and ingredients
    Ingredient ing1 = {"Flour", NULL};
    Ingredient ing2 = {"Sugar", NULL};
    Ingredient ing3 = {"Eggs", NULL};
    Ingredient ing4 = {"Butter", NULL};
    Ingredient ing5 = {"Milk", NULL};
    Ingredient ing6 = {"Chocolate", NULL};

    // Recipe 1: Pancakes
    Recipe recipe1 = {"Pancakes", 0, &ing1, NULL};
    ing1.next_ingredient = &ing2;
    ing2.next_ingredient = &ing5; // Flour, Sugar, Milk

    // Recipe 2: Chocolate Cake
    Recipe recipe2 = {"Chocolate Cake", 0, &ing1, NULL};
    ing1.next_ingredient = &ing2;
    ing2.next_ingredient = &ing3;
    ing3.next_ingredient = &ing4;
    ing4.next_ingredient = &ing6; // Flour, Sugar, Eggs, Butter, Chocolate

    // Recipe 3: Omelette
    Recipe recipe3 = {"Omelette", 0, &ing3, NULL}; // Eggs

    // Link recipes into a list
    recipe1.next = &recipe2;
    recipe2.next = &recipe3;
    recipe3.next = NULL; // End of list

    printf("--- Starting recipe search ---\n");
    find_recipe(&recipe1); // Pass the head of the recipe list
    printf("--- Search complete ---\n");

    // Print tagged status to verify changes
    printf("\nRecipe status after search:\n");
    printf("  Pancakes tagged: %s\n", recipe1.is_tagged ? "Yes" : "No");
    printf("  Chocolate Cake tagged: %s\n", recipe2.is_tagged ? "Yes" : "No");
    printf("  Omelette tagged: %s\n", recipe3.is_tagged ? "Yes" : "No");

    return 0;
}