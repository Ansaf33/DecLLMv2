#include <stddef.h> // For NULL
#include <stdio.h>  // For printf in main

// Function: compare_strings
int compare_strings(const char *param_1, const char *param_2) {
    // Handle NULL pointers first
    if (param_1 == NULL) {
        if (param_2 == NULL) {
            return 0; // Both are NULL, consider them equal
        }
        return -1; // param_1 is NULL, param_2 is not, param_1 is "less"
    }
    if (param_2 == NULL) { // param_1 is guaranteed not NULL here
        return 1; // param_2 is NULL, param_1 is not, param_1 is "greater"
    }

    // Both param_1 and param_2 are guaranteed not to be NULL
    while (*param_1 != '\0' && *param_2 != '\0') {
        if (*param_1 < *param_2) {
            return -1; // param_1 character is smaller, so param_1 is "less"
        }
        if (*param_1 > *param_2) {
            return 1; // param_1 character is greater, so param_1 is "greater"
        }
        param_1++;
        param_2++;
    }

    // One or both strings have reached the null terminator
    if (*param_1 == '\0' && *param_2 == '\0') {
        return 0; // Both strings are equal
    }
    if (*param_1 == '\0') {
        return -1; // param_1 is shorter, param_1 is "less"
    }
    // If we reach here, *param_2 == '\0' and *param_1 != '\0'
    return 1; // param_2 is shorter, param_1 is "greater"
}

int main() {
    printf("Test Cases for compare_strings:\n");

    // Equal strings
    printf("compare_strings(\"hello\", \"hello\"): %d (Expected: 0)\n", compare_strings("hello", "hello"));
    printf("compare_strings(\"\", \"\"): %d (Expected: 0)\n", compare_strings("", ""));

    // First string less than second
    printf("compare_strings(\"apple\", \"banana\"): %d (Expected: -1)\n", compare_strings("apple", "banana"));
    printf("compare_strings(\"test\", \"testing\"): %d (Expected: -1)\n", compare_strings("test", "testing"));
    printf("compare_strings(\"a\", \"b\"): %d (Expected: -1)\n", compare_strings("a", "b"));

    // First string greater than second
    printf("compare_strings(\"banana\", \"apple\"): %d (Expected: 1)\n", compare_strings("banana", "apple"));
    printf("compare_strings(\"testing\", \"test\"): %d (Expected: 1)\n", compare_strings("testing", "test"));
    printf("compare_strings(\"b\", \"a\"): %d (Expected: 1)\n", compare_strings("b", "a"));

    // NULL pointer handling
    printf("compare_strings(NULL, NULL): %d (Expected: 0)\n", compare_strings(NULL, NULL));
    printf("compare_strings(NULL, \"test\"): %d (Expected: -1)\n", compare_strings(NULL, "test"));
    printf("compare_strings(\"test\", NULL): %d (Expected: 1)\n", compare_strings("test", NULL));

    // Mixed cases with different lengths
    printf("compare_strings(\"abc\", \"ab\"): %d (Expected: 1)\n", compare_strings("abc", "ab"));
    printf("compare_strings(\"ab\", \"abc\"): %d (Expected: -1)\n", compare_strings("ab", "abc"));
    printf("compare_strings(\"ab\", \"az\"): %d (Expected: -1)\n", compare_strings("ab", "az"));

    return 0;
}