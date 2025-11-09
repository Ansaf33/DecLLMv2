#include <stdio.h>    // For printf, getline, stdin, perror
#include <stdlib.h>   // For malloc, free, realloc, exit, EXIT_FAILURE, strdup
#include <string.h>   // For strlen, strcpy, memset

// Define the structure that the original param_1 likely pointed to.
// The original code uses (param_1 + 0xd0) to access a pointer to an array of char*.
// We define it such that 'instructions' is at offset 0xd0.
typedef struct {
    char _padding[0xd0]; // Placeholder to match the offset 0xd0
    char **instructions; // This will point to the array of instruction strings
} MyStruct;

// Function: get_instructions
void get_instructions(MyStruct *data_ptr) {
    char *line_buffer = NULL; // Buffer for getline, will be allocated by getline
    size_t line_buffer_len = 0; // Size of the buffer
    ssize_t bytes_read;         // Return value of getline

    char **instructions_array = NULL; // Dynamically allocated array of char*
    int instruction_count = 0;        // Number of instructions entered

    printf("Enter the step by step instructions. A blank line to end.\n\n");

    while (1) {
        printf("Step %d: ", instruction_count + 1);
        bytes_read = getline(&line_buffer, &line_buffer_len, stdin);

        // Check for end of input (blank line or error)
        // getline returns -1 on error or EOF. A blank line (just newline) results in bytes_read = 1.
        if (bytes_read <= 1) {
            break; // Exit loop on blank line or error/EOF
        }

        // Remove trailing newline character if present
        if (line_buffer[bytes_read - 1] == '\n') {
            line_buffer[bytes_read - 1] = '\0';
        }

        // Reallocate the array to hold one more instruction pointer
        char **temp_array = (char**)realloc(instructions_array, (instruction_count + 1) * sizeof(char*));
        if (temp_array == NULL) {
            perror("Unable to realloc memory for instructions array");
            // Free any previously allocated instruction strings
            for (int i = 0; i < instruction_count; i++) {
                free(instructions_array[i]);
            }
            free(instructions_array); // Free the old array if it was not NULL
            free(line_buffer); // Free getline buffer
            exit(EXIT_FAILURE);
        }
        instructions_array = temp_array;

        // Allocate memory for the instruction string and copy it using strdup
        char *instruction_str = strdup(line_buffer); // strdup allocates memory and copies the string
        if (instruction_str == NULL) {
            perror("Unable to malloc memory for instruction string");
            // Clean up resources before exiting
            for (int i = 0; i < instruction_count; i++) {
                free(instructions_array[i]);
            }
            free(instructions_array); // Free the partially filled array
            free(line_buffer);
            exit(EXIT_FAILURE);
        }

        // Store the new instruction string in the array
        instructions_array[instruction_count] = instruction_str;
        instruction_count++; // Increment count
    }

    // After the loop, add a NULL terminator to the array of instructions
    // This is a common practice for arrays of strings, similar to argv.
    if (instructions_array != NULL) {
        char **temp_array = (char**)realloc(instructions_array, (instruction_count + 1) * sizeof(char*));
        if (temp_array == NULL) {
            perror("Unable to realloc memory for NULL terminator");
            // Even if realloc fails here, instructions_array is still valid up to instruction_count.
            // For robustness, consider handling this more gracefully. For this problem, we exit.
            for (int i = 0; i < instruction_count; i++) {
                free(instructions_array[i]);
            }
            free(instructions_array);
            free(line_buffer);
            exit(EXIT_FAILURE);
        }
        instructions_array = temp_array;
        instructions_array[instruction_count] = NULL; // NULL terminate the array
    } else {
        // If no instructions were entered, ensure data_ptr->instructions is NULL
        data_ptr->instructions = NULL;
    }

    // Assign the final array to the structure member
    data_ptr->instructions = instructions_array;

    // Free the last getline buffer if it was allocated
    if (line_buffer != NULL) {
        free(line_buffer);
    }

    return;
}

// Main function to demonstrate usage and allow compilation
int main() {
    MyStruct my_data;
    // Initialize the instructions pointer within my_data to NULL
    // This is good practice before passing to a function that might reallocate it.
    my_data.instructions = NULL;

    get_instructions(&my_data);

    printf("\n--- Instructions entered ---\n");
    if (my_data.instructions != NULL) {
        for (int i = 0; my_data.instructions[i] != NULL; i++) {
            printf("Instruction %d: %s\n", i + 1, my_data.instructions[i]);
            // Free each individual instruction string
            free(my_data.instructions[i]);
        }
        // Free the array of pointers itself
        free(my_data.instructions);
    } else {
        printf("No instructions were entered.\n");
    }

    return 0;
}