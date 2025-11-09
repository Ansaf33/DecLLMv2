#include <stddef.h> // For NULL
#include <stdio.h>  // For printf in main

// Function: compare_strings
int compare_strings(char *param_1, char *param_2) {
    // Handle NULL pointers
    if (param_1 == NULL) {
        return (param_2 == NULL) ? 0 : -1; // If param_1 is NULL: return 0 if param_2 is also NULL, else -1
    }
    if (param_2 == NULL) { // param_1 is guaranteed non-NULL here
        return 1; // If param_2 is NULL, param_1 is non-NULL, so param_1 is "greater"
    }

    // Both param_1 and param_2 are guaranteed to be non-NULL
    for (; *param_1 != '\0' && *param_2 != '\0'; param_1++, param_2++) {
        if (*param_1 < *param_2) {
            return -1;
        }
        if (*param_1 > *param_2) {
            return 1;
        }
    }

    // After loop, one or both strings reached null terminator
    if (*param_1 == '\0' && *param_2 == '\0') {
        return 0; // Both strings ended at the same time, meaning they are equal
    }
    if (*param_1 == '\0') { // param_1 ended, but param_2 did not
        return -1; // param_1 is shorter, thus "less"
    }
    // If param_2 == '\0' but param_1 != '\0'
    return 1; // param_1 is longer, thus "greater"
}

// Main function for testing
int main() {
    printf("compare_strings(\"hello\", \"hello\"): %d (expected 0)\n", compare_strings("hello", "hello"));
    printf("compare_strings(\"hello\", \"world\"): %d (expected -1)\n", compare_strings("hello", "world"));
    printf("compare_strings(\"world\", \"hello\"): %d (expected 1)\n", compare_strings("world", "hello"));
    printf("compare_strings(\"apple\", \"apricot\"): %d (expected -1)\n", compare_strings("apple", "apricot"));
    printf("compare_strings(\"apricot\", \"apple\"): %d (expected 1)\n", compare_strings("apricot", "apple"));
    printf("compare_strings(\"test\", \"testing\"): %d (expected -1)\n", compare_strings("test", "testing"));
    printf("compare_strings(\"testing\", \"test\"): %d (expected 1)\n", compare_strings("testing", "test"));
    printf("compare_strings(\"\", \"\"): %d (expected 0)\n", compare_strings("", ""));
    printf("compare_strings(\"a\", \"\"): %d (expected 1)\n", compare_strings("a", ""));
    printf("compare_strings(\"\", \"a\"): %d (expected -1)\n", compare_strings("", "a"));

    // NULL pointer tests
    printf("compare_strings(NULL, NULL): %d (expected 0)\n", compare_strings(NULL, NULL));
    printf("compare_strings(NULL, \"abc\"): %d (expected -1)\n", compare_strings(NULL, "abc"));
    printf("compare_strings(\"abc\", NULL): %d (expected 1)\n", compare_strings("abc", NULL));

    return 0;
}