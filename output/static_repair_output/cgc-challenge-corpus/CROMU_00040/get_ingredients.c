#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

typedef struct Ingredient {
    char measurement[20];
    char ingredient_name[100];
    struct Ingredient *next;
} Ingredient;

typedef struct Recipe {
    char _padding[0xcc];
    Ingredient *ingredients_list;
} Recipe;

static void _terminate() {
    fprintf(stderr, "Critical error, terminating.\n");
    exit(EXIT_FAILURE);
}

static void split_ingredient(char *line, char *measurement_buf, size_t measurement_buf_size,
                             char *ingredient_buf, size_t ingredient_buf_size) {
    line[strcspn(line, "\n")] = 0;

    char *first_space = strchr(line, ' ');

    if (first_space != NULL) {
        size_t measurement_len = first_space - line;
        if (measurement_len >= measurement_buf_size) {
            measurement_len = measurement_buf_size - 1;
        }
        strncpy(measurement_buf, line, measurement_len);
        measurement_buf[measurement_len] = '\0';

        char *ingredient_start = first_space + 1;
        while (*ingredient_start == ' ') {
            ingredient_start++;
        }
        size_t ingredient_len = strlen(ingredient_start);
        if (ingredient_len >= ingredient_buf_size) {
            ingredient_len = ingredient_buf_size - 1;
        }
        strncpy(ingredient_buf, ingredient_start, ingredient_len);
        ingredient_buf[ingredient_len] = '\0';
    } else {
        measurement_buf[0] = '\0';
        size_t ingredient_len = strlen(line);
        if (ingredient_len >= ingredient_buf_size) {
            ingredient_len = ingredient_buf_size - 1;
        }
        strncpy(ingredient_buf, line, ingredient_len);
        ingredient_buf[ingredient_len] = '\0';
    }
}

int get_ingredients(intptr_t param_1) {
    Recipe *recipe_ptr = (Recipe *)param_1;

    char *line = NULL;
    size_t len = 0;
    ssize_t bytes_read;

    char measurement_buf[1024];
    char ingredient_buf[1024];

    Ingredient *current_ingredient = NULL;
    int ingredient_count = 0;

    printf("Enter the measurement and ingredients, one per line.  A blank line ends.\n\n");

    bytes_read = getline(&line, &len, stdin);

    if (bytes_read == -1) {
        fprintf(stderr, "Error reading input or EOF encountered.\n");
    } else if (bytes_read < 2) {
        // Empty line or just newline, no ingredients
    } else {
        current_ingredient = (Ingredient *)malloc(sizeof(Ingredient));
        if (current_ingredient == NULL) {
            printf("Unable to malloc memory for ingredient.\n");
            _terminate();
        }
        memset(current_ingredient, 0, sizeof(Ingredient));
        recipe_ptr->ingredients_list = current_ingredient;

        while (bytes_read >= 2) {
            memset(measurement_buf, 0, sizeof(measurement_buf));
            memset(ingredient_buf, 0, sizeof(ingredient_buf));

            split_ingredient(line, measurement_buf, sizeof(measurement_buf),
                             ingredient_buf, sizeof(ingredient_buf));

            strncpy(current_ingredient->measurement, measurement_buf, sizeof(current_ingredient->measurement) - 1);
            current_ingredient->measurement[sizeof(current_ingredient->measurement) - 1] = '\0';

            strncpy(current_ingredient->ingredient_name, ingredient_buf, sizeof(current_ingredient->ingredient_name) - 1);
            current_ingredient->ingredient_name[sizeof(current_ingredient->ingredient_name) - 1] = '\0';

            current_ingredient->next = NULL;

            ingredient_count++;

            bytes_read = getline(&line, &len, stdin);

            if (bytes_read == -1) {
                fprintf(stderr, "Error reading input or EOF encountered after %d ingredients.\n", ingredient_count);
                break;
            } else if (bytes_read >= 2) {
                Ingredient *next_ingredient = (Ingredient *)malloc(sizeof(Ingredient));
                if (next_ingredient == NULL) {
                    printf("Unable to malloc memory for next ingredient.\n");
                    _terminate();
                }
                memset(next_ingredient, 0, sizeof(Ingredient));
                current_ingredient->next = next_ingredient;
                current_ingredient = next_ingredient;
            }
        }
    }

    if (line != NULL) {
        free(line);
    }

    return ingredient_count;
}

int main() {
    Recipe my_recipe;
    memset(&my_recipe, 0, sizeof(Recipe));

    printf("--- Starting ingredient input ---\n");
    int num_ingredients = get_ingredients((intptr_t)&my_recipe);

    printf("\n--- Finished ingredient input ---\n");
    printf("Total ingredients entered: %d\n", num_ingredients);

    Ingredient *current = my_recipe.ingredients_list;
    int i = 1;
    while (current != NULL) {
        printf("Ingredient %d: Measurement='%s', Name='%s'\n",
               i++, current->measurement, current->ingredient_name);
        Ingredient *temp = current;
        current = current->next;
        free(temp);
    }
    my_recipe.ingredients_list = NULL;

    return 0;
}