#include <stdio.h>    // For printf (in main)
#include <string.h>   // For strlen, memcmp
#include <stdlib.h>   // For strtol
#include <ctype.h>    // For isalpha, isalnum, isdigit, isprint

// Define a common return type for verification functions.
// 0 for success, -1 for failure.
#define VERIFY_SUCCESS 0
#define VERIFY_FAILURE -1

// --- Global data definitions for verify_choice functions ---
// The original code passed counts like 0xe (14), 10, 0xc (12), 0x33 (51), 3.
// Arrays are defined with at least these many elements.

const char* const valid_hair_colors[] = {
    "black", "brown", "blonde", "red", "grey", "white", "bald", "other",
    "pink", "blue", "green", "purple", "orange", "yellow" // 14 elements
};
const unsigned int NUM_VALID_HAIR_COLORS = sizeof(valid_hair_colors) / sizeof(valid_hair_colors[0]);

const char* const valid_eye_colors[] = {
    "blue", "brown", "green", "hazel", "grey", "other",
    "black", "red", "violet", "amber" // 10 elements
};
const unsigned int NUM_VALID_EYE_COLORS = sizeof(valid_eye_colors) / sizeof(valid_eye_colors[0]);

const char* const valid_suffixes[] = {
    "Jr.", "Sr.", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX", "X", "XI" // 12 elements
};
const unsigned int NUM_VALID_SUFFIXES = sizeof(valid_suffixes) / sizeof(valid_suffixes[0]);

const char* const valid_states[] = {
    "AL", "AK", "AZ", "AR", "CA", "CO", "CT", "DE", "FL", "GA", "HI", "ID", "IL", "IN", "IA", "KS", "KY", "LA", "ME", "MD", "MA", "MI", "MN", "MS", "MO", "MT", "NE", "NV", "NH", "NJ", "NM", "NY", "NC", "ND", "OH", "OK", "OR", "PA", "RI", "SC", "SD", "TN", "TX", "UT", "VT", "VA", "WA", "WV", "WI", "WY", "DC" // 51 elements
};
const unsigned int NUM_VALID_STATES = sizeof(valid_states) / sizeof(valid_states[0]);

const char* const education_levels[] = {
    "none", "high school", "associate's" // 3 elements as per original call (param_3 = 3)
};
const unsigned int NUM_EDUCATION_LEVELS = sizeof(education_levels) / sizeof(education_levels[0]);

// Function: verify_choice
// Renamed types for clarity and standard C practices.
// Returns 0 on success (match found), -1 on failure (no match).
// The original code uses memcmp with the length of the *valid choice* string.
// This implies an exact match is expected for choices, checking both string content and length.
int verify_choice(const char* const* choices, const char* choice, unsigned int num_choices) {
    size_t choice_len = strlen(choice);
    for (unsigned int i = 0; i < num_choices; ++i) {
        size_t valid_choice_len = strlen(choices[i]);
        if (valid_choice_len == choice_len && memcmp(choices[i], choice, valid_choice_len) == 0) {
            return VERIFY_SUCCESS; // Match found
        }
    }
    return VERIFY_FAILURE; // No match
}

// Functions that call verify_choice:
// Changed return type to int to propagate verification result.
int verify_hair(const char* hair_color) {
  return verify_choice(valid_hair_colors, hair_color, NUM_VALID_HAIR_COLORS);
}

int verify_eye(const char* eye_color) {
  return verify_choice(valid_eye_colors, eye_color, NUM_VALID_EYE_COLORS);
}

int verify_suffix(const char* suffix) {
  return verify_choice(valid_suffixes, suffix, NUM_VALID_SUFFIXES);
}

int verify_state(const char* state) {
  return verify_choice(valid_states, state, NUM_VALID_STATES);
}

int verify_education(const char* education_level) {
  return verify_choice(education_levels, education_level, NUM_EDUCATION_LEVELS);
}

// Function: verify_month
// `expected_len_with_null` is interpreted as the expected string length INCLUDING the null terminator.
int verify_month(const char* str, int expected_len_with_null) {
    size_t len = strlen(str);
    if (len + 1 != expected_len_with_null) {
        return VERIFY_FAILURE;
    }
    long value = strtol(str, NULL, 10);
    if ((value < 1) || (12 < value)) { // Months 1-12
        return VERIFY_FAILURE;
    }
    return VERIFY_SUCCESS;
}

// Function: verify_day
int verify_day(const char* str, int expected_len_with_null) {
    size_t len = strlen(str);
    if (len + 1 != expected_len_with_null) {
        return VERIFY_FAILURE;
    }
    long value = strtol(str, NULL, 10);
    if ((value < 1) || (31 < value)) { // Days 1-31
        return VERIFY_FAILURE;
    }
    return VERIFY_SUCCESS;
}

// Function: verify_year
int verify_year(const char* str, int expected_len_with_null) {
    size_t len = strlen(str);
    if (len + 1 != expected_len_with_null) {
        return VERIFY_FAILURE;
    }
    long value = strtol(str, NULL, 10);
    if ((value < 1900) || (2014 < value)) { // Years 1900-2014 (0x76c = 1900, 0x7de = 2014)
        return VERIFY_FAILURE;
    }
    return VERIFY_SUCCESS;
}

// Function: verify_height_feet
// `max_len` is interpreted as the maximum allowed buffer size (including null).
int verify_height_feet(const char* str, unsigned int max_len) {
    size_t len = strlen(str);
    if (max_len < len + 1) { // Check if string fits within buffer
        return VERIFY_FAILURE;
    }
    long value = strtol(str, NULL, 10);
    if ((value < 1) || (9 < value)) { // Feet 1-9
        return VERIFY_FAILURE;
    }
    return VERIFY_SUCCESS;
}

// Function: verify_height_inches
int verify_height_inches(const char* str, unsigned int max_len) {
    size_t len = strlen(str);
    if (max_len < len + 1) {
        return VERIFY_FAILURE;
    }
    long value = strtol(str, NULL, 10);
    if ((value < 0) || (11 < value)) { // Inches 0-11
        return VERIFY_FAILURE;
    }
    return VERIFY_SUCCESS;
}

// Function: verify_weight
int verify_weight(const char* str, unsigned int max_len) {
    size_t len = strlen(str);
    if (max_len < len + 1) {
        return VERIFY_FAILURE;
    }
    long value = strtol(str, NULL, 10);
    if ((value < 50) || (500 < value)) { // Weight 50-500 lbs (0x32 = 50)
        return VERIFY_FAILURE;
    }
    return VERIFY_SUCCESS;
}

// Function: verify_sex
int verify_sex(const char* str, int expected_len_with_null) {
    size_t len = strlen(str);
    if (len + 1 != expected_len_with_null) {
        return VERIFY_FAILURE;
    }
    if ((*str == 'M') || (*str == 'F')) {
        return VERIFY_SUCCESS;
    }
    return VERIFY_FAILURE;
}

// Function: verify_yes_no
int verify_yes_no(const char* str, int expected_len_with_null) {
    size_t len = strlen(str);
    if (len + 1 != expected_len_with_null) {
        return VERIFY_FAILURE;
    }
    if ((*str == 'Y') || (*str == 'N')) {
        return VERIFY_SUCCESS;
    }
    return VERIFY_FAILURE;
}

// Function: verify_name
int verify_name(const char* str, unsigned int max_len) {
    size_t len = strlen(str);
    if ((max_len < len + 1) || (len + 1 < 2)) { // Min length 1 char + null
        return VERIFY_FAILURE;
    }
    for (size_t i = 0; i < len; ++i) {
        if (!isalpha((unsigned char)str[i]) && (str[i] != '\'')) {
            return VERIFY_FAILURE;
        }
    }
    return VERIFY_SUCCESS;
}

// Function: verify_street
int verify_street(const char* str, unsigned int max_len) {
    size_t len = strlen(str);
    if ((max_len < len + 1) || (len + 1 < 5)) { // Min length 4 chars + null
        return VERIFY_FAILURE;
    }
    for (size_t i = 0; i < len; ++i) {
        char c = str[i];
        if (!isalnum((unsigned char)c) && (c != '\'') && (c != ' ') && (c != ',')) {
            return VERIFY_FAILURE;
        }
    }
    return VERIFY_SUCCESS;
}

// Function: verify_city
int verify_city(const char* str, unsigned int max_len) {
    size_t len = strlen(str);
    if ((max_len < len + 1) || (len + 1 < 5)) { // Min length 4 chars + null
        return VERIFY_FAILURE;
    }
    for (size_t i = 0; i < len; ++i) {
        char c = str[i];
        if (!isalpha((unsigned char)c) && (c != '\'') && (c != ' ')) {
            return VERIFY_FAILURE;
        }
    }
    return VERIFY_SUCCESS;
}

// Function: verify_zip_code
int verify_zip_code(const char* str, int expected_len_with_null) {
    size_t len = strlen(str);
    if (len + 1 != expected_len_with_null) { // Exact length match
        return VERIFY_FAILURE;
    }
    if (*str == '0') { // Original code disallows zip codes starting with '0'
        return VERIFY_FAILURE;
    }
    for (size_t i = 0; i < len; ++i) {
        if (!isdigit((unsigned char)str[i])) {
            return VERIFY_FAILURE;
        }
    }
    return VERIFY_SUCCESS;
}

// Function: verify_gpa
int verify_gpa(const char* str, unsigned int max_len) {
    size_t len = strlen(str);
    if (max_len < len + 1) {
        return VERIFY_FAILURE;
    }
    // Original logic expects a specific format, e.g., "D.DD" where D is a digit.
    // It checks that the character at index 1 is a '.', and others are digits.
    // This implies formats like "X", "X.X", "X.XX".
    if (len == 0 || len > 4) { // Example: "0", "4.0", "3.99"
        return VERIFY_FAILURE;
    }
    for (size_t i = 0; i < len; ++i) {
        if (i == 1) {
            if (str[i] != '.') {
                return VERIFY_FAILURE;
            }
        } else {
            if (!isdigit((unsigned char)str[i])) {
                return VERIFY_FAILURE;
            }
        }
    }
    return VERIFY_SUCCESS;
}

// Function: verify_email
int verify_email(const char* str, unsigned int max_len) {
    size_t len = strlen(str);
    if ((max_len < len + 1) || (len + 1 < 3)) { // Min length 2 chars + null, e.g., "a@b"
        return VERIFY_FAILURE;
    }
    int at_count = 0;
    for (size_t i = 0; i < len; ++i) {
        char c = str[i];
        if (!isalnum((unsigned char)c) && (c != '@') && (c != '-') && (c != '.') && (c != '_')) {
            return VERIFY_FAILURE;
        }
        if (c == '@') {
            at_count++;
            if (at_count > 1) { // Only one '@' allowed
                return VERIFY_FAILURE;
            }
        }
    }
    if (at_count == 1) { // Must contain exactly one '@'
        return VERIFY_SUCCESS;
    }
    return VERIFY_FAILURE;
}

// Function: verify_phone
int verify_phone(const char* str, int expected_len_with_null) {
    size_t len = strlen(str);
    if (len + 1 != expected_len_with_null) { // e.g., "XXX-XXX-XXXX" is 12 chars + null = 13
        return VERIFY_FAILURE;
    }
    for (size_t i = 0; i < len; ++i) {
        if ((i == 3) || (i == 7)) {
            if (str[i] != '-') {
                return VERIFY_FAILURE;
            }
        } else {
            if (!isdigit((unsigned char)str[i])) {
                return VERIFY_FAILURE;
            }
        }
    }
    return VERIFY_SUCCESS;
}

// Function: verify_number
// Verifies if a string contains only digits.
int verify_number(const char* str, unsigned int max_len) {
    size_t len = strlen(str);
    if (max_len < len + 1) {
        return VERIFY_FAILURE;
    }
    for (size_t i = 0; i < len; ++i) {
        if (!isdigit((unsigned char)str[i])) {
            return VERIFY_FAILURE;
        }
    }
    return VERIFY_SUCCESS;
}

// Function: verify_text
// Verifies if a string contains only printable characters.
int verify_text(const char* str, unsigned int max_len) {
    size_t len = strlen(str);
    if (max_len < len + 1) {
        return VERIFY_FAILURE;
    }
    for (size_t i = 0; i < len; ++i) {
        if (!isprint((unsigned char)str[i])) {
            return VERIFY_FAILURE;
        }
    }
    return VERIFY_SUCCESS;
}

// Main function to demonstrate usage
int main() {
    printf("--- Verification Tests ---\n\n");

    // Test verify_choice related functions
    printf("Verify Hair Color 'blonde': %s\n", verify_hair("blonde") == VERIFY_SUCCESS ? "SUCCESS" : "FAILURE");
    printf("Verify Hair Color 'purple': %s\n", verify_hair("purple") == VERIFY_SUCCESS ? "SUCCESS" : "FAILURE");
    printf("Verify Hair Color 'unknown': %s\n", verify_hair("unknown") == VERIFY_SUCCESS ? "SUCCESS" : "FAILURE");
    printf("\n");

    // Test verify_month
    printf("Verify Month '05' (len 3): %s\n", verify_month("05", 3) == VERIFY_SUCCESS ? "SUCCESS" : "FAILURE");
    printf("Verify Month '13' (len 3): %s\n", verify_month("13", 3) == VERIFY_SUCCESS ? "SUCCESS" : "FAILURE");
    printf("Verify Month '5' (len 2): %s\n", verify_month("5", 2) == VERIFY_SUCCESS ? "SUCCESS" : "FAILURE"); // Fails due to length
    printf("\n");

    // Test verify_year
    printf("Verify Year '1999' (len 5): %s\n", verify_year("1999", 5) == VERIFY_SUCCESS ? "SUCCESS" : "FAILURE");
    printf("Verify Year '1899' (len 5): %s\n", verify_year("1899", 5) == VERIFY_SUCCESS ? "SUCCESS" : "FAILURE");
    printf("Verify Year '2023' (len 5): %s\n", verify_year("2023", 5) == VERIFY_SUCCESS ? "SUCCESS" : "FAILURE"); // Fails, year > 2014
    printf("\n");

    // Test verify_name
    printf("Verify Name 'John' (max 10): %s\n", verify_name("John", 10) == VERIFY_SUCCESS ? "SUCCESS" : "FAILURE");
    printf("Verify Name 'O''Malley' (max 10): %s\n", verify_name("O''Malley", 10) == VERIFY_SUCCESS ? "SUCCESS" : "FAILURE");
    printf("Verify Name 'John Doe' (max 10): %s\n", verify_name("John Doe", 10) == VERIFY_SUCCESS ? "SUCCESS" : "FAILURE"); // Fails, space not allowed
    printf("\n");

    // Test verify_email
    printf("Verify Email 'test@example.com' (max 20): %s\n", verify_email("test@example.com", 20) == VERIFY_SUCCESS ? "SUCCESS" : "FAILURE");
    printf("Verify Email 'test.user@sub.example.com' (max 30): %s\n", verify_email("test.user@sub.example.com", 30) == VERIFY_SUCCESS ? "SUCCESS" : "FAILURE");
    printf("Verify Email 'no-at-sign.com' (max 20): %s\n", verify_email("no-at-sign.com", 20) == VERIFY_SUCCESS ? "SUCCESS" : "FAILURE"); // Fails, no @
    printf("Verify Email 'two@at@signs.com' (max 20): %s\n", verify_email("two@at@signs.com", 20) == VERIFY_SUCCESS ? "SUCCESS" : "FAILURE"); // Fails, two @
    printf("\n");

    // Test verify_phone
    printf("Verify Phone '123-456-7890' (len 13): %s\n", verify_phone("123-456-7890", 13) == VERIFY_SUCCESS ? "SUCCESS" : "FAILURE");
    printf("Verify Phone '1234567890' (len 11): %s\n", verify_phone("1234567890", 11) == VERIFY_SUCCESS ? "SUCCESS" : "FAILURE"); // Fails, no dashes
    printf("Verify Phone '123-ABC-7890' (len 13): %s\n", verify_phone("123-ABC-7890", 13) == VERIFY_SUCCESS ? "SUCCESS" : "FAILURE"); // Fails, letters
    printf("\n");

    // Test verify_gpa
    printf("Verify GPA '3.5' (max 5): %s\n", verify_gpa("3.5", 5) == VERIFY_SUCCESS ? "SUCCESS" : "FAILURE");
    printf("Verify GPA '4.00' (max 5): %s\n", verify_gpa("4.00", 5) == VERIFY_SUCCESS ? "SUCCESS" : "FAILURE");
    printf("Verify GPA '35' (max 5): %s\n", verify_gpa("35", 5) == VERIFY_SUCCESS ? "SUCCESS" : "FAILURE"); // Fails, no dot at index 1
    printf("Verify GPA '3.A0' (max 5): %s\n", verify_gpa("3.A0", 5) == VERIFY_SUCCESS ? "SUCCESS" : "FAILURE"); // Fails, non-digit
    printf("Verify GPA '4' (max 5): %s\n", verify_gpa("4", 5) == VERIFY_SUCCESS ? "SUCCESS" : "FAILURE"); // Passes (no dot required if len < 2)
    printf("\n");

    return 0;
}