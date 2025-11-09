#include <stdlib.h> // For malloc, realloc, free, strdup
#include <stdbool.h> // For bool
#include <string.h> // For memset
#include <stdint.h> // For intptr_t (to safely cast between void* and integer types)
#include <stdio.h>  // For printf, fprintf (in main for demonstration)

// The 'strio_data' argument is expected to be an array of 3 void pointers:
// strio_data[0] = actual buffer (char*)
// strio_data[1] = current length (size_t cast to void*)
// strio_data[2] = current capacity (size_t cast to void*)

// Function: enlarge
// Expands the buffer capacity if needed. Returns 1 on success, 0 on failure.
int enlarge(void **strio_data, int required_size_int) {
    if (required_size_int < 0) {
        return 0; // Invalid size
    }
    size_t required_size = (size_t)required_size_int;

    size_t current_capacity = (size_t)(intptr_t)strio_data[2];

    if (required_size <= current_capacity) {
        return 1; // Already sufficient
    }

    // Max capacity check from original code (0x10000000 is 256MB)
    if (required_size > 0x10000000) {
        return 0; // Too large
    }

    size_t new_capacity = current_capacity;
    if (new_capacity == 0) { // Handle initial zero capacity
        new_capacity = 1;
    }

    // Double the capacity until it's at least required_size
    while (new_capacity < required_size) {
        // Prevent overflow when doubling and cap at 0x10000000
        if (new_capacity > 0x10000000 / 2) {
            new_capacity = 0x10000000;
            break; // No need to double further
        }
        new_capacity <<= 1;
    }

    // Ensure new_capacity is at least required_size, especially if doubling reached cap
    if (new_capacity < required_size) {
        new_capacity = required_size;
    }

    void *new_buffer = realloc(strio_data[0], new_capacity);
    if (new_buffer == NULL) {
        return 0; // Failure
    }

    strio_data[0] = new_buffer;
    strio_data[2] = (void *)(intptr_t)new_capacity; // Store new capacity
    return 1; // Success
}

// Function: strio_init
// Initializes the strio_data structure. Returns true on success, false on failure.
bool strio_init(void **strio_data) {
    // Initial capacity of 0x20 (32 bytes)
    strio_data[2] = (void *)(intptr_t)0x20;
    // Allocate initial buffer
    strio_data[0] = malloc((size_t)(intptr_t)strio_data[2]);
    // Initial length is 0
    strio_data[1] = (void *)(intptr_t)0;
    // Return true if malloc was successful
    return strio_data[0] != NULL;
}

// Function: strio_free
// Frees the allocated buffer and clears the strio_data pointers.
void strio_free(void **strio_data) {
    free(strio_data[0]);
    // Clear the array of pointers. 3 * sizeof(void*) is portable.
    memset(strio_data, 0, 3 * sizeof(void*));
}

// Function: strio_append_char
// Appends a character to the string. Returns 1 on success, 0 on failure.
int strio_append_char(void **strio_data, char c) {
    size_t current_len = (size_t)(intptr_t)strio_data[1];
    size_t current_capacity = (size_t)(intptr_t)strio_data[2];

    // If current length equals capacity, we need to enlarge
    if (current_len == current_capacity) {
        if (!enlarge(strio_data, current_capacity + 1)) {
            return 0; // Enlarge failed
        }
    }

    // Append the character
    ((char*)strio_data[0])[current_len] = c;
    // Update the length
    strio_data[1] = (void *)(intptr_t)(current_len + 1);
    return 1; // Success
}

// Function: strio_dup
// Duplicates the string content into a new null-terminated C-string.
// Returns a pointer to the new string on success, NULL on failure.
char * strio_dup(void **strio_data) {
    size_t current_len = (size_t)(intptr_t)strio_data[1];
    size_t current_capacity = (size_t)(intptr_t)strio_data[2];

    // Ensure there's space for the null terminator
    if (current_len == current_capacity) {
        // Enlarge by at least 1 byte for the null terminator
        if (!enlarge(strio_data, current_capacity + 1)) {
            return NULL; // Enlarge failed
        }
    }

    // Temporarily add null terminator for strdup.
    // This position is `current_len`, which is the first byte *after* the current string data.
    ((char*)strio_data[0])[current_len] = '\0';

    // Duplicate the string
    char *duplicated_string = strdup((char*)strio_data[0]);

    // The strio_data object's length is not permanently modified by strio_dup.
    // The null terminator will be overwritten by subsequent strio_append_char calls.
    return duplicated_string;
}

// Main function to demonstrate usage
int main() {
    // strio_data is an array of 3 void pointers
    // [0] -> char* buffer
    // [1] -> size_t length (cast to void*)
    // [2] -> size_t capacity (cast to void*)
    void *strio_data[3];
    char *dup_str = NULL;
    char *dup_str2 = NULL;

    printf("Initializing strio...\n");
    if (!strio_init(strio_data)) {
        fprintf(stderr, "strio_init failed!\n");
        return 1;
    }
    printf("strio_init successful. Buffer: %p, Length: %zu, Capacity: %zu\n",
           strio_data[0], (size_t)(intptr_t)strio_data[1], (size_t)(intptr_t)strio_data[2]);

    printf("\nAppending characters...\n");
    const char *test_string = "Hello, world!";
    for (int i = 0; test_string[i] != '\0'; ++i) {
        if (!strio_append_char(strio_data, test_string[i])) {
            fprintf(stderr, "strio_append_char failed for char '%c'!\n", test_string[i]);
            strio_free(strio_data);
            return 1;
        }
    }
    printf("Finished appending initial string. Current length: %zu, Capacity: %zu\n",
           (size_t)(intptr_t)strio_data[1], (size_t)(intptr_t)strio_data[2]);


    printf("\nDuplicating string (1st time)...\n");
    dup_str = strio_dup(strio_data);
    if (dup_str == NULL) {
        fprintf(stderr, "strio_dup failed!\n");
        strio_free(strio_data);
        return 1;
    }
    printf("Duplicated string: \"%s\"\n", dup_str);
    printf("Original strio length (should not include temporary null terminator): %zu\n", (size_t)(intptr_t)strio_data[1]);


    printf("\nAppending more characters...\n");
    if (!strio_append_char(strio_data, '!')) {
        fprintf(stderr, "strio_append_char failed for '!'!\n");
        free(dup_str);
        strio_free(strio_data);
        return 1;
    }
    if (!strio_append_char(strio_data, ' ')) {
        fprintf(stderr, "strio_append_char failed for ' '!\n");
        free(dup_str);
        strio_free(strio_data);
        return 1;
    }
    if (!strio_append_char(strio_data, 'C')) {
        fprintf(stderr, "strio_append_char failed for 'C'!\n");
        free(dup_str);
        strio_free(strio_data);
        return 1;
    }
    printf("Finished appending more characters. Current length: %zu, Capacity: %zu\n",
           (size_t)(intptr_t)strio_data[1], (size_t)(intptr_t)strio_data[2]);


    printf("\nDuplicating string (2nd time)...\n");
    dup_str2 = strio_dup(strio_data);
    if (dup_str2 == NULL) {
        fprintf(stderr, "strio_dup failed for second time!\n");
        free(dup_str);
        strio_free(strio_data);
        return 1;
    }
    printf("Second duplicated string: \"%s\"\n", dup_str2);


    printf("\nFreeing duplicated strings and strio...\n");
    free(dup_str);
    free(dup_str2);
    strio_free(strio_data);
    printf("All freed successfully.\n");

    return 0;
}