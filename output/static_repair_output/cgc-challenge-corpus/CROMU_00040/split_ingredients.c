#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

// Function: split_ingredient
int split_ingredient(char *param_1, char *param_2, char *param_4) {
  char temp_word_buffer[100]; // Reused for quantity and unit words
  char *src_ptr = param_1;
  size_t current_len;

  // Initialize output parameters
  param_2[0] = '\0';
  param_4[0] = '\0';

  // 1. Extract the first word (potential quantity) into temp_word_buffer
  memset(temp_word_buffer, 0, sizeof(temp_word_buffer));
  current_len = 0;
  while (src_ptr[current_len] != ' ' && src_ptr[current_len] != '\0' &&
         current_len < sizeof(temp_word_buffer) - 1) {
    temp_word_buffer[current_len] = src_ptr[current_len];
    current_len++;
  }
  temp_word_buffer[current_len] = '\0';

  // Check if the first word is numeric (contains digits, '.', or '/')
  bool is_quantity = true;
  if (current_len == 0 && param_1[0] != '\0') {
      // If the first word is empty but the input string is not, it's not a quantity.
      // (e.g., input starts with a space, or is " some_text")
      is_quantity = false;
  } else {
      for (size_t i = 0; i < current_len; ++i) {
        if (!isdigit((unsigned char)temp_word_buffer[i]) &&
            temp_word_buffer[i] != '.' &&
            temp_word_buffer[i] != '/') {
          is_quantity = false;
          break;
        }
      }
  }

  if (is_quantity) {
    // If it's a quantity, copy it to param_2
    strcpy(param_2, temp_word_buffer);

    // Advance src_ptr past the first word
    src_ptr += current_len;
    if (*src_ptr == ' ') { // Skip the space after the quantity, if present
      src_ptr++;
    }

    // 2. Extract the second word (potential unit) into temp_word_buffer
    memset(temp_word_buffer, 0, sizeof(temp_word_buffer)); // Clear buffer for reuse
    current_len = 0;
    while (src_ptr[current_len] != ' ' && src_ptr[current_len] != '\0' &&
           current_len < sizeof(temp_word_buffer) - 1) {
      temp_word_buffer[current_len] = src_ptr[current_len];
      current_len++;
    }
    temp_word_buffer[current_len] = '\0';

    // Check if the second word is a recognized unit
    if (strcmp(temp_word_buffer, "tsp") == 0 ||
        strcmp(temp_word_buffer, "tbsp") == 0 ||
        strcmp(temp_word_buffer, "cup") == 0 ||
        strcmp(temp_word_buffer, "cups") == 0 ||
        strcmp(temp_word_buffer, "oz") == 0) {
      // If it's a unit, append it to param_2
      strcat(param_2, " ");
      strcat(param_2, temp_word_buffer);

      // Advance src_ptr past the unit word
      src_ptr += current_len;
      if (*src_ptr == ' ') { // Skip the space after the unit, if present
        src_ptr++;
      }
    }
    // The rest of the string is the description
    strcpy(param_4, src_ptr);
  } else {
    // If the first word was not a quantity, the whole string is the description
    strcpy(param_4, param_1);
    param_2[0] = '\0'; // Ensure param_2 is empty
  }

  return 0;
}

// Main function for demonstration
int main() {
    char ingredient_input[256];
    char quantity_unit_output[128];
    char description_output[128];

    // Test cases
    strcpy(ingredient_input, "1 1/2 cups chopped onions");
    split_ingredient(ingredient_input, quantity_unit_output, description_output);
    printf("Input: \"%s\"\n", ingredient_input);
    printf("Quantity/Unit: \"%s\"\n", quantity_unit_output);
    printf("Description: \"%s\"\n\n", description_output);

    strcpy(ingredient_input, "2 tsp salt");
    split_ingredient(ingredient_input, quantity_unit_output, description_output);
    printf("Input: \"%s\"\n", ingredient_input);
    printf("Quantity/Unit: \"%s\"\n", quantity_unit_output);
    printf("Description: \"%s\"\n\n", description_output);

    strcpy(ingredient_input, "1 tbsp olive oil");
    split_ingredient(ingredient_input, quantity_unit_output, description_output);
    printf("Input: \"%s\"\n", ingredient_input);
    printf("Quantity/Unit: \"%s\"\n", quantity_unit_output);
    printf("Description: \"%s\"\n\n", description_output);

    strcpy(ingredient_input, "chicken breast, diced");
    split_ingredient(ingredient_input, quantity_unit_output, description_output);
    printf("Input: \"%s\"\n", ingredient_input);
    printf("Quantity/Unit: \"%s\"\n", quantity_unit_output);
    printf("Description: \"%s\"\n\n", description_output);

    strcpy(ingredient_input, "5 oz spinach");
    split_ingredient(ingredient_input, quantity_unit_output, description_output);
    printf("Input: \"%s\"\n", ingredient_input);
    printf("Quantity/Unit: \"%s\"\n", quantity_unit_output);
    printf("Description: \"%s\"\n\n", description_output);

    strcpy(ingredient_input, " "); // Edge case: space only
    split_ingredient(ingredient_input, quantity_unit_output, description_output);
    printf("Input: \"%s\"\n", ingredient_input);
    printf("Quantity/Unit: \"%s\"\n", quantity_unit_output);
    printf("Description: \"%s\"\n\n", description_output);

    strcpy(ingredient_input, ""); // Edge case: empty string
    split_ingredient(ingredient_input, quantity_unit_output, description_output);
    printf("Input: \"%s\"\n", ingredient_input);
    printf("Quantity/Unit: \"%s\"\n", quantity_unit_output);
    printf("Description: \"%s\"\n\n", description_output);

    strcpy(ingredient_input, "1"); // Only quantity
    split_ingredient(ingredient_input, quantity_unit_output, description_output);
    printf("Input: \"%s\"\n", ingredient_input);
    printf("Quantity/Unit: \"%s\"\n", quantity_unit_output);
    printf("Description: \"%s\"\n\n", description_output);

    strcpy(ingredient_input, "1 cup"); // Quantity and unit, no description
    split_ingredient(ingredient_input, quantity_unit_output, description_output);
    printf("Input: \"%s\"\n", ingredient_input);
    printf("Quantity/Unit: \"%s\"\n", quantity_unit_output);
    printf("Description: \"%s\"\n\n", description_output);

    strcpy(ingredient_input, "1/2 cup water"); // Fraction
    split_ingredient(ingredient_input, quantity_unit_output, description_output);
    printf("Input: \"%s\"\n", ingredient_input);
    printf("Quantity/Unit: \"%s\"\n", quantity_unit_output);
    printf("Description: \"%s\"\n\n", description_output);

    strcpy(ingredient_input, ".5 cup sugar"); // Decimal
    split_ingredient(ingredient_input, quantity_unit_output, description_output);
    printf("Input: \"%s\"\n", ingredient_input);
    printf("Quantity/Unit: \"%s\"\n", quantity_unit_output);
    printf("Description: \"%s\"\n\n", description_output);

    return 0;
}