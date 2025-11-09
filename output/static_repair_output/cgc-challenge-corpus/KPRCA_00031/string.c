#include <stdio.h>   // For fprintf, stderr, printf
#include <stdlib.h>  // For calloc, realloc, free, exit
#include <string.h>  // For strlen, strcpy, strncpy, strcat, strstr
#include <stdbool.h> // For bool type

// Define the String struct to replace the ambiguous pointer usage in the original snippet.
// This structure clearly defines the capacity and the data buffer for a dynamic string.
typedef struct {
    size_t capacity; // Stores the allocated size of the data buffer (including null terminator).
                     // Corresponds to `*param_1` or `*(int*)param_1` in the original.
    char *data;      // Pointer to the character array holding the string content.
                     // Corresponds to `param_1[1]` or `*(char**)(param_1 + 4)` in the original.
} String;

// Custom error handling function, replacing the undefined `_error` from the snippet.
// It mimics the observed usage in the original code by printing an error message and exiting.
void _error(int exit_code, const char *file, int line) {
    fprintf(stderr, "Error: %s:%d (Exit Code %d)\n", file, line, exit_code);
    exit(exit_code);
}

// Function: new_string
// Allocates and initializes a new String object.
// It takes an optional initial string to populate the new String.
String *new_string(const char *initial_str) {
    String *s = (String *)calloc(1, sizeof(String));
    if (s == NULL) {
        _error(1, __FILE__, __LINE__);
    }

    s->capacity = 128; // Set initial capacity (0x80 from original snippet)
    s->data = (char *)calloc(1, s->capacity);
    if (s->data == NULL) {
        _error(1, __FILE__, __LINE__);
    }

    if (initial_str != NULL) {
        // Use strncpy for safety to prevent buffer overflows,
        // and explicitly ensure null termination.
        strncpy(s->data, initial_str, s->capacity - 1);
        s->data[s->capacity - 1] = '\0'; // Ensure null termination
    } else {
        s->data[0] = '\0'; // Ensure an empty string is properly null-terminated
    }
    return s;
}

// Function: set_string
// Sets the content of an existing String object to a new string.
// It handles reallocation if the new string requires more capacity.
int set_string(String *s, const char *new_str) {
    if (s == NULL || new_str == NULL) {
        return -1; // Indicate error for invalid input
    }

    size_t new_len = strlen(new_str);
    size_t required_capacity = new_len + 1; // +1 for the null terminator

    if (s->capacity < required_capacity) {
        // The original capacity growth logic for `set_string` was effectively `(new_len + 1) * 2`
        // (as `old_len` would be 0 after `memset`). We maintain this growth factor.
        s->capacity = required_capacity * 2; 

        char *new_data = (char *)realloc(s->data, s->capacity);
        if (new_data == NULL) {
            _error(1, __FILE__, __LINE__);
        }
        s->data = new_data;
    }
    
    // Copy the new string content. strncpy is safer than strcpy.
    // Ensure null termination by writing up to `capacity-1` and then explicitly nulling the last char.
    strncpy(s->data, new_str, s->capacity - 1);
    s->data[s->capacity - 1] = '\0'; // Ensure null termination
    
    return 0; // Indicate success
}

// Function: append_string
// Appends a string to an existing String object.
// It reallocates the buffer if necessary to accommodate the appended string.
int append_string(String *s, const char *to_append) {
    if (s == NULL || to_append == NULL) {
        return -1; // Indicate error for invalid input
    }

    size_t current_len = strlen(s->data);
    size_t append_len = strlen(to_append);
    size_t required_capacity = current_len + append_len + 1; // +1 for null terminator

    if (s->capacity < required_capacity) {
        // Original capacity growth logic: `(current_len + append_len + 1) * 2`
        s->capacity = required_capacity * 2; 
        char *new_data = (char *)realloc(s->data, s->capacity);
        if (new_data == NULL) {
            _error(1, __FILE__, __LINE__);
        }
        s->data = new_data;
    }
    strcat(s->data, to_append); // Safe to use strcat now that capacity is guaranteed
    return 0; // Indicate success
}

// Function: contains_string
// Checks if a String object contains a given substring (needle).
// Returns true if found, false otherwise.
bool contains_string(const String *s, const char *needle) {
    if (s == NULL || s->data == NULL || needle == NULL) {
        return false; // Invalid input means it cannot contain the needle
    }
    // The standard library function `strstr` provides robust and efficient substring search.
    // It correctly handles edge cases like empty strings.
    return strstr(s->data, needle) != NULL;
}

// Function: free_string
// Frees all memory associated with a String object.
void free_string(String *s) {
    if (s != NULL) {
        if (s->data != NULL) {
            free(s->data);
            s->data = NULL; // Nullify the pointer to prevent double-free issues
        }
        free(s);
    }
}

// Main function to demonstrate the usage of the String functions.
int main() {
    printf("--- String Library Demonstration ---\n");

    // Test new_string
    String *my_string = new_string("Initial String");
    printf("1. New string: '%s' (Capacity: %zu, Length: %zu)\n", 
           my_string->data, my_string->capacity, strlen(my_string->data));

    // Test append_string
    append_string(my_string, ", Appended Part.");
    printf("2. Appended: '%s' (Capacity: %zu, Length: %zu)\n", 
           my_string->data, my_string->capacity, strlen(my_string->data));

    // Test set_string with a shorter string
    set_string(my_string, "Short Test");
    printf("3. Set to 'Short Test': '%s' (Capacity: %zu, Length: %zu)\n", 
           my_string->data, my_string->capacity, strlen(my_string->data));

    // Test set_string with a much longer string to trigger realloc
    set_string(my_string, "This is a much longer string that should definitely trigger a reallocation of the underlying buffer to accommodate its length.");
    printf("4. Set to long string: '%s' (Capacity: %zu, Length: %zu)\n", 
           my_string->data, my_string->capacity, strlen(my_string->data));

    // Test contains_string
    printf("5. Contains 'longer'? %s\n", contains_string(my_string, "longer") ? "true" : "false");
    printf("6. Contains 'nonexistent'? %s\n", contains_string(my_string, "nonexistent") ? "true" : "false");
    printf("7. Contains '' (empty string)? %s\n", contains_string(my_string, "") ? "true" : "false");
    printf("8. Contains 'This'? %s\n", contains_string(my_string, "This") ? "true" : "false");

    // Test edge cases for contains_string with an empty string object
    String *empty_str_obj = new_string("");
    printf("9. Empty string object ('%s') contains 'a'? %s\n", empty_str_obj->data, contains_string(empty_str_obj, "a") ? "true" : "false");
    printf("10. Empty string object ('%s') contains ''? %s\n", empty_str_obj->data, contains_string(empty_str_obj, "") ? "true" : "false");
    free_string(empty_str_obj);

    // Test append_string again to check capacity growth after a large string
    append_string(my_string, " And now we append even more text to this already long string, pushing the capacity boundaries further!");
    printf("11. Appended even more: '%s' (Capacity: %zu, Length: %zu)\n", 
           my_string->data, my_string->capacity, strlen(my_string->data));

    // Test free_string
    free_string(my_string);
    printf("12. Original string freed.\n");

    // Test new_string with NULL initial string
    String *null_init_string = new_string(NULL);
    printf("13. New string (NULL init): '%s' (Capacity: %zu, Length: %zu)\n", 
           null_init_string->data, null_init_string->capacity, strlen(null_init_string->data));
    free_string(null_init_string);
    printf("14. NULL-initialized string freed.\n");

    return 0;
}