#include <stdio.h>    // For fprintf, stderr, printf
#include <stdlib.h>   // For calloc, realloc, free, exit
#include <string.h>   // For strlen, strcpy, strncpy, strcat, memset, strncmp
#include <stdbool.h>  // For bool
#include <stdint.h>   // For size_t (though included via stdlib.h/stddef.h usually)

// Custom error function to replace _error
void _error(int status, const char *file, int line) {
    fprintf(stderr, "ERROR: %s:%d - Exiting with status %d\n", file, line, status);
    exit(status);
}

// Define the String structure to represent the string object
// Based on the original code's usage (capacity then buffer pointer)
typedef struct {
    size_t capacity;
    char *buffer;
} String;

// Function: new_string
// Creates a new String object with an initial buffer.
// The original code allocated 8 bytes for the String object and 128 for the buffer.
// This assumes a 32-bit system where pointers are 4 bytes.
// For Linux compilable C code, especially on 64-bit, we use a proper struct
// and allocate memory accordingly.
String *new_string(const char *initial_str) {
    String *s = (String *)calloc(1, sizeof(String));
    if (s == NULL) {
        _error(1, __FILE__, __LINE__);
    }

    s->capacity = 0x80; // Initial default capacity as per original code
    size_t initial_len = (initial_str != NULL) ? strlen(initial_str) : 0;

    // Ensure initial capacity is sufficient for the initial string
    if (s->capacity < initial_len + 1) {
        s->capacity = (initial_len + 1) * 2; // Double the required capacity
    }

    s->buffer = (char *)calloc(1, s->capacity);
    if (s->buffer == NULL) {
        free(s); // Clean up String object if buffer allocation fails
        _error(1, __FILE__, __LINE__);
    }

    if (initial_str != NULL) {
        strcpy(s->buffer, initial_str);
    } else {
        s->buffer[0] = '\0'; // Ensure it's an empty string
    }
    return s;
}

// Function: set_string
// Sets the content of an existing String object.
// Returns 0 on success, -1 on invalid input.
int set_string(String *s, const char *new_value) {
    if (new_value == NULL || s == NULL || s->buffer == NULL) {
        return -1;
    }

    memset(s->buffer, 0, s->capacity); // Clear existing content

    size_t new_len = strlen(new_value);
    if (s->capacity < new_len + 1) { // +1 for null terminator
        // Reallocate: The original code used (current_len + new_len + 1) * 2.
        // For a set operation, current_len is irrelevant; we just need to fit new_value.
        s->capacity = (new_len + 1) * 2;
        char *new_buffer = (char *)realloc(s->buffer, s->capacity);
        if (new_buffer == NULL) {
            _error(1, __FILE__, __LINE__);
        }
        s->buffer = new_buffer;
    }
    strncpy(s->buffer, new_value, s->capacity - 1); // Copy up to capacity-1 chars
    s->buffer[s->capacity - 1] = '\0'; // Ensure null termination
    return 0;
}

// Function: append_string
// Appends a string to an existing String object.
// Returns 0 on success, -1 on invalid input.
int append_string(String *s, const char *to_append) {
    if (to_append == NULL || s == NULL || s->buffer == NULL) {
        return -1;
    }

    size_t current_len = strlen(s->buffer);
    size_t append_len = strlen(to_append);
    size_t required_capacity = current_len + append_len + 1; // +1 for null terminator

    if (s->capacity < required_capacity) {
        s->capacity = required_capacity * 2; // Double the required capacity
        char *new_buffer = (char *)realloc(s->buffer, s->capacity);
        if (new_buffer == NULL) {
            _error(1, __FILE__, __LINE__);
        }
        s->buffer = new_buffer;
    }
    strcat(s->buffer, to_append); // Safe now because capacity is checked
    return 0;
}

// Function: contains_string
// Checks if a String object contains a given substring.
// Returns true if found, false otherwise.
bool contains_string(const String *s, const char *substring) {
    if (s == NULL || s->buffer == NULL || substring == NULL) {
        return false;
    }

    const char *haystack = s->buffer;
    size_t haystack_len = strlen(haystack);
    size_t needle_len = strlen(substring);

    if (needle_len == 0) {
        return true; // An empty string is always considered contained
    }
    if (haystack_len < needle_len) {
        return false; // Haystack is shorter than the substring
    }

    // The original code implemented a manual search similar to strstr.
    // We can simplify this by using strncmp within a loop, which is efficient
    // and reduces the need for many intermediate variables or repeated strlen calls.
    for (size_t i = 0; i <= haystack_len - needle_len; ++i) {
        if (strncmp(haystack + i, substring, needle_len) == 0) {
            return true;
        }
    }

    return false;
}

// Function: free_string
// Frees the memory associated with a String object.
void free_string(String *s) {
    if (s != NULL) {
        if (s->buffer != NULL) {
            free(s->buffer);
            s->buffer = NULL; // Prevent double-free
        }
        free(s);
        // It's good practice to set the pointer to NULL after freeing,
        // but the caller's pointer won't be affected by this assignment
        // unless passed by reference (e.g., `String **s_ptr`).
        // For simplicity, we just free the allocated memory.
    }
}

// Main function to demonstrate usage
int main() {
    printf("--- String Management Demonstration ---\n\n");

    // Test new_string
    String *my_string = new_string("Hello");
    printf("New string created: '%s' (Capacity: %zu)\n", my_string->buffer, my_string->capacity);

    // Test append_string
    append_string(my_string, ", World!");
    printf("After appending ', World!': '%s' (Capacity: %zu)\n", my_string->buffer, my_string->capacity);

    append_string(my_string, " This is a longer string to test realloc.");
    printf("After appending more: '%s' (Capacity: %zu)\n", my_string->buffer, my_string->capacity);

    // Test set_string
    set_string(my_string, "New content.");
    printf("After setting to 'New content.': '%s' (Capacity: %zu)\n", my_string->buffer, my_string->capacity);

    set_string(my_string, "This is an even longer string to force realloc again with set_string.");
    printf("After setting to a longer string: '%s' (Capacity: %zu)\n", my_string->buffer, my_string->capacity);

    // Test contains_string
    printf("\n--- Contains String Tests ---\n");
    printf("Does '%s' contain 'New'? %s\n", my_string->buffer, contains_string(my_string, "New") ? "Yes" : "No");
    printf("Does '%s' contain 'longer'? %s\n", my_string->buffer, contains_string(my_string, "longer") ? "Yes" : "No");
    printf("Does '%s' contain 'nonexistent'? %s\n", my_string->buffer, contains_string(my_string, "nonexistent") ? "Yes" : "No");
    printf("Does '%s' contain '' (empty string)? %s\n", my_string->buffer, contains_string(my_string, "") ? "Yes" : "No");
    
    String *empty_string = new_string("");
    printf("Does '%s' contain 'a'? %s\n", empty_string->buffer, contains_string(empty_string, "a") ? "Yes" : "No");
    printf("Does '%s' contain '%s'? %s\n", my_string->buffer, my_string->buffer, contains_string(my_string, my_string->buffer) ? "Yes" : "No");
    free_string(empty_string);

    // Test freeing string
    free_string(my_string);
    printf("\nStrings freed. Program finished.\n");

    return 0;
}