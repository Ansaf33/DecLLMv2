#include <string.h> // For strcpy, strcat, strcmp, memset
#include <ctype.h>  // For isdigit
#include <stddef.h> // For size_t

// Function: split_ingredient
int split_ingredient(char *param_1, char *param_2, char *param_4) {
    char current_word[100]; // Buffer for extracted words
    char *p_input = param_1; // Pointer to traverse the input string
    size_t word_len; // Length of the current word being extracted

    // Initialize param_2 to an empty string
    *param_2 = '\0';

    // 1. Extract the first word (potential quantity or main ingredient)
    memset(current_word, 0, sizeof(current_word)); // Clear buffer
    word_len = 0;
    while (*p_input != '\0' && *p_input != ' ' && word_len < sizeof(current_word) - 1) {
        current_word[word_len++] = *p_input++;
    }
    current_word[word_len] = '\0'; // Null-terminate the extracted word

    // Skip any spaces immediately following the first word
    while (*p_input == ' ') {
        p_input++;
    }

    // 2. Determine if the first extracted word is a quantity (contains only digits, '.', '/')
    int is_quantity = 1;
    if (word_len == 0) { // An empty first word cannot be a quantity
        is_quantity = 0;
    } else {
        for (size_t i = 0; i < word_len; ++i) {
            if (!isdigit((unsigned char)current_word[i]) && current_word[i] != '.' && current_word[i] != '/') {
                is_quantity = 0; // Found a non-numeric/non-dot/non-slash character
                break;
            }
        }
    }

    if (is_quantity) {
        // If the first word is a quantity, copy it to param_2
        strcpy(param_2, current_word);

        // 3. Extract the second word (potential unit)
        memset(current_word, 0, sizeof(current_word)); // Clear buffer for the next word
        word_len = 0;
        char *start_of_second_word = p_input; // Mark the beginning of the second word
        while (*p_input != '\0' && *p_input != ' ' && word_len < sizeof(current_word) - 1) {
            current_word[word_len++] = *p_input++;
        }
        current_word[word_len] = '\0'; // Null-terminate the second word

        // Skip any spaces immediately following the second word
        while (*p_input == ' ') {
            p_input++;
        }

        // 4. Check if the second word is a recognized unit
        if (strcmp(current_word, "tsp") == 0 ||
            strcmp(current_word, "tbsp") == 0 ||
            strcmp(current_word, "cup") == 0 ||
            strcmp(current_word, "cups") == 0 ||
            strcmp(current_word, "oz") == 0) {
            // If it's a recognized unit, append it to param_2
            strcat(param_2, " ");
            strcat(param_2, current_word);
            // The remainder for param_4 starts AFTER this unit and its trailing spaces
            strcpy(param_4, p_input);
        } else {
            // If the second word is NOT a recognized unit,
            // the remainder for param_4 starts from the beginning of the second word
            // (i.e., the second word itself and everything after it).
            strcpy(param_4, start_of_second_word);
        }
        return 0; // Successfully processed a quantity and possibly a unit
    } else {
        // If the first word is NOT a quantity (e.g., "chicken", "salt")
        // The entire original input string goes to param_4.
        // param_2 remains empty.
        strcpy(param_4, param_1);
        *param_2 = '\0'; // Ensure param_2 is empty (it already is, but for clarity/safety)
        return 0; // Successfully processed a non-quantity ingredient
    }
}