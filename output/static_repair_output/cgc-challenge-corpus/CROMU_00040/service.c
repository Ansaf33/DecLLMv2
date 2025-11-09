#include <stdio.h>
#include <stdlib.h>
#include <string.h> // For strlen if needed, though direct indexing is used for newline

// Placeholder declarations for other functions and global variable
// In a real application, these would be in a header file or defined elsewhere.
void print_main_menu(void);
void new_recipe(void);
void find_recipe(void);
void print_all_tagged(void);
void print_shopping_list(void);
void _terminate(void);

int main(void) {
    char *input_buffer = NULL;
    size_t input_buffer_size = 0;
    ssize_t chars_read;
    int choice;

    while (1) {
        print_main_menu();

        chars_read = getline(&input_buffer, &input_buffer_size, stdin);

        if (chars_read == -1) {
            if (feof(stdin)) {
                fprintf(stderr, "Exiting program due to EOF.\n");
                break; // Exit loop on Ctrl+D (EOF)
            } else {
                perror("Error reading input");
                free(input_buffer);
                return 1; // Exit with error
            }
        }

        // Remove trailing newline character if present
        if (chars_read > 0 && input_buffer[chars_read - 1] == '\n') {
            input_buffer[chars_read - 1] = '\0';
            chars_read--; // Adjust count of actual characters
        }

        // Handle empty input after trimming newline
        if (chars_read == 0) {
            printf("Empty input, please try again.\n");
            continue; // Re-prompt for input
        }

        choice = atoi(input_buffer);

        switch (choice) {
            case 1:
                new_recipe();
                break;
            case 2:
                find_recipe();
                break;
            case 3:
                print_all_tagged();
                break;
            case 4:
                print_shopping_list();
                break;
            case 5:
                _terminate(); // Assuming this function handles program exit
                // If _terminate() does not call exit(), the loop would continue.
                // Adding a break here ensures loop termination if _terminate() returns.
                break;
            default:
                printf("Invalid command\n");
                break;
        }
    }

    free(input_buffer); // Free the buffer allocated by getline
    return 0; // Standard successful exit
}

// Dummy implementations for compilation
void print_main_menu(void) {
    printf("\n--- Recipe Book Menu ---\n");
    printf("1. New Recipe\n");
    printf("2. Find Recipe\n");
    printf("3. Print All Tagged Recipes\n");
    printf("4. Print Shopping List\n");
    printf("5. Exit\n");
    printf("Enter your choice: ");
    fflush(stdout); // Ensure prompt is displayed before input
}

void new_recipe(void) {
    printf("Function: New Recipe (not implemented)\n");
}

void find_recipe(void) {
    printf("Function: Find Recipe (not implemented)\n");
}

void print_all_tagged(void) {
    printf("Function: Print All Tagged (not implemented)\n");
}

void print_shopping_list(void) {
    printf("Function: Print Shopping List (not implemented)\n");
}

void _terminate(void) {
    printf("Exiting program via _terminate().\n");
    exit(0); // Exit the program
}