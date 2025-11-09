#include <stdio.h>  // For printf
#include <stdlib.h> // For malloc, free, size_t, NULL
#include <string.h> // For strlen, memcpy

// Function: strdup
char * strdup(const char *s) {
    if (s == NULL) {
        return NULL;
    }

    size_t len = strlen(s);
    char *new_s = (char *)malloc(len + 1);

    if (new_s == NULL) {
        return NULL;
    }

    memcpy(new_s, s, len + 1);

    return new_s;
}

int main() {
    char *original_string = "Hello, world!";
    char *duplicate_string = strdup(original_string);

    if (duplicate_string != NULL) {
        printf("Original: %s\n", original_string);
        printf("Duplicate: %s\n", duplicate_string);
        free(duplicate_string); // Free the allocated memory
    } else {
        printf("Failed to duplicate string or input was NULL.\n");
    }

    // Test with NULL input
    char *null_duplicate = strdup(NULL);
    if (null_duplicate == NULL) {
        printf("strdup(NULL) correctly returned NULL.\n");
    }

    // Test with empty string
    char *empty_string = "";
    char *empty_duplicate = strdup(empty_string);
    if (empty_duplicate != NULL) {
        printf("Empty string duplicate: '%s'\n", empty_duplicate);
        free(empty_duplicate);
    } else {
        printf("Failed to duplicate empty string.\n");
    }

    return 0;
}