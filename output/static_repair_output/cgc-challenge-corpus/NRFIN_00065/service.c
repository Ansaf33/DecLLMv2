#include <stdio.h>   // For printf, stdin, fgets
#include <string.h>  // For strlen, strcspn
#include <stdlib.h>  // For strtol
#include <limits.h>  // For INT_MIN, INT_MAX

// Forward declarations for custom functions
// These functions are assumed to exist elsewhere and return an int, taking no arguments.
void init_cookie(void);
int do_hugman(void);
int do_chess(void);
int do_sudoku(void);
int do_gallon_challenge(void);
int do_safari_zone(void);
int do_chance_of_survival(void);
int do_lesson_in_futility(void);

// Custom helper function mimicking the behavior of 'fread_until' from the snippet.
// Reads a line from 'stream' into 'buffer', up to 'buffer_size - 1' characters.
// Returns 0 on success, -1 on EOF or read error.
// Removes the trailing newline character if present.
int fread_until_custom(char *buffer, size_t buffer_size, FILE *stream) {
    if (fgets(buffer, buffer_size, stream) == NULL) {
        return -1; // EOF or error
    }
    buffer[strcspn(buffer, "\n")] = 0; // Remove trailing newline
    return 0; // Success
}

// Custom helper function mimicking the behavior of 'strtou' from the snippet.
// Converts a string 'str' to an integer, stores it in '*result'.
// Returns 0 on success, -1 on failure (e.g., no conversion, unconverted characters, out of range).
int strtou_custom(const char *str, int base, int *result) {
    char *endptr;
    long val = strtol(str, &endptr, base);

    // Check for conversion errors
    if (str == endptr || *endptr != '\0') {
        return -1; // No characters were converted or unconverted characters remain
    }
    if (val < INT_MIN || val > INT_MAX) {
        return -1; // Value out of int range
    }

    *result = (int)val;
    return 0; // Success
}

int main(void) {
    int choice;
    char input_buffer[40]; // Buffer for user input (0x28 = 40 bytes)
    int game_result = 0;    // Stores the return value of game functions

    init_cookie(); // Initialize security cookie or similar mechanism

    while (1) { // Main program loop for menu and game selection
        // Loop to repeatedly prompt for and validate user input
        while (1) {
            printf(
                "Shall we play a game?\n\n"
                "1. Hugman\n"
                "2. Chess\n"
                "3. Sudoku\n"
                "4. Gallon Challenge\n"
                "5. Safari Zone\n"
                "6. Chance of Survival\n"
                "7. Lesson in Futility\n"
                "8. Quit\n\n"
            );

            // Read user input using the custom fread_until-like function
            if (fread_until_custom(input_buffer, sizeof(input_buffer), stdin) == -1) {
                return 0; // Exit if EOF or read error occurs
            }

            // Validate input: check for empty string or failed conversion
            if (strlen(input_buffer) == 0 || strtou_custom(input_buffer, 10, &choice) == -1) {
                printf("Invalid input. Please enter a number.\n\n");
                continue; // Prompt for input again
            }
            break; // Valid input received, exit inner loop
        }

        // Process the user's choice
        switch (choice) {
            case 1:
                game_result = do_hugman();
                break;
            case 2:
                game_result = do_chess();
                break;
            case 3:
                game_result = do_sudoku();
                break;
            case 4:
                game_result = do_gallon_challenge();
                break;
            case 5:
                game_result = do_safari_zone();
                break;
            case 6:
                game_result = do_chance_of_survival();
                break;
            case 7:
                printf("Wouldn\'t you prefer a nice game of chess?\n");
                game_result = do_lesson_in_futility();
                break;
            case 8:
                return 0; // Quit the program
            default:
                printf("Invalid choice. Please enter a number between 1 and 8.\n\n");
                game_result = 0; // Reset result for invalid choice, no error to propagate
                continue; // Go back to the beginning of the main loop (display menu)
        }

        // Common error check for game functions
        if (game_result != 0) {
            return 0; // Quit if a game function returned an error
        }
    }
    // This part of the code should ideally not be reached as the loop is infinite or exits via return statements.
    return 0;
}

// Dummy implementations for custom functions to ensure compilability.
// In a real application, these would contain the actual game logic.
void init_cookie(void) {
    // Placeholder for initialization logic
}
int do_hugman(void) {
    printf("Playing Hugman...\n");
    return 0;
}
int do_chess(void) {
    printf("Playing Chess...\n");
    return 0;
}
int do_sudoku(void) {
    printf("Playing Sudoku...\n");
    return 0;
}
int do_gallon_challenge(void) {
    printf("Playing Gallon Challenge...\n");
    return 0;
}
int do_safari_zone(void) {
    printf("Entering Safari Zone...\n");
    return 0;
}
int do_chance_of_survival(void) {
    printf("Calculating Chance of Survival...\n");
    return 0;
}
int do_lesson_in_futility(void) {
    printf("Learning a Lesson in Futility...\n");
    return 0;
}